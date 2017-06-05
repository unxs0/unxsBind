#!/bin/bash
#
#FILE
#	companies.sh
#PURPOSE
#	Provide a baseline test suite for the idnsAdmin interface
#	Companies tab
#AUTHOR
#	(C) 2008 Hugo Urquiza for Unixservice.
#USAGE
#	./companies.sh [--nodelete] <login> <password> <url>

#
#Check command line arguments
if [ "$1" = "--nodelete" ]; then
	uNoDelete=1;
	cLogin=$2;
	cPasswd=$3;
	cURL=$4;
else
	cLogin=$1;
	cPasswd=$2;
	cURL=$3;
fi

if [ "$cLogin" = "" ] || [ "$cPasswd" = "" ] || [ "$cURL" = "" ]; then
	echo "Missing command line arguments.";
	echo "Usage:";
	echo "./companies.sh <login> <password> <url>";
	exit 1;
fi

#
#Login to the idnsAdmin interface
echo;
echo "Login to the idnsAdmin interface...";

wget  --no-check-certificate --keep-session-cookies --save-cookies /tmp/idnsAdmin.cookies --post-data="gcLogin=$cLogin&gcPasswd=$cPasswd&gcFunction=Login" $cURL
grep $cLogin /tmp/idnsAdmin.cookies > /dev/null

if [ "$?" = "0" ]; then
	echo "Logged in OK.";
else
	rm /tmp/idnsAdmin.cookies
	echo "Login failed. Aborting.";
	exit 1;
fi

#Test 1: Add new company
echo;
echo "Test 1/6: Add new company";

wget  --no-check-certificate -q --load-cookies /tmp/idnsAdmin.cookies \
--post-data="cLabel=TestScript Company&cEmail=testcompany@unixservice.com&cCode=COMPTEST&cInfo=Additional Info goes here&gcPage=Customer&gcFunction=Confirm New" -O /tmp/idnsAdmin.tmp $cURL

grep "New Company created OK" /tmp/idnsAdmin.tmp > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 1/6 passed";
else
	echo "Test 1/6 failed. I won't go any further";
	exit 1;
fi
	
#Test 2: Search company
echo;
echo "Test 2/6: Search company";

wget  --no-check-certificate -q --load-cookies /tmp/idnsAdmin.cookies \
--post-data="cSearch=TestScript&gcFunction=Search&gcPage=Customer" -O /tmp/idnsAdmin.tmp $cURL

grep "TestScript Company" /tmp/idnsAdmin.tmp > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 2/6 passed";
else
	echo "Test 2/6 failed. I won't go any further";
	exit 1;
fi

#Test 3: Modify company
echo;
echo "Test 3/6: Modify company";
#
#Get uClient value
grep "<input type=hidden name=uClient value=" /tmp/idnsAdmin.tmp > /tmp/idnsAdmin.data
uClient=`cat /tmp/idnsAdmin.data|  cut -f 4 -d = | cut -f 1 -d ">" `;

if [ "$uClient" = "" ]; then
	echo "Something went wrong. I could not determine the value of the uClient variable from the HTML output";
	echo "I won't go any further";
	exit 1;
fi

wget  --no-check-certificate -q --load-cookies /tmp/idnsAdmin.cookies \
--post-data="uClient=$uClient&cLabel=TestScript Comp. Modified&cEmail=testcompany@openisp.net&cCode=COMPTEST&cInfo=This record was modified by the automated test script&gcFunction=Confirm Modify&gcPage=Customer" -O /tmp/idnsAdmin2.tmp $cURL

grep "Company modified OK" /tmp/idnsAdmin2.tmp > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 3/6 passed";
else
	echo "Test 3/6 failed. I won't go any further";
	exit 1;
fi

#Test 4: Search company again to check record modification
echo;
echo "Test 4/6: Search company again to check record modification";

wget  --no-check-certificate -q --load-cookies /tmp/idnsAdmin.cookies \
--post-data="cSearch=TestScript&gcFunction=Search&gcPage=Customer" -O /tmp/idnsAdmin3.tmp $cURL

grep "TestScript Comp. Modified" /tmp/idnsAdmin3.tmp > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 4/6 passed";
else
	echo "Test 4/6 failed. I won't go any further";
	exit 1;
fi

if [ "$uNoDelete" = 1 ]; then
	echo "--nodelete option specified. Skipping deletion tests";
	exit 0;
fi

#Test 5: Delete the company
echo;
echo "Test 5/6: Delete the company we just created and modified";

wget  --no-check-certificate -q --load-cookies /tmp/idnsAdmin.cookies \
--post-data="uClient=$uClient&gcFunction=Confirm Delete&gcPage=Customer" -O /tmp/idnsAdmin2.tmp $cURL

grep "Company deleted OK" /tmp/idnsAdmin2.tmp > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 5/6 passed";
else
	echo "Test 5/6 failed. I won't go any further";
	exit 1;
fi

#Test 6: Verify company deletion using get hook
echo;
echo "Test 6/6: Verify company deletion using get hook";

wget  --no-check-certificate -q --load-cookies /tmp/idnsAdmin.cookies -O /tmp/idnsAdmin2.tmp "$cURL?gcPage=Customer&uClient=$uClient"

grep "No records found." /tmp/idnsAdmin2.tmp > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 6/6 passed";
else
	echo "Test 6/6 failed. I won't go any further";
	exit 1;
fi

#Logout from the idnsAdmin interface
wget  --no-check-certificate --keep-session-cookies --save-cookies /tmp/idnsAdmin.cookies -q -O /dev/null --load-cookies /tmp/idnsAdmin.cookies $cURL?gcFunction=Logout

grep $cLogin /tmp/idnsAdmin.cookies > /dev/null

if [ "$?" = "1" ]; then
	echo "Logged out OK.";
else
	echo "Logout failed.";
fi

#
#Cleanup
rm /tmp/idnsAdmin.cookies
rm /tmp/idnsAdmin2.tmp
rm /tmp/idnsAdmin3.tmp
	
