/*
FILE 
	idns-importarpaptrs.c
	svn ID removed
PURPOSE
	Standalone CLI mass arpa zone PTRs import tool.
LEGAL
	(C) Gary Wallis 2001-2012. All Rights Reserved.
	LICENSE file should be included in distribution.
*/
#include "../../mysqlrad.h"
#include "../../local.h"
#include <dirent.h>
#include <ctype.h>

MYSQL gMysql;
unsigned guMode=0;
unsigned guDebug=0;
unsigned guLine;
unsigned guClient=0;
unsigned guView=0;
unsigned guSOAZone=0;
unsigned guNSSet=0;
char gcQuery[1028];

//TOC
void ConnectDb(void);
unsigned uStrscan(char *cIn,char *cOut[]);
int SubmitJob(const char *cCommand, unsigned uNSSetArg, const char *cZoneArg,
				unsigned uPriorityArg, time_t luTimeArg);
int SubmitSingleJob(const char *cCommand,const char *cZoneArg, unsigned uNSSetArg,
		const char *cTargetServer, unsigned uPriorityArg, time_t luTimeArg
	       			,unsigned *uMasterJob);

int main(int iArgc, char *cArg[])
{

	if(iArgc!=6)
	{
		printf("usage :%s <import file spec> <tClient.uClient> <tView.cLabel>"
				" <SOA source tZone.uZone> <mode: test|commit|jobs optionally w/+debug>\n",cArg[0]);
		exit(0);
	}

	if(strstr(cArg[5],"test"))
	{
		guDebug=1;
		guMode=1;
	}
	else if(strstr(cArg[5],"commit"))
		guMode=3;
	else if(strstr(cArg[5],"jobs"))
		guMode=4;

	if(strstr(cArg[5],"debug"))
		guDebug=2;

	if(!guMode)
	{
		printf("Incorrect mode specified <mode: test|commit|jobs optionally w/+debug>\n");
		exit(1);
	}

	sscanf(cArg[2],"%u",&guClient);
	if(!guClient)
	{
		printf("Could not sscanf for guClient %s\n",cArg[2]);
		exit(1);
	}

	MYSQL_RES *res;
	MYSQL_ROW field;
	ConnectDb();

	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE uClient=%u",guClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(20);
	}
	res=mysql_store_result(&gMysql);
	guClient=0;
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&guClient);
	mysql_free_result(res);
	if(!guClient)
	{
		printf("No such guClient=%s\n",cArg[2]);
		exit(1);
	}

	sprintf(gcQuery,"SELECT uView FROM tView WHERE cLabel='%s'",cArg[3]);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(20);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&guView);
	mysql_free_result(res);
	if(!guView)
	{
		printf("No such cView=%s\n",cArg[3]);
		exit(1);
	}

	sscanf(cArg[4],"%u",&guSOAZone);
	if(!guSOAZone)
	{
		printf("Could not sscanf for guSOAZone%s\n",cArg[4]);
		exit(1);
	}

	//sprintf(gcQuery,"SELECT uZone,uNameServer FROM tZone WHERE uZone=%u",guSOAZone);
	sprintf(gcQuery,"SELECT uZone,uNSSet FROM tZone WHERE uZone=%u",guSOAZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(20);
	}
	res=mysql_store_result(&gMysql);
	guSOAZone=0;
	if((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&guSOAZone);
		sscanf(field[1],"%u",&guNSSet);
	}
	mysql_free_result(res);
	if(!guSOAZone)
	{
		printf("No such guSOAZone=%s\n",cArg[4]);
		exit(1);
	}

	printf("Starting import from file=%s uClient=%u guMode=%u guDebug=%u\n",cArg[1],guClient,guMode,guDebug);
	FILE *fp;

	if(!(fp=fopen(cArg[1],"r")))
	{
		printf("Error: Could not open: %s\n",cArg[1]);
		exit(4);
	}

	char cParam1[256]={""};
	char cParam2[256]={""};
	char cParam3[256]={""};
	char cParam4[256]={""};
	char cName[256]={""};
	char cZoneName[100]={""};
	char cPrevZoneName[100]={""};
	char cPrevcName[100]={""};
	unsigned uItems=0;
	unsigned uAddedSomething=0;
	unsigned uZone;
	unsigned uRRType=7;
	unsigned uTTL=0;
	unsigned uPrevTTL=0;//Has to be reset every new cZoneName

	while(fgets(gcQuery,1027,fp)!=NULL)
	{
		guLine++;
		//skip empty lines and comments
		if(gcQuery[0]=='\n' || gcQuery[0]==';')
			continue;

		//echo all lines
		if(guDebug==2) printf("%s",gcQuery);

		char *cOut[7]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};
		uItems=uStrscan(gcQuery,cOut);
		if(uItems==0)
		{
			if(guDebug==2) printf("uItems 0 case1\n");
			continue;
		}
		if(cOut[0]!=NULL)
			sprintf(cName,"%.255s",cOut[0]);
		if(cOut[1]!=NULL)
			sprintf(cParam1,"%.255s",cOut[1]);
		if(cOut[2]!=NULL)
			sprintf(cParam2,"%.255s",cOut[2]);
		if(cOut[3]!=NULL)
			sprintf(cParam3,"%.255s",cOut[3]);
		if(cOut[4]!=NULL)
			sprintf(cParam4,"%.255s",cOut[4]);
		if(guDebug==2) printf("cName=%s cParam1=%s cParam2=%s cParam3=%s cParam4=%s\n",cName,cParam1,cParam2,cParam3,cParam4);

		//TODO
		//Initial support must have IN and no TTLs
		if(strcasecmp(cParam2,"PTR"))
			continue;

		//Parse zone name
		unsigned a,b,c,d;
		sscanf(cName,"%u.%u.%u.%u.in-addr.arpa",&d,&c,&b,&a);
		sprintf(cZoneName,"%u.%u.%u.in-addr.arpa",c,b,a);

		static char cPrevOrigin[100]={""};
		if(strcmp(cZoneName,cPrevZoneName))
		{
			sprintf(cPrevZoneName,"%.99s",cZoneName);
			uPrevTTL=0;
			cPrevOrigin[0]=0;
			cPrevcName[0]=0;
			if(guDebug==2) printf("cZoneName=%s\n",cZoneName);
			//if(guMode==4 && uAddedSomething && uZone)
			if(guMode==4 && uZone)
			{
				time_t luClock;
	
				sprintf(gcQuery,"UPDATE tZone SET uSerial=uSerial+1,uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
							" WHERE uZone=%u",uZone);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					printf("%s\n",mysql_error(&gMysql));
					fclose(fp);
					exit(1);
				}
				time(&luClock);
				SubmitJob("New",guNSSet,FQDomainName(cZoneName),0,luClock);
				if(guDebug) printf("New job created for %s\n",cZoneName);
			}
			uAddedSomething=0;
		}
		if(guDebug==1 || guDebug==2) printf("%u.%s. IN PTR %s\n",d,cZoneName,cParam3);


		//If zone does not exist create. Add uZone if guMode>=commit
		if(guMode>2)
		{
			MYSQL_RES *res;
			MYSQL_ROW field;

			//Get uZone
			uZone=0;
			sprintf(gcQuery,"SELECT uZone FROM tZone WHERE uView=%u AND cZone='%s'",guView,cZoneName);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql)) 
			{
				printf("Error %s: %s\n",cZoneName,mysql_error(&gMysql));
				continue;
			}
			res=mysql_store_result(&gMysql);
			if((field=mysql_fetch_row(res)))
				sscanf(field[0],"%u",&uZone);
			mysql_free_result(res);

			if(!uZone)
			{
				//Requires MySQL>=5.1 for select from same table
				sprintf(gcQuery,"INSERT INTO tZone"
						" (cZone,uNSSet,uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
						"uMailServers,cMainAddress,uOwner,uCreatedBy,uCreatedDate,"
						"cHostmaster,uView,cOptions)"
						" SELECT '%s',uNSSet,CONVERT(DATE_FORMAT(NOW(),'%%Y%%m%%d00'),UNSIGNED),"
								"uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
						"uMailServers,cMainAddress,%u,1,UNIX_TIMESTAMP(NOW()),"
						"cHostmaster,%u,cOptions"
						" FROM tZone WHERE uZone=%u",
							FQDomainName(cZoneName),
							guClient,
							guView,
							guSOAZone);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					printf("INSERT tZone error %s\n",mysql_error(&gMysql));
					fclose(fp);
					exit(1);
				}
				uZone=mysql_insert_id(&gMysql);
				uAddedSomething++;
				if(guDebug) printf("Zone added %s\n",cZoneName);
			}
			else
			{
				//If RR exists skip to next line
				sprintf(gcQuery,"SELECT uResource FROM tResource WHERE uZone=%u AND ( cName='%u' OR "
						"cName='%u.%s.')"
						" AND uRRType=7"
						" AND cParam1='%s'",uZone,d,d,FQDomainName(cZoneName),FQDomainName(cParam3));
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					printf("Error %s: %s\n",cZoneName,mysql_error(&gMysql));
					continue;
				}
				res=mysql_store_result(&gMysql);
				if((field=mysql_fetch_row(res)))
				{
					mysql_free_result(res);
					continue;
				}
				mysql_free_result(res);
			}

			//Add uResource
			sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%u',uTTL=%u,uRRType=%u,"
					"cParam1='%s',"
					"cComment='idns-importarpaptrs',uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())"
					,uZone
					,d
					,uTTL
					,uRRType
					,FQDomainName(cParam3)
					,guClient);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql)) 
				printf("Error %s: %s\n",cZoneName,mysql_error(&gMysql));
			else
				uAddedSomething++;
				if(guDebug) printf("Resource added %u.%s.\n",d,cZoneName);

		}
	}

	//if(guMode==4 && uAddedSomething && uZone)
	if(guMode==4 && uZone)
	{
		time_t luClock;
	
		sprintf(gcQuery,"UPDATE tZone SET uSerial=uSerial+1,uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
					" WHERE uZone=%u",uZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
		{
			printf("%s\n",mysql_error(&gMysql));
			fclose(fp);
			exit(1);
		}
		time(&luClock);
		SubmitJob("New",guNSSet,FQDomainName(cZoneName),0,luClock);
		if(guDebug) printf("New job created for %s\n",cZoneName);
	}
	fclose(fp);
	return(0);
}//main()


void ConnectDb(void)
{
        mysql_init(&gMysql);
        if(!mysql_real_connect(&gMysql,DBIP0,DBLOGIN,DBPASSWD,DBNAME,DBPORT,DBSOCKET,0))
	{
		printf("Could not connect to MySQL server\n");
		exit(0);
	}

}//end of ConnectDb()


char *FQDomainName(char *cInput)
{
	register int i;

	for(i=0;cInput[i];i++)
	{
	
		if(!isalnum(cInput[i]) && cInput[i]!='.'  && cInput[i]!='-' && cInput[i]!='_' && cInput[i]!='@' && cInput[i]!='*')
			break;
		if(isupper(cInput[i])) cInput[i]=tolower(cInput[i]);
	}
	cInput[i]=0;

	return(cInput);

}//char *FQDomainName(char *cInput)


unsigned uStrscan(char *cIn,char *cOut[])
{
	unsigned register j=0,i,uFirstChar=1,uLen,uQuote=0;

	uLen=strlen(cIn);
	for(i=0;i<uLen;i++)
	{
		if(cIn[i]=='\"')
		{
			if(uQuote)
				uQuote=0;
			else
				uQuote=1;
		}

		if(!isspace(cIn[i]))
		{
			if(uFirstChar)
			{
				cOut[j++]=(char *)cIn+i;
				if(j>7) break;
				if(i>255) break;
				uFirstChar=0;
			}
		}
		else
		{
			if(!uQuote)
			{
				cIn[i]=0;
				uFirstChar=1;
			}
		}
	}

	cIn[i]=0;
	return(j);
}//unsigned uStrscan();


int SubmitJob(const char *cCommand, unsigned uNSSetArg, const char *cZoneArg,
				unsigned uPriorityArg, time_t luTimeArg)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	static unsigned uMasterJob=0;
	char cTargetServer[100];

	sprintf(gcQuery,"SELECT tNS.cFQDN,tNSType.cLabel FROM tNSSet,tNS,tNSType,tServer"
			" WHERE tNSSet.uNSSet=tNS.uNSSet AND tNS.uServer=tServer.uServer AND"
			" tNS.uNSType=tNSType.uNSType AND tNSSet.uNSSet=%u ORDER BY tNSType.uNSType",
			uNSSetArg);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("Error %s: %s\n",gcQuery,mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	
	while((field=mysql_fetch_row(res)))
	{
		//cTargetServer is really the target NS with the type qualification
		//Do not confuse with tServer based partions of zones and tNS NSs.
		sprintf(cTargetServer,"%.64s %.32s",field[0],field[1]);

		if(SubmitSingleJob(cCommand,cZoneArg,uNSSetArg,
				cTargetServer,uPriorityArg,luTimeArg,&uMasterJob))
		{
			printf("Error %s: %s\n",gcQuery,mysql_error(&gMysql));
			exit(1);
		}
	}//if field
	mysql_free_result(res);

	return(0);

}//int SubmitJob()


int SubmitSingleJob(const char *cCommand,const char *cZoneArg, unsigned uNSSetArg,
		const char *cTargetServer, unsigned uPriorityArg, time_t luTimeArg
	       			,unsigned *uMasterJob)
{
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uJob FROM tJob WHERE cJob='%s' AND cZone='%s' AND uNSSet=%u AND cTargetServer='%s'",
				cCommand,cZoneArg,uNSSetArg,cTargetServer);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		printf("Error %s: %s\n",gcQuery,mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	
	if(mysql_num_rows(res)==0)
	{
		unsigned uJob;

		sprintf(gcQuery,"INSERT INTO tJob SET cJob='%s',cZone='%s',"
				"uNSSet=%u,cTargetServer='%s',uPriority=%u,uTime=%lu,"
				"uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cCommand,
			cZoneArg,
			uNSSetArg,
			cTargetServer,
			uPriorityArg,
			luTimeArg,
			guClient);
		mysql_query(&gMysql,gcQuery);
		if(mysql_error(&gMysql)[0])
		{
			mysql_free_result(res);
			return(1);
		}

		if(*uMasterJob == 0)
		{
			uJob=*uMasterJob=mysql_insert_id(&gMysql);
			if(!strstr(cTargetServer,"MASTER"))
			{
				printf("Master must be first\n");
				exit(1);
			}
		}
		else
		{
			uJob=mysql_insert_id(&gMysql);
		}
	
		sprintf(gcQuery,"UPDATE tJob SET uMasterJob=%u WHERE uJob=%u",*uMasterJob,uJob);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
		{
			printf("Error %s: %s\n",gcQuery,mysql_error(&gMysql));
			exit(1);
		}
		if(mysql_affected_rows(&gMysql)==0)
		{
			if(guDebug==2 || guDebug==1);
				printf("uMasterJob %u",*uMasterJob);
		}
	}
	mysql_free_result(res);

	return(0);

}//int SubmitSingleJob()

