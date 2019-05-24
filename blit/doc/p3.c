#include <blit.h>
main(){
	Point p;
	request(MOUSE);
	for(;;wait(MOUSE)){
		if(button1()){
		    jmoveto(mouse.jxy);
		    for(; button1(); nap(2))
			jlineto(mouse.jxy, F_OR);
		}else if(button3())
			exit();
	}
}
