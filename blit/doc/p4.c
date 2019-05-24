lineto(p, f)
	Point p;
{
	cursinhibit();
	jlineto(p, f);
	cursallow();
}
