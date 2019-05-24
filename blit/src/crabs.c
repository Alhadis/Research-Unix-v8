
#include <jerq.h>

#define CRABNUM 30
#define SLEEPTIME 5

#undef	bitblt
#undef	texture
#define bitblt(s, r, d, p, c)	(*((void(*)())0x430d6))(s, r, d, p, c)
#define texture(s, r, d, c)	(*((void(*)())0x4372c))(s, r, d, c)

       Point *crab, *velocity, rounded;
        int crabnum, sleeptime;
        Bitmap *test;

Word DownCrabs[] = {
 0x4244,0x0242,0x0242,0x4240
,0xB935,0xAC9D,0xAC9D,0xB935
,0x6E5C,0x3A76,0x3A76,0x6E5C
,0x3A76,0x6E5C,0x6E5C,0x3A76
,0xAC9D,0xB935,0xB935,0xAC9D
,0x8101,0x8081,0x8081,0x8101
,0xEECD,0xAB67,0xAB67,0xEECD
,0x3276,0x6654,0x6654,0x3276
};

Word UpCrabs[] = {
 0x6E4C,0x2A66,0x2A66,0x6E4C
,0xB377,0xE6D5,0xE6D5,0xB377
,0x8081,0x8101,0x8101,0x8081
,0xB935,0xAC9D,0xAC9D,0xB935
,0x6E5C,0x3A76,0x3A76,0x6E5C
,0x3A76,0x6E5C,0x6E5C,0x3A76
,0xAC9D,0xB935,0xB935,0xAC9D
,0x0242,0x4240,0x4240,0x0242
};

Word LeftCrabs[] = {
 0x6250,0x3272,0x3272,0x6258
,0x8945,0xCCCD,0xCCCD,0x8945
,0xCEDC,0x9A56,0x9A56,0xCEDC
,0x9A16,0x8E9C,0x8E9C,0x9A16
,0x4E5C,0x1A56,0x1A56,0x4E5C
,0x9A56,0xCEDC,0xCEDC,0x9A56
,0xCCCD,0x8945,0x8945,0xCCCD
,0x3272,0x6250,0x6250,0x3272
};

Word RightCrabs[] = {
 0x4E4C,0x0A46,0x0A46,0x4E4C
,0xB333,0xA291,0xA291,0xB333
,0x6A59,0x3B73,0x3B73,0x6A59
,0x3A72,0x6A58,0x6A58,0x3A72
,0x6859,0x3971,0x3971,0x6859
,0x3B73,0x6A59,0x6A59,0x3B73
,0xA291,0xB333,0xB333,0xA291
,0x0A46,0x4E4C,0x4E4C,0x0A46
};

Bitmap downcrabmap  = {DownCrabs, 4,{{0,0},{64,8}}};
Bitmap upcrabmap    = {UpCrabs,   4,{{0,0},{64,8}}};
Bitmap leftcrabmap  = {LeftCrabs, 4,{{0,0},{64,8}}};
Bitmap rightcrabmap = {RightCrabs,4,{{0,0},{64,8}}};

Bitmap screen;
Rectangle rr,r = {100,100,200,200};
Point dp,p;

	Texture grey = {
		0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,
		0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,
	};

small(i)
register i;
{
	return((rand() % i) - (i >> 1));
}

PickVel(velocity)
Point *velocity;
{
  do {
    velocity->x = small(13);
    velocity->y = small(13);
  } while ((velocity->x == 0) && (velocity->y == 0));
}

ChangeVel(velocity)
Point *velocity;
{
   do {
     velocity->x += small(3);
     velocity->y += small(3);
   } while ((velocity->x < -7) && (velocity->x > 7) &&
            (velocity->y < -7) && (velocity->y > 7));
}

int visible;

CrabBlit(p,v)
Point p,v;
{
   int x,y,index;
   Bitmap *whichcrab;
   if (visible) {
     if (abs(v.x)>=abs(v.y)) {
       if (v.x>0) whichcrab = &upcrabmap;
       else whichcrab = &downcrabmap;
     } else {
       if (v.y>0) whichcrab = &rightcrabmap;
       else whichcrab = &leftcrabmap;
     }
     x = p.x % 4;
     y = p.y % 2;
     index = (y<<2)+x;
     bitblt(whichcrabmap,Rect(index<<3,0,(index+1)<<3,8),&screen,p,F_XOR);
   }
}

main(argc, argv)
	char *argv[];
{
        Rectangle insetscreen;
        int i;
        srand(*XMOUSE);
	dellayer(P->layer);
	P->layer=newlayer(Rect(0, 0, 0, 0));
	visible = 1;
	crabnum = 0;
        sleeptime = 0;
	while(argc>1){
		if(strcmp(argv[1], "-i")==0){
			visible = 0;
		}else
		if(strcmp(argv[1], "-s")==0){
			--argc; argv++;
			for(i=0; i<atoi(argv[1]); i++)
				sleep(3600);
		}else
		if(strcmp(argv[1], "-v")==0){
			--argc; argv++;
			sleeptime = atoi(argv[1]);
		}else
			crabnum=atoi(argv[1]);
		--argc; argv++;
	}
	if(crabnum<=0)
		crabnum=CRABNUM;
        if(sleeptime<=0)
		sleeptime=SLEEPTIME;
	crab=(Point *)alloc(crabnum*sizeof(Point));
	velocity=(Point *)alloc(crabnum*sizeof(Point));
	if(crab==0 || velocity==0)
		exit();
	screen.base = addr(&display,Pt(0,0));
	screen.width = 50;
	screen.rect = Jrect;
        insetscreen = inset(screen.rect,5);
        test = balloc(Rect(0,0,16,4));
        rectf(test,test->rect,F_CLR);
        for (i=0; i<crabnum; i++) {
          crab[i].x = muldiv(XMAX-20,i,crabnum)+6; crab[i].y = 6;
          PickVel(&(velocity[i]));
          CrabBlit(sub(crab[i],Pt(4,4)),velocity[i]);
        }

        for (;;) {
          sleep(sleeptime);
          for (i=0; i<crabnum; i++) {
          wait(CPU);
          CrabBlit(sub(crab[i],Pt(4,4)),velocity[i]);
          if (small(7)==0) ChangeVel(&(velocity[i]));
          crab[i] = add(crab[i],velocity[i]);
          if (!ptinrect(crab[i],insetscreen)) {
            crab[i] = sub(crab[i],velocity[i]);
            PickVel(&velocity[i]);
            CrabBlit(sub(crab[i],Pt(4,4)),velocity[i]);
          } else {
            rounded.x = (~3) & crab[i].x;
            rounded.y = (~3) & crab[i].y;
            texture(test,Rect(0,0,4,4),&grey,F_STORE);
            bitblt(&screen,raddp(Rect(0,0,4,4),rounded),test,Pt(0,0),F_XOR);
            if (*addr(test,Pt(0,0))||*addr(test,Pt(0,1))||
                *addr(test,Pt(0,2))||*addr(test,Pt(0,3))) {
              texture(&screen,raddp(Rect(0,0,4,4),crab[i]),&grey,F_STORE);
              crab[i] = sub(crab[i],velocity[i]);
              PickVel(&velocity[i]);
              }
            CrabBlit(sub(crab[i],Pt(4,4)),velocity[i]);
          }
          }
        }
}

