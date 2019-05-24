LL0:
	.data
	.comm	_devsw,0
	.comm	_b,16384
	.comm	_blknos,16
	.comm	_iob,16864
	.comm	_cpu,4
	.comm	_mbaddr,4
	.comm	_mbaact,4
	.comm	_umaddr,4
	.comm	_ubaddr,4
	.data
	.align	1
	.globl	_tsstd
_tsstd:
	.long	0xf550
	.comm	_ctsbuf,4216
	.comm	_ts_uba,2
	.data
	.align	2
	.globl	_tsaddr
_tsaddr:
	.long	0
	.comm	_ts,32
	.text
	.align	1
	.globl	_tsopen
_tsopen:
	.word	L27
	jbr 	L29
L30:
	movl	4(ap),r11
	.lcomm	L31,4
	clrl	-4(fp)
	tstl	_tsaddr
	jneq	L32
	ashl	$-3,92(r11),r0
	ashl	$2,r0,r0
	addl3	r0,_umaddr,r0
	movzwl	_tsstd,r1
	bicl2	$-8192,r1
	addl3	r1,(r0),r0
	movl	r0,_tsaddr
L32:
L33:
	.data	1
L37:
	.ascii	"ts\72 not ready\12\0"
	.text
	ret
L35:
	jbr 	L33
L34:
	.data	1
L39:
	.ascii	"ts\72 offline\12\0"
	.text
	ret
L38:
L41:
L40:
L44:
	ret
	.set	L27,0x800
L29:
	subl2	$8,sp
	jbr 	L30
	.data
	.text
	.align	1
	.globl	_tsclose
_tsclose:
	.word	L46
	jbr 	L48
L49:
	movl	4(ap),r11
	ret
	.set	L46,0x800
L48:
	jbr 	L49
	.data
	.text
	.align	1
	.globl	_tsstrategy
_tsstrategy:
	.word	L50
	jbr 	L52
L53:
	movl	4(ap),r11
L54:
L55:
L57:
	jbr 	L57
L58:
	jbr 	L55
L56:
	jbr 	L60
L59:
L60:
	jbr 	L62
L61:
L63:
L62:
L66:
L67:
	jbr 	L67
L68:
L65:
L64:
L69:
	ret
L71:
	.data	1
L73:
	.ascii	"ts tape error\72 er=%b, xs0=%b\0"
	.text
	.data	1
L74:
	.ascii	"\10\20SC\17UPE\16SPE\15RMR\14NXM\13NBA\12A17\11A16\10SSR\7OFL\6FC1\5FC0\4TC2\3TC1\2TC0\1-\0"
	.text
	.data	1
L75:
	.ascii	"\10\20TMK\17RLS\16LET\15RLL\14WLE\13NEF\12ILC\11ILA\10MOT\7ONL\6IES\5VCK\4PED\3WLK\2BOT\1EO"
	.ascii	"T\0"
	.text
	.data	1
L77:
	.ascii	"ts\72 unrecovered error\12\0"
	.text
	ret
L76:
L78:
	jbr 	L54
L72:
	.data	1
L80:
	.ascii	"ts\72 recovered by retry\12\0"
	.text
L79:
	ret
	ret
	.set	L50,0xf00
L52:
	jbr 	L53
	.data
