|associate(mercury;
	clear temporary_counter, clear string temporary_array(1) temporary_array(2)
	temporary_array(3) temporary_array(4) temporary_array(5) temporary_array(6)
	temporary_array(7);

	if begin equal_string(temporary_array(1), yes),
		incr mercury_counter,
		set string mercury_array(mercury_counter)
		"CHM - Chemistry and Materials",
	if end,
	if begin equal_string(temporary_array(2), yes),
		incr mercury_counter, set string mercury_array(mercury_counter)
		"CMM - Communications",
	if end,
	if begin equal_string(temporary_array(3), yes),
		incr mercury_counter, set string mercury_array(mercury_counter)
		"CMP - Computing",
	if end,
	if begin equal_string(temporary_array(4), yes),
		incr mercury_counter, set string mercury_array(mercury_counter)
		"ELC - Electronics",
	if end,
	if begin equal_string(temporary_array(5), yes),
		incr mercury_counter, set string mercury_array(mercury_counter)
		"LFS - Life Sciences",
	if end,
	if begin equal_string(temporary_array(6), yes),
		incr mercury_counter,
		set string mercury_array(mercury_counter)
		"MAS - Mathematics and Statistics",
	if end,
	if begin equal_string(temporary_array(7), yes),
		incr mercury_counter, set string mercury_array(mercury_counter)
		"PHY - Physics",
	if end;

	[chm;
		incr temporary_counter, incr mercury_counter,
		set string mercury_array(mercury_counter)
		"CHM - Chemistry and Materials";]
	[cmm;
		incr temporary_counter, incr mercury_counter,
		set string mercury_array(mercury_counter) "CMM - Communications";]
	[cmp;
		incr temporary_counter, incr mercury_counter,
		set string mercury_array(mercury_counter) "CMP - Computing";]
	[elc;
		incr temporary_counter, incr mercury_counter,
		set string mercury_array(mercury_counter) "ELC - Electronics";]
	[lfs;
		incr temporary_counter, incr mercury_counter,
		set string mercury_array(mercury_counter) "LFS - Life Sciences";]
	[mas;
		incr temporary_counter, incr mercury_counter,
		set string mercury_array(mercury_counter)
		"MAS - Mathematics and Statistics";]
	[phy;
		incr temporary_counter, incr mercury_counter,
		set string mercury_array(mercury_counter) "PHY - Physics";]
	[yes;
		incr temporary_counter,
		set string temporary_array(temporary_counter) yes;]
	[no;
		incr temporary_counter,
		set string temporary_array(temporary_counter) no;])
	|comment<summary: mercury (chm, cmm, cmp, elc, lfs, mas, and phy mercury distribution for cover sheet)>

|associate(proprietary_class;;;
	[yes; set string proprietary_string yes;]
	[no; set string proprietary_string no;])
	|comment<summary: proprietary_class (proprietary classification for cover sheet)>

|associate(government_security;;;
	[yes; set string government_string yes;]
	[no; set string government_string no;])
	|comment<summary: government_security (government security clearance for cover sheet)>

|associate(earlier;
	clear temporary_counter, clear string temporary_array(1)
	temporary_array(2) temporary_array(3) temporary_array(4);

	store string earlier_document_number_string temporary_array(1),
	store string earlier_file_case_number_string temporary_array(2),
	store string earlier_author_string temporary_array(3),
	store string earlier_date_string temporary_array(4);

	[document_number $;
		incr temporary_counter, set string temporary_array(1) $;]
	[file_case_number $;
		incr temporary_counter, set string temporary_array(2) $;]
	[author $;
		incr temporary_counter, set string temporary_array(3) $;]
	[date $;
		incr temporary_counter, set string temporary_array(4) $;]
	[$;
		incr temporary_counter,
		set string temporary_array(temporary_counter) $;])
	|comment<summary: earlier (replacing earlier document_number, file_case_number, author and date for cover sheet)>

|associate(att;
	clear temporary_counter, clear string temporary_array(1) temporary_array(2);

	store string att_networking_string temporary_array(1),
	store string att_release_string temporary_array(2);

	[is_networking yes;
		incr temporary_counter, set string temporary_array(1) yes;]
	[is_networking no;
		incr temporary_counter, set string temporary_array(1) no;]
	[is_release yes;
		incr temporary_counter, set string temporary_array(2) yes;]
	[is_release no;
		incr temporary_counter, set string temporary_array(2) no;]
	[yes;
		incr temporary_counter,
		set string temporary_array(temporary_counter) yes;]
	[no;
		incr temporary_counter,
		set string temporary_array(temporary_counter) no;])
	|comment<summary: att (att is_networking and is_release for cover sheet)>

|environment(director_name;
	divert.string director_string;)
	|comment<summary: director_name (director's name for cover sheet)>

|environment(distribute_complete_memo;
	divert.input on complete_memo_string;)
	|comment<summary: distribute_complete_memo (distribution list for complete memo for cover sheet)>

|environment(distribute_cover_sheet;
	divert.input on cover_sheet_string;)
	|comment<summary: distribute_cover_sheet (distribution list for cover sheet for cover sheet)>

|associate(totals;
	clear temporary_counter,
	store temporary_array(1) page_counter,
	set temporary_array(2) 0,
	store temporary_array(3) figure_counter,
	store temporary_array(4) table_counter,
	store temporary_array(5) reference_counter;

	store text_page_counter temporary_array(1),
	store other_page_counter temporary_array(2),
	store figure_counter temporary_array(3),
	store table_counter temporary_array(4),
	store reference_counter temporary_array(5);

	[text $;
		incr temporary_counter, set temporary_array(1) $;]
	[other $;
		incr temporary_counter, set temporary_array(2) $;]
	[figures $;
		incr temporary_counter, set temporary_array(3) $;]
	[tables $;
		incr temporary_counter, set temporary_array(4) $;]
	[references $;
		incr temporary_counter, set temporary_array(5) $;]
	[$;
		incr temporary_counter, set temporary_array(temporary_counter) $;])
	|comment<summary: totals (text, other, figures, tables, references counts for cover sheet)>

|environment(cover_sheet;
	clear string page_header, new.page,
	line.length 7.5i, page.offset .3i, font.family helvetica,
	cs_heading_page1,
	cs_title,
	cs_authors,
	cs_numbers,
	cs_keywords,
	cs_mercury,
	cs_abstract_part1,
	cs_footer_page1,
	cs_abstract_part2,
	cs_heading_page2,
	cs_distribution_list,
	cs_spacing,
	cs_proprietary,
	cs_government,
	cs_att,
	cs_other_companies,
	cs_signatures,
	cs_earlier,
	cs_footer_page2;)
	|comment<summary: cover_sheet (produce the coversheet here)>

|environment(cs_heading_page1;
	spacing on, size 16, font bold, goto 0.2i,
	text "AT&T Bell Laboratories", horizontal.motion 2.35i,
	text "Document Cover Sheet", blank.lines 0.1i, indent.line 4.4i,
	text "for Technical Memorandum"; thick.line)

|environment(cs_title;
	new.line, remember position0, font bold,
	text "Title:", horizontal.motion 5.5i,
	text "Author's Date:", font,
	return position0,
	indent .5i, line.length 5.0i,
	macro title_string,
	line.length 7.5i, indent,
	return position0, blank.lines, indent.line 6i,
	string date_string, new.line; blank.lines -0.1i, thick.line)

|environment(cs_authors;
	tab.stops 1.5i 4.25i 5.5i 6.75i, font bold,
	if else gt(author_counter, 1),
		set string plural_string "s",
	if else,
		clear string plural_string,
	tab, text "Author", string plural_string,
	tab, text "Location", tab, text "Ext.",
	tab, text "Dept.",
	font, tab.stops 4.2i 4.5i 5.5i 6.7i,
|for i in 1 2 3 4 5 6 7 8 9 [
	if begin ge(author_counter, $i),
		new.line, string author_array($i), tab,
		string location_array($i), text " ", string room_array($i), tab,
		string extension_array($i), tab, string department_array($i),
	if end,
]
	new.line; blank.lines -0.1i, thick.line)

|environment(cs_numbers;
	tab.stops 0.9i 3.4i 5.65i, font bold,
	if else gt(document_counter, 1),
		set string plural_string "s",
	if else,
		clear string plural_string,
	tab, text "Document No", string plural_string, text ".",
	tab, text "Filing Case No", string plural_string, text ".",
	tab, text "Work Program No", string plural_string, text ".",
	font, tab.stops 0.8i 3.5i 5.75i,
|for i in 1 2 3 4 5 6 7 8 9 [
	if begin ge(document_counter, $i),
		new.line, tab, string document_array($i),
		tab, string file_case_array($i), tab, string work_program_array($i),
	if end,
]
	new.line; blank.lines -0.1i, thick.line)

|environment(cs_keywords;
	temporary.font bold "Keywords",
	blank.lines 0.1i, indent.line .2i, adjust off,
	macro keyword_string, new.line; blank.lines -0.1i, thick.line)

|environment(cs_mercury;
	tab.stops 3.4i 5.65i,
	temporary.font bold "MERCURY Announcement Bulletin Sections",
	blank.lines 0.1i, indent .9i, font.size 8,
|for i in 1 4 7 [
	if begin ge(mercury_counter, $i),
		set i_plus_one $i+1,
		set i_plus_two $i+2,
		string mercury_array($i), tab,
		string mercury_array(i_plus_one), tab,
		string mercury_array(i_plus_two),
	if end,
]
	new.line; blank.lines -0.1i, thick.line)

|environment(cs_abstract_part1;
	temporary.font bold "ABSTRACT", new.line,
	remember position0;)

|environment(cs_footer_page1;
	goto 8.375i,
	remember position1,
	thick.line,

	font.size 8, add total_page_counter text_page_counter other_page_counter,
	text "Pages of Text  ", size +2, number text_page_counter, size -2, text "  ",
	text "Other Pages  ", size +2, number other_page_counter, size -2, text "  ",
	text "Total  ", size +2, number total_page_counter, size -2, new.line,
	text "No. Figs.  ", size +2, number figure_counter, size -2, text "  ",
	text "No. Tables  ", size +2, number table_counter, size -2, text "  ",
	text "No. Refs.  ", size +2, number reference_counter, size -2, font.size,

	return position1, blank.lines 1i,
	if begin equal_string(proprietary_string, yes),
		indent .5i,
		temporary.font italics
		    "       AT&T BELL LABORATORIES \(em PROPRIETARY",
		new.line,
		text "Not for use or disclosure outside AT&T Bell Laboratories",
		new.line,
		text "   except by written approval of the Director of the",
		new.line,
		text "      originating organization (see G.E.I. 2.2).",
		new.line, indent,
	if end,

	return position1, blank.lines .75i, indent.line 5.25i,
	temporary.font bold "Mailing Label";)

|environment(cs_abstract_part2;
	return position0, indent .2i, line.length 7i,
	macro abstract_string; line.length 7.5i)

|environment(cs_heading_page2;
	new.page, spacing on, goto 0.2i,

	font bold,
	text "Initial Distribution Specifications", horizontal.motion 3.0i,
	string document_array(1), new.line; blank.lines -0.1i, thick.line)

|environment(cs_distribution_list;
	font bold, horizontal.motion 1.25i,
	text "Complete Copy", horizontal.motion 2.75i, text "Cover Sheet Only",
	font, new.line, remember position0, fill off,
	macro complete_memo_string, return position0, indent 4i,
	macro cover_sheet_string;)

|environment(cs_spacing;
	goto 1.50i,

	if unequal_string(proprietary_string, yes),
		blank.lines 1.0i,
	if unequal_string(government_string, yes),
		blank.lines 0.35i,
	if le(author_counter, 1),
		blank.lines 0.5i,
	if le(author_counter, 4),
		blank.lines 0.5i,
	if le(author_counter, 7),
		blank.lines 0.5i,
	if null_strings(earlier_document_number_string,
	    earlier_file_case_number_string, earlier_author_string,
	    earlier_date_string),
		blank.lines 0.65i;
	thick.line)

|environment(cs_proprietary;
	if begin equal_string(proprietary_string, yes),
		temporary.font bold "Proprietary Classification", blank.lines .05i,
		text "This document will be be classified",
		text "       AT&T BELL LABORATORIES \(em PROPRIETARY.",
		blank.lines 0.2i, indent.line 1.0i,
		temporary.font bold "Approval:  ", line 3.5i, indent.line 2.0i,
		string director_string, text ", Director",
		blank.lines -.1i, thick.line,
	if end;)

|environment(cs_government;
	if begin equal_string(government_string, yes),
		tab.stops 0.25i, box x, tab,
		temporary.font bold "Government Security Classified",
		blank.lines -0.05i, thick.line,
	if end;)

|environment(cs_att;
	temporary.font bold "AT&T-IS Distribution", new.line,
	text "To expedite the movement of documents to AT&T-IS, ",
	text "Director-level action is requested ",
	text "regarding items (1) and (2) below, when the document is first ",
	text "distributed. In those cases where approval is not provided on ",
	text "the cover sheet, approval will be sought when a request is ",
	text "received from AT&T-IS, with consequent delay in filling the request.",
	blank.lines 0.1i,
	text "Indicate whether the document:", blank.lines 0.1i, indent .18i,
	indent.line -.18i,
	temporary.font bold "(1) ",
	text "Contains network planning information, customer proprietary ",
	text "information, or nongeneric software for use in AT&T-IS ",
	text "products or services that AT&T-BL may not furnish to AT&T-IS.",
	if else begin equal_string(att_networking_string, yes),
		text "  ", box x, text " Yes", horizontal.motion .4i,
		box empty, text " No",
	if end,
	if else begin,
		text "  ", box empty, text " Yes", horizontal.motion .4i,
		box x, text " No",
	if end,
	blank.lines 0.1i,
	indent.line -.18i,
	temporary.font bold "(2) ",
	text "May be supplied on request to AT&T-IS R&D organizations.",
	if else begin equal_string(att_release_string, yes),
		text "  ", box x, text " Yes", horizontal.motion .4i,
		box empty, text " No",
	if end,
	if else begin,
		text "  ", box empty, text " Yes", horizontal.motion .4i,
		box x, text " No",
	if end,
	indent, blank.lines, indent.line 3.25i,
	temporary.font bold "Approval:  ", line 3.5i, indent.line 4.25i,
	string director_string, text ",  Director";

	blank.lines -0.1i, thick.line)

|environment(cs_other_companies;
	temporary.font bold "Other AT&T Company Distribution", new.line,
	text "May be supplied on request to other AT&T company requesters.",
	blank.lines, indent.line 3.25i,
	temporary.font bold "Approval:  ", line 3.5i, indent.line 4.25i,
	string director_string, text ",  Director";

	blank.lines -0.1i, thick.line)

|environment(cs_signatures;
	if else gt(author_counter, 1),
		set string plural_string "s",
	if else,
		clear string plural_string,
	font bold,
	text "Author Signature", string plural_string, font, new.line,
|for i in 0 3 6 [
	tab.stops 2.635i 5.25i,
	if gt(author_counter, $i),
		blank.lines 0.1i,
	if eq(author_counter, ($i+1)),
		line 2.25i,
	if begin eq(author_counter, ($i+2)),
		line 2.25i, tab, line 2.25i,
	if end,
	if begin gt(author_counter, ($i+2)),
		line 2.25i, tab, line 2.25i, tab, line 2.25i,
	if end,
	new.line,
	if begin gt(author_counter, $i),
		tab.stops 0.25i 2.875i 5.5i,
		set i_plus_one $i+1,
		set i_plus_two $i+2,
		set i_plus_three $i+3,
		tab, string author_array(i_plus_one),
		tab, string author_array(i_plus_two),
		tab, string author_array(i_plus_three),
	if end,
]
	;

	blank.lines -0.1i, thick.line)

|environment(cs_earlier;
	if begin not_null_strings(earlier_document_number_string,
	    earlier_file_case_number_string,
	    earlier_author_string, earlier_date_string),
		temporary.font bold
		    "Complete if this document supersedes or amends an earlier one:",
		new.line,
		remember position0, blank.lines 0.05i,
		text "Earlier Document Number ", line 2.6i,
		text " Author ", line 2.8i,
		return position0, tab.stops 2.0i 5.0i,
		tab, string earlier_document_number_string, tab,
		string earlier_author_string, new.line,
		remember position0, blank.lines 0.05i,
		text "Filing Case No. ", line 3.25i, text " Date ", line 2.9i,
		return position0, tab.stops 1.5i 5.0i,
		tab, string earlier_file_case_number_string, tab,
		string earlier_date_string,
		blank.lines -0.1i, thick.line,
	if end;)

|environment(cs_footer_page2;
	temporary.font bold "For Use by Recipient of Cover Sheet:", font.size -3,
	blank.lines 0.05i,
	remember position0,
	text "     To get a complete copy of this document:",
	blank.lines 0.1i, line.length 3.5i, indent .1i, indent.line -.1i,
	text "1 Be sure your correct location is given on the mailing ",
	text "label on the other side.",
	new.line, indent.line -.1i,
	text "2 Fold this sheet in half with this side out.",
	new.line, indent.line -.1i,
	text "3 Check the address of your local Internal Technical Document ",
	text "Service if listed; otherwise, use HO 4F-112. Use no envelope.",
	new.line, indent.line -.1i,
	text "4 Indicate whether microfiche or paper copy is desired.",
	return position0, line.length 7.5i, indent 4i,
	text "\ Internal Technical Document Service",
	blank.lines, tab.stops 1i,
	text "( ) HO 4F-112", tab, text "( ) ALC 1B-102", new.line,
	text "( ) IH 7K-101", tab, text "( )  MV 1D-40", new.line,
	text "( ) WH 3E-204", tab, text "( )  CB 1C-338", blank.lines,
	text "Please send a complete ", size +2, box, size -2,
	text "\ microfiche ", size +2, box, size -2,
	text "\ paper copy of this document to the address shown on the ",
	text "other side. Computing network users may order copies via the ",
	temporary.font italics "library ",
	text "system; for information, type ``man library'' after logon.",
	new.line;)

