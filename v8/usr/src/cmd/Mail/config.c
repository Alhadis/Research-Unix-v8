#ifndef lint
static char *sccsid = "@(#)config.c	1.8 (Berkeley) 8/11/83";
#endif

/*
 * This file contains definitions of network data used by Mail
 * when replying.  See also:  configdefs.h and optim.c
 */

/*
 * The subterfuge with CONFIGFILE is to keep cc from seeing the
 * external defintions in configdefs.h.
 */
#define	CONFIGFILE
#include "configdefs.h"

/*
 * Set of network separator characters.
 */
char	*metanet = "!^:%@.";

/*
 * Host table of "known" hosts.  See the comment in configdefs.h;
 * not all accessible hosts need be here (fortunately).
 */
struct netmach netmach[] = {
	"berkeley",	'7',		AN|SN,
	"a",		'a',		SN,
	"b",		'b',		SN,
	"c",		'c',		SN,
	"d",		'd',		SN,
	"e",		'e',		SN,
	"f",		'f',		SN,
	"g",		'g',		SN,
	"ingres",	'i',		SN,
	"ing70",	'i',		SN,
	"ingvax",	'j',		SN|BN,
	"virus",	'k',		SN,
	"vlsi",		'l',		SN,
	"image",	'm',		SN,
	"esvax",	'o',		SN,
	"sesm",		'o',		SN,
	"ucbcad",	'p',		SN|BN,
	"q",		'q',		SN,
	"kim",		'n',		SN,
	"research",	'R',		BN,
	"icarus",	'I',		BN,
	"arpavax",	'r',		SN|BN,
	"src",		's',		SN,
	"mathstat",	't',		SN,
	"vax",		'v',		BN|SN,
	"ucb",		'v',		BN|SN,
	"ucbvax",	'v',		BN|SN,
	"onyx",		'x',		SN,
	"cory",		'y',		SN,
	"eecs40",	'z',		SN,
	EMPTY,		EMPTYID,	SN,	/* Filled in dynamically */
	0,		0,		0
};

/*
 * Table of ordered of preferred networks.  You probably won't need
 * to fuss with this unless you add a new network character (foolishly).
 */
struct netorder netorder[] = {
	AN,	'@',
	AN,	'%',
	SN,	':',
	BN,	'!',
	-1,	0
};

/*
 * Table to convert from network separator code in address to network
 * bit map kind.  With this transformation, we can deal with more than
 * one character having the same meaning easily.
 */
struct ntypetab ntypetab[] = {
	'%',	AN,
	'@',	AN,
	':',	SN,
	'!',	BN,
	'^',	BN,
	0,	0
};

struct nkindtab nkindtab[] = {
	AN,	IMPLICIT,
	BN,	EXPLICIT,
	SN,	IMPLICIT,
	0,	0
};
