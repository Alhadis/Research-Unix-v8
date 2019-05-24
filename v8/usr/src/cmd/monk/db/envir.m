|environment(bold;
	temporary.font bold; temporary.font)
	|comment<summary: bold (alias for b)>

|environment(b;
	temporary.font bold; temporary.font)
	|comment<summary: b (bold)>

|environment(italics;
	temporary.font italics; temporary.font)
	|comment<summary: italics (alias for i)>

|environment(i;
	temporary.font italics; temporary.font)
	|comment<summary: i (italics)>

|environment(roman;
	temporary.font roman; temporary.font)
	|comment<summary: roman (alias for r)>

|environment(r;
	temporary.font roman; temporary.font)
	|comment<summary: r (roman)>

|environment(constant_width;
	temporary.font cw; temporary.font)
	|comment<summary: constant_width (alias for cw)>

|environment(cw;
	temporary.font cw; temporary.font)
	|comment<summary: cw (constant width)>

|environment(here;
	here on, fill off;)
	|comment<summary: here (keep a block together on this page or start a new page)>

|environment(around;
	around on, fill off;)
	|comment<summary: around (keep a block together here or move it to the next page)>

|environment(around_placement;;
	around_placement)
	|comment<summary: around_placement (output all arounds here)>

|environment(full;
	width full;)
	|comment<summary: full (use the whole page)>

|environment(narrow;
	width narrow;)
	|comment<summary: narrow (use the just this column)>

|associate(blank_space;;;

	[$; spacing on, blank.lines $;])
	|comment<summary: blank_space (leave this much blank space)>

|environment(new_page;

	new.page;)
	|comment<summary: new_page (skip to the next next page)>

|environment(indent;
	indent +5;)
	|comment<summary: indent (indent by standard amount)>

|environment(center;
	center on;)
	|comment<summary: center (center each line)>

|environment(center_block;
	fill off, center.block on;)
	|comment<summary: center_block (center all lines as a block)>

|environment(small;
	size -1;)
	|comment<summary: small (reduce the point size)>

|environment(s;
	size -1;)
	|comment<summary: s (alias for small)>

|environment(big;
	size +1;)
	|comment<summary: big (increase point size)>

|environment(u;
	underline on;)
	|comment<summary: u (underline)>

|environment(underline;
	underline on;)
	|comment<summary: underline (alias for u)>

|environment(text;
	fill off;)
	|comment<summary: text (fill the text)>

|environment(left_adjust;
	adjust left;)
	|comment<summary: left_adjust (left adjust the text)>

|environment(footnote;
	incr footnote_counter,
	size -2, vertical.motion -.4m, temporary.font roman,
	number footnote_counter, temporary.font, vertical.motion +.4m,
	footnote on, indent 0.2i, indent.line -0.2i,
	number footnote_counter, text ".", horizontal.motion "|0.2i";)
	|comment<summary: footnote (numbered footnote)>

|environment(dagnote;
	size -2, vertical.motion -.4m, temporary.font roman,
	text "\(dg", temporary.font, vertical.motion +.4m,
	footnote on, indent 0.2i, indent.line -0.2i,
	text "\(dg", horizontal.motion "|0.2i";)
	|comment<summary: dagnote (footnote marked with a dagger)>

|environment(comment;
	divert.input on remember_string;)
	|comment<summary: comment (kludge for sharon to comment out user text)>

|environment(notation;
	blank.lines, fill off;
	blank.lines)
	|comment<summary: notation (notation)>
