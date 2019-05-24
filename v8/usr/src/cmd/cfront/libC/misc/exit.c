
extern void _exit(int);
extern void _cleanup();
typedef void (*PFV)();

extern void exit(int i)
{
	extern PFV _dtors[];
	PFV* pf = _dtors;
	while (*pf) pf++;
	while (_dtors < pf) (**--pf)();
	_cleanup();
	_exit(i);
}
