/*	sysent.c	4.4	81/03/08	*/

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 * Each row contains the number of arguments expected
 * and a pointer to the routine.
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/trace.h"

int	alarm();
int	chdir();
int	chmod();
int	chown();
int	chroot();
int	close();
int	creat();
int	dup();
int	exec();
int	exece();
int	fchmod();
int	fchown();
int	fork();
int	fstat();
int	ftime();
int	getgid();
int	getpid();
int	getuid();
int	gmount();
int	gtime();
int	ioctl();
int	kill();
int	link();
int	lstat();
int	mkdir();
int	mknod();
int	nap();
int	nice();
int	nosys();
int	nullsys();
int	open();
int	pause();
int	pipe();
int	profil();
int	read();
int	readlink();
int	reboot();
int	rexit();
int	rmdir();
int	saccess();
int	sbreak();
int	seek();
int	select();
int	setgid();
int	setpgrp();
int	setuid();
int	fsmount();
int	splice();
int	ssig();
int	stat();
int	stime();
int	sumount();
int	symlink();
int	sync();
int	sysacct();
int	syslock();
int	sysphys();
int	times();
int	umask();
int	unlink();
int	utime();
int	wait();
int	write();

#include "../h/vmsysent.h"

struct sysent sysent[128] =
{
	0, nosys,			/*  0 = indir */
	1, rexit,			/*  1 = exit */
	0, fork,			/*  2 = fork */
	3, read,			/*  3 = read */
	3, write,			/*  4 = write */
	2, open,			/*  5 = open */
	1, close,			/*  6 = close */
	0, wait,			/*  7 = wait */
	2, creat,			/*  8 = creat */
	2, link,			/*  9 = link */
	1, unlink,			/* 10 = unlink */
	2, exec,			/* 11 = exec */
	1, chdir,			/* 12 = chdir */
	0, gtime,			/* 13 = time */
	3, mknod,			/* 14 = mknod */
	2, chmod,			/* 15 = chmod */
	3, chown,			/* 16 = chown; now 3 args */
	1, sbreak,			/* 17 = break */
	2, stat,			/* 18 = stat */
	3, seek,			/* 19 = seek */
	0, getpid,			/* 20 = getpid */
	3, fsmount,			/* 21 = mount */
	1, sumount,			/* 22 = umount */
	1, setuid,			/* 23 = setuid */
	0, getuid,			/* 24 = getuid */
	1, stime,			/* 25 = stime */
	0, nosys,			/* 26 = former ptrace */
	1, alarm,			/* 27 = alarm */
	2, fstat,			/* 28 = fstat */
	0, pause,			/* 29 = pause */
	2, utime,			/* 30 = utime */
	2, fchmod,			/* 31 = fchmod, was stty */
	3, fchown,			/* 32 = fchown, was gtty */
	2, saccess,			/* 33 = access */
	1, nice,			/* 34 = nice */
	1, ftime,			/* 35 = ftime; formerly sleep */
	0, sync,			/* 36 = sync */
	2, kill,			/* 37 = kill */
	4, select,			/* 38 = select */
	2, setpgrp,			/* 39 = setpgrp */
	2, lstat,			/* 40 = lstat ( was tell) */
	2, dup,				/* 41 = dup */
	0, pipe,			/* 42 = pipe */
	1, times,			/* 43 = times */
	4, profil,			/* 44 = prof */
	0, nosys,			/* 45 = formerly xmount */
	1, setgid,			/* 46 = setgid */
	0, getgid,			/* 47 = getgid */
	2, ssig,			/* 48 = sig */
	5, gmount,			/* 49 = new mount, all fstyps */
	0, nosys,			/* 50 = reserved for USG */
	1, sysacct,			/* 51 = turn acct off/on */
	3, sysphys,			/* 52 = set user physical addresses */
	1, syslock,			/* 53 = lock user in core */
	3, ioctl,			/* 54 = ioctl */
	1, reboot,			/* 55 = reboot */
	0, nosys,			/* 56 = formerly xunmount */
	2, symlink,			/* 57 = create symbolic link */
	3, readlink,			/* 58 = read symbolic link */
	3, exece,			/* 59 = exece */
	1, umask,			/* 60 = umask */
	1, chroot,			/* 61 = chroot */
	0, nosys,			/* 62 = reserved to local sites */
	0, nosys,			/* 63 = used internally */
#include "../sys/vmsysent.c"
};
