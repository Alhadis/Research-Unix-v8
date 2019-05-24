|associate(author;
	incr author_counter, clear temporary_counter,
	clear string temporary_array(1) temporary_array(2) temporary_array(3)
	temporary_array(4) temporary_array(5) temporary_array(6);

	store string author_array(author_counter) temporary_array(1),
	store string initials_array(author_counter) temporary_array(2),
	store string location_array(author_counter) temporary_array(3),
	store string department_array(author_counter) temporary_array(4),
	store string extension_array(author_counter) temporary_array(5),
	store string room_array(author_counter) temporary_array(6);

	[name $;
		incr temporary_counter, set string temporary_array(1) $;]
	[initials $;
		incr temporary_counter, set string temporary_array(2) $;]
	[location $;
		incr temporary_counter, set string temporary_array(3) $;]
	[department $;
		incr temporary_counter, set string temporary_array(4) $;]
	[extension $;
		incr temporary_counter, set string temporary_array(5) $;]
	[room $;
		incr temporary_counter, set string temporary_array(6) $;]
	[$;
		incr temporary_counter,
		set string temporary_array(temporary_counter) $;])
	|comment<summary: author (name, initials, location, department, extension, and room for title box and coversheet)>

|associate(document;
	incr document_counter, clear temporary_counter,
	clear string temporary_array(1) temporary_array(2) temporary_array(3);

	store string document_array(document_counter) temporary_array(1),
	store string file_case_array(document_counter) temporary_array(2),
	store string work_program_array(document_counter) temporary_array(3);

	[number $;
		incr temporary_counter, set string temporary_array(1) $;]
	[file_case $;
		incr temporary_counter, set string temporary_array(2) $;]
	[work_program $;
		incr temporary_counter, set string temporary_array(3) $;]
	[$;
		incr temporary_counter,
		set string temporary_array(temporary_counter) $;])
	|comment<summary: document (number, file_case, and work_program for title box and coversheet)>

|environment(title;
	divert.input on title_string;)
	|comment<summary: title (title of document for title box and coversheet)>

|environment(date;
	divert.string date_string;)
	|comment<summary: date (date of tm for title box and coversheet)>

|environment(abstract;
	divert.input on abstract_string;)
	|comment<summary: abstract (abstract for coversheet)>

|environment(keywords;
	divert.input on keyword_string;)
	|comment<summary: keywords (keywords for cover sheet)>

|environment(titlebox;
	;

	new.page, indent.line +4.375i, string att_logo,
	blank.lines 2, size -2, remember position0,
	page.offset -.5i, text "subject:", new.line,
	return position0, indent +4.875i, text "date:", blank.lines,
	text "from:", indent, new.line, page.offset, size,
	line.length 3i, font bold, return position0, fill off,
	macro title_string, fill on, line.length, new.line,
	size -1,
|for i in 1 2 3 4 5 6 7 8 9 [
	if begin ge(document_counter, $i),
		new.line, text "Work Program ", string work_program_array($i),
		text " File Case ", string file_case_array($i),
	if end,
]
	size,
	return position0, indent +4.75i, string date_string,
|for i in 1 2 3 4 5 6 7 8 9 [
	if begin ge(author_counter, $i),
		blank.lines,
		string author_array($i), new.line,
		string location_array($i), text " ",
		string department_array($i), new.line,
		string room_array($i), text " ",
		string extension_array($i),
	if end,
]
	spacing on, blank.lines, fill off,
	string document_array(1), fill on, blank.lines 3,
	indent, center on, temporary.font italics "TECHNICAL MEMORANDUM",
	center off, blank.lines 2, font roman)
	|comment<summary: titlebox (produce title box here)>
