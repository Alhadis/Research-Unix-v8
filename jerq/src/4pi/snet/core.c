#include "term.h"
#include "../protocol.h"

Read()
{
	register char *loc = (char*) RcvLong(), c, *error = 0;
	register long bytes =  RcvLong();

	journal( "Read", "loc=%x bytes=%d", loc, bytes );
	while( bytes-- > 0 ){
		if( !error && !(error = ReadErr(loc,1)) )
			c = *loc++;
		SendUChar(c);
	}
	SendError(error);
}

Write()
{
	register char *loc = (char*) RcvLong(), c, *error = 0;
	register long bytes =  RcvLong();

	journal( "Write", "loc=%x bytes=%d", loc, bytes );
	while( bytes-- > 0 ){
		c = RcvUChar();
		if( !error && !(error = WriteErr(loc,1)) )
			*loc++ = c;
	}
	SendError(error);
}

String()
{
	register char *s = (char*) RcvLong(), *error;
	journal( "String", "%x", s );
	if( (error = ReadErr(s,1)) || (error = ReadErr(s+260,1)) )
		s = "";
	SendString(s);
	SendError(error);
}

LayBpt()
{
}

LiftBpt()
{
}

char *RegLoc(p,error)
{
}

char *Stack(p,error)
{
}	

char *Control(p,error)
{
}

char *Run(p,error)
{
}

char *Stop(p,error)
{
}

char *Step();

CoreP(pkt)
{
	long p = RcvLong();
	register char *error = 0;

	journal( "CoreP", "pkt=%x p=%x", pkt, p );
	switch( pkt ){
	case CP_STACK:	error = Stack(p,error);		break;
	case CP_CONTROL:	error = Control(p,error);	break;
	case CP_REGLOC:	error = RegLoc(p,error);	break;
	case CP_RUN:		error = Run(p,error);		break;
	case CP_STEP:	error = Step(p,error);		break;
	case CP_STOP:	error = Stop(p,error);		break;
	default:		assert(0);
	}
	SendError(error);
}
