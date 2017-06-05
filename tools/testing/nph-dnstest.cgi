#!/bin/sh
# $Id: nph-dnstest.cgi 674 2008-12-19 15:10:07Z hus-admin $
#
# (C) 2002-2009 Gary Wallis. GPL License applies, see http://fsf.org
#
# Set these values !Watch the matched "" quotes


#set this variables acording to your configuration.
DEFAULTDOMAIN="yahoo.com"
DEFAULTDIGOPTION="short"

SERVER1="192.168.0.1"
TITLE1="192.168.0.1 (localhost)"

#SERVER2="192.168.0.2"
#TITLE2="192.168.0.2 (moonbeam)"
#See bottom

#Nothing should be needed to be modifed beneath this line
#Unless you add more server sections, which should be easy to see how to do.

PATH="/sbin:/usr/local/bin:/bin:/usr/bin:/usr/local/sbin:/usr/sbin"
export PATH

echo "HTTP/1.0 200 Transaction ok"
echo "Content-Type: text/html"
echo ""
echo ""
echo "<title>Test DNS Servers</title>"
echo "<body bgcolor=white><font face=arial,helvetica>"
echo "<h4>Test DNS Servers</h4>"
echo "<font size=1>Questions, comments contact: support@openisp.net<hr></font>"
echo "<pre><font face=courier>"
echo ""


#echo "$0 $1 $2"
#exit

if [ $1 .eq. ""]; then
	echo "other ways to use:"
	echo "nph-dnstest.cgi?optionaldomain.dom"
	echo "nph-dnstest.cgi?-x127.0.0.1"
	echo "nph-dnstest.cgi?-x127.0.0.1+digoption"
	echo "example nph-dnstest.cgi?openisp.net+verbose"
else
	DEFAULTDOMAIN="$1"
fi

if [ $2 .eq. ""]; then
	/bin/false
else
	DEFAULTDIGOPTION=$2
fi

echo ""
echo "Domain: $DEFAULTDOMAIN"
echo "Dig option: $DEFAULTDIGOPTION"

#Server section 1
echo ""
echo "$TITLE1"
dig @$SERVER1 $DEFAULTDOMAIN +$DEFAULTDIGOPTION 2>&1
#End server section 1

#Repeat for your slave name servers
#Server section 2
#echo ""
#echo "$TITLE2"
#dig @$SERVER2 $DEFAULTDOMAIN +$DEFAULTDIGOPTION 2>&1
#End server section 2


