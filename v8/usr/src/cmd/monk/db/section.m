|environment(chapter;
	new.page, blank.lines 4, center on, font.size +1, font bold,
	incr chapter_counter,
	clear section_counter subsection_counter subsubsection_counter
	paragraph_counter,
	text "Chapter ", concatenate, number chapter_counter, new.line;

	blank.lines 4, paragraph,
	store string.from.number remember_string chapter_counter)
	|comment<summary: chapter (numbered chapter; IT)>

|environment(section;
	blank.lines 2, protect, font bold, incr section_counter,
	clear subsection_counter subsubsection_counter paragraph_counter,
	number section_counter, text ". ";

	paragraph,
	store string.from.number remember_string section_counter)
	|comment<summary: section (numbered section; IT)>

|environment(subsection;
	blank.lines, protect, font bold, incr subsection_counter,
	clear subsubsection_counter paragraph_counter,
	number section_counter, text ".",
	number subsection_counter, text " ";

	paragraph,
	store string.from.number remember_string section_counter,
	add string remember_string ".",
	add string.from.number remember_string subsection_counter)
	|comment<summary: subsection (numbered subsection; IT)>

|environment(subsubsection;
	blank.lines, protect, font italics, incr subsubsection_counter,
	clear paragraph_counter,
	number section_counter, text ".",
	number subsection_counter, text ".",
	number subsubsection_counter, text " ";

	store string.from.number remember_string section_counter,
	add string remember_string ".",
	add string.from.number remember_string subsection_counter,
	add string remember_string ".",
	add string.from.number remember_string subsubsection_counter)
	|comment<summary: subsubsection (numbered subsubsection; IT)>

|associate(paragraph;
	blank.lines, clear string temporary_string;

	if begin not_null_string(temporary_string),
		temporary.font italics, string temporary_string, temporary.font,
	if end;

	[numbered;
		incr paragraph_counter, number format paragraph_counter i,
		store string.from.number temporary_string paragraph_counter,
		add string temporary_string ") ";]
	[n;
		incr paragraph_counter, number format paragraph_counter i,
		store string.from.number temporary_string paragraph_counter,
		add string temporary_string ") ";]
	[indented;
		indent.line 3;]
	[i;
		indent.line 3;])
	|comment<summary: paragraph (paragraph n, i, numbered or indented)>

|associate(p;
	blank.lines, clear string temporary_string;

	if begin not_null_string(temporary_string),
		temporary.font italics, string temporary_string, temporary.font,
	if end;

	[numbered;
		incr paragraph_counter, number format paragraph_counter i,
		store string.from.number temporary_string paragraph_counter,
		add string temporary_string ") ";]
	[n;
		incr paragraph_counter, number format paragraph_counter i,
		store string.from.number temporary_string paragraph_counter,
		add string temporary_string ") ";]
	[indented;
		indent.line 3;]
	[i;
		indent.line 3;])
	|comment<summary: p (alias for paragraph)>

|environment(appendix;
	new.page, blank.lines 4, center on, font.size +1, font bold,
	incr appendix_counter,
	clear section_counter subsection_counter subsubsection_counter,
	text "Appendix ", concatenate, number appendix_counter A, new.line;

	blank.lines 4, paragraph,
	store string.from.number remember_string appendix_counter)
	|comment<summary: appendix (numbered appendix; IT)>

|environment(unnumbered_chapter;
	new.page, blank.lines 4, center on, font.size +1, font bold;

	blank.lines 4, paragraph)
	|comment<summary: unnumbered_chapter (unnumbered chapter)>

|environment(unnumbered_section;
	blank.lines 2, protect, font bold;

	paragraph)
	|comment<summary: unnumbered_section (unnumbered section)>

|environment(unnumbered_subsection;
	blank.lines, protect, font bold;

	paragraph)
	|comment<summary: unnumbered_subsection (unnumbered subsection)>

|environment(unnumbered_subsubsection;
	blank.lines, protect, font italics;)
	|comment<summary: unnumbered_subsubsection (unnumbered subsubsection)>

|environment(unnumbered_appendix;
	new.page, blank.lines 4, center on, font.size +1, font bold;

	blank.lines 4, paragraph)
	|comment<summary: unnumbered_appendix (unnumbered appendix)>
