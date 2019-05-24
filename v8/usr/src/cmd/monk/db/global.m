|include(global.h)
|include(monk.h)
|include(macro.h)

|include(whitespace.h)
|include(whitespace.m)

|include(style.h)
|include(style.m)

|include(section.h)
|include(section.m)

|include(envir.h)
|include(envir.m)

|include(preproc.h)
|include(preproc.m)

|include(list.h)
|include(list.m)

|include(refs.h)
|include(refs.m)

|environment(global_begin_and_end_of_text;
	file.information,
	clear appendix_counter section_counter subsection_counter
		subsubsection_counter footnote_counter table_counter
		figure_counter reference_counter,
	set string open_reference "\s-2\v'-.4m'\f1",
	set string close_reference "\v'.4m'\s+2\fP",
	set string open_double_quote "``",
	set string close_double_quote "''",
	set string open_period ".",
	set string open_comma ",";)
