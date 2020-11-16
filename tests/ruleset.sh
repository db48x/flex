#!/bin/sh
# Generate make productions for testing files from rulesets.  Also set up SOURCES
# variables for link time.  Pass it a list of back-end suffixes.
#
# This script exists because automake isn't able to handle the pattern rules that
# would be natural to use. Output is written to standard output for
# inclusion in a Makefile.am, typically by redirecting the output and
# then an automake include directive.

set -eu

RULESET_TESTS=""
RULESET_REMOVABLES=""

printf "\n# Begin generated test rules\n\n"

compatible() {
    mybackend=$1
    myruleset=$2
    [ "${mybackend}" = "nr" ] || [ "${myruleset}" != "lexcompat.rules" ]
}

for backend in "$@" ; do
    case $backend in
	nr|r|c99) ext="c" ;;
	rust) ext="rs" ;;
	*) ext=${backend} ;;
    esac
    for ruleset in *.rules; do
	if compatible "${backend}" "${ruleset}" ; then
	    testname="${ruleset%.*}_${backend}"
	    echo "${testname}_SOURCES = ${testname}.l"
	    echo "${testname}.l: \$(srcdir)/${ruleset} \$(srcdir)/testmaker.sh \$(srcdir)/testmaker.m4"
	    # we're deliberately single-quoting this because we _don't_ want those variables to be expanded yet
	    # shellcheck disable=2016
	    printf '\t$(SHELL) $(srcdir)/testmaker.sh $@\n\n'
	    RULESET_TESTS="${RULESET_TESTS} ${testname}"
	    RULESET_REMOVABLES="${RULESET_REMOVABLES} ${testname} ${testname}.${ext} ${testname}.l ${testname}.txt"
	fi
    done
    for kind in opt ser ver ; do
        for opt in -Ca -Ce -Cf -CF -Cm -Cem -Cae -Caef -CaeF -Cam -Caem ; do
            bare_opt=${opt#-}
            # The filenames must work on case-insensitive filesystems.
            bare_opt=$(echo ${bare_opt}| sed 's/F$/xF/')
            testname=tableopts_${kind}-${bare_opt}.${kind}_${backend}
            RULESET_TESTS="${RULESET_TESTS} ${testname}"
            RULESET_REMOVABLES="${RULESET_REMOVABLES} ${testname} ${testname}.${ext} ${testname}.l ${testname}.tables"
            cat << EOF
tableopts_${kind}_${bare_opt}_${kind}_${backend}_SOURCES = ${testname}.l
${testname}.l: \$(srcdir)/tableopts.rules \$(srcdir)/testmaker.sh \$(srcdir)/testmaker.m4
	\$(SHELL) \$(srcdir)/testmaker.sh \$@

EOF
        done
    done
done

# posixlycorrect is a special case becaae we need to set POSIXLY_CORRECT
# in Flex's environment while these .l files are bein processed.
for backend in "$@" ; do
    case $backend in
	nr|r|c99) ext="c" ;;
	rust) ext="rs" ;;
	*) ext=${backend} ;;
    esac
    # shellcheck disable=SC2059
    printf "posixlycorrect_${backend}.${ext}: posixlycorrect_${backend}.l \$(FLEX)\n"
    printf "\t\$(AM_V_LEX)POSIXLY_CORRECT=1 \$(FLEX) \$(TESTOPTS) -o \$@ \$<\n"
    echo ""

    echo "test-yydecl-${backend}.sh\$(EXEEXT): test-yydecl-gen.sh"
    # shellcheck disable=SC2059
    printf "\t\$(SHELL) test-yydecl-gen.sh ${backend} >test-yydecl-${backend}.sh\$(EXEEXT)\n"
    # shellcheck disable=SC2059
    printf "\tchmod a+x test-yydecl-${backend}.sh\$(EXEEXT)\n"
    echo ""

    # shellcheck disable=SC2016
    printf '%%_%s.%s: %%_%s.l $(FLEX)\n\t$(AM_V_LEX)$(FLEX) $(TESTOPTS) -o $@ $<\n\n' "${backend}" "${ext}" "${backend}"

    RULESET_TESTS="${RULESET_TESTS} test-yydecl-${backend}.sh"
    RULESET_REMOVABLES="${RULESET_REMOVABLES} test-yydecl-${backend}.sh"
done

echo ""
printf "# End generated test rules\n"

echo RULESET_TESTS = "${RULESET_TESTS}"
echo RULESET_REMOVABLES = "${RULESET_REMOVABLES}"
echo
