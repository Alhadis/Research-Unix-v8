/* NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT */
/* Writer's Workbench version 2.0, January 1981 */
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#define MAXDOCS 75
#define NVAR 10
#define MINSENT 90
#define MINWDS 1900
#define FALSE 0
#define TRUE 1
#define MINN 20

char filen[MAXDOCS][15], extra[MAXDOCS], extra2[MAXDOCS];
float kindex[MAXDOCS], avw[MAXDOCS],snonf[MAXDOCS],  simple[MAXDOCS],complex[MAXDOCS], compound[MAXDOCS],  compdx[MAXDOCS];
float ppron[MAXDOCS],padj[MAXDOCS];
float passive[MAXDOCS],explet[MAXDOCS];
int nomin[MAXDOCS],x[MAXDOCS];
int maxsent[MAXDOCS], numsent[MAXDOCS], numwds[MAXDOCS],qcount[MAXDOCS], icount[MAXDOCS],numnonf[MAXDOCS],ml[MAXDOCS],lsum[MAXDOCS],mg[MAXDOCS],gsum[MAXDOCS];
int maxindex[MAXDOCS],minindex[MAXDOCS],minsent[MAXDOCS];
float aindex[MAXDOCS],cindex[MAXDOCS],fgrad[MAXDOCS],findex[MAXDOCS],avl[MAXDOCS],fnumnonf[MAXDOCS],centlsum[MAXDOCS],centgsum[MAXDOCS];
int nsimple[MAXDOCS],ncomplex[MAXDOCS],ncompound[MAXDOCS],ncompdx[MAXDOCS],ntobe[MAXDOCS];
int npassive[MAXDOCS],pron[MAXDOCS],pos[MAXDOCS],adj[MAXDOCS],art[MAXDOCS],beg[MAXDOCS],begadv[MAXDOCS],begprep[MAXDOCS],begverb[MAXDOCS],begscon[MAXDOCS];
int begconj[MAXDOCS], begexp[MAXDOCS],noun[MAXDOCS];
float ptobe[MAXDOCS],paux[MAXDOCS],pinfin[MAXDOCS],padv[MAXDOCS],pconjc[MAXDOCS],pprepc[MAXDOCS],pnoun[MAXDOCS],pnomin[MAXDOCS],tot[MAXDOCS];
float pbegprep[MAXDOCS],pbegadv[MAXDOCS],pbegverb[MAXDOCS],pbegscon[MAXDOCS],pbegconj[MAXDOCS],pbegexp[MAXDOCS];
int tobe[MAXDOCS],aux[MAXDOCS],infin[MAXDOCS],prepc[MAXDOCS],conjc[MAXDOCS],adv[MAXDOCS];

FILE *fopen(), *stylout, *out, *scores, *tmp;


main(argc, argv)
int argc;
char *argv[];
{
	int n, i, j, x;
	int aok,ok,once,lmesg;
	int number, orignum;
	float m[NVAR], sd[NVAR];
	float *score[NVAR], mm2[NVAR], mm1[NVAR], mp1[NVAR], mp2[NVAR];
	char *file[MAXDOCS];
	float mean(), sdev();
	float smpmcmp[MAXDOCS], cmpcmdx[MAXDOCS];
	int nvar;
	char name[14], *pid;

	/*  Begin  */

	number = *argv[1];
	number=number-'0';
	if(isdigit(*(argv[1]+1)))number=10*number+(*(argv[1]+1)-'0');
	orignum=number;
	pid=argv[2];
	sprintf(name,"/tmp/%sstat.out",pid);
	if((stylout=fopen(name,"r"))==NULL) {
		fprintf(stderr, "Can't read %s.\n",name);
		exit(2);
	}
	if((out=fopen("stand.out","w"))==NULL) {
		printf("Can't write \"stand.out\".\n");
		exit(2);
	}
	if((scores=fopen("styl.scores","w"))==NULL) {
		printf("Can't write \"styl.scores\".\n");
		exit(2);
	}

	aok=FALSE;
	once=FALSE;
	lmesg=FALSE;

	/*  Read style scores  */

	for(n=0; n<number; n++) {
		fscanf(stylout,"%s %s %s\n", extra[n],extra2[n],filen[n]);
		fscanf(stylout,"readability grades:\n");
		fscanf(stylout,"	(Kincaid) %f  (auto) %f  (Coleman-Liau) %f  (Flesch) %f (%f)",&kindex[n],&aindex[n],&cindex[n],&fgrad[n],&findex[n]);
		fscanf(stylout," sentence info:");
		fscanf(stylout," no. sent %d no. wds %ld",&numsent[n],&numwds[n]);
		fscanf(stylout," av sent leng %f av word leng %f",&avw[n],&avl[n]);
		fscanf(stylout," no. questions %d no. imperatives %d",&qcount[n],&icount[n]);
		fscanf(stylout," no. content wds %d  %f%%   av leng %f",&numnonf[n],&fnumnonf[n],&snonf[n]);
		fscanf(stylout," short sent (<%d)%f%% (%d) long sent (>%d) %f%% (%d)",&ml[n],&centlsum[n],&lsum[n],&mg[n],&centgsum[n],&gsum[n]);
		fscanf(stylout," longest sent %d wds at sent %d; shortest sent %d wds at sent %d",&maxsent[n],&maxindex[n],&minsent[n],&minindex[n]);
		fscanf(stylout," sentence types:");
		fscanf(stylout," simple %f%% (%d) complex %f%% (%d)",&simple[n],&nsimple[n],&complex[n],&ncomplex[n]);
		fscanf(stylout," compound %f%% (%d) compound-complex %f%% (%d)",&compound[n],&ncompound[n],&compdx[n],&ncompdx[n]);
		fscanf(stylout," word usage:");
		fscanf(stylout," verb types as %% of total verbs");
		fscanf(stylout," tobe %f%% (%d) aux %f%% (%d) inf %f%% (%d)",&ptobe[n],&tobe[n],&paux[n],&aux[n],&pinfin[n],&infin[n]);
		fscanf(stylout," passives as %% of non-inf verbs %f%% (%d)",&passive[n],&npassive[n]);
		fscanf(stylout," types as %% of total");
		fscanf(stylout," prep %f%% (%d) conj %f%% (%d) adv %f%% (%d)",&pprepc[n],&prepc[n],&pconjc[n],&conjc[n],&padv[n],&adv[n]);
		fscanf(stylout," noun %f%% (%d) adj %f%% (%d) pron %f%% (%d)",&pnoun[n],&noun[n], &padj[n],&adj[n],&ppron[n],&pron[n]);
		fscanf(stylout," nominalizations %f %% (%d)",&pnomin[n],&nomin[n]);
		fscanf(stylout," sentence beginnings:");
		fscanf(stylout," subject opener: noun (%d) pron (%d) pos (%d) adj (%d) art (%d) tot %f%%",&noun[n],&pron[n],&pos[n],&adj[n],&art[n],&tot[n]);
		fscanf(stylout," prep %f%% (%d) adv %f%% (%d) ",&pbegprep[n],&begprep[n],&pbegadv[n],&begadv[n]);
		fscanf(stylout," verb %f%% (%d) ",&pbegverb[n],&begverb[n]);
		fscanf(stylout,"  sub_conj %f%% (%d) conj %f%% (%d)",&pbegscon[n],&begscon[n],&pbegconj[n],&begconj[n]);
		fscanf(stylout," expletives %f%% (%d)",&pbegexp[n],&begexp[n]);
	}
	fclose(stylout);


	/*  Set file to point at filen  */
	for(n=0; n<number; n++) file[n]=filen[n];

	/*  Compute sentence variation and complexity scores  */

	for(n=0; n<number; n++) {
		smpmcmp[n] = simple[n] - complex[n];
		cmpcmdx[n] = compound[n] + compdx[n];
	}

	/*  Set score arrays  */

	score[0]=kindex;
	score[1]=snonf;
	score[2]=centlsum;
	score[3]=centgsum;
	score[4]=passive;
	score[5]=pnomin;
	score[6]=avw;
	score[7]=pbegexp;
	score[8]=smpmcmp;
	score[9]=cmpcmdx;
	nvar=NVAR;

	/*  Check file lengths  */
	n=0;
	while(n < number) {
		if(numsent[n] < MINSENT && numwds[n] < MINWDS) {
			if(lmesg==FALSE) {
				printf("Files must be at least 1900 words OR 90 sentences long,\nfor mkstand to compute reliable standards.\n");
				lmesg=TRUE;
			}
			printf("File %s is too short, its scores will be excluded.\n", file[n]);
			for(i=n; i<number-1; i++) {
				file[i]=file[i+1];
				kindex[i]=kindex[i+1];
				snonf[i]=snonf[i+1];
				centlsum[i]=centlsum[i+1];
				centgsum[i]=centgsum[i+1];
				passive[i]=passive[i+1];
				pnomin[i]=pnomin[i+1];
				avw[i]=avw[i+1];
				pbegexp[i]=pbegexp[i+1];
				smpmcmp[i]=smpmcmp[i+1];
				cmpcmdx[i]=cmpcmdx[i+1];
				numwds[i]=numwds[i+1];
				numsent[i]=numsent[i+1];
			}
			number=number-1;
		}
		else n++;
	}

	/*  While aok = FALSE  */

	while(aok==FALSE) {
		ok=TRUE;

		/*  Check new number  */

		if(number<=1) {
			if(number==0) printf("All the input files ");
			if(number==1) printf("All the input files but one ");
			printf("were too short\nfor mkstand to compute reliable standards.\n");
			printf("Rerun mkstand with files that are longer than 1900 words or 90 sentences.\n");
			exit(2);
		}
		if(number < MINN && orignum >= MINN) {
			printf("Since some files had to be excluded,\nthere are no longer 20 files.\n");
			printf("Mkstand will compute the standards,\n");
			printf("but they would probably be more reliable.\n");
			printf("if you run mkstand again with additional files\n");
			printf("to replace those that were excluded.\n");
		}

		/*  Compute standards  */

		for(i=0; i<nvar; i++) {
			m[i]=mean(score[i],number);

			sd[i]=sdev(score[i],m[i],number);

			mm2[i] = m[i] - 2 * sd[i];
			mm1[i] = m[i] - sd[i];
			mp1[i] = m[i] + sd[i];
			mp2[i] = m[i] + 2 * sd[i];

		}

		/*  Check for outliers  */

		if(once==FALSE) {
			n=0;
			while(n < number) {
				for(i=0; i<nvar; i++) {
					if(score[i][n] < mm2[i] || score[i][n] > mp2[i]) {
						ok=FALSE;
						once=TRUE;
						printf("File %s has a score more than 2 standard deviations from the mean,\nand will be excluded.\n", file[n]);
						for(j=n; j<number-1; j++) {
							file[j]=file[j+1];
							kindex[j]=kindex[j+1];
							snonf[j]=snonf[j+1];
							centlsum[j]=centlsum[j+1];
							centgsum[j]=centgsum[j+1];
							passive[j]=passive[j+1];
							pnomin[j]=pnomin[j+1];
							avw[j]=avw[j+1];
							pbegexp[j]=pbegexp[j+1];
							smpmcmp[j]=smpmcmp[j+1];
							cmpcmdx[j]=cmpcmdx[j+1];
						}
						number=number-1;
						i=nvar;
					}
					else if(i==nvar-1) n++;
				}
			}
			if(ok != FALSE) aok=TRUE;
		}
		else aok=TRUE;
	}

	/*  Print scores for examination  */

	printf("The style scores used to compile the standards\n");
	printf("are in a file named styl.scores.  Please examine\n");
	printf("this file for any scores that seem unusual or invalid.\n");
	printf("If you find any, rerun mkstand without the unusual\n");
	printf("document.\n");
	for(n=0;n<number; n++) {
		fprintf(scores,"Document %s:\n", file[n]);
		fprintf(scores,"\tKincaid readability  %4.1f\n", kindex[n]);
		fprintf(scores,"\tAve. length of non-function words  %4.1f\n", snonf[n]);
		fprintf(scores,"\t%% short sentences  %4.1f\n", centlsum[n]);
		fprintf(scores,"\t%% long sentences  %4.1f\n", centgsum[n]);
		fprintf(scores,"\t%% passives  %4.1f\n", passive[n]);
		fprintf(scores,"\t%% nominalizations  %4.1f\n", pnomin[n]);
		fprintf(scores,"\tAve. sentence length  %4.1f\n", avw[n]);
		fprintf(scores,"\t%% expletives as sentence beginners  %4.1f\n", pbegexp[n]);
		fprintf(scores,"\t%% simple sentences minus %% complex sentences  %4.1f\n", smpmcmp[n]);
		fprintf(scores,"\t%% compound sentences plus %% compound-complex sentences  %4.1f\n", cmpcmdx[n]);
		fprintf(scores,"\n");
	}

	/*  Write standards to stand.out  */

	for(i=0; i<nvar; i++) {
		fprintf(out,"%4.2f %4.2f %4.2f %4.2f\n", mm2[i], mm1[i], mp1[i], mp2[i]);
		if(i==3)
			fprintf(out,"0.0 0.0 0.0 0.0\n0.0 0.0 0.0 0.0\n0.0 0.0 0.0 0.0\n0.0 0.0 0.0 0.0\n");
	}
}
float sdev(score,m,n)
float score[], m;
int n;
{
	float *sc, sumsq;
double sqrt();

	sumsq=0.0;
	for(sc = score; sc < score + n; sc++) sumsq += (*sc-m) * (*sc-m);
	return(sqrt(sumsq/(n-1)));
}
float mean(score,n)
float score[];
int n;
{

	float *sc, sum;

	sum=0.0;
	for(sc = score; sc < score +n; sc++) sum += *sc;
	return(sum/n);
}
