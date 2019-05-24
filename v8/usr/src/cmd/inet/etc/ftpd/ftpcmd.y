/*
 * Grammar for FTP commands.
 * See RFC 765.
 */

%{

#ifndef lint
static char sccsid[] = "@(#)ftpcmd.y	4.11 83/06/22";
#endif

#include <sys/types.h>

#include <sys/inet/in.h>

#include "ftp.h"

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <pwd.h>
#include <setjmp.h>

struct socket {
	unsigned short sport;
	unsigned short dport;
	long saddr;
	long daddr;
};
extern	struct socket data_dest;
extern	int logged_in;
extern	struct passwd *pw;
extern	int guest;
extern	int logging;
extern	int type;
extern	int form;
extern	int debug;
extern	int timeout;
extern	char hostname[];
extern	char *globerr;
extern	int usedefault;
char	**glob();

static	int cmd_type;
static	int cmd_form;
static	int cmd_bytesz;

char	*index();
%}

%token
	A	B	C	E	F	I
	L	N	P	R	S	T

	SP	CRLF	COMMA	STRING	NUMBER

	USER	PASS	ACCT	REIN	QUIT	PORT
	PASV	TYPE	STRU	MODE	RETR	STOR
	APPE	MLFL	MAIL	MSND	MSOM	MSAM
	MRSQ	MRCP	ALLO	REST	RNFR	RNTO
	ABOR	DELE	CWD	LIST	NLST	SITE
	STAT	HELP	NOOP	XMKD	XRMD	XPWD
	XCUP

	LEXERR

%start	cmd_list

%%

cmd_list:	/* empty */
	|	cmd_list cmd
	;

cmd:		USER SP username CRLF
		= {
			extern struct passwd *getpwnam();

			if (strcmp($3, "ftp") == 0 ||
			  strcmp($3, "anonymous") == 0) {
				if ((pw = getpwnam("ftp")) != NULL) {
					guest = 1;
					reply(331,
				  "Guest login ok, send ident as password.");
				}
			} else if (checkuser($3)) {
				guest = 0;
				pw = getpwnam($3);
				reply(331, "Password required for %s.", $3);
			}
			if (pw == NULL)
				reply(530, "User %s unknown.", $3);
			free($3);
		}
	|	PASS SP password CRLF
		= {
			pass($3);
			free($3);
		}
	|	PORT SP host_port CRLF
		= {
			usedefault = 0;
			ack($1);
		}
	|	TYPE SP type_code CRLF
		= {
			switch (cmd_type) {

			case TYPE_A:
				if (cmd_form == FORM_N) {
					reply(200, "Type set to A.");
					type = cmd_type;
					form = cmd_form;
				} else
					reply(504, "Form must be N.");
				break;

			case TYPE_E:
				reply(504, "Type E not implemented.");
				break;

			case TYPE_I:
				reply(200, "Type set to I.");
				type = cmd_type;
				break;

			case TYPE_L:
				if (cmd_bytesz == 8) {
					reply(200,
					    "Type set to L (byte size 8).");
					type = cmd_type;
				} else
					reply(504, "Byte size must be 8.");
			}
		}
	|	STRU SP struct_code CRLF
		= {
			switch ($3) {

			case STRU_F:
				reply(200, "STRU F ok.");
				break;

			default:
				reply(502, "Unimplemented STRU type.");
			}
		}
	|	MODE SP mode_code CRLF
		= {
			switch ($3) {

			case MODE_S:
				reply(200, "MODE S ok.");
				break;

			default:
				reply(502, "Unimplemented MODE type.");
			}
		}
	|	ALLO SP NUMBER CRLF
		= {
			ack($1);
		}
	|	RETR check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				retrieve(0, $4);
			if ($4 != NULL)
				free($4);
		}
	|	STOR check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				store($4, "w");
			if ($4 != NULL)
				free($4);
		}
	|	APPE check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				store($4, "a");
			if ($4 != NULL)
				free($4);
		}
	|	NLST check_login CRLF
		= {
			if ($2)
				retrieve("/bin/ls", "");
		}
	|	NLST check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				retrieve("/bin/ls %s", $4);
			if ($4 != NULL)
				free($4);
		}
	|	LIST check_login CRLF
		= {
			if ($2)
				retrieve("/bin/ls -lg", "");
		}
	|	LIST check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				retrieve("/bin/ls -lg %s", $4);
			if ($4 != NULL)
				free($4);
		}
	|	DELE check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				delete($4);
			if ($4 != NULL)
				free($4);
		}
	|	CWD check_login CRLF
		= {
			if ($2)
				cwd(pw->pw_dir);
		}
	|	CWD check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				cwd($4);
			if ($4 != NULL)
				free($4);
		}
	|	rename_cmd
	|	HELP CRLF
		= {
			help(0);
		}
	|	HELP SP STRING CRLF
		= {
			help($3);
		}
	|	NOOP CRLF
		= {
			ack($1);
		}
	|	XMKD check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				makedir($4);
			if ($4 != NULL)
				free($4);
		}
	|	XRMD check_login SP pathname CRLF
		= {
			if ($2 && $4 != NULL)
				removedir($4);
			if ($4 != NULL)
				free($4);
		}
	|	XPWD check_login CRLF
		= {
			if ($2)
				pwd();
		}
	|	XCUP check_login CRLF
		= {
			if ($2)
				cwd("..");
		}
	|	QUIT CRLF
		= {
			reply(221, "Goodbye.");
			dologout(0);
		}
	|	error CRLF
		= {
			yyerrok;
		}
	;

username:	STRING
	;

password:	STRING
	;

byte_size:	NUMBER
	;

host_port:	NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA 
		NUMBER COMMA NUMBER
		= {
			register char *a, *p;

			a = (char *)&data_dest.saddr;
			a[0] = $7; a[1] = $5; a[2] = $3; a[3] = $1;
			p = (char *)&data_dest.sport;
			p[0] = $11; p[1] = $9;
		}
	;

form_code:	N
	= {
		$$ = FORM_N;
	}
	|	T
	= {
		$$ = FORM_T;
	}
	|	C
	= {
		$$ = FORM_C;
	}
	;

type_code:	A
	= {
		cmd_type = TYPE_A;
		cmd_form = FORM_N;
	}
	|	A SP form_code
	= {
		cmd_type = TYPE_A;
		cmd_form = $3;
	}
	|	E
	= {
		cmd_type = TYPE_E;
		cmd_form = FORM_N;
	}
	|	E SP form_code
	= {
		cmd_type = TYPE_E;
		cmd_form = $3;
	}
	|	I
	= {
		cmd_type = TYPE_I;
	}
	|	L
	= {
		cmd_type = TYPE_L;
		cmd_bytesz = 8;
	}
	|	L SP byte_size
	= {
		cmd_type = TYPE_L;
		cmd_bytesz = $3;
	}
	/* this is for a bug in the BBN ftp */
	|	L byte_size
	= {
		cmd_type = TYPE_L;
		cmd_bytesz = $2;
	}
	;

struct_code:	F
	= {
		$$ = STRU_F;
	}
	|	R
	= {
		$$ = STRU_R;
	}
	|	P
	= {
		$$ = STRU_P;
	}
	;

mode_code:	S
	= {
		$$ = MODE_S;
	}
	|	B
	= {
		$$ = MODE_B;
	}
	|	C
	= {
		$$ = MODE_C;
	}
	;

pathname:	pathstring
	= {
		if ($1 && strncmp($1, "~", 1) == 0) {
			$$ = (int)*glob($1);
			if (globerr != NULL) {
				reply(550, globerr);
				$$ = NULL;
			}
			free($1);
		} else
			$$ = $1;
	}
	;

pathstring:	STRING
	;

rename_cmd:	rename_from rename_to
	= {
		if ($1 && $2)
			renamecmd($1, $2);
		else
			reply(503, "Bad sequence of commands.");
		if ($1)
			free($1);
		if ($2)
			free($2);
	}
	;

rename_from:	RNFR check_login SP pathname CRLF
	= {
		char *from = 0, *renamefrom();

		if ($2 && $4)
			from = renamefrom($4);
		if (from == 0 && $4)
			free($4);
		$$ = (int)from;
	}
	;

rename_to:	RNTO SP pathname CRLF
	= {
		$$ = $3;
	}
	;

check_login:	/* empty */
	= {
		if (logged_in)
			$$ = 1;
		else {
			reply(530, "Please login with USER and PASS.");
			$$ = 0;
		}
	}
	;

%%

extern jmp_buf errcatch;

#define	CMD	0	/* beginniAg of command */
#define	ARGS	1	/* expect miscellaneous arguments */
#define	STR1	2	/* expect SP followed by STRING */
#define	STR2	3	/* expect STRING */
#define	OSTR	4	/* optional STRING */

struct tab {
	char	*name;
	short	token;
	short	state;
	short	implemented;	/* 1 if command is implemented */
	char	*help;
};

struct tab cmdtab[] = {		/* In order defined in RFC 765 */
	{ "USER", USER, STR1, 1,	"<sp> username" },
	{ "PASS", PASS, STR1, 1,	"<sp> password" },
	{ "ACCT", ACCT, STR1, 0,	"(specify account)" },
	{ "REIN", REIN, ARGS, 0,	"(reinitialize server state)" },
	{ "QUIT", QUIT, ARGS, 1,	"(terminate service)", },
	{ "PORT", PORT, ARGS, 1,	"<sp> b0, b1, b2, b3, b4" },
	{ "PASV", PASV, ARGS, 0,	"(set server in passive mode)" },
	{ "TYPE", TYPE, ARGS, 1,	"<sp> [ A | E | I | L ]" },
	{ "STRU", STRU, ARGS, 1,	"(specify file structure)" },
	{ "MODE", MODE, ARGS, 1,	"(specify transfer mode)" },
	{ "RETR", RETR, STR1, 1,	"<sp> file-name" },
	{ "STOR", STOR, STR1, 1,	"<sp> file-name" },
	{ "APPE", APPE, STR1, 1,	"<sp> file-name" },
	{ "MLFL", MLFL, OSTR, 0,	"(mail file)" },
	{ "MAIL", MAIL, OSTR, 0,	"(mail to user)" },
	{ "MSND", MSND, OSTR, 0,	"(mail send to terminal)" },
	{ "MSOM", MSOM, OSTR, 0,	"(mail send to terminal or mailbox)" },
	{ "MSAM", MSAM, OSTR, 0,	"(mail send to terminal and mailbox)" },
	{ "MRSQ", MRSQ, OSTR, 0,	"(mail recipient scheme question)" },
	{ "MRCP", MRCP, STR1, 0,	"(mail recipient)" },
	{ "ALLO", ALLO, ARGS, 1,	"allocate storage (vacuously)" },
	{ "REST", REST, STR1, 0,	"(restart command)" },
	{ "RNFR", RNFR, STR1, 1,	"<sp> file-name" },
	{ "RNTO", RNTO, STR1, 1,	"<sp> file-name" },
	{ "ABOR", ABOR, ARGS, 0,	"(abort operation)" },
	{ "DELE", DELE, STR1, 1,	"<sp> file-name" },
	{ "CWD",  CWD,  OSTR, 1,	"[ <sp> directory-name]" },
	{ "XCWD", CWD,	OSTR, 1,	"[ <sp> directory-name ]" },
	{ "LIST", LIST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "NLST", NLST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "SITE", SITE, STR1, 0,	"(get site parameters)" },
	{ "STAT", STAT, OSTR, 0,	"(get server status)" },
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ "NOOP", NOOP, ARGS, 1,	"" },
	{ "XMKD", XMKD, STR1, 1,	"<sp> path-name" },
	{ "XRMD", XRMD, STR1, 1,	"<sp> path-name" },
	{ "XPWD", XPWD, ARGS, 1,	"(return current directory)" },
	{ "XCUP", XCUP, ARGS, 1,	"(change to parent directory)" },
	{ NULL,   0,    0,    0,	0 }
};

struct tab *
lookup(cmd)
	char *cmd;
{
	register struct tab *p;

	for (p = cmdtab; p->name != NULL; p++)
		if (strcmp(cmd, p->name) == 0)
			return (p);
	return (0);
}

#include "telnet.h"

/*
 * getline - a hacked up version of fgets to ignore TELNET escape codes.
 */
char *
getline(s, n, iop)
	char *s;
	register FILE *iop;
{
	register c;
	register char *cs;

	cs = s;
	while (--n > 0 && (c = getc(iop)) >= 0) {
		while (c == IAC) {
			c = getc(iop);	/* skip command */
			c = getc(iop);	/* try next char */
		}
		*cs++ = c;
		if (c=='\n')
			break;
	}
	if (c < 0 && cs == s)
		return (NULL);
	*cs++ = '\0';
	if (debug) {
		fprintf(stderr, "FTPD: command: %s", s);
		if (c != '\n')
			putc('\n', stderr);
		fflush(stderr);
	}
	return (s);
}

static int
toolong()
{
	long now;
	extern char *ctime();

	reply(421,
	  "Timeout (%d seconds): closing control connection.", timeout);
	time(&now);
	if (logging) {
		fprintf(stderr,
			"FTPD: User %s timed out after %d seconds at %s",
			(pw ? pw -> pw_name : "unknown"), timeout, ctime(&now));
		fflush(stderr);
	}
	dologout(1);
}

yylex()
{
	static char cbuf[512];
	static int cpos, state;
	register char *cp;
	register struct tab *p;
	int n;
	char c;

	for (;;) {
		switch (state) {

		case CMD:
			signal(SIGALRM, toolong);
			alarm(timeout);
			if (getline(cbuf, sizeof(cbuf)-1, stdin) == NULL) {
				reply(221, "You could at least say goodbye.");
				dologout(0);
			}
			alarm(0);
			if (index(cbuf, '\r')) {
				cp = index(cbuf, '\r');
				cp[0] = '\n'; cp[1] = 0;
			}
			if (index(cbuf, ' '))
				cpos = index(cbuf, ' ') - cbuf;
			else
				cpos = 4;
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cbuf);
			p = lookup(cbuf);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					nack(p->name);
					longjmp(errcatch);
					/* NOTREACHED */
				}
				state = p->state;
				yylval = (int) p->name;
				return (p->token);
			}
			break;

		case OSTR:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALL THRU */

		case STR1:
			if (cbuf[cpos] == ' ') {
				cpos++;
				state = STR2;
				return (SP);
			}
			break;

		case STR2:
			cp = &cbuf[cpos];
			n = strlen(cp);
			cpos += n - 1;
			/*
			 * Make sure the string is nonempty and \n terminated.
			 */
			if (n > 1 && cbuf[cpos] == '\n') {
				cbuf[cpos] = '\0';
				yylval = copy(cp);
				cbuf[cpos] = '\n';
				state = ARGS;
				return (STRING);
			}
			break;

		case ARGS:
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval = atoi(cp);
				cbuf[cpos] = c;
				return (NUMBER);
			}
			switch (cbuf[cpos++]) {

			case '\n':
				state = CMD;
				return (CRLF);

			case ' ':
				return (SP);

			case ',':
				return (COMMA);

			case 'A':
			case 'a':
				return (A);

			case 'B':
			case 'b':
				return (B);

			case 'C':
			case 'c':
				return (C);

			case 'E':
			case 'e':
				return (E);

			case 'F':
			case 'f':
				return (F);

			case 'I':
			case 'i':
				return (I);

			case 'L':
			case 'l':
				return (L);

			case 'N':
			case 'n':
				return (N);

			case 'P':
			case 'p':
				return (P);

			case 'R':
			case 'r':
				return (R);

			case 'S':
			case 's':
				return (S);

			case 'T':
			case 't':
				return (T);

			}
			break;

		default:
			fatal("Unknown state in scanner.");
		}
		yyerror();
		state = CMD;
		longjmp(errcatch);
	}
}

upper(s)
	char *s;
{
	while (*s != '\0') {
		if (islower(*s))
			*s = toupper(*s);
		s++;
	}
}

copy(s)
	char *s;
{
	char *p;
	extern char *malloc();

	p = malloc(strlen(s) + 1);
	if (p == NULL)
		fatal("Ran out of memory.");
	strcpy(p, s);
	return ((int)p);
}

help(s)
	char *s;
{
	register struct tab *c;
	register int width, NCMDS;

	width = 0, NCMDS = 0;
	for (c = cmdtab; c->name != NULL; c++) {
		int len = strlen(c->name);

		if (c->implemented == 0)
			len++;
		if (len > width)
			width = len;
		NCMDS++;
	}
	width = (width + 8) &~ 7;
	if (s == 0) {
		register int i, j, w;
		int columns, lines;

		lreply(214,
	  "The following commands are recognized (* =>'s unimplemented).");
		columns = 76 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			printf("    ");
			for (j = 0; j < columns; j++) {
				c = cmdtab + j * lines + i;
				printf("%s%c", c->name,
					c->implemented ? ' ' : '*');
				if (c + lines >= &cmdtab[NCMDS])
					break;
				w = strlen(c->name);
				while (w < width) {
					putchar(' ');
					w++;
				}
			}
			printf("\r\n");
		}
		fflush(stdout);
		reply(214, "Direct comments to ftp-bugs@%s.", hostname);
		return;
	}
	upper(s);
	c = lookup(s);
	if (c == (struct tab *)0) {
		reply(504, "Unknown command %s.", s);
		return;
	}
	if (c->implemented)
		reply(214, "Syntax: %s %s", c->name, c->help);
	else
		reply(214, "%-*s\t%s; unimplemented.", width, c->name, c->help);
}
