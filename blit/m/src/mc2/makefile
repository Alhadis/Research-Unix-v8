CFLAGS=

SRCS=	c2.h c20.c c21.c
OBJS=	c20.o c21.o

c2: $(OBJS)
	$(CC) -o c2 c20.o c21.o
	@echo DONE

c20.o c21.o: c2.h

sources:$(SRCS)


install:	c2
	mv c2 /usr/blit/lib/mc2

clean:
	rm -f $(OBJS) c2

lint: $(SRCS)
	lint $(SRCS)

print:	$(SRCS)
	pr $? | lpr -c
	touch print
	

