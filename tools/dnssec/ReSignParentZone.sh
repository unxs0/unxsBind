#!/bin/bash

#FILE
#	unxsBind/tools/dnssec/ReSignParentZone.sh
#AUTHOR/LEGAL
#	(C) 2010 Gary Wallis for Unixservice, LLC.
#	GPLv2 license applies.
#PURPOSE
#	unxsBind 2.0 will feature fully automated DNSSEC 
#	signing, zone key roll-over and key management.
#	Here we are just playing around with basic elements,
#	and eventually hardware crypto interfaces.
#
#	Re-sign a parent zone after adding child DS records.

if [ "$1" == "" ] || [ "$2" == "" ] || [ "$3" == "" ] || [ "$4" == "" ] || [ "$5" == "" ];then
	echo "usage: $0 <parent zone> <parent zonefile> <child zone> <child zone file> <RFC3658 NS>";
	exit 0;
fi

#basic sanity checks to avoid complex roll backs

if [ ! -f "$2" ];then
	echo "$2 must exist";
	exit 1;
fi

grep "IN DNSKEY 257" $2 > /dev/null 2>&1;
if [ $? == 0 ];then
	echo "$2 parent zone already prepared for resigning!";
	exit 1;
fi

if [ ! -f "$2.signed" ];then
	echo "$2.signed must exist";
	exit 1;
fi

if [ ! -f "$4.signed" ];then
	echo "$4.signed must exist";
	exit 1;
fi

grep "; $1" "$2" > /dev/null 2>&1;
if [ $? != 0 ];then
	echo "$1 parent zone does not match $2 error";
	exit 1;
fi

grep "$1" "$4" > /dev/null 2>&1;
if [ $? != 0 ];then
	echo "$1 parent zone not in $4 error";
	exit 1;
fi

if [ ! -f "/usr/local/idns/keys/dsset-$3." ];then
	echo "/usr/local/idns/keys/dsset-$3. must exist";
	exit 1;
fi

cKSK=`grep -l 257 /usr/local/idns/keys/K$1*`;
if [ $? != 0 ];then
	echo "cKSK error";
	exit 1;
fi
cZSK=`grep -l 256 /usr/local/idns/keys/K$1*`;
if [ $? != 0 ];then
	echo "cSZK error";
	exit 1;
fi

cp $2 $2.tmp;
if [ $? != 0 ];then
	echo "cp $2.tmp error";
	exit 1;
fi

cat $cKSK $cZSK >> $2.tmp;
if [ $? != 0 ];then
	echo "cat $cKSK $cZSKKey >> $2.tmp error";
	exit 1;
fi

#rfc3658 delegation point required  to avoid:
#	found DS RRset without NS RRset
echo "$3. NS $5." >> $2.tmp;
if [ $? != 0 ];then
	echo "cat /usr/local/idns/keys/dsset-$3. >> $2.tmp error";
	exit 1;
fi

cat /usr/local/idns/keys/dsset-$3. >> $2.tmp;
if [ $? != 0 ];then
	echo "cat /usr/local/idns/keys/dsset-$3. >> $2.tmp error";
	exit 1;
fi

#Note we are not incrementing the SOA serial number here. This is just a test script.

/usr/sbin/dnssec-signzone -o $1 -f $2.signed -k $cKSK $2.tmp $cZSK;
if [ $? != 0 ];then
	echo " /usr/sbin/dnssec-signzone -o $1 -f $2.signed -k $cKSK $2 $cZSK (error)";
	exit 1;
fi

rm -f dsset-$1.;
rm -f keyset-$1.;
rm -f $2.tmp;

#Note we assume that the default rndc key is setup correctly here.
/usr/sbin/rndc reload > /dev/null 2>&1;
if [ $? != 0 ];then
	echo "rndc reload error";
	exit 1;
fi
exit 0;
