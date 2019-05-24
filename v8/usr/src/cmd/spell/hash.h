/*      @(#)hash.h      1.1     */
#define HASHWIDTH 27
#define HASHSIZE 134217689L     /*prime under 2^HASHWIDTH*/
#ifdef pdp11
#define INDEXWIDTH 9
#else
#define INDEXWIDTH 10
#endif
#define INDEXSIZE (1<<INDEXWIDTH)
#define NI (INDEXSIZE+1)
#define BYTE 8

extern unsigned *table;
extern unsigned short index[];  /*into dif table based on hi hash bits*/

extern long hash();
