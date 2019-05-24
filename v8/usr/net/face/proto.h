struct msghdr {
	short	type;
	short	len;
	long	param1;
	long	param2;
} mh;

union msgbuf {
	char path[128];
	char buf[4096];
	struct stat statb;
} mb;
		

#define DOSTAT 1
#define DOREAD 2

#define SERVER "faceS"
