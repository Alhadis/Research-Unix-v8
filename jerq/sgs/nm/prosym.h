    /*  prosym.h contains format strings used by prosym( ) and proext( )
     *
     *  Namelist's output is formatted differently for Basic-16 (and its kin).
     *  20 bits of value are printed for Basic-16 whereas
     *  the full 32 bits are printed for all others.
     *
     *  When compiling namelist for Basic-16 use the -DB16 flag for the C
     *  preprocessor 
     *
     *  Different title strings are used depending on the -e flag.  If only
     *  statics and externals are printed, the type field is squished and
     *  the source file name is printed for all statics
     *
     *  The particular format string used at any time depends on the value
     *  of numbase.  If numbase is DECIMAL the first string is used (pr???[0]);
     *  if numbase is HEX, the second string is used (pr???[1]),
     *  otherwise the octal (pr???[2]) is used.
     */

#ifndef B16
/* NON-B16 FORMAT STRINGS */

#define VALMASK(x)	(x)

static char	*prtitle[3] = {
#ifdef TRVEC
"Name                  Value     Class  Tv        Type        Size   Line  Section\n\n",
"Name                  Value       Class  Tv        Type       Size    Line  Section\n\n",
"Name                  Value       Class  Tv        Type       Size    Line  Section\n\n"
#else
"Name                  Value   Class        Type         Size   Line  Section\n\n",
"Name                  Value     Class        Type        Size   Line  Section\n\n",
"Name                    Value     Class      Type       Size    Line  Section\n\n"
#endif
};

static char	*pretitle[3] = {

#ifdef TRVEC
"Name                  Value     Class  Tv       Type       Size   Line  Section File\n\n",
"Name                  Value       Class  Tv      Type     Size    Line  Section File\n\n",
"Name                  Value       Class  Tv      Type     Size    Line  Section File\n\n"
#else
"Name                  Value   Class        Type       Size   Line  Section File\n\n",
"Name                  Value     Class       Type     Size    Line  Section File\n\n",
"Name                  Value       Class      Type     Size    Line  Section File\n\n"
#endif

};

static char	*proffset[3] = {
			"%-20s|%8ld|%-6.6s",
			"%-20s|0x%.8lx|%-6.6s",
			"%-20s|0%.11lo|%-6.6s"
};

static char	*prpoffset[3] = {
			"%08ld %s %s\n",
			"0x%.8lx %s %s\n",
			"0%.11lo %s %s\n"
};

static char	*praddress[3] = {
			"%-20s|%8ld|%-6.6s",
			"%-20s|0x%.8lx|%-6.6s",
			"%-20s|0%.11lo|%-6.6s"
};

static char	*prpaddress[3] = {
			"%08ld %c %s\n",
			"0x%.8lx %c %s\n",
			"0%.11lo %c %s\n"
};

static char	*prnoval[3] = {
			"%-20s|        |%-6.6s",
			"%-20s|          |%-6.6s",
			"%-20s|            |%-6.6s"
};

static char	*prpnoval[3] = {
			"         %s %s\n",
			"           %s %s\n",
			"             %s %s\n"
};

static char	*prfile[3] = {
		"%-20.20s|        | file |                  |      |     |\n",
		"%-20.20s|          | file |                 |      |     |\n",
		"%-20.20s|            | file |              |       |     |\n"
};

static char	*prpfile[3] = {
			"         f %s\n",
			"           f %s\n",
			"             f %s\n"
};

#endif

#ifdef	B16
/* BASIC-16 SPECIFIC FORMAT STRINGS */

/*  VALMASK(x)  determines number of bits of x that will be printed */
#define VALMASK(x)	((x) & 0xfffff)

static char	*prtitle[3] = {
"Name            Value  Class  Tv        Type         Size  Line  Section\n\n",
"Name            Value  Class  Tv        Type         Size  Line  Section\n\n",
"Name            Value       Class  Tv        Type         Size   Line  Section\n\n"
};

static char	*pretitle[3] = {
"Name            Value     Class  Tv       Type        Size  Line  Section File\n\n",
"Name            Value     Class  Tv       Type        Size  Line  Section File\n\n",
"Name            Value   Class  Tv      Type       Size   Line  Section File\n\n"
};

/*  print value field as an offset (don't print leading zeros) */
static char	*proffset[3] = {
			"%-20s|%5ld|%-6.6s",
			"%-20s|0x%.5lx|%-6.6s",
			"%-20s|0%.7lo|%-6.6s"
};

static char	*prpoffset[3] = {
			"%05ld %s %s\n",
			"0x%.5lx %s %s\n",
			"0%.7lo %s %s\n"
};

/*  print value field as an address (do print leading zeros) */
static char	*praddress[3] = {
			"%-20s|%5ld|%-6.6s",
			"%-20s|0x%.5lx|%-6.6s",
			"%-20s|0%.7lo|%-6.6s"
};

static char	*prpaddress[3] = {
			"%05ld %c %s\n",
			"0x%.5lx %c %s\n",
			"0%.7lo %c %s\n"
};

/*  value field is meaningless;  simple print appropriate number of blanks */
static char	*prnoval[3] = {
			"%-20s|       |%-6.6s",
			"%-20s|       |%-6.6s",
			"%-20s|        |%-6.6s"
};

static char	*prpnoval[3] = {
			"         %s %s\n",
			"           %s %s\n",
			"             %s %s\n"
};

/*  print first three fields for a file */
static char	*prfile[3] = {
			"%-20.20s         |file\n",
			"%-20.20s         |file\n",
			"%-20.20s         |file\n"
};

static char	*prpfile[3] = {
			"         f %s\n",
			"           f %s\n",
			"             f %s\n"
};

#endif

/* FORMAT STRINGS FOR ALL GENERICS */

/*  print size information */
static char	*prsize[3] = {
			"|%6hd",
			"|0x%.4hx",
			"|0%.6ho"
};

/*  there is no size information */
static char	*prnosize[3] = {
			"|      ",
			"|      ",
			"|       "
};
/* function size information */
static char	*prfsize[3] = {
			"|%6ld",
			"|0x%.4lx",
			"|0%.6lo"
};

/* there is no function size information */
static char	*prnofsize[3] = {
			"|           ",
			"|           ",
			"|            "
};

/* STORAGE CLASS NAMES */

/*  ordinary C language storage classes (C_FIELD is the largest) */
static	char	*sclass[C_FIELD + 1] = {
				"",
				"auto",
				"extern",
				"static",
				"reg",
				"extdef",
				"label",
				"ulabel",
				"strmem",
				"argm't",
				"strtag",
				"unmem",
				"untag",
				"typdef",
				"ustat",
				"entag",
				"enmem",
				"regprm",
				"bitfld"
};


/*  special debugging symobol storage classes (have values beginning at 100) */
static	char	*scaclass[7] = {
				"block",
				"fcn",
				"endstr",
				"file",
				"error",
				"error",
				"hidden"
};


/*  SCLASS(x)  chooses a string in sclass or scaclass depending on size of x */
#define SCLASS(x)	(x) <= C_FIELD ? sclass[(x)] : scaclass[(x) - 100]


/*
 *	static char	ID_prosh[] = "@(#)prosym.h	1.10 1/4/84";
 */
