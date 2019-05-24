#! /bin/sh
PATH=$PATH:DMDBIN
if ismux -
then	exec 32ld DMDALIB/tek4014.m $*
else	echo tek4014 does not run stand-alone.
fi
