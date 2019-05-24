CFLAGS = -Od2

OBJ = fmt.o head.o

fmt:	$(OBJ)
	cc -o fmt $(OBJ)

install:	fmt
	strip fmt
	mv fmt /usr/bin

clean:
	rm -f *.o fmt
