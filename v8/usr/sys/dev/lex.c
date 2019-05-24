#include "lex.h"
#if NLEX > 0

/*
 *      LEXIDATA 3400 DEVICE DRIVER
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

#define ERROR 0160000
#define GO 01
#define IE 0100
#define READY 0200
#define ORDY 01000
#define RESET 04
#define LOAD 02
#define TEST 06
#define SEND 010000
#define LEXPRI PZERO+1
#define LXRST 01
#define LXULD 02
#define LXWAKE 03

short cursxy[NLEX] = {29}; /*CURSXY command code*/
int cursxmax[NLEX] = {640}; /*Max. screen x dimension*/
int cursymax[NLEX] = {512}; /*Max. screen x dimension*/

struct lexdevice{
        short wc;
        short ba;
        short cs;
        short db;
        };

struct lexparam{
        int lex_open;           /*open flag*/
        struct buf lex_buf;     /*i/o buffer header*/
        int lex_uba;            /*unibus data path descriptor*/
        } lexsoft[NLEX];

int lexprobe(),lexattach();

struct uba_device *lexinfo[NLEX];
u_short lexstd[] = {0164100,0164110,0};

struct uba_driver lexdriver =
        {lexprobe,0,lexattach,0,lexstd,"lex",lexinfo};

lexprobe(reg)           /*called by autoconfig(4) - fake an interrupt*/
caddr_t reg;{
        register int br,cvec;
        br=0x15;
        switch((int)(reg)&070){
        case 0: cvec=0370; break;
        case 010: cvec=0364; break;
                }
        return(1);
        }

lexattach(ui)           /*driver initialization*/
struct uba_dev *ui;{
        }

lexopen(dev)
dev_t dev;{
        register struct lexdevice *lxp;
        register struct uba_device *ui;
        register struct lexparam *lexs;
        if(minor(dev)>=NLEX){
                u.u_error=ENXIO;        /*device does not exist*/
                return;
                }
        lexs= &lexsoft[minor(dev)];
        ui=lexinfo[minor(dev)];
        if((lexs->lex_open)||(ui==0)||(ui->ui_alive==0)){
                u.u_error=ENXIO;        /*device already open or not really present*/
                return;
                }
        lxp=(struct lexdevice *)(ui->ui_addr);
        lexs->lex_open++;       /*flag device as open*/
        lxp->cs=RESET;
        lxp->db=0;                      /*send START to 3400*/
        lxp->cs=0;
        }

lexclose(dev)
dev_t dev;{
        register struct lexdevice *lxp;
        register struct uba_device *ui;
        register struct lexparam *lexs;
        register spri;
        ui=lexinfo[minor(dev)];
        lxp=(struct lexdevice *)(ui->ui_addr);
        lexs= &lexsoft[minor(dev)];
        spri=spl5();
        if(lxp->cs&READY==0) tsleep(&lexs->lex_buf,LEXPRI,15);              /*wait for DMA to complete*/
        splx(spri);
        if(lexs->lex_uba) ubarelse(ui->ui_ubanum,&lexs->lex_uba);

        lxp->cs=0;                      /*interrupts off*/
        lexs->lex_open=lexs->lex_uba=0; /*clear flags*/
        }

lexstrategy(bp)
register struct buf *bp;{
        register struct lexdevice *lxp;
        register struct uba_device *ui;
        register struct lexparam *lexs;
        register eabits;
        lexs= &lexsoft[minor(bp->b_dev)];
        ui=lexinfo[minor(bp->b_dev)];
        lxp=(struct lexdevice *)(ui->ui_addr);
        lexs->lex_uba=ubasetup(ui->ui_ubanum,bp,UBA_NEEDBDP);
        lxp->wc=(short)(-(bp->b_bcount)/2);
        lxp->ba=(short)(lexs->lex_uba&0xffff);
        eabits=(lexs->lex_uba>>12)&0x30;
        spl5();
        lxp->cs=(short)(IE|GO|((bp->b_flags&B_READ)?0:SEND)|eabits);
        }

lexwrite(dev)
dev_t dev;{
        physio(lexstrategy,&lexsoft[minor(dev)].lex_buf,dev,B_WRITE,minphys);
        }

lexread(dev)
dev_t dev;{
        physio(lexstrategy,&lexsoft[minor(dev)].lex_buf,dev,B_READ,minphys);
        }

lexintr(dev)
dev_t dev;{
        register struct lexdevice *lxp;
        register struct lexparam *lexs;
        register struct uba_device *ui;
        register struct buf *bp;
        lexs= &lexsoft[minor(dev)];
        bp= &lexs->lex_buf;
        ui=lexinfo[minor(dev)];
        lxp=(struct lexdevice *)(ui->ui_addr);
        bp->b_flags|=B_DONE;
        if(lxp->cs&ERROR) bp->b_flags|=B_ERROR;
        lxp->cs=0;
        if(lexs->lex_uba) ubarelse(ui->ui_ubanum,&lexs->lex_uba);
        wakeup(bp);
        }

lexioctl(dev,cmd,addr,flag)
dev_t dev;
caddr_t addr;{
        register struct lexdevice *lxp;
        register struct uba_device *ui;
        register struct lexparam *lexs;
int i;
        ui=lexinfo[minor(dev)];
        lxp=(struct lexdevice *)(ui->ui_addr);
        lexs= &lexsoft[minor(dev)];
        switch(cmd){
        case LXRST:             /*send reset to display*/
                lxp->cs=RESET;
                lxp->db=0;
                return;
        case LXULD:             /*load microcode*/
                lxp->cs=SEND|LOAD;
                lxp->db=0;
                return;
        case LXWAKE:            /*panic-attempt to wakeup*/
                printf("waking-up %x\n",&lexs->lex_buf);
                printf("flags= %x\n",lexs->lex_buf.b_flags);
                lexs->lex_buf.b_flags |= B_DONE;
                wakeup(&lexs->lex_buf);
                return;
        default:
                u.u_error=EINVAL;
                }
        }

lexcur(dev,x,y)         /*position cursor - called from bpd driver*/
dev_t dev;{
        register struct lexdevice *lxp;
        register struct uba_device *ui;
        register i;
        register struct lexparam *lexs;
        ui=lexinfo[minor(dev)];
        lxp=(struct lexdevice *)(ui->ui_addr);
        lexs= &lexsoft[minor(dev)];
        if(lexs->lex_open==0) return;
        if((lxp->cs&READY==0)||(lexs->lex_buf.b_flags&B_BUSY)) return;
        lxp->cs=SEND;
        lxp->db=cursxy[minor(dev)];
        x=(x*cursxmax[minor(dev)])/1024;
        for(i=1000;((lxp->cs&ORDY)==0)&&i;i--);
        if(i) lxp->db=(short)x;
        y=cursymax[minor(dev)]-1-(y*cursymax[minor(dev)])/1024;
        for(;((lxp->cs&ORDY)==0)&&i;i--);
        if(i) lxp->db=(short)y;
        for(;((lxp->cs&ORDY)==0)&&i;i--);
        if(i==0){
                printf("plot cursor timeout\n");
                lxp->cs=RESET;
                lxp->db=0;
                }
        }

#endif NLEX
