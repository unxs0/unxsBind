#!/bin/bash

#FILE
#	unxsBind/tools/dnssec/SignZone.sh
#AUTHOR/LEGAL
#	(C) 2010 Gary Wallis for Unixservice, LLC.
#	GPLv2 license applies.
#PURPOSE
#	unxsBind 2.0 will feature fully automated DNSSEC 
#	signing, zone key roll-over and key management.
#	Here we are just playing around with basic elements,
#	and eventually hardware crypto interfaces.
#
#	For a single zone Gen all keys, sign zone and reload 
#	a master DNSSEC named for testing only.

if [ "$1" == "" ] || [ "$2" == "" ];then
	echo "usage: $0 <zone> <zonefile>";
	exit 0;
fi

#basic sanity checks to avoid complex roll backs

if [ ! -f "$2" ];then
	echo "$2 not found!";
	exit 1;
fi

if [ -f "$2.signed" ];then
	echo "$2 already signed!";
	exit 1;
fi

grep "; $1" "$2" > /dev/null 2>&1;
if [ $? != 0 ];then
	echo "$1 zone does not match $2 error";
	exit 1;
fi

cKSKKey=`./KSKgen.sh $1`;
if [ $? != 0 ];then
	echo "KSKgen.sh $1 error";
	exit 1;
fi

cZSKKey=`./ZSKgen.sh $1`;
if [ $? != 0 ];then
	echo "ZSKgen.sh $1 error";
	exit 1;
fi

cp $2 $2.tmp;
if [ $? != 0 ];then
	echo "cp $2.tmp error";
	exit 1;
fi

cat $cKSKKey.key $cZSKKey.key >> $2.tmp;
if [ $? != 0 ];then
	echo "cat $cKSKKey $cZSKKey error";
	rm -f K$1*.key;
	rm -f K$1*.private;
	exit 1;
fi

#Note we are not incrementing the SOA serial number here. This is just a test script.

mv K$1*.private /usr/local/idns/keys/;
if [ $? != 0 ];then
	echo "mv K$1*.private /usr/local/idns/keys/ error";
	rm -f K$1*.key;
	rm -f K$1*.private;
	exit 1;
fi

mv K$1*.key /usr/local/idns/keys/;
if [ $? != 0 ];then
	echo "mv K$1*.key /usr/local/idns/keys/ error";
	rm -f K$1*.key;
	rm -f K$1*.private;
	exit 1;
fi

/usr/sbin/dnssec-signzone -o $1 -k /usr/local/idns/keys/$cKSKKey.key $2.tmp /usr/local/idns/keys/$cZSKKey.key;
if [ $? != 0 ];then
	echo "/usr/sbin/dnssec-signzone -D /usr/local/idns/keys/ -o $1 -k /usr/local/idns/keys/$cKSKKey $2.tmp /usr/local/idns/keys/$cZSKKey (error)";
	rm -f /usr/local/idns/keys/K$1*.key;
	rm -f /usr/local/idns/keys/K$1*.private;
	exit 1;
fi
#unexpected error leave keys for now
if [ ! -f $2.signed ];then
	echo "/usr/sbin/dnssec-signzone error2";
	#rm -f /usr/local/idns/keys/K$1*.key;
	#rm -f /usr/local/idns/keys/K$1*.private;
	exit 1;
fi

rm -f $2.tmp;

mv dsset-$1* /usr/local/idns/keys/;
if [ $? != 0 ];then
	echo "mv dsset-$1* /usr/local/idns/keys/ error";
	exit 1;
fi

mv keyset-$1* /usr/local/idns/keys/;
if [ $? != 0 ];then
	echo "mv keyset-$1* /usr/local/idns/keys/ error";
	exit 1;
fi


#Note we assume that the default rndc key is setup correctly here.
/usr/sbin/rndc reload > /dev/null 2>&1;
if [ $? != 0 ];then
	echo "rndc reload error";
	exit 1;
fi
exit 0;
