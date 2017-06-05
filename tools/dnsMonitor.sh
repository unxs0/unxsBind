#!/bin/bash
#
#FILE
#	dnsMonitor.sh
#	$Id$
#
#PURPOSE
#	A simple DNS server monitor that will send an email if 
#	$cDIG can't connect to the nameservers.
#	We also upgraded this tool to report the issue via the tLog system.
#AUTHOR
#	Hugo Urquiza for Unixservice. (C) 2007
#
#NOTES
#	This script may need to be slightly modified for each implementation
#	The most important update is the modification of the cDIG variable
#	If you run this script from any of the TCRB servers you must add the 
#	-b option to the dig command so it uses an external IP address as query
#	source. Otherwise, it will check against internal view.

#Setup vars
EMAIL="support@unixservice.com"
CC="support2@unixservice.com"
SUBJECT="DNS ALERT"
cDBNAME="idns"
cDBLOGIN="idns"
cDBPASSWD="wsxedc"
cDBIP="192.168.0.1"
cDIG="dig"

LogMessage()
{
	echo $cLogMsg;
	nice /usr/bin/mysql -u $cDBLOGIN -p$cDBPASSWD $cDBNAME <<EOF
INSERT INTO tLog SET cMessage='$cLogMsg - dnsMonitor',cServer='$cServer',uLogType=4,uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW());
EOF
}

$cDIG @192.168.0.1 unixservice.com +short > /dev/null 2>&1
if [ "$?" = "9" ] ; then 
	cLogMsg="ns1.unixservice.com Connect timeout";
	cServer="ns1.unixservice.com";
	LogMessage;
	echo "ns1.unixservice.com Connect timeout" | mail -s"$SUBJECT" $EMAIL;
	echo "ns1.unixservice.com Connect timeout" | mail -s"$SUBJECT"  $CC;
fi

$cDIG @192.168.0.2 unixservice.com +short > /dev/null 2>&1
if [ "$?" = "9" ] ; then 
	cLogMsg="ns2.unixservice.com Connect timeout";
	cServer="ns2.unixservice.com";
	LogMessage;
	echo "ns2.unixservice.com Connect timeout" | mail -s"$SUBJECT" $EMAIL;
	echo "ns2.unixservice.com Connect timeout" | mail -s"$SUBJECT"  $CC;
fi
