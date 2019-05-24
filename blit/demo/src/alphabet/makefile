CFILES=bounce1.c bounce2.c bounce3.c circle.c circle2.c discs.c frac.c grandmal.c kal.c lineslide.c localop.c main.c munch.c rest.c ripple.c schwartz.c spheres.c tetra.c wallpaper.c worms.c
OFILES=bounce1.o bounce2.o bounce3.o circle.o circle2.o discs.o frac.o grandmal.o kal.o lineslide.o localop.o main.o munch.o rest.o ripple.o schwartz.o spheres.o tetra.o wallpaper.o worms.o
.c.o:
	mcc -c $<

alph:	$(OFILES) rom.o
	mcc -j -o alph $(OFILES) rom.o

pp:
	pp -t"Alphabet Soup" -fMM -k $(CFILES) | dcan
