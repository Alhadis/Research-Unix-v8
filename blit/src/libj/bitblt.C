#include <jerq.h>
#define LEFTDIR	8
#define NOSHIFT 4
#define DAMMIT	4		/* you'll see why */
#undef	sw

bitblt(sm,r,dm,p,fc)
Bitmap *sm,*dm;
Rectangle r;
Point p;
int fc;
{
	register Word *source,*dest,*_source,*_dest;	/* %a2-%a5 */
	register UWord m,mask1,mask2;		/* %d2,%d3,%d4 */
	register int a,b,i;			/* %d5,%d6,%d7 */

	int j,h,w,dx1,sw,dw;

	/* clip to the destination Bitmap only */
#define	rp	dest
#define pp	source
	rp = (int *) &(dm->rect);
	pp = (int *) &p;
	if ((a = *rp++ - *pp++) > 0) {
		*(pp-1) += a;
		r.origin.x += a;
	}
	if ((a = *rp++ - *pp) > 0) {
		*pp += a;
		r.origin.y += a;
	}
	if ((a = r.origin.x + *rp++ - *(pp-1)) < r.corner.x)
		r.corner.x = a;
	if ((a = r.origin.y + *rp - *pp) < r.corner.y)
		r.corner.y = a;
	i = r.corner.y - r.origin.y;	/* going to be h */
	a = r.corner.x - r.origin.x - 1;	/* going to be dx1 */
	if (i <= 0 || a < 0)
		return;
	if (a < 16)
		goto narrow;
	h = i; 
	dx1 = a;		/* i and b are regs, avoid work! */
	sw = sm->width << 1;	/* sleazy hack to avoid shift */
	dw = dm->width << 1;	/* in outer, inner loops */
	w = ((p.x+dx1) >> 4) - (p.x >> 4) - 1;	/* inner loop */
	mask1 = ~topbits[p.x & 15];
	mask2 = topbits[((p.x+dx1) & 15) + 1];
	if (sm == dm) {		/* may have to mess with loop order */
		if (r.origin.y < p.y) {		/* swap top with bottom */
			r.origin.y += h-1;
			p.y += h-1;
			sw = -sw;
			dw = -dw;
		}
		if (r.origin.x < p.x) {	/* swap left with right */
			fc |= LEFTDIR;
			r.origin.x += dx1;
			p.x += dx1;
		}
	}
	_dest = addr(dm,p);
	_source = addr(sm,r.origin);
	a = (p.x&15) - (r.origin.x&15);
	if (a < 0)
		a += 16;
	else	/* a == 0 means no shift, remember that */
		_source--;	/* else grab long and shift right */
	b = 16 - a;
	if (a == 0)
		fc |= NOSHIFT;
	source = _source;
	dest = _dest;
	switch (fc) {
	case F_STORE | NOSHIFT:
		b = w;
		_source++;
		source = _source;
		a = h;		/* a is free => use it */
		do {
			*dest++ = (~mask1 & *dest) | (mask1 & *source++);
			if ((i = b>>2) > 0) do {
				*((long *)dest)++ = *((long *)source)++;
				*((long *)dest)++ = *((long *)source)++;
			} while (--i > 0);
			if ((i = b&3) > 0) do {
				*dest++ = *source++;
			} while (--i > 0);
			*dest = (~mask2 & *dest) | (mask2 & *source);
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--a > 0);
		break;
	case F_STORE:
		do {
asm("			mov.l	(%a2)+,%d2	# (long) m = *source++");
asm("			ror.l	%d5,%d2		# rotate m right by a");
			*dest++ = (~mask1 & *dest) | (mask1 & m);
asm("			ror.l	%d6,%d2		# rotate m right by b");
			if ((i = w) > 0) do {
				m = *source++;
asm("				ror.l	%d5,%d2 	# m >> a");
				*dest++ = m;
asm("				ror.l	%d6,%d2		# m >> b");
			} while (--i > 0);
			m = *source;
asm("			ror.l	%d5,%d2		# m >> a");
			*dest = (~mask2 & *dest) | (mask2 & m);
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--h > 0);
		break;
	case F_STORE | NOSHIFT | LEFTDIR:
		b = w;
		_source++;
		source = _source;
		a = h;
		do {
			*dest = (~mask2 & *dest) | (mask2 & *source);
			if ((i = b>>2) > 0) do {
				*--((long *)dest) = *--((long *)source);
				*--((long *)dest) = *--((long *)source);
			} while (--i > 0);
			if ((i = b&3) > 0) do {
				*(--dest) = *(--source);
			} while (--i > 0);
			dest--;
			*dest = (~mask1 & *dest) | (mask1 & *(--source));
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--a > 0);
		break;
	case F_STORE | LEFTDIR:
		do {
asm("			mov.l	(%a2),%d2	# (long) m = *source");
asm("			ror.l	%d5,%d2		# m >> a");
			*dest = (~mask2 & *dest) | (mask2 & m);
asm("			rol.l	%d5,%d2		# m << a");
			if ((i = w) > 0) do {
				m = *(--source);
asm("				rol.l	%d6,%d2		# m << b");
				*(--dest) = m;
asm("				rol.l	%d5,%d2		# m << a");
			} while (--i > 0);
			m = *(--source);
asm("			rol.l	%d6,%d2		# m << b");
			dest--;
			*dest = (~mask1 & *dest) | (mask1 & m);
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--h > 0);
		break;
	case F_OR:
	case F_OR | NOSHIFT:
		do {
asm("			mov.l	(%a2)+,%d2	# (long) m = *source++");
asm("			ror.l	%d5,%d2		# rotate m right by a");
			*dest++ |= (mask1 & m);
asm("			ror.l	%d6,%d2		# rotate m right by b");
			if ((i = w) > 0) do {
				m = *source++;
asm("				ror.l	%d5,%d2 	# m >> a");
				*dest++ |= m;
asm("				ror.l	%d6,%d2		# m >> b");
			} while (--i > 0);
			m = *source;
asm("			ror.l	%d5,%d2		# m >> a");
			*dest |= (mask2 & m);
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--h > 0);
		break;
	case F_OR | LEFTDIR:
	case F_OR | NOSHIFT | LEFTDIR:
		do {
asm("			mov.l	(%a2),%d2	# (long) m = *source");
asm("			ror.l	%d5,%d2		# m >> a");
			*dest |= (mask2 & m);
asm("			rol.l	%d5,%d2		# m << a");
			if ((i = w) > 0) do {
				m = *(--source);
asm("				rol.l	%d6,%d2		# m << b");
				*(--dest) |= m;
asm("				rol.l	%d5,%d2		# m << a");
			} while (--i > 0);
			m = *(--source);
asm("			rol.l	%d6,%d2		# m << b");
			dest--;
			*dest |= (mask1 & m);
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--h > 0);
		break;
	case F_CLR:
	case F_CLR | NOSHIFT:
		do {
asm("			mov.l	(%a2)+,%d2	# (long) m = *source++");
asm("			ror.l	%d5,%d2		# rotate m right by a");
			*dest++ &= ~(mask1 & m);
asm("			ror.l	%d6,%d2		# rotate m right by b");
			if ((i = w) > 0) do {
				m = *source++;
asm("				ror.l	%d5,%d2 	# m >> a");
asm("				not.w	%d2		# m = ~m");
				*dest++ &= m;
asm("				ror.l	%d6,%d2		# m >> b");
			} while (--i > 0);
			m = *source;
asm("			ror.l	%d5,%d2		# m >> a");
			*dest &= ~(mask2 & m);
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--h > 0);
		break;
	case F_CLR | LEFTDIR:
	case F_CLR | NOSHIFT | LEFTDIR:
		do {
asm("			mov.l	(%a2),%d2	# (long) m = *source");
asm("			ror.l	%d5,%d2		# m >> a");
			*dest &= ~(mask2 & m);
asm("			rol.l	%d5,%d2		# m << a");
			if ((i = w) > 0) do {
				m = *(--source);
asm("				rol.l	%d6,%d2		# m << b");
asm("				not.w	%d2		# m = ~m");
				*(--dest) &= m;
asm("				rol.l	%d5,%d2		# m << a");
			} while (--i > 0);
			m = *(--source);
asm("			rol.l	%d6,%d2		# m << b");
			dest--;
			*dest &= ~(mask1 & m);
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--h > 0);
		break;
	case F_XOR | NOSHIFT:
		b = w;
		_source++;
		source = _source;
		a = h;
		do {
			*dest++ ^= (mask1 & *source++);
			if ((i = b>>2) > 0) do {
				*((long *)dest)++ ^= *((long *)source)++;
				*((long *)dest)++ ^= *((long *)source)++;
			} while (--i > 0);
			if ((i = b&3) > 0) do {
				*dest++ ^= *source++;
			} while (--i > 0);
			*dest ^= (mask2 & *source);
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--a > 0);
		break;
	case F_XOR:
	default:
		do {
asm("			mov.l	(%a2)+,%d2	# (long) m = *source++");
asm("			ror.l	%d5,%d2		# rotate m right by a");
			*dest++ ^= (mask1 & m);
asm("			ror.l	%d6,%d2		# rotate m right by b");
			if ((i = w) > 0) do {
				m = *source++;
asm("				ror.l	%d5,%d2 	# m >> a");
				*dest++ ^= m;
asm("				ror.l	%d6,%d2		# m >> b");
			} while (--i > 0);
			m = *source;
asm("			ror.l	%d5,%d2		# m >> a");
			*dest ^= (mask2 & m);
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--h > 0);
		break;
	case F_XOR | NOSHIFT | LEFTDIR:
		b = w;
		_source++;
		source = _source;
		a = h;
		do {
			*dest ^= (mask2 & *source);
			if ((i = b>>2) > 0) do {
				*--((long *)dest) ^= *--((long *)source);
				*--((long *)dest) ^= *--((long *)source);
			} while (--i > 0);
			if ((i = b&3) > 0) do {
				*(--dest) ^= *(--source);
			} while (--i > 0);
			dest--;
			*dest ^= (mask1 & *(--source));
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--a > 0);
		break;
	case F_XOR | LEFTDIR:
		do {
asm("			mov.l	(%a2),%d2	# (long) m = *source");
asm("			ror.l	%d5,%d2		# m >> a");
			*dest ^= (mask2 & m);
asm("			rol.l	%d5,%d2		# m << a");
			if ((i = w) > 0) do {
				m = *(--source);
asm("				rol.l	%d6,%d2		# m << b");
				*(--dest) ^= m;
asm("				rol.l	%d5,%d2		# m << a");
			} while (--i > 0);
			m = *(--source);
asm("			rol.l	%d6,%d2		# m << b");
			dest--;
			*dest ^= (mask1 & m);
			(char *) _source += sw;
			source = _source;
			(char *) _dest += dw;
			dest = _dest;
		} while (--h > 0);
		break;
	}
	return;
narrow:
	/*
	 * width is 16 bits or less, so we can do it by reading and
	 * writing 32 bits at a time
	 */
	_source = (Word *) sm;
	_dest = (Word *) dm;
	mask1 = ((Bitmap *) _source)->width;	/* source increment */
	mask1 <<= 1;		/* hack to add to an address register */
	mask2 = ((Bitmap *) _dest)->width;	/* dest increment */
	mask2 <<= 1;
	if (_source == _dest && r.origin.y < p.y) {	/* swap top with bottom */
		r.origin.y += i-1;
		p.y += i-1;
		mask1 = -mask1;
		mask2 = -mask2;
	}
asm("	mov	&0,%d6		# (long) b = 0");
	b = topbits[a+1];
	a = (16 - (p.x & 15));	/* hocus pocus to get long mask */
asm("	rol.l	%d5,%d6		# (long) b <<= a");
	a = 16 - a - (r.origin.x & 15);	/* shift constant */
	if (a < 0) {		/* guess what! -1 == 63 to the 68000!!! */
		fc |= DAMMIT;	/* not fatal, just slow */
	}
	source = addr(_source,r.origin);
	dest = addr(_dest,p);
	switch (fc) {
	case F_STORE:
	case F_STORE | DAMMIT:
asm("		mov.l	%d6,%d1		# prepare inverse mask");
asm("		not.l	%d1		");
		do {
asm("			mov.l	(%a2),%d2	# m = *source");
asm("			ror.l	%d5,%d2		# rotate m right by a");
asm("			and.l	%d6,%d2		# m &= b");
asm("			mov.l	(%a3),%d0	# m |= *dest&~b");
asm("			and.l	%d1,%d0		");
asm("			or.l	%d0,%d2		");
asm("			mov.l	%d2,(%a3)	# *dest = m");
			(char *) source += (int) mask1;
			(char *) dest += (int) mask2;
		} while (--i > 0);
		break;
	case F_OR:
	case F_OR | DAMMIT:
		do {
asm("			mov.l	(%a2),%d2	# m = *source");
asm("			ror.l	%d5,%d2		# rotate m right by a");
asm("			and.l	%d6,%d2		# m &= b");
asm("			or.l	%d2,(%a3)	# *dest |= m");
			(char *) source += (int) mask1;
			(char *) dest += (int) mask2;
		} while (--i > 0);
		break;
	case F_CLR:
	case F_CLR | DAMMIT:
		do {
asm("			mov.l	(%a2),%d2	# m = *source");
asm("			ror.l	%d5,%d2		# rotate m right by a");
asm("			and.l	%d6,%d2		# m &= b");
asm("			not.l	%d2		# m = ^m");
asm("			and.l	%d2,(%a3)	# *dest &= m");
			(char *) source += (int) mask1;
			(char *) dest += (int) mask2;
		} while (--i > 0);
		break;
	case F_XOR:
		do {
asm("			mov.l	(%a2),%d2	# m = *source");
asm("			ror.l	%d5,%d2		# rotate m right by a");
asm("			and.l	%d6,%d2		# m &= b");
asm("			eor.l	%d2,(%a3)	# *dest ^= m");
			(char *) source += (int) mask1;
			(char *) dest += (int) mask2;
		} while (--i > 0);
		break;
	case F_XOR | DAMMIT:
		a = -a;
		do {
asm("			mov.l	(%a2),%d2	# m = *source");
asm("			rol.l	%d5,%d2		# rotate m left by a");
asm("			and.l	%d6,%d2		# m &= b");
asm("			eor.l	%d2,(%a3)	# *dest ^= m");
			(char *) source += (int) mask1;
			(char *) dest += (int) mask2;
		} while (--i > 0);
		break;
	}
	return;
}
