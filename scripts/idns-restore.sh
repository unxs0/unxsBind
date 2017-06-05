#!/bin/bash

#FILE
#	/usr/sbin/idns-restore.sh
#PURPOSE
#	Provide a somewhat safe wrapper to
#	allow iDNS backend to
#	restore

echo "$(date +%Y%m%d%H%M%S) start $1" >> /tmp/idns-restore.sh.log
if [ -f "$1" ];then
	if [ -d /tmp/idns-restore.sh.lock ];then
		echo "$(date +%Y%m%d%H%M%S) end lock exists $1" >> /tmp/idns-restore.sh.log
		exit 2;
	else
		mkdir /tmp/idns-restore.sh.lock
		cat $1 | /usr/bin/gunzip - | /usr/bin/mysql -u idns -pwsxedc idns;
		if [ "$?" != "0" ];then
			echo "$(date +%Y%m%d%H%M%S) end fail $1" >> /tmp/idns-restore.sh.log
			rmdir /tmp/idns-restore.sh.lock;
			exit 3;
		else
			echo "$(date +%Y%m%d%H%M%S) end $1" >> /tmp/idns-restore.sh.log
		fi
		rmdir /tmp/idns-restore.sh.lock;
	fi
	exit 0;
else
	echo "$(date +%Y%m%d%H%M%S) end not file $1" >> /tmp/idns-restore.sh.log
fi
exit 1;
