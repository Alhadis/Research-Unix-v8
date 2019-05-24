cfront:
	yacc gram.y
	CC -o cfront alloc.c expr.c norm2.c expr2.c print.c dcl.c repr.c typ.c dcl2.c lalex.c simpl.c typ2.c del.c lex.c size.c error.c main.c y.tab.c expand.c norm.c table.c


clean:
	rm -f *.[oi] munch.[oi] cfront y.tab.c

install:	cfront
	cp cfront /usr/bin
	strip /usr/bin/cfront
