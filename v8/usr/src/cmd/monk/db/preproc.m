|environment(reference;
	verbatim on, reference on;)
	|comment<summary: reference (a reference in refer)>

|environment(picture;
	verbatim on, file.information, blank.lines 1, picture on;

	blank.lines 1)
	|comment<summary: picture (a picture in pic)>

|environment(picture_caption;
	incr figure_counter,
	blank.lines 1, fill on, center on, temporary.font bold,
	text "Figure ", number figure_counter, text ". ", temporary.font;

	blank.lines 1, store string.from.number remember_string figure_counter)
	|comment<summary: picture_caption (the picture's caption; IT)>

|environment(table;
	verbatim on, file.information, blank.lines 1, table on;

	blank.lines 1)
	|comment<summary: table (a table in tbl)>

|environment(table_caption;
	incr table_counter,
	blank.lines 1, fill on, center on, temporary.font bold,
	text "Table ", number table_counter, text ". ", temporary.font;

	blank.lines 1, store string.from.number remember_string table_counter)
	|comment<summary: table_caption (the table's caption; IT)>

|environment(e;
	|ifvalue verbatim off [
		file.information,
	]
	inline.equation on;)
	|comment<summary: e (an inline equation in eqn)>

|environment(equation;
	verbatim on, file.information, blank.lines 1, equation on;

	blank.lines 1)
	|comment<summary: equation (an equation in eqn)>

|environment(equation_caption;
	incr figure_counter,
	blank.lines 1, fill on, center on, temporary.font bold,
	text "Figure ", number figure_counter, text ". ", temporary.font;

	blank.lines 1, store string.from.number remember_string figure_counter)
	|comment<summary: equation_caption (the equation's caption; IT)>

|associate(graph;
	verbatim on, file.information, blank.lines 1, graph on preproc_argument;

	blank.lines 1, clear string preproc_argument;

	[width $; set string preproc_argument $;])
	|comment<summary: graph (a graph in grap)>

|environment(graph_caption;
	incr figure_counter,
	blank.lines 1, fill on, center on, temporary.font bold,
	text "Figure ", number figure_counter, text ". ", temporary.font;

	blank.lines 1, store string.from.number remember_string figure_counter)
	|comment<summary: graph_caption (the graph's caption; IT)>

|environment(bargraph;
	verbatim on, file.information, blank.lines 1, bargraph on;

	blank.lines 1)
	|comment<summary: bargraph (a bar graph in bar)>

|environment(bargraph_caption;
	incr figure_counter,
	blank.lines 1, fill on, center on, temporary.font bold,
	text "Figure ", number figure_counter, text ". ", temporary.font;

	blank.lines 1, store string.from.number remember_string figure_counter)
	|comment<summary: bargraph_caption (the bar graph's caption; IT)>

|environment(figure;
	verbatim on, file.information, blank.lines 1, fill off;

	blank.lines 1)
	|comment<summary: figure (a figure)>

|environment(figure_caption;
	incr figure_counter,
	blank.lines 1, fill on, center on, temporary.font bold,
	text "Figure ", number figure_counter, text ". ", temporary.font;

	blank.lines 1, store string.from.number remember_string figure_counter)
	|comment<summary: figure_caption (the figures caption; IT)>

|environment(computeroutput;
	verbatim on, file.information, blank.lines 1, fill off, font cw;

	blank.lines 1)
	|comment<summary: computeroutput (computeroutput example using constant width)>

|environment(computeroutput_caption;
	incr figure_counter,
	blank.lines 1, fill on, center on, temporary.font bold,
	text "Figure ", number figure_counter, text ". ", temporary.font;

	blank.lines 1, store string.from.number remember_string figure_counter)
	|comment<summary: computeroutput_caption (computeroutput example's caption)>

|environment(save;
	verbatim on, citation save;

	citation save end remember_string)
	|comment<summary: save (save last important thing, shown as IT, in this label)>

|environment(savepage;
	verbatim on, citation save;

	citation savepage end)
	|comment<summary: savepage (save last page number in this label)>

|environment(remember;
	verbatim on, citation remember;

	citation remember end)
	|comment<summary: remember (remember the important thing or page number saved in this label)>
