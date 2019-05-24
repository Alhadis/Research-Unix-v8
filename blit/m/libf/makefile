OBJ = addmag.o ecvt.o fladd.o flcmp.o fldiv.o flmul.o flneg.o \
	flsub.o frexp.o fsqrt.o ftol.o gcvt.o itof.o ldexp.o \
	ltof.o modf.o printf.o submag.o uitof.o ultof.o

libf.a:	$(OBJ)
	rm -f libf.a
	ar rc libf.a `mlorder $(OBJ) | tsort`

install: libf.a
	cp libf.a /usr/jerq/lib

$(OBJ):	fp.h

clean:
	rm -f *.o libf.a

.c.o:
	mcc -S $<
	qed -i -x massage.qed $*.s
	mcc -c $*.s
	rm $*.s
