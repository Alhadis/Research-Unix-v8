#include <jerq.h>
#include "states.h"
#include "../menu.h"

typedef long MLONG;

#include "../protocol.h"
#include "../traps.h"
#include "../isp.h"

enum updown { UP, DOWN };
enum updown bptsw = UP;

struct Proc *p = 0;
long	loc, limit, longfromhost(), addrfromhost();
int enabled = 0, (*fcn)(), bptsave[BPTS], anticipate = 0;
extern int buttmenu[4];

char *trackbase;
int  tracklen;
char trackcopy[TRACK_MAX];
int modified;

long hatebs()
{
	static prev = ERRORED;

	if( !p ) return prev = ERRORED;
	if( !(p->state&BUSY) || p->fcn != fcn ){
	    if( prev != ERRORED ) lprintf( "\n68000: process deleted/reloaded\n" );
	    return prev = ERRORED;
	}
	if( !(p->state&ZOMBIE) ) return prev = ACTIVE;
	if( !p->traploc ) return prev = HALTED;
	switch( p->traptype ){
		case BPT_TRAP: return prev = BREAKPTED;
		case TRACE_TRAP:
			if( modified ) return prev = MODIFIED;
			return prev = STEPPED;
		default: return prev = TRAPPED;
	}
}

long align( a ) long a;
{
	return a&1 ? a+1 : a;
}

long read( a ) long a;
{
	if(a < LO_ADDR  ) return LO_ADDR;
	return a;
}

long write( a ) long a;
{
	if( a == (long) DADDR ) return a;
	if( a < LO_ADDR || a > HI_ADDR ) return LO_ADDR;
	return a;
}

#define mlong(x) (*(long *) read(align(x)))	
#define mword(x) (*(int *) read(align(x)))

main(){
	request( RCV|KBD|SEND|MOUSE );
	m_init();
	joff();
}	

joff()
{
	register long reg, i;
	long fp, function, retaddr;
	char c;

	for( ;; ){
		while( (c = charfromhost()) != ESCAPE ) put( c );
		switch( c = charfromhost() ){
		case DO_INVOKE: invokef(); break;
		case DO_WRAP: exit();
		case DO_KBD:    choosecursor();
				line();
				dmcursor();
			 	break;
		case M_MENU:
		case M_ITEM:
		case M_HIT:
		case M_NULL:
		case M_BUTT:
		case M_CONFIRM:
		case M_PENDULUM:
		case M_LIMITS:
		case M_BITVEC:
		case M_SUGGEST:
				menuop(c); break;
		case G_BITMAP:
		case G_RECTANGLE:
		case G_TEXTURE:
		case G_POINT:
				graphop(c); break;
		case PEEK_LONG:
			loc = addrfromhost();
			if( loc > INDEX_BASE && loc <= LAST_INDEX ) switch( loc ){
				case PROC_INDEX:   longtohost(p); break;
				case TEXT_INDEX:   longtohost(p->fcn); break;
				case FRAME_INDEX:  longtohost(p->sp); break;
				case TYPE_INDEX:   longtohost((long) p->traptype);
						   break;
				case TLOC_INDEX:   longtohost(p->traploc); break;
				case HATEBS_INDEX: longtohost( hatebs() );
						   break;
			}
			else longtohost( mlong(loc) );
			break;
		case PEEK_WORD:
			wordtohost( mword( addrfromhost() ) ); break;
		case PEEK_BYTE:
			sendchar( *(char *) read(addrfromhost())); break;
		case POKE_LONG:
			reg = write(align( addrfromhost()));
			* (long *) reg = longfromhost();
			break;
		case POKE_WORD:
			reg = write(align( addrfromhost()));
			* (int *) reg = (int) wordfromhost();
			break;
		case POKE_BYTE:
			reg = write(addrfromhost());
			* (char *) reg = (char) charfromhost();
			break;
		case PEEK_STR:
			reg = read(addrfromhost());
			strntohost( (char *) reg, charfromhost() ); break;
		case DO_DEBUG:
			choosecursor();
			if( p = debug() ) fcn = p->fcn;
			bptsw = DOWN;
			enabled = 0;
			dmcursor();
			break;
		case DO_VIDEO:
			for( i = 2; i; --i ){
				rectf( p->layer, p->layer->rect, F_XOR );
				nap( 20 );
			}
			break;
		case DO_BPTS:
			loc = addrfromhost(); setbpt(charfromhost()); break;
		case DO_BPTC:
			clrbpt(charfromhost()); break;
		case DO_STACK:
			function = 0;
			fp = addrfromhost();
			longtohost( retaddr = mlong(fp+4) );
			if( mword( retaddr-6 ) == JSR_ABS ){
				function = mlong( retaddr-4 );
			} else if( mword( retaddr-2 ) == JSR_IND ){
				if( mword( retaddr-8 ) == MOVL_ABS )
					function = mlong( mlong( retaddr-6 ) );
			}
			longtohost( function );
			break;
		case DO_FUNC:
			function = addrfromhost(); break;
		case DO_DELTAS:
			longtohost(mword(function+4)==MOVML ? mlong(function+6) :0);
			break;
		case DO_HALT:
			halt(); break;
		case DO_ACT:
			run(); break;
		case DO_LIMIT:
			limit = addrfromhost(); break;
		case DO_SING:
			single(limit); break;
		case DO_ENABLE:
			enabled = 1; break;
		case DO_DISABLE:
			enabled = 0; break;
		case DO_ANTIC:
			anticipate = 1; break;
		case DO_TRACK:
			trackbase = (char *) addrfromhost();
			tracklen = charfromhost();
			{	register i;
				for( i = 0; i < tracklen; ++i )
					trackcopy[i] = trackbase[i];
			}
			break;	
		default: put( c );
		}
	}
}

run()
{
	switch( hatebs() ){
		case ERRORED:
		case TRAPPED:
		case ACTIVE: return;
 		case HALTED: break;
		case BREAKPTED:
			single(0);
			if( hatebs() != STEPPED ) return;	/* MODIFIED ? */
		case MODIFIED:
		case STEPPED:
			p->traptype = 0;
			p->state |= RUN;	/* trap() in mpx does sw(0) */
	}
	laybpts();
	p->state &= ~ZOMBIE;
	p->traploc = 0;
}

halt()
{
	if( hatebs() != ACTIVE ) return;
	p->state |= ZOMBIE;
	liftbpts();
}

letrun()
{
	p->state &= ~ZOMBIE;
	p->state |= RUN;	/* trap() in mpx does sw(0) */
	p->traploc = 0;
	wait( CPU );
	while( hatebs() == ACTIVE ) wait( CPU );
}

trapped( t )
{
	return p->state&ZOMBIE && p->traploc && p->traptype == t;
}

jsrsize( pc )
long pc;
{
	register inst = mword(pc), mode_reg;

	if( (inst&0xFFC0) == 0x4E80 ){	/* jsr */
		mode_reg = inst & 077;
		if( (mode_reg&070) == 020 ) return 2;		/* jsr (%a?) */
		if( mode_reg == 072 ) return 4;	/* jsr <16>(%pc) */
		if( mode_reg == 071 ) return 6;	/* jsr <32> */
		return 0;
	}
	if( (inst&0xFF00) == 0x6100 ){	/* bsr */
		return inst & 0x00FF ? 2 : 4;		/* <8> or <16> */
	}
	return 0;
}

call_step( pc )
long pc;
{
	long	*oldsp = p->sp, afterjsr = pc + jsrsize(pc);
	int 	save = mword(afterjsr);

	for( ;; ){
		*(int *) afterjsr = TRAP_BPT;
		traceoff();
		letrun();
		*(int *) afterjsr = save;
		if( !trapped(BPT_TRAP) ) return;
		*( (long *) p->traploc ) -= 2;		/* fake a trace trap */
		p->traptype = TRACE_TRAP;
		traceon();
		if( p->sp < oldsp ){		/* called recursively ! */
			single( 0 );
			continue;
		}
		return;
	}
}

traceon()
{
	*( (int *) (p->traploc-2) ) |= 0x8000;
}

traceoff()
{
	*( (int *) (p->traploc-2) ) &= 0x7FFF;
}

trackchange()
{
	register i;

	for( i = 0; i < tracklen; ++i )
		if( trackbase[i] != trackcopy[i] ) return 1;
	return 0;
}

single(limit)
register long limit;
{
	register long startpc, pc, loop;

	modified = 0;
	switch( hatebs() ){
		default: return;
		case BREAKPTED:	*( (long *) p->traploc ) -= 2;
		case STEPPED:	traceon(); break;
	}
	pc = startpc = *((long *) p->traploc);
	for( loop = 1; loop; ){
		if( limit && jsrsize(pc) ) call_step( pc );
		else letrun();
		loop = 0;
		if( trapped( TRACE_TRAP) ){	/* There has to be a better way */
			loop = 1;		/* to say this */
			if( trackbase && trackchange() ){
				modified = 1;
				loop = 0;
			}
			pc = *( (long *) p->traploc );
			if( pc < startpc || pc >= limit ) loop = 0;
		}
	}
	traceoff();
	return;
}

line()
{
	static char buf[128];
	static int i = 0;
	register char c;
	
	if( anticipate ){
		buf[i] = '\0';
		lprintf( "%s", buf );
		anticipate = 0;
	} else i = 0;
	for( ;; ){
		put( '_' );
		while( (c = kbdchar()) == -1 ){
			if( P->state & (RESHAPED|MOVED) ) put( '_' );
			if( enabled && p->state&ZOMBIE && limit-- > 1 ){
				liftbpts();
				run();
			} else if( enabled &&
			    (!(p->state&BUSY) || p->fcn!=fcn || p->state&ZOMBIE )){
				put( '\v' );
				liftbpts();
				sendchar( ESCAPE );
				sendchar( DO_EVENT );
				sendchar( '\r' );
				anticipate = 1;
				enabled = 0;
				return;
			}
			if( (own()&MOUSE) && (freehit(2) || freehit(3)) ){
				put( '\v' );
				sendchar( '\r' );
				anticipate = 1;
				return;
			}
			sleep( 6 );
		}
		lprintf( "\b_\b" );
		if( c == '\r' || c == '\n' ) break;
		if( c != '\b' ) put( buf[i++] = c );
		else if( i > 0 ) lprintf( "\b%c\b", buf[--i] );
	}
	sendnchars( i, buf );
	sendchar( '\r' );
}

hosterr()
{
	*(long *)0 = 0;
}


laybpt(i)
{
	if( !bptloc[i] || hatebs() == ERRORED ) return;
	if( mword(bptloc[i]) == TRAP_BPT )
	    lprintf( "68000: bpt TRAP already set:%D\n", bptloc[i] );
	bptsave[i] = mword(bptloc[i]);
	*(int *) bptloc[i] = TRAP_BPT;
}

liftbpt(i)
{
	if( !bptloc[i] || hatebs() == ERRORED ) return;
	if( mword(bptloc[i]) != TRAP_BPT )
	    lprintf( "68000: bpt TRAP altered:%D\n", bptloc[i] );
	*(int *) bptloc[i] = bptsave[i];
}


laybpts()
{
	register int i;

	if( bptsw == DOWN || hatebs() == ERRORED ) return;
	for( i = 0; i < BPTS; ++i) laybpt(i);
	bptsw = DOWN;
}

liftbpts()
{
	register int i;

	if( bptsw == UP || hatebs() == ERRORED ) return;
	for( i = 0; i < BPTS; ++i ) liftbpt(i);
	bptsw = UP;
}


setbpt( i )
{
	bptloc[i] = write(align(loc));
	if( bptsw == DOWN ) laybpt( i );
}

clrbpt( i )
{
	if( bptsw == DOWN ) liftbpt( i );
	bptloc[i] = 0;
}

#define ARGSPACE 100

invokef()
{
	static struct Arg { char args[ARGSPACE]; } a;
	static union Func {
		long (*d0_ret)();
		long *(*a0_ret)();
		long start;
	} f;
	register long adf;
	register z = p->state&ZOMBIE;
	register enum updown b = bptsw;

	p->state |= ZOMBIE;
	if( b == DOWN ) liftbpts();
	switch( adf = addrfromhost() ){
		case I_DATA:
			longtohost( (*f.d0_ret)(a) );
			break;
		case I_ADDR:
			longtohost( (*f.a0_ret)(a) );
			break;
		default:
			f.start = adf;
			longtohost( a.args );
			break;
	}
	if( b == DOWN ) laybpts();
	if( !z ) p->state &= ~ZOMBIE;
}
