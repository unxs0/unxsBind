#!/bin/bash
#
#FILE
#	addCompanyAndRelated.sh
#PURPOSE
#	Provide a test script for adding a company and 
#	its cotacts, zones and RRs
#	Companies tab
#AUTHOR
#	(C) 2008 Hugo Urquiza for Unixservice.
#USAGE
#	./addCompanyAndRelated.sh <login> <password> <url> <company>

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
	echo "./addCompanyAndRelated.sh <login> <password> <url> <company>";
	exit 1;
fi


#
#Check if the company exists
uClientCompany=`ForeignKey "tClient" "uClient" "cLabel" $cCompany`;

if [ "$uClientCompany" != "" ]; then
	echo "Oops! The company you are trying to create seems to exist. uClientCompany=$uClientCompany";
	echo "I won't go no further.";
	exit 1;
fi

#Ask for user confirmation
echo "Are you sure you want to add the company $cCompany. This will also create a test contact, and a test webzone (including RRs) (Y/N)";
choice="N";
read choice
if [ "$choice" = "N" ] || [ "$choice" = "n" ]; then
	echo "Ok. No add test will be performed.";
	exit 0;
fi

#
#Login to the idnsAdmin interface

ismLogin $cLogin $cPasswd $cURL "/tmp/idnsAdmin.cookies";

#Left by now as example use
#uClientCompany=`ForeignKey "tClient" "uClient" "cLabel" $cCompany`;

#Create new company

#Test 1: Add new company
echo;
echo "Test 1/5: Add new company";

cPost="cLabel=$cCompany&cEmail=testcompany@unixservice.com&cCode=COMPTEST&cInfo=Additional Info goes here&gcPage=Customer&gcFunction=Confirm New";

#debug only
#echo "POST request=$cPost";

wget  --no-check-certificate -q --load-cookies /tmp/idnsAdmin.cookies --post-data="$cPost" -O /tmp/idnsAdmin.tmp $cURL

grep "New Company created OK" /tmp/idnsAdmin.tmp > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 1/5 passed";
else
	echo "Test 1/5 failed. I won't go any further";
	exit 1;
fi
	
#Test 2: Search company
echo;
echo "Test 2/5: Search company";

wget  --no-check-certificate -q --load-cookies /tmp/idnsAdmin.cookies \
--post-data="cSearch=$cCompany&gcFunction=Search&gcPage=Customer" -O /tmp/idnsAdmin.tmp $cURL

grep $cCompany /tmp/idnsAdmin.tmp > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 2/5 passed";
else
	echo "Test 2/5 failed. I won't go any further";
	exit 1;
fi

#
#Get uClient value
grep "<input type=hidden name=uClient value=" /tmp/idnsAdmin.tmp > /tmp/idnsAdmin.data
uClient=`cat /tmp/idnsAdmin.data|  cut -f 4 -d = | cut -f 1 -d ">" `;

if [ "$uClient" = "" ]; then
	echo "Something went wrong. I could not determine the value of the uClient variable from the HTML output";
	echo "I won't go any further";
	exit 1;
fi

#debug only
#echo "uClient=$uClient";

#Create a contact
echo;
echo "Test 3/5: Add a contact for the $cCompany company";

cPost="cCustomer=$cCompany&gcPage=CustomerUser&cClientName=$cCompany.TestContact&cEmail=$cCompany.TestContact%40isp.net&cInfo=Additional+info+of+the+contact+goes+here.%0D%0AIt+can+have+newlines%2C+etc.%0D%0A%0D%0AAnd+blank+lines%2C+of+course&cUserName=$cCompany.TestContactlogin&cPassword=&cPassword=wsxedc&uPerm=0&cuPerm=Organization+Customer&uForClient=$uClient&gcFunction=Confirm+New";

wget  --no-check-certificate -q -O /tmp/idnsAdmin.tmp --load-cookies /tmp/idnsAdmin.cookies --post-data="$cPost" $cURL;

grep "Company contact created OK" /tmp/idnsAdmin.tmp  > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 3/5 passed";
else
	echo "Test 3/5 failed. I won't go any further";
	exit 1;
fi

#Create a zone
echo;
echo "Test 4/5: Add a zone for the $cCompany company";

cPost="gcPage=Zone&uNameServer=1&cZone=$cCompany.com&uExpire=604800&uRefresh=10800&uTTL=86400&uRetry=3600&uZoneTTL=86400&cHostmaster=nsadmin.unixservice.com&cView=external&uSecondaryOnly=0&uForClient=$uClient&gcFunction=Confirm New";

wget  --no-check-certificate -q -O /tmp/idnsAdmin.tmp --load-cookies /tmp/idnsAdmin.cookies --post-data="$cPost" $cURL;

#
#NOTE: After zone creation, the zone is loaded, so we don't see the 'Zone Created' message no more
#actually, that is used by the interface itself to determine if it has to submit a job for iDNS
#What to search in the output for then? Easy, {{cZone}} Resource Records at the bottom table
#if we got that, it means we created a zone OK ;)
#
grep "$cCompany.com Resource Records" /tmp/idnsAdmin.tmp > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 4/5 passed";
else
	echo "Test 4/5 failed. I won't go any further";
	exit 1;
fi

uZone=`ForeignKey "tZone" "uZone" "cZone" $cCompany.com`;

if [ "$uZone" = "" ]; then
	echo "Oops! Something went wrong. I could not determine the value of uZone for the zone I just created.";
	echo "I won't go any further";
	exit 1;
fi


#Create its RRs
echo;
echo "Test 5/5: Add a RR to the zone I just created";

#Create @ IN A 1.2.3.4
cPost="gcPage=Resource&uView=2&cNavList=No+results.&cZone=$cCompany.com&uResource=0&uNameServer=1&cName=%40&uTTL=&cRRType=A&cParam1=1.2.3.4&cComment=Created+by+addCompanyAndRelated.sh&gcFunction=New+Confirm";

wget  --no-check-certificate -q -O /tmp/idnsAdmin.tmp --load-cookies /tmp/idnsAdmin.cookies --post-data="$cPost" $cURL;

grep "Zone Resource Created" /tmp/idnsAdmin.tmp > /dev/null

if [ "$?" = "0" ]; then
	echo "Test 5/5 passed";
else
	echo "Test 5/5 failed. I won't go any further";
	exit 1;
fi


#
#Logout from the idnsAdmin interface
ismLogout $cLogin $cURL "/tmp/idnsAdmin.cookies" "1";

	
