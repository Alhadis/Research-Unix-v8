L=libmj.a
INCL=/usr/jerq/include
CC=mcc
AS=mas
AR=ar
CFLAGS=-O

OBJ=$L(arc.o) $L(cosdeg.o) $L(disc.o) $L(discture.o) $L(exit.o) $L(fgets.o) \
	$L(getfont.o) $L(font.o) $L(fopen.o) $L(jarc.o) $L(jcircle.o) \
	$L(circle.o) $L(jdisc.o) $L(jelarc.o) $L(elarc.o) $L(jeldisc.o) \
	$L(eldisc.o) $L(jellipse.o) $L(ellipse.o) $L(popen.o) \
	$L(printf.o) $L(rdwr.o) $L(get.o) $L(put.o) $L(comm.o) $L(data.o)  
FILES=arc.o cosdeg.o disc.o discture.o exit.o fgets.o getfont.o \
	font.o fopen.o jarc.o jcircle.o circle.o jdisc.o \
	jelarc.o elarc.o jeldisc.o eldisc.o jellipse.o ellipse.o \
	popen.o printf.o rdwr.o get.o put.o comm.o data.o  
CFILES=arc.c circle.c comm.c cosdeg.c data.c disc.c discture.c\
	elarc.c eldisc.c ellipse.c exit.c fgets.c font.c \
	fopen.c get.c getfont.c jarc.c jcircle.c jdisc.c \
	jelarc.c jeldisc.c jellipse.c popen.c printf.c put.c rdwr.c  

$L:	$(OBJ)

$L(%.o):	%.s
	$(CC) -c $(CFLAGS) $%.s && $(AR) r $L $%.o && rm -f $%.o
$L(%.o):	%.c $(INCL)/jerq.h
	$(CC) -c $(CFLAGS) $%.c && $(AR) r $L $%.o && rm -f $%.o

new:	$(FILES)
		$(AR) uv libj.a `mlorder $(FILES) | tsort`
		$(AR) ma clip.o libj.a line.o

print:
	@pr $(CFILES)

pp:
	pp -tGraphics -kjerq $(CFILES) | dcan

lint:
	lint -h -c -u -n -I/usr/jerq/include $(CFILES)
$L(clip.o):	line.h
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
$L(jlineto.o):	line.h
$L(jstring.o):	$(INCL)/font.h
$L(line.o):	line.h
$L(menu.o):	$(INCL)/font.h
$L(popen.o):	$(INCL)/jerqio.h
$L(printf.o):	$(INCL)/jerqio.h
$L(put.o):	$(INCL)/jerqio.h
$L(rdwr.o):	$(INCL)/jerqio.h
$L(string.o):	$(INCL)/font.h
$L(strwidth.o):	$(INCL)/font.h
$L(wait.o):	$(INCL)/queue.h

clean:
	rm -f *.o $L
install:	$L
	cp $L /usr/blit/lib
