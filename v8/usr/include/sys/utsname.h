/*
 *	structure for the "uname" system call.  This file is
 *	not used by the operating system proper, but is included
 *	here for compatibility with the BTL development UNIX systems.
 */

struct utsname {
	char sysname[9];
	char nodename[9];
	char release[9];
	char version[9];
};
