typedef struct _PCBL{
	struct _PCBL *Pnextp;
	struct _PCBL *Pprevp;
	char *Pbase;		/* Pointer to head of character buffer */
	char *Pptr;		/* Pointer to current read position */
	char *Phiwat;		/* Pointer to high water mark for this buffer */
}_PCBL;		/* Pad circular buffer list */
typedef struct _PAD{
	_PCBL	*Pcbl;		/* Circular list for this file descriptor */
	FILE	*Pfile;		/* Stdio file structure */
}PAD;
#define	Pgetc(p)	(--(p)->Pfile->_cnt>=0? *(p)->Pfile->_ptr++&0377:_Pfilbuf(p))
#define	Pgetchar()	Pgetc(Pstdin)
PAD *Popen();
PAD *Pfopen();
PAD *Pfdopen();
