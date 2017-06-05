#!/bin/bash
#FILE
#	compareSOA.sh
#PURPOSE
#	Run a SOA check for a list of zones against two NSs
#	Will inform of unmatched zone serial numbers.
#AUTHOR
#	(C) 2008 Hugo Urquiza for Unixservice.
#

#Target NSs
cNS1="ns1.isp.net";
cNS2="ns2.isp.net";

cZoneList="zones.txt";

while read cLine
do
	cSOA1=`dig @$cNS1 SOA $cLine +short | cut -d " " -f 3`;
	cSOA2=`dig @$cNS2 SOA $cLine +short | cut -d " " -f 3`;
	if [ "$cSOA1" -ne "$cSOA2" ]; then
		echo "$cLine serial number differs";
		#echo "cSOA1='$cSOA1'";
		#echo "cSOA2='$cSOA2'";
	fi
	
done < $cZoneList
