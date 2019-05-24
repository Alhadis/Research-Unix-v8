From research!ulysses!smb  Sat Jan 28 18:10:55 1984
Date: Sat, 28 Jan 84 18:10:55 est
From: ulysses!smb (Steven Bellovin)
Message-Id: <8401282310.AA25070@ulysses.UUCP>
Received: by ulysses.UUCP (4.12/3.7)
	id AA25070; Sat, 28 Jan 84 18:10:55 est
To: research!rob
Subject: h/jioctl.h

/*
**	Unix to Blit I/O control codes
*/

#ifndef _JIOCTL_
#define	_JIOCTL_

#define	JBOOT		_IO(j, 1)
#define	JTERM		_IO(j, 2)
#define	JMPX		_IO(j, 3)
#define	JTIMO		_IO(j, 4)	/* Timeouts in seconds */
#define	JWINSIZE	_IOR(j, 5, struct winsize)
#define	JTIMOM		_IO(j, 6)	/* Timeouts in millisecs */
#define	JZOMBOOT	_IO(j, 7)
#define JSWINSIZE	_IOW(j, 8, struct winsize)
#define JSMPX		_IOW(j, 9, int)

struct winsize
{
	char	bytesx, bytesy;	/* Window size in characters */
	short	bitsx, bitsy;	/* Window size in bits */
};

/**	Channel 0 control message format **/

struct jerqmesg
{
	char	cmd;		/* A control code above */
	char	chan;		/* Channel it refers to */
};

/*
**	Character-driven state machine information for Blit to Unix communication.
*/

#define	C_SENDCHAR	1	/* Send character to layer process */
#define	C_NEW		2	/* Create new layer process group */
#define	C_UNBLK		3	/* Unblock layer process */
#define	C_DELETE	4	/* Delete layer process group */
#define	C_EXIT		5	/* Exit */
#define	C_BRAINDEATH	6	/* Send terminate signal to proc. group */
#define	C_SENDNCHARS	7	/* Send several characters to layer proc. */
#define	C_RESHAPE	8	/* Layer has been reshaped */

/*
**	Usual format is: [command][data]
*/
#endif _JIOCTL_

