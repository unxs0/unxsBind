#!/bin/bash
#
#FILE
#	common.sh
#PURPOSE
#	Provide common functions to the scripts in the test suite
#AUTHOR
#	(C) 2008. Hugo Urquiza for Unixservice.
#

cMysqlLogin="idns";
cMysqlPwd="wsxedc";
cMysqlDb="idns";

function ForeignKey
{
	#This function will try to retrieve a value from the specified table
	#Arguments are: (cTable,cField,cIndex,cValue)
	cTable=$1;
	cField=$2;
	cIndex=$3;
	cValue=$4;
	
	cQuery="SELECT $cField FROM $cTable WHERE $cIndex='$cValue'";
	echo $cQuery | mysql -u $cMysqlLogin -p$cMysqlPwd $cMysqlDb > /tmp/ForeignKey.result
	exec < /tmp/ForeignKey.result
	while read line
	do
		if [ "$line" != "$cField" ]; then
			echo $line;
		fi
	done
}

function ismLogin
{
	#This function implements the login into any (hopefully)
	#Unixservice interface or backend.
	#Arguments are (cLogin,cPasswd,cURL,cCookie)
	#All arguments are self explanatory but cCookie, which is the path 
	#where this function will store the session cookies

	cLogin=$1;
	cPasswd=$2;
	cURL=$3;
	cCookie=$4;
	
	if [ "$cLogin" = "" ] || [ "$cPasswd" = "" ] || [ "$cURL" = "" ] || [ "$cCookie" = "" ]; then
		echo "ismLogin(): Missing arguments";
		return;
	fi

	wget --no-check-certificate -q -O /dev/null --keep-session-cookies --save-cookies $cCookie \
		--post-data="gcLogin=$cLogin&gcPasswd=$cPasswd&gcFunction=Login" $cURL

	grep $cLogin $cCookie > /dev/null

	if [ "$?" = "0" ]; then
		echo "Logged in OK.";
	else
		rm $cCookie
		echo "Login failed. Aborting.";
		exit 1;
	fi

}

function ismLogout
{
	#This function implements the logout from any
	#Unixservice interface or backend.
	#Arguments are (cLogin,cURL,cCookie,uDelete=1|0)
	#See note for cCookie at ismLogin()
	#uDelete specifies if this function shall delete the cookie
	#file after logging out.

	cLogin=$1;
	cURL=$2;
	cCookie=$3
	uDelete=$4;
	
	if [ "$cURL" = "" ] || [ "$cCookie" = "" ]; then
		echo "ismLogout(): Missing arguments";
		return;
	fi

	#By default, delete the cookies file
	if [ "$uDelete" = "" ]; then uDelete=1; fi

	wget --no-check-certificate --keep-session-cookies --save-cookies $cCookie -q -O /dev/null --load-cookies $cCookie "$cURL?gcFunction=Logout";

	grep $cLogin $cCookie > /dev/null

	if [ "$?" = "1" ]; then
		echo "Logged out OK.";
	else
		echo "Logout failed.";
	fi
	
	if [ "$uDelete" = "1" ]; then
		rm $cCookie;
	fi
}
