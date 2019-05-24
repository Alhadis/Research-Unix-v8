/*
 *	rp07rest: quick and dirty to copy 4 reels of tape to RP07
 *	Tape errors are not tolerated.  Input errors are ignored.
 */

#include <stdio.h>
#include <assert.h>

#define TAPE "/dev/rmt2"	/* tape to use */
#define SECTSIZE 512		/* bytes per sector */
#define NSECT 1008000		/* sectors per RP07 */
#define TRKSIZE 50		/* sectors per track */
#define NREELS 4		/* reels of tape in the dump */
#define BPT (NSECT/(NREELS*TRKSIZE))	/* buffer loads per tape */

char buf[SECTSIZE*TRKSIZE];

main (argc, argv)
	int argc;
	char **argv;
{
	register int disk, tape, n, i;
	int reel;

	if (argc != 2) {
		fprintf (stderr, "arg count");
		exit (1);
	}

	disk = open (argv[1], 1);
	if (disk < 0) {
		perror (argv[1]);
		exit (1);
	}

	for (reel = 1; reel <= NREELS; reel++) {

		do {
			char answer[500];
			do {
				printf ("Have you mounted volume %d? ", reel);
				gets (answer);
			} while (strcmp (answer, "yes") != 0);

			tape = open (TAPE, 0);
			if (tape < 0)
				perror (TAPE);
		} while (tape < 0);

		for (i = 0; i < BPT; i++) {
			n = read (tape, buf, sizeof(buf));
			if (n != sizeof (buf)) {
				perror (TAPE);
				exit (1);
			}
			n = write (disk, buf, sizeof(buf));
			if (n != sizeof (buf)) {
				perror (argv[1]);
				exit (1);
			}
		}

		close (tape);
	}

	printf ("End of restore\n");
	exit (0);
}
