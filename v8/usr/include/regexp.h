#define NSUBEXP  10
typedef struct regexp { /* see regexp(3) */
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char program[1];
} regexp;
