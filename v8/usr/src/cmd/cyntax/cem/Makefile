CFLAGS	= -O
BIN	= /usr/bin
LIB	= /usr/lib/cyntax

OBS	=\
	diag.o\
	equiv.o\
	fix.o\
	main.o\
	misc.o\
	read.o\
	string.o\
	symbol.o\
	table.o\
	time.o\
	type.o\
	var.o

all	: cem cyntax

cem	: $(OBS)
	$(CC) $(LFLAGS) $(OBS) -o cem

diag.o	: cem.h obj_version.h stdobj.h types.h symbol.h type.h
equiv.o	: cem.h obj_version.h stdobj.h types.h symbol.h type.h var.h
fix.o	: cem.h obj_version.h stdobj.h symbol.h type.h
main.o	: cem.h
misc.o	: cem.h
read.o	: cem.h obj_version.h stdobj.h
string.o: cem.h
symbol.o: cem.h obj_version.h symbol.h
table.o	: cem.h obj_version.h stdobj.h symbol.h type.h
time.o	: cem.h
type.o	: cem.h obj_version.h stdobj.h symbol.h type.h
var.o	: cem.h obj_version.h stdobj.h types.h equiv.h symbol.h type.h var.h

clean	:
	-rm -f $(OBS) cem cyntax

install	: $(BIN)/cyntax $(LIB)/cem

$(BIN)/cyntax: cyntax
	cp cyntax /usr/bin/cyntax

$(LIB)/cem: cem
	cp cem /usr/lib/cyntax/cem
