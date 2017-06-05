#!/bin/bash
#
#FILE
#	delCompany.sh
#PURPOSE
#	Provide a test script for the delete company function
#	Companies tab
#AUTHOR
#	(C) 2008 Hugo Urquiza for Unixservice.
#USAGE
#	./delCompany.sh <login> <password> <url> <company>

#Include common functions
source ./include/common.sh

#
#Check command line arguments
cLogin=$1;
cPasswd=$2;
cURL=$3;
cCompany=$4;

if [ "$cLogin" = "" ] || [ "$cPasswd" = "" ] || [ "$cURL" = "" ] || [ "$cCompany" = "" ]; then
	echo "Missing command line arguments.";
	echo "Usage:";
	echo "./delCompany.sh <login> <password> <url> <company>";
	exit 1;
fi


#Ask for user confirmation
echo "Are you sure you want to delete the company $cCompany. This will also affect company contacts, zones and their RRs. There's NO UNDO (Y/N)";
choice="N";
read choice
if [ "$choice" = "N" ] || [ "$choice" = "n" ]; then
	echo "Ok. No deletion test will be performed.";
	exit 0;
fi

#
#Login to the idnsAdmin interface
wget --no-check-certificate -q -O /dev/null --keep-session-cookies --save-cookies /tmp/idnsAdmin.cookies --post-data="gcLogin=$cLogin&gcPasswd=$cPasswd&gcFunction=Login" $cURL

grep $cLogin /tmp/idnsAdmin.cookies > /dev/null

if [ "$?" = "0" ]; then
	echo "Logged in OK.";
else
	rm /tmp/idnsAdmin.cookies
	echo "Login failed. Aborting.";
	exit 1;
fi


uClientCompany=`ForeignKey "tClient" "uClient" "cLabel" $cCompany`;

if [ "$uClientCompany" = "" ]; then
	echo "Oops! It seems the company you are trying to delete does not exist!";
	echo "I'll quit,";
	exit 1;
fi

uZone=`ForeignKey "tZone" "uZone" "uOwner" $uClientCompany`;
uResource=`ForeignKey "tResource" "uResource" "uOwner" $uClientCompany`;
uClient=`ForeignKey "tClient" "uClient" "uOwner" $uClientCompany`;

#
#Determine what to check after company deletion
#

if [ "$uZone" != "" ]; then
	uHasZone=1;
else
	uHasZone=0;
fi

if [ "$uResource" != "" ]; then
	uHasResource=1;
else
	uHasResource=0;
fi

if [ "$uClient" != "" ]; then
	uHasContact=1;
else
	uHasContact=0;
fi


#Prepare POST data
cPost="uClient=$uClientCompany&gcFunction=Confirm Delete&gcPage=Customer";

#echo $cPost;
#
#Submit delete command
wget --no-check-certificate -q -O /tmp/idnsAdmin.tmp --load-cookies /tmp/idnsAdmin.cookies --post-data="$cPost" $cURL

grep "Company deleted OK" /tmp/idnsAdmin.tmp > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 1/1 passed";
else
	echo "Test 1/1 failed. I won't go any further";
	exit;
fi

if [ "$uHasZone" = "1" ]; then
	echo "Company had zones, verifying if they were deleted";
	uZone=`ForeignKey "tZone" "uZone" "uOwner" $uClientCompany`;
	if [ "$uZone" = "" ]; then
		echo "Test 1.1 passed. Zones were deleted";
	else
		echo "Test 1.1 failed. Zones seem to be still at the database";
	fi
else
	echo "Company didn't have zones (Test 1.1 skipped)";
fi

if [ "$uHasResource" = "1" ]; then
	echo "Company zones had RRs, verifying if they were deleted";
	uResource=`ForeignKey "tResource" "uResource" "uOwner" $uClientCompany`;
	if [ "$uResource" = "" ]; then
		echo "Test 1.2 passed. Zone RRs were deleted";
	else
		echo "Test 1.2 failed. Zone RRs seem to be still at the database";
	fi
else
	echo "Company didn't have zone RRs (Test 1.2 skipped)";
fi

if [ "$uHasContact" = "1" ]; then
	echo "Company had contacts, verifying if they were deleted";
	uClient=`ForeignKey "tClient" "uClient" "uOwner" $uClientCompany`;
	if [ "$uClient" = "" ]; then
		echo "Test 1.3 passed. Company contacts were deleted";
	else
		echo "Test 1.3 failed. Company contacts seem to be still at the database";
	fi
else
	echo "Company didn't have contacts (Test 1.3 skipped)";
fi

#
#Logout from the idnsAdmin interface
wget --no-check-certificate --keep-session-cookies --save-cookies /tmp/idnsAdmin.cookies -q -O /dev/null --load-cookies /tmp/idnsAdmin.cookies "$cURL?gcFunction=Logout"

grep $cLogin /tmp/idnsAdmin.cookies > /dev/null

if [ "$?" = "1" ]; then
	echo "Logged out OK.";
else
	echo "Logout failed.";
fi

#
#Cleanup
rm /tmp/idnsAdmin.cookies

	
