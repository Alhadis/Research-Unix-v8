CC68=/usr/local/cc68
LD68=/usr/local/ld68
LD68FLAGS= -T 3fb000 -N -e _main
SYMBOLS125=/usr/lib/m68k/bgsym_1.25
CC68FLAGS=-I/usr/rlb/m68k/bitgraph/bg1.25

all:	mkfont24 mkfont40 rot fonts rl

mkfont24:	mkfont24.o
	cc mkfont24.o -o mkfont24

mkfont40:	mkfont40.o
	cc mkfont40.o -o mkfont40

rot:	rot.o
	cc rot.o -o rot

rl:	rl.o
	cc rl.o -o rl
#-------------------------------------------------------------
# make fonts option - remakes ../font.c68 if anything changed
#
FONTS=font24_a font24_b font24_c font24_d font24_e font24_f font24_g

fonts:	$(FONTS) mkfont24 mkfont40 mk_fonts
	mk_fonts > ../fonts.c68

font24_c \
font24_e \
font24_f:	font24_d mk_corners
	mk_corners

#-------------------------------------------------------------
clean:
	-rm -f *.o *.o20 *.o125 core *.out .,* ,* mkfont24 mkfont40

.SUFFIXES:

.SUFFIXES:  .c68 .o20 .o125 .O125 .c .o

.c68.o20:
	-ln $*.c68 $*.c
	$(CC68) -DV2_0 -S $(CC68FLAGS) $*.c
	-rm -fn $*.c
	sed -e 's/\\"/\\042/g' -e 's/\\\\/\\134/g' $*.s > /tmp/$*.s
	mv /tmp/$*.s .
	as68 -o $*.o20 $*.s
	-rm -fn $*.s

.c68.o125:
	-ln $*.c68 $*.c
	$(CC68) -DV1_25 -S $(CC68FLAGS) $*.c
	-rm -fn $*.c
	sed -e 's/\\"/\\042/g' -e 's/\\\\/\\134/g' $*.s > /tmp/$*.s
	mv /tmp/$*.s .
	as68 -o $*.o125 $*.s
	-rm -fn $*.s

.c68.O125:
	-ln $*.c68 $*.c
	$(CC68) -O -DV1_25 -S $(CC68FLAGS) $*.c
	-rm -fn $*.c
	sed -e 's/\\"/\\042/g' -e 's/\\\\/\\134/g' $*.s > /tmp/$*.s
	mv /tmp/$*.s .
	as68 -o $*.O125 $*.s
	-rm -fn $*.s

.c.o:
	$(CC) -c $(CFLAGS) $*.c
