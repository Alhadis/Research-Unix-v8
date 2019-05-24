#include <stdio.h>

main()
{
	dumppage(stdin,stdout);
	exit(0);
}

dumppage(in,out)
FILE *in, *out;
{
	int nrasters, rastwid, rsize, i;

	nrasters=getw(in);
	rastwid =getw(in);
	if (rastwid & 0x8000) {
		rsize = rastwid &= ~0x8000;
		rastwid = (rastwid+15)/16;
	} else
		rsize = 16*rastwid;
	fprintf(out,"nrasters = %d, rsize = %d, rastwid = %d\n",
	    nrasters,rsize,rastwid);

	for (i=1; (!feof(in)) && i<=nrasters; i++) {
		fprintf(out,"raster %d:\n",i);
		dumprast(rastwid,in,out);
	}

}

dumprast(nw,in,out)
int nw; FILE *in, *out;
{
	unsigned short count, ctype, word, i, nwords;

	while (nw>0) {
		count=getc(in); if (feof(in)) break;
		count &= 255; ctype=count&128; count &= 127;
		fprintf(out,"%3d%c",count,ctype?'*':' ');
		nw -= count;

		nwords = ctype ? 1 : count;

		for (i=0; i<nwords; i++) {
			fread((char *)&word,2,1,in);
			if (feof(in)) break;
			if (i>0 && i%16==0) fprintf(out,"\n    ");
			fprintf(out," %4.4x",word);
		}
		fprintf(out,"\n");
	}
	if (nw != 0) fprintf(out,"error: nw = %d\n",nw);
}

getw(stream)
FILE *stream;
{
	register l, h;
	l=getc(stream); h=getc(stream); return (h&255)<<8 | (l&255);
}
