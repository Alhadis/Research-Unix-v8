/*
   format of color map for experimental anti-aliasing:
  0- 62   brightest rainbow
 64-126   3/4 bright
128-190   1/2 bright
192-254   1/4 bright
    255   black
*/

#include <stdio.h>
#define RED   0x4
#define GREEN 0x2
#define BLUE  0x1
#define SAVCOLOR    mapr[i]=255.499*r;mapg[i]=255.499*g;mapb[i]=255.499*b;

int
cvmap ( map, maxi, mapr,mapg,mapb )  /* load color map, return black index*/
  char *map;   /* string describing kind of map */
  int maxi;
  int mapr[],mapg[],mapb[]; /* colors, 0 through maxi */
{
  int i, j;
  int max1, max2;
  double r, g, b;
  double dm = maxi;
  double sqrt();
  int antial = 0;
  FILE *mapf;
  if(maxi<0) error("maxi must be >= 0");
  if(maxi>=254) error("maxi must be < 254");
  switch(*map){
    case 'a': /* experimental -- for anti-aliasing */
      antial++;
      maxi=62;
      dm = maxi;
      for(i=0; i<=maxi; i++){
        rainbow(1-i/dm,1.,1.,&r,&g,&b);
        SAVCOLOR;
      }
      break;
    case 'f': /* input from file */
      mapf = fopen(map+1,"r");
      if( mapf==NULL ) error("can't open %s",map+1);
      for(i=0; i<=maxi; i++){
        if(fscanf(mapf,"%d %d %d",&mapr[i],&mapg[i],&mapb[i])!=3)
          error("trouble reading color map %d",i);
      }
      fclose(mapf);
      break;
    case 'r': /* red v. blue */
      for(i=0; i<=maxi; i++){
        r=sqrt(i/dm); g=sqrt(0.); b=sqrt(1-i/dm);
        brit(&r,&g,&b);
        SAVCOLOR;
      }
      break;
    case NULL:
    case 's': /* blue-to-red rainbow */
      for(i=0; i<=maxi; i++){
        rainbow(1-i/dm,1.,1.,&r,&g,&b);
        SAVCOLOR;
      }
      break;
    case 'g':   /* normal map, corrected for gamma */
      for(i=0; i<=maxi; i++){
        r=sqrt(i/dm); g=sqrt(i/dm); b=sqrt(i/dm);
        SAVCOLOR;
      }
      break;
    default:
    err:
      fprintf(stderr,"unrecognized color map %s\n",map);
  }
  for(i=0; i<=maxi; i++){
    cvput(0x1C);cvput(i);
      cvput(mapr[i]);cvput(mapg[i]);cvput(mapb[i]);
  }
  if(antial){
    for(j=1;j<4;j++){
      for(i=0; i<=maxi; i++){
        cvput(0x1C);cvput(i+j*(maxi+1));
          r=mapr[i]/254.5; r=r*r; r=(4-j)*r/4; cvput((int)(254.999*sqrt(r)));
          r=mapg[i]/254.5; r=r*r; r=(4-j)*r/4; cvput((int)(254.999*sqrt(r)));
          r=mapb[i]/254.5; r=r*r; r=(4-j)*r/4; cvput((int)(254.999*sqrt(r)));
      }
    }
  }
  cvput(0x1B); cvput(255); cvput(0);    /* LUTA */
  return(255);
}

brit(r,g,b)
  double *r, *g, *b;
{
  /*  convert from r+g+b=1 to max(r,g,b)=1 */
  double t;
  t = (*r>*b)?*r:*b;
  if(*g>t) t = *g;
  *r /= t;
  *g /= t;
  *b /= t;
}



/**** taken from /n/research/netlib/misc/rainbow *****/
/*   rainbow(h, s, v, r, g, b)
     double h, s, v, *r, *g, *b;

 This routine computes colors suitable for use in color level plots.
 Typically s=v=1 and h varies from 0 (red) to 1 (blue) in
 equally spaced steps.  (h>1 gives magenta.)
 To convert for frame buffer, use   R = (int)(255.999*sqrt(*r))  etc.
 complaints =>  Eric Grosse   research!ehg    201-582-5828
*/

#include <math.h>
double huextab[] = {
  .0000,.0156,.0313,.0469,.0625,.0781,.0938,.1094,.1250,.1406,
  .1563,.1719,.1875,.2031,.2188,.2344,.2500,.2656,.2813,.2969,
  .3125,.3281,.3438,.3594,.3750,.3906,.4063,.4219,.4375,.4531,
  .4688,.4844,.5000,.5156,.5313,.5469,.5625,.5781,.5938,.6094,
  .6250,.6406,.6563,.6719,.6875,.7031,.7188,.7344,.7500,.7656,
  .7813,.7969,.8125,.8281,.8438,.8594,.8750,.8906,.9063,.9219,
  .9375,.9531,.9688,.9844,1.  };
double huettab[] = {
  .0000,.0052,.0105,.0160,.0218,.0277,.0339,.0404,.0471,.0542,
  .0616,.0693,.0775,.0862,.0954,.1052,.1157,.1269,.1390,.1519,
  .1660,.1824,.1974,.2115,.2248,.2374,.2495,.2609,.2719,.2824,
  .2926,.3023,.3118,.3210,.3300,.3456,.3600,.3761,.3943,.4153,
  .4399,.4688,.5048,.5211,.5359,.5494,.5617,.5729,.5832,.5925,
  .6010,.6088,.6160,.6225,.6285,.6340,.6390,.6436,.6478,.6516,
  .6552,.6584,.6614,.6642,.6667 };
  /* computed from the FMC-1 color difference formula */
  /* Hitachi monitor, max(r,g,b)=1, n=65,  7 Jan 1985 */

rainbow(h, s, v, r, g, b)
double h, s, v, *r, *g, *b;
{
  double *x = huextab;
  double *t = huettab;
  while( (x[1]<=h) && (x[1]<1.) ){ ++x; ++t; }
  h = t[0] + (t[1]-t[0])*(h-x[0])/(x[1]-x[0]);
  dhsv2rgb(h,s,v,r,g,b);
}

dhsv2rgb(h, s, v, r, g, b)    /*...hexcone model...*/
double h, s, v, *r, *g, *b;    /* all variables in range [0,1] */
{
  int i;
  double f, m, n, k;
  h *= 6;
  i = (int)floor(h);
  f = h-i;
  m = (1-s);
  n = (1-s*f);
  k = (1-(s*(1-f)));
  switch(i){
    case 0: *r=1; *g=k; *b=m; break;
    case 1: *r=n; *g=1; *b=m; break;
    case 2: *r=m; *g=1; *b=k; break;
    case 3: *r=m; *g=n; *b=1; break;
    case 4: *r=k; *g=m; *b=1; break;
    case 5: *r=1; *g=m; *b=n; break;
    default: error("bad i: %f %d",h,i);
  }
  f = *r;
  if( f < *g ) f = *g;
  if( f < *b ) f = *b;
  f = v / f;
  *r *= f;
  *g *= f;
  *b *= f;
}
