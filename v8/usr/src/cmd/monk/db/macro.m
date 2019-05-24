|include(macro.h)

|macro(.top.of.page;
	[
'\"	#T used by tbl to draw boxes ?????
'\"	:2 store for multi-page table heading
'\"	:A multi-page table heading being used
'\"	:T top of table printed
'\"	:d prevent )h from outputting a floating group
'\"	:o footer trap location
'\"	:q amount of footnote text
'\"	:r position at end of page header macro
'\"	:u position to return to for second column output
'\"	:v width of stored footnote 0=narrow, 1=wide
'\"	:w output counter for floating keeps
'\"	:z input counter for floating keeps
'\"	P page number register
'\"	;i remembered indent TEMP
'\"	;o prevailing page offset ??????? bug with .po
'\"	;t copy of :w TEMP
'\"	;w convert to alpha format TEMP
.		de )h
.		ev 2					\" use header environment
.		if "\*(.T"aps" \{\
.			nr ;o \\n(.o			\" save prevailing page offset
.			po 0				\" always from left margin
.			lt 7.5i				\" width of photo composer
.			ps 10				\" 10 point roman font
.			vs 10p
.			ft 1
.			tl '--''--'			\" output the cut marks
.			ft				\" restore values
.			vs
.			ps
.			lt
.			po \\n(;ou\}
.		nr P +1					\" incr page number
.		lt \\nWu				\" use line length for title
.		TP					\" user-redefinable macro
.		br					\" force a break in the header environment
.		ev					\" back to main environment
.		ie \\n(:q .ch )f -\\n(:ou		\" reset footer trap
.		el .nr :v 0				\" no saved footnotes, thus narrow
.		mk :r					\" position at end of page header macro
.		mk :u					\" save where second column starts
'\"	output floating displays
.		if \\n(:d=0&(\\n(:z-\\n(:w) \{\
.			nr ;t \\n(:w%26+1		\" get index of next output element
.			af ;w a				\" conversion register for display queue names
.			nr ;w \\n(;t			\" form the next queue element name
.			)z				\" output at least one floating display
.			)s				\" but as many as will fit
.			rr ;t ;w \}			\" free storage
'\"	print top part of multi-page table
.		nr :T 0					\" mark multi-page table header not printed
.		if \\n(:A>0 \{\
.			nr ;i \\n(.i			\" save prevailing indent
.			in 0				\" indent back
.			:2				\" heading
.			nr :T 1				\" mark multi-page table header printed
.			in \\n(;iu \}			\" restore prevailing indent
.		mk #T					\" mark spot for table drawing ???????
.		rr ;i ;o ;t ;w				\" free storage
.		ns					\" avoid .sp in user text here
..
])

|macro(top.of.page;
	[
'\"	EH string containing even top title info
'\"	OH string containing odd top title info
'\"	P page number register
'\"	PH string containing top title info
'\"	;P page number register TEMP
.		de TP
'		sp
.		af ;P \\gP				\" save format of P
.		af P 1					\" normal format for next line
.		nr ;P \\nP				\" must use different name for P
.		af P \\g(;P				\" restore format to P
.		af ;P 1					\" normal format for control register
'		sp 2
.		if \\n(;P-1 .tl \\*(PH			\" output headers
.		if !\\n(;P%2 .tl \\*(EH
.		if \\n(;P%2 .tl \\*(OH
'		sp 2
.		if \\n(;P=4 .if "\\*(f0"model" \{.nr L 13.25i	\" model sheet kludge
.		pl 13.25i \}				\" model sheet kludge
.		rr ;P					\" free storage
..
])

|macro(.end.of.page;
	[
'\"	#T used by tbl to draw boxes ?????
'\"	:2 store for multi-page table heading
'\"	:A multi-page table heading being used
'\"	:C 0=one column, 1=first column, 2=second column
'\"	:K position for )f trap
'\"	:T top of table printed
'\"	:d prevent )h from outputting a floating group
'\"	:f bottom of table printed
'\"	:m initial footer place and partial footnote diverter trap
'\"	:o footer trap location
'\"	:q amount of footnote text
'\"	:v width of stored footnote 0=narrow, 1=wide
'\"	:w output counter for floating keeps
'\"	:z input counter for floating keeps
'\"	T. ?????
'\"	;i remembered indent TEMP
'\"	;o page offset transfer TEMP
'\"	;t copy of :w TEMP
'\"	;u hidden def of )f TEMP
'\"	;w convert to alpha format TEMP
'\"	;y room left on the page TEMP
.		de )f
'\"	print bottom part of boxed multi-page table
.		if \\n(:A \{\
.			rn )f ;u			\"hide footer
.			nr T. 1
.			if \\n(:f=0 .T# 1		\" print multi-page table footer
'			br				\" output new page
.			nr :f 1				\" remember footer has been printed
.			rn ;u )f \}			\" restore footer
.		nr ;y \\n(.pu-\\n(nlu-\\n(:mu		\" compute amount of room left on the page
.		if \\n(:qu>4v .nr ;y -4v		\" output a reasonable amount
'\"	if footnotes and there is room left and full page or left column output,
'\"	or narrow footnotes, call footnote expander
.		ie \\n(:q&\\n(;y&((\\n(:C<2):(\\n(:v=0)) .)o
.		el \{\
.			ch )f -\\n(:mu			\" hide footnote divider trap
.			nr :o \\n(:m+\\n(:q \}		\" reset footer trap location
.		ie \\n(:C=0 'bp				\" new page if not multi-column
'\"	end second column
.		el .ie \\n(:C=2 \{\
.			nr ;o \\nOu			\" save second column offset
.			nr O \\n(:Ou			\" restore first column offset
.			nr :O \\n(;o			\" save second column offset
.			po \\nOu 			\" reset page offset
.			nr :C 1 			\" set col indicator
'			bp				\" force next page
.			rr ;o \}			\" free storage
'\"	end first column
.		    el \{\
.			rt \\n(:uu			\" return to top
.			nr ;o \\nOu			\" save first column offset
.			nr O \\n(:Ou			\" restore second column offset
.			nr :O \\n(;o			\" save first column offset
.			po \\nOu 			\" advance page offset
.			nr :C 2 			\" set col indicator
.			if \\n(:q .ch )f -\\n(:ou	\" reset footer trap
'\"	output floating displays that are narrow
.			if \\n(:d=0&(\\n(:z-\\n(:w) \{\
.				nr ;t \\n(:w%26+1	\" get index of next output element
.				af ;w a			\" conversion register for display queue names
.				nr ;w \\n(;t		\" form the next queue element name
'\"	inhibit if wide display
.				if \\n(!\\n(;w=1 \{\
.					)z		\" output at least one floating display
.					)s \}		\" but as many as will fit
.				rr ; o ;t ;w \}		\" free storage
'\"	print top part of multi-page table
.			nr :T 0				\" mark multi-page table header not printed
.			if \\n(:A>0 \{\
.				nr ;i \\n(.i		\" save prevailing indent
.				in 0			\" indent back
.				:2			\" heading
.				nr :T 1			\" mark multi-page table header printed
.				in \\n(;iu \}		\" restore prevailing indent
.			mk #T				\" mark spot for table drawing ???????
.			rr ;i ;t ;w			\" free storage
.			ns \}				\" avoid .sp in user text here
.		if \\n(:A \{\
.			nr :K \\n(:ou+2v		\" position for )f trap
.			ch >f -(\\n(:Ku+1v)		\" when moved by >f macro
'			br \}
.		rr ;y					\" free storage
..
])

|macro(.float.output;
	[
'\"	!{a-z} width of stored floating 0=narrow, 1=wide
'\"	:C 0=one column, 1=first column, 2=second column
'\"	:d prevent )h from outputting a floating group
'\"	:w output counter for floating keeps
'\"	:z input counter for floating keeps
'\"	?{a-z} text of stored floating
'\"	|{a-z} size of stored floating
'\"	;i remembered indent TEMP
'\"	;l remembered line spacing TEMP
'\"	;q remembered fill/no-fill mode TEMP
'\"	;w convert to alpha format TEMP
.		de )z
.		if !\\n(:z-\\n(:w .tm no floats to output
.		nr :w \\n(:w%26+1			\" get index of next output element
.		af ;w a					\" conversion register for display queue names
.		nr ;w \\n(:w				\" form the next queue element name
.		nr ;q \\n(.u				\" save prevailing fill/no-fill mode
.		nr ;l \\n(.L				\" save line spacing
.		nr ;i \\n(.i				\" save prevailing indent
.		ev 1					\" use footnote environment
.		nf					\" bring it back in no-fill
.		ls 1					\" output display in ls 1
.		in 0					\" indent back
.		nr :d 1					\" inhibit header from calling )y
.		rs					\" restore spacing
.		br					\" output partial line before display
.		?\\n(;w					\" lay out one keep
.		if \\n(;q .fi				\" restore fill if necessary
.		ls \\n(;L				\" restore line spacing
.		in \\n(;iu				\" restore prevailing indent
.		nr :d 0					\" allow )h to process fl keeps again
.		ev					\" back to previous environment
.		ne 2					\" require two lines under float
.		rm ?\\n(;w				\" free storage
.		rr |\\n(;w !\\n(;w
.		rr ;i ;l ;q ;w
..
])

|macro(.multiple.float.output;
	[
'\"	!{a-z} width of stored floating 0=narrow, 1=wide
'\"	:C 0=one column, 1=first column, 2=second column
'\"	:m initial footer place and partial footnote diverter trap
'\"	:o footer trap location
'\"	:r position at end of page header macro
'\"	:w output counter for floating keeps
'\"	:z input counter for floating keeps
'\"	|{a-z} size of stored floating
'\"	;h height of floating display on top of queue TEMP
'\"	;t copy of :w TEMP
'\"	;w convert to alpha format TEMP
.		de )s
'\"	inhibit if no floats in queue
.		if \\n(:z-\\n(:w \{\
.			nr ;t \\n(:w%26+1		\" get index of next output element
.			af ;w a				\" conversion register for display queue names
.			nr ;w \\n(;t			\" form the next queue element name
.			nr ;h \\n(|\\n(;w		\" height of current display
'\"	The following if condition tests: (1) if wide displays and (2) we are on the
'\"	first column and (3) if the float fits on the current page
'\"	or (4) the float is too large to fit on any one page, and (5) we have used
'\"	less than half of the current page. (the five conditions are grouped
'\"	in the form ((1 & 2) & (1 : (2 & 3))). If this total condition is true,
'\"	then a float is output.
.			if (((\\n(!\\n(;w=1)&(\\n(:C<2))&\
((\\n(;h<\\n(.t):((\\n(;h>(\\n(.p-\\n(:r-\\n(:m))&\
(\\n(nl<=(\\n(.p-\\n(:r-\\n(:o/2u+\\n(:r))))) \{\
.				)z			\" output it
.				)s \}			\" recurse
.			rr ;h ;t ;w \}			\" free storage
..
])

|macro(.flush.float.output;
	[
'\"	:d prevent )h from outputting a floating group
'\"	:w output counter for floating keeps
'\"	:z input counter for floating keeps
'\"	;o copy of :w TEMP
.		de )w
'\"	inhibit if no floats in queue
.		if \\n(:z-\\n(:w \{\
.			nr ;o \\n(:w			\" save output count
.			)s
.			if \\n(:w=\\n(;o \{\
.				nr :d 1			\" inhibit header from calling )y
.				rs			\" turn on spacing
.				bp			\" top of next page
.				nr :d 0 \}		\" allow )h to process fl keeps again
.			)w \}				\" recurse
.		rr ;o					\" free storage
..
])

|macro(.footnote.output;
	[
'\"	dump accumulated footnote text
'\"	long text may spring )n trap so the remaining partial
'\"	text may go into :3
'\"	:3 store for partial footnotes
'\"	:m initial footer place and partial footnote diverter trap
'\"	:o footer trap location
'\"	:q amount of footnote text
'\"	:v width of stored footnote 0=narrow, 1=wide
.		de )o
.		ev 1					\" use footnote environment
.		nf					\" bring it back in no-fill
.		ls 1					\" output display in ls 1
.		in 0					\" indent back
.		:F					\" lay out footnotes
'\"	did we spring the partial footnote trap
.		ie "\\n(.z":3" \{\
.			br				\" get last partial line
.			di				\" end diversion
.			nr :q \\n(dnu 			\" fix amount of footnote text
.			rn :3 :F \}			\" put text back in :F
.		el .nr :q 0				\" clear amount of footnote text
.		ev					\" back to main environment
.		if \\n(:qu<=2v \{\			\" ignore just divider line
.			nr :q 0				\" clear amount of footnote text
.			rm :F \}			\" clear text of footnote
.		nr :o \\n(:mu+\\n(:qu			\" reset footnote location register
.		ch )f -\\n(:ou				\" reset trap for footer
..
])

|macro(.footnote.diverter;
	[
'\"	partial footnote text diverter
'\"	called via trap planted at -\n(:m
'\"	since the default footer trap is also at -n(:m,
'\"	this macro actually invoked when the footer trap
'\"	has been moved up due to footnote processing
'\"	:3 store for partial footnotes
'\"	:q amount of footnote text
'\"	;o prevailing page offset ??????? bug with .po
.		de )n
.		if \\n(:q \{\
.			di :3
.			nr ;o \\n(.o		\" save prevailing page offset
.			po \\nOu		\" always from left margin
.			ps 10			\" 10 point roman font		
.			vs 10p
.			ft 1
.			ie \n(.A=0 \l@\\n(.lu@
.			el _____________________________
.			ft			\" restore values
.			vs
.			ps
.			po \\n(;ou\}
..
])

|macro(.end.of.file;
	[
.		de )q
..
])

|macro(.initalize;
	[
'\"	REQUIRED INITIALIZATIONS
'\"	initialize various regs
'\"	!{a-z} width of stored floating 0=narrow, 1=wide
'\"	#T used by tbl to draw boxes ?????
'\"	:0 store for initally diverted text
'\"	:1 store for centered block text
'\"	:2 store for multi-page table heading
'\"	:3 store for partial footnotes
'\"	:A multi-page table heading being used
.		nr :A 0
'\"	:C 0=one column, 1=first column, 2=second column
.		nr :C 0
'\"	:F Footnote text
'\"	:K position for )f trap
.		nr :K 0
'\"	:O page offset for other column
.		nr :O 0
'\"	:T top of table printed
.		nr :T 0
'\"	:W page width for full page output
.		nr :W 0
'\"	:d prevent )h from outputting a floating group
.		nr :d 0
'\"	:f bottom of table printed
.		nr :f 0
'\"	:m initial footer place and partial footnote diverter trap
.		nr :m .8i
'\"	:o footer trap location
.		nr :o \n(:mu
'\"	:q amount of footnote text
.		nr :q 0
'\"	:r position at end of page header macro
.		nr :r .6i
'\"	:u position to return to for second column output
.		nr :u .6i
'\"	:v width of stored footnote 0=narrow, 1=wide
.		nr :v 0
'\"	:w output counter for floating keeps
.		nr :w 0
'\"	:x flag indicating footnote in progress
.		nr :x 0
'\"	:y flag indicating group/float in progress
.		nr :y 0
'\"	:z input counter for floating keeps
.		nr :z 0
'\"	?{a-z} text of stored floating
'\"	AT string containing AT&T in helvetica
.		ds AT \fP\f(HBA\h'-.2m'T\h'-.15m'\s0\s13&\s0\s16\h'-.15m'T
'\"	DT date string
.		if \n(mo-0 .ds DT January
.		if \n(mo-1 .ds DT February
.		if \n(mo-2 .ds DT March
.		if \n(mo-3 .ds DT April
.		if \n(mo-4 .ds DT May
.		if \n(mo-5 .ds DT June
.		if \n(mo-6 .ds DT July
.		if \n(mo-7 .ds DT August
.		if \n(mo-8 .ds DT September
.		if \n(mo-9 .ds DT October
.		if \n(mo-10 .ds DT November
.		if \n(mo-11 .ds DT December
.		as DT " \n(dy, 19\n(yr
'\"	EH string containing even top title info
'\"	F default font
.		nr F 1
'\"	L page length
.		if !\nL .nr L 11i
'\"	LO string containing logo
.		ie "\*(.T"aps" .ds LO \s36\(Lb\s0
.		el .ds LO \s36\(L1\s0\s16\v'-.4'\*(AT\v'.4'\h'-\w'\*(AT'u'\fP\s0
'\"	OH string containing odd top title info
'\"	P page number
.		if \nP .nr P -1
.		nr P \nP 1
'\"	PH string containing top title info
.		ds PH ''- \\nP -''		\" top of page string
'\"	O page offset
.		if !\w'\gO' .ie t .nr O .963i
.			el .nr O 0i
.		if "\*(.T"202" .nr O \nO-.34375i
.		if "\*(.T"aps" .nr O \nO-.21875i
'\"	S default point size
.		if !\nS .nr S 10
'\"	T. ?????
'\"	W page width
.		if !\nW .nr W 6i
'\"	|{a-z} size of stored floating
'\"	INITIAL SET UP
.		wh 0 )h					\" trap for header
.		wh -\n(:mu )f				\" trap for footer
.		ch )f 15i				\" move it over partial footnote diverter
.		wh -\n(:mu )n				\" trap for partial footnote diverter
.		ch )f -\n(:mu
.		em )q					\" end macro
'\"	TROFF ACCENTS (` ' ^ ~ cedilla and 2 umlauts)
'\"	The accent string must follow immediately the character to be accented.
'\"	These strings are "tuned" to the Times Roman type faces ONLY.
'\"	The R, I, and B fonts are assumed to be mounted in
'\"	positions 1, 2, and 3, respectively.
'\"	All of these strings alter the number register `:'.
'\"	Grave accent -- {aeou}\*`
.		ds ` \\k:\h@-\\n(.wu*8u/10u@\h@\\n(.fu/2u*2u+1u-\\n(.fu*.2m@\(ga\h@|\\n:u@
'\"	Lower-case acute accent -- {aeou}\*'
.		ds ' \\k:\h@-\\n(.wu*8u/10u@\h@\\n(.fu/2u*2u+1u-\\n(.fu*.2m+.07m@\(aa\h@|\\n:u@
'\"	Upper-case acute accent -- {aeou}\*+
.		ds + \\k:\h@-\\n(.wu*8u/10u@\h@\\n(.fu/2u*2u+1u-\\n(.fu*.2m+.07m@\v@-.2m@\(aa\v@.2m@\h@|\\n:u@
'\"	Circumflex -- {aeou}\*^
.		ds ^ \\k:\h@-\\n(.wu*8u/10u@\h@\\n(.fu/2u*2u+1u-\\n(.fu*.15m-.07m@\
\h@\\n(.fu-1u/2u*.02m@^\h@|\\n:u@
'\"	Tilde -- n\*~ (But watch out for ".tr ~")
.		ds ~ \\k:\h@-\\n(.wu*8u/10u@\h@\\n(.fu/2u*2u+1u-\\n(.fu*.2m-.07m@\
\h@\\n(.fu-1u/2u*.05m@~\h@|\\n:u@
'\"	Cedilla -- c\*,
.		ds , \\k:\h@-\\n(.wu*85u/100u@\v@.07m@,\v@-.07m@\h@|\\n:u@
'\"	Lower-case umlaut -- {aeou}\*:
.		ds : \\k:\h@-\\n(.wu*85u/100u@\h@\\n(.fu/2u*2u+1u-\\n(.fu*3u*.06m@\
\h@3u-\\n(.fu/2u*.05m-.1m@\
\v@-.6m@\z.\h@\\n(.fu-1u/2u*.05m+.2m@.\v@.6m@\h@|\\n:u@
'\"	Upper-case umlaut -- {AEOU}\*;
.		ds ; \\k:\h@-\\n(.wu*75u/100u@\h@\\n(.fu/2u*2u+1u-\\n(.fu*3u*.09m@\
\h@3u-\\n(.fu/2u*.06m-.15m@\h@\\n(.fu-1u/2u*.04m@\
\v@-.85m@\z.\h@.3m@.\v@.85m@\h@|\\n:u@
'\"	Nasal -- c\*-
.		ds - \\k:\h@-\\n(.wu*75u/100u@\v@.7m@`\v@-.7m@\h@|\\n:u@
'\"	Slash -- c\*/
.		ds / \\k:\h@-\\n(.wu*85u/100u@\v@.2m@\(aa\v@-.2m@\h@|\\n:u@
])
