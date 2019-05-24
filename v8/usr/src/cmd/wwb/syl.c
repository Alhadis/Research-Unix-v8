/* NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT */
/* Writer's Workbench version 2.0, January 1981 */
#include <stdio.h>
#include <ctype.h>
#define MAXCHAR 50
#define isvowel(c)	(c=='a'||c=='e'||c=='i'||c=='o'||c=='u')
char a[MAXCHAR],b[MAXCHAR];

main(argc,argv)
int argc;
char *argv[];
{
	register int nsyl,var;
	if(argc == 3){
		if(*argv[1] != '-'){
			fprintf(stderr,"Usage: syl [-integer] filename\n");
			exit(1);
		}
		var = *(argv[1]+1);
		if(!isdigit(var)){
			fprintf(stderr,"Usage: syl [-integer] filename\n");
			exit(1);
		}
		var = atoi(argv[1]+1);
	}
	else var = 0;
	while(getword()!=EOF){
		if(a[0] == '\0')continue;
		change();
		nsyl=estimate();
		if(nsyl >= var)
			printf("%d-%s\n",nsyl,a);
	}

}
getword(){
	int skip,c2;
	int ret;
	register char *aptr;
	char last;
	ret = skip = 0;
	for(aptr=a;aptr<&a[MAXCHAR];aptr++){
		*aptr = getchar();
		if(*aptr == EOF){
			ret=EOF;
			break;
		}
		if(isdigit(*aptr)){
			skip = 1;
			continue;
		}
		if(isspace(*aptr)) break;
		if(!isalnum(*aptr)){
			last = *(aptr-1);
			switch(*aptr){
			case '-':
				if(((c2=getchar())=='o' && last=='o')||
				    (c2=='e' &&  last=='e'))
					ungetc(c2,stdin);
				else if(skip==0){
					ungetc(c2,stdin);
					goto found;
				}
				else{
					ungetc(c2,stdin);
					goto found;
				}
				break;
			case '\'':
				if(last=='x' || last=='c' ||last=='s'
				     || last=='h' ||(*(aptr-2)=='g' && last=='e'))
					*aptr='\'';
				else if ((c2=getchar())=='t' &&  last=='n'){
					ungetc(c2,stdin);
					*aptr='\'';
				}
				else{
					ungetc(c2,stdin);
					aptr--;
				}
				break;
			case ',':
			case ';':
			case '?':
			case '!':
			case ':':
			case '~':
			case '/':
				goto found;
			default:
				aptr--;
			}
		}
	}
found:
	*aptr= '\0';
	return(ret);
}

change()
{
	register char *aptr, *bptr;

	char temp;
	for(aptr=a, bptr=b;*aptr!='\0';aptr++,bptr++){
		switch(*aptr){
		case 'a':
		case 'A':
		case 'Y':
		case 'E':
		case 'O':
		case 'I':
		case 'U':
		case 'o':
		case '\'':
			*bptr='V';
			break;

		case 'e':
			if(*(aptr+1)=='a'){
				if(scmp("creat",(aptr-2)) && *(aptr+3) != 'u')
					*bptr = 'S';
				else if(scmp("reac",(aptr-1)) && *(aptr+3) != 'h')
					*bptr='S';
				else if(*(aptr+2) == '\0')*bptr = 'S';
				else if(*(aptr+2) == 's' && *(aptr+3) == '\0')
					*bptr = 'S';
				else if(scmp("reali",(aptr-1)))*bptr='S';
				else *bptr='V';
			}
			else if(*(aptr+1)=='o'){
				if(aptr == &a[1] && (*(aptr-1) == 'd' || *(aptr-1) == 'r'))
					*bptr = 'S';
				else if(aptr == &a[2] && (*(aptr-2) == 'p' && *(aptr-1) == 'r'))
					*bptr= 'S';
				else if(*(aptr+2)=='u') *bptr='S';
				else if(*(aptr+2) == '0') *bptr='S';
				else *bptr='V';
			}
			else if(*(bptr-2)=='C' && *(aptr-2)!='l' &&
			    *(aptr-1)=='l') *bptr='V';
			else if( *(aptr+1)=='l' &&*(aptr+2)=='y' )
				 *bptr='C';
			else if(scmp("liness",aptr+1))*bptr='C';
			else if(scmp("lihood",aptr+1))*bptr='C';
			else if(scmp("ful",aptr+1))*bptr = 'C';
			else if(scmp("nless",aptr+1))*bptr = 'C';
			else if(scmp("ment",aptr+1))*bptr = 'C';
			else if(scmp("ship",aptr+1))*bptr='C';
			else *bptr='V';
			break;

		case 'i':
			if(*(aptr+1)=='e'){
				if(*(aptr+2)=='r') *bptr='S';
				else if(*(aptr+2)=='t') *bptr='S';
				else if(scmp("icient",aptr-2))*bptr='V';
				else if(scmp("nient",aptr-1))*bptr='V';
				else if(*(aptr+2)=='n' && *(aptr+3)=='t')
					*bptr='S';
				else if(scmp("icienc",aptr-2))*bptr='V';
				else if(scmp("nienc",aptr-1))*bptr='V';
				else if(*(aptr+2)=='n' && *(aptr+3)=='c')
					*bptr='S';
				else *bptr='V';
			}
			else if(*(aptr+1)=='o'){
				if(*(aptr-1)=='v' && *(aptr+2)=='r') *bptr='V';
				else if(*(aptr-1)=='r') *bptr='S';
				else if(*(aptr+2)=='u') *bptr='S';
				else if(*(aptr-1)=='v' || *(aptr-1)=='V')
					*bptr='S';
				else if(*(aptr+2) == '\0') *bptr='S';
				else *bptr='V';
			}
			else if(*(aptr+1)=='a'){
				if((*(aptr-1)=='c' || *(aptr-1)=='t') &&
					  *(aptr-2)=='o') *bptr='S';
				else if (*(aptr-1)=='c' || *(aptr-1)=='t')
					*bptr='V';
				else *bptr='S';
			}
			else *bptr='V';
			break;
		case 'u':
			if(*(aptr-1) == 'q' || *(aptr-1) == 'Q')
				*bptr = 'C';
			else if( (*(aptr-1) == 'g' || *(aptr-1) == 'G') &&
			    (isvowel(*(aptr+1))||*(aptr+1) == 'y')) *bptr='C';
			else if(*(aptr+1)=='a'){
				if(*(aptr-1)=='t' || *(aptr-1)=='d' ||
					*(aptr-1)=='s') *bptr='S';
				else if(*(aptr+2)=='t') *bptr='S';
				else *bptr='V';
			}
			else if(*(aptr+1)=='e'){
				if(*(aptr-1)=='l') *bptr='S';
				else if(*(aptr-2)=='g' && *(aptr-1)=='r')
					*bptr='S';
				else *bptr='V';
			}
			else *bptr='V';
			break;
		case 'y':
			if((aptr>a&& *(aptr+1) != '\0') && *(bptr-1) == 'V'
				&& (isvowel(*(aptr+1))))
				*bptr='C';
			else *bptr='V';
			break;
		case 'n':
			if(*(aptr-1)=='i' && *(aptr+1)=='g'){
				*(bptr-1)='C';
				*bptr='V';
			}
			else *bptr='C';
			break;
		default:
			*bptr='C';
		}
	}

	*bptr='\0';
			/*sitting on last char*/
	aptr--;
	bptr--;
	if(*(aptr-1)=='e' && *aptr=='d'){
		if(*(aptr-2)=='t' || *(aptr-2) == 'd'||(*(bptr-3)=='C' && *(aptr-2)=='l'
		    && *(aptr-3) !='l')){
			*(bptr) = *(bptr-2) = 'C';
			*(bptr-1)='V';
		}
		else{
			*bptr = *(bptr-1)= 'C';
		}
		return;
	}
	temp = *(aptr-2);
	if((temp=='s'||temp=='g'|| temp== 'h'|| temp=='c'|| temp=='z')  && (*(aptr-1)=='e' &&  *aptr=='s')){
		*bptr = *(bptr-2)='C';
		*(bptr-1)='V';
		return;
	}
	if(*(bptr-3)=='C' && temp=='l' && *(aptr-1)=='e' && *aptr=='s'){
		*bptr = *(bptr-2)='C';
		*(bptr-1)='V';
		return;
	}
	if(*(aptr-1)=='e' && *aptr=='s'){
		*bptr = *(bptr-1) = 'C';
		return;
	}
	if(*(bptr-2)=='C' && *(aptr-1)=='l' && *aptr=='e'){
		*bptr='V';
		*(bptr-1)='C';
		return;
	}
	if(*aptr=='e') *bptr='C';

}

estimate()
{
	register char *bptr;
	int k,f,i;
	k=0;
	f=1;
	for(bptr=b;*bptr!='\0';bptr++){
		if(f==1 && *bptr=='V'){
			f=2;
			k++;
		}
		if(f==1 && *bptr=='S') k++;
		if(f==2 && *bptr=='C') f=1;
		else if(f==2 && *bptr=='S'){
			f=1;
			k++;
		}
	}
	if(k>0) return(k);
	else return(1);
}

scmp(s1,s2)
char *s1,*s2;
{
	char *s, *p;
	for(s=s1, p=s2;*s != '\0';s++,p++){
		if(*s != *p)return(0);
	}
	return(1);
}
