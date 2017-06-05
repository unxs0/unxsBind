#!/bin/bash
#
#FILE
#	$Id$
#PURPOSE
#	Import all unxsDNS.cgi templates in one fell swoop.
#AUTHOR/LEGAL
#	(C) 2010,2015 Gary Wallis for Unixservice, LLC.
#	GPLv2 license applies. See included LICENSE file in source root dir.
#
if [ -z "$CGIDIR" ];then
	CGIDIR="/var/www/unxs/cgi-bin/";
fi

if [ -e "${CGIDIR}iDNS.cgi" ]; then
    for cFile in `find ./templates.default -maxdepth 1 -type f`; do
    	if [[ $cFile != *.swp ]] && [[ $cFile != *.default ]];then
        	${CGIDIR}iDNS.cgi ImportTemplateFile  `echo $cFile | cut -f 3 -d /`  ./$cFile Plain "unxsDNS";
	fi
    done
else
    echo "iDNS.cgi isn't present in the CGIDIR you've defined!";
fi
