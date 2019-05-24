|include(monk.h)
|include(macro.h)

|environment(document_begin_and_end_of_text;
	;
)

|environment(ret_address;
	verbatim on, indent 30, fill off, here on;
	blank.lines)
	|comment<summary: ret_address (return address)>

|environment(address;
	verbatim on, fill off, here on, blank.lines;
	blank.lines)
	|comment<summary: address (who to address the letter to)>

|environment(greeting;
	blank.lines 2;
	blank.lines)
	|comment<summary: greeting (greeting)>

|environment(body;
	blank.lines, fill on;
	blank.lines)
	|comment<summary: body (body of letter)>

|environment(closing;
	blank.lines 2, indent 30;
	blank.lines)
	|comment<summary: closing (salutation)>

|environment(signature;
	verbatim on, blank.lines 3, indent 30;)
	|comment<summary: signature (signature)>
