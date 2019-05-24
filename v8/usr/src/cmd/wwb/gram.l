%{
/* NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT */
/* Writer's Workbench version 2.0, January 1981 */
#include <ctype.h>
int catch = 1;
int ct = 0;
int ret=0;
int linect = 0;
char name[50];
int i;
char *p;
FILE *fopen(), *cat;
%}
%%
verb"\t"to\n   search();
art"\t"a\n	cons();
art"\t"an\n	vowel();
^"."[0-9]+	linect = atoi(&yytext[1]);
^".F ".*	{
		p = name;
		for(i=3;i<yyleng;i++)
			*p++ = yytext[i];
		}
.       |
\n      ;
%%
cons(){
	int c;
	int f;
	char *w1;
	char w[100];
	w1 = w;
	while((c=input()) != '\t')*w1++ = c;
	if(strncmp("conj",w,4) == 0)return(ret);
	if(strncmp("prep",w,4) == 0)return(ret);
	if(strncmp("be",w,2) == 0)return(ret);
	if(strncmp("verb",w,4) == 0)return(ret);
	w1 = w;
	while((f=input()) == ' ');
	if(!isalpha(f))return(ret);
	if(isupper(f))return(ret);
	if(f == 'a'||f == 'e'||f =='i'||f =='o'||f == 'u'|| f == 'h'){
		*w1++ = f;
		while((c=input()) != '\n'){
			if(!isalpha(c))break;
			*w1++ = c;
		}
		if(w1 < &w[2])return(ret);
		*w1 = '\0';
		if(f == 'h'){
			if(hexc(w)== 0)return(ret);
		} else if(f == 'u'){
			if(uexc(w) == 1)return(ret);
		} else if(f == 'e'){
			if(w[1] == 'u')return(ret);
		} else if(f == 'o'){
			if(strncmp("one",w,3) == 0)return(ret);
			if(strncmp("once",w,4) == 0)return(ret);
		}
		ret = 1;
		if(++ct == 1)printf("Possible grammatical errors:\n");
		printf("\t\"a %s\" should be \"an %s\"",w,w);
		printf(" after line %d file %s\n",linect,name);
	}
	return(ret);
}
vowel(){
	int c,f;
	char *w1;
	char w[100];
	w1 = w;
	while((c=input()) != '\t')*w1++ = c;
	if(strncmp("conj",w,4) == 0)return(ret);
	if(strncmp("prep",w,4) == 0)return(ret);
	if(strncmp("be",w,4) == 0)return(ret);
	if(strncmp("verb",w,4) == 0)return(ret);
	w1 = w;
	while((f=input()) == ' ');
	if(!isalpha(f))return(ret);
	if(isupper(f))return(ret);
	if(f != 'a' && f != 'i' ){
		*w1++ = f;
		while((c=input()) != '\n'){
			if(!isalpha(c))break;
			*w1++ = c;
		}
		if(w1 < &w[2])return(ret);
		*w1 = '\0';
		if(f == 'h'){
			if(hexc(w))return(ret);
		}
		else if(f == 'u'){
			if(uexc(w) == 0)return(ret);
		} else if(f == 'e'){
			if(w[1] != 'u')return(ret);
		} else if(f == 'o' ){
			if( strncmp("one",w,3) != 0)return(ret);
			if(strncmp("once",w,4) != 0)return(ret);
		}
		ret = 1;
		if(++ct == 1)printf("Possible grammatical errors:\n");
		printf("\t\"an %s\" should be \"a %s\"",w,w);
		printf(" after line %d file %s\n",linect,name);
	}
	return(ret);
}
search()
{
	char *sv;
	int c;
	char *w1, *p1, *p2;
	char words[100];
	char part1[10],part2[10];
	int f;
#ifdef SPCATCH
	if(catch != 2){
		if((cat=fopen(SPCATCH,"a"))==NULL) catch=0;
		else catch = 2;
	}
#else
	catch=0;
#endif
	w1 = words;
	f = 0;
more:
	for(p1 = part1; (c= input())!= '\t';)
		*p1++ = c;
	while((c=input())!='\n')
		*w1++ = c;
	*w1++ = ' ';
	if(strncmp(part1,"adv",3) != 0)
		return(ret);

	while(1){
		for(p2=part2;(c=input())!='\t';)
			*p2++ = c;
		if(strncmp(part2,"verb",4) != 0)
			break;

		sv = w1;
		while((c=input())!='\n')
			*w1++ = c;
		if(strncmp(sv,"to",2) == 0 && w1 == sv+2)break;
		*w1++ = ' ';
		f = 1;
	}
	if(f == 0)return(ret);
	*w1 = '\0';
	if(++ct==1)printf("Possible grammatical errors:\n\n");
	printf("\tsplit infinitive: \"to %s\"",words);
	printf(" after line %d file %s\n",linect,name);
	if(catch == 2){
		fprintf(cat,"%s\n",words);
	}
	ret=1;
	return(ret);
}
hexc(w)
char *w;
{
	if(strncmp(w,"heir",4) == 0)return(1);
	if(strncmp(w,"herb",4) == 0)return(1);
	if(strncmp(w,"hour",4) == 0)return(1);
	if(strncmp(w,"honest",6) == 0)return(1);
	if(strncmp(w,"hombre",6) == 0)return(1);
	if(strncmp(w,"honor",4) == 0)return(1);
	return(0);
}
uexc(w)
char *w;
{
	int c1, c2;
	if(strncmp(w,"uni",3) == 0){
		if(*(w+3) != 'm' && *(w+3) != 'n')return(1);
		else return(0);
	}
	c1 = *(w+1);
	if(c1 == 'b' || c1 == 'k')return(1);
	c2 = *(w+2);
	if(c1 == 'r' || c1 == 's'){
		if(c2== 'a' || c2 == 'e' || c2 == 'i' || c2 == 'o' || c2 == 'u')
			return(1);
		else return(0);
	}
	if(strncmp("unanim",w,6) == 0)return(1);
	if(strncmp("unary",w,5) == 0)return(1);
	if(c1 == 't' && c2 != 't')return(1);
	return(0);
}
