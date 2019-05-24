
typedef void (*PFVV)();

extern PFVV _new_handler;

extern void* operator new(long size)
{
	extern char* malloc(unsigned);
	char* p;

	while ( (p=malloc(unsigned(size)))==0 ) {
		if(_new_handler)
			(*_new_handler)();
		else
			return 0;
	}
	return (void*)p;
}
