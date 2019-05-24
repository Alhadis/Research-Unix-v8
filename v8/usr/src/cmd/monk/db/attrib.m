|include(macro.h)
|include(list.h)
|include(whitespace.h)
|include(style.h)

|attribute(page.offset; init;
	[
.		po \nOu
]
	$ [
.		po $
])

|attribute(line.length; init;
	[
.		ll \nWu
.		lt \nWu
]
	$ [
.		ll $
.		lt $
])

|attribute(page.length; init;
	[
.		pl \nLu
]
	$ [
.		pl $
])

|attribute(spacing; init off;
	on [
.		rs
]	off [
.		ns
])

|attribute(new.page; nostack;
	[
.		bp
])

|attribute(line.spacing; stack, init;
	$ [
.		ls $
]	[
.		ls 1
])

|attribute(blank.lines; nostack;
	$ [
.		sp $
]	[
.		sp 1
])

|attribute(horizontal.motion; nostack;
	$ [\h'$'])

|attribute(vertical.motion; nostack;
	$ [\v'$'])

|attribute(concatenate; nostack;
	[\c])

|attribute(new.line; nostack;
	[
.		br
])

|attribute(indent; stack, init;
	$ [
.		in $
]	[
'		in 0
])

|attribute(indent.line; nostack;
	$ [
.		ti $
]	[
'		ti 0
])

|attribute(fill; stack, init on;
	on [
.		fi
]	off [
.		nf
])

|attribute(adjust; stack, init both;
	right [
.		ad r
]	left [
.		ad l
]	both [
.		ad b
]	on [
.		ad
]	off [
.		na
])

|attribute(center; stack, default off;
	on [
.		ce 9999
]	off [
.		ce 0
])

|attribute(center.block; stack, default off;
	on [
.		di :1			\" begin second diversion
]	off [
.		br			\" get last partial line
.		di			\" end diversion of centered block
.		nr ;q \n(.u		\" save prevailing fill/no-fill mode
.		nf			\" bring it back in no-fill
.		nr ;L \n(.L		\" save line spacing
.		ls 1			\" output display in ls 1
.		nr ;i \n(.i		\" save prevailing indent
.		nr ;y 1			\" initialize width of block
.		if \n(dl>\n(;y .nr ;y \n(dl
.		if \n(;y<\n(.l .in (\n(.lu-\n(;yu)/2u \"indent by half of white-space
.		:1			\" text
.		if \n(;q .fi		\" restore fill if necessary
.		ls \n(;L		\" restore line spacing
.		in \n(;iu		\" restore prevailing indent
.		rr ;y			\" free storage
.		rm :1
])

|attribute(text; nostack;
	$ [$])

|attribute(error; nostack;
	$ [
.		tm "__FILE__":__LINE__: $
])

|attribute(width; stack, default full;
	narrow [
.		if \n(:C \{\
.			ll \nW		\" go narrow
.			lt \nW \}
]	full [
.		if \n(:C \{\
.			ll \n(:W	\" go wide
.			lt \n(:W \}
])

|attribute(here; stack, default off;
	on [
'\"	group mechanism
'\"	:0 store for initally diverted text
'\"	:1 store for centered block text
'\"	:C 0=one column, 1=first column, 2=second column
'\"	:W page width for full page output
'\"	:d prevent )h from outputting a floating group
'\"	:m initial footer place and partial footnote diverter trap
'\"	:o footer trap location
'\"	:r position at end of page header macro
'\"	:y flag indicating group/float in progress
'\"	W page width
'\"	;L remembered line spacing TEMP
'\"	;i remembered indent TEMP
'\"	;q remembered fill/no-fill mode TEMP
'\"	;y initialize width of block TEMP
.		if \n(:y .tm "__FILE__":__LINE__: Groups can not be nested
.		nr :y 1			\" set flag indicating group
.		br			\" output partial line before display
.		di :0			\" collect in :0
]	off [
'\"	Control page orientation of blocks.
'\"	Blocks are output as soon after their definition as
'\"	feasible.  The following rules are used:
'\"	     1. if the block will fit on the current page, output it there.
'\"	     2. if the block won't fit on any page, and we have used less than
'\"		half of the current page, then output the block on the
'\"		current page.
'\"	     3. skip to the next page.
.		br			\" get last partial line
.		di			\" end diversion
'\"	The following if condition tests: (1) if the block is too large to
'\"	fit on the current page and either (2) it will fit on an unused page
'\"	or (3) we have already used more than half of the current page. (the
'\"	three conditions are grouped in the form 1 & (2 : 3)). If this
'\"	total condition is true, then a page is ejected and the block output
'\"	on the next page.
.		nr :d 1			\" prevent )h from outputting a floating group
.		if (\n(dn>=\n(.t)&((\n(dn<(\n(.p-\n(:r-\n(:m)):\
(\n(nl>(\n(.p-\n(:r-\n(:o/2u+\n(:r)))\
.			ne \n(.tu+1v	\" spring the trap
.		nr ;q \n(.u		\" save prevailing fill/no-fill mode
.		nf			\" bring it back in no-fill
.		nr ;L \n(.L		\" save line spacing
.		ls 1			\" output display in ls 1
.		nr ;i \n(.i		\" save prevailing indent
.		in 0			\" indent back
.		rs			\" restore spacing
.		:0			\" text
.		nr :d 0			\" allow )h to output a floating group
.		if \n(;q .fi		\" restore fill if necessary
.		ls \n(;L		\" restore line spacing
.		in \n(;iu		\" restore prevailing indent
.		nr :y 0			\" reset flag indicating group
.		rr ;L ;i ;q		\" free storage
.		rm :0
])

|attribute(around; stack, default off;
	on [
'\"	float mechanism
'\"	!{a-z} width of stored floating 0=narrow, 1=wide
'\"	:0 store for initally diverted text
'\"	:1 store for centered block text
'\"	:C 0=one column, 1=first column, 2=second column
'\"	:W page width for full page output
'\"	:m initial footer place and partial footnote diverter trap
'\"	:o footer trap location
'\"	:r position at end of page header macro
'\"	:w output counter for floating keeps
'\"	:y flag indicating group/float in progress
'\"	:z input counter for floating keeps
'\"	W page width
'\"	?{a-z} text of stored floating
'\"	|{a-z} size of stored floating
'\"	;L prevailing line spacing
'\"	;f prevailing font
'\"	;i prevailing indent
'\"	;j prevailing adjust
'\"	;l prevailing line length
'\"	;o prevailing page offset ??????? bug with .po
'\"	;s prevailing point size
'\"	;u prevailing fill/no-fill mode
'\"	;v prevailing vertical spacing
'\"	;w convert to alpha format
'\"	;y initialize width of block
.		if \n(:y .tm "__FILE__":__LINE__: Groups can not be nested
.		nr :y 1			\" set flag indicating group
.		nr ;s \n(.s		\" save prevailing point size
.		nr ;f \n(.f		\" save prevailing font
.		nr ;u \n(.u		\" save prevailing fill/no-fill mode
.		nr ;j \n(.j		\" save prevailing adjust
.		nr ;v \n(.v		\" save prevailing vertical spacing
.		nr ;L \n(.L		\" save prevailing line spacing
.		nr ;l \n(.l		\" save prevailing line length
.		nr ;i \n(.i		\" save prevailing indent
.		ev 1			\" use footnote environment
.		ps \n(;s		\" restore point size
.		vs \n(;sp+2p		\" restore vertical spacing
.		ft \n(;f		\" restore font
.		nf			\" bring it back in no-fill
.		if \n(;u .fi		\" restore fill if necessary
.		ad \n(;j		\" restore page adjust
.		vs \n(;vu		\" restore vertical spacing
.		ls \n(;L		\" restore line spacing
.		ll \n(;lu		\" restore line length
.		in \n(;iu		\" restore indent
.		ti \n(;iu		\" turn off indentation
.		hy 14			\" turn on hyphenation
.		lt \n(;lu		\" restore title length
.		di :0			\" collect in :0
]	off [
'\"	Control page orientation of floats.
'\"	Floats are output as soon after their definition as
'\"	feasible.  The following rules are used:
'\"	     1. if the queue contains only this float and it will fit on the
'\"		current page, output it there.
'\"	     2. if the queue contains only this float and it won't fit on any
'\"		page, and we have used less than half of the current page,
'\"		then output the float on the current page.
'\"	     3. let the top of page macro handle it.
.		br			\" get last partial line
.		di			\" end diversion of centered block
.		ev			\" back to previous environment
.		nr :z \n(:z%26+1	\" get index of next output element
.		if \n(:z-\n(:w=0 .tm "__FILE__":__LINE__: too many floating displays
.		af ;w a			\" conversion register for display queue names
.		nr ;w \n(:z		\" form the next queue element name
.		rn :0 ?\n(;w		\" put the display into the queue
.		nr !\n(;w 0		\" narrow
.		if !\n(:C .nr !\n(;w 1	\" wide display if only one column
.		nr |\n(;w \n(dn		\" put the size of text
'\"	The following if condition tests: (1) if there is only one float
'\"	on the queue to output and (2) if wide displays and (3) we are on the
'\"	first column and (4) if the float fits on the current page or
'\"	(5) the float is too large to fit on any one page, and (6) we have used
'\"	less than half of the current page. (the six conditions are grouped
'\"	in the form (1 & (2 & 3) & (4 : (5 & 6))). If this total condition is true,
'\"	then a float is output.
.		if ((\n(:z-\n(:w=1)&((\n(!\n(;w=1)&(\n(:C<2))&\
((\n(dn<\n(.t):((\n(dn>(\n(.p-\n(:r-\n(:m))&\
(\n(nl<=(\n(.p-\n(:r-\n(:o/2u+\n(:r))))) .)z
.		nr :y 0			\" reset flag indicating group
.		rr ;L ;f ;i ;j ;l ;o ;s ;u ;v ;w	\" free storage
])

|attribute(around_placement; nostack;
	[
.		)w
])

|attribute(footnote; stack, default off;
	on [
'\"	:C 0=one column, 1=first column, 2=second column
'\"	:F Footnote text
'\"	:W page width for full page output
'\"	:m initial footer place and partial footnote diverter trap
'\"	:o footer trap location
'\"	:q amount of footnote text
'\"	:v width of stored footnote 0=narrow, 1=wide
'\"	:x flag indicating footnote in progress
'\"	W page width
'\"	;L prevailing line spacing
'\"	;f prevailing font
'\"	;i prevailing indent
'\"	;j prevailing adjust
'\"	;l prevailing line length
'\"	;o prevailing page offset ??????? bug with .po
'\"	;s prevailing point size
'\"	;u prevailing fill/no-fill mode
'\"	;v prevailing vertical spacing
.		if \n(:x .tm "__FILE__":__LINE__: Footnotes can not be nested
.		nr :x 1			\" set flag indicating footnote
.		nr ;s \n(.s		\" save prevailing point size
.		nr ;f \n(.f		\" save prevailing font
.		nr ;u \n(.u		\" save prevailing fill/no-fill mode
.		nr ;j \n(.j		\" save prevailing adjust
.		nr ;v \n(.v		\" save prevailing vertical spacing
.		nr ;L \n(.L		\" save prevailing line spacing
.		nr ;l \n(.l		\" save prevailing line length
.		nr ;i \n(.i		\" save prevailing indent
.		ev 1			\" use footnote environment
.		ps \n(;s		\" restore point size
.		vs \n(;sp+2p		\" restore vertical spacing
.		ft \n(;f		\" restore font
.		nf			\" bring it back in no-fill
.		if \n(;u .fi		\" restore fill if necessary
.		ad \n(;j		\" restore page adjust
.		vs \n(;vu		\" restore vertical spacing
.		ls \n(;L		\" restore line spacing
.		ll \n(;lu		\" restore line length
.		in \n(;iu		\" restore indent
.		ti \n(;iu		\" turn off indentation
.		hy 14			\" turn on hyphenation
.		lt \n(;lu		\" restore title length
|ifvalue wide on [
.		if \n(:C \{\
.			ll \n(:W	\" go wide
.			lt \n(:W \}
]
'DUMMY
.		da :F			\" collect in :F
.		if !\n(:q \{\
.			nr ;o \n(.o	\" save prevailing page offset
.			po 0		\" always from left margin
.			ps 10		\" 10 point roman font		
.			vs 10p
.			ft 1
.			ie \n(.A=0 \l'72p'\" layout partial rule if new footnote
.			el  __________
.			ft		\" restore values
.			vs
.			ps
.			po \n(;ou\}
]	off [
.		br			\" get last partial line
.		di			\" end diversion
|ifvalue wide on [
.		if \n(:C \{\
.			ll \nW		\" go narrow
.			lt \nW \}
.		nr :v 1			\" wide footnote
]
'DUMMY
|ifnotvalue wide on [
.		if !\n(:C .nr :v 1	\" wide footnote if only one column
]
'DUMMY
.		ev			\" go back to previous environment
.		nr :x 0			\" clear flag indicating footnote
.		nr :q +\n(dnu		\" add in amount of new footnotes
'\"	Move up footer trap, but not above current position on page
.		nr :o +\n(dnu
.		if !\n(.pu-\n(nlu-.5p-\n(:ou .nr :o \n(.pu-\n(nlu-.5p
'\"	or below :m!
.		if !\n(:ou-\n(:mu .nr :o \n(:mu
.		ch )f -\n(:ou		\" move footer trap
.		rr ;L ;f ;i ;j ;l ;s ;u ;v	\" free storage
])

|attribute(size; stack, default;
	$ [\&\s$]
	[\&\s\nS])

|attribute(font.size; stack, init;
	$ [
.		ps $
.		vs \n(.sp+2p
]	[
.		ps \nSp
.		vs \nSp+2p
])

|attribute(font; stack, init;
	roman [
|ifvalue fill off [\&\f1]
|ifvalue fill on [
.		ft 1
]
]	italics [
|ifvalue fill off [\&\f2]
|ifvalue fill on [
.		ft 2
]
]	bold [
|ifvalue fill off [\&\f3]
|ifvalue fill on [
.		ft 3
]
]	special [
|ifvalue fill off [\&\f4]
|ifvalue fill on [
.		ft 4
]
]	cw [
|ifvalue fill off [\&\f(CW]
|ifvalue fill on [
.		ft CW
]
]	[
|ifvalue fill off [\&\f\nF]
|ifvalue fill on [
.		ft \nF
]
])

|attribute(temporary.font; nostack;
	roman [\f1]
	roman $ [\f1$\fP]
	italics [\f2]
	italics $ [\f2$\fP]
	bold [\f3]
	bold $ [\f3$\fP]
	special [\f4]
	special $ [\f4$\fP]
	cw [\f(CW]
	cw $ [\f(CW$\fP]
	[\fP]
)

|attribute(font.family; stack, default times;
	times [
.		fp 1 R
.		fp 2 I
.		fp 3 B
.		fp 4 BI
]	palatino [
.		fp 1 PA
.		fp 2 PI
.		fp 3 PB
.		fp 4 PX
]	bembo [
.		fp 1 B1
.		fp 2 B2
.		fp 3 B3
.		fp 4 B4
]	optima [
.		fp 1 O1
.		fp 2 O2
.		fp 3 O3
.		fp 4 O4
]	souvenir [
.		fp 1 SV
.		fp 2 SI
.		fp 3 SB
.		fp 4 SX
]	helvetica [
.		fp 1 H
.		fp 2 HI
.		fp 3 HB
.		fp 4 HX
]	cw [
.		fp 1 CW
.		fp 2 I
.		fp 3 B
.		fp 4 BI
])

|attribute(if; nostack;
	$test [
.		if $test \
]	begin $test [
.		if $test \{\
]	end [\}\c
]	else $test [
.		ie $test \
]	else begin $test [
.		ie $test \{]
	else [
.		el \
]	else begin [
.		el \{])

|attribute(tab; nostack;
	[\&	])

|attribute(thick.line; nostack;
	$ [
.		ps 24
\l'$'
.		ps
]	[
.		ps 24
\l'\n(.lu'
.		ps
])

|attribute(line; nostack;
	$ [\l'$']
	[\l'\n(.lu'])

|attribute(box; nostack;
	x [\s12\o'\(mu\(sq'\s0]
	empty [\s12\(sq\s0]
	[\(sq])

|attribute(underline; stack, default off;
	on [\kx]
	off [\l'|\nxu\(ul'])

|attribute(hyphenate; stack, init on;
	on [
.		hy 14
]	off [
.		nh
])

|attribute(case; stack, init any;
	upper [
'\" need to write this yet
]	lower [
'\" need to write this yet
]	capitalize [
'\" need to write this yet
]	any [
'\" need to write this yet
])

|attribute(tab.stops; nostack;
	$* [
.		ta $*
]	[
.		ta .5i 1.0i 1.5i 2.0i 2.5i 3.0i 3.5i 4.0i 4.5i 5.0i 5.5i 6.0i 6.5i 7.0i 7.5i
])

|attribute(set; nostack;
	$* $number [
|for i in $* {
.		nr $i $number
}]	string $* $string [
|for i in $* {
.		ds $i $string
}])

|attribute(store; nostack;
	$* $$. [
|for i in $* {
.		nr $i \n$$.
}]	$* $number.register [
|for i in $* {
.		nr $i \n($number.register
}]	string $* $$. [
|for i in $* {
.		ds $i \*$$.
}]	string $* $string.register [
|for i in $* {
.		ds $i \*($string.register
}]	number.from.string $* $$. [
|for i in $* {
.		nr $i \*$$.
}]	number.from.string $* $string.register [
|for i in $* {
.		nr $i \*($string.register
}]	string.from.number $* $$. [
|for i in $* {
.		ds $i \n$$.
}]	string.from.number $* $number.register [
|for i in $* {
.		ds $i \n($number.register
}])

|attribute(clear; nostack;
	$* [
|for i in $* {
.		nr $i 0
}]	string $* [
.		rm $*
])

|attribute(add; nostack;
	$result $operand $* [
.		nr $result \n($operand
|for i in $* {
.		nr $result +\n($i
}]	$result $operand [
.		nr $result \n($operand
]	string $string $text [
.		as $string $text
]	string.from.number $string $number.register [
.		as $string \n($number.register
]	string.from.string $string $string.register [
.		as $string \*($string.register
])


|attribute(incr; nostack;
	$* [
|for i in $* {
.		nr $i +1
}])

|attribute(decr; nostack;
	$* [
|for i in $* {
.		nr $i -1
}])

|attribute(protect; nostack;
	$ [
.		ne $
]	[
.		ne 3
])

|attribute(warn.orphan; nostack;
	$ [
'\" need to write this yet
]	[
'\" need to write this yet
])

|attribute(two.columns; nostack;
	on [
'\"	:C 0=one column, 1=first column, 2=second column
'\"	:O page offset for other column
'\"	:W page width for full page output
'\"	:u position to return to for second column output
'\"	O page offset
'\"	W page width
.		if \n(:C .tm "__FILE__":__LINE__: already in two column mode
.		br			\" force out last partial line
.		nr :C 1			\" flag for two column mode, first column
.		nr :W \nW		\" save line length
.		nr W \n(.lu*8u/17u	\" new line length
.		nr :O \n(.lu*9u/17u+\nOu\" page offset for other column
.		ll \nWu			\" set new line length
.		lt \nWu			\" set new title length
.		mk :u			\" save where second column starts
]	off [
.		if !\n(:C .tm "__FILE__":__LINE__: already in full page mode
.		br			\" force out last partial line
.		if \n(:C>1 .ne \n(.tu+1v\" spring footer trap for second column
.		nr W \n(:Wu		\" restore line length
.		ll \nWu			\" set new line length
.		lt \nWu			\" set new title length
.		nr :C 0			\" flag for full page mode
])

|attribute(divert.string; nostack;
	$string.name [
.		ds $string.name ])

|attribute(divert.number; nostack;
	$number.name [
.		nr $number.name ])

|attribute(divert.output; stack, default off;
	on $string.name [
.		di $string.name		\" start diverting into macro
]	off [
.		br			\" get last partial line
.		di			\" turn off diversions
])

|attribute(divert.input; stack, default off;
	on $string.name [
.		eo
.		de $string.name
]	off [
..
.		ec
])

|attribute(remember; nostack;
	$ [
.		mk $
])

|attribute(return; nostack;
	$$. [
.		sp |\n$$.u
]	$ [
.		sp |\n($u
])

|attribute(goto; nostack;
	$ [
.		sp |$
])

|attribute(string; nostack;
	$$. [\*$$.]
	$string.register [\*($string.register])

|attribute(macro; nostack;
	$ [
.		$
])

|attribute(number; nostack;
	$$. $format [
.		af $$. $format
		\n$$.]
	$number.register $format [
.		af $number.register $format
		\n($number.register]
	$$. [\n$$.]
	$number.register [\n($number.register]
	++ $$. [\n+$$.]
	++ $number.register [\n+($number.register]
	format $* $format [
|for i in $* {
.		af $i $format
}])

|attribute(reference; stack, default off;
	on {
.		[
}	off {
.		]
}	placement {
.		[
		$LIST$
})

|attribute(table; stack, default off;
	on [
.		TS
]	off [
.		TE
])

|attribute(picture; stack, default off;
	on [
.		PS
]	off [
.		PE
])

|attribute(equation; stack, default off;
	on [
.		EQ
]	off [
.		EN
])

|attribute(inline.equation; stack, default off;
	on []
	off [])

|attribute(graph; stack, default off;
	on $$. [
.		G1 \*$$.
]	on $string.register [
.		G1 \*($string.register
]	off [
.		G2
])

|attribute(bargraph; stack, default off;
	on [
.		B1
]	off [
.		B0
])

|attribute(citation; nostack;
	save [
.		CD "]
	save end $$. [" "\*$$."
]	save end $string.register [" "\*($string.register"
]	savepage end [" "\nP"
]	remember [
.		CU "]
	remember end ["])

|attribute(insert; nostack;
	$file.name [
.		so $file.name
])

|attribute(list.tags; stack, init none;
	bullet [
.		ds list_type bullet
]	dash [
.		ds list_type dash
]	outline [
.		ds list_type outline
]	number [
.		ds list_type number
]	none [
.		ds list_type none
])

|attribute(format.type; stack, init none;
	model [
.		ds format_type model
]	none [
.		ds format_type none
])

|attribute(verbatim; stack, init off;
	off []
	on [])

|attribute(file.information; nostack;
	[
.		lf __LINE__ __FILE__
])

|attribute(white.text; nostack;
	$text [|ifvalue verbatim off []|ifvalue verbatim on [$text]])

|attribute(nl.paragraph; nostack;
	[|ifvalue verbatim on [
]|ifvalue verbatim off []])
