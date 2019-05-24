# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
for i in $*
	do case $i in
		-ver) echo $0 version 2.0;exit;;
		-flags) echo $0 \[-ver\] \[-flags\];exit;;
		-*) echo unknown punctrules flag $i; exit;;
		*) echo punctrules takes no files;shift;continue;;
	esac
done
cat << !
      1.  Periods and commas _a_l_w_a_y_s go inside double quote marks.
          EXAMPLE: "I want to go to the fair," he said.

          The only allowable exception is when a single character
          is in quotes, e.g., use a tilde "~".

      2.  Semicolons and colons _a_l_w_a_y_s go outside of double quotes.
          EXAMPLE: He knew what was meant by "hardcopy"; he didn't
          know about "software."

      3.  Question marks and exclamation marks go inside or outside of
          double quote marks depending on the sentence sense.
          EXAMPLE:  "Where are you going?" he asked.
                     What is meant by the word "firmware"?

      4.  When a quote ends with a question mark that ends a clause
          and a comma would normally appear at the end of the clause,
          it is standard to leave the comma out. (The first example
          sentence under item 3 illustrates this.)

      5.  When using single quote marks instead of double quote marks,
          the same rules apply. (Single quotes are considered
          incorrect, except inside of a quotation enclosed in double
          quotes.)

      6.  When a sentence is enclosed in parentheses, the period goes
          inside the closing parenthesis.
          EXAMPLE: (This is a sentence.)

      7.  If the words inside parentheses do not constitute a sen-
          tence, but are at the end of the sentence, the period goes
          after the closing parenthesis.
          EXAMPLE: This is a sentence (but not this).

      8.  No commas, semicolons, or colons should appear before a left
          parenthesis.  If such punctuation is needed, it is placed
          after the phrase in parentheses.
          EXAMPLE: After eating salt (sodium chloride), he threw up.

      9.  Dashes never occur next to commas, semicolons, blank spaces,
          or parentheses.
          EXAMPLE: Before World War I--but not afterwards--Iceland was
          a part of Denmark.
!
