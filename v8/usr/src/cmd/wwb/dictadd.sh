# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
file=
sexflag=
fflag=
for i in $*
	do case $i in
		-ver) echo $0 version 2.4: 2.1;exit;;
		-flags) echo $0 \[-flags\] \[-ver\];exit;;
		-*) echo unknown dictadd flag $i;exit;;
                *) echo dictadd takes no files;shift;continue;;
	esac
done
if [ -f $HOME/lib ]
then echo "Dictadd can't make a directory \$HOME/lib for you
because you already have a file named \$HOME/lib.
Change the name of the file to something else
and run the program again."
	exit 1
fi
if test ! -d $HOME/lib
then echo dictadd is making a directory: \$HOME/lib for you.
	mkdir $HOME/lib
fi
echo 'Do you want to add words to $HOME/lib/ddict for use with 
wwb and proofr? (y or n)'
read ans1
if test \( $ans1 = 'y' -o $ans1 = 'Y' -o $ans1 = 'yes' -o $ans1 = 'Yes' \)
then
	 if 	test \( -r $HOME/lib/ddict -a  ! -w $HOME/lib/ddict \)
	then echo Dictadd can\'t write on the file \$HOME/lib/ddict\; check your permissions.
		exit 1
	fi
else
	echo 'Do you want to add words to $HOME/lib/sexdict for use with 
sexist? (y or n)'
	read ans2
	if test \( $ans2 = 'y' -o $ans2 = 'Y' -o $ans2 = 'yes' -o $ans2 = 'Yes' \)
	then
		sexflag=S;
		if test	\(  -r $HOME/lib/sexdict -a  ! -w $HOME/lib/sexdict \)
		then echo Dictadd can\'t write on the file \$HOME/lib/sexdict\; check your permissions.
			exit 1
		fi
	else echo "Type the name of the file you want the words added to:\nDo not use symbolic names such as \$HOME."
		fflag=F;
		read file
		if test \( -r $file -a ! -w $file \)
		then echo Dictadd can\'t write on the file $file\; check your permissions.
			exit 1
		fi
	fi
fi
echo Do you want instructions?  \(y or n\)
read ans
if test \( $ans = 'y' -o $ans = 'Y' -o $ans = 'yes' -o $ans = 'Yes' \)
then echo "


_O_v_e_r_v_i_e_w:   Dictadd  is  a  program  that  works  with   the
            wwb, proofr, and sexist programs. These programs
            find all sentences with wordy or sexist diction,
            but  you  can  use dictadd  to suppress words or
            phrases,  or to add new  ones to your dictionary
            files  \$HOME/lib/ddict  and   \$HOME/lib/sexdict,
            or  any other  file you prefer.   Procedures are
            given below.


_P_r_o_c_e_d_u_r_e_s__f_o_r__S_u_p_p_r_e_s_s_i_n_g__P_h_r_a_s_e_s      _P_r_o_c_e_d_u_r_e_s__f_o_r__S_t_o_r_i_n_g__P_h_r_a_s_e_s


_U_s_e wwb, proofr, sexist, diction,        _W_a_i_t for > sign.
or dictplus to get a list of wordy             |
sentences.      |                              |
                |                              |
                |                              |
_F_i_n_d the bracketed information          _T_y_p_e phrase.
in the list.    |                              |
                |                              |
_D_e_t_e_r_m_i_n_e _t_h_e _i_n_f_o_r_m_a_t_i_o_n _p_a_t_t_e_r_n       _T_y_p_e other phrases until
within the brackets:                    all are stored.
                                               |
  o+ blank spaces before and after it;          |
                                               |
  o+ a blank space before it;                   |
                                               |
  o+ no blank spaces at all.                    |
                |                              |
                |                              |
_W_a_i_t for > sign.                               |
                |                              |
                |                              |
CAUTION:  Don't type brackets.                 |
          Make sure to type the                |
          spaces, if there are any.            |
                                               |
_T_y_p_e:    ~Phrase                               |
       carriage return.                        |
                |                              |
                |                              |
_T_y_p_e _o_t_h_e_r _p_h_r_a_s_e_s until all                   |
are suppressed.                                |
                |                              |
                |_______________________________|_

                                 |
                                 |
                         _T_y_p_e _q (for quit).

      Comment:  Wwb, proofr, and sexist automatically
                search your dictionaries, but you must 
                tell diction and dictplus to do so, as in:

                diction -f \$HOME/lib/ddict filename"

fi
if test $sexflag
then	$L/dictadd $HOME/lib/sexdict
elif test $fflag 
then	$L/dictadd $file
else	$L/dictadd $HOME/lib/ddict
fi
