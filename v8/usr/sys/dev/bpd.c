#include "bpd.h"
#if NBPD > 0

/*
 *      BIT-PAD DEVICE DRIVER
*/

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/ioctl.h"
#include "../h/buf.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/pte.h"
#include "../h/ubavar.h"
#include "../h/ubareg.h"


#define RIE 0100
#define DTR 02
#define RTS 04
#define SYNC 0100
#define DMASK 077
#define ERROR 060000
#define CMOD 01
#define STRM 02
#define CURON 03
#define POINT 04
#define NOPLOT 05
#define REPLOT 06
#define SLPRI PZERO+1

struct bpddevice{
        short rcs;
        short rdb;
        short tcs;
        short tdb;
        };

struct bpdparam{
        int bpd_x;              /*bitpad x address*/
        int bpd_y;              /*bitpad y address*/
        int bpd_cflags;         /*bitpad buttons*/
        int bpd_tx;             /*temp x address*/
        int bpd_ty;             /*temp y address*/
        int bpd_tflags;         /*temp buttons*/
        int bpd_curx;           /*current x screen cursor*/
        int bpd_cury;           /*current y screen cursor*/
        int bpd_bn;             /*byte no. - for sequencing*/
        int bpd_open;           /*device open flag*/
        int bpd_stream;         /*stream flag*/
        int bpd_cursor;         /*cursor enable flag*/
	int bpd_noplot;		/*noplot mode hack*/
        int bpd_on;             /*set if bit-pad running*/
	int bpd_ubad;		/*physical unibus address for dma's*/ 
        } bpdsoft[NBPD];

char curs_data[512];		/*buffer for cursor dma transfers*/

int bpdprobe(),bpdattach();

struct uba_device *bpdinfo[NBPD];
u_short bpdstd[]= {0176510,0176520,0};

struct uba_driver bpddriver =
        {bpdprobe,0,bpdattach,0,bpdstd,"bpd",bpdinfo};

bpdprobe(reg)           /*called by autoconfig(4) - fake an interrupt*/
caddr_t reg;{
        register int br,cvec;
        br=0x15;
        switch((int)(reg)&070){
        case 010: cvec=0410; break;
        case 020: cvec=0420; break;
                }
        return(1);
        }

bpdattach(ui)           /*driver initialization*/
struct uba_dev *ui;{
	
        }

bpdopen(dev)
dev_t dev;{
        register struct bpddevice *bpp;
        register struct uba_device *ui;
        register struct bpdparam *bpds;
        if(minor(dev)>=NBPD){
                u.u_error=ENXIO;        /*device does not exist*/
                return;
                }
        bpds= &bpdsoft[minor(dev)];
        ui=bpdinfo[minor(dev)];
        if((bpds->bpd_open)||(ui==0)||(ui->ui_alive==0)){
                u.u_error=ENXIO;        /*device already open or not really present*/
                return;
                }
        bpp=(struct bpddevice *)(ui->ui_addr);
        bpds->bpd_open++;       /*flag device as open*/
        bpds->bpd_on++;         /*flag device as on*/
        bpds->bpd_stream=bpds->bpd_cursor=bpds->bpd_x=bpds->bpd_y=bpds->bpd_cflags=0;
        bpp->tcs=0;             /*output interrupt off*/
        bpp->rcs= RTS|DTR;      /*set up modem controls*/
        bpp->tdb= 'L';          /*set to 35 samples/sec. stream*/
        bpp->rcs= RIE|DTR|RTS;  /*enable receive interrupts*/
        }

bpdclose(dev)
dev_t dev;{
        register struct bpddevice *bpp;
        register struct uba_device *ui;
        register struct bpdparam *bpds;
        ui=bpdinfo[minor(dev)];
        bpp=(struct bpddevice *)(ui->ui_addr);
        bpds= &bpdsoft[minor(dev)];
        bpp->rcs= 0;            /*turn interrupts off*/
        bpds->bpd_stream=bpds->bpd_open=0;
        }

bpdintr(dev) dev_t dev;
{
 register struct uba_device *ui = bpdinfo[minor(dev)];
 register struct bpddevice *bpp = (struct bpddevice *)(ui->ui_addr);
 register struct bpdparam *bpds = &bpdsoft[minor(dev)];
 int dx, dy;

    if(bpp->rdb & ERROR)
      { bpds->bpd_bn= -1;
	return;
      }
    if(bpp->rdb & SYNC) bpds->bpd_bn=1;
    if(bpds->bpd_bn < 0) return;
    switch(bpds->bpd_bn)
      { case 1:	bpds->bpd_tflags = (int)(bpp->rdb&DMASK);
		break;
	case 2: bpds->bpd_tx = (int)(bpp->rdb&DMASK);
                break;
        case 3: bpds->bpd_tx += (int)((bpp->rdb&DMASK)<<6);
		break;
	case 4: bpds->bpd_ty = (int)(bpp->rdb&DMASK);
		break;
        case 5: bpds->bpd_ty += (int)((bpp->rdb&DMASK)<<6);
                if(bpds->bpd_tflags != bpds->bpd_cflags) wakeup(bpds);
                if((bpds->bpd_cursor)&&(bpds->bpd_on))
		  { if(bpds->bpd_tx<bpds->bpd_x)
			bpds->bpd_curx=(bpds->bpd_tx+1)/2;
		    else if(bpds->bpd_tx>bpds->bpd_x)
			bpds->bpd_curx=(bpds->bpd_tx)/2;
		    if(bpds->bpd_ty<bpds->bpd_y)
			bpds->bpd_cury=(bpds->bpd_ty+1)/2;
		    else if(bpds->bpd_ty>bpds->bpd_y)
			bpds->bpd_cury=(bpds->bpd_ty)/2;

		    if (!bpds->bpd_noplot)
			 {
				dx = bpds->bpd_x - bpds->bpd_tx;
			 	if(dx < 0) dx = -dx;
			  	dy = bpds->bpd_y - bpds->bpd_ty;
			  	if(dy < 0) dy = -dy;
			  	if(dx > 1 || dy > 1)
					omcur(dev,bpds->bpd_curx,bpds->bpd_cury);
			  }
		  }
		bpds->bpd_x=bpds->bpd_tx;
		bpds->bpd_y=bpds->bpd_ty;
		bpds->bpd_cflags=bpds->bpd_tflags;
		bpds->bpd_bn=0;
		break;
      }
    bpds->bpd_bn++;
}

bpdioctl(dev,cmd,addr,flag)
dev_t dev;
caddr_t addr;{
        register struct bpddevice *bpp;
        register struct uba_device *ui;
        register struct bpdparam *bpds;
        char c;
        ui=bpdinfo[minor(dev)];
        bpp=(struct bpddevice *)(ui->ui_addr);
        bpds= &bpdsoft[minor(dev)];
        switch(cmd){
        case CMOD:              /*send char. to change mode*/
                c=fubyte(addr)&0177;
                bpds->bpd_on=(c=='S')?0:1;
                bpp->tdb=(short)c;
                break;
        case STRM:              /*put driver into stream mode*/
                bpds->bpd_stream=1;
                break;
        case CURON:             /*turn cursor on*/
                bpds->bpd_cursor=1;
                break;
        case POINT:             /*turn off stream mode*/
                bpds->bpd_stream=0;
                break;
 	case NOPLOT:		/*track without plotting on device*/
		bpds->bpd_noplot=1;
		break;
	case REPLOT:
		bpds->bpd_noplot=0;
		break;
               }
        }

bpdread(dev)
dev_t dev;{
        register struct bpdparam *bpds;
        register *dp= (int *)(u.u_base);
        bpds= &bpdsoft[minor(dev)];
        if(bpds->bpd_stream==0){
                while(bpds->bpd_cflags) sleep(bpds,SLPRI);      /*wait for buttons up*/
                while(!(bpds->bpd_cflags)) sleep(bpds,SLPRI);   /*wait for button down*/
                }
        suword(dp++,bpds->bpd_x);
        suword(dp++,bpds->bpd_y);
        suword(dp,bpds->bpd_cflags);
        u.u_base=(caddr_t)dp;
        u.u_count=0;
        }
