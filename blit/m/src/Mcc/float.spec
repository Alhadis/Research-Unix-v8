Here are the specs for the floating point routines:
The routines for unary ops take their argument in %d0, return their
result in %d0, and may freely use %d1, %a0, and %a1; any other
registers must be saved and restored across the call.
The routines for binary ops take their first (left hand) argument in %d0,
and their second argument from the top of the stack: it can be obtained
in the called program by 8(%fp).  As before, %d1, %a0, and %a1 are free to
be used, and the result is placed into %d0.
There are some routines for long arithmetic that should be consulted;
they are called the same way.  dmr or rob can probably direct you to the source.

Unary routines:
	flneg%%		negation
	itof%%		convert from int to float
	uitof%%		convert from unsigned int to float
	ltof%%		convert from long to float
	ultof%%		convert from unsigned long to float
	ftol%%		convert from float to long
	fltst%%		test a float for ==0, !=0, >0, <0, etc.
			returns in %d0 a word (16-bit) quantity that,
			when tested against 0, gives the same results as
			the floating point number would.

Binary routines
	fladd%%		addition
	flsum%%		subtraction
	flmul%%		multiplication
	fldiv%%		division (don't know what to do about faults)
	flcmp%%		comparison (returns result like fltst%% above)
