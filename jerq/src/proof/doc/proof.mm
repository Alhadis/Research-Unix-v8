.PH ""
.PF "'''Page \\\\nP'"
.ds HP 10 10
.ds p \f2proof\fP
.ds pp \f2Proof\fP
.ds t \f2troff\fP
.de ah
.ta 8n +8n +8n +8n +8n +8n +8n +8n +8n +8n
..
.fp 1 PA
.fp 2 PI
.fp 3 PB
.nr Pt 1
.tr ~ 
.EQ
delim $$
.EN
.DS C
Proof

Andrew Hume
.DE
.H 1 "Introduction"
\*(pp displays \*t output on the Blit much as it would appear
on a typesetter.
This document describes the structure of \*p
and explains some of the design choices.
.P
\*(pp operates in two modes:
in one mode it is displaying typeset characters and graphics,
and in the other it emulates the normal \f2mpx\fP terminal program.
Fonts are preserved while it is in the terminal mode.
The \*p layer can be distinguished from other layers by
vertical and horizontal scroll bars (\*t mode) or
by a striped border (terminal mode).
.H 1 "Terminal mode"
The terminal mode program is really the main program.
It is a direct steal of the code from \f2mpx\fP.
The \*t mode is activated by sending an ACK (\f(CW017\fP) character.
The terminal program never owns the mouse.
.P
When \*p is run for the first time and the Blit program \f2proof.m\fP is downloaded,
it starts executing as the terminal program.
.H 1 "Troff mode"
In \*t mode there are two programs co-operating.
The Blit program is in control and handles the user interfaceand screen graphics.
The host program acts as a font server and as a compressor of the \*t intermediate
code.
The protocol interface is simple and is based on the assumption that most
(say 99%) of the chars transmitted come from the host.
The host sends characters in packets of at most 128 characters.
At the end of each packet, the Blit program sends back
a (possibly empty) sequences of commands terminated by a NAK.
.H 2 "Compressed intermediate code"
Both the protocol and the program structure derive strongly from the compressed
form of the \*t intermediate code.
The \*t intermediate code has 17 commands listed
(along with the average number of bytes per command) in Figure 1.
.DS
.TS
center, allbox;
c c c
a aFCW n.
Command	Format	Length
=
abs horiz	H	4.0
abs ver	V	5.0
change font	f	2.0
comments	#	0.0
dev control	x	11.0
draw cmd	D	11.0
endline	n	7.0
multiple char	C	3.0
new word	w	1.0
nnCHAR	nnc	3.0
page	p	2.2
rel horiz	h	4.1
rel ver	v	0.0
set size	s	2.5
single char	c	2.0
text cmd	t	0.0
.TE
.SP
.FG "\*t intermediate code"
.DE
The new non-ASCII encoding was designed to minimise the number of bytes
transmitted for a given document.
Documents can be loosely divided up into two categories:
text intensive and graphics intensive.
The former is the norm,
the latter includes documents with a lot of
\f2tbl\fP, \f2eqn\fP or \f2pic\fP output.
Table 1 shows the most significant commands for each type ranked by percentage
of total bytes.
The category \f2noise\fP refers to irrelevant white space, comments and endlines.
.DS
.TS
center, allbox;
c s c s
c c c c
a n a n.
text	graphics
command	%	command	%
nnCHAR	52	noise	25
rel horiz	18	nnCHAR	21
noise	14	multiple char	17
single char	9	rel horiz	17
		abs horiz	7
.TE
.SP
.TB "Most frequent commands
.DE
.P
The encoding is a stream of bytes (rather than bits).
The new codes are shown in Figure 2.
The highlights are
.BL
.LI
rightwards relative horizontal motions of 0-99 units
are directly encoded as commands
.LI
all motion is relative
.LI
the most common point sizes are directly encoded
.LI
all font changes are directly encoded
.LI
the device control and draw commands are parsed on the host
but are only interpreted in the Blit
.LE
.DS
.TS
center, allbox;
c c
c a.
Code	Meaning
0..127	character in current <font, pointsize>
128..227	relative horizontal motion of 0-99 units
228	(N) new page N
229-238	set font to \f2n\fP (0-9)
239	(B) set point size to B
240	(N) move relative horizontal N units
241	(N) move relative vertical N units
242	(...) send device control command
243	(...) send draw command
244	change into a terminal
245	exit
246-255	set point size (6-15)
.TE
.SP
.FG "New encoding"
.DE
.P
The most important decision was to make characters exactly seven bits.
This is easy enough for ASCII characters but poses a problem
for characters from the special fonts.
The special font contains 137 characters but because the Blit fonts
are ASCII, the 14 characters \f(CW" # +  < = > @ \e ^ _ { | } ~\fP
in every normal font (like Times Roman) are from the special font
as Merganthaler did not supply them in the normal fonts.
There is a well defined mapping from the multiple character names
in the special font to an integer between 1 and 124 (three spare slots).
The mapping is contained in the file \f(CW/usr/jerq/font/SMAP\fP.
.P
Similarly, there exists a mapping between the multiple character name
characters in the normal fonts (such as the em dash \f2em\fP).
The mapping is in the file \f(CW/usr/jerq/font/IMAP\fP.
The characters are stored in the front of the normal font from character position
one onwards.
.P
So how does \*p map characters?
If the character has a one character name
then just send the character.
If the character has a long name,
look it up in SMAP and IMAP.
If it is in IMAP, just send the mapped value.
Otherwise, send a font change to the special font and send the mapped value.
.H 1 "Program structure"
The source is laid out into two directories; \f(CWterm\fP and \f(CWhost\fP.
They communicate via the protocol in \f(CWcomm.h\fP.
.H 2 "Host"
The host has a simple life:
it simply reads its input, compresses it and sends it to the terminal.
It stores the address of page commands for seeking.
It ignores comments, endlines and other whites space equivalents.
It optimises cursor movement.
.P
The commands from the terminal are executed in \f(CWio.c\fP and the
mapping of special (and other) characters is done in \f(CWmap.c\fP.
Debugging can be turned on with \f(CW-d\fP and is put into the file \f(CWdebug\fP.
Even more debugging will be generated if you use \f(CW-v\fP as well.
.H 2 "Terminal"
