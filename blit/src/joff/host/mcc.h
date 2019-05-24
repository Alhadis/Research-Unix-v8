/*
	SCJ
*/ 

# define TNULL PTR      /* pointer to UNDEF */
# define TVOID FTN	/* function returning UNDEF (for void) */
# define UNDEF 0
# define FARG 1
# define CHAR 2
# define SHORT 3
# define INT 4
# define LONG 5
# define FLOAT 6
# define DOUBLE 7
# define STRTY 8
# define UNIONTY 9
# define ENUMTY 10
# define MOETY 11
# define UCHAR 12
# define USHORT 13
# define UNSIGNED 14
# define ULONG 15
# define VOID 16

# define ASG 1+
# define UNARY 2+
# define NOASG (-1)+
# define NOUNARY (-2)+

# define PTR  0040
# define FTN  0100
# define ARY  0140

# define BTMASK 0x1F
# define BTSHIFT 5
# define TSHIFT 2
# define C_TMASK 0140
# define ISPTR(x) ((x&C_TMASK)==PTR)
# define ISFTN(x)  ((x&C_TMASK)==FTN)  /* is x a function type */
# define ISARY(x)   ((x&C_TMASK)==ARY)   /* is x an array type */
# define INCREF(x) (((x&~BTMASK)<<TSHIFT)|PTR|(x&BTMASK))
# define DECREF(x) (((x>>TSHIFT)&~BTMASK)|(x&BTMASK))

