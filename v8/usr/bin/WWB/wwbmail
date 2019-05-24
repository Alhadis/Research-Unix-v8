# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
trap 'rm -f /tmp/$$*; trap 0; exit' 0 1 2 3 15
uuname > /tmp/$$uu &
pflag=
for i in $*
	do case $i in
		-p) pflag=p; continue;;
		-ver) echo $0 version 2.1;exit;;
		-flags) echo $0 \[-p\] \[-flags\] \[-ver\];exit;;
		-*) echo unknown wwbmail flag $i;exit;;
		*) echo wwbmail takes no flags;shift;continue;;
	esac
done
echo 'Do you want instructions? (y or n)'
read ans
if test \( $ans = 'y' -o $ans = 'Y' -o $ans = 'Y' -o $ans = 'yes' -o $ans = 'Yes' \)
then echo '
Procedure for Sending Mail to WWB

1. Note:  At the end of these instructions, your terminal will return a "0".
	  This means you are in the editor.

   Begin your message with an "a" on a line by itself.
	 Comment 1:  This puts you in append mode.
	 Comment 2:  If you decide not to send your message after typing "a",
		     hit "break" or "delete" and type "q" to quit the editor.
		     (You may have to hit "q" twice)

2. Type your message.

3. Type a dot (.) on a line by itself, immediately after the message.
	Comment:  You may use any of the text editing capabilities of the
		  editor, such as substituting or deleting lines.

4. End your edited message with the command sequence "w" and "q",
   each on a separate line.
	Terminal Response: You will see a character count after you type "w".

Use of the option -p (wwbmail -p) will enable you to bypass attempts to send
electronic mail and process only a pre-addressed, paper copy of your message.'
fi
> /tmp/$$wwbmail
ed  /tmp/$$wwbmail
if test ! -s /tmp/$$wwbmail
	then echo 'no message sent'
		exit 1
fi
if test $pflag
then cat > wwbmailtmp << !



                        ***********************
                        *  Mary L. Fox        *
                        *  1M-214             *
                        *  Bell Laboratories  *
                        *  6 Corporate Place  *
                        *  Piscataway, NJ     *
                        ***********************



!
cat /tmp/$$wwbmail >> wwbmailtmp
echo 'A copy of your message is stored in the file "wwbmailtmp".
	Cat that file, circle the address shown at the top of
	the paper copy, and mail it.  After you have catted
	"wwbmailtmp", remove it from your directory.'
	exit 1
fi
wait
if (test "`grep mhuxh /tmp/$$uu`")
then mail mhuxh!mlf < /tmp/$$wwbmail
	echo 'Your message has been sent to mhuxh!mlf.'
elif (test "`grep mhuxm /tmp/$$uu`")
then mail mhuxm!mhuxh!mlf < /tmp/$$wwbmail
	echo 'Your message has been sent to mhuxm!mhuxh!mlf'
elif (test "`grep mhtsa /tmp/$$uu`")
then mail mhtsa!mhuxh!mlf < /tmp/$$wwbmail
	echo 'Your message has been sent to mhtsa!mhuxh!mlf'
else cat > wwbmailtmp << !



                        ***********************
                        *  Mary L. Fox        *
                        *  1M-214             *
                        *  Bell Laboratories  *
                        *  6 Corporate Place  *
                        *  Piscataway, NJ     *
                        ***********************



!
cat /tmp/$$wwbmail >> wwbmailtmp
echo 'I do not recognize an electronic mail link between our systems.
	A copy of your message is stored in the file "wwbmailtmp".
	Cat that file, circle the address shown at the top, and put
	it in the regular mail.  After you have catted "wwbmailtmp",
	remove the file from your directory.'
fi
