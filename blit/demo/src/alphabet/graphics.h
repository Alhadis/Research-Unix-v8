/*
 * graphics.h - constants for the SUN graphics board version 1
 *
 * Bill Nowicki June 14, 1981
 */

# define GXUnit0Base	(0x100000)	/* Base adress of multibus memory space */

/*
 * The low order 11 bits consist of the X or Y address times 2.
 * The lowest order bit is ignored, so word addressing works efficiently.
 */

# define GXselectX (0<<11)	/* the address is loaded into an X register */
# define GXselectx (0<<11)	/* the address is loaded into an X register */
# define GXselectY (1<<11)	/* the address is loaded into an Y register */
# define GXselecty (1<<11)	/* the address is loaded into an Y register */

/*
 * There are four sets of X and Y register pairs, selected by the following bits
 */

# define GXaddressSet0  (0<<12)
# define GXaddressSet1  (1<<12)
# define GXaddressSet2  (2<<12)
# define GXaddressSet3  (3<<12)

/*
 * The following bits indicate which registers are to be loaded
 */

# define GXnone   (0<<14)
# define GXothers (1<<14)
# define GXsource (2<<14)
# define GXmask   (3<<14)

# define GXupdate (1<<16)	/* actually update the frame buffer */


/*
 * These registers can appear on the left of an assignment statement.
 * Note they clobber X register 0.
 */

# define GXfunction *(short *)(GXUnit0Base+GXothers+(0<<1) )
# define GXwidth    *(short *)(GXUnit0Base+GXothers+(1<<1) )
# define GXcontrol  *(short *)(GXUnit0Base+GXothers+(2<<1) )
# define GXintClear *(short *)(GXUnit0Base+GXothers+(3<<1) )
# define GXsrc	    *(short *)(GXUnit0Base+GXsource)

/*
 * The following bits are written into the GX control register.
 * It is reset to zero on hardware reset and power-up.
 */

# define GXintEnable   (1<<3)
# define GXvideoEnable (1<<7)

/*
 * The following are "function" encodings loaded into the function register
 */

# define GXnoop		0xAA
# define GXinvert	0x55
# define GXcopy        	0xCC
# define GXcopyInverted 0x33
# define GXclear	0x00
# define GXset		0xFF


/*
 * These may appear on the left of assignment statements to just
 * set the X and Y registers of set number zero to the given values.
 */

# define GXsetX(X)	*(short *)(GXUnit0Base|GXselectX|(X<<1)) = 1;
# define GXsetY(Y)	*(short *)(GXUnit0Base|GXselectY|(Y<<1)) = 1;
# define GXsetX1(x)	*(short *)(GXUnit0Base|GXselectX|GXaddressSet1|(x<<1)) = 1;
