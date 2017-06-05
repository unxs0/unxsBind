/*
FILE
	main.c
	svn ID removed
	/usr/sbin/idns-logerror
AUTHOR
	(C) 2006-2009, Gary Wallis for Unixservice USA
PURPOSE
	Read and follow the end of a logfile (tail -f style.)
	Add to master iDNS tLog named error messages of dashboard interest
NOTES
	The tLog.cServer is only for the first cluster member reporting the error.

SAMPLE LINE
	28-Jun-2007 21:06:05.270 xfer-in: transfer of 'e-exam.com/IN' from 213.52.215.2#53: failed to connect: timed out
	29-Jun-2007 00:53:50.687 xfer-in: transfer of 'emojo.is/IN' from 213.219.48.200#53: failed while receiving responses: REFUSED
TODO
*/

#include "../../mysqlrad.h"
#include <ctype.h>

//#define DBIP "localhost"

#define LOGFILE "/var/log/named-idns.log"
#define ERRLOG "/tmp/idns-logerror.log"

void daemonize(void);

int main(int iArgc, char *cArgv[])
{
	char cBuffer[512];
	register unsigned int i;
	char *cp,*cp2;
	char cZone[65];
	char cQuery[512];
	char cHostname[100]={""};
	MYSQL gMysql;
	MYSQL_RES *res;
	MYSQL_ROW field;
	MYSQL_RES *res2;
	MYSQL_ROW field2;
	FILE *ifp,*efp;
	char *cError="";
	char *cError1="failed to connect: timed out";
	char *cError2="failed while receiving responses:";

	if(iArgc==2 && !strncmp(cArgv[1],"--fg",4))
	{
		printf("Running %s in foreground\n",cArgv[0]);
		efp=stderr;
	}
	else if(iArgc==2 && !strncmp(cArgv[1],"--help",6))
	{
		printf("%s help\n\n--help\tthis page\n--fg\trun program in foreground mode\n",
				cArgv[0]);
		return(0);
	}
	else if(iArgc>=2)
	{
		printf("Usage: %s [--help | --fg]\n",cArgv[0]);
		return(0);
	}
	else if(1)
	{
		daemonize();
		if((efp=fopen(ERRLOG,"a"))==NULL)
		{
			fprintf(stderr,"Could not open error log!\n");
			return(3);
        	}
	}

	if((ifp=fopen(LOGFILE,"r"))==NULL)
	{
                fprintf(stderr,"Could not open log file!\n");
		return(2);
        }


        mysql_init(&gMysql);

	gethostname(cHostname,98);
	fprintf(efp,"%s started and mysql_init() ran ok.\ncHostname=%s\n",
			cArgv[0],cHostname);

	fseek(ifp,0,SEEK_END);
	while(1)
	{
		sleep(1);
		while(fgets(cBuffer,512,ifp))
		{
			cZone[0]=0;

			//This will eventually become a configurable if then else
			//structure for different errors we wish to log
			if(strstr(cBuffer,cError1))
			{
				cError=cError1;
			}
			else if(strstr(cBuffer,cError2))
			{
				cError=cError2;
			}
			else if(1)
			{
				break;
			}

			if((cp=strstr(cBuffer,"xfer-in: transfer of '")))
			{
				if((cp2=strchr(cp+22,'/')))
				{
					*cp2=0;
					sprintf(cZone,"%.64s",cp+22);
				}
			}

			for(i=0;cZone[i]&&i<65;i++)
			{
				if(isalnum(cZone[i]) || cZone[i]=='.' || cZone[i]=='-')
				{
					continue;
				}
				else
				{
					cZone[0]=0;
					break;
				}
			}

			if(cZone[0])
			{
        			if (!mysql_real_connect(&gMysql,DBIP0,DBLOGIN,
					DBPASSWD,DBNAME,0,NULL,0))
        			{
        				if (!mysql_real_connect(&gMysql,DBIP1,DBLOGIN,
						DBPASSWD,DBNAME,0,NULL,0))
			                		fprintf(efp,"Database server unavailable!\n");
					goto Return_Point;
        			}
				//debug only
				//printf("%s\n",cZone);
				//goto Return_Point;
				sprintf(cQuery,"SELECT uZone FROM tZone WHERE cZone='%.64s'",
					cZone);
				mysql_query(&gMysql,cQuery);
				if(mysql_errno(&gMysql))
				{
					fprintf(efp,"mySQL Error:%s\n",mysql_error(&gMysql));
					goto Return_Point;
				}
				
				res=mysql_store_result(&gMysql);
				if((field=mysql_fetch_row(res)))
				{
					sprintf(cQuery,"SELECT uLog FROM tLog WHERE uLogType=5 AND cLabel='%.64s'"
							" AND cMessage='xfer-in %s'\n",cZone,cError);
					mysql_query(&gMysql,cQuery);
					if(mysql_errno(&gMysql))
					{
						fprintf(efp,"mySQL Error:%s\n",
								mysql_error(&gMysql));
						goto Return_Point;
					}
					res2=mysql_store_result(&gMysql);
					if((field2=mysql_fetch_row(res2)))
					{
						sprintf(cQuery,"UPDATE tLog SET uPermLevel=uPermLevel+1,"
								"uModDate=UNIX_TIMESTAMP(NOW()),uModBy=1,"
								"cServer='%.64s' WHERE uLog=%s",cHostname,field2[0]);
						mysql_query(&gMysql,cQuery);
						if(mysql_errno(&gMysql))
						{
							fprintf(efp,"mySQL Error:%s\n",
									mysql_error(&gMysql));
							goto Return_Point;
						}
					}
					else
					{
						//uPermLevel as repeat counter
						sprintf(cQuery,"INSERT INTO tLog SET cLabel='%.64s',uLogType=5,"
								"cMessage='xfer-in %s',cServer='%.64s',uPermLevel=1,"
								"uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),"
								"cTableName='tZone',uTablePK=%s",
									cZone,cError,cHostname,field[0]);
						mysql_query(&gMysql,cQuery);
						if(mysql_errno(&gMysql))
						{
							fprintf(efp,"mySQL Error:%s\n",
									mysql_error(&gMysql));
							goto Return_Point;
						}
					}
					mysql_free_result(res2);
				}//if((field=mysql_fetch_row(res)))
				mysql_free_result(res);
			}//if(cZone[0])
			mysql_close(&gMysql);
		}//while(fgets(cBuffer,512,ifp))
	}//while(1)

Return_Point:
	fclose(ifp);
	fprintf(efp,"%s ended with a signal or error.\n",cArgv[0]);
	fclose(efp);
	mysql_close(&gMysql);
	return(0);

}//main()


void daemonize(void)
{
	switch(fork())
	{
		default:
			_exit(0);

		case -1:
			fprintf(stderr,"fork failed\n");
			_exit(1);

		case 0:
		break;
	}

	if(setsid()<0)
	{
		fprintf(stderr,"setsid failed\n");
		_exit(1);
	}

}//void daemonize(void)

