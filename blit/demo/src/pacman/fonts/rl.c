/*
 * reverse a file line by line
 *
 * copies stdin to stdout
 *
 * compile with: cc rl.c -o rl
 */
#include <stdio.h>
main()
{
	rev();
}
rev()
{
	long pos;
        char buf[512];
	if ( gets(buf) != NULL ) {
		rev();
		puts(buf);
	}
}
