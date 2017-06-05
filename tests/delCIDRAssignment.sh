#!/bin/bash
#FILE
#	delCIDRAssignment.sh
#PURPOSE
#	Test the CIDR Assignment Tool (Removal)
#	tBlock tab
#AUTHOR
#	(C) 2008 Hugo Urquiza for Unixservice
#USAGE
#	./delCIDRAssignment.sh <login> <password> <url>
#

source ./include/common.sh

#Check command line arguments
cLogin=$1;
cPasswd=$2;
cURL=$3;

if [ "$cLogin" = "" ] || [ "$cPasswd" = "" ] || [ "$cURL" = "" ]; then
        echo "Missing command line arguments.";
        echo "Usage:";
        echo "./addCIDRAssignment.sh <login> <password> <url>";
        exit 1;
fi

ismLogin $cLogin $cPasswd $cURL "/tmp/iDNS.cookies";

echo "Test 1/2 CIDR math check";

#Test 1, check maths
cPost="cForClientPullDown=Root&cIPBlock=192.168.0.1/24&gcCommand=Remove Assigned IP Block&gcFunction=tBlockTools&uMathCheck=1";
wget --load-cookies /tmp/iDNS.cookies --no-check-certificate -O /tmp/iDNS.tBlock.out -q --post-data="$cPost" $cURL;

#Test 1 parse output
#Expected:
#	Number of networks: 1
#	Number of IPs: 254

uNetNum=`grep "Number of networks" /tmp/iDNS.tBlock.out | cut -d : -f 2`;
uIPsNum=`grep "Number of IPs" /tmp/iDNS.tBlock.out | cut -d : -f 2`;
#echo "uNetNum=$uNetNum";
#echo "uIPsNum=$uIPsNum";

if [ "$uNetNum" = "1" ] & [ "$uIPsNum" = "254" ]; then
	echo "Test 1.a passed";
else
	echo "Test 1.a failed. Aborting";
	exit 1;
fi

cPost="cForClientPullDown=Root&cIPBlock=192.168.0.1/23&gcCommand=Remove Assigned IP Block&gcFunction=tBlockTools&uMathCheck=1";
wget --load-cookies /tmp/iDNS.cookies --no-check-certificate -O /tmp/iDNS.tBlock.out -q --post-data="$cPost" $cURL;

uNetNum=`grep "Number of networks" /tmp/iDNS.tBlock.out | cut -d : -f 2`;
uIPsNum=`grep "Number of IPs" /tmp/iDNS.tBlock.out | cut -d : -f 2`;

if [ "$uNetNum" = "2" ] & [ "$uIPsNum" = "510" ]; then
	echo "Test 1.b passed";
else
	echo "Test 1.b failed. Aborting";
	exit 1;
fi

cPost="cForClientPullDown=Root&cIPBlock=192.168.0.1/22&gcCommand=Remove Assigned IP Block&gcFunction=tBlockTools&uMathCheck=1";
wget --load-cookies /tmp/iDNS.cookies --no-check-certificate -O /tmp/iDNS.tBlock.out -q --post-data="$cPost" $cURL;

uNetNum=`grep "Number of networks" /tmp/iDNS.tBlock.out | cut -d : -f 2`;
uIPsNum=`grep "Number of IPs" /tmp/iDNS.tBlock.out | cut -d : -f 2`;

if [ "$uNetNum" = "4" ] & [ "$uIPsNum" = "1022" ]; then
	echo "Test 1.c passed";
else
	echo "Test 1.c failed. Aborting";
	exit 1;
fi

cPost="cForClientPullDown=Root&cIPBlock=192.168.0.1/32&gcCommand=Remove Assigned IP Block&gcFunction=tBlockTools&uMathCheck=1";
wget --load-cookies /tmp/iDNS.cookies --no-check-certificate -O /tmp/iDNS.tBlock.out -q --post-data="$cPost" $cURL;

uNetNum=`grep "Number of networks" /tmp/iDNS.tBlock.out | cut -d : -f 2`;
uIPsNum=`grep "Number of IPs" /tmp/iDNS.tBlock.out | cut -d : -f 2`;

if [ "$uNetNum" = "1" ] & [ "$uIPsNum" = "1" ]; then
	echo "Test 1.d passed";
else
	echo "Test 1.d failed. Aborting";
	exit 1;
fi

echo "It seems the CIDR math is working";

#
#Now let's test if the tool actually does something
echo;
echo "Test 2/2 Remove CIDR block assigment";

uZone=`ForeignKey "tZone" "uZone" "cZone" "0.168.192.in-addr.arpa"`;
uBlock=`ForeignKey "tBlock" "uBlock" "cLabel" "192.168.0.0/24"`;

if [ "$uZone" = "" ] | [ "$uBlock" = "" ]; then
	echo "Test 2 failed. There is missing data.";
	echo "Further information:";
	echo "uZone=$uZone";
	echo "uBlock=$uBlock";
	exit 1;
fi

cPost="cForClientPullDown=Root&cIPBlock=192.168.0.1/24&gcCommand=Remove Assigned IP Block&gcFunction=tBlockTools";
wget --load-cookies /tmp/iDNS.cookies --no-check-certificate -O /tmp/iDNS.tBlock.out -q --post-data="$cPost" $cURL;

#A /24 assignment has to:
#1. Create a tBlock record
#2. Create a tZone record

uZone=`ForeignKey "tZone" "uZone" "cZone" "0.168.192.in-addr.arpa"`;
uBlock=`ForeignKey "tBlock" "uBlock" "cLabel" "192.168.0.0/24"`;


if [ "$uZone" = "" ] & [ "$uBlock" = "" ]; then
	echo "Test 2 seems OK. Please review uZone and uBlock values below (They should be blank):";
	echo "uZone=$uZone";
	echo "uBlock=$uBlock";
else
	echo "Test 2 failed. Either the tZone or tBlock record was not deleted.";
	echo "Further information:";
	echo "uZone=$uZone";
	echo "uBlock=$uBlock";
fi

ismLogout $cLogin $cURL "/tmp/iDNS.cookies";

