L=libmj.a
INCL=/usr/jerq/include
CC=3cc
AS=3as
AR=3ar
CFLAGS=

OBJ=arc.o circle.o comm.o cosdeg.o data.o disc.o discture.o elarc.o \
	eldisc.o ellipse.o exit.o fgets.o font.o fopen.o get.o getfont.o \
	popen.o printf.o put.o rdwr.o

CFILES=arc.c circle.c comm.c cosdeg.c data.c disc.c discture.c elarc.c \
	eldisc.c ellipse.c exit.c fgets.c font.c fopen.c get.c getfont.c \
	popen.c printf.c put.c rdwr.c

$L:	$(OBJ)
	$(AR) uv $L $^

lint:
	lint -h -c -u -n -I/usr/jerq/include $(CFILES)

clip.o:	$(INCL)/line.h
comm.o:	$(INCL)/jerqio.h
data.o:	$(INCL)/jerqio.h
defont.o:	$(INCL)/font.h
exit.o:	$(INCL)/jerqio.h
fgets.o:	$(INCL)/jerqio.h
font.o:	$(INCL)/jerqio.h
font.o:	$(INCL)/font.h
fopen.o:	$(INCL)/jerqio.h
get.o:	$(INCL)/jerqio.h
getfont.o:	$(INCL)/font.h
getfont.o:	$(INCL)/jerqio.h
jlineto.o:	$(INCL)/line.h
jstring.o:	$(INCL)/font.h
line.o:	$(INCL)/line.h
menu.o:	$(INCL)/font.h
popen.o:	$(INCL)/jerqio.h
printf.o:	$(INCL)/jerqio.h
put.o:	$(INCL)/jerqio.h
rdwr.o:	$(INCL)/jerqio.h
string.o:	$(INCL)/font.h
strwidth.o:	$(INCL)/font.h
wait.o:	$(INCL)/queue.h

.PRECIOUS:	$L

clean:
	rm -f *.o core
install:	$L
	rm -f /usr/jerq/lib/libmj.a
	ln $L /usr/jerq/lib/libmj.a
