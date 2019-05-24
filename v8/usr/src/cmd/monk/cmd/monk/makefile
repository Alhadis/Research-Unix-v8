MHOME = /usr/lib/monk

HEAD = .

CFLAGS = -O -I$(HEAD)
LDFLAGS = -s

FRC =

MONKFILES = \
	db_manage.o \
	envir.o \
	hash.o \
	pr_struct.o \
	read_attrib.o \
	read_cmpress.o \
	read_db.o \
	read_define.o \
	read_user.o \
	regcmp.o \
	regex.o \
	run_monk.o \
	support.o \
	textput.o \
	tokenize.o \
	translate.o \
	warn.o

COMPRESSFILES = \
	cdb_manage.o \
	compress.o \
	hash.o \
	pr_struct.o \
	read_attrib.o \
	read_db.o \
	read_define.o \
	regcmp.o \
	regex.o \
	support.o \
	tokenize.o \
	translate.o \
	warn.o


all:	monk compress

monk:	$(MONKFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o monk $(MONKFILES)

compress:	$(COMPRESSFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o compress $(COMPRESSFILES)

clean:
	-rm -f *.o cdb_manage.c

clobber: clean
	-rm -f monk compress

install: all
	cp monk $(MHOME)
	cp compress $(MHOME)

cdb_manage.c: db_manage.c
	cp db_manage.c cdb_manage.c

cdb_manage.o: \
	$(HEAD)/dbcompress.h \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)
	$(CC) $(CFLAGS) -c -DCOMPRESS cdb_manage.c

compress.o: \
	$(HEAD)/dbcompress.h \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

db_manage.o: \
	$(HEAD)/dbcompress.h \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

edit_user.o: \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(FRC)

envir.o: \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

hash.o: \
	$(HEAD)/hash.h \
	$(HEAD)/search.h \
	$(FRC)

pr_struct.o: \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(FRC)

read_attrib.o: \
	$(HEAD)/dbcompress.h \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

read_cmpress.o: \
	$(HEAD)/dbcompress.h \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

read_db.o: \
	$(HEAD)/dbcompress.h \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

read_define.o: \
	$(HEAD)/dbcompress.h \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

read_user.o: \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

regcmp.o: \
	$(FRC)

regex.o: \
	$(FRC)

run_monk.o: \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

support.o: \
	$(HEAD)/dbcompress.h \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

textput.o: \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

tokenize.o: \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(FRC)

translate.o: \
	$(HEAD)/dbcompress.h \
	$(HEAD)/rd.h \
	$(HEAD)/search.h \
	$(HEAD)/warn.h \
	$(FRC)

warn.o: \
	$(HEAD)/warn.h \
	$(FRC)

FRC:
