/*
FILE
	main.c (for tHitCollector exe)
	svn ID removed
AUTHOR
	(C) 2006-2012, Dylan and Gary for Unixservice USA - GPLv2 Applies
PURPOSE
	Process BIND9 named.stats and collect data for tHit table.
	Add data to rrd table and create graphs.
NOTES
	Limitations:
	1-. When NSs are restarted we need to handle wrap around correctly. That
	would be to add to the counters in this single case only.

SAMPLE-INPUT
	named.stats file:

	+++ Statistics Dump +++ (1225119901)
	success 57986358
	referral 8102751
	nxrrset 22136026
	nxdomain 7203075
	recursion 26459073
	failure 9103749
	success 256 0.0.10.in-addr.arpa external
	referral 0 0.0.10.in-addr.arpa external
	nxrrset 0 0.0.10.in-addr.arpa external
	nxdomain 55 0.0.10.in-addr.arpa external
	recursion 0 0.0.10.in-addr.arpa external
	failure 0 0.0.10.in-addr.arpa external
	success 10302 140.43.206.in-addr.arpa external
	referral 0 140.43.206.in-addr.arpa external
	nxrrset 10 140.43.206.in-addr.arpa external
	nxdomain 1219 140.43.206.in-addr.arpa external
	recursion 0 140.43.206.in-addr.arpa external
	failure 0 140.43.206.in-addr.arpa external
	...
	--- Statistics Dump --- (1225119901)


*/

#include "../../mysqlrad.h"

MYSQL gMysql;

unsigned TextConnectDb(void);

int main(int iArgc, char *cArgv[])
{
	char cZone[256]={"allzone.stats"};
	char cHost[101];
	char cPrevZone[256]={"allzone.stats"};
	FILE *fp;
	char cBuffer[512];
	char cQuery[512];
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cStatType[32];
	long unsigned luStat;
	long unsigned luHits=0;
	long unsigned luSuccess=0;
	long unsigned luReferral=0;
	long unsigned luNxrrset=0;
	long unsigned luNxdomain=0;
	long unsigned luRecursion=0;
	long unsigned luFailure=0;


	if(iArgc==4 && !strcmp(cArgv[1],"Initialize"))
	{
		cZone[0]=0;
		if(!strcmp(cArgv[2],"--cZone"))
			sprintf(cZone,"%.255s",cArgv[3]);
		if(!cZone[0])
		{
			printf("Initializing rrdtool db error! No --cZone supplied\n");
			exit(1);
		}

		printf("Initializing rrdtool db for %s\n",cZone);

		sprintf(cQuery,"rrdtool create /var/log/named/%s.rrd --start N --step 600"
				" DS:allhits:COUNTER:1200:U:U"
				" DS:success:COUNTER:1200:U:U"
				" DS:referral:COUNTER:1200:U:U"
				" DS:nxrrset:COUNTER:1200:U:U"
				" DS:nxdomain:COUNTER:1200:U:U"
				" DS:recursion:COUNTER:1200:U:U"
				" DS:failure:COUNTER:1200:U:U"
				" RRA:MAX:0.5:288:1440"
				" RRA:LAST:0.5:1:1440",cZone);
		if(system(cQuery))
		{
			printf("Initializing rrdtool db error!\n");
			exit(1);
		}
		exit(0);
	}
	else if(iArgc>=2 && !strcmp(cArgv[1],"AddData"))
	{
		cZone[0]=0;
		if(iArgc==4 && !strcmp(cArgv[2],"--cZone"))
			sprintf(cZone,"%.100s",cArgv[3]);
		if(!cZone[0])
		{
			printf("Initializing rrdtool db error! No --cZone supplied\n");
			exit(1);
		}

		if(TextConnectDb())
			exit(1);

		sprintf(cQuery,"SELECT SUM(uHitCount),SUM(uSuccess),SUM(uReferral),SUM(uNxrrset),"
				"SUM(uNxdomain),SUM(uRecursion),SUM(uFailure)"
				" FROM tHit WHERE cZone='%s' GROUP BY cZone",
						cZone);
		mysql_query(&gMysql,cQuery);
		if(mysql_errno(&gMysql))
		{
			printf("mySQL Error:%s\n",mysql_error(&gMysql));
			goto Return_Point;
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			sscanf(field[0],"%lu",&luHits);
			sscanf(field[1],"%lu",&luSuccess);
			sscanf(field[2],"%lu",&luReferral);
			sscanf(field[3],"%lu",&luNxrrset);
			sscanf(field[4],"%lu",&luNxdomain);
			sscanf(field[5],"%lu",&luRecursion);
			sscanf(field[6],"%lu",&luFailure);

			printf("Adding aggregated by cHost data to %s.rrd. luHits=%lu\n",cZone,luHits);

			sprintf(cQuery,"rrdtool update /var/log/named/%s.rrd"
				" N:%lu:%lu:%lu:%lu:%lu:%lu:%lu",
				cZone,
				luHits,luSuccess,luReferral,
				luNxrrset,luNxdomain,luRecursion,luFailure);
			if(system(cQuery))
			{
				printf("Adding data to %s.rrd error!\n",cZone);
				mysql_free_result(res);
				goto Return_Point;
			}
		}
		mysql_free_result(res);
		exit(0);
	}
	else if(iArgc>=2 && !strcmp(cArgv[1],"Graph"))
	{
		cZone[0]=0;
		if(iArgc==4 && !strcmp(cArgv[2],"--cZone"))
			sprintf(cZone,"%.100s",cArgv[3]);
		if(!cZone[0])
		{
			printf("Initializing rrdtool db error! No --cZone supplied\n");
			exit(1);
		}

		printf("Creating rrdtool graph %s\n",cZone);

		sprintf(cQuery,"rrdtool graph /var/log/named/%1$s.png"
				" --title=\"%1$s aggregated stats\""
				" --vertical-label=\"events per second\""
				" --base=1000"
				" --height=120"
				" --width=500"
				" --slope-mode"
			//	" --font TITLE:10:"
			//	" --font AXIS:6:"
			//	" --font LEGEND:8:"
			//	" --font UNIT:8:"
				" 'DEF:all=/var/log/named/%1$s.rrd:allhits:LAST'"
				" 'DEF:success=/var/log/named/%1$s.rrd:success:LAST'"
				" 'DEF:referral=/var/log/named/%1$s.rrd:referral:LAST'"
				" 'DEF:nxrrset=/var/log/named/%1$s.rrd:nxrrset:LAST'"
				" 'DEF:nxdomain=/var/log/named/%1$s.rrd:nxdomain:LAST'"
				" 'DEF:recursion=/var/log/named/%1$s.rrd:recursion:LAST'"
				" 'DEF:failure=/var/log/named/%1$s.rrd:failure:LAST'"
				" 'AREA:all#dddddd:all'"
				" 'LINE1:success#00ff00:success'"
				" 'LINE1:referral#00cc00:referral'"
				" 'LINE1:nxrrset#cc0000:nxrrset'"
				" 'LINE1:nxdomain#cccc00:nxdomain'"
				" 'LINE1:recursion#0000ff:recursion'"
				" 'LINE1:failure#ff0000:failure'"
				" 'GPRINT:all:MAX:Max values\\: all\\: %%2.2lf'"
				" 'GPRINT:success:MAX:success\\: %%2.2lf'"
				" 'GPRINT:referral:MAX:referral\\: %%2.2lf'"
				" 'GPRINT:nxrrset:MAX:nxrrset\\: %%2.2lf'"
				" 'GPRINT:nxdomain:MAX:nxdomain\\: %%2.2lf'"
				" 'GPRINT:recursion:MAX:recursion\\: %%2.2lf'"
				" 'GPRINT:failure:MAX:failure\\: %%2.2lf'",
					cZone);
		if(system(cQuery))
		{
			printf("Creating rrdtool graph error!\n");
			exit(1);
		}
		exit(0);
	}
	else if(iArgc>=2)
	{
		printf("Usage: %s [Initialize | AddData | Graph : All with --cZone <cZone in/for tHit>]\n"
			" With no args, individual NS named.stats processing."
			" All other options are only used on graph server.\n",
			cArgv[0]);
		exit(0);
		
	}
	//debug only
	//exit(0);

	if(gethostname(cHost,100)!=0)
	{
		printf("gethostname() failed: aborted\n");
		exit(1);
	}


	if(TextConnectDb())
		exit(1);

	//printf("%s started and mysql_init() and connect() ran ok.\n",cArgv[0]);

	if((fp=fopen("/usr/local/idns/named.d/named.stats","r"))==NULL)
	{
                printf("Could not open named.stats\n");
		goto Return_Point;
        }

	while(fgets(cBuffer,512,fp))
	{
		cStatType[0]=0;
		luStat=0;

		//Skip top and bottom lines
		if(cBuffer[0]=='+')
			continue;

		//Ignores external internal
		sscanf(cBuffer,"%s %lu %s",cStatType,&luStat,cZone);

		if(cBuffer[0]=='-')
			sprintf(cZone,"End");

		if(!strcmp(cZone,cPrevZone))
		{
			//debug only
			//printf("%s,%lu,%s\n",cStatType,luStat,cZone);

			//Add all stats here per zone
			luHits+=luStat;
			//Only one per zone
			if(!strcmp(cStatType,"success"))
				luSuccess=luStat;
			else if(!strcmp(cStatType,"referral"))
				luReferral=luStat;
			else if(!strcmp(cStatType,"nxrrset"))
				luNxrrset=luStat;
			else if(!strcmp(cStatType,"nxdomain"))
				luNxdomain=luStat;
			else if(!strcmp(cStatType,"recursion"))
				luRecursion=luStat;
			else if(!strcmp(cStatType,"failure"))
				luFailure=luStat;
		}
		else
		{

			//debug only
			//printf("%s:\nluHits=%lu\nluSuccess=%lu\nluReferral=%lu\nluNxrrset=%lu\nluNxdomain=%lu\n"
			//	"luRecursion=%lu\nluFailure=%lu\n\n",cPrevZone,
			//	luHits,
			//	luSuccess,
			//	luReferral,
			//	luNxrrset,
			//	luNxdomain,
			//	luRecursion,
			//	luFailure);
			//goto Skip_Here;
			//end debug only

			sprintf(cQuery,"SELECT uHit FROM tHit WHERE cZone='%s' AND cHost='%s'",
					cPrevZone,cHost);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
			{
				printf("mySQL Error:%s\n",mysql_error(&gMysql));
				goto Return_Point;
			}
			res=mysql_store_result(&gMysql);
			if(mysql_num_rows(res) && (field=mysql_fetch_row(res)))
			{
				//named.stats does not reset except on NS restart.
				sprintf(cQuery,"UPDATE tHit SET uHitCount=%lu,uModDate=UNIX_TIMESTAMP(NOW()),"
						"uSuccess=%lu,"
						"uReferral=%lu,"
						"uNxrrset=%lu,"
						"uNxdomain=%lu,"
						"uRecursion=%lu,"
						"uFailure=%lu,"
						"uModBy=1 WHERE uHit=%s",
							luHits,
							luSuccess,luReferral,luNxrrset,luNxdomain,luRecursion,luFailure,
								field[0]);
				mysql_query(&gMysql,cQuery);
				if(mysql_errno(&gMysql))
				{
					printf("mySQL Error:%s\n",mysql_error(&gMysql));
					mysql_free_result(res);
					goto Return_Point;
				}
			}
			else
			{
				MYSQL_RES *res2;
				MYSQL_ROW field2;
				unsigned uOwner=1;

				//If tZone record was deleted but named.stats has the old zone
				//we still insert but owner is root.
				sprintf(cQuery,"SELECT uOwner FROM tZone WHERE cZone='%s' LIMIT 1",cPrevZone);
				mysql_query(&gMysql,cQuery);
				if(mysql_errno(&gMysql))
				{
					printf("mySQL Error:%s\n",mysql_error(&gMysql));
					goto Return_Point;
				}
				res2=mysql_store_result(&gMysql);
				if((field2=mysql_fetch_row(res2)))
					sscanf(field2[0],"%u",&uOwner);
				mysql_free_result(res2);

				sprintf(cQuery,"INSERT INTO tHit SET cZone='%s',uHitCount=%lu,uOwner=%u,"
						"uSuccess=%lu,"
						"uReferral=%lu,"
						"uNxrrset=%lu,"
						"uNxdomain=%lu,"
						"uRecursion=%lu,"
						"uFailure=%lu,"
						"cHost='%.100s',"
						"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							cPrevZone,luHits,uOwner,
							luSuccess,luReferral,luNxrrset,
							luNxdomain,luRecursion,luFailure,cHost);
				mysql_query(&gMysql,cQuery);
				if(mysql_errno(&gMysql))
				{
					printf("mySQL Error:%s\n",mysql_error(&gMysql));
					mysql_free_result(res);
					goto Return_Point;
				}
			}
			mysql_free_result(res);

//debug only
//Skip_Here:
			sprintf(cPrevZone,"%.255s",cZone);

			//zone changed we need to read the first line of new zone
			//	again since we skipped assignment above.
			sscanf(cBuffer,"%s %lu %s",cStatType,&luStat,cZone);

			//debug only
			//printf("%s,%lu,%s\n",cStatType,luStat,cZone);

			luHits=luStat;//reset
			if(!strcmp(cStatType,"success"))
				luSuccess=luStat;
			else if(!strcmp(cStatType,"referral"))
				luReferral=luStat;
			else if(!strcmp(cStatType,"nxrrset"))
				luNxrrset=luStat;
			else if(!strcmp(cStatType,"nxdomain"))
				luNxdomain=luStat;
			else if(!strcmp(cStatType,"recursion"))
				luRecursion=luStat;
			else if(!strcmp(cStatType,"failure"))
				luFailure=luStat;
		}
	}
	mysql_close(&gMysql);
	return(0);

Return_Point:
	mysql_close(&gMysql);
	return(1);

}//main()
