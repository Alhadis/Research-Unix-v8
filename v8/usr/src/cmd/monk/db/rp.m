|include(envir.h)
|include(preproc.h)
|include(monk.h)
|include(macro.h)

|include(titlebox.h)
|include(titlebox.m)

|define_counter(i_plus_one, i)

|environment(titlebox;
	;

	new.page, fill off, size +2, font bold, center on,
	macro title_string, font roman, size -2, fill on,
|for i in 1 2 3 4 5 6 7 8 9 [
	if begin ge(author_counter, $i),
		new.line, size +1, temporary.font italics,
		string author_array($i), temporary.font, size -1,
		if begin unequal_string_registers(location_array($i), location_array($i+1)),
			if begin equal_string(location_array($i), CM),
				text "Carnegie-Mellon University", new.line,
				text "Electrical and Computer Engineering Department", new.line,
				text "Pittsburgh, PA 15213", new.line,
			if end,
			if begin equal_string(location_array($i), HL),
				text "AT&T Bell Laboratories", new.line,
				text "Short Hills, New Jersey 07078", new.line,
			if end,
			if begin equal_string(location_array($i), HO),
				text "AT&T Bell Laboratories", new.line,
				text "Holmdel, New Jersey 07733", new.line,
			if end,
			if begin equal_string(location_array($i), MH),
				text "AT&T Bell Laboratories", new.line,
				text "Murray Hill, New Jersey 07974", new.line,
			if end,
			if begin equal_string(location_array($i), PY),
				text "AT&T Bell Laboratories", new.line,
				text "Piscataway, New Jersey 08854", new.line,
			if end,
			if begin equal_string(location_array($i), WH),
				text "AT&T Bell Laboratories", new.line,
				text "Whippany, New Jersey 07981", new.line,
			if end,
		if end,
	if end,
]

	blank.lines, size +1, temporary.font italics "ABSTRACT",
	size -1, center off, blank.lines, macro abstract_string, blank.lines,
)
	|comment<summary: titlebox (produce title box here)>

|environment(document_begin_and_end_of_text;
	clear author_counter document_counter;)
