#!/bin/bash
#
#FILE
#	contacts.sh
#PURPOSE
#	Provide a baseline test suite for the idnsAdmin interface
#	Contacts tab
#AUTHOR
#	(C) 2008 Hugo Urquiza for Unixservice.
#USAGE
#	./contacts.sh <login> <password> <url>

#
#Check command line arguments
cLogin=$1;
cPasswd=$2;
cURL=$3;

if [ "$cLogin" = "" ] || [ "$cPasswd" = "" ] || [ "$cURL" = "" ]; then
	echo "Missing command line arguments.";
	echo "Usage:";
	echo "./contacts.sh <login> <password> <url>";
	exit 1;
fi

#
#Login to the idnsAdmin interface
wget  --no-check-certificate -q -O /dev/null --keep-session-cookies --save-cookies /tmp/idnsAdmin.cookies --post-data="gcLogin=$cLogin&gcPasswd=$cPasswd&gcFunction=Login" $3

grep $cLogin /tmp/idnsAdmin.cookies > /dev/null

if [ "$?" = "0" ]; then
	echo "Logged in OK.";
else
	rm /tmp/idnsAdmin.cookies
	echo "Login failed. Aborting.";
	exit 1;
fi

#Test 1: Create a new contact

#Create the test company we are going to use (--nodelete so it keeps th company ;)
./companies.sh --nodelete $cLogin $cPasswd $cURL
#
#Get uClient value
grep "<input type=hidden name=uClient value=" /tmp/idnsAdmin.tmp > /tmp/idnsAdmin.data
uCompanyClient=`cat /tmp/idnsAdmin.data|  cut -f 4 -d = | cut -f 1 -d ">" `;

wget  --no-check-certificate -q -O /tmp/idnsAdmin.tmp --load-cookies /tmp/idnsAdmin.cookies \
--post-data="gcPage=CustomerUser&cClientName=TestContact&cEmail=testcontact@unixservice.com&cInfo=This is a test contact created by an automated script&cUserName=testContactLogin&cPassword=wsxedc&uSaveClrPassword=1&cuPerm=Organization Customer&uForClient=$uCompanyClient&gcFunction=Confirm New" $cURL

grep "New company contact created OK" /tmp/idnsAdmin.tmp

if [ "$?" = "0" ]; then
	echo "Test 1/?? passed";
else
	echo "Test 1/?? failed. I won't go ay further";
	exit 1;
fi

#Logout from the idnsAdmin interface
wget  --no-check-certificate --keep-session-cookies --save-cookies /tmp/idnsAdmin.cookies -q -O /dev/null --load-cookies /tmp/idnsAdmin.cookies $3?gcFunction=Logout

grep $cLogin /tmp/idnsAdmin.cookies > /dev/null

if [ "$?" = "1" ]; then
	echo "Logged out OK.";
else
	echo "Logout failed.";
fi

#
#Cleanup
rm /tmp/idnsAdmin.cookies

	
