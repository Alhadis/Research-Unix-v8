'''\"
'''\" registers
'''\"	a - abstract continuation flag - 0 (no), >0 (yes)
'''\"	b - mercury selections counter
'''\"	c - distribution continuation flag - 0 (no), 1 (yes)
'''\"	e - complete copy basic distribution length
'''\"	g - complete copy overflow distribution length
'''\"	h - cover sheet basic distribution length
'''\"	i - cover sheet overflow distribution length
'''\"	j - CI-II info supplied - 0 (no) 1 (yes)
'''\"	k - keyword flag - 0 (none), 1 (some) - reused as scratch
'''\"	l - number of vertical units per line - troff and nroff
'''\"	m - memorandum type flag - 1 TM, 2 IM, 3 TC
'''\"	n - document number counter
'''\"	o - title flag - 0 (no), 1 (yes - vertical size of title diversion)
'''\"	p - proprietary notice flag - 0 (none), 1 (default), 2(BR)
'''\"	q - scratch
'''\"	r - security flag - 0 (no), 1 (yes)
'''\"	s - software flag - 0 (no), 1 (yes)
'''\"	t - mark title position
'''\"	u - author count
'''\"	v - scratch - but remembered
'''\"	w - scratch - but remembered
'''\"	x - mark scratch position
'''\"	y - mark scratch position
'''\"	z - mark scratch position
'''\" strings
'''\"	a) - mercury info
'''\"	b) - mercury info
'''\"	d) - date
'''\"	k) - keywords
'''\"	N1 - first document number
'''\"	p) - proprietary 1
'''\"	q) -     "       2
'''\"	r) -     "       3
'''\"	s) - time stamp string
'''\"	t) - memo type (TM, IM, TC)
'''\"	v) - s for document nos
'''\"	w) - s for filing case nos
'''\"	x) - s for work project nos
'''\"	a( - AT&T-IS info 1
'''\"	b( -    " 2
'''\"	c( -    " 3 (director name)
'''\"	m( - authors 1-3 sig
'''\"	n( - authors 4-6 sig
'''\"	o( - authors 7-9 sig
'''\"	p( - authors 10-12 sig
'''\"	s( - S software string
'''\"	t( - memo type ("for Technical Memorandum", etc.)
'''\"	N2 - second document number
'''\"	N3 - third document number
'''\" 	Fi - up to 3 filing cases
'''\"	Xi - up to 3 work program numbers
'''\" diversions
'''\"	ZA - abstract
'''\"	ZI - author info section
'''\"	ZC - complete copy addressee primary
'''\"	ZO - complete copy addressee overflow
'''\"	ZS - cover sheet addressee primary
'''\"	ZD - cover sheet addressee overflow
'''\"	ZN - document number info
'''\"	ZT - title
'''\"
'''\" initialization
'''\"
.	\" to foil ms
.rn FE F5
.ch NP 16i
.ch FO 16i
.ch FX 16i
.ch BT 16i
.nr FM .01i
.nr 1T 1
.nr BE 1
.nr PI 5n
.if !\n(PD .nr PD 0.3v
.pl 11i
.de FT
.fp 1 H
.fp 2 HI
.fp 3 HB
.fp 4 HX
.ps 10
.vs 12
..
.de FB
.fp 1 R
.fp 2 I
.fp 3 B
.fp 4 BI
.ps 10
.vs 12
..
.FT
.de FE
.F5
.nr F4 +\\n(FP
..
.nr a 0 1
.nr b 0 1
.nr c 0
.nr e 0
.nr g 0
.nr h 6
.nr i 0
.nr j 0
.nr k 0
.nr m 0
.nr n 0 1
.nr o 0
.nr p 1
.nr q 0
.nr r 0
.nr s 0
.nr t 0
.nr u 0 1
.nr v 0
.nr w 0
.nr x 0
.nr y 0
.nr z 0
.nr dv 0
.if '\*(.T'aps' .nr dv 1
'''\"	initialize units per vertical space
.nr l 120
.nr lp 66
.nr np 2 1
.af np i
.nr tp 2 1
.af tp i
.nr la 0
.nr a1 0
.nr a2 0
.nr ar 0
.nr u! 1
.nr ud 1
.di ZI
.di
.di ZN
.di
.di ZC
.di
.di ZO
.di
.di ZS
.di
.di ZD
.di
'''\"initialize date string
.if \n(mo-0 .ds d) January
.if \n(mo-1 .ds d) February
.if \n(mo-2 .ds d) March
.if \n(mo-3 .ds d) April
.if \n(mo-4 .ds d) May
.if \n(mo-5 .ds d) June
.if \n(mo-6 .ds d) July
.if \n(mo-7 .ds d) August
.if \n(mo-8 .ds d) September
.if \n(mo-9 .ds d) October
.if \n(mo-10 .ds d) November
.if \n(mo-11 .ds d) December
.as d) " \n(dy, 19\n(yr
'''			\" initialize strings
.ds m!
.ds m(
.ds n!
.ds n(
.ds o!
.ds o(
'''			\" initialize proprietary notice
.ds o) "AT&T BELL LABORATORIES \(em PROPRIETARY
.ds p) "Use pursuant to G.E.I. 2.2
.ds q)
.ds r)
'''			\" initialize trademark symbol
.ds MT \v'-0.5m'\s-4TM\s+4\v'0.5m'
.ds s) 0
'''			\"initialize csmacro version string
.ds ve MCSL (11/05/84)
.in 0
'''\"
'''			\" macros to collect information
'''\"
.de DT 				\" macro for date
'''\"					store date if non-empty
.if !'\\$1'' .ds d) \\$1 \\$2 \\$3 \\$4
..
.de TI				\" macro for title -TI = mm(TL)
.br
.sy ls -l \\n(.F|awk '{print $6 " " $7 "%" $8}'|sed -f /usr/lib/tmac/time.sed>/tmp/tp1k1
.so /tmp/tp1k1
.nr XY \\n(yr/4
.nr XY \\n(yr-(\\n(XY*4)
.if \\n(XY=0 .nr XY 1
.nr XY (\\n(XY+\\n(OM+(\\n(YD-1))*24*60*60
.nr XY \\n(XY+(\\n(HR*60*60)
.nr XY \\n(XY+(\\n(MI*60)
.sy rm /tmp/tp1k1
.ds s) \\n(XY
.fi
.ll 4.25i
'''\"					diversion for title ZT = mm(tI)
.di ZT
..
.de AH				\" macro for author info AH = mm(AU)
'''\"					don't count author unless non-empty
.if !'\\$1'' .nr u \\n+u
.if \\nu=1 \{\
.	br
'''\"					end title diversion on first author
.	di
.	nr o \\n(dn
.	ll
.	nf
.	ds d! \\$3
.	nr m2 \\$3/10 \}
.ta 1.0i 3.6i 4.0i 5.1i 6.0i
.br
'''\"					append to author list ZI = mm(aV)
.da ZI
	\\$1	\\$2	\\$5	\\$4	\\$3
.br
.da
'''\"					end append; info for signature lines
.AA \\nu "\\$1" \\$3
.ta 0.5i 1.0i 1.5i 2.0i 2.5i
..
.de AA
.di A\\$1
\\$2
.br
.di
.if !'\\*(d!'\\$3' \{\
.	nr u! \\n(u!+1
.	nr m3 \\$3/10
.	if !\\n(m2=\\n(m3 .nr ud \\n(ud+1 \}
.ie \\$1<4 \{\
.	as m! \\l'2.25i'	
.	as m( 	\\$2 \}
.el .ie \\$1<7 \{\
.	as n! \\l'2.25i'	
.	as n(	\\$2 \}
.el \{\
.	as o! \\l'2.25i'	
.	as o(	\\$2 \}
..
.de SA				\" macro for abstract info SA = mm(AS)
.br
.fi
.nr LL 7.0i
.FB
.ft 1
.di ZA				\" ZA = mm(aS)
..
.de SE 				\" macro for end of abstract info SE = mm(AE)
.br
.di
.nr la \\n(dn
.ll
.FT
.ft 1
.nf
..
.de KW				\" macro for keyword info KW = mm(OK)
.ds k)
.if !'\\$1'' .as k) \\$1
.if !'\\$2'' .as k); \\$2
.if !'\\$3'' .as k); \\$3
.if !'\\$4'' .as k); \\$4
.if !'\\$5'' .as k); \\$5
.if !'\\$6'' .as k); \\$6
.if !'\\$7'' .as k); \\$7
.if !'\\$8'' .as k); \\$8
.if !'\\$9'' .as k); \\$9
'''\"					set k flag if we have some keywords
.ie !'\\*(k)'' .nr k 1
.el .nr k 0
..
.de TY				\" macro for document type TY = mm(MT)
.if '\\$1'TM' \{\
.	nr m 1
.	ds t) TM
.	ds QF TECHNICAL MEMORANDUM
.	ds t( "for Technical Memorandum \}
.if '\\$1'IM' \{\
.	nr m 2
.	ds t) IM
.	ds QF INTERNAL MEMORANDUM
.	ds t( "for Internal Memorandum \}
.if '\\$1'TC' \{\
.	nr m 3
.	ds t) TC
.	ds QF TECHNICAL CORRESPONDENCE
.	ds t( "for Technical Correspondence \}
.ie '\\$2'y' .nr s 1
.el .nr s 0
..
.de NU				\" macro for document number info NU = mm(dN fC wP)
.ie \\ns=1 .ds s( S
.el .ds s(
.ie \\n(wp=0 \{\
.	ie '\\$5'' .ds CX 311403-0101
.	el .ds CX \\$5
.	ds X1 \\*(CX
.	nr wp \\n(wp+1 \}
.el \{\
.	ds CX \\$5
.	if !'\\$5'' \{\
.		if \\n(wp=1 .ds X2 \\*(CX
.		if \\n(wp=2 .ds X3 \\*(CX
.		nr wp \\n(wp+1
.		ds x) s\} \}
.if !'\\$4'' \{\
.	ie !\\n(fc=0 \{\
.		if \\n(fc=1 .ds F2 \\$4
.		if \\n(fc=2 .ds F3 \\$4
.		ds w) s
.		nr fc \\n(fc+1 \}
.	el \{\
.		ds F1 \\$4
.		nr fc \\n(fc+1 \} \}
.ie !'\\$1'' \{\
.	ds NN \\$1-\\$2-\\$3\\*(t)\\*(s(
.	if \\nn=0 .ds N1 \\*(NN
.	if \\nn=1 \{\
.		ds v) s
.		ds N2 \\*(NN\}
.	if \\nn=2 .ds N3 \\*(NN
.	nr n \\n+n \}
.el .ds NN
.ta 0.8i 3.5i 5.75i
.br
.da ZN				\" ZN = mm(dM fC wO)
	\\*(NN	\\$4	\\*(CX
.br
.da
.ta 0.5i 1.0i 1.5i
..
.de MY				\" macro for mercury selections MY = mm(mE)
.ds a)
.ds b)
.if '\\$1'y' \{\
.	as a) "	CHM - Chemistry and Materials
.	nr b \\n+b \}
.if '\\$2'y' \{\
.	as a) "	CMM - Communications
.	nr b \\n+b \}
.if '\\$3'y' \{\
.	as a) " 	CMP - Computing
.	nr b \\n+b \}
.if \\nb=3 .rn a) b)
.if '\\$4'y' \{\
.	as a) "	ELC - Electronics
.	nr b \\n+b \}
.if \\nb=3 .rn a) b)
.if '\\$5'y' \{\
.	as a) "	LFS - Life Sciences
.	nr b \\n+b \}
.if \\nb=3 .rn a) b)
.if '\\$6'y' \{\
.	as a) "	MAS - Mathematics and Statistics
.	nr b \\n+b \}
.if \\nb=3 .rn a) b)
.	if \\nb<6 \{\
.	if '\\$7'y' \{\
.		as a) "	PHY - Physics
.		nr b \\n+b \} \}
.if \\nb=3 .rn b) a)
..
.de PR				\" macro for proprietary marking PR = mm(PM)
.if '\\$1'BR' \{\
.	nr p 2
.	ds o) "AT&T BELL LABORATORIES PROPRIETARY \(em (RESTRICTED)
.	ds p) "Solely for authorized persons having a need to know
.	ds q) "pursuant to G.E.I. 2.2 \}
..
.de GS				\" GS = mm(gS)
.nr r 1
..
.de CI
.nr j 1
.ds b( 0				\" macro for handling CI-II CI = mm(cI)
.if '\\$1'y' .ds b( 1
.if '\\$1'Y' .ds b( 1
.if '\\$1'1' .ds b( 1
.br
'''\"				basic distribution leng-to be tailored-set e & h
.if \\np<2 .nr e \\ne+8
.if \\nr=0 .nr e \\ne+2
.if \\nu<=3 .nr e \\ne+3
.if \\nu<=6 .nr e \\ne+3
.if \\nu<=9 .nr e \\ne+3
.nr e \\ne+13
.nr h \\ne
..
.de CO				\" macro for complete copy addressees CO = mm(cC)
.ta 2.0i
.nf
.br
.ie \\ne>0 \{\
.	da ZC				\" ZC = mm(cA)
.	if '\\$1'y' .so /usr/lib/tmac/complet.1127
.	dt \\ne OC \}
.el .da ZC
..
.de OC				\" macro for complete copy overflow - OC = mm(cD)
.ta 2.0i
.br
.da
.da ZO				\" ZO = mm(cO)
..
.de CV				\" macro for cover sheet only addresses CV = mm(cS)
.ta 2.0i
.nf
.br
.ie \\nh>0 \{\
.	da ZS				\" ZS = mm(dA)
.	if '\\$1'y' .so /usr/lib/tmac/cover.1127
.	dt \\nh OV \}
.el .da ZD
..
.de OV				\" macro for cover sheet only overflow OV = mm(cT)
.ta 2.0i
.br
.da
.da ZD				\" ZD = mm(cO)
..
.de CE				\" ending all distribution diversions CE = mm(cE)
.br
.if "\\n(.z"ZC" \{\
.	nr g 0
.	rm OC \}
.if "\\n(.z"ZO" \{\
.	nr g -1
.	rm OC \}
.if "\\n(.z"ZS" \{\
.	nr i 0
.	rm OV \}
.if "\\n(.z"ZD" \{\
.	nr i -1
.	rm OV \}
.da
.if \\ng=-1 \{\
.	ie \\n(dn>0 .nr g (\\n(dn)/\\nl+4
.	el .nr g 0 \}
.if \\ni=-1 \{\
.	ie \\n(dn>0 .nr i (\\n(dn)/\\nl+4
.	el .nr i 0 \}
..
'''\"
'''\" macros to help format document
'''\"
.de HD
.po .25i
.sp|0.2i
..
.de FC				\" footer macro FC = mm(fO)
.pl 11.0i
'bp
..
.de ST			\" macro for abstract overflow trap ST = mm(yY)
.ZB
.rm ST				\" ZB = mm(aT)
..
.de ZB
.ch ST 16i			\" macro for abstract overflow trap ZB = mm(aT)
.if \\na>0 \{\
.	ft 2
.	ce
(continued)
.	ft 1 \}
.pl 11.0i
.nr a \\n+a
.rn ZB XX
'bp
.rn XX ZB
.wh -0.35i ZB
.HC				\" HC = mm(cH)
.HX				\" HX = mm(tH)
'sp 0.05i
.ce
.ft 3
Abstract (continued)
.ft 1
.in 0.2i
'sp 1
.FB
..
.de TK				\" macro for thick lines TKK = mm(tK)
.ps 24
\l'7.5i'
.ps
..
.de HX				\" macro for Title headings and text HX = mm(tH)
.TK
'sp 0.05i
'''\"					mark t - Title heading
.mk t
.ft 3
Title:
.ft
'sp|\\ntu
.in 0.7i
.ZT
.in 0
.ta 0.5i
.nr q \\no/\\nl
.ie \\nq>2 'sp|\\ntu+\\nq
.el 'sp|\\ntu+2
.TK
.					\" m1 - mark end of title section - save
.mk m1
..
.de HC				\" macro for continuation header HC = mm(cH)
.nr np \\n+(np
.nf
.in 0
.FT
.ft 3
.ta 5.25i
	\\*(N1\f2 (page \\n(np of \\n(tp)
.sp 0.1i
..
.de DL				\" macro for distribution list headers DL = mm(dH)
.ft 3
.ta 1.0i 4.75i
	\\$1	\\$2
.sp 0.05i
.ft 1
.ta 0.5i 1.0i
..
.de EJ				\" macro for ejecting continuation page EJ = mm(eP)
'bp
.wh 0 HD
'''\"				put out continuation page header & title section
.HC
.HX
..
.de CP				\" macro for continuation page CP = mm(cP)
'''\"					calc vert. units for cc overflow (if any)
.ie \\nv<=\\n(.t .nr v 1
.el .nr v 0
.if \\nv=1 .if \\nw<=\\n(.t .nr v 2
.				\" check if cont page needs to be ejected
.in 0
.if \\nc=1 \{\
.				\" - if no abstract overflow
.	if \\na=0 .EJ
.	if \\na>0 \{\
.				\"or if abstract over but no room for list overfl
.		ie \\nv<2 .EJ
.				\" just tk line if abstract over & room for list
.		el .TK \}
.	FT
.	ie \\ne=0 \{\
.		if \\ng>4 .if \\ni>4 .DL "Complete Copy" "Cover Sheet Only"
.		if \\ng>4 .if !\\ni>4 .DL "Complete Copy" ""
.		if !\\ng>4 .if \\ni>4 .DL "" "Cover Sheet Only" \}
.	el \{\
.		if \\ng>4 .if \\ni>4 .DL "Complete Copy (continued)" "Cover Sheet Only (continued)"1
.		if \\ng>4 .if !\\ni>4 .DL "Complete Copy (continued)" ""
.		if !\\ng>4 .if \\ni>4 .DL "" "Cover Sheet Only (continued)" \}
.	mk z
'''\"					put out complete copy list overflow
.	ZO
.	mk x
.	sp|\\nzu
.	in 4i
'''\"					put out cover sheet list overflow
.	ZD
.	mk y
.	in 0
.	if \\nx-\\ny .sp|\\nxu
.	TK \}
..
.de ZP					\"compute total pages and diversion lengths
'''\"				calculate vert. units for cc overflow (if any)
.ie \\ng>4 .nr v (\\ng)*\\nl
.el .nr v 0
'''\"					also for cs overflow (if any)
.ie \\ni>4 .nr w (\\ni)*\\nl
.el .nr w 0
.					\" set c=1 if either g or i >0
.if \\ng>4 .nr c 1
.if \\ni>4 .nr c 1
.					\" calculate total pages in job (default 2)
.					\" a1 - page 1 portion abstract (units)
.nr a1 \\nyu-\\nxu-1v
.ie \\n(la>\\n(a1 \{\
.					\" ar - remainder abstract (units)
.	nr ar \\n(la-\\n(a1
.	nr tp \\n+(tp
.					\" a2 - available continuation page space
.					\" m1 is mark after tk line after title
.					\" 2v for Abstract (continued) + one blank
.	nr a2 11.0i-\\n(m1-2v
.	ZZ \}
.el .if \\nc>0 .nr tp \\n+(tp
..
.de ZZ				\" ZZ = mm(t1)
.ie \\n(ar>\\n(a2 \{\
.	nr ar \\n(ar-\\n(a2
.	nr tp \\n+(tp
.	ZZ \}
.el .if \\n(ar+\\nv>\\n(a2 .nr tp \\n+(tp
..
'''\"
'''\" main macro to handle output of cover sheet
'''\"
.de SC
.if \\nu=0 \{\
.	tm WARNING: author must be supplied \}
.if \\no=0 \{\
.	tm WARNING: document title must be supplied \}
.if \\nm=0 \{\
.	tm WARNING: memorandum type undefined or unknown \}
.if \\nj=0 \{\
.	tm WARNING: CI-II Review information  must be supplied \}
.if \\nn=0 \{\
.	tm WARNING: document number must be supplied \}
.ll 7.5i
.ft 1
.if \\n(nl .bp
.in 0
.HD
'''\" the rs is to restore spacing - ditches big space at top
.rs
.sp|0.2i
'''\"					put out timestamp if non-empty
.ie !'\\*(s)'0' \{\
.	ti 6.5i
\\*(s) \}
.el .sp1
.sp 0.05i
.nf
.ps 16
.ft 3
.ta 4.85i
.					\" put out page 1 heading
.ie \\n(dv=1 	Document Cover Sheet
.el AT&T Bell Laboratories	Document Cover Sheet
.wh 0 HD
.sp 0.1i
.ti 4.65i
.ie \\n(dv=1 \{\
.	ti 0
.	ta 0.25i 4.55i
	\s36\(Lb\s0	\\*(t( \}
'''\"					put out memorandum type
.el \\*(t(
.ft
.ps 10
.HX
.sp 0.05i
.ft 3
.ie \\nu>1 .ds u) s
.el .ds u)
.ta 1.5i 3.75i 5.1i 6.0i
	Author\\*(u)	Location	Ext.	Dept.
.ft
'''\"					output author info
.ZI
.TK
.sp 0.05i
.ft 3
.ta 1.0i 3.3i 5.3i
	Document No\\*(v).	Filing Case No\\*(w).	Work Project No\\*(x).
.ft
.sp 0.05i
'''\"					output document number
.ZN
.TK
'''\"					output keywords if they exist
.if \\nk>0 \{\
.	ft 3
Keywords:
.	ft
.	sp 0.05i
.	ti 0.2i
\\*(k)
.	TK \}
'''\"					output mercury info if it exists
.if \\nb>0 \{\
.	ft 3
MERCURY Announcement Bulletin Sections
.	ft
.	sp 0.05i
.	ta 0.6i 3.1i 5.6i
.	ps 8
.	if \\nb>3 \\*(b)
\\*(a)
.	ps
.	TK \}
.ft 3
Abstract
.ft
.mk x
.nr b1 \\nx/\\n(.v+1
.nr b2 (\\n(b1*\\n(.v)-\\nx
.sp \\n(b2u
.mk x
'''\" calculate position (17v includes 2v to print version at bottom of page)
.nr y \\n(lpv-17v
.if \\n(F4>0 .nr y \\ny-\\n(F4-1v
.sp|\\nyu
.sp -1
.ZP
'''\"					handle abstract page 1 continuation
.ie \\n(la>\\n(a1 \{\
.	ce
.	ft2
(continued on page iii)
.	ft1
.	br \}
.el .sp1
.if \\n(F4>0 \{\
.	FA
.	FG \}
.TK
.ps 8
.vs 10
.nr z \\$1
.nr k \\$2
.nr q \\nz+\\nk
Pages of Text  \\s+2\\$1  \\s-2Other Pages  \\s+2\\$2  \\s-2Total  \\s+2\\nq\\s-2
.sp 0.05i
.ie '\\$3'' .nr f 0
.el .nr f \\$3
.ie '\\$4'' .nr z 0
.el .nr z \\$4
.ie '\\$5'' .nr k 0
.el .nr k \\$5
No. Figs.  \\s+2\\nf  \\s-2No. Tables  \\s+2\\nz  \\s-2No. Refs.  \\s+2\\nk\\s-2
.ps
.vs
.mk z
.sp .67i
'''\"					output proprietary notice if it exists
.if \\np=1 \{\
.	ft 2
.	ti 1.1i
\\*(o)
.	ft
.	ti 1.75i
\\*(p) \}
.if \\np=2 \{\
.	ft 2
.	ti 0.6i
\\*(o)
.	ft
.	ti 0.875i
\\*(p)
.	ti 1.875i
\\*(q) \}
.if '\\*(b('1' \{\
.	ds p) "CI-II
.	ds q) "Not for disclosure to AT&T Information Systems
.	ds r) "Subject to FCC separation requirements under Computer Inquiry II
.	sp1
.	ti 2.35i
\\*(p)
.	ti 1.1i
\\*(q)
.	ti 0.5i
\\*(r)
.	in 0 \}
.sp |\\nzu+12v
.ps 8
\\*(ve
.ps
.sp|\\nzu
.sp 1
.ft 3
.ti 5.25i
Mailing Label
.ft
.sp|\\nxu
.in 0.2i
.nf
'''\"					abstract
.if !\\n(la=\\n(a1 \{\
.	wh -0.30i ST \}
.pl \\nyu
.ta 0.5i 1.0i 1.5i 2.0i 2.5i
.FB
.ft 1
'''\"					output the abstract
.ZA
.if \\n(la=\\n(a1 .sp-1
.rn ZB XX
.wh -0.05i FC
'''\"					output continuation page
.CP
'bp
.FT
.ft 1
.in 0
.wh 0 HD
.nf
.ft 3
.ta 5.25i
Initial Distribution Specifications	\\*(N1\f2 (page ii of \\n(tp)\f3
.ft 1
.TK
.if \\ne>0 \{\
.	DL "     Complete Copy" "     Cover Sheet Only"
.	mk z
'''\"					put out complete copy list
.	ZC
.	if !\\ng=0 \{\
.		ft 2
.		ti 1.25i
(continued)
.		ft 1 \}
.	sp|\\nzu
.	in 4i
'''\"					put out cover sheet list
.	ZS
.	if !\\ni=0 \{\
.		ft 2
.		ti 4.75i
(continued)
.		ft 1 \}
.	in 0
'''\"					starter space value - then tailor
.	sp|5
.	sp \\ne
.	TK \}
'''\"					output proprietary section if not default
.if \\np>1 \{\
\f3Proprietary Classification\f1
.	sp1
.	nf
.	ft 2
.	in 0.2i
AT&T BELL LABORATORIES PROPRIETARY - (RESTRICTED)
.	ft 1
  Solely for authorized persons having a need to know pursuant to G.E.I. 2.2
.	in 0
.	sp1
.	if \\n(u!=1 \{\
.	ta 3.25i
\f3	Approval: \f1\l'3.5i'
.	ta 5.3i
	Department Head \}
.if \\n(u!>1 \{\
.	ta 0.2i 1.0i 3.25i 5.5i
\f3	Approval:\f1	\l'1.9i'	\l'2.0i'	\l'2.0i'
.	ta 0.15i
	( Department Heads ) \}
.sp -0.1i
.	TK \}
'''\"					put out security section if selected
.if \\nr=1 \{\
\f3Government Security Classified.\f1  See AT&T-BL \f2Security Handbook.
.	ft 1
.	sp -0.05i
.	TK \}
.ft 3
'''\"					\" always put out CI-II Review
CI-II Review
.ft 1
.in 0.2i
.fi
.ie '\\*(b('1' \{\
This document \f2contains\f1 one or more of the types of information listed
below and, in accordance with CI-II, \f2may not\f1 be furnished to AT&T-IS. \}
.el \{\
This document \f2does not contain\f1 any of the types of information listed
below and, in accordance with CI-II, \f2may\f1 be furnished to AT&T-IS. \}
.sp1
.ps-3
.vs-4
.in 0.3i
\f2Network Information\f1 - Unpublished information related to the existing
operation of or committed changes to intercarrier
connection or the connection and/or operation of customer premises equipment
with AT&T Communications network(s) by means of which regulated carrier
services are furnished.
.sp1
\f2Proprietary Information of Telecommunications Customers\f1 - Unpublished
information acquired from billing or message detail records that is related to
telecommunications service provided by AT&T Communications to specifically
identified customers and that finds a principal use in marketing (e.g.,
information describing the kinds and quantities of telecommunications service
provided to an identified customer or information describing traffic and usage
patterns of an identified customer).
.sp1
\f2Nongeneric Software for Customer Premises Equipment (CPE) or Enhanced Services\f1 - 
Any software that is of use by Information Systems in CPE products or enhanced
services and that is not generic software.
.br
.ps
.vs
.nf
.in0
.sp1
.if \\n(ud=1 \{\
.	ta 3.25i
\f3	Approval: \f1\l'3.5i'
.	ta 5.5i
	Director \}
.if \\n(ud>1 \{\
.	ta 0.2i 1.0i 3.25i 5.5i
\f3	Approval:\f1	\l'1.9i'	\l'2.0i'	\l'2.0i'
.	ta 0.15i
	( Directors ) \}
.sp -0.1i
.TK
'''\"					put out author signature section
.ft 3
Author Signature\\*(u)
.ft 1
.sp1
.ta 2.635i 5.25i
\\*(m!
.ta 0.25i 2.875i 5.5i
\\*(m(
.if \\nu>3 \{\
.	sp 0.1i
.	ta 2.635i 5.25i
\\*(n!
.	ta 0.25i 2.875i 5.5i
\\*(n( \}
.if \\nu>6 \{\
.	sp 0.1i
.	ta 2.635i 5.25i
\\*(o!
.	ta 0.25i 2.875i 5.5i
\\*(o( \}
.sp -0.1i
.TK
'''\"					recipient section always output
.ft 3
For Use by Recipient of Cover Sheet:
.ft 1
.ps -3
.vs -4
.sp1
.mk z
     Computing network users may order copies via the \f2library \f1 command;
      for information, type "man library" after logon.
     Otherwise:
.sp1
.rn fo xx
1 Enter PAN if AT&T-BL (or SS# if non-AT&T-BL). \l'1.4i'
2 Fold this sheet in half with this side out.
3 Check the address of your local Internal Technical Document Service
   if listed; otherwise, use HO 4F-112. Use no envelope.
4 Indicate whether microfiche or paper copy is desired.
.sp|\\nzu
.in 4i
Internal Technical Document Service
.sp1
.ta 1i
( )  AK 2N-02	( ) IH 7K-101
( )  ALC 1B-102A	( ) MV 1D-40
( )  CB 1C-338	( ) RD 20D-215
( )  HO 4F-112	( ) WH 3E-204
.sp1
Please send a complete \\s+2\\(sq\\s-2 microfiche \\s+2\\(sq\\s-2 paper copy of this document to
the address shown on the other side.
.in
.ps
.vs
.SR
..
.	\"IZ - initialization
.de IZ
.fp 1 R
.fp 2 I
.fp 3 B
.fp 4 BI
.nr TN 0
.em EM
.po .5i
.nr PO .5i
.if \\n(FM=0 .nr FM 1i
.nr YY 0-\\n(FMu
.if !\\n(PD .if n nr PD 1v
.if t .if !\\n(PD .nr PD 0.3v
.wh 0 NP
.wh \\n(.pu-\\n(FMu FO
.ch FO 16i
.wh \\n(.pu-\\n(FMu FX
.ch FO \\n(.pu-\\n(FMu
.if t .wh -\\n(FMu/2u BT
.if n .wh -\\n(FMu/2u-1v BT
..
.\"		macro to restore ms foiling
.de SR
.nr BE 0
.nr 1T 0
.nr FM 0
.nr PD 0
.nr HM 0
.nr KG 0
.nr FP 0
.nr GA 0
.nr FP 0
.\" changed rn F5 FE added rn FJ FS
.bp
.nr FC -1
.nr % 1
.IZ
.rm IZ
.RT
.rn ZT WT
.rn d) DY
.ds MN \\*(N1 \\*(N2 \\*(N3
.nr MM \\nn
.nr MC \\n(fc
.nr MG \\n(wp
.nr NA \\nu
.so /usr/lib/tmac/tmac.rscover
. \" a line for troff to eat
.S1
.ll 6i
.nr LL 6i
.rr a b c d e f g h i j k
.rr l m n o p q r s t u
.rr v w x y z
.rr lp np la a1 a2 ar wp fc m1
.rm DT TI AH ZA SE KW TY NU MY
.rm PR CI CO OC CV OV CE HD
.rm FC ST TK HX HC DL EJ
.rm CP SC a) b) k) N1 p) q) r)
.rm d) o) s) ve m! n! o!
.rm t) w) x) y) z) a( b( c( m(
.rm n( o( p( s( t( SA ZI ZC ZO
.rm ZS ZD ZN FT FB ZT CX NN GS
.rm ZB XX ZP ZZ
..
