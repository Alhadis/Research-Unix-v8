#include <jerq.h>
introutine(){
	if(button123())
		exit();
}
Word grey[16]={
	0xAAAA,	0x5555,	0xAAAA,	0x5555,	0xAAAA,	0x5555,	0xAAAA,	0x5555,
	0xAAAA,	0x5555,	0xAAAA,	0x5555,	0xAAAA,	0x5555,	0xAAAA,	0x5555,
};
Word block[16]={
	0xCCCC,	0xCCCC,	0x3333,	0x3333,	0xCCCC,	0xCCCC,	0x3333,	0x3333,
	0xCCCC,	0xCCCC,	0x3333,	0x3333,	0xCCCC,	0xCCCC,	0x3333,	0x3333,
};
main(){
	register i, j;
	jinit();
	sysinit();
	spl0();
	cursinhibit();
	for(i=0; i<8; i++)
		for(j=0; j<10; j++)
			jtexture(Rect(i*100, j*100, (i+1)*100, (j+1)*100),
				((i+j)&1)? grey : block, F_STORE);
	for(;;)
		;
}