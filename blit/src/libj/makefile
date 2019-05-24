L=libj.a
INCL=/usr/jerq/include
CC=mcc
AS=mas
AR=ar
DCAN=dcan
CFLAGS=-O -m

OBJ=$L(atan2.o) $L(cosdeg.o) $L(sqrt.o) $L(exit.o) $L(fgets.o) $L(getfont.o) \
	$L(font.o) $L(fopen.o) $L(gcalloc.o) $L(introutine.o) $L(jarc.o) $L(arc.o) \
	$L(jcircle.o) $L(circle.o) $L(jdisc.o) $L(disc.o) $L(discture.o) \
	$L(jelarc.o) $L(elarc.o) $L(jeldisc.o) $L(eldisc.o) $L(jellipse.o) \
	$L(ellipse.o) $L(jpoint.o) $L(jstring.o) $L(menu.o) $L(balloc.o) \
	$L(min.o) $L(norm.o) $L(point.o) $L(popen.o) $L(printf.o) $L(ptinrect.o) \
	$L(rdwr.o) $L(get.o) $L(put.o) $L(alloc.o) $L(comm.o) $L(data.o) \
	$L(rectXrect.o) $L(rectf.o) $L(jtexture.o) $L(Jrect.o) $L(rectclip.o) \
	$L(rol.o) $L(screenswap.o) $L(string.o) $L(bitblt.o) $L(strwidth.o) \
	$L(defont.o) $L(texture.o) $L(wait.o) $L(jlineto.o) $L(addr.o) \
	$L(masks.o) $L(line.o) $L(arith.o) $L(clip.o) $L(display.o)
FILES=atan2.o cosdeg.o sqrt.o exit.o fgets.o getfont.o \
	font.o fopen.o gcalloc.o introutine.o jarc.o arc.o \
	jcircle.o circle.o jdisc.o disc.o discture.o \
	jelarc.o elarc.o jeldisc.o eldisc.o jellipse.o \
	ellipse.o jpoint.o jstring.o menu.o balloc.o \
	min.o norm.o point.o popen.o printf.o ptinrect.o \
	rdwr.o get.o put.o alloc.o comm.o data.o \
	rectXrect.o rectf.o jtexture.o Jrect.o rectclip.o \
	rol.o screenswap.o string.o bitblt.o strwidth.o \
	defont.o texture.o wait.o jlineto.o addr.o \
	masks.o line.o arith.o clip.o display.o
CFILES= alloc.c arc.c arith.c atan2.c balloc.c\
	bitblt.s circle.c clip.c comm.c cosdeg.c data.c\
	disc.c discture.c display.c ellipse.c exit.c fgets.c\
	fopen.c gcalloc.c get.c introutine.c\
	jarc.c jcircle.c jdisc.c jellipse.c jlineto.c jpoint.c\
	Jrect.c jstring.c jtexture.c line.c masks.c menu.c min.c\
	point.c popen.c printf.c ptinrect.c put.c\
	rdwr.c rectXrect.c rectclip.c rectf.c screenswap.c sqrt.c string.c\
	getfont.c font.c defont.c eldisc.c jeldisc.c\
	strwidth.c texture.s

$L:	$(OBJ)

$L(%.o):	%.s
	$(CC) -c $(CFLAGS) $%.s && $(AR) r $L $%.o && rm -f $%.o
$L(%.o):	%.c $(INCL)/jerq.h
	$(CC) -c $(CFLAGS) $%.c && $(AR) r $L $%.o && rm -f $%.o

new:	$(FILES)
		$(AR) uv $L `mlorder $(FILES) | tsort`
		$(AR) ma clip.o $L line.o
		rm $(FILES)

$L(texture.o):	texture.s
	/lib/cpp -P texture.s > goo.s && \
	$(CC) -c $(CFLAGS) goo.s && $(AR) r $L $%.o && rm -f $%.o goo.s

print:
	@pr $(CFILES)

pp:
	pp -tGraphics -kjerq $(CFILES) | $(DCAN)

lint:
	lint -h -c -u -n -I/usr/jerq/include $(CFILES)

$L(clip.o):	$(INCL)/line.h
$L(comm.o):	$(INCL)/jerqio.h
$L(data.o):	$(INCL)/jerqio.h
$L(defont.o):	$(INCL)/font.h
$L(exit.o):	$(INCL)/jerqio.h
$L(fgets.o):	$(INCL)/jerqio.h
$L(font.o):	$(INCL)/jerqio.h
$L(font.o):	$(INCL)/font.h
$L(fopen.o):	$(INCL)/jerqio.h
$L(get.o):	$(INCL)/jerqio.h
$L(getfont.o):	$(INCL)/font.h
$L(getfont.o):	$(INCL)/jerqio.h
$L(jlineto.o):	$(INCL)/line.h
$L(jstring.o):	$(INCL)/font.h
$L(line.o):	$(INCL)/line.h
$L(menu.o):	$(INCL)/font.h
$L(popen.o):	$(INCL)/jerqio.h
$L(printf.o):	$(INCL)/jerqio.h
$L(put.o):	$(INCL)/jerqio.h
$L(rdwr.o):	$(INCL)/jerqio.h
$L(string.o):	$(INCL)/font.h
$L(strwidth.o):	$(INCL)/font.h
$L(wait.o):	$(INCL)/queue.h

.PRECIOUS:	$L

clean:
	rm -f core *.o
install:	$L
	rm -f /usr/blit/lib/$L
	cp $L /usr/blit/lib
