#!/bin/bash

#priority is by order in the hosts file
#so first we give priority to the dns specific zones
#then we add the old giant zone A records
/usr/sbin/unxsBindHosts callingcloud.net 2 '^callingcloud.net' > /etc/hosts.fromdb;
if [ $? != 0 ];then
	echo "error";
	exit 1;
fi
/usr/sbin/unxsBindHosts '^callingcloud.net' 2 >> /etc/hosts.fromdb;
if [ $? != 0 ];then
	echo "error";
	exit 1;
fi

if [ ! -f "/etc/hosts.fromdb" ];then
	echo "error";
	exit 1;
fi

#debug exit
exit 0;

cat /etc/hosts.0 /etc/hosts.fromdb > /etc/hosts;
