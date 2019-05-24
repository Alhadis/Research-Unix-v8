#include "map.h"
#include <stdio.h>
#include <iplot.h>
#define NTRACK 10
#define NFILE 30
#define HALFWIDTH 8192
#define SCALERATIO 10
#define RESOL 2.
#define TWO_THRD 0.66666666666666667

static int (*projection)();
double atof(), floor(), ceil(), fmin(), fmax();
extern char *getenv(), *malloc();
float reduce();
short getshort();

#define map(x) int (*x())()

static char *mapdir = "/usr/dict";
static char *file[NFILE+1] = {
	"world",
	0
};
map(azequidistant);
map(mercator);
map(sp_mercator);
map(cylindrical);
map(rectangular);
map(orthographic);
map(sinusoidal);
map(mollweide);
map(aitoff);
map(azequalarea);
map(stereographic);
map(laue);
map(gilbert);
map(gnomonic);
map(perspective);
map(cylequalarea);
map(conic);
map(polyconic);
map(bonne);
map(lambert);
map(albers);
map(sp_albers);
map(mecca);
map(homing);
map(guyou);
map(square);
map(tetra);
map(hex);
map(elliptic);
map(bicentric);
int Xguyou();
int Xsquare();
int Xtetra();

int nocut();
int picut();
int guycut();
int tetracut();
int hexcut();


static struct {
	char *name;
	int (*(*prog)())();
	int npar;
	int (*cut)();
	int poles;/*1 S pole is a line, 2 N pole is, 3 both*/
	int spheroid;	/* poles must be at 90 deg */
} index[] = {
	{"azequidistant", azequidistant, 0, nocut, 1, 0},
	{"mercator", mercator, 0, picut, 0, 0},
	{"sp_mercator", sp_mercator, 0, picut, 0, 1},
	{"cylindrical", cylindrical, 0, picut, 0, 0},
	{"rectangular", rectangular, 0, picut, 3, 0},
	{"orthographic", orthographic, 0, nocut, 0, 0},
	{"sinusoidal", sinusoidal, 0, picut, 0, 0},
	{"mollweide", mollweide, 0, picut, 0, 0},
	{"aitoff", aitoff, 0, picut, 0, 0},
	{"azequalarea", azequalarea, 0, nocut, 1, 0},
	{"stereographic", stereographic, 0, nocut, 0, 0},
	{"laue", laue, 0, nocut, 0, 0},
	{"gilbert",gilbert,0,picut,0, 0},
	{"gnomonic", gnomonic, 0, nocut, 0, 0},
	{"perspective", perspective, 1, nocut, 0, 0},
	{"cylequalarea", cylequalarea, 1, picut, 3, 0},
	{"conic", conic, 1, picut, 0, 0},
	{"polyconic", polyconic, 0, picut, 0, 0},
	{"bonne", bonne, 1, picut, 0, 0},
	{"lambert", lambert, 2, picut, 0, 0},
	{"albers", albers, 2, picut, 3, 0},
	{"sp_albers", sp_albers, 2, picut, 3, 1},
	{"mecca", mecca, 1, picut, 0, 0},
	{"homing", homing, 1, picut, 0, 0},
	{"guyou", guyou, 0, guycut, 0, 0},
	{"square", square, 0, picut, 0, 0},
	{"tetra", tetra, 0, tetracut, 0, 0},
	{"hex", hex, 0, hexcut, 0, 0},
	{"elliptic", elliptic, 1, nocut, 0, 0},
	{"bicentric", bicentric, 1, nocut, 0, 0},
	0
};

static int (*cut)();
static int poles;
static float orientation[3] = { 90., 0., 0. };
static oriented;
static int delta = 1;
static float limits[4] = {
	-90., 90., -180., 180.
};
static int limcase;
static float rlimits[4];
static float lolat, hilat, lolon, hilon;
static float window[4] = {
	-90., 90., -180., 180.
};
static float rwindow[4];
static float params[2];
static float xmin = 100.;
static float xmax = -100.;
static float ymin = 100.;
static float ymax = -100.;
static float xcent, ycent;
float xrange, yrange;
static int left = -HALFWIDTH;
static int right = HALFWIDTH;
static int bottom = -HALFWIDTH;
static int top = HALFWIDTH;
static int bflag = 1;
static int sflag = 0;
static int rflag = 0;
static int mflag = 0;
static float position[3];
static float center[2] = {0., 0.};
static float grid[3] = { 10., 10., RESOL };
static float dlat, dlon;
static float scaling;
static struct track {
	int tracktyp;
	char *tracknam;
} track[NTRACK];
static int ntrack;

main(argc,argv)
char **argv;
{
	int i,k;
	char *s, *t;
	float x, y, lat, lon;
	float dd;
	if(sizeof(short)!=2)
		abort();	/* getshort() won't work */
	s = getenv("MAP");
	if(s)
		file[0] = s;
	s = getenv("MAPDIR");
	if(s)
		mapdir = s;
	if(argc<=1) 
		error("usage: map projection params options");
	for(k=0;index[k].name;k++) {
		s = index[k].name;
		t = argv[1];
		while(*s == *t){
			if(*s==0) goto found;
			s++;
			t++;
		}
	}
	fprintf(stderr,"projections:\n");
	for(i=0;index[i].name;i++) 
		fprintf(stderr,"%s\n",index[i].name);
	exit(1);
found:
	argv += 2;
	argc -= 2;
	cut = index[k].cut;
	for(i=0;i<index[k].npar;i++) {
		if(i>=argc||option(argv[i])) {
			fprintf(stderr,"%s needs %d params\n",index[k].name,index[k].npar);
			exit(1);
		}
		params[i] = atof(argv[i]);
	}
	argv += i;
	argc -= i;
	while(argc>0&&option(argv[0])) {
		argc--;
		argv++;
		switch(argv[-1][1]) {
		case 'm':
			i = 0;
			if(!mflag) while(file[i]!=0)
					i++;
			for(i=0;i<NFILE&&argc>i&&!option(argv[i]);i++)
				file[i] = argv[i];
			file[i] = 0;
			mflag++;
			argc -= i;
			argv += i;
			break;
		case 'b':
			bflag = 0;
			break;
		case 'g':
			for(i=0;i<3&&argc>i&&!option(argv[i]);i++)
				grid[i] = atof(argv[i]);
			switch(i) {
			case 0:
				grid[0] = grid[1] = 0.;
				break;
			case 1:
				grid[1] = grid[0];
			}
			argc -= i;
			argv += i;
			break;
		case 't':
		case 'u':
			for(i=0;ntrack<NTRACK&&argc>i&&!option(argv[i]);i++) {
				track[ntrack].tracktyp = argv[-1][1];
				track[ntrack++].tracknam = argv[i];
			}
			argc -= i;
			argv +=i;
			break;
		case 'r':
			rflag++;
			break;
		case 's':
			sflag++;
			break;
		case 'o':
			for(i=0;i<3&&i<argc&&!option(argv[i]);i++)
				orientation[i] = atof(argv[i]);
			oriented++;
			argv += i;
			argc -= i;
			break;
		case 'l':
			for(i=0;i<argc&&i<4&&!option(argv[i]);i++)
				limits[i] = atof(argv[i]);
			argv += i;
			argc -= i;
			break;
		case 'd':
			if(argc>0&&!option(argv[0])) {
				delta = atoi(argv[0]);
				argv++;
				argc--;
			}
			break;
		case 'w':
			for(i=0;i<argc&&i<4&&!option(argv[i]);i++)
				window[i] = atof(argv[i]);
			argv += i;
			argc -= i;
			break;
		case 'c':
			for(i=0;i<2&&argc>i&&!option(argv[i]);i++) 
				center[i] = atof(argv[i]);
			argc -= i;
			argv += i;
			break;
		case 'p':
			for(i=0;i<3&&argc>i&&!option(argv[i]);i++)
				position[i] = atof(argv[i]);
			argc -= i;
			argv += i;
			if(i!=3||position[2]<=0) 
				error("incomplete positioning");
			break;
		}
	}
	if(argc>0)
		error("error in arguments");
	pathnames();
	radbds(limits,rlimits);
	limcase = limits[2]<-180.?0:
		  limits[3]>180.?2:
		  1;
	if(
		window[0]>=window[1]||
		window[2]>=window[3]||
		window[0]>90.||
		window[1]<-90.||
		window[2]>180.||
		window[3]<-180.)
		error("unreasonable window");
	radbds(window,rwindow);
	if(index[k].spheroid && fabs(orientation[0])!=90)
		error("can't tilt the spheroid");
	if(limits[2]>limits[3])
		limits[3] += 360;
	if(!oriented)
		orientation[2] = (limits[2]+limits[3])/2;
	orient(orientation[0],orientation[1],orientation[2]);
	projection = (*index[k].prog)(params[0],params[1]);
	if(projection == 0)
		error("unreasonable projection parameters");
	grid[0] = fabs(grid[0]);
	grid[1] = fabs(grid[1]);
	lolat = limits[0];
	hilat = limits[1];
	lolon = limits[2];
	hilon = limits[3];
	if(lolon>=hilon||lolat>=hilat||lolat<-90.||hilat>90.)
		error("unreasonable limits");
	dlat = fmin(hilat-lolat,window[1]-window[0])/16;
	dlon = fmin(hilon-lolon,window[3]-window[2])/32;
	dd = fmax(dlat,dlon);
	while(grid[2]>fmin(dlat,dlon)/2)
		grid[2] /= 2;
	for(lat=lolat;lat<hilat+dd-FUZZ;lat+=dd) {
		if(lat>hilat)
			lat = hilat;
		for(lon=lolon;lon<hilon+dd-FUZZ;lon+=dd) {
			if(lon>hilon)
				lon = hilon;
			if(normproj(lat,lon,&x,&y)<=0)
				continue;
			if(x<xmin) xmin = x;
			if(x>xmax) xmax = x;
			if(y<ymin) ymin = y;
			if(y>ymax) ymax = y;
		}
	}
	xrange = xmax - xmin;
	yrange = ymax - ymin;
	if(xrange<=0||yrange<=0)
		error("map seems to be empty");
	scaling = (2*HALFWIDTH)*0.9;
	if(position[2]!=0) {
		if(normproj(position[0]-.5,position[1],&xcent,&ycent)<=0||
		   normproj(position[0]+.5,position[1],&x,&y)<=0)
			error("unreasonable position");
		scaling /= (position[2]*hypot(x-xcent,y-ycent));
		if(normproj(position[0],position[1],&xcent,&ycent)<=0)
			error("unreasonable position");
	} else {
		scaling /= (xrange>yrange?xrange:yrange);
		xcent = (xmin+xmax)/2;
		ycent = (ymin+ymax)/2;
	}
	xcent -= center[0]/2;
	ycent -= center[1]/2;
	openpl();
	range(left,bottom,right,top);
	if(!sflag)
		erase();
	pen("dotted");
	if(grid[0]>0.)
		for(lat=ceil(lolat/grid[0])*grid[0];
		    lat<=hilat;lat+=grid[0]) 
			dogrid(lat,lat,lolon,hilon,0);
	if(grid[1]>0.)
		for(lon=ceil(lolon/grid[1])*grid[1];
		    lon<=hilon;lon+=grid[1]) 
			dogrid(lolat,hilat,lon,lon,0);
	pen("solid");
	if(bflag) {
		if(lolat>-90)
			dogrid(lolat+FUZZ,lolat+FUZZ,lolon,hilon,0);
		if(hilat<90)
			dogrid(hilat-FUZZ,hilat-FUZZ,lolon,hilon,0);
		if(hilon-lolon<360) {
			dogrid(lolat,hilat,lolon+FUZZ,lolon+FUZZ,0);
			dogrid(lolat,hilat,hilon-FUZZ,hilon-FUZZ,0);
		}
		if(poles&1)
			dogrid(window[0]+FUZZ,window[0]+FUZZ,window[2],window[3],1);
		if(poles&2)
			dogrid(window[1]-FUZZ,window[1]-FUZZ,window[2],window[3],1);
		if(window[3]-window[2]<360) {
			dogrid(window[0],window[1],window[2]+FUZZ,window[2]+FUZZ,1);
			dogrid(window[0],window[1],window[3]-FUZZ,window[3]-FUZZ,1);
		}
	}
	lolat = floor(limits[0]/10)*10;
	hilat = ceil(limits[1]/10)*10;
	lolon = floor(limits[2]/10)*10;
	hilon = ceil(limits[3]/10)*10;
	if(lolon>hilon)
		hilon += 360.;
	/*do tracks first so as not to lose the standard input*/
	for(i=0;i<ntrack;i++)
		satellite(&track[i]);
	pen("solid");
	for(i=0;file[i];i++)
		getdata(file[i]);

	move(right,bottom);
	closepl();
	return(0);
}


normproj(lat,lon,x,y)
float lat,lon;
float *x, *y;
{
	int i;
	struct place geog;
	latlon(lat,lon,&geog);
/*
	printp(&geog);
*/
	normalize(&geog);
	if(!inwindow(&geog))
		return(-1);
	i = (*projection)(&geog,x,y);
	if(rflag) 
		*x = -*x;
/*
	printp(&geog);
	fprintf(stderr,"%d %.3f %.3f\n",i,*x,*y);
*/
	return(i);
}

inwindow(geog)
struct place *geog;
{
	if(geog->nlat.l<rwindow[0]||
	   geog->nlat.l>rwindow[1]||
	   geog->wlon.l<rwindow[2]||
	   geog->wlon.l>rwindow[3])
		return(0);
	else return(1);
}

inlimits(g)
struct place *g;
{
	if(rlimits[0]>g->nlat.l||
	   rlimits[1]<g->nlat.l)
		return(0);
	switch(limcase) {
	case 0:
		if(rlimits[2]+TWOPI>g->wlon.l&&
		   rlimits[3]<g->wlon.l)
			return(0);
		break;
	case 1:
		if(rlimits[2]>g->wlon.l||
		   rlimits[3]<g->wlon.l)
			return(0);
		break;
	case 2:
		if(rlimits[2]>g->wlon.l&&
		   rlimits[3]-TWOPI<g->wlon.l)
			return(0);
		break;
	}
	return(1);
}

option(s) 
char *s;
{

	if(s[0]=='-' && (s[1]<'0'||s[1]>'9'))
		return(s[1]!='.'&&s[1]!=0);
	else
		return(0);
}


long patch[18][36];

getdata(mapfile)
char *mapfile;
{
	char indexfile[32];
	int cx,cy;
	int kx,ky;
	int k;
	long b;
	long *p;
	int ip, jp;
	int n;
	struct place g;
	int i, j;
	float lat, lon;
	int conn;
	FILE *ifile, *xfile;

	for(i=0;indexfile[i]=mapfile[i];i++);
	indexfile[i++] = '.';
	indexfile[i++] = 'x';
	indexfile[i] = 0;
	xfile = fopen(indexfile,"r");
	if(xfile==NULL)
		filerror("can't find map index", indexfile);
	for(i=0,p=patch[0];i<18*36;i++,p++)
		*p = 1;
	while(!feof(xfile) && fscanf(xfile,"%d%d%ld",&i,&j,&b)==3)
		patch[i+9][j+18] = b;
	fclose(xfile);
	ifile = fopen(mapfile,"r");
	if(ifile==NULL)
		filerror("can't find map data", mapfile);
	for(lat=lolat;lat<hilat;lat+=10.)
		for(lon=lolon;lon<hilon;lon+=10.) {
			if(!seeable(lat,lon))
				continue;
			i = pnorm(lat);
			j = pnorm(lon);
			if((b=patch[i+9][j+18])&1)
				continue;
			fseek(ifile,b,0);
			while((ip=getc(ifile))>=0&&(jp=getc(ifile))>=0){
				if(ip!=(i&0377)||jp!=(j&0377))
					break;
				n = getshort(ifile);
				conn = 0;
				if(n > 0) {	/* absolute coordinates */
					for(k=0;k<n;k++){
						kx = SCALERATIO*getshort(ifile);
						ky =  SCALERATIO*getshort(ifile);
						if(k%delta!=0&&k!=n-1)
							continue;
						conv(kx,&g.nlat);
						conv(ky,&g.wlon);
						conn = plotpt(&g,conn);
					}
				} else {	/* differential, scaled by SCALERATI0 */
					n = -n;
					kx = SCALERATIO*getshort(ifile);
					ky = SCALERATIO*getshort(ifile);
					for(k=0; k<n; k++) {
						kx += (char)getc(ifile);
						ky += (char)getc(ifile);
						if(k%delta!=0&&k!=n-1)
							continue;
						conv(kx,&g.nlat);
						conv(ky,&g.wlon);
						conn = plotpt(&g,conn);
					}
				}
				if(k==1) {
					conv(kx,&g.nlat);
					conv(ky,&g.wlon);
					conn = plotpt(&g,conn);
				}
			}
		}
	fclose(ifile);
}

seeable(lat0,lon0)
float lat0,lon0;
{
	float x, y;
	float lat, lon;
	for(lat=lat0;lat<=lat0+10;lat+=grid[2])
		for(lon=lon0;lon<=lon0+10;lon+=grid[2])
			if(normproj(lat,lon,&x,&y)>0)
				return(1);
	return(0);
}

char lbl[50];
satellite(t)
struct track *t;
{
	int visible, skip;
	register conn;
	float lat,lon;
	struct place place;
	static FILE *ifile = stdin;
	register char *s;
	if(t->tracknam[0]!='-'||t->tracknam[1]!=0) {
		fclose(ifile);
		if(fopen(t->tracknam,"r")==NULL)
			filerror("can't find track", t->tracknam);
	}
	pen(t->tracktyp=='t'?"dotdash":"solid");
	visible = 0;
	for(;;) {
		conn = 0;
		while(!feof(ifile) && fscanf(ifile,"%f%f",&lat,&lon)==2){
			latlon(lat,lon,&place);
			conn = plotpt(&place,conn);
			visible = conn;
		}
		skip = 1;
		s = lbl;
		for(;;) {
			if(feof(ifile) || fscanf(ifile,"%c",s) == EOF)
				return;
			if(*s=='\n')
				break;
			if(skip) switch(*s) {
				case '"':
					skip = 0;
					continue;
				case ' ':
				case '\t':
					continue;
				default:
					skip = 0;
				}
			s++;
		}
		*s = 0;
		if(visible)
			text(lbl);
	}
}

pnorm(x)
float x;
{
	int i;
	i = x/10.;
	i %= 36;
	if(i>=18) return(i-36);
	if(i<-18) return(i+36);
	return(i);
}

conv(k,g)
struct coord *g;
{
	g->l = (0.0001/SCALERATIO)*k;
	sincos(g);
}

error(s)
char *s;
{
	closepl();
	fprintf(stderr,"map: \r\n%s\n",s);
	exit(1);
}

filerror(s,f)
char *s, *f;
{
	closepl();
	fprintf(stderr,"\r\n%s %s\n",s,f);
	exit(1);
}

cpoint(xi,yi,conn)
{
	if(xi<left||xi>=right)
		return(0);
	if(yi<bottom||yi>=top)
		return(0);
	if(!conn)
		move(xi,yi);
	else
		vec(xi,yi);
	return(1);
}


struct place oldg;
plotpt(g,conn)
struct place *g;
{
	if(!inlimits(g))
		return(0);
	normalize(g);
	return(plotwin(g,conn));
}

plotwin(g,conn)
struct place *g;
{
	int kx,ky;
	int ret;
	float cutlon;
	if(!inwindow(g))
		return(0);
	switch((*cut)(g,&oldg,&cutlon)) {
	case 2:
		if(conn) {
			ret = duple(g,cutlon)|duple(g,cutlon);
			copyplace(g,&oldg);
			return(ret);
		}
	case 0:
		conn = 0;
	case 1:
		copyplace(g,&oldg);
		if(doproj(g,&kx,&ky)<=0)
			return(0);
		return(cpoint(kx,ky,conn));
	}
	/*NOTREACHED*/
}

doproj(g,kx,ky)
struct place *g;
int *kx,*ky;
{
	float x,y;
/*fprintf(stderr,"dopr1 %f %f \n",g->nlat.l,g->wlon.l);*/
	if((*projection)(g,&x,&y)<=0)
		return(0);
	if(rflag)
		x = -x;
/*fprintf(stderr,"dopr2 %f %f\n",x,y);*/
	*kx = (x-xcent)*scaling;
	*ky = (y-ycent)*scaling;
	return(1);
}

duple(g,cutlon)
struct place *g;
float cutlon;
{
	int kx,ky;
	int okx,oky;
	struct place ig;
	revlon(g,cutlon);
	revlon(&oldg,cutlon);
	copyplace(g,&ig);
	invert(&ig);
	if(!inlimits(&ig))
		return(0);
	if(doproj(g,&kx,&ky)<=0 || doproj(&oldg,&okx,&oky)<=0)
		return(0);
	line(okx,oky,kx,ky);
	return(1);
}

revlon(g,cutlon)
struct place *g;
float cutlon;
{
	g->wlon.l = reduce(cutlon-reduce(g->wlon.l-cutlon));
	sincos(&g->wlon);
}


/*	recognize problems of cuts
 *	move a point across cut to side of its predecessor
 *	if its very close to the cut
 *	return(0) if cut interrupts the line
 *	return(1) if line is to be drawn normally
 *	return(2) if line is so close to cut as to
 *	be properly drawn on both sheets
*/

picut(g,og,cutlon)
struct place *g,*og;
float *cutlon;
{
	*cutlon = PI;
	return(ckcut(g,og,PI));
}

nocut(g,og,cutlon)
struct place *g,*og;
float *cutlon;
{
	return(1);
}

ckcut(g1,g2,lon)
struct place *g1, *g2;
float lon;
{
	float d1, d2;
	float f1, f2;
	int kx,ky;
	extern float diddle();
	d1 = reduce(g1->wlon.l -lon);
	d2 = reduce(g2->wlon.l -lon);
	if((f1=fabs(d1))<FUZZ)
		d1 = diddle(g1,lon,d2);
	if((f2=fabs(d2))<FUZZ) {
		d2 = diddle(g2,lon,d1);
		if(doproj(g2,&kx,&ky)>0)
			move(kx,ky);
	}
	if(f1<FUZZ&&f2<FUZZ)
		return(2);
	if(f1>PI*TWO_THRD||f2>PI*TWO_THRD)
		return(1);
	return(d1*d2>=0);
}

float diddle(g,lon,d)
struct place *g;
float d,lon;
{
	float d1;
	d1 = FUZZ/2;
	if(d<0)
		d1 = -d1;
	g->wlon.l = reduce(lon+d1);
	sincos(&g->wlon);
	return(d1);
}

float reduce(lon)
float lon;
{
	if(lon>PI)
		lon -= 2*PI;
	else if(lon<-PI)
		lon += 2*PI;
	return(lon);
}


float tetrapt = 35.26438968;	/* atan(1/sqrt(2)) */

dogrid(lat0,lat1,lon0,lon1,inv)
float lat0,lat1,lon0,lon1;
{
	float slat,slon,dd;
	register int conn;
	slat = slon = 0;
	if(lat1>lat0)
		slat = fmin(grid[2],dlat);
	else
		slon = fmin(grid[2],dlon);;
	conn = 0;
	while(lat0<=lat1&&lon0<=lon1) {
		conn = gridpt(lat0,lon0,conn,inv);
		if(projection==Xguyou&&slat>0) {
			if(lat0<-45&&lat0+slat>-45)
				conn = gridpt(-45.,lon0,conn,inv);
			else if(lat0<45&&lat0+slat>45)
				conn = gridpt(45.,lon0,conn,inv);
		} else if(projection==Xtetra&&slat>0) {
			if(lat0<-tetrapt&&lat0+slat>-tetrapt) {
				gridpt(-tetrapt-.001,lon0,conn,inv);
				conn = gridpt(-tetrapt+.001,lon0,0,inv);
			}
			else if(lat0<tetrapt&&lat0+slat>tetrapt) {
				gridpt(tetrapt-.001,lon0,conn,inv);
				conn = gridpt(tetrapt+.001,lon0,0,inv);
			}
		}
		lat0 += slat;
		lon0 += slon;
	}
	gridpt(lat1,lon1,conn,inv);
}

gridpt(lat,lon,conn,inv)
float lat,lon;
{
	struct place g;
	struct place p;
/*fprintf(stderr,"%f %f\n",lat,lon);*/
	latlon(lat,lon,&g);
	if(inv) {
		copyplace(&g,&p);
		invert(&p);
		if(!inlimits(&p))
			return(0);
		return(plotpt(&p,conn));
	}
	return(plotpt(&g,conn));
}

radbds(w,rw)
float *w,*rw;
{
	register i;
	for(i=0;i<4;i++)
		rw[i] = w[i]*RAD;
	rw[0] -= FUZZ;
	rw[1] += FUZZ;
	rw[2] -= FUZZ;
	rw[3] += FUZZ;
}

short
getshort(f)
FILE *f;
{
	register c;
	c = getc(f);
	return(c | getc(f)<<8);
}

double
fmin(x,y)
double x,y;
{
	return(x<y?x:y);
}

double
fmax(x,y)
double x,y;
{
	return(x>y?x:y);
}

pathnames()
{
	int i;
	char *t;
	for(i=0; i<NFILE && file[i]; i++) {
		if(*file[i]=='/' || access(file[i],04)==0)
			continue;
		t = malloc(strlen(file[i])+strlen(mapdir)+2);
		strcpy(t,mapdir);
		strcat(t,"/");
		strcat(t,file[i]);
		file[i] = t;
	}
}
