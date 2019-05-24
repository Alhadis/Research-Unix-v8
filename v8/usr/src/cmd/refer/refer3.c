# include "refer..c"
corout(in, out, rprog, arg, outlen)
	char *in, *out, *rprog;
{
# define move(x, y) close(y); dup(x); close(x);
int pipev[2], fr1, fr2, fw1, fw2, n;

pipe (pipev); fr1= pipev[0]; fw1 = pipev[1];
pipe (pipev); fr2= pipev[0]; fw2 = pipev[1];
if (fork()==0)
	{
	close (fw1); close (fr2);
	move (fr1, 0);
	move (fw2, 1);
	execl(rprog, "deliv", arg, 0);
	err ("Can't run %s", rprog);
	}
close(fw2); close(fr1);
write (fw1, in , strlen(in));
close(fw1);
wait(0);
n = iread (fr2, out, outlen);
out[n]=0;
close(fr2);
return(n);
}
