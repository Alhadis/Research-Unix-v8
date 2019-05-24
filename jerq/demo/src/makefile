BIN=/usr/jerq/bin
CC=$(BIN)/3cc
AS=$(BIN)/3as
DIRS=asteroids centipede fence swar

nothing:
	@echo "make install or clean or mux or stand-alone"

mux:	EWD a asteroids/ asteroids/a.out b ball bltdemo bounce centipede/\
	centipede/a.out\
	clock disc fence/ fence/a.out juggle lunch m magnet maxwell moire\
	pogo road rose runch swar/ swar/a.out tracks weird

%/a.out:
	cd $%; make
#	this silliness to get around a bug in make
%/:	/dev/tty
	echo $%
%/cp:	%/a.out
	cp $%/a.out ../mux/$%
%/clean:
	cd $%; make clean

clean:
	rm -f *.o EWD a b ball bltdemo bounce \
	  clock disc juggle lunch m magnet maxwell moire\
	  pogo road rose runch tracks weird
	for i in ${DIRS}; do (cd $$i; make clean); done

stand-alone:

cp:	asteroids/cp centipede/cp fence/cp swar/cp
install:	mux stand-alone cp
	cp EWD a  b ball bltdemo bounce clock disc\
	  juggle lunch m magnet maxwell moire pogo\
	  road rose runch tracks weird ../mux
