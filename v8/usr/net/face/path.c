#include "trie.h"
extern char	*permalloc();
Trie *
tlookup(s, t)
	register char *s;
	register Trie *t;
{
	while(t){
		if(t->byte==*s){
			t=t->link;
			if(*s++==0)
				return t;
		}else
			t=t->next;
	}
	return 0;
}
Trie *
tcreate(s, v)
	register char *s;
	register Trie *v;
{
	register Trie *t, *trie;
	trie=t=(Trie *)permalloc(sizeof(Trie));
	t->next=0;
	if((t->byte=*s++)==0)
		t->link=v;
	else{
		do{
			t->link=(Trie *)permalloc(sizeof(Trie));
			t=t->link;
			t->next=0;
			t->byte=*s;
		}while(*s++);
	}
	t->link=v;
	return trie;
}
tinsert(s, v, t)
	register char *s;
	register Trie *v, *t;
{
	for(;;){
		if(t->byte==*s){
			if(*s++==0){		/* replacement */
				t->link=v;
				break;
			}
			t=t->link;
		}else if(t->next==0){	/* insertion */
			t->next=tcreate(s, v);
			break;
		}else
			t=t->next;
	}
}
