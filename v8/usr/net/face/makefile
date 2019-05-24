CFLAGS=-I. -I/usr/include -I/usr/include/sys
SOURCE=server.c daemon.c perm.c start.c sub.c sys.c wait3.s work.c comm.c\
	setup.c face.c inode.c path.c simulate.c common.c
FOBJ=server.o face.o inode.o path.o simulate.o ntdkmgr.o common.o
OBJ=daemon.o nullperm.o sys.o wait3.o start.o sub.o work.o comm.o bcopy.o common.o
COBJ=daemon.O start.O sub.O work.O face.O inode.O path.O simulate.O

all: faced faces

faced:	$(OBJ) profit.o
	$(CC) -o faced $(LDFLAGS) $(OBJ) profit.o -ldk -ljobs

faces:  $(FOBJ) profit.o
	$(CC) -o faces $(LDFLAGS) $(FOBJ) profit.o -ldk -ljobs

cyntax:	$(COBJ)
	cyntax $(COBJ) && touch cyntax

server.o: proto.h server.c
daemon.o: proto.h fserv.h daemon.c
clean:
	rm -f *.o core faced faces clrmount
