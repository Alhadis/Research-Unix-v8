DIR=/usr/jerq/bin
CC=$(DIR)/mcc
AS=$(DIR)/mas
CFLAGS=-j

ramterm: vitty.o bootram.o load.o
	$(DIR)/mld -o ramterm -b 256 bootram.o load.o vitty.o -lsys -llayer -lj -lc

vitty:	vitty.o
	$(CC) -o vitty -j vitty.o

romterm: vitty.o bootrom.o load.o
	$(DIR)/mld -o romterm -b 262144 -B 153600 bootrom.o load.o vitty.o -lsys -llayer -lj -lc

bootram.o:	bootrom.s
	sed -f romtoram < bootrom.s > goo.s
	$(AS) -o bootram.o goo.s
	rm -f goo.s

testrom:	testrom.o load.o
	$(DIR)/mld -o testrom -b 262144 -B 153600 testrom.o load.o

testprom:	testrom
	cp testrom junk
	mstrip junk
	dd if=junk of=testbits bs=32 skip=1
	push panther testbits /usr/guest/aek/bits
	/usr/games/fortune
	panther "cd /usr/guest/aek/bits; dodbits testbits"

bits:	bits0
bits0: romterm
	strip romterm
	dd if=romterm of=bits0 bs=32 skip=1 count=256
	dd if=romterm of=bits1 bs=32 skip=257 count=256
	dd if=romterm of=bits2 bs=32 skip=513 count=256

prom:	bits0
	push panther bits[012] /usr/guest/aek/bits
	/usr/games/fortune
	panther "cd /usr/guest/aek/bits; dodbits bits0; dodbits bits1; dodbits bits2"

pp:
	pp -kjerq vitty.c | dcan

romterm.nm:	romterm
	mnm romterm > romterm.nm

/usr/jerq/src/mpx/term/romterm.nm:	romterm.nm
	sort +2 romterm.nm -o $>
