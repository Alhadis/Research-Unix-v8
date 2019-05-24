/* NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT */
/* Writer's Workbench version 2.3, April 7, 1981 */
#define NVAR=14
#define MAXLENG 79.0
#define READAB 0
#define NONF 1
#define PASS 8
#define NOM 9
#define AVW 10
#define EXPL 11
#define S_CPX 12
#include <stdio.h>

char *disclm ="BECAUSE YOUR TEXT IS SHORT \(< 2000 WORDS & < 100 SENTENCES\),\n\
THE FOLLOWING ANALYSIS MAY BE MISLEADING.\n\n";

char *note0 =
"NOTE: Your  document is being  compared   against  standards\n\
derived from 30 TMs, classified as  good by Department Heads\n\
in the Research Area.\n";
char *note2 =
"NOTE: Your text is being compared  against standards derived\n\
from 34 instructional texts produced by Department 45272.\n\n";
char *note1 =
"NOTE:    Your text is being compared to standards for Craft\n\
derived from the research of E. Coke.\n\n";
char *note3 =
"NOTE: Your text is being compared to standards given in file\n\"%s.\"\n";

char *kincaid = "\nRREEAADDAABBIILLIITTYY\n\
\n     The Kincaid readability formula predicts that your text\n\
can be read by someone with %2.0f or more years  of  schooling,\n";

char *more_str =
"\n   _M_o_r_e__I_n_f_o_r_m_a_t_i_o_n__o_n__S_e_n_t_e_n_c_e__S_t_r_u_c_t_u_r_e\n\
\n     Since passives,  nominalizations, as well as expletives\n\
all require  some form  of the verb \"to be,\"  an easy way to\n\
identify difficult paragraphs is to look for the overuse  of\n\
forms  of \"be.\"  The _f_i_n_d_b_e program makes this easy; it for-\n\
mats a text and underlines and capitalizes all forms  of \"to\n\
be.\"  To use it type:\n\n\
                      findbe filename\n\n\
You can also use the _s_t_y_l_e program  to print your sentences\n\
containing both a passive verb and a nominalization.  Type:\n\n\
                      style -n filename\n";

char *read0 =
"    Good TMs\n\
average close to 13th grade level, even  though the audience\n\
has more education than that.\n";
char *read2 =
" Good train-\n\
ing materials  average close  to the 10th grade level,  even\n\
though the audience has more education than that.\n";
char *read3 =
"  Your  good\n\
materials average close to grade %4.1f.\n";
char *read1 =
"  Since Coke\n\
found that 62%% of Bell System craft  have difficulty reading\n\
documents  with readability scores above 12.0,  the majority\n\
of the craft will probably not be able to read this.\n";

char *begmsg =
"\n     Writing teachers also stress  that  no  more  than   75\n\
percent   of  the  sentences in a text should begin with the\n\
subject  of  the  sentence;  these  start  with the  subject\n\
%2.0f%% of  the time.   Try starting more of your sentences with\n\
prepositions, adverbs, or  conjunctions.  This  change  will\n\
have  the added benefit of adding variety of sentence length\n\
and type.\n";

char *pas_nom ="\n   _P_a_s_s_i_v_e_s__a_n_d__N_o_m_i_n_a_l_i_z_a_t_i_o_n_s\n\
\n     You have appropriately limited your use of passives and\n\
nominalizations \(nouns made from verbs, e.g. \"description\"\).\n";

char *badpas =
"\n     This text contains a %s higher percentage of passive verbs\n\
(%4.1f%%) than is common in good documents of this type (%2.0f%%).\n";

char *badnom ="\n     This text  has  a  higher percentage of nominalizations\n\
(%4.1f%%) than is common in good documents of this type (%2.0f%%).\n";

char *badexp ="\n   _E_x_p_l_e_t_i_v_e_s\n\
\n     This text  contains a  higher percentage  of expletives\n\
(%4.1f%%) than is common in good documents of this type (%2.0f%%).\n";

FILE *fp;
main(argc,argv)
int argc;
char *argv[];
{
	char *rather="rather";
	char *very="very";
	char *too="too";
	char *much="much";
	char *null="";
	char *qual;
	char *note;
	char *readmsg;
	int h,h0,fflag,sflag,ret;
	char save[81];
	int i,aud,npara,p,j;
	int numabst;
	float kindex, avw,snonf,  simple,complex, compound,  compdx;
	float ppron,padj, var[NVAR],array[NVAR][4],sc[NVAR];
	float passive,explet;
	int nomin,x;
	int numsent, maxsent, qcount, icount,ml,lsum,mg,gsum;
	long numwds,numnonf;
	int maxindex,minindex,minsent;
	float aindex,cindex,fgrad,findex,avl,fnumnonf,centlsum,centgsum;
	int nsimple,ncomplex,ncompound,ncompdx,ntobe;
	int npassive,pron,pos,adj,art,beg,begadv,begprep,begverb,begscon;
	int begconj, begexp,noun;
	float ptobe,paux,pinfin,padv,pconjc,pprepc,pnoun,pnomin,tot;
	float pbegprep,pbegadv,pbegverb,pbegscon,pbegconj,pbegexp;
	int tobe,aux,infin,prepc,conjc,adv;
	char path[80];
	fflag=0;
	sflag=0;
	aud=0;
	note = note0;
	readmsg = read0;
	strcpy(path,LIB");
	if(argc >= 2){
		i=0;
		if(*(argv[1]+i) =='f') {
			fflag=1;
			i++;
		}
		if(*(argv[1]+i) =='s') {
			sflag=1;
			i++;
		}
		switch (*(argv[1]+i)){
		case 'm':
			aud=0;
			note = note0;
			readmsg = read0;
			break;
		case 't':
			aud=2;
			note = note2;
			readmsg = read2;
			strds(LIB/train.st","training");
			break;
		case 'c':
			aud=1;
			note = note1;
			readmsg = read1;
			strds(LIB/crft.st","Craft");
			break;
		case 'x':
			aud=3;
			note = note3;
			readmsg = read3;
			if((fp=fopen(argv[2],"r"))==NULL){
				fprintf(stderr,"Prose can't find your standards file \"%s,\" try specifying a more complete pathname.\n",argv[2]);
				exit(1);
			}
			break;
		}
	}

	if(aud==0)
		strds(LIB/tm.st","TM");
findbeg:
	if(fgets(save,81,stdin)==NULL){
		fprintf(stderr,"The file named after  the -f flag doesn't seem\
 to contain a style table.\nOr else style cannot produce a table for your\
 file.\n  Try running style alone on your file.\n");
				exit(1);
	}
	ret=sscanf(save,"%*s%f%*s%f%*s%f%*s%f (%f)",
		&kindex,&aindex,&cindex,&fgrad,&findex);
        if(ret !=5 ) goto findbeg;
	gets(path);
	scanf("%*s%*s%d%*s%*s%ld",&numsent,&numwds);
	scanf("%*s%*s%*s%f%*s%*s%*s%f",&avw,&avl);
	scanf("%*s%*s%d%*s%*s%d",&qcount,&icount);
	scanf("%*s%*s%*s%d%f%%%*s%*s%f",&numnonf,&fnumnonf,&snonf);
	scanf("%*s%*s (<%d) %f%% (%d) %*s%*s (>%d) %f%% (%d)",
		&ml,&centlsum,&lsum,&mg,&centgsum,&gsum);
	scanf("%*s%*s%d%*s%*s%*s%d;%*s%*s%d%*s%*s%*s%d\n",
		&maxsent,&maxindex,&minsent,&minindex);
	gets(path);
	scanf("%*s%f%% (%d) %*s %f%% (%d)",&simple,&nsimple,&complex,&ncomplex);
	scanf("%*s%f%% (%d) %*s%f%% (%d)\n",
		&compound,&ncompound,&compdx,&ncompdx);
	gets(path);
	gets(path);
	scanf("%*s%f%% (%d) %*s %f%% (%d)%*s%f%% (%d)",
		&ptobe,&tobe,&paux,&aux,&pinfin,&infin);
	scanf("%*s%*s %% %*s%*s%*s%f%% (%d)\n",&passive,&npassive);
	gets(path);
	scanf("%*s %f%% (%d)%*s%f%% (%d)%*s%f%% (%d)",
		&pprepc,&prepc,&pconjc,&conjc,&padv,&adv);
	scanf("%*s%f%% (%d)%*s %f%% (%d)%*s%f%% (%d)",
		&pnoun,&noun, &padj,&adj,&ppron,&pron);
	scanf("%*s%f %% (%d)\n",&pnomin,&nomin);
	gets(path);
	scanf("%*s%*s%*s (%d)%*s (%d)%*s (%d)%*s (%d)%*s (%d)%*s%f%%",
		&noun,&pron,&pos,&adj,&art,&tot);
	scanf("%*s%f%% (%d)%*s%f%% (%d) ",&pbegprep,&begprep,&pbegadv,&begadv);
	scanf("%*s%f%% (%d) ",&pbegverb,&begverb);
	scanf("%*s%f%% (%d)%*s%f%% (%d)",&pbegscon,&begscon,&pbegconj,&begconj);
	scanf("%*s%f%% (%d)",&pbegexp,&begexp);

	/*variables used by the prose program*/
	var[READAB]=kindex;
	var[NONF]=snonf;
	var[2]=centlsum;
	var[3]=centgsum;
	var[4]=simple;
	var[5]=complex;
	var[6]=compound;
	var[7]=compdx;
	var[PASS]=passive;
	var[NOM]=pnomin;
	var[AVW]=avw;
	var[EXPL]=pbegexp;
	var[S_CPX]=simple-complex;
	var[13]=compound+compdx;

	/*read criterion values for variables from appropriate file*/
	ret=0;
	for(i=0;i <NVAR; i++)
		for(j=0;j<4; j++){
			ret+=fscanf(fp,"%f",&array[i][j]);
		}

	/* make sure values were read correctly*/
        if(ret!=(NVAR*4)){
		fprintf(stderr,"The standards file is not formatted\
 properly.\nIt should have %d lines, each with 4 floating point numbers.\n",
		NVAR);
		exit(1);
	}

	/* Compare criterion values against STYLE produced scores*/

	for(i=0; i< NVAR; i++){
		sc[i]=0;
		for(j=0; j < 4;j++){
			if(var[i] > array[i][j]) sc[i]=j+1;
		}
	}

	/*  Read percent of abstract words  */


	if(numwds < 2000 && numsent <100)
		printf(disclm);
	if(sflag) goto shortans ;
	printf(note,argv[2]);
			/*READABILITY*/
	printf(kincaid,kindex);
	if(sc[READAB]==0){
		printf("which is a low score for  this type of document.");
		if (aud==0)
			printf("  If this is\nan instructional text\
 (in paragraph form), run prose -t  for\na more appropriate review.\n");
		else if(aud==2)putchar('\n');
	}
	else if(sc[READAB] < 3)
		printf("which is a  good score for documents like this.\n");
	else {		/* ==3 or 4*/
		if(sc[READAB] == 3)qual=rather;
		else qual=very;
		printf("which is %s high for this type of document.",qual);
		if(aud != 1)printf(readmsg,(array[READAB][1]+array[READAB][2])/2.0);
		if(sc[NONF] > 2)
			prtfile(LIB/rd2.t");
		if(aud == 1)printf(readmsg);

	}


	printf("\nVVAARRIIAATTIIOONN\n");
	if(sc[S_CPX]==4 || sc[S_CPX]==0 || tot >MAXLENG)
			prtfile(LIB/var.t");
	if(sc[S_CPX]==0||sc[S_CPX]==4)
		printf("\n     In this  text %2.0f%% of  the sentences are\
 simple, %2.0f%% are\ncomplex, giving a difference of %3.0f.  This\
 difference should\nrange  from %2.0f to %2.0f for good documents of this\
 type.\n",simple,complex, simple-complex,array[S_CPX][1],array[S_CPX][2]);
	if(sc[S_CPX]==4){
		if(sc[AVW] < 2)
			prtfile(LIB/type.t");
		else 
			prtfile(LIB/type2.t");
	}
	else if(sc[S_CPX]==0){
		if(sc[READAB] < 3)
			prtfile(LIB/var4.t");
		else
			prtfile(LIB/var6.t");
	}
	else printf("\n     You have an appropriate distribution of sentence\
 types.\n");
	if(maxsent >= 50){
		if(sc[S_CPX]==0 || sc[S_CPX]==4)
			printf("\n     Additionally,  the  longest sentence is\
 %d  words long.\n",maxsent);
		else printf("\n     The   longest  sentence,\
  however,  is %d  words  long.\n",maxsent);
		prtfile(LIB/var5.t");
	}
	if(sc[AVW] > 2){
		if(sc[S_CPX]==4 || sc[S_CPX]==0)
			printf("\n     Such  changes would  also  help  to\
 reduce the  average\n");
		else  printf("\n     You should,  however,  consider\
 shortening your average\n");
		printf("sentence length.  Your average is %2.0f words,\
  which  is ",var[AVW]);
		if(sc[AVW]==3)printf("high.\n");
		if(sc[AVW]==4) printf(" very\nhigh.  ");
		printf("A good average would be %2.0f to %2.0f words.\n",
			array[AVW][1],array[AVW][2]);
	}
	if(tot >MAXLENG){
		printf(begmsg,tot);
	}
	printf("\nSSEENNTTEENNCCEE SSTTRRUUCCTTUURREE\n");
	if(sc[PASS]<=2 && sc[NOM] <=2) {
		printf(pas_nom);
	}
	else {
		printf("\n   _P_a_s_s_i_v_e_s\n");
		if(sc[PASS] > 2){
			if(sc[PASS] == 3)qual=null;
			else qual=much;
			printf(badpas,qual,var[PASS],(array[PASS][1]+array[PASS][2])/2.0);
			prtfile(LIB/pass.t");
		}
		else printf("\n     You have limited your passives appropriately.\n");
		printf("\n   _N_o_m_i_n_a_l_i_z_a_t_i_o_n_s\n");
		if(sc[NOM] > 2){
			printf(badnom,var[NOM],(array[NOM][1]+array[NOM][2])/2.0);
			prtfile(LIB/nom.t");
		}
		else printf("\n     You  have  appropriately limited  your\
  nominalizations\n\(nouns made from verbs, e.g., \"description\"\).\n");
	}
	if(sc[EXPL]==4){
		printf(badexp,var[EXPL],(array[EXPL][1]+array[EXPL][2])/2.0);
		prtfile(LIB/exp.t");
	}
	if(sc[PASS]>2 && sc[NOM]>2){
		printf(more_str);
	}
	if(!fflag){
		if(aud==0)
			prtfile(LIB/popttm.t");
		else if(aud==2)
			prtfile(LIB/poptt.t");
	}
	prtfile(LIB/fur.t");
	exit(0);
shortans : 
	if(aud==0) printf("Compared to TMs.\n\n");
	else if(aud==1) printf("Evaluated for Craft.\n\n");
	else if(aud==2) printf("Compared to training material.\n\n");
	else if(aud==3)printf("Compared to file \"%s\"\n",argv[2]);
	printf("Reading grade level--%2.0f: ",kindex);
	if(sc[READAB]==1 || sc[READAB]==2) printf("Good\n");
	else if(sc[READAB]==0) printf("Low\n");
	else if(sc[READAB]==3) printf("High\n");
	else if(sc[READAB]==4)printf("Very high\n");


	printf("Variation--");
	if(sc[S_CPX] == 4)
		 printf("Too many short, simple sentences--subordinate.\n");
	else if(sc[S_CPX]==0) printf("Too many complex sentences.\n");
	else printf("Good sentence type distribution.\n");
	if(sc[AVW] > 2){
		if(sc[AVW]==3)
			qual = too;
		else qual = very;
		printf("             Sentences are %s long--avg\
 length=%2.1f words\n",qual,var[AVW]);
		printf("                                   --Good length\
 = %2.1f to %2.1f\n",array[AVW][1],array[AVW][2]);
	}
	if( maxsent >50)
		 printf("Longest sentence is %d words---perhaps it is a list\n"
			,maxsent);
	if(tot >MAXLENG)
		printf("             %2.0f%%= Too many sentences begin with\
 the subject.\n",tot);

	printf("Passives--%2.0f%%: ",passive);
	if(sc[PASS]<=2)printf("Good\n");
	else if(sc[PASS] ==3)printf("High\n");
	else if(sc[PASS]==4)printf("Very high\n");

	printf("Nominalizations--%2.0f%%: ",pnomin);
	if(sc[NOM]<=2)printf("Good\n");
	else if(sc[NOM]==3)printf("High\n");
	else if(sc[NOM]==4)printf("Very high\n");

	if(sc[EXPL]==4)printf("Expletive--%2.0f%%: Very high\n",pbegexp);
	if (!fflag) printf("Don't forget the styl.tmp file.\n");
	putchar('\n');
}
prtfile(s)
char *s;
{
	FILE *ff;
	char ch[512];
	int num;
	if((ff=fopen(s,"r"))==NULL){
		fprintf(stderr,"Prose can't find the file %s\n",s);
		exit(1);
	}
	while((num=fread(ch,sizeof(ch[0]),512,ff)))
		fwrite(ch,sizeof(ch[0]),num,stdout);
	fclose(ff);
}
strds(s1,s2)
char *s1, *s2;
{
	if((fp=fopen(s1,"r"))==NULL){
		fprintf(stderr,"Prose can't find the %s standards file %s\n",
			s2,s1);
		exit(1);
	}
}
