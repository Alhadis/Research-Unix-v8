#include <jerq.h>

main(argc, argv)
char **argv;
{
	Point move; Rectangle r, canon();
	move.x = 0;
	move.y = 0;
	if (argc > 2) {
		move.x = atoi(argv[1]);
		move.y = atoi(argv[2]);
	}
	r = raddp(P->layer->rect, move);
	r = canon(r);
	reshape(r, 1);
	exit();
}
