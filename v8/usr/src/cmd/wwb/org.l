%{
/* NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT */
/* Writer's Workbench version 2.0, January 1981 */
		/* flag=0 means processing is in middle of sentence, */
/*		   flag=1 means have just finished a sentence, */
/*		   flag=2 means should print current sentence at its end */
#define MAXCHARS 1000
#define STORE if (ptr+yyleng >= str+MAXCHARS) ptr=str;  for(i=0;i<yyleng; ++i) *ptr++=yytext[i]
 char *ptr,str[MAXCHARS];
%}
%START IN OUT 
B	[\t \n]
BP	[\t \n()"]
U	[A-Z]
A	[a-zA-Z]
N	[0-9]
%%
 int i,flag,begin,in, eqn;
 in=begin=eqn=0;
 flag=1;
 ptr= str;
 BEGIN OUT;

^"."DS.*\n	|
^"."AL.*\n	|
^"."BL.*\n	|
^"."VL.*\n	|
^"."RL.*\n	|
^"."ML.*\n	|
^"."DL.*\n	|
^"."LB.*\n	|
^"."FS.*\n	|
^"."EQ.*\n	|
^"."TS.*\n	{in++; BEGIN IN;}

^"."TE.*\n	|
^"."FE.*\n	|
^"."LE.*\n	|
^"."EN.*\n	|
^"."DE.*\n	{in--; if(in <=0) BEGIN OUT;}

^"."EQ{B}+delim{B}+\$\${B}+"."EN{B}+	eqn=1;
\$[^$\n]*\$	{if (eqn==1);
		 else REJECT;
		}

^"."TL.*\n.*\n	|
^"."AS.*\n	|
^"."AE.*\n	|
^"."MT.*\n      |
^"."TM.*\n      |
^"."AU.*\n      ECHO;

^"."ce.*\n	;

<OUT>^"."P.*\n	|
<OUT>^"."H.*\n	{if(ptr> str){
			if (*ptr!='\0') {
					*ptr='\0';
			}
			printf("%s",str);
		}
		flag=2; /* flag==2 means print next sentence */
		ptr= str;
		ECHO;
		}

<OUT>^"."I{B}+\".*\"	{for(i=4; i<yyleng-1;i++) *ptr++=yytext[i];}
<OUT>^"."I{B}+/[^"].*\n		;

<OUT>^"."[^I].*\n	{if(begin==0) ECHO;
		else ;
		}

<OUT>(et"."{B}+al|a"."d|A"."M|a"."m|b"."c|Ch|{B}ch|{B}ckts|dB|Dept|dept|Depts|depts)"."	|
<OUT>(Dr|Drs|e"."g|Eq|eq|et"."al|etc|Fig|fig|Figs|figs)"."	|
<OUT>({B}ft|i"."e|{B}in|Inc|Jr|jr|lb|lbs|{B}mi|Mr|Mrs|Ms|No|{B}no|Nos|{B}nos)"."	|
<OUT>(P"."{B}*M|p"."m|Ph"."D|Ph"."d|PhD|Ref|ref|Refs|refs|sec|St|vs|v|yr)"."	|
<OUT>"."{N}	|/* this is the main place text is stored to be output later */
<OUT>{B}+{A}"."{B}+	|
<OUT>"\n"	|
<OUT>.	{if(flag==1) {ptr= str;
		flag=0;
		}
	STORE;
	begin=1;
	}

<OUT>\:/.*\n"."TS	|/* end a list,table, display after the preceding "*"*/
<OUT>\:/.*\n"."DS	|
<OUT>\:/.*\n".".L      {BEGIN IN;
			if(flag==2){
				if(ptr > str){*ptr++=':';
					*ptr++='.';
					*ptr++='\0';
					printf("%s\n",str);
				}
				flag=1;
				ptr= str;
			}
			else {flag=1;
				*ptr++=':';
				*ptr++='.';
			}
}
<OUT>[.?][ \t]*\n/"."[^PH]	|  /* if sentence end & flag==2, print it */
<OUT>[.?]{BP}+/{U}	{if(flag==2){
				if(ptr > str){*ptr++=yytext[0];
					*ptr++='\0';
					printf("%s\n",str);
				}
				flag=1;
				ptr= str;
			}
			else {flag=1;
			}
			}

<IN>.	;
<IN>\n	;
%%
yywrap(){
	if(ptr> str) {*ptr++='\0';
			printf("%s",str);
	}
	return(1);
}
main(argc,argv)
char *argv[];
{
	if(argc <= 1)
		yylex();
	else {
		while(argc > 1){
			argc--; argv++;
			if(freopen(argv[0],"r",stdin) == NULL){
				fprintf(stderr,"org cannot open %s\n",argv[0]);
				exit(1);
			}
			yylex();
		}
	}
}
