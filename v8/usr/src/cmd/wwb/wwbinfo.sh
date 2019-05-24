# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
for i in $*
	do case $i in
		-ver) echo $0 version 2.0;exit;;
		-flags) echo $0 \[-flags\] \[-ver\];exit;;
		-*) echo unknown wwbinfo flag $i;exit;;
		*) echo wwbinfo takes no files;shift;continue;;
	esac
done
cat << !
		       COMMAND-FUNCTION	TABLE
______________________________________________________________________
			     _C_o_m_m_a_n_d_s
acro file..................finds acronyms
chunk file.................segments at phrase boundaries
match stylefile1 2  N......collates statistics from different texts
org file...................shows text structure
rewrite file...............first draft of document with probs. highlighted
sexist file................finds sexist phrases and suggests changes
spelltell pattern..........prints commonly misspelled words containing pattern
style file file1...........summarizes stylistic features
syl -n file................prints words	of n syllables or longer
topic file.................summarizes content
   parts file..............assigns grammatical parts of	speech
wwb file...................runs	proofreading and sytlistic analysis
   proofr file.............proofreading	comments
      dictplus file........finds awkward phrases, and suggests changes
	 diction file......finds awkward phrases
	 suggest phrase....suggests substitutions for awkward phrases
      double file..........detects repeated typings of words
      punct file...........corrects punctuation
      spellwwb file........checks spelling, using spelldict
      gram file........finds split infinitives and incorrect indefinite articles
	 parts file........assigns grammatical parts of	speech
   prose file..............extended editorial comments
      style file...........summarizes stylistic	features
	 parts file........assigns grammatical parts of	speech
wwbmail....................sends mail to WWB Development group

			   _E_x_p_l_a_n_a_t_i_o_n_s
punctrules.................explains punctuation	rules
splitrules.................explains split infinitives
worduse word...............a glossary of words often confused in writing
wwbhelp word...............gets information about word (e.g. spell)
wwbinfo....................prints a copy of this table
wwbstand...................prints standards used by prose to evaluate documents

		      _E_n_v_i_r_o_n_m_e_n_t_a_l _T_a_i_l_o_r_i_n_g
dictadd....................adds	phrases	to ddict dictionary
spelladd...................adds	words to spelldict dictionary
mkstand....................builds standards for prose from user documents

		      _U_s_e_r _S_p_e_c_i_f_i_e_d _D_i_c_t_i_o_n_a_r_i_e_s
ddict......................personal list of awkward phrases
spelldict..................personal list of spellings
______________________________________________________________________
    Note.  Indented commands are automatically run by the less 
indented commands that immediately precede them.
!
