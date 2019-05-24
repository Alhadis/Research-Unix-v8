/*
 *	rp07dump: quick and dirty to copy an entire RP07 to tape
 *	Tape errors are not tolerated.  Input errors are handled.
 *	Dumping is to 4 reels; each reel gets exactly 1/4 of the disk.
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
	long blockno;	/* block number on disk */

	if (argc != 2) {
		fprintf (stderr, "arg count");
		exit (1);
	}

	disk = open (argv[1], 0);
	if (disk < 0) {
		perror (argv[1]);
		exit (1);
	}

	blockno = 0;
	for (reel = 1; reel <= NREELS; reel++) {

		do {
			char answer[500];
			do {
				printf ("Have you mounted volume %d? ", reel);
				gets (answer);
			} while (strcmp (answer, "yes") != 0);

			tape = open (TAPE, 1);
			if (tape < 0)
				perror (TAPE);
		} while (tape < 0);

		for (i = 0; i < BPT; i++) {
			n = read (disk, buf, sizeof(buf));
			if (n != sizeof (buf)) {
				long boff;
				long bytebase;
				fprintf (stderr, "trouble near block %ld\n",
					blockno);
				perror (argv[1]);

				for (n = 0; n < sizeof (buf); n++)
					buf[n] = '?';

				bytebase = blockno * SECTSIZE;
				for (boff=0; boff<sizeof(buf); boff+=SECTSIZE) {
					if (lseek (disk,bytebase+boff,0) < 0) {
						perror ("lseek");
						exit (1);
					}
					n = read (disk, buf+boff, SECTSIZE);
					if (n != SECTSIZE) {
						fprintf (stderr,
						    "can't read sector %ld\n",
						    blockno + boff / SECTSIZE);
					}
				}

				if (lseek (disk,(blockno+TRKSIZE)*SECTSIZE,0)
				    < 0) {
					perror ("seek 2");
					exit (1);
				}
			}
			blockno += TRKSIZE;
			n = write (tape, buf, sizeof(buf));
			if (n != sizeof (buf)) {
				perror (argv[1]);
				exit (1);
			}
		}

		close (tape);
	}

	printf ("End of dump\n");
	exit (0);
}
