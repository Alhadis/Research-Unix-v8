#define PADS_TERM
typedef enum PI_Proto PI_Proto;

long assertf();
#define assert(e) ( assertf( (long) (e) ) )

typedef int  (*PFI)();
typedef long (*PFL)();
typedef char (*PFC)();

char *dec();
char *hex();

#ifdef JOURNAL
#define journal Journal
#else
#define journal if(0)
#endif JOURNAL

char *AlignErr();
char *ReadErr();
char *WriteErr();
void SendError();
