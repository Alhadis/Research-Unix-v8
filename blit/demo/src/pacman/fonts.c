#define FONTSIZE	24	/* basic font size in bits		       */
#define BIGFONT		40	/* font size for pacman and monsters	       */
#define BYTESIZE	8
unsigned char chr24[][FONTSIZE*FONTSIZE/BYTESIZE] = {

/* character a */
#if BLIT
	{ 0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0377, 0377, 0377, 
	  0377, 0377, 0377,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000,   },
#endif

/* character b */
#if BLIT
	{ 0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000,   },
#endif

/* character c */
#if BLIT
	{ 0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0001, 0377, 
	  0000, 0003, 0377,  0000, 0007, 0000,  0000, 0016, 0000, 
	  0000, 0034, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000,   },
#endif

/* character d */
#if BLIT
	{ 0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0377, 0200, 0000, 
	  0377, 0300, 0000,  0000, 0340, 0000,  0000, 0160, 0000, 
	  0000, 0070, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000,   },
#endif

/* character e */
#if BLIT
	{ 0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0070, 0000, 
	  0000, 0160, 0000,  0000, 0340, 0000,  0377, 0300, 0000, 
	  0377, 0200, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000,   },
#endif

/* character f */
#if BLIT
	{ 0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0030, 0000, 
	  0000, 0030, 0000,  0000, 0030, 0000,  0000, 0034, 0000, 
	  0000, 0016, 0000,  0000, 0007, 0000,  0000, 0003, 0377, 
	  0000, 0001, 0377,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000,   },
#endif

/* character g */
#if BLIT
	{ 0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0063, 0063, 0063, 
	  0314, 0314, 0314,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000,   },
#endif

/* character h */
#if BLIT
	{ 0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0030, 0000,  0000, 0074, 0000,  0000, 0176, 0000, 
	  0000, 0176, 0000,  0000, 0074, 0000,  0000, 0030, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000,   },
#endif

/* character i */
#if BLIT
	{ 0001, 0370, 0000,  0007, 0376, 0000,  0037, 0377, 0200, 
	  0077, 0377, 0300,  0077, 0377, 0300,  0177, 0377, 0340, 
	  0177, 0377, 0340,  0377, 0377, 0360,  0377, 0377, 0360, 
	  0377, 0377, 0360,  0377, 0377, 0360,  0377, 0377, 0360, 
	  0377, 0377, 0360,  0177, 0377, 0340,  0177, 0377, 0340, 
	  0077, 0377, 0300,  0077, 0377, 0300,  0037, 0377, 0200, 
	  0007, 0376, 0000,  0001, 0370, 0000,  0000, 0000, 0000, 
	  0000, 0000, 0000,  0000, 0000, 0000,  0000, 0000, 0000,   },
#endif
};

unsigned char chr40[][BIGFONT*BIGFONT/BYTESIZE] = {

/* character a */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0377, 0377, 0377, 0340,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0037, 0377, 0377, 0377, 0370, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0037, 0377, 0377, 0377, 0370,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0007, 0377, 0377, 0377, 0340, 
	  0003, 0377, 0377, 0377, 0300,  0001, 0377, 0377, 0377, 0200, 
	  0000, 0177, 0377, 0376, 0000,  0000, 0077, 0377, 0374, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0000, 0377, 0000, 0000,   },
#endif

/* character b */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0377, 0377, 0377, 0340,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0037, 0377, 0377, 0377, 0370, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0360, 
	  0377, 0377, 0377, 0377, 0000,  0377, 0377, 0377, 0360, 0000, 
	  0377, 0377, 0377, 0000, 0000,  0377, 0377, 0360, 0000, 0000, 
	  0377, 0377, 0360, 0000, 0000,  0377, 0377, 0377, 0000, 0000, 
	  0377, 0377, 0377, 0360, 0000,  0377, 0377, 0377, 0377, 0000, 
	  0177, 0377, 0377, 0377, 0360,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0037, 0377, 0377, 0377, 0370,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0007, 0377, 0377, 0377, 0340, 
	  0003, 0377, 0377, 0377, 0300,  0001, 0377, 0377, 0377, 0200, 
	  0000, 0177, 0377, 0376, 0000,  0000, 0077, 0377, 0374, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0000, 0377, 0000, 0000,   },
#endif

/* character c */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0377, 0377, 0377, 0340,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0037, 0377, 0377, 0377, 0370, 
	  0077, 0377, 0377, 0377, 0370,  0077, 0377, 0377, 0377, 0340, 
	  0177, 0377, 0377, 0377, 0200,  0177, 0377, 0377, 0376, 0000, 
	  0177, 0377, 0377, 0370, 0000,  0177, 0377, 0377, 0340, 0000, 
	  0377, 0377, 0377, 0200, 0000,  0377, 0377, 0376, 0000, 0000, 
	  0377, 0377, 0370, 0000, 0000,  0377, 0377, 0360, 0000, 0000, 
	  0377, 0377, 0360, 0000, 0000,  0377, 0377, 0370, 0000, 0000, 
	  0377, 0377, 0376, 0000, 0000,  0377, 0377, 0377, 0200, 0000, 
	  0177, 0377, 0377, 0340, 0000,  0177, 0377, 0377, 0370, 0000, 
	  0177, 0377, 0377, 0376, 0000,  0177, 0377, 0377, 0377, 0200, 
	  0077, 0377, 0377, 0377, 0340,  0077, 0377, 0377, 0377, 0370, 
	  0037, 0377, 0377, 0377, 0370,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0007, 0377, 0377, 0377, 0340, 
	  0003, 0377, 0377, 0377, 0300,  0001, 0377, 0377, 0377, 0200, 
	  0000, 0177, 0377, 0376, 0000,  0000, 0077, 0377, 0374, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0000, 0377, 0000, 0000,   },
#endif

/* character d */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0200, 
	  0007, 0377, 0377, 0377, 0000,  0017, 0377, 0377, 0376, 0000, 
	  0017, 0377, 0377, 0374, 0000,  0037, 0377, 0377, 0370, 0000, 
	  0077, 0377, 0377, 0360, 0000,  0077, 0377, 0377, 0340, 0000, 
	  0177, 0377, 0377, 0300, 0000,  0177, 0377, 0377, 0200, 0000, 
	  0177, 0377, 0377, 0000, 0000,  0177, 0377, 0376, 0000, 0000, 
	  0377, 0377, 0374, 0000, 0000,  0377, 0377, 0370, 0000, 0000, 
	  0377, 0377, 0360, 0000, 0000,  0377, 0377, 0340, 0000, 0000, 
	  0377, 0377, 0340, 0000, 0000,  0377, 0377, 0360, 0000, 0000, 
	  0377, 0377, 0370, 0000, 0000,  0377, 0377, 0374, 0000, 0000, 
	  0177, 0377, 0376, 0000, 0000,  0177, 0377, 0377, 0000, 0000, 
	  0177, 0377, 0377, 0200, 0000,  0177, 0377, 0377, 0300, 0000, 
	  0077, 0377, 0377, 0340, 0000,  0077, 0377, 0377, 0360, 0000, 
	  0037, 0377, 0377, 0370, 0000,  0017, 0377, 0377, 0374, 0000, 
	  0017, 0377, 0377, 0376, 0000,  0007, 0377, 0377, 0377, 0000, 
	  0003, 0377, 0377, 0377, 0200,  0001, 0377, 0377, 0377, 0200, 
	  0000, 0177, 0377, 0376, 0000,  0000, 0077, 0377, 0374, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0000, 0377, 0000, 0000,   },
#endif

/* character e */
#if BLIT
	{ 0000, 0000, 0000, 0000, 0000,  0000, 0016, 0000, 0160, 0000, 
	  0000, 0076, 0000, 0174, 0000,  0000, 0176, 0000, 0176, 0000, 
	  0001, 0377, 0000, 0377, 0200,  0003, 0377, 0000, 0377, 0300, 
	  0007, 0377, 0000, 0377, 0340,  0017, 0377, 0000, 0377, 0360, 
	  0017, 0377, 0201, 0377, 0360,  0037, 0377, 0201, 0377, 0370, 
	  0077, 0377, 0201, 0377, 0374,  0077, 0377, 0201, 0377, 0374, 
	  0177, 0377, 0303, 0377, 0376,  0177, 0377, 0303, 0377, 0376, 
	  0177, 0377, 0303, 0377, 0376,  0177, 0377, 0303, 0377, 0376, 
	  0377, 0377, 0347, 0377, 0377,  0377, 0377, 0347, 0377, 0377, 
	  0377, 0377, 0347, 0377, 0377,  0377, 0377, 0347, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0037, 0377, 0377, 0377, 0370,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0007, 0377, 0377, 0377, 0340, 
	  0003, 0377, 0377, 0377, 0300,  0001, 0377, 0377, 0377, 0200, 
	  0000, 0177, 0377, 0376, 0000,  0000, 0077, 0377, 0374, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0000, 0377, 0000, 0000,   },
#endif

/* character f */
#if BLIT
	{ 0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0140, 0000, 0006, 0000, 
	  0001, 0340, 0000, 0007, 0200,  0003, 0360, 0000, 0017, 0300, 
	  0007, 0360, 0000, 0017, 0340,  0017, 0370, 0000, 0037, 0360, 
	  0017, 0370, 0000, 0037, 0360,  0037, 0374, 0000, 0077, 0370, 
	  0077, 0374, 0000, 0077, 0374,  0077, 0376, 0000, 0177, 0374, 
	  0177, 0376, 0000, 0177, 0376,  0177, 0377, 0000, 0377, 0376, 
	  0177, 0377, 0000, 0377, 0376,  0177, 0377, 0201, 0377, 0376, 
	  0377, 0377, 0201, 0377, 0377,  0377, 0377, 0303, 0377, 0377, 
	  0377, 0377, 0303, 0377, 0377,  0377, 0377, 0347, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0037, 0377, 0377, 0377, 0370,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0007, 0377, 0377, 0377, 0340, 
	  0003, 0377, 0377, 0377, 0300,  0001, 0377, 0377, 0377, 0200, 
	  0000, 0177, 0377, 0376, 0000,  0000, 0077, 0377, 0374, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0000, 0377, 0000, 0000,   },
#endif

/* character g */
#if BLIT
	{ 0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0014, 0000, 0000, 0000, 0060, 
	  0016, 0000, 0000, 0000, 0160,  0037, 0000, 0000, 0000, 0370, 
	  0077, 0200, 0000, 0001, 0374,  0077, 0300, 0000, 0003, 0374, 
	  0177, 0340, 0000, 0007, 0376,  0177, 0360, 0000, 0017, 0376, 
	  0177, 0370, 0000, 0037, 0376,  0177, 0374, 0000, 0077, 0376, 
	  0377, 0376, 0000, 0177, 0377,  0377, 0377, 0000, 0377, 0377, 
	  0377, 0377, 0201, 0377, 0377,  0377, 0377, 0303, 0377, 0377, 
	  0377, 0377, 0347, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0037, 0377, 0377, 0377, 0370,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0007, 0377, 0377, 0377, 0340, 
	  0003, 0377, 0377, 0377, 0300,  0001, 0377, 0377, 0377, 0200, 
	  0000, 0177, 0377, 0376, 0000,  0000, 0077, 0377, 0374, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0000, 0377, 0000, 0000,   },
#endif

/* character h */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0377, 0377, 0377, 0340,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0037, 0377, 0377, 0377, 0370, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0017, 0377, 0377, 0377, 0376, 
	  0000, 0377, 0377, 0377, 0377,  0000, 0017, 0377, 0377, 0377, 
	  0000, 0000, 0377, 0377, 0377,  0000, 0000, 0017, 0377, 0377, 
	  0000, 0000, 0017, 0377, 0377,  0000, 0000, 0377, 0377, 0377, 
	  0000, 0017, 0377, 0377, 0377,  0000, 0377, 0377, 0377, 0377, 
	  0017, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0037, 0377, 0377, 0377, 0370,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0007, 0377, 0377, 0377, 0340, 
	  0003, 0377, 0377, 0377, 0300,  0001, 0377, 0377, 0377, 0200, 
	  0000, 0177, 0377, 0376, 0000,  0000, 0077, 0377, 0374, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0000, 0377, 0000, 0000,   },
#endif

/* character i */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0377, 0377, 0377, 0340,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0037, 0377, 0377, 0377, 0370, 
	  0037, 0377, 0377, 0377, 0374,  0007, 0377, 0377, 0377, 0374, 
	  0001, 0377, 0377, 0377, 0376,  0000, 0177, 0377, 0377, 0376, 
	  0000, 0037, 0377, 0377, 0376,  0000, 0007, 0377, 0377, 0376, 
	  0000, 0001, 0377, 0377, 0377,  0000, 0000, 0177, 0377, 0377, 
	  0000, 0000, 0037, 0377, 0377,  0000, 0000, 0017, 0377, 0377, 
	  0000, 0000, 0017, 0377, 0377,  0000, 0000, 0037, 0377, 0377, 
	  0000, 0000, 0177, 0377, 0377,  0000, 0001, 0377, 0377, 0377, 
	  0000, 0007, 0377, 0377, 0376,  0000, 0037, 0377, 0377, 0376, 
	  0000, 0177, 0377, 0377, 0376,  0001, 0377, 0377, 0377, 0376, 
	  0007, 0377, 0377, 0377, 0374,  0037, 0377, 0377, 0377, 0374, 
	  0037, 0377, 0377, 0377, 0370,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0007, 0377, 0377, 0377, 0340, 
	  0003, 0377, 0377, 0377, 0300,  0001, 0377, 0377, 0377, 0200, 
	  0000, 0177, 0377, 0376, 0000,  0000, 0077, 0377, 0374, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0000, 0377, 0000, 0000,   },
#endif

/* character j */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0001, 0377, 0377, 0377, 0300, 
	  0000, 0377, 0377, 0377, 0340,  0000, 0177, 0377, 0377, 0360, 
	  0000, 0077, 0377, 0377, 0360,  0000, 0037, 0377, 0377, 0370, 
	  0000, 0017, 0377, 0377, 0374,  0000, 0007, 0377, 0377, 0374, 
	  0000, 0003, 0377, 0377, 0376,  0000, 0001, 0377, 0377, 0376, 
	  0000, 0000, 0377, 0377, 0376,  0000, 0000, 0177, 0377, 0376, 
	  0000, 0000, 0077, 0377, 0377,  0000, 0000, 0037, 0377, 0377, 
	  0000, 0000, 0017, 0377, 0377,  0000, 0000, 0007, 0377, 0377, 
	  0000, 0000, 0007, 0377, 0377,  0000, 0000, 0017, 0377, 0377, 
	  0000, 0000, 0037, 0377, 0377,  0000, 0000, 0077, 0377, 0377, 
	  0000, 0000, 0177, 0377, 0376,  0000, 0000, 0377, 0377, 0376, 
	  0000, 0001, 0377, 0377, 0376,  0000, 0003, 0377, 0377, 0376, 
	  0000, 0007, 0377, 0377, 0374,  0000, 0017, 0377, 0377, 0374, 
	  0000, 0037, 0377, 0377, 0370,  0000, 0077, 0377, 0377, 0360, 
	  0000, 0177, 0377, 0377, 0360,  0000, 0377, 0377, 0377, 0340, 
	  0001, 0377, 0377, 0377, 0300,  0001, 0377, 0377, 0377, 0200, 
	  0000, 0177, 0377, 0376, 0000,  0000, 0077, 0377, 0374, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0000, 0377, 0000, 0000,   },
#endif

/* character k */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0377, 0377, 0377, 0340,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0037, 0377, 0377, 0377, 0370, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0347, 0377, 0377,  0377, 0377, 0347, 0377, 0377, 
	  0377, 0377, 0347, 0377, 0377,  0377, 0377, 0347, 0377, 0377, 
	  0177, 0377, 0303, 0377, 0376,  0177, 0377, 0303, 0377, 0376, 
	  0177, 0377, 0303, 0377, 0376,  0177, 0377, 0303, 0377, 0376, 
	  0077, 0377, 0201, 0377, 0374,  0077, 0377, 0201, 0377, 0374, 
	  0037, 0377, 0201, 0377, 0370,  0017, 0377, 0201, 0377, 0360, 
	  0017, 0377, 0000, 0377, 0360,  0007, 0377, 0000, 0377, 0340, 
	  0003, 0377, 0000, 0377, 0300,  0001, 0377, 0000, 0377, 0200, 
	  0000, 0176, 0000, 0176, 0000,  0000, 0076, 0000, 0174, 0000, 
	  0000, 0016, 0000, 0160, 0000,  0000, 0000, 0000, 0000, 0000,   },
#endif

/* character l */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0377, 0377, 0377, 0340,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0037, 0377, 0377, 0377, 0370, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0347, 0377, 0377,  0377, 0377, 0303, 0377, 0377, 
	  0377, 0377, 0303, 0377, 0377,  0377, 0377, 0201, 0377, 0377, 
	  0177, 0377, 0201, 0377, 0376,  0177, 0377, 0000, 0377, 0376, 
	  0177, 0377, 0000, 0377, 0376,  0177, 0376, 0000, 0177, 0376, 
	  0077, 0376, 0000, 0177, 0374,  0077, 0374, 0000, 0077, 0374, 
	  0037, 0374, 0000, 0077, 0370,  0017, 0370, 0000, 0037, 0360, 
	  0017, 0370, 0000, 0037, 0360,  0007, 0360, 0000, 0017, 0340, 
	  0003, 0360, 0000, 0017, 0300,  0001, 0340, 0000, 0007, 0200, 
	  0000, 0140, 0000, 0006, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000,   },
#endif

/* character m */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0377, 0377, 0377, 0340,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0037, 0377, 0377, 0377, 0370, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0347, 0377, 0377, 
	  0377, 0377, 0303, 0377, 0377,  0377, 0377, 0201, 0377, 0377, 
	  0377, 0377, 0000, 0377, 0377,  0377, 0376, 0000, 0177, 0377, 
	  0177, 0374, 0000, 0077, 0376,  0177, 0370, 0000, 0037, 0376, 
	  0177, 0360, 0000, 0017, 0376,  0177, 0340, 0000, 0007, 0376, 
	  0077, 0300, 0000, 0003, 0374,  0077, 0200, 0000, 0001, 0374, 
	  0037, 0000, 0000, 0000, 0370,  0016, 0000, 0000, 0000, 0160, 
	  0014, 0000, 0000, 0000, 0060,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000,   },
#endif

/* character n */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0377, 0377, 0377, 0340,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0037, 0377, 0377, 0377, 0370, 
	  0077, 0360, 0037, 0370, 0014,  0077, 0340, 0017, 0360, 0014, 
	  0177, 0300, 0007, 0340, 0006,  0177, 0300, 0007, 0340, 0002, 
	  0177, 0301, 0307, 0340, 0342,  0177, 0303, 0347, 0341, 0362, 
	  0377, 0307, 0367, 0343, 0373,  0377, 0307, 0367, 0343, 0373, 
	  0377, 0307, 0367, 0343, 0373,  0377, 0303, 0347, 0341, 0363, 
	  0377, 0301, 0307, 0340, 0343,  0377, 0300, 0007, 0340, 0003, 
	  0377, 0300, 0007, 0340, 0003,  0377, 0340, 0017, 0360, 0007, 
	  0377, 0360, 0037, 0370, 0017,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0373, 0376, 0377, 0177, 0337, 
	  0373, 0376, 0377, 0177, 0337,  0361, 0374, 0176, 0077, 0217, 
	  0361, 0374, 0176, 0077, 0217,  0340, 0370, 0074, 0037, 0007, 
	  0340, 0370, 0074, 0037, 0007,  0300, 0160, 0030, 0016, 0003, 
	  0300, 0160, 0030, 0016, 0003,  0200, 0040, 0000, 0004, 0001,   },
#endif

/* character o */
#if BLIT
	{ 0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0001, 0377, 0000, 0377, 0200, 
	  0003, 0377, 0201, 0377, 0300,  0003, 0307, 0201, 0343, 0300, 
	  0003, 0203, 0201, 0301, 0300,  0003, 0203, 0201, 0301, 0300, 
	  0003, 0203, 0201, 0301, 0300,  0003, 0307, 0201, 0343, 0300, 
	  0003, 0377, 0201, 0377, 0300,  0001, 0377, 0000, 0377, 0200, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000,   },
#endif

/* character p */
#if BLIT
	{ 0000, 0000, 0125, 0000, 0000,  0000, 0012, 0252, 0240, 0000, 
	  0000, 0025, 0125, 0124, 0000,  0000, 0052, 0252, 0252, 0000, 
	  0001, 0125, 0125, 0125, 0000,  0002, 0252, 0252, 0252, 0200, 
	  0005, 0125, 0125, 0125, 0100,  0012, 0252, 0252, 0252, 0240, 
	  0005, 0125, 0125, 0125, 0120,  0012, 0252, 0252, 0252, 0250, 
	  0025, 0125, 0125, 0125, 0124,  0052, 0252, 0252, 0252, 0250, 
	  0125, 0125, 0125, 0125, 0124,  0052, 0000, 0252, 0000, 0052, 
	  0124, 0000, 0124, 0000, 0024,  0050, 0070, 0052, 0034, 0052, 
	  0124, 0174, 0124, 0076, 0025,  0250, 0174, 0052, 0076, 0052, 
	  0124, 0174, 0124, 0076, 0025,  0250, 0070, 0052, 0034, 0052, 
	  0124, 0000, 0124, 0000, 0025,  0252, 0000, 0252, 0000, 0052, 
	  0125, 0125, 0125, 0125, 0125,  0252, 0252, 0252, 0252, 0252, 
	  0125, 0125, 0125, 0125, 0125,  0252, 0052, 0052, 0052, 0052, 
	  0124, 0024, 0024, 0024, 0025,  0250, 0210, 0210, 0210, 0212, 
	  0121, 0101, 0101, 0101, 0105,  0242, 0242, 0242, 0242, 0242, 
	  0125, 0125, 0125, 0125, 0125,  0252, 0252, 0252, 0052, 0212, 
	  0121, 0124, 0125, 0125, 0125,  0240, 0250, 0052, 0052, 0212, 
	  0101, 0124, 0124, 0025, 0005,  0240, 0250, 0050, 0012, 0002, 
	  0100, 0120, 0024, 0025, 0005,  0200, 0040, 0010, 0012, 0002, 
	  0100, 0120, 0020, 0004, 0001,  0200, 0040, 0000, 0000, 0000,   },
#endif

/* character q */
#if BLIT
	{ 0000, 0000, 0252, 0000, 0000,  0000, 0012, 0252, 0240, 0000, 
	  0000, 0052, 0252, 0250, 0000,  0000, 0052, 0252, 0252, 0000, 
	  0000, 0252, 0252, 0252, 0200,  0002, 0252, 0252, 0252, 0200, 
	  0002, 0252, 0252, 0252, 0240,  0012, 0252, 0252, 0252, 0240, 
	  0012, 0252, 0252, 0252, 0240,  0012, 0252, 0252, 0252, 0250, 
	  0052, 0252, 0252, 0252, 0250,  0052, 0252, 0252, 0252, 0250, 
	  0052, 0252, 0252, 0252, 0252,  0052, 0000, 0252, 0000, 0052, 
	  0050, 0000, 0052, 0000, 0052,  0050, 0070, 0052, 0034, 0052, 
	  0250, 0174, 0052, 0076, 0052,  0250, 0174, 0052, 0076, 0052, 
	  0250, 0174, 0052, 0076, 0052,  0250, 0070, 0052, 0034, 0052, 
	  0250, 0000, 0052, 0000, 0052,  0252, 0000, 0252, 0000, 0052, 
	  0252, 0252, 0252, 0252, 0252,  0252, 0252, 0252, 0252, 0252, 
	  0252, 0252, 0252, 0252, 0252,  0252, 0252, 0252, 0252, 0252, 
	  0252, 0252, 0252, 0252, 0252,  0252, 0252, 0252, 0252, 0252, 
	  0252, 0252, 0252, 0252, 0252,  0252, 0252, 0252, 0252, 0252, 
	  0252, 0252, 0252, 0252, 0252,  0252, 0252, 0252, 0052, 0212, 
	  0252, 0252, 0252, 0052, 0212,  0240, 0250, 0052, 0052, 0212, 
	  0240, 0250, 0052, 0052, 0212,  0240, 0250, 0050, 0012, 0002, 
	  0240, 0250, 0050, 0012, 0002,  0200, 0040, 0010, 0012, 0002, 
	  0200, 0040, 0010, 0012, 0002,  0200, 0040, 0000, 0000, 0000,   },
#endif

/* character r */
#if BLIT
	{ 0000, 0000, 0000, 0007, 0000,  0000, 0000, 0000, 0037, 0200, 
	  0000, 0000, 0000, 0037, 0300,  0000, 0000, 0000, 0067, 0300, 
	  0000, 0000, 0000, 0354, 0000,  0000, 0000, 0001, 0230, 0000, 
	  0000, 0000, 0007, 0060, 0000,  0000, 0000, 0014, 0060, 0000, 
	  0000, 0000, 0070, 0140, 0000,  0000, 0000, 0140, 0140, 0000, 
	  0000, 0000, 0300, 0140, 0000,  0000, 0001, 0200, 0140, 0000, 
	  0000, 0003, 0000, 0140, 0000,  0000, 0006, 0000, 0140, 0000, 
	  0000, 0014, 0000, 0140, 0000,  0000, 0030, 0000, 0060, 0000, 
	  0000, 0030, 0000, 0060, 0000,  0000, 0060, 0000, 0060, 0000, 
	  0000, 0060, 0000, 0030, 0000,  0000, 0060, 0000, 0014, 0000, 
	  0000, 0374, 0000, 0077, 0000,  0001, 0376, 0000, 0177, 0200, 
	  0003, 0377, 0000, 0377, 0300,  0007, 0377, 0201, 0377, 0340, 
	  0017, 0377, 0303, 0377, 0360,  0017, 0377, 0303, 0377, 0360, 
	  0017, 0377, 0303, 0377, 0360,  0017, 0377, 0303, 0377, 0360, 
	  0017, 0377, 0303, 0377, 0360,  0017, 0377, 0303, 0377, 0360, 
	  0007, 0377, 0201, 0377, 0340,  0003, 0377, 0000, 0377, 0300, 
	  0001, 0376, 0000, 0177, 0200,  0000, 0374, 0000, 0077, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000,   },
#endif

/* character s */
#if BLIT
	{ 0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0177, 0377, 0377, 0377, 0376,  0177, 0377, 0377, 0377, 0376, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0377, 0377, 0377, 0374, 
	  0037, 0377, 0377, 0377, 0370,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0007, 0377, 0377, 0377, 0340, 
	  0003, 0377, 0377, 0377, 0300,  0001, 0377, 0377, 0377, 0200, 
	  0000, 0177, 0377, 0376, 0000,  0000, 0077, 0377, 0374, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0000, 0377, 0000, 0000,   },
#endif

/* character t */
#if BLIT
	{ 0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0030, 0000, 0000, 
	  0000, 0000, 0074, 0000, 0000,  0000, 0000, 0176, 0000, 0000, 
	  0000, 0000, 0377, 0000, 0000,  0000, 0001, 0377, 0200, 0000, 
	  0000, 0003, 0377, 0300, 0000,  0000, 0007, 0377, 0340, 0000, 
	  0000, 0017, 0377, 0360, 0000,  0000, 0037, 0377, 0370, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0000, 0377, 0377, 0377, 0000,  0001, 0377, 0377, 0377, 0200, 
	  0003, 0377, 0377, 0377, 0300,  0007, 0377, 0377, 0377, 0340, 
	  0017, 0377, 0347, 0377, 0360,  0037, 0377, 0347, 0377, 0370, 
	  0077, 0377, 0201, 0377, 0374,  0177, 0377, 0000, 0377, 0376, 
	  0377, 0374, 0000, 0077, 0377,  0077, 0300, 0000, 0003, 0374,   },
#endif

/* character u */
#if BLIT
	{ 0000, 0000, 0030, 0000, 0000,  0000, 0000, 0030, 0000, 0000, 
	  0000, 0000, 0030, 0000, 0000,  0000, 0000, 0030, 0000, 0000, 
	  0000, 0000, 0030, 0000, 0000,  0000, 0000, 0030, 0000, 0000, 
	  0006, 0000, 0030, 0000, 0140,  0003, 0000, 0030, 0000, 0300, 
	  0001, 0200, 0030, 0001, 0200,  0000, 0300, 0030, 0003, 0000, 
	  0000, 0140, 0030, 0006, 0000,  0000, 0060, 0030, 0014, 0000, 
	  0000, 0030, 0030, 0030, 0000,  0000, 0014, 0000, 0060, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0377, 0360, 0000, 0017, 0377, 
	  0377, 0360, 0000, 0017, 0377,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0000, 0000, 0000, 0000,  0000, 0000, 0000, 0000, 0000, 
	  0000, 0014, 0000, 0060, 0000,  0000, 0030, 0030, 0030, 0000, 
	  0000, 0060, 0030, 0014, 0000,  0000, 0140, 0030, 0006, 0000, 
	  0000, 0300, 0030, 0003, 0000,  0001, 0200, 0030, 0001, 0200, 
	  0003, 0000, 0030, 0000, 0300,  0006, 0000, 0030, 0000, 0140, 
	  0000, 0000, 0030, 0000, 0000,  0000, 0000, 0030, 0000, 0000, 
	  0000, 0000, 0030, 0000, 0000,  0000, 0000, 0030, 0000, 0000, 
	  0000, 0000, 0030, 0000, 0000,  0000, 0000, 0030, 0000, 0000,   },
#endif

/* character v */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0001, 0377, 0200, 0340,  0016, 0174, 0377, 0076, 0160, 
	  0014, 0174, 0176, 0076, 0060,  0034, 0376, 0176, 0177, 0070, 
	  0074, 0376, 0176, 0177, 0074,  0074, 0376, 0176, 0177, 0074, 
	  0174, 0174, 0176, 0076, 0076,  0174, 0070, 0176, 0034, 0076, 
	  0174, 0000, 0176, 0000, 0076,  0174, 0000, 0176, 0000, 0076, 
	  0174, 0000, 0176, 0000, 0076,  0376, 0000, 0377, 0000, 0177, 
	  0377, 0001, 0377, 0200, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0373, 0376, 0377, 0177, 0337, 
	  0373, 0376, 0377, 0177, 0337,  0361, 0374, 0176, 0077, 0217, 
	  0361, 0374, 0176, 0077, 0217,  0340, 0370, 0074, 0037, 0007, 
	  0340, 0370, 0074, 0037, 0007,  0300, 0160, 0030, 0016, 0003, 
	  0300, 0160, 0030, 0016, 0003,  0200, 0040, 0000, 0004, 0001,   },
#endif

/* character w */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0377, 0377, 0377, 0340,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0037, 0377, 0377, 0377, 0370, 
	  0060, 0037, 0370, 0017, 0374,  0040, 0017, 0360, 0007, 0374, 
	  0140, 0007, 0340, 0003, 0376,  0100, 0007, 0340, 0003, 0376, 
	  0107, 0007, 0343, 0203, 0376,  0117, 0207, 0347, 0303, 0376, 
	  0337, 0307, 0357, 0343, 0377,  0337, 0307, 0357, 0343, 0377, 
	  0337, 0307, 0357, 0343, 0377,  0317, 0207, 0347, 0303, 0377, 
	  0307, 0007, 0343, 0203, 0377,  0300, 0007, 0340, 0003, 0377, 
	  0300, 0007, 0340, 0003, 0377,  0340, 0017, 0360, 0007, 0377, 
	  0360, 0037, 0370, 0017, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0373, 0376, 0377, 0177, 0337, 
	  0373, 0376, 0377, 0177, 0337,  0361, 0374, 0176, 0077, 0217, 
	  0361, 0374, 0176, 0077, 0217,  0340, 0370, 0074, 0037, 0007, 
	  0340, 0370, 0074, 0037, 0007,  0300, 0160, 0030, 0016, 0003, 
	  0300, 0160, 0030, 0016, 0003,  0200, 0040, 0000, 0004, 0001,   },
#endif

/* character x */
#if BLIT
	{ 0000, 0000, 0377, 0000, 0000,  0000, 0017, 0377, 0360, 0000, 
	  0000, 0077, 0377, 0374, 0000,  0000, 0177, 0377, 0376, 0000, 
	  0001, 0377, 0377, 0377, 0200,  0003, 0377, 0377, 0377, 0300, 
	  0007, 0377, 0377, 0377, 0340,  0017, 0377, 0377, 0377, 0360, 
	  0017, 0377, 0377, 0377, 0360,  0037, 0377, 0377, 0377, 0370, 
	  0077, 0377, 0377, 0377, 0374,  0077, 0001, 0377, 0200, 0374, 
	  0176, 0000, 0377, 0000, 0176,  0174, 0000, 0176, 0000, 0076, 
	  0374, 0000, 0176, 0000, 0077,  0374, 0000, 0176, 0000, 0077, 
	  0374, 0070, 0176, 0034, 0077,  0374, 0174, 0176, 0076, 0077, 
	  0374, 0376, 0176, 0177, 0077,  0374, 0376, 0176, 0177, 0077, 
	  0374, 0376, 0176, 0177, 0077,  0374, 0174, 0176, 0076, 0077, 
	  0376, 0174, 0377, 0076, 0177,  0377, 0001, 0377, 0200, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0377, 0377, 0377, 0377, 0377, 
	  0377, 0377, 0377, 0377, 0377,  0373, 0376, 0377, 0177, 0337, 
	  0373, 0376, 0377, 0177, 0337,  0361, 0374, 0176, 0077, 0217, 
	  0361, 0374, 0176, 0077, 0217,  0340, 0370, 0074, 0037, 0007, 
	  0340, 0370, 0074, 0037, 0007,  0300, 0160, 0030, 0016, 0003, 
	  0300, 0160, 0030, 0016, 0003,  0200, 0040, 0000, 0004, 0001,   },
#endif
};
