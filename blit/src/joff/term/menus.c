#include "common.h"
#include "../menu.h"

char *m_main[] =
	{ M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT,
	  M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, 0 };

char *m_scratch[] =
	{ M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT,
	  M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT,
	  M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT,
	  M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT,
	  M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT,
	  M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT,
	  M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT,
	  M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT,
	  0 };

char *m_callret[] = { M_SLOT, M_SLOT, M_SLOT, M_SLOT, 0 };

char *m_memory[] =
	{ M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT,
	  M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, M_SLOT, 0 };

static char digits[10];
static int index;

static pic9( n )
long n;
{
	if(n/10) pic9(n/10);
	digits[index++] = "0123456789"[n%10];
}

char *decimal(i)
long i;
{
/*	if( i < 0 ) return 0;			allow negative */
	digits[0] = ' ';
	index = 1;
	if( i < 0 ){
		digits[index++] = '-';
		i = -i;
	}
	pic9((long)i);
	digits[index++] = ' ';
	digits[index] = '\0';
	return digits;
}

/*bvpos(n)
{
	register i;

	for( i = 0; i < M_BVSIZE; ++i )
		if( (bitvector[i>>3]>>(i&7)) & 1 ){
			if( n-- == 0 ) return i;
		}
	return -1;
}*/

bvpos(n)
register n;		/* used on host and jerq! */
{
	register i, j, v;

	for( i = 0; i < M_BVSIZE/8; ++i ) if( v = bitvector[i] ){
		for( j = 0; j < 8; ++j ){
			if( (v&1) && n--==0 ) return i*8+j;
			v >>= 1;
		}
	}
	return -1;
}

char *bvgen(i)
{
	i = bvpos(i);
	return i < 0 ? (char *)0 : decimal((long) i);
}

char *limitsgen(i)
{
	if( i > hinumeric-lonumeric ) return 0;
	return decimal((long)(i+lonumeric));
}

NewMenu menus[] = { {0},
	{m_main}, {m_memory}, {m_scratch}, {m_callret}, { 0, 0 }, {0} }; 

int currmenu, currbutt, buttmenu[4];

char m_strings[100][M_SLOTLEN+1];

m_init()
{
	register m, i, s = 0;

	for( m = 1; menus[m].item; ++m )
		for( i = 0; menus[m].item[i]; ++i )
			menus[m].item[i] = &(m_strings[s++][0]);
}
