# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT 
# Writer's Workbench version 2.x, February 1982  llc's version
#	        make file for THE WRITER'S WORKBENCH
#
#	This package includes programs that suggest improvements to
#	written documents.  Many of the programs are built on Style
#	and Diction - this make file assumes deroff, style, and diction
#	are normal commands on the system & doesn't include them
#
#
#	LIB is the directory for storing the object code
#		for all programs run by shell files. 
#	BIN is the directory used for storing the shell files that run
#		the programs and the object code for deroff.
#
LIB = /usr/lib/style
BIN = /usr/bin/WWB
# BIN = /usr/llc/wwb/commands/bin
LDFLAGS = -ll
CFLAGS = -O
LCFLAGS = -O

all: wwb proofr dictadd

wwb:	proofr prose chunk org syl mkstand

proofr:	punct double gram

punct:	punlx

punlx: punct.l
	lex punct.l
	$(CC) $(LCFLAGS)  lex.yy.c $(LDFLAGS) -o punlx

dictadd:	dictadd.c
		$(CC) $(CFLAGS)  dictadd.c -o dictadd

double: double.c cbtype.c cbtype.h
	cc $(CFLAGS) double.c cbtype.c -o double

gram:	gramlx

gramlx:	gram.l
	lex gram.l
	$(CC) $(CFLAGS)  lex.yy.c $(LDFLAGS) -o gramlx

prose:	prose.c
	$(CC) $(CFLAGS)  -DLIB=\"$(LIB) prose.c -o prose

chunk:	chunk.c
	$(CC) $(CFLAGS)  chunk.c -o chunk

org:	orglx

orglx:	org.l
	lex org.l
	$(CC) $(CFLAGS)  lex.yy.c $(LDFLAGS) -o orglx

mkstand:	mkstand.c
	$(CC) $(LCFLAGS)  mkstand.c -lm -o mkstand

syl:	syl.c
	$(CC) $(CFLAGS)  syl.c -o syl

install:	all
	: PROOFR
		cp dictadd.sh $(BIN)/dictadd
		cp dictplus.sh $(BIN)/dictplus
		cp script.sed $(LIB)/script.sed
		mv double $(BIN)/double
		cp proofr.sh $(BIN)/proofr
		cp spelladd.sh $(BIN)/spelladd
		cp spellwwb.sh $(BIN)/spellwwb
		cp gram.sh $(BIN)/gram
		mv gramlx $(LIB)/gramlx
		cp splitrules.sh $(BIN)/splitrules
		rm -f $(BIN)/proofer
		ln $(BIN)/proofr $(BIN)/proofer
		mv dictadd $(LIB)
		cp punctrules.sh $(BIN)/punctrules
		cp punct.sh $(BIN)/punct
		mv punlx $(LIB)/punlx
		cp seddiff $(LIB)

	: PROSE
		cp prose.sh $(BIN)/prose
		for i in crft.st exp.t fur.t nom.t pass.t poptt.t popttm.t \
			tm.st train.st type.t rd2.t \
			type2.t var.t var4.t var5.t var6.t ; \
		do	cp $$i $(LIB); done
		mv prose $(LIB)
	: CHUNK
		cp chunk.sh $(BIN)/chunk
		mv chunk $(LIB)
		cp chkin.sed $(LIB)
		cp chkout.sed $(LIB)
	: ORG
		cp org.sh $(BIN)/org
		mv orglx $(LIB)
	: SYL
		cp syl.sh $(BIN)/syl
		mv syl $(LIB)
	: MATCH
		cp match.sh $(BIN)/match
	: WWB
		cp wwb.sh $(BIN)/wwb
		cp wwbinfo.sh $(BIN)/wwbinfo

	: SEXIST
		cp sexist.sh $(BIN)/sexist
		cp sexist.sg $(LIB)
		cp sexist.d $(LIB)
	: ACRO
		cp acro.sh $(BIN)/acro
		cp acro.sed $(LIB)

	: MKSTAND
		cp mkstand.sh $(BIN)/mkstand
		mv mkstand $(LIB)
	: WORDUSE
		cp worduse.sh $(BIN)/worduse
		cp wordlist.d  $(LIB)
	: SPELLTELL
		cp spelltell.sh $(BIN)/spelltell
		cp spelllist.d  $(LIB)
	: WWBHELP
		cp wwbhelp.sh $(BIN)/wwbhelp
		cp helplist.d  $(LIB)
	: WWBMAIL
		cp wwbmail.sh $(BIN)/wwbmail
	: WWBSTAND
		cp wwbstand.sh $(BIN)/wwbstand
		cp standlkup.sh  $(LIB)/standlkup
clean:
	rm -f lex.yy.c
	rm -f *.o chunk dictadd double gramlx
	rm -f mkstand orglx prose punlx syl punct org gram
