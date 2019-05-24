;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Dwarf, a sample Redcode program
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	name	'dwarf'
site	data	-1		; address of last 0 'bomb'
start	add	#5	site	; move site forward 5 locations
	mov	#0	@site	; write 0 'bomb'
	jmp	start		; loop
	end	start
