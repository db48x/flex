/* flex - tool to generate fast lexical analyzers */

/*  Copyright (c) 1990 The Regents of the University of California. */
/*  All rights reserved. */

/*  This code is derived from software contributed to Berkeley by */
/*  Vern Paxson. */

/*  The United States Government has rights in this work pursuant */
/*  to contract no. DE-AC03-76SF00098 between the United States */
/*  Department of Energy and the University of California. */

/*  This file is part of flex. */

/*  Redistribution and use in source and binary forms, with or without */
/*  modification, are permitted provided that the following conditions */
/*  are met: */

/*  1. Redistributions of source code must retain the above copyright */
/*     notice, this list of conditions and the following disclaimer. */
/*  2. Redistributions in binary form must reproduce the above copyright */
/*     notice, this list of conditions and the following disclaimer in the */
/*     documentation and/or other materials provided with the distribution. */

/*  Neither the name of the University nor the names of its contributors */
/*  may be used to endorse or promote products derived from this software */
/*  without specific prior written permission. */

/*  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR */
/*  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR */
/*  PURPOSE. */


#include "flexdef.h"
#include "tables.h"

/* This typedef is only used for computing footprint sizes,
 * You need to make sure they match reality in the skeleton file to 
 * get accurate numbers, but they don't otherwise matter.
 * FIXME: This should go away when Flex ships only macros.
 */
struct yy_trans_info {int32_t yy_verify; int32_t yy_nxt;};

/* Helper functions */

static const char *rust_get_int16_decl (void)
{
	return (gentables)
		? "static const flex_int16_t %s[%d] = {   0,\n"
		: "static const flex_int16_t * %s = 0;\n";
}


static const char *rust_get_int32_decl (void)
{
	return (gentables)
		? "static const flex_int32_t %s[%d] = {   0,\n"
		: "static const flex_int32_t * %s = 0;\n";
}

static const char *rust_get_state_decl (void)
{
	return (gentables)
		? "static const yy_state_type %s[%d] = {   0,\n"
		: "static const yy_state_type * %s = 0;\n";
}

/* Methods */

static const char *rust_suffix (void)
{
	char   *suffix;
	suffix = "rs";
	return suffix;
}

static void rust_ntod(size_t num_full_table_rows)
// Generate nxt table for ntod
{
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_NXT, (void**)&yy_nxt, sizeof(%s)},\n",
		    ctrl.long_align ? "flex_int32_t" : "flex_int16_t");

	/* Unless -Ca, declare it "short" because it's a real
	 * long-shot that that won't be large enough.
	 */
	if (gentables)
		out_str_dec
			("static const %s yy_nxt[][%d] =\n    {\n",
			 ctrl.long_align ? "flex_int32_t" : "flex_int16_t",
			 num_full_table_rows);
	else {
		out_dec ("#undef YY_NXT_LOLEN\n#define YY_NXT_LOLEN (%d)\n", num_full_table_rows);
		out_str ("static const %s *yy_nxt =0;\n",
			 ctrl.long_align ? "flex_int32_t" : "flex_int16_t");
	}
	/* It would be no good trying to return an allocation size here,
	 * as it's not known before table generation is finished.
	 */
}

static void rust_mkctbl (size_t sz)
// Make full-speed compressed transition table
{
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_TRANSITION, (void**)&yy_transition, sizeof(%s)},\n",
		    (sz >= INT16_MAX
		     || ctrl.long_align) ? "flex_int32_t" : "flex_int16_t");
}

static void rust_mkftbl(void)
// Make full table
{
	// FIXME: why are there two places this is emitted, here and in rust_gentabs_accept()?
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_ACCEPT, (void**)&yy_accept, sizeof(%s)},\n",
		    ctrl.long_align ? "flex_int32_t" : "flex_int16_t");
}

static size_t rust_gentabs_acclist(size_t sz)
// Generate accept list initializer
{
	out_str_dec (ctrl.long_align ? rust_get_int32_decl () :
		     rust_get_int16_decl (), "yy_acclist", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_ACCLIST, (void**)&yy_acclist, sizeof(%s)},\n",
		    ctrl.long_align ? "flex_int32_t" : "flex_int16_t");
	return sz * (ctrl.long_align ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t rust_gentabs_accept(size_t sz)
// Generate accept table initializer
{
	out_str_dec (ctrl.long_align ? rust_get_int32_decl () : rust_get_int16_decl (),
		     "yy_accept", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_ACCEPT, (void**)&yy_accept, sizeof(%s)},\n",
		    ctrl.long_align ? "flex_int32_t" : "flex_int16_t");
	return sz * (ctrl.long_align ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t rust_gentabs_yy_base(size_t sz)
// Generate yy_meta base initializer
{
	out_str_dec ((tblend >= INT16_MAX || ctrl.long_align) ?
		     rust_get_int32_decl () : rust_get_int16_decl (),
		     "yy_base", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_BASE, (void**)&yy_base, sizeof(%s)},\n",
		    (sz >= INT16_MAX
		     || ctrl.long_align) ? "flex_int32_t" : "flex_int16_t");
	return sz * ((sz >= INT16_MAX || ctrl.long_align) ? sizeof(int32_t) : sizeof(int16_t)); 
}

static size_t rust_gentabs_yy_def(size_t sz)
// Generate yy_def initializer
{
	out_str_dec ((sz >= INT16_MAX || ctrl.long_align) ?
		     rust_get_int32_decl () : rust_get_int16_decl (),
		     "yy_def", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_DEF, (void**)&yy_def, sizeof(%s)},\n",
		    (sz >= INT16_MAX
		     || ctrl.long_align) ? "flex_int32_t" : "flex_int16_t");
	return sz * ((sz >= INT16_MAX || ctrl.long_align) ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t rust_gentabs_yy_nxt(size_t tblafter)
// Generate yy_nxt initializer
{
	/* Begin generating yy_nxt */
	out_str_dec ((tblafter >= INT16_MAX || ctrl.long_align) ?
		     rust_get_int32_decl () : rust_get_int16_decl (), "yy_nxt",
		     tblafter);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_NXT, (void**)&yy_nxt, sizeof(%s)},\n",
		    (tblafter >= INT16_MAX
		     || ctrl.long_align) ? "flex_int32_t" : "flex_int16_t");
	return tblafter * ((tblafter >= INT16_MAX || ctrl.long_align) ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t rust_gentabs_yy_chk(size_t tblafter)
// Generate yy_chk initializer
{
	out_str_dec ((tblafter >= INT16_MAX || ctrl.long_align) ?
		     rust_get_int32_decl () : rust_get_int16_decl (), "yy_chk",
		     tblafter);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_CHK, (void**)&yy_chk, sizeof(%s)},\n",
		    (tblafter >= INT16_MAX
		     || ctrl.long_align) ? "flex_int32_t" : "flex_int16_t");
	return tblafter * ((tblafter >= INT16_MAX || ctrl.long_align) ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t rust_nultrans(int fullspd, size_t afterdfa)
// Generate nulltrans initializer
{
	// Making this a backend method may be overzealous.
	// How many other languages have to special-case NUL
	// because it's a string terminator?
	out_str_dec (rust_get_state_decl (), "yy_NUL_trans", afterdfa);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_NUL_TRANS, (void**)&yy_NUL_trans, sizeof(%s)},\n",
		    (fullspd) ? "struct yy_trans_info*" : "flex_int32_t");
	return afterdfa * (fullspd ? sizeof(struct yy_trans_info *) : sizeof(int32_t));
}

static const char *rust_trans_offset_type(int total_table_size)
{
	return (total_table_size >= INT16_MAX || ctrl.long_align) ?
			"flex_int32_t" : "flex_int16_t";
}

const char *rust_skel[] = {
#include "cpp-skel.h"
    0,
};

/* This backend is only accessed through this method table */
struct flex_backend_t rust_backend = {
	.suffix = rust_suffix,
	.skel = rust_skel,
	.ntod = rust_ntod,
	.mkctbl = rust_mkctbl,
	.mkftbl = rust_mkftbl,
	.gentabs_acclist = rust_gentabs_acclist,
	.gentabs_accept = rust_gentabs_accept,
	.gentabs_yy_base = rust_gentabs_yy_base,
	.gentabs_yy_def = rust_gentabs_yy_def,
	.gentabs_yy_nxt = rust_gentabs_yy_nxt,
	.gentabs_yy_chk = rust_gentabs_yy_chk,
	.nultrans = rust_nultrans,
	.trans_offset_type = rust_trans_offset_type,
	.c_like = true,
};
