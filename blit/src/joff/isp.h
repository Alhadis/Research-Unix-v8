
#define TRAP_BPT (0x4E40)     /* trap	0		*/
#define JSR_ABS  (0x4EB9)     /* jsr	function	*/
#define LINK_FP  (0x4E56)     /* link	fp,delta	*/
#define MOVML	 (0x48EE)     /* movm.l mask,offset(fp) */
#define MOVMLRTS (0x4CEE)     /* movm.l offset(fp),mask */
#define MOVL_ABS (0x2079)     /* mov.l	addr,%a0	*/
#define MOVL_IM  (0x207C)     /* mov.l	&const,%a0	*/
#define JSR_IND  (0x4E90)     /* jsr	(%a0)		*/

#define FPROLOG 10	      /* sizeof LINK; MOVM	*/
#define FEPILOG 10	      /* sizeof MOVM;UNLK;RTS	*/
