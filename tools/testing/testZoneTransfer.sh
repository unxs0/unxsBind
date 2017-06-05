#!/bin/bash
#FILE
#	testZoneTransfer.sh
#PURPOSE
#	Test iDNS zone transfer in an automated fashion
#AUTHOR
#	(C) 2008 Hugo Urquiza for Unixservice
#

cBackend=$1;
cLogin=$2;
cPasswd=$3;

#
#Test 1: Check login
wget --keep-session-cookies --save-cookies /tmp/teztZone.cookies --no-check-certificate --post-data "gcLogin=$cLogin&gcPasswd=$cPasswd&gcFunction=Login"\
	-O /tmp/loginTest.html https://$cBackend/cgi-bin/iDNS.cgi;

grep "logged in" /tmp/loginTest.html > /dev/null;

if [ "$?" = "0" ]; then
	echo "Logged in OK";
else
	echo "Login has failed. Test aborted.";
	exit 1;
fi

#
#Test 2: Create tZone record
wget --load-cookies /tmp/teztZone.cookies --no-check-certificate --post-data "gcCommand=Create New Reccord&gcFunction=tClientTools&cLabel=Hugo&cInfo=Test record&cEmail=support2@unixservice.com" -O - https://$cBackend/cgi-bin/iDNS.cgi;





