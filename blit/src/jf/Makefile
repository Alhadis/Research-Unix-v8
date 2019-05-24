.SUFFIXES: .c .o
CC = mcc
CFLAGS =

HEADERS = jf.h
JFSRC = main.c spaceman.c charsubs.c fontsubs.c filesubs.c magnify.c
UTLSRC = buttons.c kbdstring.c strcat2.c brotate.c rsupport.c \
	menuhit3.c lexit3.c mice.c reshaped.c
JFOBJ = main.o spaceman.o charsubs.o fontsubs.o filesubs.o magnify.o
UTLOBJ = buttons.o kbdstring.o strcat2.o brotate.o rsupport.o \
	menuhit3.o lexit3.o mice.o reshaped.o

jf: jf.m
jf.m: $(JFOBJ) $(UTLOBJ)
	mcc -g $(JFOBJ) $(UTLOBJ) 
	mopt a.out
	mv m.out jf.m
	rm a.out

$(JFOBJ): $(HEADERS)

PFOBJ = pfont.o buttons.o mice.o
pfont.m: $(PFOBJ)
	mcc -g $(PFOBJ) -o pfont.m

list:
	@pr $(HEADERS) $(JFSRC) pfont.c

man:
	@troff -man jf.9

clean:
	rm -f $(JFOBJ) $(UTLOBJ) pfont.o jf.m pfont.m

install:	jf
	cp jf.m /usr/blit/mbin
