|environment(bullet_list;
	blank.lines 1, list.tags bullet;

	indent, blank.lines 1)
	|comment<summary: bullet_list (bulletted list using \(bu \- \s-1\(bu\s+1 \s-1\-\s+1 \s-2\(bu\s+2 \s-2\-\s+2 \s-3\(bu\s+3)>

|environment(dash_list;
	blank.lines 1, list.tags dash;

	indent, blank.lines 1)
	|comment<summary: dash_list (dasheded list using \- \(bu \s-1\-\s+1 \s-1\(bu\s+1 \s-2\-\s+2 \s-2\(bu\s+2 \s-3\-\s+3)>

|environment(number_list;
	clear level_1 level_2 level_3 level_4 level_5 level_6 level_7,
	number format level_1 level_2 level_3 level_4 level_5 level_6 level_7 1,
	blank.lines 1, list.tags number;

	indent, blank.lines 1)
	|comment<summary: number_list (numbered list using 1 1.1 1.1.1 1.1.1.1 1.1.1.1.1 1.1.1.1.1.1 1.1.1.1.1.1.1)>

|environment(outline_list;
	clear level_1 level_2 level_3 level_4 level_5 level_6 level_7,
	number format level_1 I,
	number format level_2 A,
	number format level_3 level_5  1,
	number format level_4 level_6  a,
	number format level_7 i,
	blank.lines 1, list.tags outline;

	indent, blank.lines 1)
	|comment<summary: outline_list (outline list using I A 1 a (1) (a) i))>

|environment(level1;
	if begin equal_string(list_type, number),
		clear level_2 level_3 level_4 level_5 level_6 level_7,
		incr level_1,
		store string.from.number remember_string level_1,
	if end,
	if begin equal_string(list_type, bullet),
		set string remember_string \(bu,
	if end,
	if begin equal_string(list_type, dash),
		set string remember_string \-,
	if end,
	if begin equal_string(list_type, outline),
		clear level_2 level_3 level_4 level_5 level_6 level_7,
		incr level_1,
		store string.from.number remember_string level_1,
	if end;
	indent 0.2i, indent.line -0.2i, string remember_string,
	horizontal.motion "|0.2i")
	|comment<summary: level1 (a list item at level 1; IT)>

|environment(level2;
	if begin equal_string(list_type, number),
		clear level_3 level_4 level_5 level_6 level_7,
		incr level_2,
		store string.from.number remember_string level_1,
		add string remember_string ".",
		add string.from.number remember_string level_2,
	if end,
	if begin equal_string(list_type, bullet),
		set string remember_string \-,
	if end,
	if begin equal_string(list_type, dash),
		set string remember_string \(bu,
	if end,
	if begin equal_string(list_type, outline),
		clear level_3 level_4 level_5 level_6 level_7,
		incr level_2,
		store string.from.number remember_string level_2,
	if end;
	indent 0.4i, indent.line -0.2i, string remember_string,
	horizontal.motion "|0.2i")
	|comment<summary: level2 (a list item at level 2; IT)>

|environment(level3;
	if begin equal_string(list_type, number),
		clear level_4 level_5 level_6 level_7,
		incr level_3,
		store string.from.number remember_string level_1,
		add string remember_string ".",
		add string.from.number remember_string level_2,
		add string remember_string ".",
		add string.from.number remember_string level_3,
	if end,
	if begin equal_string(list_type, bullet),
		set string remember_string \s-1\(bu\s+1,
	if end,
	if begin equal_string(list_type, dash),
		set string remember_string \s-1\-\s+1,
	if end,
	if begin equal_string(list_type, outline),
		clear level_4 level_5 level_6 level_7,
		incr level_3,
		store string.from.number remember_string level_3,
	if end;
	indent 0.6i, indent.line -0.2i, string remember_string,
	horizontal.motion "|0.2i")
	|comment<summary: level3 (a list item at level 3; IT)>

|environment(level4;
	if begin equal_string(list_type, number),
		clear level_5 level_6 level_7,
		incr level_4,
		store string.from.number remember_string level_1,
		add string remember_string ".",
		add string.from.number remember_string level_2,
		add string remember_string ".",
		add string.from.number remember_string level_3,
		add string remember_string ".",
		add string.from.number remember_string level_4,
	if end,
	if begin equal_string(list_type, bullet),
		set string remember_string \s-1\-\s+1,
	if end,
	if begin equal_string(list_type, dash),
		set string remember_string \s-1\(bu\s+1,
	if end,
	if begin equal_string(list_type, outline),
		clear level_5 level_6 level_7,
		incr level_4,
		store string.from.number remember_string level_4,
	if end;
	indent 0.8i, indent.line -0.2i, string remember_string,
	horizontal.motion "|0.2i")
	|comment<summary: level4 (a list item at level 4; IT)>

|environment(level5;
	if begin equal_string(list_type, number),
		clear level_6 level_7,
		incr level_5,
		store string.from.number remember_string level_1,
		add string remember_string ".",
		add string.from.number remember_string level_2,
		add string remember_string ".",
		add string.from.number remember_string level_3,
		add string remember_string ".",
		add string.from.number remember_string level_4,
		add string remember_string ".",
		add string.from.number remember_string level_5,
	if end,
	if begin equal_string(list_type, bullet),
		set string remember_string \s-2\(bu\s+2,
	if end,
	if begin equal_string(list_type, dash),
		set string remember_string \s-2\-\s+2,
	if end,
	if begin equal_string(list_type, outline),
		clear level_6 level_7,
		incr level_5,
		set string remember_string "(",
		add string.from.number remember_string level_5,
		add string remember_string ")",
	if end;
	indent 1.0i, indent.line -0.2i, string remember_string,
	horizontal.motion "|0.2i")
	|comment<summary: level5 (a list item at level 5; IT)>

|environment(level6;
	if begin equal_string(list_type, number),
		clear level_7,
		incr level_6,
		store string.from.number remember_string level_1,
		add string remember_string ".",
		add string.from.number remember_string level_2,
		add string remember_string ".",
		add string.from.number remember_string level_3,
		add string remember_string ".",
		add string.from.number remember_string level_4,
		add string remember_string ".",
		add string.from.number remember_string level_5,
		add string remember_string ".",
		add string.from.number remember_string level_6,
	if end,
	if begin equal_string(list_type, bullet),
		set string remember_string \s-2\-\s+2,
	if end,
	if begin equal_string(list_type, dash),
		set string remember_string \s-2\(bu\s+2,
	if end,
	if begin equal_string(list_type, outline),
		clear level_7,
		incr level_6,
		set string remember_string "(",
		add string.from.number remember_string level_6,
		add string remember_string ")",
	if end;
	indent 1.2i, indent.line -0.2i, string remember_string,
	horizontal.motion "|0.2i")
	|comment<summary: level6 (a list item at level 6; IT)>

|environment(level7;
	if begin equal_string(list_type, number),
		incr level_7,
		store string.from.number remember_string level_1,
		add string remember_string ".",
		add string.from.number remember_string level_2,
		add string remember_string ".",
		add string.from.number remember_string level_3,
		add string remember_string ".",
		add string.from.number remember_string level_4,
		add string remember_string ".",
		add string.from.number remember_string level_5,
		add string remember_string ".",
		add string.from.number remember_string level_6,
		add string remember_string ".",
		add string.from.number remember_string level_7,
	if end,
	if begin equal_string(list_type, bullet),
		set string remember_string \s-3\(bu\s+3,
	if end,
	if begin equal_string(list_type, dash),
		set string remember_string \s-3\-\s+3,
	if end,
	if begin equal_string(list_type, outline),
		incr level_7,
		store string.from.number remember_string level_7,
		add string remember_string ")",
	if end;
	indent 1.4i, indent.line -0.2i, string remember_string,
	horizontal.motion "|0.2i")
	|comment<summary: level7 (a list item at level 7; IT)>
