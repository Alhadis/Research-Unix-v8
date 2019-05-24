/* NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT */
/* Writer's Workbench version 2.0, January 1981 */
#include<stdio.h>
#define min(a,b)	(a>b?b:a)
#define MAXWORDS 200
#define MAXCHAR 2000
#define MAJOR 7
char buf[MAXCHAR];
char *bptr;
struct word{
	int brklvl;
	char *string;
};
struct word line[MAXWORDS];
struct word *fst;
struct word *lst;

main(){
	int l;
struct word *ptr;
	do{
		lst=fst= &line[0];
		bptr = buf;
		l=readline();
		findbrk();
		if(fst != lst){
			divide();
			writes();
		}
	} while(l != EOF);
}


readline()
{
	int flag;
	do{
		lst->string = bptr;
		flag=getword();
		if(lst->string == bptr)
			break;
		lst++;
	} while(flag == 1 && lst <= &line[MAXWORDS]);
	return(flag);
}


getword()
{
	int c,i;
	int ret;
	lst->brklvl = 0;
	ret = 1;
	for(i=0;i < MAXCHAR-1; ++i){
		c= *bptr++ = getchar();
		switch(c){
		case '.':
		case '?':
		case '!':
			lst->brklvl=5;
			ret = 0;
			goto done;
		case ';':
		case ':':
			lst->brklvl=4;
			goto done;
		case ',':
			lst->brklvl=3;
			goto done;
		case ' ':
		case '\t':
		case '\n':
			--bptr;
			if(bptr==lst->string) {
				break;
			}
			lst->brklvl=0;
			goto done;
		case EOF:
			--bptr;
			if(bptr==lst->string){
				lst->brklvl=0;
				return(EOF);
			}
			lst->brklvl=0;
			ret = EOF;
			goto done;

		}
	}
done:
	*bptr++='\0';
	return(ret);
}


int m[6] = {7,7,7,5,5,0};
divide()
{
	struct word *ptr;
	int i,pc,ba[3][20],j,np,ct,nct,jct;
	i=0;
	j=1;
	for(ptr=fst;ptr <lst;ptr++){
		if(ptr->brklvl != 0){
			ba[0][i]=j;
			ba[1][i]=ptr->brklvl;
			i++;
		}
		j++;
	}
	pc = i;
	for(np=pc-1;np>0;np--) ba[2][np]=ba[0][np]-ba[0][np-1];
	ba[2][0]=ba[0][0];

	ct=0;
	for (i=0;i<pc-1;i++){
		ct=ct+ba[2][i];
		while (ct >7){
			if (ct <= 10) {
				(fst+(ba[0][i]-1))->brklvl=6;
				ct=0;
			}
			else {
				nct = ct/2;
				if (nct > 7) nct =7;
				(fst + (ba[0][i] - ct + nct) )->brklvl = 6;
				ct = ct - nct;
			}
		}
		if(ct >2){
			jct=0;
			for(j=i+1;j<pc;j++){
				jct=jct+ba[2][j];
				if(jct >min(m[ba[1][i]],7-ct)){
					(fst+(ba[0][i]-1))->brklvl=6;
					ct=0;
					break;
				}
				else if(ba[1][i] <=ba[1][j]) break;
			}
		}
	}
}
writes()
{
	struct word *ptr;
	for(ptr=fst;ptr <lst; ptr++){
		printf("%s ",ptr->string);
		if(ptr->brklvl>=5)putchar('\n');
	}
}

char *wds[]={
	"and",
	"is",
	"at",
	"but",
	"or",
	"if",
	"because",
	"since",
	"to",	/* beginning of minor wds*/
	"in",
	"that",
	"for",
	"with",
	"on",
	"by",
	"from",
	"which",
	"would",
	"when",
	"what",
	"into",
	"than",
	"could",
	"may",
	"where",
	"through",
	"how",
	"between",
	"without",
	"why",
	"among",
	"beside",
	"besides",
	0
};

findbrk(){
	struct word *ptr;
	int ret;
	char **p;
	ret=0;
	for(ptr=fst; ptr< lst; ptr++){
		for(p=wds;*p!=0;p++){
			if(strcmp(ptr->string,*p)== 0){
				if((ptr-1)->brklvl!=3){
					if(p > &wds[MAJOR])
						(ptr-1)->brklvl=1;
					else (ptr-1)->brklvl = 2;
				}
				ret=1;
				break;
			}   
		}
	}
	return(ret);
}

