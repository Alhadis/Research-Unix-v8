|include(monk.h)

|environment(document_begin_and_end_of_text;
	font.family helvetica, font.size 18, fill off;)

|environment(title;
	new.page, blank.lines 1, center on, font bold;

	blank.lines 1)
	|comment<summary: title (title of the song)>

|environment(verse;
	blank.lines .5, here on;)
	|comment<summary: verse (verse of the song)>

|environment(refrain;
	blank.lines .5, indent 2, here on;

	blank.lines .5)
	|comment<summary: refrain (refrain of the song)>

|environment(smaller;
	font.size -2;)
	|comment<summary: smaller (print the song a little smaller than normal)>
