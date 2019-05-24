/* BITBLT()	Moves bits around on the screen, and does a LOT of it
 *
 *	WARNING
 *	WARNING:  to any future modifier of this code.  This is highly 
 * 		hand optimized code, it pushes the frame pointer and
 *		references locals off the stack pointer, 
 *		and it even simulates register
 *		allocation.  Be very careful if you edit this.
 */

#include <jerq.h>
		/* alignment codes for narrow rectangles */
#define S_STRADDLE	0x4	/* source straddles a word boundary */
#define D_STRADDLE	0x8	/* dest straddles a word boundary */
#define LEFTDIR	8
#define NOSHIFT 4
#define DAMMIT	4		/* you'll see why */
#undef	sw
#define DX1	m

#undef bitblt
bitblt(sm,r,dm,p,fc)
#define bitblt Sbitblt
Bitmap *sm,*dm;
Rectangle r;
Point p;
int fc;
{
	register Word *source,*dest;		/* %r8-%r7 */
	register Word sw, dw;			/* %r6-%r5 */
	register UWord m;			/* %r4     */
	register int i;				/* %r3     */

	int a,b,j,h,w,dx1,px31,rx31;
	unsigned int mask1,mask2,mask3,mask4,ntmask1,ntmask2;
	int dummy1, dummy2;		/* for future use, CANNOT CHANGE */
					/* NUMBER OF LOCALS DUE TO %fp 	*/
					/* TRICKS			*/


	/* clip to the source Bitmap */
	if(r.origin.x < sm->rect.origin.x){
		p.x+=sm->rect.origin.x-r.origin.x;
		r.origin.x=sm->rect.origin.x;
	}
	if(r.corner.x > sm->rect.corner.x)
		r.corner.x=sm->rect.corner.x;
	if(r.origin.y < sm->rect.origin.y){
		p.y+=sm->rect.origin.y-r.origin.y;
		r.origin.y=sm->rect.origin.y;
	}
	if(r.corner.y > sm->rect.corner.y)
		r.corner.y=sm->rect.corner.y;
	/*
	 * If the sm->rect and r are disjoint, r is now degenerate,
	 * due to the clipping, and the next clipping code will reject it.
	 * This is safe because we will only increase origin or decrease corner.
	 */
	/* clip to the destination Bitmap */
	if(p.x < dm->rect.origin.x){
		r.origin.x+=dm->rect.origin.x-p.x;
		p.x=dm->rect.origin.x;
	}
	if(p.y < dm->rect.origin.y){
		r.origin.y+=dm->rect.origin.y-p.y;
		p.y=dm->rect.origin.y;
	}
	if(r.corner.x-r.origin.x > dm->rect.corner.x-p.x)
		r.corner.x=r.origin.x+(dm->rect.corner.x-p.x);
	if(r.corner.y-r.origin.y > dm->rect.corner.y-p.y)
		r.corner.y=r.origin.y+(dm->rect.corner.y-p.y);
	i = r.corner.y - r.origin.y;	/* going to be h */
	dw = r.corner.x - r.origin.x - 1;	/* going to be dx1 */
	if (i <= 0 || dw < 0)
		return;
	if (dw < 32)
		goto narrow;
	DX1 = dw;
	h = i; 
	ntmask1 = topbits[p.x & 0x1f];
	mask1 = ~ntmask1;
	mask2 = topbits[((p.x+DX1) & 31) + 1];
	ntmask2 = ~mask2;
	/*
		the following code is wrong.
		the moral is, do it right
	*/
	/*w = ((p.x+DX1) >> 5) - (p.x >> 5);	/* inner loop+1: sub 1 later*/
	w = (((p.x+DX1)&~31)/32) - ((p.x&~31)/32);	/* inner loop+1: sub 1 later*/
	sw = ((sm->width - w) << 2);	/* sleazy hack to avoid shift */
	dw = ((dm->width - w) << 2);	/* in outer, inner loops */
	if (sm == dm) {		/* may have to mess with loop order */
		if (r.origin.y < p.y) {		/* swap top with bottom */
			r.origin.y += i-1;
			p.y += i-1;
			if (r.origin.x < p.x) {	/* swap left with right */
				fc |= LEFTDIR;
				r.origin.x = r.origin.x + DX1;
				p.x = p.x + DX1;
				sw = -sw;
				dw = -dw;
			}
			else
			{
				sw -= (sm->width << 3);	/* -(w+n) == (w-n) - 2*w  */
				dw -= (dm->width << 3) ;
			}
		}
		else
		{
			if (r.origin.x < p.x) {	/* swap left with right */
				fc |= LEFTDIR;
				r.origin.x = r.origin.x + DX1;
				p.x = p.x + DX1;
				sw = (sm->width + w) << 2;
				dw = (dm->width + w) << 2;
			}
		}
	}
	w--;			/* subtract the 1 like we promised */
	px31 = p.x & 0x1f;		/* commonly used expression */
	rx31 = r.origin.x & 0x1f;	/* commonly used expression */
	dest = addr(dm,p);
	source = addr(sm,r.origin);
	a = px31 - rx31;
	if(a == 0)
		fc |= NOSHIFT;
	else if (a < 0)
		a += 32;
		/* a == 0 means no shift, remember that */
	b = 32 - a;
	switch (fc) {

	case F_OR | NOSHIFT | LEFTDIR:
		b = w>>2;
		w = w&3;
		m = h;		/* m is free => use it */
		do {
			*dest-- |= mask2 & *source--;
			if ((i = b) > 0) do {
				*dest |= *source;
				*(dest-1) |= *(source-1);
				*(dest-2) |= *(source-2);
				*(dest-3) |= *(source-3);
				dest -= 4;
				source -= 4;
			} while (--i > 0);
			if ((i = w) > 0) do {
				*dest-- |= *source--;
			} while (--i > 0);
			*dest |= mask1 & *source;
			asm(" ADDW2	%r6,%r8");	/*source += sw; */
			asm(" ADDW2	%r5,%r7");	/*dest += dw; */
		} while (--m != 0);
		break;
	case F_OR | LEFTDIR:
		if ((px31) < (rx31))
			source++;	/* adjust for pipeline */
		do {
			m = *source--;	/* m is a free register */
			*dest-- |= (((m >> a) | (*source << b)) & mask2);
			if ((i=w) > 0) do {
				m = (*source--) >> a;
				*dest-- |= m | (*source << b);
			} while (--i > 0);
			m = *source;	/* m is a free register */
			*dest |= (((m >> a) | (*(source-1) << b)) & mask1);
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--h > 0);
		break;
	case F_OR | NOSHIFT:
		b = w>>2;
		w = w&3;
		m = h;		/* m is free => use it */
		do {
			*dest++ |= (mask1 & *source++);
			if ((i = b) > 0) do {
				*dest |= *source;
				*(dest+1) |= *(source+1);
				*(dest+2) |= *(source+2);
				*(dest+3) |= *(source+3);
				dest += 4;
				source += 4;
			} while (--i > 0);
			if ((i = w) > 0) do {
				*dest++ |= *source++;
			} while (--i > 0);
			*dest |= (mask2 & *source);
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--m != 0);
		break;
	case F_OR:
		if ((px31) > (rx31))
			source--;	/* adjust for pipeline */
		do {
			m = *source++;	/* m is a free register */
			*dest++ |= (((m << b) | (*source >> a)) & mask1);
			if ((i=w) > 0) do {
				m = (*source++) << b;
				*dest++ |= m | (*source >> a);
			} while (--i > 0);
			m = *source;	/* m is a free register */
			*dest |= (((m << b) | (*(source+1) >> a)) & mask2);
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--h > 0);
		break;
	case F_CLR | NOSHIFT | LEFTDIR:
		b = w>>2;
		w = w&3;
		m = h;		/* m is free => use it */
		do {
			*dest-- &= ~(mask2 & *source--);
			if ((i = b) > 0) do {
				*dest &= ~(*source);
				*(dest-1) &= ~(*(source-1));
				*(dest-2) &= ~(*(source-2));
				*(dest-3) &= ~(*(source-3));
				dest -= 4;
				source -= 4;
			} while (--i > 0);
			if ((i = w) > 0) do {
				*dest-- &= ~(*source--);
			} while (--i > 0);
			*dest &= ~(mask1 & *source);
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--m != 0);
		break;
	case F_CLR | LEFTDIR:
		if ((px31) < (rx31))
			source++;	/* adjust for pipeline */
		do {
			m = *source--;	/* m is a free register */
			*dest-- &= ~((((m >> a) | (*source << b)) & mask2));
			if ((i=w) > 0) do {
				m = (*source--) >> a;
				*dest-- &= ~(m | (*source << b));
			} while (--i > 0);
			m = *source;	/* m is a free register */
			*dest &= ~((((m >> a) | (*(source-1) << b)) & mask1));
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--h > 0);
		break;
	case F_CLR | NOSHIFT:
		b = w>>2;
		w = w&3;
		m = h;		/* m is free => use it */
		do {
			*dest++ &= ~((mask1 & *source++));
			if ((i = b) > 0) do {
				*dest &= ~(*source);
				*(dest+1) &= ~(*(source+1));
				*(dest+2) &= ~(*(source+2));
				*(dest+3) &= ~(*(source+3));
				dest += 4;
				source += 4;
			} while (--i > 0);
			if ((i = w) > 0) do {
				*dest++ &= ~(*source++);
			} while (--i > 0);
			*dest &= ~((mask2 & *source));
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--m != 0);
		break;
	case F_CLR:
		if ((px31) > (rx31))
			source--;	/* adjust for pipeline */
		do {
			m = *source++;	/* m is a free register */
			*dest++ &= ~((((m << b) | (*source >> a)) & mask1));
			if ((i=w) > 0) do {
				m = (*source++) << b;
				*dest++ &= ~(m | (*source >> a));
			} while (--i > 0);
			m = *source;	/* m is a free register */
			*dest &= ~((((m << b) | (*(source+1) >> a)) & mask2));
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--h > 0);
		break;
	case F_XOR | NOSHIFT | LEFTDIR:
		b = w>>2;
		w = w&3;
		m = h;		/* m is free => use it */
		do {
			*dest-- ^= mask2 & *source--;
			if ((i = b) > 0) do {
				*dest ^= *source;
				*(dest-1) ^= *(source-1);
				*(dest-2) ^= *(source-2);
				*(dest-3) ^= *(source-3);
				dest -= 4;
				source -= 4;
			} while (--i > 0);
			if ((i = w) > 0) do {
				*dest-- ^= *source--;
			} while (--i > 0);
			*dest ^= mask1 & *source;
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--m != 0);
		break;
	case F_XOR | LEFTDIR:
		if ((px31) < (rx31))
			source++;	/* adjust for pipeline */
		do {
			m = *source--;	/* m is a free register */
			*dest-- ^= (((m >> a) | (*source << b)) & mask2);
			if ((i=w) > 0) do {
				m = (*source--) >> a;
				*dest-- ^= m | (*source << b);
			} while (--i > 0);
			m = *source;	/* m is a free register */
			*dest ^= (((m >> a) | (*(source-1) << b)) & mask1);
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--h > 0);
		break;
	case F_XOR | NOSHIFT:
		b = w>>2;
		w = w&3;
		m = h;		/* m is free => use it */
		do {
			*dest++ ^= (mask1 & *source++);
			if ((i = b) > 0) do {
				*dest ^= *source;
				*(dest+1) ^= *(source+1);
				*(dest+2) ^= *(source+2);
				*(dest+3) ^= *(source+3);
				dest += 4;
				source += 4;
			} while (--i > 0);
			if ((i = w) > 0) do {
				*dest++ ^= *source++;
			} while (--i > 0);
			*dest ^= (mask2 & *source);
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--m != 0);
		break;
	case F_XOR:
		if ((px31) > (rx31))
			source--;	/* adjust for pipeline */
		asm(" PUSHW %ap");
		m = topbits[a];
		asm(" MCOMW %r4, %r1");
		asm(" MOVW %r4, %r2");
		asm(" MOVW 0(%fp),%ap");
		asm(" PUSHW %fp");
	asm("BW_XORLOOP:");
		asm(" ROTW %ap, 0(%r8), %r4");
		asm(" ANDW2 %r2,%r4");
		asm(" ADDW2 &4,%r8");
		asm(" ROTW %ap,0(%r8),%r0");
		asm(" ANDW3 %r0,%r1,%fp");
		asm(" ORW2 %r4,%fp");
		asm(" ANDW2 -0x28(%sp),%fp");
		asm(" XORW2 %fp,0(%r7)");
		asm(" ADDW2 &4,%r7");
			/* if (i = w) */
		asm(" MOVW -0x38(%sp),%r3");
		asm(" BEB BW_XORINNER");
		do{
			asm(" ANDW3 %r2,%r0,%r4");
			asm(" ADDW2 &4,%r8");
			asm(" ROTW %ap, 0(%r8),%r0");
			asm(" ANDW3 %r0,%r1,%fp");
			asm(" ORW2 %r4,%fp");
			asm(" XORW2 %fp,0(%r7)");
			asm(" ADDW2 &4, %r7");
		} while (--i > 0);
	asm("BW_XORINNER:");
		asm(" ROTW %ap, 0(%r8),%r4");
		asm(" ANDW2 %r2,%r4");
		asm(" LRSW3 %ap, 4(%r8),%r0");
		asm(" ORW2 %r4,%r0");
		asm(" ANDW2 -0x24(%sp),%r0");
		asm(" XORW2 %r0,0(%r7)");
		asm(" ADDW2	%r6,%r8");	/*source += sw; */
		asm(" ADDW2	%r5,%r7");	/*dest += dw; */
			/* } while (--h > 0); */
		asm(" DECW -0x3c(%sp)");
		asm(" BGB BW_XORLOOP");

		asm(" POPW %fp");
		asm(" POPW %ap");

	/* above is similar to: 
/*		do {
/*			m = *source++ << b;	/* m is a free register */
/*			*dest++ ^= ((m & LMASK) | ((*source >> a)& RMASK) & mask1);
/*			if ((i=w) > 0) do {
/*				m = ((*source++) << b) & LMASK;
/*				*dest++ ^= m | ((*source >> a)&RMASK);
/*			} while (--i > 0);
/*			m = *source;	/* m is a free register */
/*			*dest ^= (((m << b) | (*(source+1) >> a)) & mask2);
/*			asm(" addw2	%r6,%r8");	/*source += sw; */
/*			asm(" addw2	%r5,%r7");	/*dest += dw; */
/*		} while (--h > 0);		*/
		break;
	case F_STORE | NOSHIFT | LEFTDIR:
		b = w>>2;
		w = w&3;
		m = h;		/* m is free => use it */
		do {
			*dest = (ntmask2 & *dest) | (mask2 & *source--);
			--dest;
			if ((i = b) > 0) do {
				*dest = *source;
				*(dest-1) = *(source-1);
				*(dest-2) = *(source-2);
				*(dest-3) = *(source-3);
				dest -= 4;
				source -= 4;
			} while (--i > 0);
			if ((i = w) > 0) do {
				*dest-- = *source--;
			} while (--i > 0);
			*dest = (ntmask1 & *dest) | (mask1 & *source);
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--m != 0);
		break;
	case F_STORE | LEFTDIR:
		if ((px31) < (rx31))
			source++;	/* adjust for pipeline */
		do {
			m = *source--;	/* m is a free register */
			*dest = (((m >> a) | (*source << b)) & mask2) |
						 (*dest & ntmask2);
			--dest;
			if ((i=w) > 0) do {
				m = (*source--) >> a;
				*dest-- = m | (*source << b);
			} while (--i > 0);
			m = *source;	/* m is a free register */
			*dest = (((m >> a) | (*(source-1) << b)) & mask1) |
						 (*dest & ntmask1);
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--h > 0);
		break;
	case F_STORE | NOSHIFT:
widestore:
		b = w >> 2;
		w = w & 3;
		m = h;
		do {
			*dest = (ntmask1 & *dest) | (mask1 & *source++);
			dest++;
			if ((i = b) > 0) do {
				*dest = *source;
				*(dest+1) = *(source+1);
				*(dest+2) = *(source+2);
				*(dest+3) = *(source+3);
				dest += 4;
				source += 4;
			} while (--i > 0);
			if ((i = w) > 0) do {
				*dest++ = *source++;
			} while (--i > 0);
			*dest = (ntmask2 & *dest) | (mask2 & *source);
			asm(" addw2	%r6,%r8");
			asm(" addw2	%r5,%r7");
		} while (--m != 0);
		break;
	case F_STORE:
		if ((px31) > (rx31))
			source--;	/* adjust for pipeline */
		do {
			m = *source++;	/* m is a free register */
			*dest = (((m << b) | (*source >> a)) & mask1) |
						 (*dest & ntmask1);
			dest++;
			if ((i=w) > 0) do {
				m = (*source++) << b;
				*dest++ = m | (*source >> a);
			} while (--i > 0);
			m = *source;	/* m is a free register */
			*dest = (((m << b) | (*(source+1) >> a)) & mask2) |
						 (*dest & ntmask2);
			asm(" addw2	%r6,%r8");	/*source += sw; */
			asm(" addw2	%r5,%r7");	/*dest += dw; */
		} while (--h > 0);
		break;
	}

	return;
narrow:
	/*
	 * width is 32 bits or less.  There are four basic cases
 	 * (in addition to the function code), which depend on whether
	 * the source and dest straddle word boundaries or not
	 */

	m = p.x & 31;		/* commonly used expression  */
	sw = r.origin.x & 31;		/* commonly used expression  */
	if (sw + dw > 31)	/* if source is NOT aligned */
	{
		fc |= S_STRADDLE;
		mask1 = ONES >> sw;
		mask2 = topbits[((sw + dw) & 31) + 1];
	}

	if (m + dw > 31)	/* if dest is NOT aligned */
	{
		fc |= D_STRADDLE;
		mask3 = ONES >> m;
		mask4 = topbits[((m + dw) & 31) + 1];
	}
	px31 = m;
	m = m - sw;
	a = dw;

	if ((sm == dm) && (r.origin.y < p.y))
	{	/* may have to mess with loop order */
		r.origin.y += i-1;
		p.y += i-1;
		sw = -(sm->width << 2);	/* sleazy hack to avoid shift */
		dw = -(dm->width << 2);	/* in outer, inner loops */
	}
	else
	{
		sw = sm->width << 2;
		dw = dm->width << 2;
	}

	source = addr(sm,r.origin);
	dest = addr(dm,p);

	switch(fc)
	{
	case F_STORE:
		mask1 = topbits[a+1] >> (px31);
		asm(" MOVW 0x20(%fp),%r1");
		do {
			asm(" ROTW %r4,0(%r8),%r2");
			asm(" XORW2 0(%r7),%r2");
			asm(" ANDW2 %r1,%r2");
			asm(" XORW2 %r2,0(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		break;
	case F_STORE | S_STRADDLE:
		mask4 = 32 - m;		/* REALLY the other shift count */
		mask3 = topbits[a+1] >> px31;
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" MOVW 0x28(%fp),%r0");	/* put mask3 in r0 */
		asm(" MOVW 0x2c(%fp),%ap");	/* put other shift in ap */

		do {
			asm(" LLSW3 %ap,0(%r8),%r1"); /* 32-m,lft shft */
			asm(" LRSW3 %r4,4(%r8),%r2");	/* m, right shift */
			asm(" ORW2  %r2, %r1");
			asm(" XORW2 0(%r7),%r1");
			asm(" ANDW2 %r0,%r1");
			asm(" XORW2 %r1,0(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		asm(" POPW %ap");	/* restore ap from stack */
		break;
	case F_STORE | D_STRADDLE:
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" MOVW 0x28(%fp),%r0");	/* put mask3 in r0 */
		asm(" MOVW 0x2c(%fp),%ap");	/* put mask4 in ap */
		do {
			asm(" ROTW %r4,0(%r8),%r1");
			asm(" XORW3 0(%r7),%r1,%r2");
			asm(" ANDW2 %r0,%r2");
			asm(" XORW2 %r2,0(%r7)");
			asm(" XORW2 4(%r7),%r1");
			asm(" ANDW2 %ap,%r1");
			asm(" XORW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		    } while (--i > 0);
		asm(" POPW %ap");	/* restore ap from stack */
		break;
	case F_STORE | S_STRADDLE | D_STRADDLE:
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" SUBW3 %r4,&0x20,%ap");	/* right shift distance */
		asm(" MOVW 0x20(%fp),%r0");	/* r0 <- mask1 */
		asm(" MOVW 0x24(%fp),%r2");	/* r2 <- mask2 */
		asm(" PUSHW %fp");	/* store fp on stack */
		do {
			asm(" ANDW3 %r0, 0(%r8), %r1");
			asm(" ANDW3 %r2, 4(%r8),%ap");
			asm(" ORW2 %ap, %r1");
			asm(" ROTW %r4, %r1, %r1");
			asm(" XORW3 0(%r7), %r1, %ap");
			asm(" ANDW2 0x28(%fp), %ap");
			asm(" XORW2 %ap, 0(%r7)");
			asm(" XORW2 4(%r7), %r1");
			asm(" ANDW2 0x2c(%fp), %r1");
			asm(" XORW2 %r1, 4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		asm(" POPW %fp");	/* restore fp from stack */
		asm(" POPW %ap");	/* restore ap from stack */
		break;
	case F_OR:
		 mask1 = topbits[a+1] >> px31;
		asm(" MOVW 0x20(%fp),%r1");	/* mask1 */
		do {
			asm(" ROTW %r4,0(%r8),%r2");
			asm(" ANDW2 %r1,%r2");
			asm(" ORW2 %r2,0(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		break;
	case F_OR | S_STRADDLE:
		asm(" MOVW 0x20(%fp),%r0");	/* store mask1 in a reg */
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" MOVW 0x24(%fp),%ap");	/* store mask1 in a reg */

		do {
			asm(" ANDW3 %r0,0(%r8),%r2"); /* x20(fp)=mask1 */
			asm(" ANDW3 %ap,4(%r8),%r1"); /* x24(fp)=mask2 */
			asm(" ORW2 %r2,%r1");
			asm(" ROTW %r4,%r1,%r1");
			asm(" ORW2 %r1,0(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		asm(" POPW %ap");	/* restore ap from stack */
		break;
	case F_OR | D_STRADDLE:
		if (a <= 16)			/* very narrow, 17 bits max */
		{
		asm(" MOVW &0xffff0000,%r2");
		asm(" ORW3 0x28(%fp),0x2c(%fp),%r0");	/* compute mask */
		do {
			asm(" ROTW %r4,0(%r8),%r1");
			asm(" ANDW2 %r0,%r1");	/* mask */
			asm(" ORH2 %r1,2(%r7)");
			asm(" ANDW2 %r2, %r1");
			asm(" ORW2 %r1, 4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		   } while (--i > 0);
		}
		else
		{
		asm(" MOVW 0x28(%fp),%r0");	/* store mask3 in a reg */
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" MOVW 0x2c(%fp),%ap");	/* store mask4 in a reg */
		do {
			asm(" ROTW %r4,0(%r8),%r1");
			asm(" ANDW3 %r0,%r1,%r2");
			asm(" ORW2 %r2,0(%r7)");
			asm(" ANDW2 %ap,%r1");
			asm(" ORW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		    } while (--i > 0);
		asm(" POPW %ap");	/* restore ap from stack */
		}
		break;
	case F_OR | S_STRADDLE | D_STRADDLE:
		asm(" MOVW 0x20(%fp),%r0");	/* store mask1 in a reg */
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" MOVW 0x24(%fp),%ap");	/* store mask2 in a reg */
		if (a > 16){		/* not super narrow */
		do {
			asm(" ANDW3 %r0,0(%r8),%r1");
			asm(" ANDW3 %ap,4(%r8),%r2");
			asm(" ORW2 %r2,%r1");
			asm(" ROTW %r4,%r1,%r1");
			asm(" ANDW3 0x28(%fp),%r1,%r2");
			asm(" ORW2 %r2,0(%r7)");
			asm(" ANDW2 0x2c(%fp),%r1");
			asm(" ORW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		}
		else
		{
		do {
			asm(" ANDW3 %r0,0(%r8),%r1");
			asm(" ANDW3 %ap,4(%r8),%r2");
			asm(" ORW2 %r2,%r1");
			asm(" ROTW %r4,%r1,%r1");
			asm(" ORH2 %r1,2(%r7)");
			asm(" ANDW2 &0xffff0000,%r1");
			asm(" ORW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		}
		asm(" POPW %ap");	/* restore ap from stack */
		break;
	case F_CLR:
		mask1 = topbits[a+1] >> px31;
		asm(" MOVW 0x20(%fp),%r1");	/* mask1 */
		do {
			asm(" ROTW %r4,0(%r8),%r2");
			asm(" ANDW2 %r1,%r2");
			asm(" MCOMW %r2,%r2");
			asm(" ANDW2 %r2,0(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		break;
	case F_CLR | S_STRADDLE:
		asm(" MOVW 0x20(%fp),%r0");	/* store mask1 in a reg */
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" MOVW 0x24(%fp),%ap");	/* store mask1 in a reg */

		do {
			asm(" ANDW3 %r0,0(%r8),%r2"); /* x20(fp)=mask1 */
			asm(" ANDW3 %ap,4(%r8),%r1"); /* x24(fp)=mask2 */
			asm(" ORW2 %r2,%r1");
			asm(" ROTW %r4,%r1,%r1");
			asm(" MCOMW %r1,%r1");
			asm(" ANDW2 %r1,0(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		asm(" POPW %ap");	/* restore ap from stack */
		break;
	case F_CLR | D_STRADDLE:
		if (a <= 16)			/* very narrow, 17 bits max */
		{
		asm(" MOVW &0xffff,%r2");
		asm(" ORW3 0x28(%fp),0x2c(%fp),%r0");	/* compute mask */
		do {
			asm(" ROTW %r4,0(%r8),%r1");
			asm(" ANDW2 %r0,%r1");	/* mask */
			asm(" MCOMW %r1,%r1");
			asm(" ANDH2 %r1,2(%r7)");
			asm(" ORW2 %r2,%r1");
			asm(" ANDW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		   } while (--i > 0);
		}
		else
		{
		asm(" MOVW 0x28(%fp),%r0");	/* store mask3 in a reg */
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" MOVW 0x2c(%fp),%ap");	/* store mask4 in a reg */
		do {
			asm(" ROTW %r4,0(%r8),%r1");
			asm(" ANDW3 %r0,%r1,%r2");
			asm(" MCOMW %r2,%r2");
			asm(" ANDW2 %r2,0(%r7)");
			asm(" ANDW2 %ap,%r1");
			asm(" MCOMW %r1,%r1");
			asm(" ANDW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		    } while (--i > 0);
		asm(" POPW %ap");	/* restore ap from stack */
		}
		break;
	case F_CLR | S_STRADDLE | D_STRADDLE:
		asm(" MOVW 0x20(%fp),%r0");	/* store mask1 in a reg */
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" MOVW 0x24(%fp),%ap");	/* store mask2 in a reg */
		if (a > 16){		/* not super narrow */
		do {
			asm(" ANDW3 %r0,0(%r8),%r1");
			asm(" ANDW3 %ap,4(%r8),%r2");
			asm(" ORW2 %r2,%r1");
			asm(" ROTW %r4,%r1,%r1");
			asm(" ANDW3 0x28(%fp),%r1,%r2");
			asm(" MCOMW %r2,%r2");
			asm(" ANDW2 %r2,0(%r7)");
			asm(" ANDW2 0x2c(%fp),%r1");
			asm(" MCOMW %r1,%r1");
			asm(" ANDW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		}
		else
		{
		do {
			asm(" ANDW3 %r0,0(%r8),%r1");
			asm(" ANDW3 %ap,4(%r8),%r2");
			asm(" ORW2 %r2,%r1");
			asm(" ROTW %r4,%r1,%r1");
			asm(" MCOMW %r1,%r1");
			asm(" ANDH2 %r1,2(%r7)");
			asm(" ORW2 &0xffff,%r1");
			asm(" ANDW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		}
		asm(" POPW %ap");	/* restore ap from stack */
		break;
	case F_XOR:
		mask1 = topbits[a+1] >> px31;
		asm(" MOVW 0x20(%fp),%r1");	/* mask1 */
		do {
			asm(" ROTW %r4,0(%r8),%r2");
			asm(" ANDW2 %r1,%r2");
			asm(" XORW2 %r2,0(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		break;
	case F_XOR | S_STRADDLE:
		asm(" MOVW 0x20(%fp),%r0");	/* store mask1 in a reg */
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" MOVW 0x24(%fp),%ap");	/* store mask1 in a reg */

		do {
			asm(" ANDW3 %r0,0(%r8),%r2"); /* x20(fp)=mask1 */
			asm(" ANDW3 %ap,4(%r8),%r1"); /* x24(fp)=mask2 */
			asm(" ORW2 %r2,%r1");
			asm(" ROTW %r4,%r1,%r1");
			asm(" XORW2 %r1,0(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		asm(" POPW %ap");	/* restore ap from stack */
		break;
	case F_XOR | D_STRADDLE:
		if (a <= 16)			/* very narrow, 17 bits max */
		{
		asm(" MOVW &0xffff0000,%r2");
		asm(" ORW3 0x28(%fp),0x2c(%fp),%r0");	/* compute mask */
		do {
			asm(" ROTW %r4,0(%r8),%r1");
			asm(" ANDW2 %r0,%r1");	/* mask */
			asm(" XORH2 %r1,2(%r7)");
			asm(" ANDW2 %r2,%r1");
			asm(" XORW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		   } while (--i > 0);
		}
		else
		{
		asm(" MOVW 0x28(%fp),%r0");	/* store mask3 in a reg */
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" MOVW 0x2c(%fp),%ap");	/* store mask4 in a reg */
		do {
			asm(" ROTW %r4,0(%r8),%r1");
			asm(" ANDW3 %r0,%r1,%r2");
			asm(" XORW2 %r2,0(%r7)");
			asm(" ANDW2 %ap,%r1");
			asm(" XORW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		    } while (--i > 0);
		asm(" POPW %ap");	/* restore ap from stack */
		}
		break;
	case F_XOR | S_STRADDLE | D_STRADDLE:
		asm(" MOVW 0x20(%fp),%r0");	/* store mask1 in a reg */
		asm(" PUSHW %ap");	/* store ap on stack */
		asm(" MOVW 0x24(%fp),%ap");	/* store mask2 in a reg */
		if (a > 16){		/* not super narrow */
		do {
			asm(" ANDW3 %r0,0(%r8),%r1");
			asm(" ANDW3 %ap,4(%r8),%r2");
			asm(" ORW2 %r2,%r1");
			asm(" ROTW %r4,%r1,%r1");
			asm(" ANDW3 0x28(%fp),%r1,%r2");
			asm(" XORW2 %r2,0(%r7)");
			asm(" ANDW2 0x2c(%fp),%r1");
			asm(" XORW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		}
		else
		{
		do {
			asm(" ANDW3 %r0,0(%r8),%r1");
			asm(" ANDW3 %ap,4(%r8),%r2");
			asm(" ORW2 %r2,%r1");
			asm(" ROTW %r4,%r1,%r1");
			asm(" XORH2 %r1,2(%r7)");
			asm(" ANDW2 &0xffff0000,%r1");
			asm(" XORW2 %r1,4(%r7)");
			asm(" ADDW2 %r6, %r8");
			asm(" ADDW2 %r5, %r7");
		} while (--i > 0);
		}
		asm(" POPW %ap");	/* restore ap from stack */
		break;
	}
	return;
}
