/*
FILE 
	idns-importzones.c
	svn ID removed
PURPOSE
	Standalone CLI mass zone import tool.
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
unsigned guLine;
unsigned guClient=0;
unsigned guView=0;
unsigned guNameServer=0;
char gcQuery[1028];
typedef struct {
	unsigned uSerial;
	unsigned uTTL;
	unsigned uRefresh;
	unsigned uRetry;
	unsigned uExp;
	unsigned uNegTTL;
	char cHostmaster[100];
	char cNameServer[100];
} structSOA;

//TOC
void ConnectDb(void);
structSOA *ProcessSOA(FILE *fp);
void ProcessRRLine(char *cLine,char *cZoneName,const unsigned uZone,const unsigned uCustId,const unsigned uNameServer);
unsigned uStrscan(char *cIn,char *cOut[]);
int SubmitJob(const char *cCommand, unsigned uNameServerArg, const char *cZoneArg,
				unsigned uPriorityArg, time_t luTimeArg);
int SubmitSingleJob(const char *cCommand,const char *cZoneArg, unsigned uNameServerArg,
		const char *cTargetServer, unsigned uPriorityArg, time_t luTimeArg
	       			,unsigned *uMasterJob);

int main(int iArgc, char *cArg[])
{

	register unsigned n,i;
	char cMasterIPs[100]={""};

#ifdef TESTSTRSCAN
	char *cOut[7]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	sprintf(gcQuery,"1 2 \"three\" \"four four 4 4\" skdjfh skh ksdfh");
	printf("%s\n",gcQuery);
	n=uStrscan(gcQuery,cOut);
	for(i=0;i<n;i++)
		if(cOut[i]!=NULL) printf("%u: %s\n",i,cOut[i]);
	exit(0);
#endif

	if(iArgc!=6)
	{
		printf("usage :%s <import dir path> <tClient.uClient> <tView.cLabel>"
				" <tNameServer.cLabel> <mode: test|debug|commit|commitwjobs>\n",cArg[0]);
		exit(0);
	}

	if(strcmp(cArg[5],"test")==0)
		guMode=1;
	else if(strcmp(cArg[5],"debug")==0)
		guMode=2;
	else if(strcmp(cArg[5],"commit")==0)
		guMode=3;
	else if(strcmp(cArg[5],"commitwjobs")==0)
		guMode=4;

	if(!guMode)
	{
		printf("Incorrect mode specified <mode: test|debug|commit|commitwjobs>\n");
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

	sprintf(gcQuery,"SELECT uNameServer,cMasterIPs FROM tNameServer WHERE cLabel='%s'",cArg[4]);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(20);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&guNameServer);
		sprintf(cMasterIPs,"%.99s",field[1]);
	}
	mysql_free_result(res);
	if(!guNameServer)
	{
		printf("No such tNameServer.cLabel=%s\n",cArg[4]);
		exit(1);
	}
	if(!cMasterIPs[0])
		printf("!Warning no cMasterIPs for selected tNameServer set!\n");

	printf("Starting import dir=%s uClient=%u guMode=%u\n",cArg[1],guClient,guMode);
	struct dirent **direntNamelist;
		
	n=scandir(cArg[1],&direntNamelist,0,0);
	if(n<0)
	{
		printf("scandir() error: Does %s exist?\n",cArg[1]);
		exit(2);
	}
	else if(n==2)
	{
		printf("No files found.\n");
		exit(3);
	}
	for(i=0;i<n;i++)
	{

		if(direntNamelist[i]->d_name[0]=='.')
		{
			;
		}
		else
		{
			FILE *fp;
			char cZone[101]={""};
			structSOA *importSOA;

			guLine=0;

			sprintf(gcQuery,"%s/%s",cArg[1],direntNamelist[i]->d_name);
			if(!(fp=fopen(gcQuery,"r")))
			{
				printf("Error: Could not open: %s\n",gcQuery);
				exit(4);
			}

			//Process
			//Remove standard suffixes
			if(!strncmp(direntNamelist[i]->d_name+strlen(direntNamelist[i]->d_name)-3,".db",3))
				direntNamelist[i]->d_name[strlen(direntNamelist[i]->d_name)-3]=0;
			sprintf(cZone,"%.100s",FQDomainName(direntNamelist[i]->d_name));
			if(guMode<3) printf("\n%s for zone %s\n",gcQuery,cZone);

			//Major import component
			importSOA=ProcessSOA(fp);

			//Add more rule based checking of TTLs	
			//if(	importSOA->uTTL &&
			//allow 0 TTL
			if(
				importSOA->uSerial &&
				importSOA->uRefresh &&
				importSOA->uRetry &&
				importSOA->uExp &&
				importSOA->uNegTTL &&
				importSOA->cHostmaster[0] )
			{
				unsigned uZone=0;

				//Import zone
				importSOA->cNameServer[strlen(importSOA->cNameServer)-1]=0;
				importSOA->cHostmaster[strlen(importSOA->cHostmaster)-1]=0;

				if(guMode>=3)
				{
					//Allow repeat import
					//We need to remove any possible RRs first
					sprintf(gcQuery,"DELETE FROM tResource WHERE"
							" uZone=(SELECT uZone FROM tZone WHERE cZone='%s' AND uOwner=%u AND uView=%u)",
								cZone,guClient,guView);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql)) 
					{
						printf("%s\n",mysql_error(&gMysql));
						fclose(fp);
						exit(1);
					}
					//Then we can remove the zone if it exists
					sprintf(gcQuery,"DELETE FROM tZone WHERE cZone='%s' AND uOwner=%u AND uView=%u",
								cZone,guClient,guView);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql)) 
					{
						printf("%s\n",mysql_error(&gMysql));
						fclose(fp);
						exit(1);
					}

					//uZoneTTL is the NegTTL
					//allow-transfer { key allslaves-master.;}; HACK FIX ASAP TODO
					sprintf(gcQuery,"INSERT INTO tZone SET cZone='%s',uNameServer=%u,"
						"uSerial=CONVERT(DATE_FORMAT(NOW(),'%%Y%%m%%d00'),UNSIGNED),uExpire=%u,"
						"uRefresh=%u,uTTL=%u,uRetry=%u,uZoneTTL=%u,uMailServers=0,cMainAddress='0.0.0.0',"
						"uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),uModBy=0,uModDate=0,"
						"cHostmaster='%s',uView=%u,"
						"cOptions='allow-transfer { key allslaves-master.;};',"
						"cMasterIPs='%s'",
							cZone,
							guNameServer,
							//importSOA->uSerial,
							importSOA->uExp,
							importSOA->uRefresh,
							importSOA->uTTL,
							importSOA->uRetry,
							importSOA->uNegTTL,
							guClient,
							importSOA->cHostmaster,guView,cMasterIPs);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql)) 
					{
						printf("%s\n",mysql_error(&gMysql));
						fclose(fp);
						exit(1);
					}
					uZone=mysql_insert_id(&gMysql);
				}

				while(fgets(gcQuery,1027,fp)!=NULL)
				{
					guLine++;
				  	//skip empty lines
				  	if(gcQuery[0]!='\n' || gcQuery[0]!=';')
						ProcessRRLine(gcQuery,cZone,uZone,guClient,guNameServer);
				}

				if(guMode>=4)
				{
					time_t luClock;

					sprintf(gcQuery,"UPDATE tZone SET uSerial=uSerial+1,uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
								" WHERE cZone='%.255s'",cZone);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql)) 
					{
						printf("%s\n",mysql_error(&gMysql));
						fclose(fp);
						exit(1);
					}
					time(&luClock);
					SubmitJob("New",guNameServer,cZone,0,luClock);
				}
			}
			else
			{
				printf("Error %s: ProcessSOA() failed!\n",cZone);
			}
			fclose(fp);
		}
	}
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


structSOA *ProcessSOA(FILE *fp)
{
	static structSOA importSOA;
	register unsigned uIn=0;
	register unsigned uParamCount=0;
	char *cp;

	importSOA.uSerial=0;
	importSOA.uTTL=86400;
	importSOA.uRefresh=0;
	importSOA.uRetry=0;
	importSOA.uExp=0;
	importSOA.uNegTTL=0;
	importSOA.cHostmaster[0]=0;
	importSOA.cNameServer[0]=0;

	while(fgets(gcQuery,1024,fp)!=NULL)
	{
		guLine++;

		if((cp=strchr(gcQuery,';')))
		{
			*cp='\n';
			*(cp+1)=0;
		}

		if(strstr(gcQuery,"("))
		{
			if((cp=strstr(gcQuery,"SOA")) || (cp=strstr(gcQuery,"soa")))
				sscanf(cp+4,"%s %s",importSOA.cNameServer,
						importSOA.cHostmaster);
			uIn=1;
			continue;
		}
		else if(strstr(gcQuery,")"))
		{
			if(uIn && uParamCount==4)
				sscanf(gcQuery,"%u",&importSOA.uNegTTL);
			return(&importSOA);
		}
		else if(strstr(gcQuery,"$TTL"))
			sscanf(gcQuery,"$TTL %u",&importSOA.uTTL);
		else if((cp=strstr(gcQuery,"SOA")) || (cp=strstr(gcQuery,"soa")))
			sscanf(cp+4,"%s %s",importSOA.cNameServer,
						importSOA.cHostmaster);

		if(uIn)
		{
			if(!uParamCount)
				sscanf(gcQuery,"%u",&importSOA.uSerial);
			else if(uParamCount==1)
				sscanf(gcQuery,"%u",&importSOA.uRefresh);
			else if(uParamCount==2)
				sscanf(gcQuery,"%u",&importSOA.uRetry);
			else if(uParamCount==3)
				sscanf(gcQuery,"%u",&importSOA.uExp);
			else if(uParamCount==4)
				sscanf(gcQuery,"%u",&importSOA.uNegTTL);
			uParamCount++;
		}

		if(uParamCount>5)
			return(&importSOA);
	}
	
	return(&importSOA);

}//structSOA *ProcessSOA(FILE *fp)


void ProcessRRLine(char *cLine,char *cZoneName,const unsigned uZone,const unsigned uCustId,const unsigned uNameServer)
{
	char cName[100]={""};
	char cNamePlus[200]={""};
	char cParam1[256]={""};
	char cParam2[256]={""};
	char cParam3[256]={""};
	char cParam4[256]={""};
	char cParam5[256]={""};
	char cType[100]={""};
	static char cPrevZoneName[100]={""};
	static char cPrevcName[100]={""};
	unsigned uRRType=0;
	unsigned uItems=0;
	static unsigned uPrevTTL=0;//Has to be reset every new cZoneName
	unsigned uTTL=0;
	static char cPrevOrigin[100]={""};
	char *cp;

	if(strcmp(cZoneName,cPrevZoneName))
	{
		sprintf(cPrevZoneName,"%.99s",cZoneName);
		uPrevTTL=0;
		cPrevOrigin[0]=0;
		cPrevcName[0]=0;
	}
	
	if((cp=strchr(cLine,';')))
	{
		*cp='\n';
		*(cp+1)=0;
	}


	//debug only
	if(guMode==2) printf("cLine=%s",cLine);
	if(!cLine[0] || cLine[0]==';') return;

	char *cOut[7]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	if(cLine[0]=='\t' || cLine[0]==' ')
	{
		//uItems=sscanf(cLine,"%99s %255s %255s %255s %255s %255s\n",
		//	cType,cParam1,cParam2,cParam3,cParam4,cParam5);
		uItems=uStrscan(cLine,cOut);
		if(uItems==0)
		{
			if(guMode==2) printf("uItems 0 case1\n");
			return;
		}
		if(cOut[0]!=NULL)
			sprintf(cType,"%.99s",cOut[0]);
		if(cOut[1]!=NULL)
			sprintf(cParam1,"%.255s",cOut[1]);
		if(cOut[2]!=NULL)
			sprintf(cParam2,"%.255s",cOut[2]);
		if(cOut[3]!=NULL)
			sprintf(cParam3,"%.255s",cOut[3]);
		if(cOut[4]!=NULL)
			sprintf(cParam4,"%.255s",cOut[4]);
		if(cOut[5]!=NULL)
			sprintf(cParam5,"%.255s",cOut[5]);
		if(cPrevcName[0])
			sprintf(cName,"%.99s",cPrevcName);
		else	
			strcpy(cName,"\t");
	}
	else if(cLine[0]=='@')
	{
		//uItems=sscanf(cLine,"@ %99s %255s %255s %255s %255s %255s\n",
		//	cType,cParam1,cParam2,cParam3,cParam4,cParam5);
		uItems=uStrscan(cLine,cOut);
		if(uItems==0)
		{
			if(guMode==2) printf("uItems 0 case2\n");
			return;
		}
		if(cOut[0]!=NULL)
			sprintf(cType,"%.99s",cOut[0]);
		if(cOut[1]!=NULL)
			sprintf(cParam1,"%.255s",cOut[1]);
		if(cOut[2]!=NULL)
			sprintf(cParam2,"%.255s",cOut[2]);
		if(cOut[3]!=NULL)
			sprintf(cParam3,"%.255s",cOut[3]);
		if(cOut[4]!=NULL)
			sprintf(cParam4,"%.255s",cOut[4]);
		if(cOut[5]!=NULL)
			sprintf(cParam5,"%.255s",cOut[5]);
		if(cPrevcName[0])
			sprintf(cName,"%.99s",cPrevcName);
		else	
			strcpy(cName,"\t");
	}
	else
	{
		//uItems=sscanf(cLine,"%99s %100s %255s %255s %255s %255s %255s\n",
		//	cName,cType,cParam1,cParam2,cParam3,cParam4,cParam5);
		uItems=uStrscan(cLine,cOut);
		if(uItems==0)
		{
			if(guMode==2) printf("uItems 0 case3\n");
			return;
		}
		if(cOut[0]!=NULL)
			sprintf(cName,"%.99s",cOut[0]);
		if(cOut[1]!=NULL)
			sprintf(cType,"%.99s",cOut[1]);
		if(cOut[2]!=NULL)
			sprintf(cParam1,"%.255s",cOut[2]);
		if(cOut[3]!=NULL)
			sprintf(cParam2,"%.255s",cOut[3]);
		if(cOut[4]!=NULL)
			sprintf(cParam3,"%.255s",cOut[4]);
		if(cOut[5]!=NULL)
			sprintf(cParam4,"%.255s",cOut[5]);
		if(cOut[6]!=NULL)
			sprintf(cParam5,"%.255s",cOut[6]);
		if(!cPrevcName[0])
			sprintf(cPrevcName,"%.99s",cName);
	}
	if(guMode==2) printf("Pre shift: uItems=%u cName='%s' cType='%s' cParam1='%s' cParam2='%s' cParam3='%s' cParam4='%s' cParam5='%s'\n",
			uItems,cName,cType,cParam1,cParam2,cParam3,cParam4,cParam5);
	if( (cName[0]==0 || cName[0]=='\t') && cType[0]==0 && cParam1[0]==0 && cParam2[0]==0)
	{
		if(guMode==2) printf("Early return on line with only white space chars\n");
		return;
	}

	if(cName[0]!='$')
	{
		//Shift left on inline TTL NOT $TTL directive
		if(!strchr(cType,'.') && strcmp(cName,"MX") && strcmp(cName,"SRV"))
		{
			sscanf(cType,"%u",&uTTL);
			if(uTTL>1 && uTTL<800000)
			{
				if(guMode==2) printf("Shift left TTL\n");
				strcpy(cType,cParam1);
				strcpy(cParam1,cParam2);
				strcpy(cParam2,cParam3);
				strcpy(cParam3,cParam4);
				strcpy(cParam4,cParam5);
			}
		}
	}

	//Shift left on IN
	if(!strcasecmp(cType,"IN"))
	{
		if(guMode==2) printf("Shift left IN\n");
		strcpy(cType,cParam1);
		strcpy(cParam1,cParam2);
		strcpy(cParam2,cParam3);
		strcpy(cParam3,cParam4);
		strcpy(cParam4,cParam5);
	}

	//This does not allow us to import cName's with these NAMES
	//when IN is missing from zone file
	//Shift right cType on cName
	if(
		( !strcasecmp(cName,"A")
		|| !strcasecmp(cName,"MX")
		|| !strcasecmp(cName,"PTR")
		|| !strcasecmp(cName,"NS")
		|| !strcasecmp(cName,"CNAME")
		|| !strcasecmp(cName,"TXT")
		|| !strcasecmp(cName,"AAAA")
		|| !strcasecmp(cName,"SRV") )
			&&
		( strcasecmp(cType,"A")
		&& strcasecmp(cType,"MX")
		&& strcasecmp(cType,"PTR")
		&& strcasecmp(cType,"NS")
		&& strcasecmp(cType,"CNAME")
		&& strcasecmp(cType,"TXT")
		&& strcasecmp(cType,"AAAA")
		&& strcasecmp(cType,"SRV") )

						)
	{
		if(guMode==2) printf("Shift right cType\n");
		strcpy(cParam4,cParam3);
		strcpy(cParam3,cParam2);
		strcpy(cParam2,cParam1);
		strcpy(cParam1,cType);
		strcpy(cType,cName);
		cName[0]='\t';
	}


	//debug only
	if(guMode==2) printf("Post shift: uItems=%u cName='%s' cType='%s' cParam1='%s' cParam2='%s' cParam3='%s' cParam4='%s' cParam5='%s'\n",
			uItems,cName,cType,cParam1,cParam2,cParam3,cParam4,cParam5);

	//Check for recognized data and verify needed params
	if(!strcasecmp(cType,"A") || !strcasecmp(cType,"AAAA"))
	{
		uRRType=1;
		if(!cParam1[0] || cParam2[0])
		{
			printf("Error %s: Incorrect A format: %s (%u)\n",
					cZoneName,cLine,guLine);
			return;
		}
		if(!strcmp(IPNumber(cParam1),"0.0.0.0"))
		{
			printf("Error %s: Incorrect A IP number format: %s (%u)\n",cZoneName,cLine,guLine);
			return;
		}
		cParam2[0]=0;
		cParam3[0]=0;
		cParam4[0]=0;
	}
	else if(!strcasecmp(cType,"CNAME"))
	{
		char cZone[254];
		char cName[254];
		uRRType=5;
		if(!cParam1[0] || cParam2[0])
		{
			printf("Error %s: Incorrect CNAME format: %s\n",
					cZoneName,cLine);
			return;
		}
		if(cParam1[strlen(cParam1)-1]!='.')
		{
			if(guMode<3) printf("Warning %s: Incorrect FQDN missing period: %s\n",cZoneName,cParam1);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam1,cPrevOrigin);
				sprintf(cParam1,"%.255s",gcQuery);
				if(guMode<3) printf("Fixed: CNAME RHS fixed from $ORIGIN: %s\n",cParam1);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam1,cZoneName);
				sprintf(cParam1,"%.255s",gcQuery);
				if(guMode<3) printf("Fixed: CNAME RHS fixed from cZoneName: %s\n",cParam1);
			}
		}
		sprintf(cZone,"%s.",FQDomainName(cZoneName));
		strcpy(cName,FQDomainName(cParam1));
		if(strcmp(cName+strlen(cName)-strlen(cZone),cZone))
			if(guMode<3) printf("Warning %s: CNAME RHS should probably end with zone: %s\n",cZoneName,cLine);
		cParam2[0]=0;
		cParam3[0]=0;
		cParam4[0]=0;
	}
	else if(!strcasecmp(cType,"NS"))
	{
		MYSQL_RES *res;
		char cNS[100];

		uRRType=2;
		if(!cParam1[0] || cParam2[0])
		{
			printf("Error %s: Incorrect NS format: %s (%u)\n",cZoneName,cLine,guLine);
			return;
		}
		if(cParam1[strlen(cParam1)-1]!='.')
		{
			if(guMode<3) printf("Warning %s: Incorrect FQDN missing period: %s\n",
					cZoneName,cLine);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam1,cPrevOrigin);
				sprintf(cParam1,"%.255s",gcQuery);
				if(guMode<3) printf("Fixed: NS RHS fixed from $ORIGIN: %s\n",cParam1);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam1,cZoneName);
				sprintf(cParam1,"%.255s",gcQuery);
				if(guMode<3) printf("Fixed: NS RHS fixed from cZoneName: %s\n",cParam1);
			}
		}

		//Get rid of last period for check
		strcpy(cNS,cParam1);
		cNS[strlen(cNS)-1]=0;
		sprintf(gcQuery,"SELECT uNameServer FROM tNameServer WHERE uNameServer=%u"
					" AND ( (cList LIKE '%s MASTER%%') OR (cList LIKE '%%%s SLAVE%%'))",uNameServer,cNS,cNS);

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
		{
			printf("Error %s: %s\n",cZoneName,mysql_error(&gMysql));
			return;
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)) 
		{
			printf("Error %s:NS RR Ignored. Part of uNameServer cList\n",
					cZoneName);
			return;
		}
		mysql_free_result(res);
		cParam2[0]=0;
		cParam3[0]=0;
		cParam4[0]=0;
	}
	else if(!strcasecmp(cType,"MX"))
	{
		unsigned uMX=0;
		uRRType=3;
		if(!cParam1[0] || !cParam2[0] )
		{
			printf("Error %s: Missing MX param: %s (%u)\n",
					cZoneName,cLine,guLine);
			return;
		}
		sscanf(cParam1,"%u",&uMX);
		if(uMX<0 || uMX>99999)
		{
			printf("Error %s: Incorrect MX format: %s (%u)\n",
					cZoneName,cLine,guLine);
			return;
		}
		if(cParam2[strlen(cParam2)-1]!='.')
		{
			if(guMode<3) printf("Warning %s: Incorrect FQDN missing period: %s (%u)\n",
					cZoneName,cLine,guLine);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam2,cPrevOrigin);
				sprintf(cParam2,"%.255s",gcQuery);
				if(guMode<3) printf("Fixed: MX RHS fixed from $ORIGIN: %s\n",cParam2);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam2,cZoneName);
				sprintf(cParam2,"%.255s",gcQuery);
				if(guMode<3) printf("Fixed: MX RHS fixed from cZoneName: %s\n",cParam2);
			}
		}
		cParam3[0]=0;
		cParam4[0]=0;
	}
	else if(!strcasecmp(cType,"PTR"))
	{
		unsigned uFirstDigit=0;

		uRRType=7;
		if(!cParam1[0] || cParam2[0])
		{
			printf("Error %s: Incorrect PTR format: %s\n",
					cZoneName,cLine);
			return;
		}
		if(cName[0]=='#')
		{
			sscanf(cName,"#%u.%*s",&uFirstDigit);
			if(!uFirstDigit)
			{
				printf("Error %s: Incorrect PTR LHS should start with a non zero digit, '*' or '#nnn': %s\n",cZoneName,cLine);
				return;
			}
		}
		else
		{
			sscanf(cName,"%u.%*s",&uFirstDigit);
			if(!uFirstDigit && cName[0]!='*')
			{
				printf("Error %s: Incorrect PTR LHS should start with a non zero digit, '*' or '#nnn': %s\n",cZoneName,cLine);
				return;
			}
		}
		cParam2[0]=0;
		cParam3[0]=0;
		cParam4[0]=0;
	}
	else if(!strcasecmp(cType,"TXT"))
	{
		uRRType=6;
		if((cp=strchr(cLine,'"')))
			sprintf(cParam1,"%.255s",cp);
		//Adjust for cr/lf also
		if(!cParam1[0] || cParam1[0]!='\"' || (cParam1[strlen(cParam1)-2]!='\"' &&
					cParam1[strlen(cParam1)-1]!='\"') )
		{
			printf("Error %s: Incorrect TXT format: %s\n",
					cZoneName,cParam1);
			return;
		}
		//debug only
		if(guMode<3) printf("TXT: %s\n",cParam1);
		cParam2[0]=0;
		cParam3[0]=0;
		cParam4[0]=0;
	}
	else if(!strcasecmp(cType,"SRV"))
	{
		//_sip._tls       IN       SRV 0 5 443 ed.ba.co.jp.
		uRRType=8;
		if(!cParam1[0] || !cParam2[0] || !cParam3[0] || !cParam4[0])
		{
			printf("Error %s: Incorrect SRV format\n",cZoneName);
			return;
		}
		//set ranges via RFC please
		unsigned uParam1=0;
		sscanf(cParam1,"%u",&uParam1);
		if(uParam1<0 || uParam1>9999)
		{
			printf("Error %s: Incorrect SRV format priority %s\n",cZoneName,cParam1);
			return;
		}
		unsigned uParam2=0;
		sscanf(cParam2,"%u",&uParam2);
		if(uParam2<1 || uParam2>9999)
		{
			printf("Error %s: Incorrect SRV format weight %s\n",cZoneName,cParam2);
			return;
		}
		unsigned uParam3=0;
		sscanf(cParam3,"%u",&uParam3);
		if(uParam3<1 || uParam3>16000)
		{
			printf("Error %s: Incorrect SRV format port %s\n",cZoneName,cParam3);
			return;
		}
		if(cParam4[strlen(cParam4)-1]!='.')
		{
			printf("Error %s: Incorrect SRV format FQDN missing period %s\n",cZoneName,cParam4);
			return;
		}
	}

	//Special cases that should keep a static var for next line
	else if(!strcasecmp(cName,"$TTL"))
	{
		if(cType[0])
		{
			sscanf(cType,"%u",&uPrevTTL);
			//debug only
			printf("$TTL changed: %u (%s %s)\n",uPrevTTL,cType,cParam1);
			return;
		}
	}
	else if(!strcasecmp(cName,"$ORIGIN"))
	{
		if(cType[0] && strcmp(cType,".") && cType[strlen(cType)-1]=='.')
			sprintf(cPrevOrigin,"%.99s",cType);
		{
			cPrevcName[0]=0;//Reset?
			if(!strncasecmp(cPrevOrigin,cZoneName,strlen(cPrevOrigin)-1))
				cPrevOrigin[0]=0;
			else
				//debug only
				printf("$ORIGIN Changed: %s\n",cPrevOrigin);
			return;
		}
	}

	//Unrecognized lines
	//Current missing features -we know about and need: hinfo ignored
	else if(1)
	{
		printf("Error:%s RR Not recognized: '%s'\n",cZoneName,cLine);
		return;
	}

	if(!uTTL && uPrevTTL) uTTL=uPrevTTL;
	if(cPrevOrigin[0]) 
		sprintf(cNamePlus,"%.99s.%.99s",cName,cPrevOrigin);
	else
		sprintf(cNamePlus,"%.99s",cName);


	//Commit to db
	if(guMode>=3)
	{
		if(uRRType==6)//TXT special case
			sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',"
				"cComment='idns-importzones',uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())"
					,uZone
					,FQDomainName(cNamePlus)
					,uTTL
					,uRRType
					,cParam1
					,uCustId);
		else
			sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%s',uTTL=%u,uRRType=%u,"
					"cParam1='%s',cParam2='%s',cParam3='%s',cParam4='%s',"
					"cComment='idns-importzones',uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())"
					,uZone
					,FQDomainName(cNamePlus)
					,uTTL
					,uRRType
					,FQDomainName(cParam1)
					,FQDomainName(cParam2)
					,FQDomainName(cParam3)
					,FQDomainName(cParam4)
					,uCustId);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			fprintf(stdout,"Error %s: %s\n",cZoneName,mysql_error(&gMysql));
	}

}//void ProcessRRLine()


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


//Needs to be extended for ipv6
char *IPNumber(char *cInput)
{
	unsigned a=0,b=0,c=0,d=0;

	sscanf(cInput,"%u.%u.%u.%u",&a,&b,&c,&d);

	if(a>255) a=0;
	if(b>255) b=0;
	if(c>255) c=0;
	if(d>255) d=0;

	sprintf(cInput,"%u.%u.%u.%u",a,b,c,d);

	return(cInput);

}//char *IPNumber(char *cInput)


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


int SubmitJob(const char *cCommand, unsigned uNameServerArg, const char *cZoneArg,
				unsigned uPriorityArg, time_t luTimeArg)
{
	MYSQL_RES *res2;
	MYSQL_ROW field;
	static unsigned uMasterJob=0;
	static char cTargetServer[101]={""};

	//Submit one job per EACH NS in the list, group with
	//uMasterJob
	sprintf(gcQuery,"SELECT cList FROM tNameServer WHERE uNameServer=%u",
			uNameServerArg);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		printf("Error %s: %s\n",gcQuery,mysql_error(&gMysql));
		exit(1);
	}
	res2=mysql_store_result(&gMysql);
	
	if((field=mysql_fetch_row(res2)))
	{
		register int i,j=0;

		for(i=0;i<strlen(field[0]);i++)
		{
			if(field[0][i]!='\n' && field[0][i])
			{
				cTargetServer[j++]=field[0][i];
			}
			else
			{
				cTargetServer[j]=0;
				j=0;

				if(SubmitSingleJob(cCommand,cZoneArg,uNameServerArg,
					cTargetServer,uPriorityArg,luTimeArg,&uMasterJob))
				{
					printf("Error %s: %s\n",gcQuery,mysql_error(&gMysql));
					exit(1);
				}
			}//if else
		}//for
	}//if field
	mysql_free_result(res2);

	return(0);

}//int SubmitJob()


int SubmitSingleJob(const char *cCommand,const char *cZoneArg, unsigned uNameServerArg,
		const char *cTargetServer, unsigned uPriorityArg, time_t luTimeArg
	       			,unsigned *uMasterJob)
{
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uJob FROM tJob WHERE cJob='%s' AND cZone='%s' AND uNameServer=%u AND cTargetServer='%s'",
				cCommand,cZoneArg,uNameServerArg,cTargetServer);
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
				"uNameServer=%u,cTargetServer='%s',uPriority=%u,uTime=%lu,"
				"uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cCommand,
			cZoneArg,
			uNameServerArg,
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
			if(guMode<3);
				printf("uMasterJob %u",*uMasterJob);
		}
	}
	mysql_free_result(res);

	return(0);

}//int SubmitSingleJob()

