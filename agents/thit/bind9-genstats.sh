#!/bin/bash

#
#FILE
#	unxsBind/agents/thit/bind9-genstats.sh
#PURPOSE
#	Gather DNS statistics via rndc, named-stats and then run our own
#	/usr/sbin/tHitCollector to parse and place data in MySQL db.
#NOTES


#Place next line in root crontab for iDNS tHit subsystem
#*/5 * * * * /usr/sbin/bind9-genstats.sh >> /var/log/unxsbindlog 2>&1

#unxsVZ standard log format
fLog() { echo "`date +%b' '%d' '%T` $0[$$]: $@"; }


#Configure
STAT_FILE=/usr/local/idns/named.d/named.stats;
RNDC="/usr/sbin/rndc -c /etc/unxsbind-rndc.conf";
THIT=/usr/sbin/tHitCollector;

#Gen named.stats
rm -f $STAT_FILE;
$RNDC stats;
RNDC_RET=$?;
if [ $RNDC_RET -ne 0 ]; then
	fLog "Error running $RNDC:$RNDC_RET";
	exit $RNDC_RET;
fi

#Load into tHit
$THIT;
THIT_RET=$?;
if [ $THIT_RET -ne 0 ]; then
	fLog "Error running $THIT:$THIT_RET";
	exit $THIT_RET;
fi

#Everything cool
exit 0;
