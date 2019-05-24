#include <jerq.h>
#include <font.h>
#include <menu.h>

static Bitmap disp = { (Word *)0x700000, 25, 0, 0, XMAX, YMAX };

#define scale(x, inmin, inmax, outmin, outmax)\
	(outmin + muldiv(x-inmin,outmax-outmin,inmax-inmin))

#define bound(x, low, high) min(high, max( low, x ))

#define SPACING		14
#define DISPLAY		16
#define CHARWIDTH	9
#define DELTA		6
#define BARWIDTH	18

static Texture tarrow =
{
	0x0000, 0x0000, 0x0200, 0x0300, 0x01C0, 0x01F0, 0xFFFC, 0xFFFF,
	0xFFFC, 0x01F0, 0x01C0, 0x0300, 0x0200, 0x0000, 0x0000, 0x0000,
};

static NMitem *
tablegen(i, table)
	NMitem *table;
{
	return &table[i];
}

NMitem *mhit(m, but, depth)
register NMenu *m;
{
	register int width, i, j, top, newtop, hit, newhit, items, lines, length;
	Point p, q, savep, baro, barc;
	Rectangle sr, tr, mr;	/* scroll, text, menu */
	Rectangle rside, rhit;
	register Bitmap *b;
	register char *from, *to;
	char fill[64];
	Bitmap *bhelp = 0, *arrow;
	NMitem *(*generator)(), *mi, *table, *ret;

#define sro sr.origin
#define src sr.corner
#define tro tr.origin
#define trc tr.corner
#define mro mr.origin
#define mrc mr.corner

	generator = (table=m->item) ? tablegen : m->generator;
	arrow = balloc(Rect(0, 0, 16, 16));
	texture(arrow, arrow->rect, &tarrow, F_STORE);
	p = mouse.xy;
	for(length = items = 0; (mi=(*generator)(items, table))->text; ++items)
		length = max(length, strlen(mi->text));
	if(items == 0)
		return 0;
	width = length*CHARWIDTH+10;
	sro.x = sro.y = src.x = tro.x = mro.x = mro.y = 0;
	if(items <= DISPLAY)
		lines = items;
	else{
		lines = DISPLAY;
		tro.x = src.x = BARWIDTH;
		sro.x = sro.y = 1;
	}
#define ASCEND 2
	tro.y = ASCEND;
	mrc = trc = add(tro, Pt(width+20, min(items, lines)*SPACING));
	src.y = mrc.y-1;
	newtop = bound(m->prevtop, 0, items-lines);
	p.y -= bound(m->prevhit, 0, lines-1)*SPACING+SPACING/2;
	p.x = bound(p.x-(src.x+width/2), 0, XMAX-mrc.x);
	p.y = bound(p.y, 0, YMAX-mrc.y);
	sr = raddp(sr, p);
	tr = raddp(tr, p);
	mr = raddp(mr, p);
	rside = mr;
	rside.origin.x = rside.corner.x-20;
	b = balloc(mr);
	cursinhibit();
	if(b)
		bitblt(&disp, mr, b, mro, F_STORE);
	rectf(&disp, mr, F_OR);
	cursallow();
PaintMenu:
	cursinhibit();
	rectf(&disp, inset(mr, 1), F_CLR);
	cursallow();
	top = newtop;
	if(items > DISPLAY){
		baro.y = scale(top, 0, items, sro.y, src.y);
		baro.x = sr.origin.x;
		barc.y = scale(top+DISPLAY, 0, items, sro.y, src.y);
		barc.x = sr.corner.x;
		rectf(&disp, Rpt(baro,barc), F_XOR);
	}
	for(p=tro, i=top; i < min(top+lines, items); ++i){
		q = p;
		mi = generator(i, table);
		from = mi->text;
		for(to = &fill[0]; *from; ++from)
			if(*from & 0x80)
				for(j=length-(strlen(from+1)+(to-&fill[0])); j-->0;)
					*to++ = *from & 0x7F;
			else
				*to++ = *from;
		*to = '\0';
		q.x += (width-strwidth(&defont, fill))/2;
		string(&defont, fill, &disp, q, F_XOR);
		if(mi->next)
			bitblt(arrow, arrow->rect, &disp, Pt(trc.x-18, p.y-2), F_OR);
		p.y += SPACING;
	}
	savep = mouse.xy;
	ret=0;
	for(newhit = hit = -1; button(but); nap(2)){
		p = mouse.xy;
		if(depth){
			if(p.x < mro.x){
				hit = -1;
				break;
			}
			if(button(5-but)) break;
		}
		if(ptinrect(p, sr)){
			if(ptinrect(savep,tr)){
				p.y = (baro.y+barc.y)/2;
				cursset(p);
			}
			newtop = scale(p.y, sro.y, src.y, 0, items);
			newtop = bound(newtop-DISPLAY/2, 0, items-DISPLAY);
			if(newtop != top)
/* ->->-> */			goto PaintMenu;
		}else if(ptinrect(savep,sr)){
			register dx, dy;
			if(abs(dx = p.x-savep.x) < DELTA)
				dx = 0;
			if(abs(dy = p.y-savep.y) < DELTA)
				dy = 0;
			if(abs(dy) >= abs(dx))
				dx = 0;
			else
				dy = 0;
			cursset(p = add(savep, Pt(dx,dy)));
		}
		savep = p;
		newhit = -1;
		if(ptinrect(p, tr)){
			newhit = bound((p.y-tro.y)/SPACING, 0, lines-1);
			if(newhit!=hit && hit>=0
			 && abs(tro.y+SPACING*newhit+SPACING/2-p.y) > SPACING/3)
				newhit = hit;
			rhit = tr;
			rhit.origin.y += newhit*SPACING-ASCEND/2;
			rhit.corner.y = rhit.origin.y + SPACING;
		}
		if(newhit != -1)
			mi = (*generator)(top+newhit, table);
		if(newhit == hit)
		{
			if((newhit != -1) && (bhelp == 0) && button1())
				helpon(mi->help, rhit, &bhelp);
			else if(bhelp && !button1())
				helpoff(&bhelp);
		}
		else
		{
			flip(tr, hit);
			helpoff(&bhelp);
			flip(tr, newhit);
			hit = newhit;
			if((newhit != -1) && button1())
				helpon(mi->help, rhit, &bhelp);
		}
		if((newhit != -1) && ptinrect(p, rside))
		{
			if(mi->next){
				if(mi->fn)
					(*mi->fn)();
				ret=mhit(mi->next, but, depth+1);
			}
		}
		if(newhit==0 && top>0){
			newtop = top-1;
			p.y += SPACING;
			cursset(p);
/* ->->-> */		goto PaintMenu;
		}
		if(newhit==DISPLAY-1 && top<items-lines){
			newtop = top+1;
			p.y -= SPACING;
			cursset(p);
/* ->->-> */		goto PaintMenu;
		}
	}
	if(bhelp)
		helpoff(&bhelp);
	if(b){
		cursinhibit();
		screenswap(b, b->rect, b->rect);
		cursallow();
		bfree(b);
	}
	if(hit>=0){
		m->prevhit = hit;
		m->prevtop = top;
		return ret?ret:(*generator)(hit+top, table);
	}else
		return 0;
}

static
flip(r,n)
	Rectangle r;
{
	if(n<0)
		return;
	++r.origin.x;
	r.corner.y = (r.origin.y += SPACING*n-1) + SPACING;
	--r.corner.x;
	rectf(&disp, r, F_XOR);
}

static
helpon(msg, r, bhelp)
	char *msg;
	Rectangle r;
	Bitmap **bhelp;
{
	register Bitmap *b;
	register w;

	w = strwidth(&defont, msg)+10;
	if(r.corner.x+w < XMAX)
	{
		r.origin.x = r.corner.x;
		r.corner.x += w;
	}
	else
	{
		r.corner.x = r.origin.x;
		r.origin.x -= w;
	}
	if(*bhelp = b = balloc(r = inset(r, -1)))
	{
		rectf(b, r, F_OR);
		rectf(b, inset(r, 1), F_XOR);
		string(&defont, msg, b, add(b->rect.origin, Pt(5, 1)), F_XOR);
		screenswap(b, b->rect, b->rect);
	}
}

static
helpoff(bhelp)
	Bitmap **bhelp;
{
	Bitmap *bh;

	if(bh = *bhelp)
	{
		screenswap(bh, bh->rect, bh->rect);
		bfree(bh);
		*bhelp = 0;
	}
}
