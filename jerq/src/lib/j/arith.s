	.file	"arith.c"
	.data
	.text
	.align	4
	.def	add;	.val	add;	.scl	2;	.type	050;	.endef
	.globl	add
add:
	save	&.R1
	addw2	&.F1,%sp
	movaw	0(%ap),%r8
	movaw	4(%ap),%r7
	movw	%r8,%r0
	addw2	&2,%r8
	movw	%r7,%r1
	addw2	&2,%r7
	addh2	0(%r1),0(%r0)
	addh2	0(%r7),0(%r8)
	movw	0(%ap),%r0
	jmp	.L30
.L30:
	.def	.ef;	.val	.;	.scl	101;	.line	6;	.endef
	.ln	6
	.set	.F1,0
	.set	.R1,2
	ret	&.R1
	.def	add;	.val	.;	.scl	-1;	.endef
	.data
	.text
	.align	4
	.def	sub;	.val	sub;	.scl	2;	.type	050;	.endef
	.globl	sub
sub:
	save	&.R2
	addw2	&.F2,%sp
	movaw	0(%ap),%r8
	movaw	4(%ap),%r7
	movw	%r8,%r0
	addw2	&2,%r8
	movw	%r7,%r1
	addw2	&2,%r7
	subh2	0(%r1),0(%r0)
	subh2	0(%r7),0(%r8)
	movw	0(%ap),%r0
	jmp	.L31
.L31:
	.def	.ef;	.val	.;	.scl	101;	.line	6;	.endef
	.ln	6
	.set	.F2,0
	.set	.R2,2
	ret	&.R2
	.def	sub;	.val	.;	.scl	-1;	.endef
	.data
	.text
	.align	4
	.def	inset;	.val	inset;	.scl	2;	.type	050;	.endef
	.globl	inset
inset:
	save	&.R3
	addw2	&.F3,%sp
	movw	8(%ap),%r8
	movw	%r2,0(%fp)
	movaw	0(%ap),%r7
	movw	%r7,%r0
	addw2	&2,%r7
	movtwh	%r8,%r1
	addh2	%r1,0(%r0)
	movw	%r7,%r0
	addw2	&2,%r7
	movtwh	%r8,%r1
	addh2	%r1,0(%r0)
	movw	%r7,%r0
	addw2	&2,%r7
	movtwh	%r8,%r1
	subh2	%r1,0(%r0)
	movtwh	%r8,%r0
	subh2	%r0,0(%r7)
	movw	0(%fp),%r1
	movaw	0(%ap),%r0
	movw	4(%r0),4(%r1)
	movw	0(%r0),0(%r1)
	jmp	.L32
.L32:
	.def	.ef;	.val	.;	.scl	101;	.line	8;	.endef
	.ln	8
	movw	0(%fp),%r0
	.set	.F3,4
	.set	.R3,2
	ret	&.R3
	.def	inset;	.val	.;	.scl	-1;	.endef
	.data
	.text
	.align	4
	.def	div;	.val	div;	.scl	2;	.type	050;	.endef
	.globl	div
div:
	save	&.R4
	addw2	&.F4,%sp
	movw	4(%ap),%r8
	movaw	0(%ap),%r7
	movw	%r7,%r0
	addw2	&2,%r7
	movw	%r0,0(%fp)
	movbhw	*0(%fp),%r0
	divw2	%r8,%r0
	movh	%r0,*0(%fp)
	movbhw	0(%r7),%r0
	divw2	%r8,%r0
	movh	%r0,0(%r7)
	movw	0(%ap),%r0
	jmp	.L33
.L33:
	.def	.ef;	.val	.;	.scl	101;	.line	6;	.endef
	.ln	6
	.set	.F4,4
	.set	.R4,2
	ret	&.R4
	.def	div;	.val	.;	.scl	-1;	.endef
	.data
	.text
	.align	4
	.def	mul;	.val	mul;	.scl	2;	.type	050;	.endef
	.globl	mul
mul:
	save	&.R5
	addw2	&.F5,%sp
	movw	4(%ap),%r8
	movaw	0(%ap),%r7
	movw	%r7,%r0
	addw2	&2,%r7
	movtwh	%r8,%r1
	MULH2	%r1,0(%r0)
	movtwh	%r8,%r0
	MULH2	%r0,0(%r7)
	movw	0(%ap),%r0
	jmp	.L34
.L34:
	.def	.ef;	.val	.;	.scl	101;	.line	6;	.endef
	.ln	6
	.set	.F5,0
	.set	.R5,2
	ret	&.R5
	.def	mul;	.val	.;	.scl	-1;	.endef
	.data
	.text
	.align	4
	.def	rsubp;	.val	rsubp;	.scl	2;	.type	050;	.endef
	.globl	rsubp
rsubp:
	save	&.R6
	addw2	&.F6,%sp
	movw	%r2,0(%fp)
	movaw	0(%ap),%r8
	movaw	8(%ap),%r7
	movw	%r8,%r0
	addw2	&2,%r8
	movw	%r7,%r1
	addw2	&2,%r7
	subh2	0(%r1),0(%r0)
	movw	%r8,%r0
	addw2	&2,%r8
	movw	%r7,%r1
	subw2	&2,%r7
	subh2	0(%r1),0(%r0)
	movw	%r8,%r0
	addw2	&2,%r8
	movw	%r7,%r1
	addw2	&2,%r7
	subh2	0(%r1),0(%r0)
	subh2	0(%r7),0(%r8)
	movw	0(%fp),%r1
	movaw	0(%ap),%r0
	movw	4(%r0),4(%r1)
	movw	0(%r0),0(%r1)
	jmp	.L35
.L35:
	.def	.ef;	.val	.;	.scl	101;	.line	8;	.endef
	.ln	8
	movw	0(%fp),%r0
	.set	.F6,4
	.set	.R6,2
	ret	&.R6
	.def	rsubp;	.val	.;	.scl	-1;	.endef
	.data
	.text
	.align	4
	.def	raddp;	.val	raddp;	.scl	2;	.type	050;	.endef
	.globl	raddp
raddp:
	save	&.R7
	addw2	&.F7,%sp
	movw	%r2,0(%fp)
	movaw	0(%ap),%r8
	movaw	8(%ap),%r7
	movw	%r8,%r0
	addw2	&2,%r8
	movw	%r7,%r1
	addw2	&2,%r7
	addh2	0(%r1),0(%r0)
	movw	%r8,%r0
	addw2	&2,%r8
	movw	%r7,%r1
	subw2	&2,%r7
	addh2	0(%r1),0(%r0)
	movw	%r8,%r0
	addw2	&2,%r8
	movw	%r7,%r1
	addw2	&2,%r7
	addh2	0(%r1),0(%r0)
	addh2	0(%r7),0(%r8)
	movw	0(%fp),%r1
	movaw	0(%ap),%r0
	movw	4(%r0),4(%r1)
	movw	0(%r0),0(%r1)
	jmp	.L36
.L36:
	.def	.ef;	.val	.;	.scl	101;	.line	8;	.endef
	.ln	8
	movw	0(%fp),%r0
	.set	.F7,4
	.set	.R7,2
	ret	&.R7
	.def	raddp;	.val	.;	.scl	-1;	.endef
	.data
	.text
	.align	4
	.def	eqpt;	.val	eqpt;	.scl	2;	.type	044;	.endef
	.globl	eqpt
eqpt:
	save	&.R8
	addw2	&.F8,%sp
	movaw	0(%ap),%r8
	movaw	4(%ap),%r7
	cmpw	0(%r8),0(%r7)
	jne	.L39
	movw	&1,%r0
	jmp	.L40
.L39:
	movw	&0,%r0
.L40:
	jmp	.L38
.L38:
	.def	.ef;	.val	.;	.scl	101;	.line	4;	.endef
	.ln	4
	.set	.F8,0
	.set	.R8,2
	ret	&.R8
	.def	eqpt;	.val	.;	.scl	-1;	.endef
	.data
	.text
	.align	4
	.def	eqrect;	.val	eqrect;	.scl	2;	.type	044;	.endef
	.globl	eqrect
eqrect:
	save	&.R9
	addw2	&.F9,%sp
	movaw	0(%ap),%r8
	movaw	8(%ap),%r7
	movw	%r8,%r0
	addw2	&4,%r8
	movw	%r7,%r1
	addw2	&4,%r7
	cmpw	0(%r0),0(%r1)
	jne	.L43
	cmpw	0(%r8),0(%r7)
	jne	.L43
.L45:
	movw	&1,%r0
	jmp	.L44
.L43:
	movw	&0,%r0
.L44:
	jmp	.L42
.L42:
	.def	.ef;	.val	.;	.scl	101;	.line	4;	.endef
	.ln	4
	.set	.F9,0
	.set	.R9,2
	ret	&.R9
	.def	eqrect;	.val	.;	.scl	-1;	.endef
	.data
