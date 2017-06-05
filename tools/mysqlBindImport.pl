#!/usr/bin/perl
#FILE
#	mysqlBindImport.pl
#PURPOSE
#	Connect to a mysqlBind database 
#	either local or remote and import 
#	zone and RRs data into the iDNS database.
#AUTHOR
#	(C) 2008 Hugo Urquiza for Unixservice.
#USAGE
#	./mysqlBindImport.pl [mysqlBindMySQLHost]
#


#
#Configuration section starts

#mysqlBind database login info
$cmysqlBindLogin='mysqlbind';
$cmysqlBindPwd='wsxedc';
$cmysqlBindDb='mysqlbind';

#iDNS database login info
$ciDNSLogin='idns';
$ciDNSPwd='wsxedc';
$ciDNSDb='idns';

#
#Configuration section ends

#------------------------------------------------------#
#Do not touch anything below this line; unless you know what you are doing ;)
#
use DBI;


$mysqlBindMySQLHost=$ARGV[1];

if($mysqlBindMySQLHost eq '')
{
	$mysqlBindMySQLHost="localhost";
}

#
#Open the MySQL vector to the mysqlBind database

my $dbmysqlBind=DBI->connect ("DBI:mysql:$cmysqlBindDb:$mysqlBindMySQLHost",$cmysqlBindLogin,$cmysqlBindPwd) or die DBI->errstr;

#
#Open the MySQL vector to the iDNS database
my $dbiDNS=DBI->connect ("DBI:mysql:$ciDNSDb:localhost",$ciDNSLogin,$ciDNSPwd) or die DBI->errstr;

#
#Import tZone data
$cQuery="SELECT uZone,cZone,uNameServer,cHostmaster,uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,uMailServers,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate FROM tZone ORDER BY uZone";


my $res=$dbmysqlBind->prepare($cQuery);
$res->execute() or die DBI->errstr;


while(@field=$res->fetchrow_array())
{
	$cQuery="INSERT INTO tZone SET uZone='$field[0]',cZone='$field[1]',cHostmaster='$field[2]',uSerial='$field[3]',uExpire='$field[4]',uRefresh='$field[5]',uTTL='$field[6]',uRetry='$field[7]',uZoneTTL='$field[8]',uMailServers='$field[9]',uOwner='$field[10]',uCreatedBy='$field[11]',uCreatedDate='$field[12]',uModBy='$field[13]',uModDate='$field[14]',uView=2";
	my $run=$dbiDNS->prepare($cQuery);
	$run->execute() or die DBI->errstr;
}

#
#Import tResource data
	

#$cQuery="SELECT uResource,uZone,cName,uTTL,uRRType,cParam1,cParam2,cComment,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate FROM tResource ORDER BY uResource";
#my $res=$dbmysqlBind->prepare($cQuery);
#$res->execute() or die DBI->errstr;

#while(@field=$res->fetchrow_array())
#{
#	$cQuery="INSERT INTO tResource SET 
#}
