M4 = @m4

MHOME = /usr/lib/monk

INSDIR = $(MHOME)/db

COMPRESS = $(MHOME)/compress -d .

FRC =

DBS = \
	attrib.def \
	cspress.doc \
	global.def \
	letter.doc \
	macro.def \
	model.doc \
	rp.doc \
	song.doc \
	tm.doc

CDBS = \
	cspress.comp \
	global.comp \
	letter.comp \
	macro.comp \
	model.comp \
	rp.comp \
	song.comp \
	tm.comp

all: $(DBS) $(CDBS)

attrib.def: \
	attrib.m \
	macro.h \
	list.h \
	style.h \
	whitespace.h \
	m4.h \
	$(FRC)
	$(M4) m4.h attrib.m | sed \
	-e "s/^\([.']\)[	 ]*\([a-z][a-z]\) /\1\2/" \
	-e "s/^\([.']\)[	 ]*/\1/" \
	-e '/^.\\"/d' \
	-e 's/[	 ]*\\".*//' \
	-e 's/^|||*/|/' \
	-e '/^|[| ]*$$/d' \
	-e 's/\\n\([0-9]\)/\1/g' \
	-e 's/\\n\$$i/$$i/g' \
	-e 's/^[	 ]*//' \
	> attrib.def
	@echo "attrib.def updated"

cspress.doc: \
	cspress.m \
	envir.h \
	preproc.h \
	monk.h \
	macro.h \
	titlebox.h \
	titlebox.m \
	m4.h \
	$(FRC)
	$(M4) m4.h cspress.m | sed \
	-e 's/^|||*/|/' \
	-e '/^|[| ]*$$/d' \
 	-e 's/\\n\([0-9]\)/\1/g' \
	-e 's/\\n\$$i/$$i/g' \
	-e 's/^[	 ]*//' \
	> cspress.doc
	@echo "cspress.doc updated"

cspress.comp: cspress.doc attrib.def global.def macro.def $(FRC)
	$(COMPRESS) cspress

global.def: \
	global.m \
	global.h \
	monk.h \
	macro.h \
	style.h \
	style.m \
	section.h \
	section.m \
	envir.h \
	envir.m \
	preproc.h \
	preproc.m \
	list.h \
	list.m \
	refs.h \
	refs.m \
	whitespace.h \
	whitespace.m \
	m4.h \
	$(FRC)
	$(M4) m4.h global.m | sed \
	-e 's/^|||*/|/' \
	-e '/^|[| ]*$$/d' \
	-e 's/\\n\([0-9]\)/\1/g' \
	-e 's/\\n\$$i/$$i/g' \
	-e 's/^[	 ]*//' \
	> global.def
	@echo "global.def updated"

global.comp: attrib.def global.def macro.def $(FRC)
	$(COMPRESS)

letter.doc: \
	letter.m \
	monk.h \
	macro.h \
	m4.h \
	$(FRC)
	$(M4) m4.h letter.m | sed \
	-e 's/^|||*/|/' \
	-e '/^|[| ]*$$/d' \
 	-e 's/\\n\([0-9]\)/\1/g' \
	-e 's/\\n\$$i/$$i/g' \
	-e 's/^[	 ]*//' \
	> letter.doc
	@echo "letter.doc updated"

letter.comp: letter.doc attrib.def global.def macro.def $(FRC)
	$(COMPRESS) letter

macro.def: \
	macro.m \
	macro.h \
	m4.h \
	$(FRC)
	$(M4) m4.h macro.m | sed \
	-e "s/^\([.']\)[	 ]*\([a-z][a-z]\) /\1\2/" \
	-e "s/^\([.']\)[	 ]*/\1/" \
	-e '/^.\\"/d' \
	-e 's/[	 ]*\\".*//' \
	-e 's/^|||*/|/' \
	-e '/^|[| ]*$$/d' \
	-e 's/\\n\([0-9]\)/\1/g' \
	-e 's/\\n\$$i/$$i/g' \
	-e 's/^[	 ]*//' \
	> macro.def
	@echo "macro.def updated"

macro.comp: macro.def attrib.def global.def macro.def $(FRC)
	$(COMPRESS)

model.doc: \
	model.m \
	envir.h \
	preproc.h \
	monk.h \
	macro.h \
	refs.h \
	titlebox.h \
	titlebox.m \
	m4.h \
	$(FRC)
	$(M4) m4.h model.m | sed \
	-e 's/^|||*/|/' \
	-e '/^|[| ]*$$/d' \
 	-e 's/\\n\([0-9]\)/\1/g' \
	-e 's/\\n\$$i/$$i/g' \
	-e 's/^[	 ]*//' \
	> model.doc
	@echo "model.doc updated"

model.comp: model.doc attrib.def global.def macro.def $(FRC)
	$(COMPRESS) model

rp.doc: \
	rp.m \
	envir.h \
	preproc.h \
	monk.h \
	macro.h \
	titlebox.h \
	titlebox.m \
	m4.h \
	$(FRC)
	$(M4) m4.h rp.m | sed \
	-e 's/^|||*/|/' \
	-e '/^|[| ]*$$/d' \
 	-e 's/\\n\([0-9]\)/\1/g' \
	-e 's/\\n\$$i/$$i/g' \
	-e 's/^[	 ]*//' \
	> rp.doc
	@echo "rp.doc updated"

rp.comp: rp.doc attrib.def global.def macro.def $(FRC)
	$(COMPRESS) rp

song.doc: \
	song.m \
	monk.h \
	m4.h \
	$(FRC)
	$(M4) m4.h song.m | sed \
	-e 's/^|||*/|/' \
	-e '/^|[| ]*$$/d' \
 	-e 's/\\n\([0-9]\)/\1/g' \
	-e 's/\\n\$$i/$$i/g' \
	-e 's/^[	 ]*//' \
	> song.doc
	@echo "song.doc updated"

song.comp: song.doc attrib.def global.def macro.def $(FRC)
	$(COMPRESS) song

tm.doc: \
	tm.m \
	envir.h \
	preproc.h \
	monk.h \
	macro.h \
	titlebox.h \
	titlebox.m \
	coversheet.h \
	coversheet.m \
	m4.h \
	$(FRC)
	$(M4) m4.h tm.m | sed \
	-e 's/^|||*/|/' \
	-e '/^|[| ]*$$/d' \
 	-e 's/\\n\([0-9]\)/\1/g' \
	-e 's/\\n\$$i/$$i/g' \
	-e 's/^[	 ]*//' \
	> tm.doc
	@echo "tm.doc updated"

tm.comp: tm.doc attrib.def global.def macro.def $(FRC)
	$(COMPRESS) tm

clean:

clobber: clean
	-rm -f $(DBS) $(CDBS)

install: all
	cp $(DBS) $(CDBS) $(INSDIR)
