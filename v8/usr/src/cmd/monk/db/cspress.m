|include(envir.h)
|include(preproc.h)
|include(monk.h)
|include(macro.h)

|include(titlebox.h)
|include(titlebox.m)

|environment(titlebox;
	blank.lines 2.75i;)
	|comment<summary: titlebox (produce title box here)>

|environment(document_begin_and_end_of_text;
	clear author_counter document_counter,
	set page_width 5.25i, line.length,
	set page_length 10.625i, page.length,
	set font_size 12, font.size;)
