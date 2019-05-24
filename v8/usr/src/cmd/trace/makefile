CFLAGS =
OBJS = malloc_t.o cmalloc.o trace.expr.o trace1.o trace2.o trace3.o trace4.o trace5.o trace6.o trace7.o trace8.o trace9.o
DEST = /usr/bin

#	FILES:
#		trace.h		constants and macro definitions
#		trace.d		data structures
#		trace.expr.c	expressions
#		trace1.c	reads in the tables, sets up data structures
#		trace2.c	memory allocation	
#		trace3.c	the symbolic execution procedures
#				file "assert.c" is included in trace3.c
#		trace4.c	state space handler
#		trace5.c	hashed index for previously seen states
#		trace6.c	computes hash values
#		trace7.c	lookup table of queue state templates
#		trace8.c	2nd order lookup tables
#		trace9.c	lookup table of leaf-states
#		malloc_t.c	memory allocation interface

trace:	$(OBJS) trace.d trace.h
	$(CC) $(CFLAGS) $(OBJS) -o trace

trace3.o:	trace3.c assert.c

pp:
	pp -fPO makefile *.[cdh] | dcan

wc:
	wc  *.[cdh]

install:
	strip trace
	cp trace ${DEST}/trace

clean:
	rm -f *.o pret.out mon.out pret.tmp pret.err core a.out trace
