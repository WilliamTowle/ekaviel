#!/bin/sh

config_set()
{
	VAR=$1
	shift
	SETVAL=$@

	[ -r config.mak ] && grep -v "^${VAR}=" config.mak > tmp.$$
	echo "${VAR}=${SETVAL}" >> tmp.$$
	mv tmp.$$ config.mak
}

config_get()
{
	VAR=$1
	if [ -r config.mak ] ; then
	    grep "^${VAR}=" config.mak | sed 's/^[^=]*= *//'
	else
	    echo ''
	fi
}

###	===== make =====
##	...probably /usr/bin/make (/usr/bin/gmake)
#
#[ -r `dirname $0`/detect.cache ] && . `dirname $0`/detect.cache

##	===== C compiler - GCC? =====

echo -n "Determining compiler... "

[ -z "$CC" ] || CC=`config_get CC`
if [ "${CC}" ] ; then
	echo "using ${CC}"
else
	CC=`which gcc 2>/dev/null`
	if [ "$CC" ] ; then
		echo "found gcc ($CC)"
	else
		CC=`which cc 2>/dev/null`
		if [ "$CC" ] ; then
			echo "found cc ($CC)"
		else
			echo "cannot be determined!"
			exit 1
		fi
	fi
fi

config_set CC ${CC}


echo -n "Determining linker... "

[ -z "$LD" ] && LD=`config_get LD`
if [ "${LD}" ] ; then
	echo "using ${LD}"
elif [ -z "$LD" ] ; then
	echo "using LD=CC (${CC})"
	LD=${CC}
fi

config_set LD ${LD}


##	===== Environment? =====

echo -n "Testing environment..."
case `uname -s` in
IRIX)
	EXEEXT=
	OSNAME=Irix
	;;
Linux|Linux)
	EXEEXT=
	OSNAME=Linux
	;;
*)
	# Amiga should have -DAMIGA set
	echo "Unknown environment"
esac
OSDEFS=-DOS_${OSNAME}
AUDPATH=aud${OSNAME}

##	===== Environment? =====

echo -n "Need for -lm... "
# May need to use -lncurses and/or -lcurses (Linux doesn't)

cat >tmp.c <<EOF
#include <math.h>
int main() { return log(2); }
EOF
OUTPUT=`${CC} tmp.c -o aouttmp${EXEEXT} 2>&1`
if [ -z "$OUTPUT" ] ; then
	echo "no"
	rm tmp.c aouttmp${EXEEXT} || exit 1
	LIBS=''
else
	OUTPUT=`${CC} tmp.c -lm -oaouttmp${EXEEXT} 2>&1`
	rm tmp.c aouttmp${EXEEXT}
	if [ "$OUTPUT" ] ; then
		echo "failed"
		echo $OUTPUT
		exit 1
	fi
	echo "yes"
	LIBS=-lm
fi


##	===== GENERATE CONFIG FILES =====

config_set TOPLEV `pwd`
config_set SRCROOT '${TOPLEV}/src'
config_set BLDROOT '${TOPLEV}/$(shell echo "build-"`uname -m`)'
config_set SCRIPTS '${TOPLEV}/scripts'
config_set AUDPATH ${AUDPATH}

config_set OSDEFS ${OSDEFS}
config_set AUDINCL -I'${SRCROOT}'/${AUDPATH}
config_set LIBS ${LIBS}

#sed 's/"//g' `dirname $0`/detect.cache > config.mak
SRCROOT=`pwd`/src
[ -r ${SRCROOT}/${AUDPATH}/Makefile.in ] || mv ${SRCROOT}/aud*/Makefile.in ${SRCROOT}/${AUDPATH}/Makefile.in

MSG=`basename $0`": Configure phase complete -- now \`make depend\`"
echo $MSG | sed 's/./=/g'
echo $MSG
echo $MSG | sed 's/./=/g'

grep -v '(shell' config.mak | sed '/ / s/=/="/' | sed '/="/ s/$/"/' > `dirname $0`/detect.cache
