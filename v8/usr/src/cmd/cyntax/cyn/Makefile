LIB	= /usr/lib/cyntax
CFLAGS	= -O

OBS	=\
	c00.o\
	c01.o\
	c02.o\
	c03.o\
	c04.o\
	c05.o\
	c06.o\
	c07.o\
	c08.o\
	c09.o\
	c10.o\
	c11.o\
	c12.o\
	c13.o\
	c14.o\
	c15.o\
	c16.o\
	c17.o\
	c18.o\
	c19.o\
	c20.o\
	c21.o\
	c22.o\
	c23.o\
	c24.o\
	c25.o\
	c26.o\
	c27.o\
	c28.o\
	c29.o\
	c30.o\
	c31.o\
	c32.o\
	c33.o\
	c34.o\
	c35.o\
	c36.o\
	c37.o\
	c38.o\
	c39.o\
	c40.o

ccom	: $(OBS)
	$(CC) -o ccom $(LFLAGS) $(OBS)

install	: $(LIB)/ccom

$(LIB)/ccom: ccom
	cp ccom $(LIB)/ccom

clean	:
	-rm -f $(OBS) ccom
