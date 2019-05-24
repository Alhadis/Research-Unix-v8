L=layer.a
CC=mcc
INCL=/usr/jerq/include
DCAN=dcan
SOURCE=dellayer.c layerop.c lbitblt.c lblt.c lpoint.c lrectf.c lscroll.c lsegment.c ltexture.c newlayer.c upfront.c lbox.c
OBJ=$L(layerop.o) $L(lbitblt.o) $L(lblt.o) $L(lrectf.o) $L(dellayer.o)\
	$L(lscroll.o) $L(lpoint.o) $L(ltexture.o) $L(lsegment.o) $L(newlayer.o)\
	$L(upfront.o) $L(lbox.o) 
CFLAGS=-O -m

$L: $(OBJ)

$L(%.o):	%.c $(INCL)/layer.h $(INCL)/jerq.h
	$(CC) -c $(CFLAGS) $%.c && ar r $L $%.o && rm -f $%.o

lint:
	lint $(SOURCE)

print:
	@pr layer.h $(SOURCE)

pp:
	pp -tLayer -kjerq layer.h $(SOURCE) | $(DCAN)

newlayer.a: $(OBJ)
	ar cr layer.a `mlorder $(OBJ) | tsort`

clean:
	rm -f core *.o

