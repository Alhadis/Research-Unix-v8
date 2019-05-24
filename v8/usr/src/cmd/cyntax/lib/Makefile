LIB		= /usr/lib/cyntax
CCOM		= $(LIB)/ccom
CEM		= $(LIB)/cem

all		: libc libj

libj		: llib-lj $(CCOM) $(CEM)
		/lib/cpp -M -I/usr/jerq/include llib-lj | $(CCOM) -llibj - out && $(CEM) -p out && mv out libj

lib%		: llib-l% var% $(CCOM) $(CEM)
		/lib/cpp -M llib-l$% | $(CCOM) `cat var$%` -llib$% - out && $(CEM) -p out && mv out lib$%

install		: $(LIB)/libc $(LIB)/libj

$(LIB)/lib%	: lib%
		cp lib$% $(LIB)/lib$%

clean		:
		-rm -f libc libj out
