|include(envir.h)
|include(preproc.h)
|include(monk.h)
|include(macro.h)

|include(titlebox.h)
|include(titlebox.m)

|include(coversheet.h)
|include(coversheet.m)

|environment(document_begin_and_end_of_text;
	clear author_counter document_counter mercury_counter
		other_page_counter text_page_counter,
	clear string government_string proprietary_string;)

|environment(signature;
	here on, blank.lines 1, indent 30, font bold,
|for i in 1 2 3 4 5 6 7 8 9 [
	if begin ge(author_counter, $i),
		blank.lines 3, string author_array($i),
	if end,
]
	font, indent, blank.lines -1

	;blank.lines 3)
	|comment<summary: signature (signatures and typing credits)>
