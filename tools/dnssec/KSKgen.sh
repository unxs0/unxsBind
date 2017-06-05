#!/bin/bash

#FILE
#	unxsBind/tools/dnssec/KSKgen.sh
#AUTHOR/LEGAL
#	(C) 2010 Gary Wallis for Unixservice, LLC.
#	GPLv2 license applies.
#PURPOSE
#	unxsBind 2.0 will feature fully automated DNSSEC 
#	signing, zone key roll-over and key management.
#	Here we are just playing around with basic elements,
#	and eventually hardware crypto interfaces.

if [ "$1" == "" ];then
	echo "usage: $0 <KSK zone (dnssec-keygen nametype)>";
	exit 0;
fi

cKeyname=`/usr/sbin/dnssec-keygen -f KSK -r /dev/urandom -a RSASHA1 -b 2048 -n ZONE $1`;
if [ $? != 0 ] || [ "$cKeyname" == "" ];then
	echo "/usr/sbin/dnssec-keygen error1";
	exit 1;
fi

if [ ! -f $cKeyname.key ];then
	echo "/usr/sbin/dnssec-keygen error2";
	exit 1;
fi

if [ ! -f $cKeyname.private ];then
	echo "/usr/sbin/dnssec-keygen error3";
	exit 1;
fi

echo $cKeyname;
exit 0;
