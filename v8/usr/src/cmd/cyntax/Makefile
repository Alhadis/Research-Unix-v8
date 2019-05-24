BIN	= /usr/bin
LIB	= /usr/lib/cyntax

all	:
	cd cyn; make
	cd cem; make
	cd lib; make

install	:
	cd cyn; make install BIN=$(BIN) LIB=$(LIB)
	cd cem; make install BIN=$(BIN) LIB=$(LIB)
	cd lib; make install BIN=$(BIN) LIB=$(LIB)

clean	:
	cd cyn; make clean
	cd cem; make clean
	cd lib; make clean
