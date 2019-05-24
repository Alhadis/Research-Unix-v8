TERM=blit
PATH=:/usr/rob/bin:$HOME/bin$PATH
JPATH=:/usr/blit/demo/mpx
CDPATH=:$HOME
QEDFILE=/usr/rob/qed/q/qfile
mail=/usr/spool/mail
umask 0
PS1='j$ '
case ${REXEC=NO} in
NO)
	stty erase '^H' cr0 tabs nl0 ff0 bs0 -nl
	HISTORY=/tmp/$$
	>$HISTORY
	 chmod 600 $HISTORY
esac
export PATH HOME TERM QEDFILE mail CDPATH  HISTORY PS1 JPATH
