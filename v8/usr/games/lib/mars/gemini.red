;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Gemini, a sample Redcode program
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	name	'gemini'
src	data	0		
dst	data	96
start	mov	@src	@dst
	cmp	src	#9
	jmp	finish
	add	#1	src
	add	#1	dst
	jmp	start
finish	mov	#96	90
	jmp	90
	end	start
