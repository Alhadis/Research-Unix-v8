CFLAGS = -g
YFLAGS = -d

SOURCE = eqn.y e.h main.c diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
 input.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
 pile.c shift.c size.c sqrt.c text.c

FILES =  main.o diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
 input.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
 pile.o shift.o size.o sqrt.o text.o eqn.o

a.out:	$(FILES)
	cc $(CFLAGS) $(FILES) 

y.tab.h:	eqn.o

e.def:	y.tab.h
	-cmp -s y.tab.h e.def || cp y.tab.h e.def

$(FILES):	e.h e.def

prcan:	NEW $(SOURCE) makefile
	@prcan $?
	@touch prcan

bundle:
	@bundle $(SOURCE) makefile NEW

backup:	NEW $(SOURCE) makefile eqntest.a
	push snb $? eqn
	@touch backup

bowell:	NEW $(SOURCE) makefile eqntest.a
	push bowell $? /src/cmd/eqn
	@touch bowell

install:	a.out
	strip a.out
	cp a.out /usr/bin/eqn

lint:	y.tab.c
	lint -spb *.c

clean:
	rm -f a.out *.o make.out y.tab.h e.def
