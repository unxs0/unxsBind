#!/bin/bash
#
#FILE
#	restorezones.sh
#PURPOSE
#	Provide a baseline test suite for the idnsAdmin interface
#	Restore Zones tab
#AUTHOR
#	(C) 2008 Hugo Urquiza for Unixservice.
#USAGE
#	./restorezones.sh <login> <password> <url>

#
#Check command line arguments
cLogin=$1;
cPasswd=$2;
cURL=$3;

if [ "$cLogin" = "" ] || [ "$cPasswd" = "" ] || [ "$cURL" = "" ]; then
	echo "Missing command line arguments.";
	echo "Usage:";
	echo "./restorezones.sh <login> <password> <url>";
	exit 1;
fi

#
#Login to the idnsAdmin interface
wget -q -o /dev/null --keep-session-cookies --save-cookies /tmp/idnsAdmin.cookies --post-data="gcLogin=$cLogin&gcPasswd=$cPasswd&gcFunction=Login" $3

grep $cLogin /tmp/idnsAdmin.cookies > /dev/null

if [ "$?" = "0" ]; then
	echo "Logged in OK.";
else
	rm /tmp/idnsAdmin.cookies
	echo "Login failed. Aborting.";
	exit 1;
fi

#
#Logout from the idnsAdmin interface
wget --keep-session-cookies --save-cookies /tmp/idnsAdmin.cookies -q -o /dev/null --load-cookies /tmp/idnsAdmin.cookies $3?gcFunction=Logout

grep $cLogin /tmp/idnsAdmin.cookies > /dev/null

if [ "$?" = "1" ]; then
	echo "Logged out OK.";
else
	echo "Logout failed.";
fi

#
#Cleanup
rm /tmp/idnsAdmin.cookies

	
