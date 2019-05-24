#include "names.h"

char * dottynames[] = {
	"<error>",
	"byte",
	"word",
	"long",
	"instr",
	"struct",
	"enum",
	"string"
};

char * basetypenames[] = {
	"undef",
	"farg",
	"char",
	"short",
	"int",
	"long",
	"float",
	"double",
	"struct",
	"union",
	"enum",
	"moe",
	"uchar",
	"ushort",
	"unsigned",
	"ulong",
	"void",
	0
};

char * storagenames[] = {
	"err",
	"glb",
	"sta",
	"com",
	"reg",
	"lcl",
	"arg",
	0
};

char * regnames[] = {
	"d0","d1", "d2", "d3", "d4", "d5", "d6", "d7",
	"a0","a1", "a2", "a3", "a4", "a5", "a6/fp", "a7/sp",
	0
};

char * ntypenames[] = {
	"UNDF",
	"EUNDF",
	"ABS",
	"EABS",
	"TEXT",
	"ETEXT",
	"DATA",
	"EDATA",
	"BSS",
	"EBSS",
	"CTXT",
	"ECTXT",
	"RDAT",
	"ERDAT",
	"COMM",
	"ECOMM",
	0
};
	
char *sdbnames[] = {
	"0",
	"GSYM",
	"STSYM",
	"LCSYM",
	"RSYM",
	"LSYM",
	"PSYM",
	"BSTR",
	"ESTR",
	"SSYM",
	"BENUM",
	"EENUM",
	"ENUM",
	"BCOMM",
	"ECOMM",
	"ECOML",
	"LBRAC",
	"RBRAC",
	"BFUN",
	"ENTRY",
	"SLINE",
	"SO",
	"SOL",
	"FNAME",
	"LENG",
	"EFUN",
	"DLINE",
	"0x1b",
	"0x1c",
	"0x1d",
	"0x1e",
	"0x1f",
	"NARGS",
	"TYID",
	"DIM",
	0
};

char *trapnames[] = {
	"BREAKPOINT",
	"TRACE",
	"BUS ERROR",
	"ADDRESS ERROR",
	"ILLEGAL INSTRUCTION",
	"ZERO DIVIDE",
	"CHK INSTRUCTION",
	"TRAPV INSTRUCTION",
	"PRIVILEGE VIOLATION",
	"SETUP KEY",
	"1011 EMULATOR"
};

nameindex( l, id )		/* assumes you don't search for [0] */
char *l[], *id;
{	int i;

	for( i = 1; l[i]; ++i ) if( !strcmp( l[i], id ) ) return i;
	return 0;
}
