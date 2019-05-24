/* broadcast umpire, reads strings of the form
 * "/dev/pt/pt.." from its standard input
 * and adds those files to its broadcast list
 */

#define	printf

int readmask, writemask;
struct msg {
	int len;
	char data[64];
	struct msg *next;
} from[32];

main()
{	int n, rdm, i;
	register int j;
	register struct msg *p;
	readmask = 1;	/* control channel is fd 0 */
	writemask = 0;
	close(0);
	if(open("/dev/pt/ump", 0) < 0 || open("/dev/pt/maze", 0) < 0) {
		perror("open");
		exit(1);
	}
loop:
	rdm = readmask;
	n = select(20, &rdm, 0, 6000);
	if(n == -1) {
		perror("ump");
		goto loop;
	}
	if(n == 0)
		goto loop;
	printf("got %d 0%o from select\n", n, rdm);
	if(rdm & 1)	/* new fan */
		newfan();
	/* here to try to flush queues of blocked writers */
	for(i = 1; i < 20; i++) {
		if(!(rdm & (1 << i)))
			continue;
		p = from + i;
		printf("\t:reading %d\n", i);
		p->len = read(i, p->data, 64);
		printf("\t:got %s from %d %d\n", p->data, i, p->len);
		if(p->len < 0) {
quit:
			shutdown(i);
			continue;
		}
		for(j = 0; j < p->len; j++)
			if(p->data[j] == 4) {	/*EOF*/
				p->len = j;
				if(j > 0)
					broadcast(i);
				goto quit;
			}
		broadcast(i);
	}
	goto loop;
}
shutdown(i)
{
	readmask &= ~(1 << i);
	writemask &= ~(1 << i);
	close(i);
}
char name[32];
newfan()
{	char x[2];
	int n;
	n = read(0, name, sizeof(name));
	if(n > 0 && name[n-1] == '\n') {
		n--;
		name[n] = 0;
	}
	if(n <= 0)
		return;
	n = open(name, 2);
	if(n < 0) {
	/*	perror(name);	
		abort();	* Nothing we can do, anyway */
		return;
	}
	printf("newfan %s %d\n", name, n);
	x[0] = 'y';
	x[1] = ' ' + n;
	write(n, x, 2);
	readmask |= 1 << n;
	writemask |= 1 << n;
}

broadcast(who)
{	int n, wrm;
	register int j;
	register struct msg *p = from + who;
	wrm = writemask;
	n = select(20, 0, &wrm, 0);
	printf("\t:wrtsel %d 0%o \n", n, wrm);
	for(j = 1; j < 20; j++) {
		if(wrm & (1 << j)) {
			if(write(j, p->data, p->len)!=p->len)
				shutdown(j);
			printf("\t:send %s to %d\n", p->data, j);
		}
		else	/* should queue it */
			;
	}
}
