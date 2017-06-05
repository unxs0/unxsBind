#!/bin/bash
#FILE 
#	addtNameServer.sh
#PURPOSE
#	Test the tNameServer tab [New] function
#	verifying the code change at changeset [346]
#	as per ticket #42 request.
#AUTHOR
#	(C) 2008 Hugo Urquiza for Unixservice.
#USAGE
#	./addtNameServer.sh <login> <pwd> <idnsurl> <nameserverset>

source ./include/common.sh

#Check command line arguments
cLogin=$1;
cPasswd=$2;
cURL=$3;
cNSSet=$4;

if [ "$cLogin" = "" ] || [ "$cPasswd" = "" ] || [ "$cURL" = "" ] || [ "$cNSSet" = "" ]; then
        echo "Missing command line arguments.";
        echo "Usage:";
        echo "./addtNameServer.sh <login> <pwd> <idnsurl> <nameserverset>";
        exit 1;
fi

ismLogin $cLogin $cPasswd $cURL "/tmp/iDNS.cookies";

echo "Test 1: Add tNameServer record, cLabel='$cNSSet'";

cPost="gcFunction=tNameServerTools&gluRowid=1&gcCommand=Create+New+Record&uNameServer=1&cLabel=$cNSSet&cList=ns1.testns.com+MASTER%0D%0Ans2.testns.com+SLAVE&cMasterIPs=192.168.0.1&uOwner=0&uCreatedBy=0&uCreatedDate=0&uModBy=0&uModDate=0";

wget --load-cookies /tmp/iDNS.cookies --no-check-certificate -O /tmp/iDNS.tNameServer.out -q --post-data="$cPost" $cURL;

cMasterIPs=`ForeignKey "tNameServer" "cMasterIPs" "cLabel" "$cNSSet"`;

echo "$cMasterIPs" | grep ";" > /dev/null;

if [ "$?" = "0" ]; then
	echo "Test 1 OK";
else
	echo "Test 1 failed";
fi


ismLogout $cLogin $cURL "/tmp/iDNS.cookies";


