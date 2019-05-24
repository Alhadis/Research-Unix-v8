|associate(style;;;

	[one_column;
		two.columns off;]
	[two_column;
		two.columns on;]
	[page width $;;
		set page_width $, line.length]
	[page length $;;
		set page_length $, page.length]
	[font size $;;
		set font_size $, font.size])
	|comment<summary: style one_column (full page output)>
	|comment<summary: style two_column (two columns per page)>
	|comment<summary: style page width (width of the physical page)>
	|comment<summary: style page length (length of the physical page)>
	|comment<summary: style font size (default font size)>
