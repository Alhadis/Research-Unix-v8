/*
 * relocate command--
 * reloc file [-]octal [ - ]
 *
 * relocate object or a.out file up in core
 * by the possibly negated octal number.
 *
 * if optional 3rd arg is given,
 * replace "setd" at start by "nop"
 */

int	tbuf[256];
int	rbuf[256];
int	fin;
int	fout;
int	*txtp;
int	*relp;
long	relloc;
long	txtloc;
int	dotdot;
long	txtsiz;
unsigned	t1;
unsigned	t2;
unsigned	t4;
unsigned	t7;
int	txtw;

main(argc, argv)
char *argv[];
{
	int sign, c;

	if (argc<3) {
	usage:
		write(1, "reloc file [-]octal\n", 20);
		exit(1);
	}
	dotdot = 0;
	if (*argv[2] == '-') {
		sign = -1;
		argv[2]++;
	} else
		sign = 1;
	while (*argv[2]) {
		c = *argv[2]++ - '0';
		if (c<0 || c>7)
			goto usage;
		dotdot = (dotdot<<3) + c;
	}
	dotdot *= sign;
	if ((fin = open(argv[1], 0)) < 0) {
		write(1, "File not readable\n", 18);
		exit(1);
	}
	if ((fout = open(argv[1], 1)) < 0) {
		write(1, "File not writable\n", 18);
		exit(1);
	}
	txtw = read(fin, (char *)tbuf, 512);
	if (tbuf[0]!=0411 && tbuf[0]!=0410 && tbuf[0]!=0407) {	/* magic */
		write(1, "Bad format\n", 11);
		exit(1);
	}
	if (tbuf[7] != 0) {		/* relocation? */
		write (1, "No relocation bits\n", 19);
		exit(1);
	}
	t1 = tbuf[1];
	t2 = tbuf[2];
	t4 = tbuf[4];
	txtloc = 020;
	if (argc>3 && argv[3][1]=='o') {
		dotdot += 020;
		tbuf[0] = 0405;	/* old magic */
		tbuf[1] += tbuf[2];
		tbuf[2] = 0;
		tbuf[4] = tbuf[3];
		tbuf[3] = 0;
		tbuf[6] = 0240;
		tbuf[7] = t7 = 0240;
	}
	txtsiz = (unsigned)tbuf[1];
	txtsiz += (unsigned)tbuf[2];
	relloc = txtloc + txtsiz;
	txtsiz /= 2;
	lseek(fin, relloc & ~0777, 0);
	read(fin, (char *)rbuf, 512);
	txtp = &tbuf[8];
	relp = &rbuf[(relloc&0777) >> 1];
	if (argc>3)		/* nop out "setd" at start */
		if (tbuf[8] == 0170011)
			tbuf[8] = 0240;
	while(txtsiz--) {
		switch (*relp & 017) {

		case 01:	/* pc ref to abs */
			*txtp -= dotdot;
			break;

		case 02:	/* ref to text */
		case 04:		/* ref to data */
		case 06:		/* ref to bss */
			*txtp += dotdot;

		}
		advance();
	}
	if (txtp != &tbuf[0])
		write(fout, (char *)&tbuf[0], txtw);


	txtsiz = t4 / 2;
	relloc = 020;
	relloc += t1;
	relloc += t1;
	relloc += t2;
	relloc += t2;
	lseek(fin, relloc & ~0777, 0);
	lseek(fout, relloc & ~0777, 0);
	txtw = read(fin, (char *)tbuf, 512);
	txtp = &tbuf[(relloc&0777) >> 1] + 4;
	while((txtsiz -= 6) > 0) {
		switch(*txtp & 77) {

		case 2:
		case 3:
		case 4:
			*(txtp + 1) += dotdot;
		}
		adv6();
	}
	if(txtp != &tbuf[0])
		write(fout, (char *)&tbuf[0], txtw);
	exit(0);
}

advance()
{
	
	relp++;
	relloc += 2;
	if (relp == &rbuf[256]) {
		lseek(fin, relloc, 0);
		read(fin, (char *)&rbuf[0], 512);
		relp = &rbuf[0];
	}
	txtp++;
	txtloc += 2;
	if (txtp >= &tbuf[256]) {
		write(fout, (char *)&tbuf[0], txtw);
		lseek(fin, txtloc, 0);
		txtw = read(fin, (char *)&tbuf[0], 512);
		txtp = &tbuf[0];
	}
}

adv6()
{
	txtp += 6;
	if(txtp >= &tbuf[256]) {
		write(fout, (char *)tbuf, txtw);
		txtw = read(fin, (char *)tbuf, 512);
		txtp -= 256;
	}
}
