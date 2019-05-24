CC=mcc
AS=mas
DIRS=asteroids kbd pacman swar

nothing:
	@echo "make install or clean or mpx or stand-alone"

mpx:	EWD a asteroids/ asteroids/a.out b ball bounce \
	clock disc juggle kbd/ kbd/a.out m maxwell\
	nball pacman/ pacman/a.out rae road spheres swar/ swar/a.out weird

%/a.out:
	cd $%; make
#	this silliness to get around a bug in make
%/:	/dev/tty
	echo $%
%/cp:	%/a.out
	cp $%/a.out ../mpx/$%
%/clean:
	cd $%; make clean
%ball:	%ball.c norm.o
	mcc $%ball.c norm.o && mopt a.out && mv m.out $%ball
%:	%.c
	mcc $%.c && mopt a.out && mv m.out $%

clean:
	rm -f a.out m.out core *.o EWD a b ball bounce \
	  clock disc juggle m maxwell nball rae road spheres weird
	for i in ${DIRS}; do (cd $$i; make clean); done

stand-alone:

cp:	asteroids/cp kbd/cp pacman/cp swar/cp
install:	mpx stand-alone cp
	cp EWD a b ball bounce clock disc juggle\
	   m maxwell nball rae road spheres weird\
	   ../mpx
