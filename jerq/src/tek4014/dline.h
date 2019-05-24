/*
 * Definitions for globals involved with lines
 */
#ifdef LINE_C

short Jdmajor;		/* Delta for major direction */
short Jdminor;		/* Delta for minor direction */
short Jxmajor;	/* flag: is x the major direction? */
short Jslopeneg;	/* flag: is slope of line negative? */
Point Jdsetline();
Point PtCurrent;
Word DotMask;

#else

extern short Jdmajor;		/* Delta for major direction */
extern short Jdminor;		/* Delta for minor direction */
extern short Jxmajor;	/* flag: is x the major direction? */
extern short Jslopeneg;	/* flag: is slope of line negative? */
extern Point Jdsetline();
extern Point PtCurrent;
extern Word DotMask;

#endif
