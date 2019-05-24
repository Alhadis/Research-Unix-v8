#define		DEFONT
#include <jerq.h>
#include <font.h>

static int bits[] = {
	0xFF80,	0x3,	0x360,	0x60,	0xE038,	0x703,	0x600,	0x0,
	0x0,	0x3,	0x1E06,	0xFC7,	0xE063,	0xFCFC,	0xFF3F,	0x1F80,
	0x0,	0x3000,	0xC07E,	0x6,	0x1FC7,	0xE7E3,	0xFDFE,	0x7E61,
	0x9F80,	0xCC36,	0x30D,	0xC67E,	0x7F0F,	0x1FC7,	0xE7FB,	0xD86,
	0xC361,	0xB0DF,	0xE3C3,	0xF0,	0x3000,	0x3800,	0x1800,	0x18,
	0x3C,	0x60,	0x0,	0xC03,	0xC000,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0xF0,	0xC3C0,	0x77,	0xFF80,
	0x3,	0x360,	0x1F9,	0xB06C,	0x606,	0x300,	0x0,	0x0,
	0x3,	0x330E,	0x1CEE,	0x70E3,	0x1CE,	0x373,	0xB9C0,	0x0,
	0x6000,	0x60C3,	0xF,	0x18EE,	0x7673,	0x180,	0xE761,	0x8600,
	0xCC66,	0x39D,	0xC6E7,	0x6399,	0x98EE,	0x70C3,	0xD86,	0xC333,
	0x1980,	0x6303,	0x30,	0x7800,	0x1800,	0x1800,	0x18,	0x66,
	0x60,	0x601,	0x8C00,	0xC000,	0x0,	0x0,	0x0,	0x180,
	0x0,	0x0,	0x0,	0x180,	0xC060,	0x77,	0xFF8C,	0x3,
	0x363,	0x33FD,	0xB6CC,	0xE0C,	0x189,	0x90C0,	0x0,	0x6,
	0x619E,	0x60,	0x31E3,	0x180,	0x361,	0xB0C0,	0x0,	0xC000,
	0x30C3,	0x19,	0x986C,	0x63B,	0x180,	0xC061,	0x8600,	0xCCC6,
	0x3FD,	0xE6C3,	0x61B0,	0xD86C,	0xC3,	0xD86,	0xC333,	0x1980,
	0x6301,	0x8030,	0xCC00,	0x1C00,	0x1800,	0x18,	0x60,	0x60,
	0x0,	0xC00,	0xC000,	0x0,	0x0,	0x0,	0x180,	0x0,
	0x0,	0x0,	0x180,	0xC060,	0xDD,	0xFF9E,	0x3,	0x363,
	0x3360,	0xECCC,	0xC0C,	0x185,	0xA0C0,	0x0,	0x6,	0x6386,
	0xC0,	0x3363,	0x180,	0x361,	0xB0C0,	0x1C1,	0x8000,	0x1803,
	0x3F30,	0xD86C,	0x61B,	0x180,	0xC061,	0x8600,	0xCD86,	0x3FD,
	0xE6C3,	0x61B0,	0xD86C,	0xC3,	0xD86,	0xC31E,	0xF00,	0xC301,
	0x8031,	0x8600,	0xC00,	0x1800,	0x18,	0x60,	0x60,	0x0,
	0xC00,	0xC000,	0x0,	0x0,	0x0,	0x180,	0x0,	0x0,
	0x0,	0x180,	0xC060,	0xC001,	0xFF9E,	0x3,	0x7,	0xFBE0,
	0x18D8,	0xC,	0x183,	0xC0C0,	0x0,	0xC,	0x6786,	0x180,
	0x7663,	0x180,	0x673,	0xB9C7,	0x1C3,	0x3FC,	0xC06,	0x61B0,
	0xD8EC,	0x61B,	0x1F8,	0xC061,	0x8600,	0xCF06,	0x36D,	0xF6C3,
	0x63B0,	0xD8EE,	0xC3,	0xD86,	0xC31E,	0xF01,	0x8300,	0xC030,
	0x0,	0x1F,	0x9807,	0xE019,	0xF860,	0x7E7F,	0x601,	0x8C60,
	0xC3F9,	0xF87C,	0x7F1F,	0xDFC7,	0xF7E3,	0x1986,	0xC361,	0xB0DF,
	0xE380,	0xC071,	0xE218,	0xFF9E,	0x3,	0x3,	0x31F8,	0x3070,
	0xC,	0x18F,	0xF7F8,	0x3FC,	0xC,	0x6D86,	0x307,	0xE7FB,
	0xF9FC,	0xC3F,	0x1FC7,	0x6,	0x3FC,	0x60C,	0x6EB0,	0xDFCC,
	0x61B,	0xF180,	0xC07F,	0x8600,	0xCE06,	0x30D,	0xB6C3,	0x7F30,
	0xDFC7,	0xE0C3,	0xD86,	0xDB0C,	0x60F,	0xC300,	0xC030,	0x0,
	0x30,	0xDFCC,	0x33FB,	0xDF8,	0xC361,	0x8601,	0x8CC0,	0xC36D,
	0x8CC6,	0x61B0,	0xDC0C,	0x183,	0x1986,	0xC333,	0x1980,	0xCF00,
	0xC03D,	0x1E00,	0xFF9E,	0x3,	0x3,	0x307C,	0x6070,	0xC,
	0x183,	0xC7F8,	0x3FC,	0x18,	0x7986,	0x600,	0x7060,	0x1D8E,
	0x1873,	0x80C0,	0x3,	0x0,	0xC18,	0x69BF,	0xD8EC,	0x61B,
	0x180,	0xCF61,	0x8600,	0xCF06,	0x30D,	0xBEC3,	0x6030,	0xDE00,
	0x70C3,	0xCCC,	0xDB1E,	0x606,	0x300,	0x6030,	0x0,	0x0,
	0xD86C,	0x61B,	0xC60,	0xC361,	0x8601,	0x8D80,	0xC36D,	0x8CC6,
	0x61B0,	0xD80C,	0x183,	0x18CC,	0xC31E,	0x1981,	0x8380,	0xC070,
	0xC8C,	0xFFB3,	0x0,	0x7,	0xF86C,	0xDCD8,	0xC,	0x185,
	0xA0C0,	0xE000,	0x18,	0x7186,	0xC00,	0x3060,	0xD86,	0x3061,
	0x80C0,	0x1C1,	0x83FC,	0x1818,	0x6DB0,	0xD86C,	0x61B,	0x180,
	0xC361,	0x8600,	0xCD86,	0x30D,	0x9EC3,	0x6036,	0xDB00,	0x30C3,
	0xCCC,	0xFF1E,	0x60C,	0x300,	0x6030,	0x0,	0x1F,	0xD86C,
	0x61B,	0xF860,	0xC361,	0x8601,	0x8F00,	0xC36D,	0x8CC6,	0x61B0,
	0xD807,	0xE183,	0x18CC,	0xDB0C,	0xF03,	0x180,	0xC060,	0x3,
	0xFFE1,	0x8000,	0x3,	0x33FD,	0xB6CF,	0xC,	0x189,	0x90C0,
	0xE000,	0x7030,	0x6186,	0x1800,	0x3060,	0xD86,	0x3061,	0x80C7,
	0x1C0,	0xC3FC,	0x3000,	0x67B0,	0xD86C,	0x63B,	0x180,	0xC361,
	0x8618,	0xCCC6,	0x30D,	0x9EC3,	0x6037,	0xD980,	0x30C3,	0xC78,
	0xFF33,	0x618,	0x300,	0x3030,	0x0,	0x30,	0xD86C,	0x61B,
	0x60,	0x7F61,	0x8601,	0x8F80,	0xC36D,	0x8CC6,	0x61B0,	0xD800,
	0x3183,	0x1878,	0xFF1E,	0x606,	0x180,	0xC060,	0x0,	0xFFFF,
	0x8003,	0x3,	0x31F8,	0x36C6,	0x6,	0x300,	0xC0,	0xC000,
	0x7030,	0x3306,	0x180E,	0x7063,	0x9DCE,	0x3073,	0xB9C7,	0x180,
	0x6000,	0x6000,	0x3030,	0xD8EE,	0x7673,	0x180,	0xE761,	0x861D,
	0xCC66,	0x30D,	0x8EE7,	0x601B,	0x98CE,	0x70C3,	0x9C78,	0xE733,
	0x618,	0x300,	0x3030,	0x0,	0x30,	0xD86C,	0x361B,	0x60,
	0x361,	0x8601,	0x8CC0,	0xC36D,	0x8CC6,	0x7F1F,	0xD800,	0x31B3,
	0x1878,	0xE733,	0xC0C,	0x180,	0xC060,	0x8,	0xFF8C,	0x3,
	0x0,	0x60,	0x1C7F,	0x3,	0x600,	0x1,	0x8000,	0x60,
	0x1E1F,	0x9FE7,	0xE061,	0xF8FC,	0x303F,	0x1F80,	0x300,	0x3000,
	0xC018,	0x1FB0,	0xDFC7,	0xE7E3,	0xFD80,	0x7E61,	0x9F8F,	0x8C37,
	0xFB0D,	0x8E7E,	0x600F,	0xD867,	0xE0C1,	0xF830,	0xC361,	0x861F,
	0xE3C0,	0x18F0,	0x0,	0x1F,	0xDFC7,	0xE3F9,	0xFC60,	0x361,
	0x8601,	0x8C63,	0xF36D,	0x8C7C,	0x6000,	0xD80F,	0xE0E1,	0xF830,
	0xC361,	0x981F,	0xE0F0,	0xC3C0,	0x0,	0xFF80,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x60,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x1800,	0xFF,	0x0,	0x0,	0x0,	0x0,	0xC300,	0xD,
	0x8000,	0x0,	0x0,	0x6000,	0xC000,	0x0,	0x0,	0x0,
	0x1800,	0x0,	0xC000,	0x0,	0xFF80,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0xFF,	0x0,	0x0,	0x0,	0x0,	0x7E00,	0x7,	0x0,
	0x0,	0x0,	0x6000,	0xC000,	0x0,	0x0,	0x0,	0x3000,
	0x0,	0xC000,	0xF0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
	0x0,	0x0,};

static Bitmap strike = {
	bits,
	55,
	0, 0, 872, 14,
	0,
};

struct
{
	short n;		/* number of chars in font */
	char height;		/* height of bitmap */
	char ascent;		/* top of bitmap to baseline */
	long unused;		/* in case we think of more stuff */
	Bitmap *bits;		/* where the characters are */
	Fontchar info[128];	/* n+2 character descriptors */
}
defont = {
	126,
	14,
	10,
	0,
	&strike,
	{
		{ 0,	0,	0,	0,	0 },
		{ 0,	0,	13,	0,	9 },
		{ 9,	0,	0,	0,	0 },
		{ 9,	0,	0,	0,	0 },
		{ 9,	0,	0,	0,	0 },
		{ 9,	0,	0,	0,	0 },
		{ 9,	0,	0,	0,	0 },
		{ 9,	2,	11,	0,	9 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	0,	0,	0,	0 },
		{ 18,	13,	13,	0,	9 },
		{ 27,	0,	11,	0,	9 },
		{ 36,	0,	4,	0,	9 },
		{ 45,	2,	10,	0,	9 },
		{ 54,	0,	11,	0,	9 },
		{ 63,	0,	11,	0,	9 },
		{ 72,	0,	11,	0,	9 },
		{ 81,	0,	4,	0,	9 },
		{ 90,	0,	11,	0,	9 },
		{ 99,	0,	11,	0,	9 },
		{ 108,	2,	9,	0,	9 },
		{ 117,	2,	10,	0,	9 },
		{ 125,	7,	11,	0,	9 },
		{ 134,	5,	7,	0,	9 },
		{ 143,	8,	10,	0,	9 },
		{ 152,	0,	12,	0,	9 },
		{ 161,	0,	11,	0,	9 },
		{ 170,	0,	11,	0,	9 },
		{ 179,	0,	11,	0,	9 },
		{ 188,	0,	11,	0,	9 },
		{ 197,	0,	11,	0,	9 },
		{ 206,	0,	11,	0,	9 },
		{ 215,	0,	11,	0,	9 },
		{ 224,	0,	11,	0,	9 },
		{ 233,	0,	11,	0,	9 },
		{ 242,	0,	11,	0,	9 },
		{ 251,	4,	10,	0,	9 },
		{ 260,	3,	11,	0,	9 },
		{ 269,	0,	11,	0,	9 },
		{ 278,	4,	9,	0,	9 },
		{ 287,	0,	11,	0,	9 },
		{ 296,	0,	11,	0,	9 },
		{ 305,	3,	11,	0,	9 },
		{ 314,	0,	11,	0,	9 },
		{ 323,	0,	11,	0,	9 },
		{ 332,	0,	11,	0,	9 },
		{ 341,	0,	11,	0,	9 },
		{ 350,	0,	11,	0,	9 },
		{ 359,	0,	11,	0,	9 },
		{ 368,	0,	11,	0,	9 },
		{ 377,	0,	11,	0,	9 },
		{ 386,	0,	11,	0,	9 },
		{ 395,	0,	11,	0,	9 },
		{ 404,	0,	11,	0,	9 },
		{ 413,	0,	11,	0,	9 },
		{ 422,	0,	11,	0,	9 },
		{ 431,	0,	11,	0,	9 },
		{ 440,	0,	11,	0,	9 },
		{ 449,	0,	11,	0,	9 },
		{ 458,	0,	11,	0,	9 },
		{ 467,	0,	11,	0,	9 },
		{ 476,	0,	11,	0,	9 },
		{ 485,	0,	11,	0,	9 },
		{ 494,	0,	11,	0,	9 },
		{ 503,	0,	11,	0,	9 },
		{ 512,	0,	11,	0,	9 },
		{ 521,	0,	11,	0,	9 },
		{ 530,	0,	11,	0,	9 },
		{ 539,	0,	11,	0,	9 },
		{ 548,	0,	11,	0,	9 },
		{ 557,	0,	12,	0,	9 },
		{ 566,	0,	11,	0,	9 },
		{ 575,	0,	4,	0,	9 },
		{ 584,	11,	13,	0,	9 },
		{ 593,	0,	4,	0,	9 },
		{ 602,	4,	11,	0,	9 },
		{ 611,	0,	11,	0,	9 },
		{ 620,	4,	11,	0,	9 },
		{ 629,	0,	11,	0,	9 },
		{ 638,	4,	11,	0,	9 },
		{ 647,	0,	11,	0,	9 },
		{ 656,	4,	13,	0,	9 },
		{ 665,	0,	11,	0,	9 },
		{ 674,	1,	11,	0,	9 },
		{ 683,	1,	13,	0,	9 },
		{ 692,	0,	11,	0,	9 },
		{ 701,	0,	11,	0,	9 },
		{ 710,	4,	11,	0,	9 },
		{ 719,	4,	11,	0,	9 },
		{ 728,	4,	11,	0,	9 },
		{ 737,	4,	13,	0,	9 },
		{ 746,	4,	13,	0,	9 },
		{ 755,	4,	11,	0,	9 },
		{ 764,	4,	11,	0,	9 },
		{ 773,	1,	11,	0,	9 },
		{ 782,	4,	11,	0,	9 },
		{ 791,	4,	11,	0,	9 },
		{ 800,	4,	11,	0,	9 },
		{ 809,	4,	11,	0,	9 },
		{ 818,	4,	13,	0,	9 },
		{ 827,	4,	11,	0,	9 },
		{ 836,	0,	11,	0,	9 },
		{ 845,	0,	13,	0,	9 },
		{ 854,	0,	11,	0,	9 },
		{ 863,	3,	7,	0,	9 },
		{ 872,	0,	0,	0,	0 },
	}
};
