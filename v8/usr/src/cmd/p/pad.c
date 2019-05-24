#include <stdio.h>
#include "pad.h"
#define	NPCBL	8	/* Number of entries in the circular list */

PAD *
Pfopen(f)
	FILE *f;
{
	register PAD *p=NULL;
	register i;
	if(f!=NULL && (p=(PAD *)malloc(sizeof (PAD)))!=NULL){
		p->Pcbl=(_PCBL *)malloc(NPCBL*sizeof(_PCBL));
		if(p->Pcbl==NULL){
			free(p);
			return(NULL);
		}
		for(i=0; i<NPCBL; i++){
			p->Pcbl[i].Pnextp= &p->Pcbl[(i+1)%NPCBL];
			p->Pcbl[(i+1)%NPCBL].Pprevp= &p->Pcbl[i];
			p->Pcbl[i].Pbase=NULL;
			p->Pcbl[i].Phiwat=NULL;
		}
		p->Pfile=f;
		f->_cnt=0;	/* To be safe */
		f->_flag|=_IOREAD;
	}
	return(p);
}

int
_Pfilbuf(p)
register PAD *p;
{
	register _PCBL *pcbl;
	register FILE *f;

	pcbl = p->Pcbl;
	f = p->Pfile;
	if ((f->_flag & _IOREAD) == 0 || f->_flag & _IOSTRG)
		return (EOF);

	/*
	 * if this buffer is full, use next one
	 */
	if (pcbl->Phiwat >= pcbl->Pbase + BUFSIZ) {
		p->Pcbl = pcbl = pcbl->Pnextp;
		f->_base = f->_ptr = pcbl->Pbase;
		f->_cnt = pcbl->Phiwat - pcbl->Pbase;
		}

	if (pcbl->Pbase == NULL) {
		if ((pcbl->Pbase = (char *) malloc(BUFSIZ)) == NULL) {
			fprintf(stderr,"pad: can't malloc\n");	/* ? */
			f->_flag |= _IOERR;
			return (EOF);
			}
		pcbl->Phiwat = f->_base = pcbl->Pbase;
		f->_flag |= _IOMYBUF;
		}

	/*
	 * if no more saved characters in this buffer,
	 * read some new ones
	 */
	if (f->_cnt <= 0) {
		f->_ptr = pcbl->Phiwat;
		f->_cnt = read(fileno(f),f->_ptr,(pcbl->Pbase + BUFSIZ) - f->_ptr);
		if (f->_cnt > 0) {
			pcbl->Phiwat += f->_cnt;
			pcbl->Pnextp->Phiwat = pcbl->Pnextp->Pbase;
			}
		}

	if (--f->_cnt < 0) {
		f->_flag |= (f->_cnt == -1 ? _IOEOF : _IOERR);
		f->_cnt = 0;
		return (EOF);
		}
	return (*f->_ptr++);
}

Pclose(p)
	register PAD *p;
{
	register i;
	register _PCBL *pcbl=p->Pcbl;
	register _PCBL *old;
	for(i=0; i<NPCBL; i++){
		if(pcbl->Pbase)
			free(pcbl->Pbase);
		old=pcbl;
		pcbl=pcbl->Pnextp;
		free(old);
	}
	fclose(p->Pfile);
	free(p);
}

int
Pbackc(p)
	register PAD *p;
{
	register _PCBL *pcbl;
	register FILE *f;
	f=p->Pfile;
	pcbl=p->Pcbl;
	++f->_cnt;
	if(--f->_ptr < pcbl->Pbase){
		pcbl=pcbl->Pprevp;		/* Only local... */
		if(pcbl->Phiwat <= pcbl->Pbase){
			--f->_cnt;
			++f->_ptr;
			return(EOF);
		}
		p->Pcbl=pcbl;			/* ...until now */
		f->_ptr=pcbl->Phiwat-1;
		f->_cnt=1;
	}
	return(*f->_ptr);
}

