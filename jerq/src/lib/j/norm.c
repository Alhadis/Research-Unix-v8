norm(x,y,z)
{
	return (sqrt(x*x + y*y + z*z));
}

sqrtryz(x,y,z)
{
	register long sumsq;

	sumsq = x*x - y*y - z*z;
	if (sumsq <= 0)
		return 0;
	return ( sqrt(sumsq) );
}
