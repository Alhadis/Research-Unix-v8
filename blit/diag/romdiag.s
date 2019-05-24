long	0
long	START
	data	1
	even
	global	display
display:
	long	24576
	short	50
	short	0
	short	0
	short	800
	short	1024
	space	4
	text
	global	main
main:
	link	%fp,&F%1
	movm.l	&M%1,S%1(%fp)
START:
L%36:
	mov.w	&0,393248
	mov.w	&6144,393240
	mov.l	&3,%d0
	mov.b	%d0,393264
	mov.l	&17,%d0
	mov.b	%d0,393264
	mov.l	&24576,%a2
	mov.l	&25600,%d2
L%39:
	mov.l	&0,(%a2)+
L%38:
	sub.l	&1,%d2
	bne	L%39
L%37:
	mov.l	&24576,%a2
	mov.l	&1,%d6
L%42:
	mov.l	&0,%d5
L%45:
	mov.l	&1,%d0
	mov.b	393264,%d1
	ext.w	%d1
	and.w	%d0,%d1
	beq	L%46
	cmp.b	393266,&13
	bne	L%46
	br	L%47
L%46:
	mov.l	&0,%d3
L%50:
	cmp.w	%d3,&800
	bge	L%49
	mov.w	%d3,%d0
	eor.w	%d5,%d0
	and.w	&1023,%d0
	mov.w	%d0,%d4
	mov.w	%d3,%d0
	asr.w	&3,%d0
	mov.l	&25,%d1
	muls.w	%d4,%d1
	ext.l	%d1
	lsl.l	&2,%d1
	lea.l	0(%a2,%d1.l),%a1
	lea.l	0(%a1,%d0.w),%a3
	mov.l	&7,%d0
	and.w	%d3,%d0
	mov.w	&128,%d1
	asr.w	%d0,%d1
	eor.b	%d1,(%a3)
L%48:
	add.w	&1,%d3
	br	L%50
L%49:
	add.w	%d6,%d5
L%44:
	mov.l	&10,%d0
	mov.w	%d6,%d1
	lsl.w	%d0,%d1
	cmp.w	%d5,%d1
	bne	L%45
L%43:
L%51:
	add.w	&1,%d6
	mov.l	&2,%d5
L%54:
	cmp.w	%d5,%d6
	bge	L%53
	mov.w	%d6,%d0
	ext.l	%d0
	divs.w	%d5,%d0
	muls.w	%d5,%d0
	cmp.w	%d0,%d6
	bne	L%55
	br	L%51
L%55:
L%52:
	add.w	&1,%d5
	br	L%54
L%53:
L%40:
	br	L%42
L%41:
L%47:
	mov.l	&24576,%a2
	mov.l	&25600,%d2
L%58:
	mov.l	&0,(%a2)+
L%57:
	sub.l	&1,%d2
	bne	L%58
L%56:
	mov.w	&1,393248
L%61:
	mov.l	&0,%d5
L%64:
	cmp.w	%d5,&32
	bge	L%63
	mov.l	&1,%d0
	mov.b	393264,%d1
	ext.w	%d1
	and.w	%d0,%d1
	beq	L%65
	cmp.b	393266,&13
	bne	L%65
	br	L%66
L%65:
	mov.l	&-2147483648,%d0
	lsr.l	%d5,%d0
	mov.l	%d0,%d7
	mov.l	&24576,%a2
	mov.l	&25600,%d2
L%69:
	mov.l	%d7,(%a2)+
L%68:
	sub.l	&1,%d2
	bne	L%69
L%67:
	mov.l	&200000,%d2
L%72:
L%71:
	sub.l	&1,%d2
	bne	L%72
L%70:
	mov.l	&24576,%a2
	mov.l	&25600,%d2
L%75:
	mov.l	&0,(%a2)+
L%74:
	sub.l	&1,%d2
	bne	L%75
L%73:
L%62:
	add.w	&1,%d5
	br	L%64
L%63:
L%59:
	br	L%61
L%60:
L%66:
	mov.l	&24576,%a2
	mov.l	&25600,%d2
L%78:
	mov.l	&0,(%a2)+
L%77:
	sub.l	&1,%d2
	bne	L%78
L%76:
L%81:
	mov.l	&0,%d5
L%84:
	cmp.w	%d5,&32
	bge	L%83
	mov.l	&1,%d0
	mov.b	393264,%d1
	ext.w	%d1
	and.w	%d0,%d1
	beq	L%85
	cmp.b	393266,&13
	bne	L%85
	br	L%86
L%85:
	mov.l	&7,%d0
	and.w	%d5,%d0
	mov.w	&128,%d1
	asr.w	%d0,%d1
	ext.l	%d1
	mov.l	%d1,%d7
	mov.w	%d5,%d0
	asr.w	&3,%d0
	add.w	&24576,%d0
	mov.w	%d0,%a0
	mov.l	%a0,%a3
	mov.l	&25600,%d2
L%89:
	mov.l	%d7,%d0
	mov.b	%d0,(%a3)
	add.l	&4,%a3
L%88:
	sub.l	&1,%d2
	bne	L%89
L%87:
	mov.l	&200000,%d2
L%92:
L%91:
	sub.l	&1,%d2
	bne	L%92
L%90:
	mov.l	&24576,%a2
	mov.l	&25600,%d2
L%95:
	mov.l	&0,(%a2)+
L%94:
	sub.l	&1,%d2
	bne	L%95
L%93:
L%82:
	add.w	&1,%d5
	br	L%84
L%83:
L%79:
	br	L%81
L%80:
L%86:
	mov.l	&24576,%a2
	mov.l	&25600,%d2
L%98:
	mov.l	&0,(%a2)+
L%97:
	sub.l	&1,%d2
	bne	L%98
L%96:
	mov.l	&256*1024-4, %sp	# set up stack
	jsr	autotest
	mov.w	&0x2700, %sr
	br	L%36
L%34:
	movm.l	S%1(%fp),&M%1
	unlk	%fp
	rts
	set	S%1,-32
	set	T%1,-32
	set	F%1,-36
	set	M%1,06374
	data	1
	text
	global	Clear
Clear:
	link	%fp,&F%2
	movm.l	&M%2,S%2(%fp)
	mov.l	&24576,%a2
	mov.w	&25600,%d2
L%104:
	mov.l	&0,(%a2)+
L%103:
	sub.w	&1,%d2
	bne	L%104
L%102:
	mov.w	&3392,%d2
L%107:
L%106:
	sub.w	&1,%d2
	bne	L%107
L%105:
L%101:
	movm.l	S%2(%fp),&M%2
	unlk	%fp
	rts
	set	S%2,-8
	set	T%2,-8
	set	F%2,-12
	set	M%2,02004
	data	1
	text
	global	autotest
autotest:
	link	%fp,&F%3
	movm.l	&M%3,S%3(%fp)
	mov.w	&2,(%sp)
	jsr	putkbd
	mov.w	&0,-2+S%3(%fp)
L%116:
	cmp.w	-2+S%3(%fp),&128
	bge	L%115
	mov.l	&1,%d0
	mov.w	-2+S%3(%fp),%d1
	add.w	%d1,%d1
	or.w	%d0,%d1
	mov.w	%d1,(%sp)
	jsr	putkbd
L%114:
	add.w	&1,-2+S%3(%fp)
	br	L%116
L%115:
	mov.w	&1,(%sp)
	jsr	putkbd
	mov.w	&4,(%sp)
	jsr	putkbd
	data	2
L%118:
	byte	0153,0145,0171,0142,0157,0141,0162,0144
	byte	00
	text
	mov.l	&L%118,(%sp)
	jsr	herald
	mov.w	&30,(%sp)
	mov.w	&5,-(%sp)
	mov.l	&kbdchar,-(%sp)
	jsr	kbdtest
	add.l	&6,%sp
	jsr	intkbdin
	data	2
L%121:
	byte	0151,0156,0164,055,0153,0145,0171,0142
	byte	0157,0141,0162,0144,00
	text
	mov.l	&L%121,(%sp)
	jsr	herald
	mov.w	&30,(%sp)
	mov.w	&5,-(%sp)
	mov.l	&intkbdch,-(%sp)
	jsr	kbdtest
	add.l	&6,%sp
	data	2
L%122:
	byte	0155,0157,0165,0163,0145,00
	text
	mov.l	&L%122,(%sp)
	jsr	herald
	mov.w	&65,(%sp)
	mov.l	&always,-(%sp)
	jsr	mousetes
	add.l	&4,%sp
	jsr	Clear
	jsr	intmouse
	data	2
L%125:
	byte	0151,0156,0164,055,0155,0157,0165,0163
	byte	0145,00
	text
	mov.l	&L%125,(%sp)
	jsr	herald
	mov.w	&66,(%sp)
	mov.l	&sixtyhz,-(%sp)
	jsr	mousetes
	add.l	&4,%sp
	jsr	Clear
	jsr	bounce1
	jsr	Clear
L%108:
	movm.l	S%3(%fp),&M%3
	unlk	%fp
	rts
	set	S%3,0
	set	T%3,-4
	set	F%3,-8
	set	M%3,00
	data	1
	text
	global	herald
herald:
	link	%fp,&F%4
	movm.l	&M%4,S%4(%fp)
	mov.w	&3,(%sp)
	mov.w	&10,-(%sp)
	mov.w	&5,-(%sp)
	mov.l	&display,-(%sp)
	mov.l	8(%fp),-(%sp)
	mov.l	&defont,-(%sp)
	jsr	string
	add.l	&16,%sp
L%127:
	movm.l	S%4(%fp),&M%4
	unlk	%fp
	rts
	set	S%4,0
	set	T%4,0
	set	F%4,-4
	set	M%4,00
	data	1
	text
	global	putkbd
putkbd:
	link	%fp,&F%5
	movm.l	&M%5,S%5(%fp)
L%129:
	mov.l	&2,%d0
	mov.b	393264,%d1
	ext.w	%d1
	and.w	%d0,%d1
	bne	L%130
	br	L%129
L%130:
	mov.w	8(%fp),%d0
	mov.b	%d0,393266
L%128:
	movm.l	S%5(%fp),&M%5
	unlk	%fp
	rts
	set	S%5,0
	set	T%5,-2
	set	F%5,-6
	set	M%5,00
	data	1
	text
	global	bounce1
bounce1:
	link	%fp,&F%6
	movm.l	&M%6,S%6(%fp)
	mov.l	&47,%d0
	mov.w	Jrect+4,%d1
	sub.w	%d0,%d1
	mov.w	%d1,-2+S%6(%fp)
	mov.l	&47,%d0
	mov.w	Jrect+6,%d1
	sub.w	%d0,%d1
	mov.w	%d1,-4+S%6(%fp)
	mov.w	&1,-10+S%6(%fp)
	mov.w	&1,-12+S%6(%fp)
	mov.w	&3,(%sp)
	mov.l	&Jrect,%a0
	mov.l	4(%a0),-(%sp)
	mov.l	0(%a0),-(%sp)
	mov.l	&display,-(%sp)
	jsr	rectf
	add.l	&12,%sp
	mov.w	Jrect,-6+S%6(%fp)
	mov.w	Jrect+2,-8+S%6(%fp)
	mov.b	&0,260
L%135:
	cmp.b	260,&13
	beq	L%134
	mov.w	&3,(%sp)
	mov.l	&47,%d0
	add.w	-8+S%6(%fp),%d0
	mov.w	%d0,-(%sp)
	mov.l	&47,%d0
	add.w	-6+S%6(%fp),%d0
	mov.w	%d0,-(%sp)
	mov.w	-8+S%6(%fp),-(%sp)
	mov.w	-6+S%6(%fp),-(%sp)
	mov.l	&display,-(%sp)
	jsr	rectf
	add.l	&12,%sp
	mov.w	-6+S%6(%fp),%d0
	add.w	-10+S%6(%fp),%d0
	mov.w	%d0,-6+S%6(%fp)
	mov.w	-6+S%6(%fp),%d0
	cmp.w	%d0,-2+S%6(%fp)
	beq	L%137
	mov.w	-6+S%6(%fp),%d0
	cmp.w	%d0,Jrect
	bne	L%136
L%137:
	mov.w	-10+S%6(%fp),%d0
	neg.w	%d0
	mov.w	%d0,-10+S%6(%fp)
L%136:
	mov.w	-8+S%6(%fp),%d0
	add.w	-12+S%6(%fp),%d0
	mov.w	%d0,-8+S%6(%fp)
	mov.w	-8+S%6(%fp),%d0
	cmp.w	%d0,-4+S%6(%fp)
	beq	L%139
	mov.w	-8+S%6(%fp),%d0
	cmp.w	%d0,Jrect+2
	bne	L%138
L%139:
	mov.w	-12+S%6(%fp),%d0
	neg.w	%d0
	mov.w	%d0,-12+S%6(%fp)
L%138:
L%133:
	br	L%135
L%134:
L%131:
	movm.l	S%6(%fp),&M%6
	unlk	%fp
	rts
	set	S%6,0
	set	T%6,-14
	set	F%6,-18
	set	M%6,00
	data	1
	text
	global	kbdtest
kbdtest:
	link	%fp,&F%7
	movm.l	&M%7,S%7(%fp)
	mov.b	&0,-1+S%7(%fp)
L%141:
	mov.l	8(%fp),%a0
	jsr	(%a0)
	mov.w	%d0,%d2
	mov.w	%d2,%d0
	cmp.w	%d0,&13
	beq	L%142
	mov.b	%d2,-2+S%7(%fp)
	mov.l	12(%fp),(%sp)
	mov.w	%d2,-(%sp)
	jsr	drawchar
	add.l	&2,%sp
	pea.l	-2+S%7(%fp)
	mov.l	&defont,-(%sp)
	jsr	strwidth
	add.l	&8,%sp
	add.w	%d0,12(%fp)
	br	L%141
L%142:
	jsr	Clear
L%140:
	movm.l	S%7(%fp),&M%7
	unlk	%fp
	rts
	set	S%7,-4
	set	T%7,-6
	set	F%7,-10
	set	M%7,04
	data	1
	text
	global	kbdchar
kbdchar:
	link	%fp,&F%8
	movm.l	&M%8,S%8(%fp)
L%148:
L%147:
	mov.l	&1,%d0
	mov.b	393264,%d1
	ext.w	%d1
	and.w	%d0,%d1
	beq	L%148
L%146:
	mov.b	393266,%d0
	ext.w	%d0
	and.w	&255,%d0
	br	L%145
L%145:
	movm.l	S%8(%fp),&M%8
	unlk	%fp
	rts
	set	S%8,0
	set	T%8,0
	set	F%8,-4
	set	M%8,00
	data	1
	text
	global	intkbdin
intkbdin:
	link	%fp,&F%9
	movm.l	&M%9,S%9(%fp)
	mov.l	&Auto2, 0x68
	mov.l	&3,%d0
	mov.b	%d0,393264
	mov.w	&149,%d0
	mov.b	%d0,393264
	mov.b	&0,260
	jsr	binit
	mov.w	&0x2100, %sr
L%149:
	movm.l	S%9(%fp),&M%9
	unlk	%fp
	rts
	set	S%9,0
	set	T%9,0
	set	F%9,-4
	set	M%9,00
	data	1
	text
	global	auto2
auto2:
	link	%fp,&F%10
	movm.l	&M%10,S%10(%fp)
	mov.b	393266,260
L%152:
	movm.l	S%10(%fp),&M%10
	unlk	%fp
	rts
	set	S%10,0
	set	T%10,0
	set	F%10,-4
	set	M%10,00
	data	1
	text
	global	intkbdch
intkbdch:
	link	%fp,&F%11
	movm.l	&M%11,S%11(%fp)
L%156:
L%155:
	tst.b	260
	beq	L%156
L%154:
	mov.b	260,%d0
	ext.w	%d0
	mov.w	%d0,%d2
	mov.b	&0,260
	mov.w	%d2,%d0
	br	L%153
L%153:
	movm.l	S%11(%fp),&M%11
	unlk	%fp
	rts
	set	S%11,-4
	set	T%11,-4
	set	F%11,-8
	set	M%11,04
	data	1
	text
	global	always
always:
	link	%fp,&F%12
	movm.l	&M%12,S%12(%fp)
	mov.w	&20000,%d2
L%160:
L%159:
	mov.w	%d2,%d0
	sub.w	&1,%d2
	tst.w	%d0
	bne	L%160
L%158:
	br	L%157
L%157:
	movm.l	S%12(%fp),&M%12
	unlk	%fp
	rts
	set	S%12,-4
	set	T%12,-4
	set	F%12,-8
	set	M%12,04
	data	1
	text
	global	sixtyhz
sixtyhz:
	link	%fp,&F%13
	movm.l	&M%13,S%13(%fp)
	mov.w	&0,262
L%164:
L%163:
	tst.w	262
	beq	L%164
L%162:
	br	L%161
L%161:
	movm.l	S%13(%fp),&M%13
	unlk	%fp
	rts
	set	S%13,0
	set	T%13,0
	set	F%13,-4
	set	M%13,00
	data	1
	text
	global	intmouse
intmouse:
	link	%fp,&F%14
	movm.l	&M%14,S%14(%fp)
	mov.l	&Auto1, 0x64
	mov.w	&0x2000, %sr
L%165:
	movm.l	S%14(%fp),&M%14
	unlk	%fp
	rts
	set	S%14,0
	set	T%14,0
	set	F%14,-4
	set	M%14,00
	data	1
	text
	global	auto1
auto1:
	link	%fp,&F%15
	movm.l	&M%15,S%15(%fp)
	mov.w	&0, 384*1024+070
	mov.w	&1,262
L%167:
	movm.l	S%15(%fp),&M%15
	unlk	%fp
	rts
	set	S%15,0
	set	T%15,0
	set	F%15,-4
	set	M%15,00
	data	1
	text
	global	mousetes
mousetes:
	link	%fp,&F%16
	movm.l	&M%16,S%16(%fp)
	mov.l	&10,%d2
	mov.l	&10,%d3
	mov.w	%d3,(%sp)
	mov.w	%d2,-(%sp)
	mov.w	12(%fp),-(%sp)
	jsr	drawchar
	add.l	&4,%sp
	mov.b	&0,260
L%171:
	cmp.b	260,&13
	beq	L%170
	mov.w	393218,%d0
	and.w	&1023,%d0
	mov.w	%d0,%d4
	mov.w	393216,%d0
	and.w	&1023,%d0
	mov.w	%d0,%d5
	tst.w	%d4
	blt	L%173
	tst.w	%d5
	bge	L%172
L%173:
	br	L%169
L%172:
	mov.w	%d3,(%sp)
	mov.w	%d2,-(%sp)
	mov.w	12(%fp),-(%sp)
	jsr	drawchar
	add.l	&4,%sp
	cmp.w	%d4,&10
	bge	L%174
	mov.l	&10,%d4
L%174:
	cmp.w	%d5,&10
	bge	L%175
	mov.l	&10,%d5
L%175:
	cmp.w	%d4,&790
	ble	L%176
	mov.w	&790,%d4
L%176:
	cmp.w	%d5,&1014
	ble	L%177
	mov.w	&1014,%d5
L%177:
	mov.w	&800,%d0
	sub.w	%d4,%d0
	mov.w	%d0,%d4
	mov.w	&1024,%d0
	sub.w	%d5,%d0
	mov.w	%d0,%d5
	mov.w	%d5,(%sp)
	mov.w	%d4,-(%sp)
	mov.w	12(%fp),-(%sp)
	jsr	drawchar
	add.l	&4,%sp
	mov.w	%d4,%d2
	mov.w	%d5,%d3
L%169:
	mov.l	8(%fp),%a0
	jsr	(%a0)
	br	L%171
L%170:
	mov.w	%d3,(%sp)
	mov.w	%d2,-(%sp)
	mov.w	12(%fp),-(%sp)
	jsr	drawchar
	add.l	&4,%sp
L%168:
	movm.l	S%16(%fp),&M%16
	unlk	%fp
	rts
	set	S%16,-16
	set	T%16,-16
	set	F%16,-20
	set	M%16,074
	data	1
	text
	global	drawchar
drawchar:
	link	%fp,&F%17
	movm.l	&M%17,S%17(%fp)
	mov.w	8(%fp),%d0
	mov.b	%d0,-2+S%17(%fp)
	mov.b	&0,-1+S%17(%fp)
	mov.w	&3,(%sp)
	mov.l	10(%fp),-(%sp)
	mov.l	&display,-(%sp)
	pea.l	-2+S%17(%fp)
	mov.l	&defont,-(%sp)
	jsr	string
	add.l	&16,%sp
L%178:
	movm.l	S%17(%fp),&M%17
	unlk	%fp
	rts
	set	S%17,0
	set	T%17,-2
	set	F%17,-6
	set	M%17,00
	data	1
	text
	global	binit
binit:
	link	%fp,&F%18
	movm.l	&M%18,S%18(%fp)
	mov.l	&Auto4, 0x70
L%179:
	movm.l	S%18(%fp),&M%18
	unlk	%fp
	rts
	set	S%18,0
	set	T%18,-2
	set	F%18,-6
	set	M%18,00
	data	1
	text
	global	auto4
auto4:
	link	%fp,&F%19
	movm.l	&M%19,S%19(%fp)
	mov.b	393233,%d0
	ext.w	%d0
	and.w	&255,%d0
	mov.w	%d0,%d2
	mov.w	&2,(%sp)
	mov.w	&50,-(%sp)
	mov.w	&40,-(%sp)
	mov.w	&30,-(%sp)
	mov.w	&0,-(%sp)
	mov.l	&display,-(%sp)
	jsr	rectf
	add.l	&12,%sp
	mov.l	&1,%d0
	and.w	%d2,%d0
	beq	L%182
	mov.w	&30,(%sp)
	mov.w	&5,-(%sp)
	mov.w	&51,-(%sp)
	jsr	drawchar
	add.l	&4,%sp
	br	L%183
L%182:
	mov.l	&2,%d0
	and.w	%d2,%d0
	beq	L%184
	mov.w	&30,(%sp)
	mov.w	&5,-(%sp)
	mov.w	&50,-(%sp)
	jsr	drawchar
	add.l	&4,%sp
	br	L%185
L%184:
	mov.l	&4,%d0
	and.w	%d2,%d0
	beq	L%186
	mov.w	&30,(%sp)
	mov.w	&5,-(%sp)
	mov.w	&49,-(%sp)
	jsr	drawchar
	add.l	&4,%sp
L%186:
L%185:
L%183:
L%181:
	movm.l	S%19(%fp),&M%19
	unlk	%fp
	rts
	set	S%19,-4
	set	T%19,-4
	set	F%19,-8
	set	M%19,04
	data	1
Auto1:
	movm.l	&0xC0C0, -(%sp)
	jsr	auto1
	movm.l	(%sp)+, &0x0303
	rte
Auto2:
	movm.l	&0xC0C0, -(%sp)
	jsr	auto2
	movm.l	(%sp)+, &0x0303
	rte
Auto4:
	movm.l	&0xC0C0, -(%sp)
	jsr	auto4
	movm.l	(%sp)+, &0x0303
	rte
	text
	global	STARTOFM
STARTOFM:
	link	%fp,&F%20
	movm.l	&M%20,S%20(%fp)
L%187:
	movm.l	S%20(%fp),&M%20
	unlk	%fp
	rts
	set	S%20,0
	set	T%20,0
	set	F%20,-4
	set	M%20,00
	data	1
