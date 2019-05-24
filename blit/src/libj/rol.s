text
global rol
rol:
	mov.w	4(%sp), %d0
	mov.w	6(%sp), %d1
	rol.w	%d1, %d0
	rts

global ror
ror:
	mov.w	4(%sp), %d0
	mov.w	6(%sp), %d1
	ror.w	%d1, %d0
	rts
	
