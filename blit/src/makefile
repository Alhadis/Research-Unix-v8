all:
	echo type make foo.m to install foo in /usr/blit/mbin
%.m:	%.c
	mcc $%.c
	mopt a.out
	cp m.out /usr/blit/mbin/$%.m
	rm a.out m.out
install:	56.m crabs.m hp.m icon.m jps.m m33.m pen.m ruler.m scope.m windows.m
