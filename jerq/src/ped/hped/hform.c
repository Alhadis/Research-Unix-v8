/*
*	Beautification program	
*	n=number of object pointers in oa[]
*
*	it coisiders two kinds of relations:
*	those that are nonlinear, they create an equation
*	for each cluster member and use the cluster value
*	(e.g mean slope)
*	linear relations pair the first cluster member
*	with all the others, and create one less equ.
*	than cluster members. the cluster value is ignored
*
*	point relations are treated through pseudosides
*	so that we can use the same clustering routines.
*/

#include "beauty.h"
#include "locus.h"

#ifdef REALPED
short detail=0;
char flags[128] = {"SELXY"};
#else
extern char flags[];
#endif

#define find(A) (pindex(flags,A))

FILE *fq;

extern double cut();
extern Point ccenter();

clust * cls;
clustP pcls;	/* pointer to first available loc in cls[] */

Point * row;	/* where a copy of all coordinates is stored */
short * offrow;	/* used to go from row[] back to objects */
short * nrow;	/* number of points in same contour on row[] */

sides * stt;	/* side list. used by everything */
sideP * stp;	/* pointers to stp for processing by cluster1() */
sideP * cmem;	/* store members of clusters */
sideP ** lp;	/* preliminary cluster pointers */
short * lpstat;	/* status of lp pointers */

sideP *pmem;

sides * srow;	/* make point list look like sides */

eqnS * eq;	/* side equations */
short neq, oeq;

/*	preferred slope values	*/
#define NEAR_G 0.1
short GN = 5;
double gslope[6] = { 0, QPI, HPI, TPI, PI };

short epsilon;

/*
	major routine called by other parts of program
	from main() in fped
	from hwork() in ped
*/

formatt(n,oa)
	short n;
	objP oa[];
{
	register  i, j;
	register sideP st;
	register sideP *pp;
	register clustP clj;
	register Point * pt;
	clustP opcls, topcls;
	Point pc;
	short ktotal;
	short nsides;
	short k;
	double q;
	clustP cnx;

	rrecord("%d objects to be formatted",n);
#ifdef REALPED
	if( (fq = fopen("ped.sort","a")) == NULL) {
		rrecord("Cannot print ped.sort");
		fq = stderr;
	}
	else setbuf(fq,NULL);
#else
	fq = stderr;
#endif

	fprintf(fq,"\nNEW RUN\n");
	set_up(n,oa);
	ktotal = mk_row(n,oa);	/* Build `row' and forget about objects	*/
	if(!ktotal) goto telos;
	nsides = mk_sides(n);	/*	Create array sides[] from row[]	*/
	rrecord("%d sides found",nsides);	
	if(nsides<1) goto telos;

	neq = 0;	/* start with zero eqs */
	/*
	*	cluster sides according to various ways
	*/

	pcls = cls;
	pmem = cmem;

	for(j=0,st=stt,pp=stp; j<nsides; j++) *pp++ = st++;

	/* By slope */

	for(j=0,st=stt; j<nsides; j++,st++) st->t = st->s;
	cluster1(nsides,stp,1);

	/*	fix slopes to good values	*/
	for(clj=cls; clj<pcls; clj++){
		for(k=0;k<GN;k++){
			if( fabs(clj->val-gslope[k]) < NEAR_G){
 				clj->val = gslope[k];
				if(clj->n==1){ /*make eqs right away*/
					if(detail) rrecord("%d ->eq",clj-cls);
					st = *(clj->mem);
					st->s = clj->val;
					eq[neq].s1 = eq[neq].s2 = st;
					eq[neq++].u = SLOPE;
				}
				break;
			}
		}
	}

	if(detail) list(cls,pcls,stt);

	/*	make slope eqs	*/
	k = 0;
	cnx = cls;
	for(clj=cls;clj<pcls;clj++){
		/* look for right angles	*/
		q = clj->val + HPI;
		if(cnx <= clj) cnx = clj+1;
		while(cnx<pcls){
			if(fabs(cnx->val-q) < DELTA){
				if(cnx->n<2) {
					st = *(cnx->mem);
					st->s = q;
					eq[neq].s1 = eq[neq].s2 = st;
					eq[neq++].u = SLOPE;
				}
				if(clj->n<2){
					eq[neq].s1 = eq[neq].s2 = *(clj->mem);
					eq[neq++].u = SLOPE;
				}
				break;
			}
			cnx++;
		}

		if(clj->n<2) continue;
		k++;
		for(i=0;i<clj->n;i++) {
			st = *(clj->mem+i);
			st->s = clj->val;
			eq[neq].s1 = st;
			eq[neq].s2 = st;
			eq[neq++].u = SLOPE;
		}
	}

	rrecord("%d clusters from slope, %d eqs",k,neq);

	epsilon = smalls+0.5;
	if(epsilon<5) epsilon = 5;

	if(find('E')){	/*	make extension equations	*/
		oeq = neq;
		opcls = pcls;
		k = 0;
		for(clj=cls; clj<opcls; clj++) {
			if(clj->n<2) continue;
			pc = ccenter(clj);
			for(j=0;j<clj->n;j++){
				st = *(clj->mem+j);
				st->t = cut(st,pc,stt);
			}
			topcls = pcls;
			cluster1(clj->n,clj->mem,epsilon);
			k += add_eq(topcls,pcls,EXTENSION);
		}

		if(detail) list(opcls,pcls,stt);
		rrecord("%d clusters form exten., %d eqs",k,neq-oeq);
	}

	if(find('L')) {	/* find length clusters */
		for(st=stt,j=0; j<nsides; j++, st++) st->t = st->l;
		create(nsides,epsilon,EQUAL,stp,stt,"lengths");
	}

	/*find point clusters */
	for(i=0,st=srow,pt=row; i<ktotal; i++,st++,pt++) st->p1 = st->p2 = pt;
	for(i=0,st=srow,pp=stp; i<ktotal; i++) *pp++ = st++;
	if(find('X')){
		for(i=0,st=srow;i<ktotal;i++,st++) st->t = (st->p1)->x;
		create(ktotal,epsilon,SAMEX,stp,srow,"x coinc");
	}
	if(find('Y')){
		for(i=0,st=srow;i<ktotal;i++,st++) st->t = (st->p1)->y;
		create(ktotal,epsilon,SAMEY,stp,srow,"y coinc");
	}

	varinit (ktotal);
	solve (eq, neq);

	spill_row(ktotal,n,oa);

	telos: clean_up();

#ifdef REALPED
	fclose(fq);
#endif
}

/*	Make cluster and equations	*/

create(n,tolerence,type,sbase,base,label)
	sideP * sbase;
	sideP base;
	char * label;
{
	register i;

	pcls  = cls;
	pmem = cmem;
	oeq = neq;

	cluster1(n,sbase,tolerence);
	if(detail) list(cls,pcls,base);
	i = add_eq(cls,pcls,type);
	rrecord("%d clusters from %s., %d eqs",i,label,neq-oeq);
}

llcomp(p1,p2)
	sideP *p1, *p2;
{
	register sideP e1, e2;
	e1 = *p1;
	e2 = *p2;
	if(e1->t < e2->t) return(-1);
	else if(e1->t > e2->t) return(1);
	else return(0);
}


cluster1(n,v,tol)
	sideP v[];
{
	register sideP *vp, *vq;
	register sideP svp, svq, ovp;
	register i, j;
	double d_span, d_adj;
	double t, s, v1, v2;
	short pc;

	qsort(v,n,sizeof(sideP),llcomp);

#ifdef RISK
	for(i=0,vp=v;i<n;i++,vp++){
		svp = *vp;
		fprintf(fq,"cl: %d %g,%g(%d) %g,%g(%d) s=%g\n",svp-stt,
		svp->p1->x, svp->p1->y, svp->p1-row,
		svp->p2->x, svp->p2->y, svp->p2-row,
		svp->t);
	}
#endif

	d_span = tol*DELTA; 
	d_adj = tol*DELTA1;
	if(detail)
	fprintf(fq,"clust tol: span = %g, adj=%g\n",d_span,d_adj);

	vq = vp = v;
	svp = *vp;	svq = *vq;
	t = svq->t;
	vp++;
	pc = 0;
	lp[pc++] = v;

	for(i=1; i<n; i++,vp++){
		ovp = svp;	svp = *vp;
		if( fabs(svp->t-t)<d_span && fabs(svp->t-ovp->t)<d_adj ) continue;
		lp[pc++] = vp;
		vq = vp;	svq = *vq;
		t = svq->t;
	}
	lp[pc++] = vp;
	for(i=0;i<pc;i++) lpstat[i] = 1;

	if(detail)
	for(i=0;i<pc;i++) fprintf(fq,"<%d,%d>",lp[i]-v,lpstat[i]);
	if(detail) fprintf(fq," lp\n");

	for(i=1;i<(pc-1);i++){
		if(!lpstat[i]) continue;
		if( !(lp[i]>(lp[i-1]+1) && lp[i]<(lp[i+1]-1)) ) continue;
		v2 = (*(lp[i+1]-1))->t - (*lp[i-1])->t;
		v1 = (*lp[i])->t - (*(lp[i]-1))->t;
		if(!v2) error(1,"zero range around cluster %d",i);
		else t = v1/v2;
		if(t<0.5) {	/*disolve clusters*/
			lpstat[i] = lpstat[i+1] = 0;
		}		
	}

	for(i=1;i<(pc-1);i++){
		if(!lpstat[i]) continue;
		while(lp[i]>(lp[i-1]+1) && lp[i]<(lp[i+1]-1)){
			v1 = ( (*lp[i-1])->t + (*(lp[i]-1))->t )/2;
			v2 = ( (*lp[i])->t + (*(lp[i+1]-1))->t )/2;
			t = (*(lp[i]-1))->t;
			if(fabs(t-v2)<fabs(t-v1)) lp[i]--;
			else break;
		}
	}
	if(detail) 
	for(i=0;i<pc;i++) fprintf(fq,"<%d,%d>",lp[i]-v,lpstat[i]);
	if(detail) fprintf(fq," lp\n");

	for(i=1; i<pc; i++){
		if(lp[i-1]>=lp[i]) continue;
		if(!lpstat[i]) {
			for(vp=lp[i-1];vp<lp[i];vp++) single(*vp);
			continue;
		}
		if(lp[i]-lp[i-1]<2) single(*(lp[i]-1)); /*one point cluster*/
		else {
			vq = lp[i-1];	svq = *vq;
			pcls->n = lp[i]-lp[i-1];
			pcls->mem = pmem;
			pmem += pcls->n;
			for(j=0;j<pcls->n;j++) *(pcls->mem+j) = *(vq+j); 
			pcls->val = ( (*lp[i-1])->t + (*(lp[i]-1))->t )/2;
			pcls++;
		}
	}
}

single(pp)
	sideP pp;
{
	pcls->n = 1;	pcls->mem = pmem;	pmem += 1;
	*(pcls->mem) = pp;
	pcls->val = pp->t;
	pcls++;
}

add_eq(C1,C2,kind)
	clustP C1, C2;
{
	register i, k;
	register clustP cli;
	register sideP st0;

	k = 0;
	for(cli=C1;cli<C2;cli++){
		if(cli->n<2) continue;
		k++;
		st0 = *(cli->mem);
		for(i=1; i<cli->n; i++) {
			eq[neq].s1 = st0;
			eq[neq].s2 = *(cli->mem+i);
			eq[neq++].u = kind;
		}
	}
	return(k);
}
