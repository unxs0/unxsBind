#!/bin/sh
#
#FILE
#	mysqlcluster.sh
#	$Id: mysqlcluster.sh 103 2009-05-28 19:42:18Z Hugo $
#
#PURPOSE
#	Connect to all members of a MySQL replication cluster and check slave status.
#	If any member is behind set a warning image for possible use in web dashboard.
#	Other actions that could be taken:
#		1-. Send an email to a responsable party.
#		2-. Insert into idns.tLog a system error message.
#REQUIREMENTS
#	Passwordless ssh must be enabled to all external hosts (ssh-keygen.)
#	Ths script is run from a crontab at least every 30 mins
#	Must install the images in this dir into /var/www/unxs/html/images/ 
#

#
#Modify the line below with your MyQSL root user password.
PASSWD="ultrasecret"

if [ "$1" == "" ] || [ "$2" == "" ];then
	echo "usage: $0 <host1> <host2>"
	exit 0;
fi



#local server
echo "show slave status\G" | /usr/bin/mysql -p$PASSWD | grep "Seconds_Behind_Master: 0" > /dev/null;
if [ $? == 0 ];then
	uLocalStatus="0";
else
	uLocalStatus="1";
fi

#server1
ssh $1 "echo 'show slave status\G' | /usr/bin/mysql -p$PASSWD | grep 'Seconds_Behind_Master: 0'" > /dev/null;
if [ $? == 0 ];then
	uHost1Status="0";
else
	uHost1Status="1";
fi

#server2
ssh $2 "echo 'show slave status\G' | /usr/bin/mysql -p$PASSWD | grep 'Seconds_Behind_Master: 0'" > /dev/null;
if [ $? == 0 ];then
	uHost2Status="0";
else
	uHost2Status="1";
fi

if [ "$uLocalStatus" == "0" ] && [ "$uHost1Status" == "0" ] && [ "$uHost2Status" == "0" ];then

#	echo "ok";
	cp /var/www/unxs/html/images/green.gif /var/www/unxs/html/images/mrcstatus.gif;
else
#	echo "fail";
	cp /var/www/unxs/html/images/red.gif /var/www/unxs/html/images/mrcstatus.gif;
fi
