#!/bin/bash
#
#FILE
# $Id: importTemplates.sh 1335 2010-04-09 16:05:21Z Gary $
#PURPOSE
#	Import idnsOrg.cgi templates in one fell swoop.
#	Very useful when moving application to new server.
#	Or during development work with vi editing of templates.
#AUTHOR/LEGAL
#	(C) 2007-2009 Hugo Urquiza,Jonathan Petersson for Unixservice, LLC.
#	(C) 2010 Gary Wallis for Unixservice, LLC.
#	GPLv2 license applies. See included LICENSE file in source root dir.
#CONTRIBUTORS
#	Jonathan Petersson
#
if [ ! -z "$CGIDIR" ]; then
    if [ -e "${CGIDIR}iDNS.cgi" ]; then
    	for i in `find ./templates -maxdepth 1 -type f `; do
	  ${CGIDIR}iDNS.cgi ImportTemplateFile  `echo $i | cut -f 3 -d /`  ./$i plain "idnsOrg Interface";
    	done
    else
    	echo "iDNS.cgi isn't present in the CGIDIR you've defined!";
    fi
else
    echo "You haven't defined a CGIDIR, export CGIDIR=/cgi-bin/";
fi
