L=libsys.a
INCL=/usr/jerq/include
CC=mcc
AS=mas
AR=ar
CFLAGS=-O -m

OBJ=$L(jinit.o) $L(cursor.o) $L(buttons.o) $L(kbd.o) \
	$L(acia.o) $L(queue.o) $L(resume.o) $L(wait.o)
FILES=jinit.o cursor.o buttons.o kbd.o acia.o queue.o resume.o wait.o
CFILES=acia.c buttons.c cursor.c jinit.c kbd.c queue.c resume.s wait.c

$L:	$(OBJ)

$L(%.o):	%.s $(INCL)/jerq.h
	$(CC) -c $(CFLAGS) $%.s && $(AR) r $L $%.o && rm -f $%.o
$L(%.o):	%.c $(INCL)/jerq.h
	$(CC) -c $(CFLAGS) $%.c && $(AR) r $L $%.o && rm -f $%.o

new:	$(FILES)
		$(AR) uv libj.a `mlorder $(FILES) | tsort`
		$(AR) ma clip.o libj.a line.o

print:
	@pr $(CFILES)

pp:
	pp -tSystem -kjerq $(CFILES) | dcan

lint:
	lint -h -c -u -n -I/usr/jerq/include $(CFILES)
