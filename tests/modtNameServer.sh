#!/bin/bash
#FILE 
#	addtNameServer.sh
#PURPOSE
#	Test the tNameServer tab [Modify] function
#	verifying the code change at changeset [346]
#	as per ticket #42 request.
#AUTHOR
#	(C) 2008 Hugo Urquiza for Unixservice.
#USAGE
#	./modtNameServer.sh <login> <pwd> <idnsurl> <nameserverset>

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

uNameServer=`ForeignKey "tNameServer" "uNameServer" "cLabel" "$cNSSet"`;

if [ "$uNameServer" = "" ]; then
	echo "There's no tNameServer record with the specified cLabel value: $cNSSet";
	ismLogout $cLogin $cURL "/tmp/iDNS.cookies";
	exit 1;
else
	echo "Are you sure you want to modify the tNameServer record $cNSSet? This could affect in production data (Y/N)";
	choice="Y";
	read choice
	if [ "$choice" = "N" ] || [ "$choice" = "n" ]; then
		ismLogout $cLogin $cURL "/tmp/iDNS.cookies";
		exit 0;
	fi
fi

echo "Test 1: Modify tNameServer record, cLabel='$cNSSet'";

cPost="gcFunction=tNameServerTools&gluRowid=1&gcCommand=Confirm+Modify&uNameServer=6&cLabel=$cNSSet&cList=ns1.testns.com+MASTER%0D%0Ans2.testns.com+SLAVE%0D%0A&cMasterIPs=192.168.10.30&uOwner=1&uCreatedBy=1&uCreatedDate=1217952435&uModBy=0&uModDate=0";

wget --load-cookies /tmp/iDNS.cookies --no-check-certificate -O /tmp/iDNS.tNameServer.out -q --post-data="$cPost" $cURL;

cMasterIPs=`ForeignKey "tNameServer" "cMasterIPs" "cLabel" "$cNSSet"`;

echo "$cMasterIPs" | grep ";" > /dev/null;

if [ "$?" = "0" ]; then
	echo "Test 1 OK";
else
	echo "Test 1 failed";
fi


ismLogout $cLogin $cURL "/tmp/iDNS.cookies";


