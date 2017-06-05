/*
FILE
	svn ID removed
AUTHOR
	(C) 2001-2010 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
PURPOSE
	Command line support for importing text file based DNS data for migration or upgrades.
NOTES
	This code used to be in bind.c, was removed to start organizing and clean up.

	Import does not support uOwner. Must after import:
	1-. Use iDNS.cgi tZone Mass Operations MassUpdate Selecting customer from drop down.
	2-. Then since that tool may not currently support RRs, we must:
	mysql> update tResource,tZone set tResource.uOwner=8726 where tResource.uZone=tZone.uZone and tZone.uOwner=8726;
	Where uOwner would be replaced by the correct uOwner via tClient search.

*/

#include "mysqlrad.h"
#include <dirent.h>
#include <openisp/ucidr.h>

#define macro_MySQLQueryBasic \
	mysql_query(&gMysql,gcQuery);\
	if(mysql_errno(&gMysql))\
	{\
		printf("%s\n",mysql_error(&gMysql));\
		exit(1);\
	}

//
//Local data
//
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

//
//Extern protos
//
void EncryptPasswdWithSalt(char *cPasswd,char *cSalt);//main.c

//
//TOC protos
//
void ProcessRRLine(const char *cLine,char *cZoneName,const unsigned uZone,
		const unsigned uCustId,const unsigned uNSSet,
		const unsigned uCreatedBy,const char *cComment);
void ImportSORRs(void);
void ProcessSORRLine(const char *cLine,char *cZoneName,const unsigned uZone,
		const unsigned uCustId,const unsigned uNSSet,
		const unsigned uCreatedBy,const char *cComment);
structSOA *ProcessSOA(FILE *fp);
void Import(void);
void DropImportedZones(void);
void ImportCompanies(void);
void DropCompanies(void);
void ImportUsers(void);
void DropUsers(void);
void ImportBlocks(void);
void DropBlocks(void);
void ImportRegistrars(void);
void DropRegistrars(void);
void AssociateRegistrarsZones(void);
void AssociateCompaniesZones(void);
void FixBlockOwnership(void);
unsigned uGetNSSet(char *cNameServer);

//Mass update operations
void MassZoneUpdate(void);//mainfunc.h CLI command
void MassZoneNSUpdate(char *cLabel);//mainfunc.h CLI command

//Export operations
void ExportRRCSV(char *cCompany, char *cOutFile);

//New
void ImportAxfr(void);


void ProcessRRLine(const char *cLine,char *cZoneName,const unsigned uZone,
			const unsigned uCustId,const unsigned uNSSet,
			const unsigned uCreatedBy,const char *cComment)
{
	char cName[100]={""};
	char cNamePlus[200]={""};
	char cParam1[256]={""};
	char cParam2[256]={""};
	char cParam3[256]={""};
	char cParam4[256]={""};
	char cType[256]={""};
	static char cPrevZoneName[100]={""};
	static char cPrevcName[100]={""};
	unsigned uRRType=0;
	static unsigned uPrevTTL=0;//Has to be reset every new cZoneName
	unsigned uTTL=0;
	static char cPrevOrigin[100]={""};
	char *cp;
	char cResourceImportTable[256]="tResourceImport";

	GetConfiguration("cResourceImportTable",cResourceImportTable,0);


	if(strcmp(cZoneName,cPrevZoneName))
	{
		//printf("New zone %s\n",cZoneName);
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
	//printf("%s",cLine);

	if(cLine[0]=='\t')
	{
		sscanf(cLine,"%100s %255s %255s %255s %255s\n",
			cType,cParam1,cParam2,cParam3,cParam4);
		if(cPrevcName[0])
			sprintf(cName,"%.99s",cPrevcName);
		else	
			strcpy(cName,"\t");
	}
	else if(cLine[0]==' ')
	{
		sscanf(cLine,"%99s %100s %255s %255s %255s %255s\n",
			cName,cType,cParam1,cParam2,cParam3,cParam4);
		sprintf(cName,"@");
		sprintf(cPrevcName,"%.99s",cName);
	}
	else if(cLine[0]=='@')
	{
		sscanf(cLine,"@ %100s %255s %255s %255s %255s\n",
			cType,cParam1,cParam2,cParam3,cParam4);
		if(cPrevcName[0])
			sprintf(cName,"%.99s",cPrevcName);
		else	
			strcpy(cName,"\t");
	}
	else
	{
		sscanf(cLine,"%99s %100s %255s %255s %255s %255s\n",
			cName,cType,cParam1,cParam2,cParam3,cParam4);
		sprintf(cPrevcName,"%.99s",cName);
	}

	if(!cLine[0] || cLine[0]=='\n')
			return;

	if(cName[0]!='$')
	{
		//Shift left on inline TTL NOT $TTL directive
		sscanf(cType,"%u",&uTTL);
		if(uTTL>1 && uTTL<800000)
		{
			strcpy(cType,cParam1);
			strcpy(cParam1,cParam2);
			strcpy(cParam2,cParam3);
			strcpy(cParam3,cParam4);
		}
	}

	//Shift left on IN
	if(!strcasecmp(cType,"IN"))
	{
		strcpy(cType,cParam1);
		strcpy(cParam1,cParam2);
		strcpy(cParam2,cParam3);
		strcpy(cParam3,cParam4);
	}


	//Check for recognized data and verify needed params
	if(!strcasecmp(cType,"A"))
	{
		uRRType=1;
		if(!cParam1[0] || cParam2[0])
		{
			fprintf(stdout,"ProcessRRLine() Error %s: Incorrect A format: %s\n",
					cZoneName,cLine);
			return;
		}
		if(!strcmp(IPNumber(cParam1),"0.0.0.0"))
		{
			fprintf(stdout,"Incorrect A IP number format: %s\n",cLine);
			return;
		}
		if(strchr(cName,'_'))
		{
			fprintf(stdout,"ProcessRRLine() Error %s: cName (check-names): %s\n",
				cZoneName,cLine);
			return;
		}
	}
	else if(!strcasecmp(cType,"CNAME"))
	{
		char cZone[254];
		//char cName[254];
		uRRType=5;
		if(!cParam1[0] || cParam2[0])
		{
			fprintf(stdout,"ProcessRRLine() Error %s: Incorrect CNAME format: %s\n",
					cZoneName,cLine);
			return;
		}
		if(cParam1[strlen(cParam1)-1]!='.')
		{
			fprintf(stdout,"Warning: Incorrect FQDN missing period: %s\n",cParam1);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam1,cPrevOrigin);
				sprintf(cParam1,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: CNAME RHS fixed from $ORIGIN: %s\n",cParam1);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam1,cZoneName);
				sprintf(cParam1,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: CNAME RHS fixed from cZoneName: %s\n",cParam1);
			}
		}
		sprintf(cZone,"%s.",FQDomainName(cZoneName));

		//strcpy(cName,FQDomainName(cParam1));
		//if(strcmp(cName+strlen(cName)-strlen(cZone),cZone))
		//	fprintf(stdout,"Warning: CNAME RHS should probably end with zone: %s\n",cLine);
	}
	else if(!strcasecmp(cType,"NS"))
	{
		MYSQL_RES *res;
		char cNS[100];

		uRRType=2;
		if(!cParam1[0] || cParam2[0])
		{
			fprintf(stdout,"ProcessRRLine() Error %s: Incorrect NS format: %s\n",
					cZoneName,cLine);
			return;
		}
		if(cParam1[strlen(cParam1)-1]!='.')
		{
			fprintf(stdout,"Warning %s: Incorrect FQDN missing period: %s\n",
					cZoneName,cLine);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam1,cPrevOrigin);
				sprintf(cParam1,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: NS RHS fixed from $ORIGIN: %s\n",cParam1);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam1,cZoneName);
				sprintf(cParam1,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: NS RHS fixed from cZoneName: %s\n",cParam1);
			}
		}

		//Get rid of last period for check.
		strcpy(cNS,cParam1);
		cNS[strlen(cNS)-1]=0;
		sprintf(gcQuery,"SELECT uNS FROM tNS WHERE uNSSet=%u AND cFQDN LIKE '%s' ",
					uNSSet,cNS);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
		{
			fprintf(stdout,"ProcessRRLine() Error %s: %s\n",cZoneName,mysql_error(&gMysql));
			return;
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)) 
		{
			fprintf(stdout,"ProcessRRLine() warning %s:NS RR Ignored. NS belongs to current uNSSet=%u\n",
					cZoneName,uNSSet);
			return;
		}
		mysql_free_result(res);
	}
	else if(!strcasecmp(cType,"MX"))
	{
		unsigned uMX=999999;
		uRRType=3;
		if(!cParam1[0] || !cParam2[0] )
		{
			fprintf(stdout,"ProcessRRLine() Error %s: Missing MX param: %s\n",
					cZoneName,cLine);
			return;
		}
		sscanf(cParam1,"%u",&uMX);
		if(uMX>99999)
		{
			fprintf(stdout,"ProcessRRLine() Error %s: Incorrect MX format: %s\n",
					cZoneName,cLine);
			return;
		}
		if(cParam2[strlen(cParam2)-1]!='.')
		{
			fprintf(stdout,"Warning %s: Incorrect FQDN missing period: %s\n",
					cZoneName,cLine);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam2,cPrevOrigin);
				sprintf(cParam2,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: MX RHS fixed from $ORIGIN: %s\n",cParam2);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam2,cZoneName);
				sprintf(cParam2,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: MX RHS fixed from cZoneName: %s\n",cParam2);
			}
		}
	}
	else if(!strcasecmp(cType,"PTR"))
	{
		unsigned uFirstDigit=0;

		uRRType=7;
		if(!cParam1[0] || cParam2[0])
		{
			fprintf(stdout,"ProcessRRLine() Error %s: Incorrect PTR format: %s\n",
					cZoneName,cLine);
			return;
		}
		sscanf(cName,"%u.%*s",&uFirstDigit);
		if(!uFirstDigit)
		{
			//Check this rule again
			fprintf(stdout,"ProcessRRLine() Error %s: Incorrect PTR LHS should start with a non zero digit: %s\n",
				cZoneName,cLine);
			return;
		}

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
			fprintf(stdout,"ProcessRRLine() Error %s: Incorrect TXT format: %s\n",
					cZoneName,cParam1);
			return;
		}
		//debug only
		//fprintf(stdout,"TXT: %s\n",cParam1);
	}
	else if(!strcasecmp(cType,"SPF"))
	{
		uRRType=11;
		if((cp=strchr(cLine,'"')))
			sprintf(cParam1,"%.255s",cp);
		//Adjust for cr/lf also
		if(!cParam1[0] || cParam1[0]!='\"' || (cParam1[strlen(cParam1)-2]!='\"' &&
					cParam1[strlen(cParam1)-1]!='\"') )
		{
			fprintf(stdout,"ProcessRRLine() Error %s: Incorrect SPF format: %s\n",
					cZoneName,cParam1);
			return;
		}
		//debug only
		fprintf(stdout,"SPF: %s\n",cParam1);
	}

	//Special cases that should keep a static var for next line
	else if(!strcasecmp(cName,"$TTL"))
	{
		if(cType[0])
		{
			sscanf(cType,"%u",&uPrevTTL);
			//debug only
			fprintf(stdout,"$TTL changed: %u (%s %s)\n",
					uPrevTTL,cType,cParam1);
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
				fprintf(stdout,"$ORIGIN Changed: %s\n",
					cPrevOrigin);
			return;
		}
	}

	//Unrecognized lines
	//Current missing features -we know about and need: hinfo ignored
	else if(1 && cLine[0] && cLine[0]!='\n')
	{
		fprintf(stdout,"ProcessRRLine() Error %s: RR Not recognized: %s %s\n",cZoneName,cLine,cType);
		return;
	}

	if(!uTTL && uPrevTTL) uTTL=uPrevTTL;
	if(cPrevOrigin[0]) 
		sprintf(cNamePlus,"%.99s.%.99s",cName,cPrevOrigin);
	else
		sprintf(cNamePlus,"%.99s",cName);
	if(uRRType==6 || uRRType==11)//TXT and SPF special case
	sprintf(gcQuery,"INSERT INTO %s SET  uZone=%u,cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',"
			"cComment='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())"
					,cResourceImportTable
					,uZone
					,FQDomainName(cNamePlus)
					,uTTL
					,uRRType
					,cParam1
					,cComment
					,uCustId
					,uCreatedBy);
	else
	sprintf(gcQuery,"INSERT INTO %s SET uZone=%u,cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',"
			"cParam2='%s',cComment='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())"
					,cResourceImportTable
					,uZone
					,FQDomainName(cNamePlus)
					,uTTL
					,uRRType
					,FQDomainName(cParam1)
					,FQDomainName(cParam2)
					,cComment
					,uCustId
					,uCreatedBy);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
		fprintf(stdout,"ProcessRRLine() Error %s: %s\n",cZoneName,mysql_error(&gMysql));

	return;

}//void ProcessRRLine()


void Import(void)
{
	struct dirent **namelist;
	structSOA *importSOA;
	register int n,i;
	char cHostMaster[256]="hostmaster.isp.net";
	char cuView[256]="2";
	char cuNSSet[256]="0";
	char cuOwner[256]="1";
	char cZoneImportTable[256]="tZoneImport";

	fprintf(stdout,"ImportZones() Start\n");

	GetConfiguration("cHostMaster",cHostMaster,0);
	GetConfiguration("cuView",cuView,0);
	GetConfiguration("cZoneImportTable",cZoneImportTable,0);
	GetConfiguration("cuNSSet",cuNSSet,0);
	GetConfiguration("cuOwner",cuOwner,0);

	fprintf(stdout,"Importing master zones from /usr/local/idns/import directory.\n"
		"File name must be same as zone name.\nOnly A, CNAME, MX, PTR and"
		" NS records supported.\nSOA ttl values must be seconds.\n"
		"You can also set cHostMaster, cuNSSet, cuView, cuOwner and cZoneImportTable in\n"
		"tConfiguration. Current defaults:\n"
		"cHostMaster=%s\n"
		"cuNSSet=%s (if set to zero will try to determine from SOA or fallback to uNSSet=1)\n"
		"cuView=%s\n"
		"cuOwner=%s\n"
		"cZoneImportTable=%s\n\n",cHostMaster,cuNSSet,cuView,cuOwner,cZoneImportTable);
	fprintf(stdout,"Confirm <enter any key>, <ctrl-c> to abort\n");
	getchar();
		
	n=scandir("/usr/local/idns/import",&namelist,0,0);
			
	if(n<0)
	{
		fprintf(stdout,"scandir() error: Does /usr/local/idns/import exist?\n");
		return;
	}
	else if(n==2)
	{
		fprintf(stdout,"No files found.\n");
		return;
	}

	for(i=0;i<n;i++)
	{

		//Added some end of list test hack a long time ago. Remove?
		if(namelist[i]->d_name[0]=='.' || 
			strstr(namelist[i]->d_name+strlen(namelist[i]->d_name)-5,
				".done"))
		{
			;
		}
		else
		{
			FILE *fp;

			sprintf(gcQuery,"/usr/local/idns/import/%.100s",
					namelist[i]->d_name);
			if(!(fp=fopen(gcQuery,"r")))
			{
				fprintf(stdout,"Error: Could not open: %s\n",gcQuery);
				return;
			}

			//Start processing
			fprintf(stdout,"%.100s\n\n",namelist[i]->d_name);
	
			//Major import component
			importSOA=ProcessSOA(fp);

			//Add more rule based checking of TTLs	
			if(	importSOA->uTTL &&
				importSOA->uSerial &&
				importSOA->uRefresh &&
				importSOA->uRetry &&
				importSOA->uExp &&
				importSOA->uNegTTL &&
				importSOA->cHostmaster[0] )
			{
				//All zones will belong to the default #1 NS
				unsigned uZone,uOwner=1,uNSSet=0;

				sscanf(cuOwner,"%u",&uOwner);
				sscanf(cuNSSet,"%u",&uNSSet);

				//Import zone
				importSOA->cNameServer[strlen(importSOA->cNameServer)-1]=0;
				importSOA->cHostmaster[strlen(importSOA->cHostmaster)-1]=0;
				//debug only
				//printf("uTTL=%u\n",importSOA->uTTL);
				//printf("uSerial=%u\n",importSOA->uSerial);
				//printf("uRefresh=%u\n",importSOA->uRefresh);
				//printf("uRetry=%u\n",importSOA->uRetry);
				//printf("uExp=%u\n",importSOA->uExp);
				//printf("uNegTTL=%u\n",importSOA->uNegTTL);
				//printf("cHostmaster=%s\n",importSOA->cHostmaster);
				//printf("cNameServer=%s\n",importSOA->cNameServer);

				//Try to use an existing tNSSet
				if(uNSSet==0)
				{
					uNSSet=uGetNSSet(importSOA->cNameServer);
					
					if(uNSSet==0)
						uNSSet=1;//First tNSSet
					else
						fprintf(stdout,"Using uNSSet=%u determined via:%s\n",
								uNSSet,importSOA->cNameServer);
				}
				else
				{
					uNSSet=1;//First tNSSet
				}


				//uZoneTTL is the NegTTL
				sprintf(gcQuery,"INSERT INTO %s SET cZone='%s',uNSSet=%u,uSerial=%u,"
					"uExpire=%u,uRefresh=%u,uTTL=%u,uRetry=%u,uZoneTTL=%u,uMailServers=0,"
					"cMainAddress='0.0.0.0',uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),"
					"uModBy=0,uModDate=0,cHostmaster='%s IMPORTED',uView=%.2s"
					,cZoneImportTable
					,FQDomainName(namelist[i]->d_name)
					,uNSSet
					,importSOA->uSerial
					,importSOA->uExp
					,importSOA->uRefresh
					,importSOA->uTTL
					,importSOA->uRetry
					,importSOA->uNegTTL
					,uOwner
					,cHostMaster
					,cuView);
					//,importSOA->cHostmaster);
	
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					fprintf(stdout,"Error %s: %s\n",
							namelist[i]->d_name,mysql_error(&gMysql));
					if(!strncmp("Duplicate entry",mysql_error(&gMysql),15))
					{
						MYSQL_RES *res4;
						MYSQL_ROW field4;

						
						uZone=0;
						uOwner=0;
						sprintf(gcQuery,
							"SELECT uZone,uOwner FROM %s WHERE cZone='%s'",
						cZoneImportTable,
						FQDomainName(namelist[i]->d_name));
						mysql_query(&gMysql,gcQuery);
						res4=mysql_store_result(&gMysql);
						if((field4=mysql_fetch_row(res4)))
						{
							sscanf(field4[0],"%u",&uZone);
							sscanf(field4[1],"%u",&uOwner);
						}
						mysql_free_result(res4);
						if(uZone)
						{
							fprintf(stdout,"Warning %s: Adding RR to a zone"
								" that already exists!\n",namelist[i]->d_name);
							goto AddToExisting;
						}
					}

					continue;
				}
				uZone=mysql_insert_id(&gMysql);

AddToExisting:
				if(!uOwner) uOwner=1;
				//Process rest of file for RRs
				//printf("Importing RRs\n");
				while(fgets(gcQuery,254,fp)!=NULL)
				{
				  //skip empty lines
				  if(gcQuery[0]!='\n')
					ProcessRRLine(gcQuery,
						FQDomainName(namelist[i]->d_name),
						uZone,uOwner,uNSSet,1," IMPORTED");
				}
			}
			else
			{
				fprintf(stdout,"Error %s:\nCould not process SOA correctly\n",
						namelist[i]->d_name);
				fprintf(stdout,"uTTL=%u\n",importSOA->uTTL);
				fprintf(stdout,"uSerial=%u\n",importSOA->uSerial);
				fprintf(stdout,"uRefresh=%u\n",importSOA->uRefresh);
				fprintf(stdout,"uRetry=%u\n",importSOA->uRetry);
				fprintf(stdout,"uExp=%u\n",importSOA->uExp);
				fprintf(stdout,"uNegTTL=%u\n",importSOA->uNegTTL);
				fprintf(stdout,"cHostmaster=%s\n",importSOA->cHostmaster);
				fprintf(stdout,"cNameServer=%s\n",importSOA->cNameServer);
				return;
			}
			fclose(fp);

		}//If valid file for importing

	}//for each file in import dir
	fprintf(stdout,"\nImportZones() Done\n");

}//void Import(void)


void ImportSORRs(void)
{
	//
	//This function import the zone RRs for secondary service only zones
	//into the iDNS database.
	//These records will be read only, which will be limited at interface level.
	//The code here is heavily based on the code of the Import() function above.
	//
	//Steps:
	//1. Query the database to find out the secondary service only zones
	//2. Once we have the zone name, we build the zone file path using:
	//   iDNSRootFolder/named.d/slave/[external|internal]/(cZone 1st char)/cZone
	//3. We open the file
	//4. Must remove all existing RRs at the zone, we are updating!
	//5. We run a modified version of the loop in the Import() function that imports the zone RRs by 
	//calling ProcessRRLine()

	MYSQL_RES *res;
	MYSQL_ROW field;
	char cFileName[512]={""};
	char *cView="";
	FILE *fp;
	unsigned uZone=0;
	unsigned uOwner=0;
	unsigned uNSSet=0;
	structSOA *importSOA;
	

	fprintf(stdout,"ImportSORRs() Start\n");
	
	//1. Query the database to find out the secondary service only zones
	sprintf(gcQuery,"SELECT cZone,uView,uZone,uOwner,uNSSet FROM tZone WHERE uSecondaryOnly=1 ORDER BY uZone");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"Error: %s\n",mysql_error(&gMysql));
		exit(1);
	}
	
	res=mysql_store_result(&gMysql);

	while((field=mysql_fetch_row(res)))
	{
		//2. Once we have the zone name, we build the zone file path
		if(!strcmp(field[1],"1"))
			cView="internal";
		else
			cView="external";
		
		sprintf(cFileName,"/usr/local/idns/named.d/slave/%s/%c/%s",
				cView
				,field[0][0]
				,field[0]
		       );
		//3. We open the file
		if((fp=fopen(cFileName,"r"))==NULL)
		{
			fprintf(stderr,"Warning: could not open zonefile for zone %s, path was %s\n",
				field[0],cFileName);
			continue;
		}
		
		//4. Must remove all existing RRs at the zone, we are updating!
		
		sscanf(field[2],"%u",&uZone);
		sscanf(field[3],"%u",&uOwner);
		sscanf(field[4],"%u",&uNSSet);
	
		sprintf(gcQuery,"DELETE FROM tResource WHERE uZone='%u'",uZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"Error: %s\n",mysql_error(&gMysql));
			exit(1);
		}

		//5. We run a modified version of the loop in the Import() function
		importSOA=ProcessSOA(fp); //Unused, just to move file pointer to 'after soa position'
		
		while(fgets(gcQuery,254,fp)!=NULL)
		{
		  //skip empty lines
		  if(gcQuery[0]!='\n')
			ProcessSORRLine(gcQuery,field[0],uZone,uOwner,uNSSet,1," SecOnly IMPORTED");
		}
	}//while((field=mysql_fetch_row(res)))

	fprintf(stdout,"ImportSORRs() End\n");
	
}//void ImportSORRs(void)


void ProcessSORRLine(const char *cLine,char *cZoneName,const unsigned uZone,
			const unsigned uCustId,const unsigned uNSSet,
			const unsigned uCreatedBy,const char *cComment)
{
	char cName[100]={""};
	char cNamePlus[200]={""};
	char cParam1[256]={""};
	char cParam2[256]={""};
	char cParam3[256]={""};
	char cParam4[256]={""};
	char cType[256]={""};
	static char cPrevZoneName[100]={""};
	static char cPrevcName[100]={""};
	unsigned uRRType=0;
	static unsigned uPrevTTL=0;//Has to be reset every new cZoneName
	unsigned uTTL=0;
	static char cPrevOrigin[100]={""};
	char *cp;


	if(strcmp(cZoneName,cPrevZoneName))
	{
		//printf("New zone %s\n",cZoneName);
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
	//printf("<u>%s</u>\n",cLine);

	if(cLine[0]=='\t')
	{
		sscanf(cLine,"%100s %255s %255s %255s %255s\n",
			cType,cParam1,cParam2,cParam3,cParam4);
		if(cPrevcName[0])
			sprintf(cName,"%.99s",cPrevcName);
		else	
			strcpy(cName,"\t");
	}
	else if(cLine[0]=='@')
	{
		sscanf(cLine,"@ %100s %255s %255s %255s %255s\n",
			cType,cParam1,cParam2,cParam3,cParam4);
		if(cPrevcName[0])
			sprintf(cName,"%.99s",cPrevcName);
		else	
			strcpy(cName,"\t");
	}
	else
	{
		sscanf(cLine,"%99s %100s %255s %255s %255s %255s\n",
			cName,cType,cParam1,cParam2,cParam3,cParam4);
		sprintf(cPrevcName,"%.99s",cName);
	}

	if(!cLine[0] || cLine[0]==';')
			return;

	if(cName[0]!='$')
	{
		//Shift left on inline TTL NOT $TTL directive
		sscanf(cType,"%u",&uTTL);
		if(uTTL>1 && uTTL<800000)
		{
			strcpy(cType,cParam1);
			strcpy(cParam1,cParam2);
			strcpy(cParam2,cParam3);
			strcpy(cParam3,cParam4);
		}
	}

	//Shift left on IN
	if(!strcasecmp(cType,"IN"))
	{
		strcpy(cType,cParam1);
		strcpy(cParam1,cParam2);
		strcpy(cParam2,cParam3);
		strcpy(cParam3,cParam4);
	}

	//Check for recognized data and verify needed params
	if(!strcasecmp(cType,"A"))
	{
		uRRType=1;
		if(!cParam1[0] || cParam2[0])
		{
			fprintf(stdout,"Error %s: Incorrect A format: %s\n",
					cZoneName,cLine);
			return;
		}
		if(!strcmp(IPNumber(cParam1),"0.0.0.0"))
		{
			fprintf(stdout,"Incorrect A IP number format: %s\n",cLine);
			return;
		}
	}
	else if(!strcasecmp(cType,"CNAME"))
	{
		char cZone[254];
		char cName[254];
		uRRType=5;
		if(!cParam1[0] || cParam2[0])
		{
			fprintf(stdout,"Error %s: Incorrect CNAME format: %s\n",
					cZoneName,cLine);
			return;
		}
		if(cParam1[strlen(cParam1)-1]!='.')
		{
			fprintf(stdout,"Warning: Incorrect FQDN missing period: %s\n",cParam1);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam1,cPrevOrigin);
				sprintf(cParam1,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: CNAME RHS fixed from $ORIGIN: %s\n",cParam1);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam1,cZoneName);
				sprintf(cParam1,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: CNAME RHS fixed from cZoneName: %s\n",cParam1);
			}
		}
		sprintf(cZone,"%s.",FQDomainName(cZoneName));
		strcpy(cName,FQDomainName(cParam1));
		if(strcmp(cName+strlen(cName)-strlen(cZone),cZone))
			fprintf(stdout,"Warning: CNAME RHS should probably end with zone: %s\n",cLine);
	}
	else if(!strcasecmp(cType,"NS"))
	{
		MYSQL_RES *res;
		char cNS[100];

		uRRType=2;
		if(!cParam1[0] || cParam2[0])
		{
			fprintf(stdout,"Error %s: Incorrect NS format: %s\n",
					cZoneName,cLine);
			return;
		}
		if(cParam1[strlen(cParam1)-1]!='.')
		{
			fprintf(stdout,"Warning %s: Incorrect FQDN missing period: %s\n",
					cZoneName,cLine);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam1,cPrevOrigin);
				sprintf(cParam1,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: NS RHS fixed from $ORIGIN: %s\n",cParam1);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam1,cZoneName);
				sprintf(cParam1,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: NS RHS fixed from cZoneName: %s\n",cParam1);
			}
		}

		//Get rid of last period for check.
		strcpy(cNS,cParam1);
		cNS[strlen(cNS)-1]=0;
		sprintf(gcQuery,"SELECT uNS FROM tNS WHERE uNSSet=%u AND cFQDN LIKE '%s' ",
					uNSSet,cNS);
		mysql_query(&gMysql,gcQuery);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
		{
			fprintf(stdout,"Error %s: %s\n",cZoneName,mysql_error(&gMysql));
			return;
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)) 
			//Silent return
			return;
		mysql_free_result(res);
	}
	else if(!strcasecmp(cType,"MX"))
	{
		unsigned uMX=999999;
		uRRType=3;
		if(!cParam1[0] || !cParam2[0] )
		{
			fprintf(stdout,"Error %s: Missing MX param: %s\n",
					cZoneName,cLine);
			return;
		}
		sscanf(cParam1,"%u",&uMX);
	
		if(uMX>99999)
		{
			fprintf(stdout,"Error %s: Incorrect MX format: %s\n",
					cZoneName,cLine);
			return;
		}
		if(cParam2[strlen(cParam2)-1]!='.')
		{
			fprintf(stdout,"Warning %s: Incorrect FQDN missing period: %s\n",
					cZoneName,cLine);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam2,cPrevOrigin);
				sprintf(cParam2,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: MX RHS fixed from $ORIGIN: %s\n",cParam2);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam2,cZoneName);
				sprintf(cParam2,"%.255s",gcQuery);
				fprintf(stdout,"Fixed: MX RHS fixed from cZoneName: %s\n",cParam2);
			}
		}
	}
	else if(!strcasecmp(cType,"PTR"))
	{
		unsigned uFirstDigit=0;

		uRRType=7;
		if(!cParam1[0] || cParam2[0])
		{
			fprintf(stdout,"Error %s: Incorrect PTR format: %s\n",
					cZoneName,cLine);
			return;
		}
		sscanf(cName,"%u.%*s",&uFirstDigit);
		if(!uFirstDigit)
		{
			//Check this rule again
			fprintf(stdout,"Error %s: Incorrect PTR LHS should start with"
					" a non zero digit: %s\n",
						cZoneName,cLine);
			return;
		}

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
			fprintf(stdout,"Error %s: Incorrect TXT format: %s\n",
					cZoneName,cParam1);
			return;
		}
		//debug only
		//fprintf(stdout,"TXT: %s\n",cParam1);
	}
	else if(!strcasecmp(cType,"SPF"))
	{
		uRRType=11;
		if((cp=strchr(cLine,'"')))
			sprintf(cParam1,"%.255s",cp);
		//Adjust for cr/lf also
		if(!cParam1[0] || cParam1[0]!='\"' || (cParam1[strlen(cParam1)-2]!='\"' &&
					cParam1[strlen(cParam1)-1]!='\"') )
		{
			fprintf(stdout,"Error %s: Incorrect SPF format: %s\n",
					cZoneName,cParam1);
			return;
		}
		//debug only
		fprintf(stdout,"SPF: %s\n",cParam1);
	}

	//Special cases that should keep a static var for next line
	else if(!strcasecmp(cName,"$TTL"))
	{
		if(cType[0])
		{
			sscanf(cType,"%u",&uPrevTTL);
			//debug only
			//printf("$TTL changed: %u (%s %s)\n",
			//		uPrevTTL,cType,cParam1);
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
				//printf("$ORIGIN Changed: %s\n",
				//	cPrevOrigin);
			return;
		}
	}

	//Unrecognized lines
	//Current missing features -we know about and need: hinfo ignored
	else if(1)
	{
		fprintf(stdout,"Error %s: RR Not recognized: %s %s\n",cZoneName,cLine,cType);
		return;
	}

	if(!uTTL && uPrevTTL) uTTL=uPrevTTL;
	if(cPrevOrigin[0]) 
		sprintf(cNamePlus,"%.99s.%.99s",cName,cPrevOrigin);
	else
		sprintf(cNamePlus,"%.99s",cName);
	
	if(!uRRType) return; //NOP if not valid RR is found
	if(uRRType==6 || uRRType==11)//TXT and SPF special case
		sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%s',uTTL=%u,"
				"uRRType=%u,cParam1='%s',cComment='%s',uOwner=%u,uCreatedBy=%u,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())",
					uZone
					,FQDomainName(cNamePlus)
					,uTTL
					,uRRType
					,cParam1
					,cComment
					,uCustId
					,uCreatedBy);
	else
		sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%s',uTTL=%u,"
				"uRRType=%u,cParam1='%s',cParam2='%s',cComment='%s',uOwner=%u,"
				"uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					uZone
					,FQDomainName(cNamePlus)
					,uTTL
					,uRRType
					,FQDomainName(cParam1)
					,FQDomainName(cParam2)
					,cComment
					,uCustId
					,uCreatedBy);
	mysql_query(&gMysql,gcQuery);
	//printf("%s\n",gcQuery);
	if(mysql_errno(&gMysql)) 
		fprintf(stdout,"Error %s: %s\n",cZoneName,mysql_error(&gMysql));

	return;

}//void ProcessSORRLine()


void DropImportedZones(void)
{
	char cZoneImportTable[256]="tZoneImport";
	char cResourceImportTable[256]="tResourceImport";

	//GetConfiguration("cZoneImportTable",cZoneImportTable,0);
	//GetConfiguration("cResourceImportTable",cResourceImportTable,0);

	fprintf(stdout,"DropImportedZones() Start\n");

	//sprintf(gcQuery,"DELETE FROM %s WHERE cHostmaster LIKE '%% IMPORTED'",cZoneImportTable);
	sprintf(gcQuery,"TRUNCATE %s",cZoneImportTable);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));

	//sprintf(gcQuery,"DELETE FROM %s WHERE cComment LIKE '%% IMPORTED'",cResourceImportTable);
	sprintf(gcQuery,"TRUNCATE %s",cResourceImportTable);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));

	fprintf(stdout,"DropImportedZones() Done\n");

}//DropImportedZones()


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
		if((cp=strchr(gcQuery,';')))
		{
			*cp='\n';
			*(cp+1)=0;
		}
		//debug only
		//printf("%s",gcQuery);

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


//Import tClient records that will be owners of tClient user records
void ImportCompanies(void)
{
	FILE *fp;
	char cQuery[256];
	unsigned uClient;
	char *cp;

	fprintf(stdout,"ImportCompanies() Start\n");

	if(!(fp=fopen("/usr/local/idns/csv/companycode.csv","r")))
	{
		fprintf(stdout,"Error: Could not open: /usr/local/idns/csv/companycode.csv\n");
				return;
	}

	//Start processing. Unix files only!
	while(fgets(gcQuery,254,fp)!=NULL)
	{
		uClient=0;//tClient PK

		//skip empty lines and comments
		if(gcQuery[0]=='\n' || gcQuery[0]=='#')
			continue;
		gcQuery[strlen(gcQuery)-1]=0;


		//Quick and dirty parsing input must be in good shape!
		//Parse uClient
		if((cp=strchr(gcQuery,','))) 
		{
			*cp=0;
			sscanf(gcQuery,"%u",&uClient);
		
			sprintf(cQuery,"INSERT INTO tClient SET uClient=%u,cLabel='%.99s',"
				"cCode='Organization',cInfo='ImportCompanies() IMPORTED',"
				"uOwner=1,uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())",uClient,cp+1);
			//debug only
			//fprintf(stdout,"%s\n",cQuery);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
				fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
		}
		else
			fprintf(stdout,"Skipping invalid line: %s\n",gcQuery);
	}

	if(fp) fclose(fp);

	fprintf(stdout,"ImportCompanies() End\n");

}//void ImportCompanies(void)


void DropCompanies(void)
{
	fprintf(stdout,"DropCompanies() Start\n");

	sprintf(gcQuery,"DELETE FROM tClient WHERE cInfo LIKE '%%ImportCompanies() IMPORTED'");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));

	fprintf(stdout,"DropCompanies() End\n");

}//void DropCompanies(void)


//Import tClient records that will be owned by an existing tClient company record
void ImportUsers(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	FILE *fp;
	char cQuery[256];
	char cCompany[256];
	char cFirst[100];
	char cLast[100];
	char cEmail[100];
	char cLogin[100];
	char cPasswd[100];
	char *cp,*cp2;
	unsigned uCompany,uUser;

	fprintf(stdout,"ImportUsers() Start\n");

	if(!(fp=fopen("/usr/local/idns/csv/companyuser.csv","r")))
	{
		fprintf(stdout,"Error: Could not open: /usr/local/idns/csv/companyuser.csv\n");
				return;
	}

	//Start processing. Unix files only!
	while(fgets(gcQuery,254,fp)!=NULL)
	{
		uCompany=0;
		uUser=0;
		cFirst[0]=0;
		cLast[0]=0;
		cEmail[0]=0;
		cLogin[0]=0;
		cPasswd[0]=0;
		cCompany[0]=0;

		//skip empty lines and comments
		if(gcQuery[0]=='\n' || gcQuery[0]=='#')
			continue;
		gcQuery[strlen(gcQuery)-1]=0;

		//debug only
		//fprintf(stdout,"%s\n",gcQuery);

		//Quick and dirty parsing input must be in good shape!
		//Parse uCompany
		if((cp=strchr(gcQuery,','))) 
		{
			*cp=0;
			sscanf(gcQuery,"%u",&uCompany);
			*cp=',';
		}
		//debug only
		//fprintf(stdout,"uCompany=%u\n",uCompany);

		//Parse uUser
		if((cp2=strchr(cp+1,','))) 
		{
			*cp2=0;
			sscanf(cp+1,"%u",&uUser);
			*cp2=',';
		}
		//debug only
		//fprintf(stdout,"uUser=%u\n",uUser);

		//Parse cFirst
		if((cp=strchr(cp2+1,','))) 
		{
			*cp=0;
			sprintf(cFirst,"%.99s",cp2+1);
			*cp=',';
		}
		//debug only
		//fprintf(stdout,"cFirst=%s\n",cFirst);

		//Parse cLast
		if((cp2=strchr(cp+1,','))) 
		{
			*cp2=0;
			sprintf(cLast,"%.99s",cp+1);
			*cp2=',';
		}
		//debug only
		//fprintf(stdout,"cLast=%s\n",cLast);

		//Parse cEmail
		if((cp=strchr(cp2+1,','))) 
		{
			*cp=0;
			sprintf(cEmail,"%.99s",cp2+1);
			*cp=',';
		}
		//debug only
		//fprintf(stdout,"cEmail=%s\n",cEmail);

		//Parse cLogin
		if((cp2=strchr(cp+1,','))) 
		{
			*cp2=0;
			sprintf(cLogin,"%.99s",cp+1);
			*cp2=',';
		}
		//debug only
		//fprintf(stdout,"cLogin=%s\n",cLogin);

		//Parse cPasswd
		sprintf(cPasswd,"%.99s",cp2+1);
		//debug only
		//fprintf(stdout,"cPasswd=%s\n",cPasswd);


		//Sanity checks
		if(!uCompany || !uUser || !cFirst[0] || !cLast[0] || !cLogin[0])
			fprintf(stdout,"Warning %s may be incorrectly formatted\n",
					gcQuery);

		sprintf(cQuery,"SELECT cLabel FROM tClient WHERE uOwner=1 AND uClient=%u AND"
				" cInfo LIKE '%%ImportCompanies() IMPORTED'",uCompany);
		mysql_query(&gMysql,cQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
			exit(2);
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
			sprintf(cCompany,"%.255s",field[0]);
		mysql_free_result(res);

		if(cCompany[0])
		{
			//Company same as user, fix company
			sprintf(cQuery,"%.99s %.99s",cFirst,cLast);
			if(!strcmp(cCompany,cQuery))
			{
				sprintf(gcQuery,"UPDATE tClient SET cLabel='%.250s Co.',uModBy=1,"
						"uModDate=UNIX_TIMESTAMP(NOW()) WHERE uClient=%u",cCompany,uCompany);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
				{
					fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
					exit(2);
				}
			}
			sprintf(gcQuery,"INSERT INTO tClient SET cLabel='%.99s %.99s',cInfo='ImportUsers() IMPORTED',"
					"uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),cEmail='%s',"
					"uClient=%u",TextAreaSave(cFirst),TextAreaSave(cLast),uCompany,cEmail,uUser);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				fprintf(stdout,"Warning: %s\n",mysql_error(&gMysql));
				continue;
			}

			//tAuthorize issues
			if(!cLogin[0])
				sprintf(cLogin,"%.4s%.4s",cFirst,cLast);
			if(!cPasswd[0])
				sprintf(cPasswd,"%.3sH56%.3s",cFirst,cLast);
			EncryptPasswdWithSalt(cPasswd,"..");
			sprintf(gcQuery,"INSERT INTO tAuthorize SET cLabel='%.32s',uPerm=6,uCertClient=%u,cPasswd='%s',"
				"uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),cIPMask='0.0.0.0/0'",
				cLogin,uUser,cPasswd,uCompany);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				fprintf(stdout,"Warning: %s\n",mysql_error(&gMysql));
		}
		else
		{
			fprintf(stdout,"Error: %s company not found\n",cCompany);
		}

	}

	if(fp) fclose(fp);

	
	fprintf(stdout,"ImportUsers() End\n");

}//void ImportUsers(void)


void DropUsers(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cQuery[256];
	unsigned uClient;

	fprintf(stdout,"DropUsers() Start\n");

	sprintf(cQuery,"SELECT uClient FROM tClient WHERE uOwner!=1 AND cInfo LIKE '%%ImportUsers() IMPORTED'");
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
		exit(2);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		uClient=0;
		sscanf(field[0],"%u",&uClient);

		sprintf(gcQuery,"DELETE FROM tAuthorize WHERE uCertClient=%u",uClient);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
	}
	mysql_free_result(res);

	sprintf(cQuery,"DELETE FROM tClient WHERE uOwner!=1 AND cInfo LIKE '%%ImportUsers() IMPORTED'");
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
		exit(2);
	}

	fprintf(stdout,"DropUsers() End\n");

}//void DropUsers(void)


void ImportBlocks(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	FILE *fp;
	char cQuery[256];
	char cCompany[256];
	char cBlock[100];
	char *cp;
	unsigned uClient;

	fprintf(stdout,"ImportBlocks() Start\n");

	if(!(fp=fopen("/usr/local/idns/csv/companyblock.csv","r")))
	{
		fprintf(stdout,"Error: Could not open: /usr/local/idns/csv/companyblock.csv\n");
				return;
	}

	//Start processing. Unix files only!
	while(fgets(gcQuery,254,fp)!=NULL)
	{
		cBlock[0]=0;
		cCompany[0]=0;
		uClient=0;

		//skip empty lines and comments
		if(gcQuery[0]=='\n' || gcQuery[0]=='#')
			continue;
		gcQuery[strlen(gcQuery)-1]=0;

		//debug only
		//fprintf(stdout,"%s\n",gcQuery);

		//Quick and dirty parsing input must be in good shape!
		//Parse Company uClient
		if((cp=strchr(gcQuery,','))) 
		{
			*cp=0;
			sscanf(gcQuery,"%u",&uClient);
			*cp=',';
		}
		//debug only
		//fprintf(stdout,"uClient=%u\n",uClient);

		//Parse cBlock
		sprintf(cBlock,"%.99s",cp+1);
		//fprintf(stdout,"cBlock=%s\n",cBlock);

		//Sanity checks
		if(!uClient || !cBlock[0] )
			fprintf(stdout,"Warning %s may be incorrectly formatted\n",
					gcQuery);

		sprintf(cQuery,"SELECT cLabel FROM tClient WHERE uOwner=1 AND cInfo LIKE '%%ImportCompanies()"
				" IMPORTED' AND uClient=%u",uClient);
		mysql_query(&gMysql,cQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
			exit(2);
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
			sprintf(cCompany,"%.255s",field[0]);
		mysql_free_result(res);

		//debug only
		//fprintf(stdout,"cCompany=%s\n",cCompany);
		if(cCompany[0])
		{
			sprintf(cQuery,"INSERT INTO tBlock SET cLabel='%.32s',cComment='ImportBlocks() IMPORTED',"
				"uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",cBlock,uClient);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
			{
				fprintf(stdout,"Warning: %s\n",mysql_error(&gMysql));
			}
		}
		else
		{
			fprintf(stdout,"Error: %s company not found\n",cCompany);
		}

	}

	if(fp) fclose(fp);
	fprintf(stdout,"ImportBlocks() End\n");

}//void ImportBlocks(void)


void DropBlocks(void)
{
	fprintf(stdout,"DropBlocks() Start\n");

	sprintf(gcQuery,"DELETE FROM tBlock WHERE cComment LIKE '%%ImportBlocks() IMPORTED'");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));

	fprintf(stdout,"DropBlocks() End\n");

}//void DropBlocks(void)


void AssociateCompaniesZones(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	FILE *fp;
	char cQuery[256];
	char cCompany[256];
	char cZone[100];
	char cIP[20];
	char *cp;
	unsigned uClient;
	unsigned uA;
	unsigned uB;
	unsigned uC;
	unsigned uD;
	unsigned uResource;

	fprintf(stdout,"AssociateCompaniesZones() Start\n");

	if(!(fp=fopen("/usr/local/idns/csv/companyzone.csv","r")))
	{
		fprintf(stdout,"Error: Could not open: /usr/local/idns/csv/companyzone.csv\n");
				return;
	}

	//Start processing. Unix files only!
	while(fgets(gcQuery,254,fp)!=NULL)
	{
		cCompany[0]=0;
		cZone[0]=0;
		uClient=0;

		//skip empty lines and comments
		if(gcQuery[0]=='\n' || gcQuery[0]=='#')
			continue;
		gcQuery[strlen(gcQuery)-1]=0;

		//debug only
		//fprintf(stdout,"%s\n",gcQuery);

		//Quick and dirty parsing input must be in good shape!
		//Parse cCompany
		if((cp=strchr(gcQuery,','))) 
		{
			*cp=0;
			sscanf(gcQuery,"%u",&uClient);
			*cp=',';
		}

		sprintf(cZone,"%.99s",cp+1);
		//fprintf(stdout,"cZone=%s\n",cZone);

		//Sanity checks
		if(!uClient || !cZone[0] )
			fprintf(stdout,"Warning %s may be incorrectly formatted\n",
					gcQuery);

		sprintf(cQuery,"SELECT uClient FROM tClient WHERE uOwner=1 AND cInfo"
				" LIKE '%%ImportCompanies() IMPORTED' AND uClient=%u",uClient);
		mysql_query(&gMysql,cQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
			exit(2);
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
			sprintf(cCompany,"%.255s",field[0]);
		mysql_free_result(res);

		if(cCompany[0])
		{
			sprintf(cQuery,"UPDATE tZone SET uOwner=%u,uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
					" WHERE cZone='%s'",uClient,cZone);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
			{
				fprintf(stdout,"Warning: %s\n",mysql_error(&gMysql));
			}
			if(mysql_affected_rows(&gMysql)<1)
			{
				fprintf(stdout,"Warning: %s not updated.\n",cZone);
				continue;//Skip RRs
			}
			sprintf(cQuery,"UPDATE tResource,tZone SET tResource.uOwner=%u,tResource.uModBy=1,"
					"tResource.uModDate=UNIX_TIMESTAMP(NOW()) WHERE"
					" tZone.uZone=tResource.uZone AND tZone.cZone='%s'",uClient,cZone);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
			{
				fprintf(stdout,"Warning: %s\n",mysql_error(&gMysql));
			}
		}
		else
		{
			fprintf(stdout,"Error: %s company not found\n",cCompany);
		}

	}
	if(fp) fclose(fp);

	//In this section we use block ownership to change PTR RRs ownership
	//Overlapping blocks should not be allowed to exist in tBlock
	//so the first block that contains the IP is the block we use.
	//This simplistic algo only works for class C in-addr.arpa zones
	sprintf(cQuery,"SELECT tResource.uResource,tResource.cName,tZone.cZone FROM tResource,tZone WHERE"
			" tResource.uZone=tZone.uZone AND tResource.uRRType=7");
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
		exit(2);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		uA=255;
		uB=255;
		uC=255;
		uD=255;

		sscanf(field[0],"%u",&uResource);

		//Try to reconstruct the IP
		sscanf(field[1],"%u",&uD);
		sscanf(field[2],"%u.%u.%u.in-addr.arpa",&uC,&uB,&uA);
		sprintf(cIP,"%u.%u.%u.%u\n",uA,uB,uC,uD);
		if(uA>0 && uA<255 && uB<255 && uC<255 && uD<255)
		{
			MYSQL_RES *res2;
			MYSQL_ROW field2;

			//This next step is Big-O n. i.e. very slow...
			sprintf(cQuery,"SELECT cLabel,uOwner FROM tBlock");
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
			{
				fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
				exit(2);
			}
			res2=mysql_store_result(&gMysql);
			while((field2=mysql_fetch_row(res2)))
			{
				if(uIpv4InCIDR4(cIP,field2[0])==1)
				{
					sscanf(field2[1],"%u",&uClient);
					sprintf(cQuery,"UPDATE tResource SET uOwner=%u WHERE uResource=%u",
						uClient,uResource);
					mysql_query(&gMysql,cQuery);
					if(mysql_errno(&gMysql))
					{
						fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
						exit(2);
					}
					//debug only
					//fprintf(stdout,"%s is in %s\n",cIP,field2[0]);
					break;	
				}
			}
			mysql_free_result(res2);
		}
		else
		{
			fprintf(stdout,"Warning: %s seems incorrect. Skiping.\n",cIP);
		}
	}
	mysql_free_result(res);
	
	fprintf(stdout,"AssociateCompaniesZones() End\n");

}//void AssociateCompaniesZones(void)


//Import tRegistrar records
void ImportRegistrars(void)
{
	FILE *fp;
	char cQuery[256];

	fprintf(stdout,"ImportRegistrars() Start\n");

	if(!(fp=fopen("/usr/local/idns/csv/registrar.txt","r")))
	{
		fprintf(stdout,"Error: Could not open: /usr/local/idns/csv/registrar.txt\n");
				return;
	}

	//Start processing. Unix files only!
	while(fgets(gcQuery,254,fp)!=NULL)
	{
		//skip empty lines NOT comments #1 registery in the world :)
		if(gcQuery[0]=='\n')
			continue;
		gcQuery[strlen(gcQuery)-1]=0;

		sprintf(cQuery,"INSERT INTO tRegistrar SET cLabel='%.99s',uOwner=1,uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())",gcQuery);
		//debug only
		//fprintf(stdout,"%s\n",cQuery);
		mysql_query(&gMysql,cQuery);
		if(mysql_errno(&gMysql))
			fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
	}

	if(fp) fclose(fp);

	fprintf(stdout,"ImportRegistrars() End\n");

}//void ImportRegistrars(void)


void DropRegistrars(void)
{
	fprintf(stdout,"DropRegistrars() Start\n");

	sprintf(gcQuery,"TRUNCATE tRegistrar");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));

	fprintf(stdout,"DropRegistrars() End\n");

}//void DropRegistrars(void)


void AssociateRegistrarsZones(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	FILE *fp;
	char cQuery[256];
	char cRegistrar[256];
	unsigned uRegistrar;
	char cZone[100];
	char *cp;

	fprintf(stdout,"AssociateRegistrarsZones() Start\n");

	if(!(fp=fopen("/usr/local/idns/csv/registrarzone.csv","r")))
	{
		fprintf(stdout,"Error: Could not open: /usr/local/idns/csv/registrarzone.csv\n");
				return;
	}

	//Start processing. Unix files only!
	while(fgets(gcQuery,254,fp)!=NULL)
	{
		cRegistrar[0]=0;
		cZone[0]=0;
		uRegistrar=0;

		if(gcQuery[0]=='\n' || gcQuery[0]=='#')
			continue;
		gcQuery[strlen(gcQuery)-1]=0;

		//Sample lines
		//442subs.co.uk,Nominet
		//442subsusa.com,Network Solutions

		//Quick and dirty parsing input must be in good shape!
		//Parse cZone
		if((cp=strchr(gcQuery,','))) 
		{
			*cp=0;
			sprintf(cZone,"%.99s",gcQuery);
			*cp=',';
		}
		//debug only
		//fprintf(stdout,"cZone=%s\n",cZone);

		sprintf(cRegistrar,"%.99s",cp+1);
		//debug only
		//fprintf(stdout,"cRegistrar=%s\n",cRegistrar);

		//Sanity checks
		if(!cRegistrar[0] || !cZone[0] )
		{
			fprintf(stderr,"Error: %s may be incorrectly formatted\n",
					gcQuery);
			continue;
		}

		sprintf(cQuery,"SELECT uRegistrar FROM tRegistrar WHERE cLabel='%s'",cRegistrar);
		mysql_query(&gMysql,cQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
			exit(2);
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
			sscanf(field[0],"%u",&uRegistrar);
		mysql_free_result(res);

		if(uRegistrar)
		{
			sprintf(cQuery,"UPDATE tZone SET uRegistrar=%u,uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
					" WHERE cZone='%s'",uRegistrar,cZone);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
			{
				fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
			}
			if(mysql_affected_rows(&gMysql)<1)
			{
				fprintf(stdout,"Warning: %s not updated.\n",cZone);
			}
		}
		else
		{
			fprintf(stdout,"Warning: %s registrar not found\n",cRegistrar);
		}

	}
	if(fp) fclose(fp);

}//void AssociateRegistrarsZones(void)


void MassZoneUpdate(void)
{
	FILE *fp;
	char cQuery[512];
	char cZone[256];
	unsigned uNSSet;
	MYSQL_RES *res;
	MYSQL_ROW field;

	fprintf(stdout,"MassZoneUpdate() Start\n");

	if(!(fp=fopen("/usr/local/idns/csv/zones2update.txt","r")))
	{
		fprintf(stdout,"Error: Could not open: /usr/local/idns/csv/zones2update.txt\n");
				return;
	}

	//Start processing. Unix files only!
	while(fgets(cZone,254,fp)!=NULL)
	{
		if(cZone[0]=='\n' || cZone[0]=='#')
			continue;
		cZone[strlen(cZone)-1]=0;

		uNSSet=0;
		sprintf(cQuery,"SELECT uNSSet FROM tZone WHERE cZone='%.255s'",cZone);
		mysql_query(&gMysql,cQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
			exit(2);
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
			sscanf(field[0],"%u",&uNSSet);
		mysql_free_result(res);
		if(!uNSSet)
		{
			fprintf(stdout,"Warning: %s not found.\n",cZone);
			continue;
		}

		//debug only
		//fprintf(stdout,"%s\n",cQuery);
		//break;

		//1-. Update zone serial number
		sprintf(cQuery,"UPDATE tZone SET uSerial=uSerial+1,uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
				" WHERE cZone='%.255s'",cZone);
		mysql_query(&gMysql,cQuery);
		if(mysql_errno(&gMysql))
			fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
		if(mysql_affected_rows(&gMysql)<1)
		{
			fprintf(stdout,"Warning: %s not updated.\n",cZone);
			continue;
		}

		//2-. Submit zone mod job only if we have a NS and the update worked
		//external view only
		SubmitJob("Modify",uNSSet,cZone,0,0);
	}

	if(fp) fclose(fp);

	fprintf(stdout,"MassZoneUpdate() End\n");

}//void MassZoneUpdate(void)


void MassZoneNSUpdate(char *cLabel)
{
	FILE *fp;
	char cQuery[512];
	char cZone[256];
	char cMasterIPs[256]={""};
	unsigned uNSSet=0;
	MYSQL_RES *res;
	MYSQL_ROW field;

	fprintf(stdout,"MassZoneNSUpdate() Start\n");

	if(!cLabel[0])
	{
		fprintf(stderr,"Error: No tNameServer.cLabel provided. Aborting.\n");
		exit(1);
	}

	if(!(fp=fopen("/usr/local/idns/csv/zones2NSupdate.txt","r")))
	{
		fprintf(stdout,"Error: Could not open: /usr/local/idns/csv/zones2NSupdate.txt\n");
				return;
	}
	//3 == MASTER EXTERNAL
	sprintf(cQuery,"SELECT uNSSet,cMasterIPs FROM tNS,tNSSet WHERE tNS.uNSSet=tNSSet.uNSSet AND"
			" tNSSet.cLabel='%.32s' AND tNS.uNSType=3",cLabel);
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
		exit(2);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&uNSSet);
		sprintf(cMasterIPs,"%.255s",field[1]);
	}
	mysql_free_result(res);
	if(!uNSSet)
	{
		fprintf(stderr,"Error: %s not found in tNameServer with a MASTER EXTERNAL. Aborting.\n",cLabel);
		exit(3);
	}
	if(!cMasterIPs[0])
	{
		fprintf(stderr,"Error: No tNameServer.cMasterIPs for %s. Aborting.\n",cLabel);
		exit(4);
	}


	//Start processing. Unix files only!
	while(fgets(cZone,254,fp)!=NULL)
	{
		if(cZone[0]=='\n' || cZone[0]=='#')
			continue;
		cZone[strlen(cZone)-1]=0;


		//1-. Update zone serial number, uNSSet, cMasterIPs OLD FORK
		//1-. Update zone serial number, uNSSet NEW GPL FORK
		sprintf(cQuery,"UPDATE tZone SET uNSSet=%u,uSerial=uSerial+1,uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
				" WHERE cZone='%.255s'",uNSSet,cZone);

		//debug only
		//fprintf(stdout,"%s\n",cQuery);
		//continue;


		mysql_query(&gMysql,cQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stdout,"Error: %s\n",mysql_error(&gMysql));
			continue;
		}
		if(mysql_affected_rows(&gMysql)<1)
		{
			fprintf(stdout,"Warning: %s not updated.\n",cZone);
			continue;
		}

		//2-. Submit zone mod job only if we have a NS and the update worked
		//Modify New means new NS = new slave.zones file
		SubmitJob("Modify New",uNSSet,cZone,0,0);
	}

	if(fp) fclose(fp);

	fprintf(stdout,"MassZoneNSUpdate() End\n");

}//void MassZoneNSUpdate(char *cLabel)


void ExportRRCSV(char *cCompany, char *cOutFile)
{
	//This function exports the data from the tResource table
	//into a CSV file. The exported data belongs to the company 
	//indicated by the cCompany argument.
	//CSV output file is specified by the cOutFile argument.
	//If empty, we will just use stdout.
	MYSQL_RES *res;
	MYSQL_ROW field;

	unsigned uClient=0;

	sprintf(gcQuery,"USE "DBNAME);
	macro_mySQLQueryTextError;

	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s'",TextAreaSave(cCompany));
	macro_mySQLRunAndStoreText(res);
	if((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&uClient);
	}
	else
	{
		fprintf(stderr,"Company %s not found.\n",cCompany);
		exit(0);
	}

	mysql_free_result(res);

	if(cOutFile[0])
	{
		sprintf(gcQuery,"SELECT tZone.cZone,cName,tResource.uTTL,tRRType.cLabel,"
				"cParam1,cParam2,FROM_UNIXTIME(tResource.uCreatedDate) "
				"INTO OUTFILE '%s' FIELDS TERMINATED BY ',' OPTIONALLY "
				"ENCLOSED BY '\"' LINES TERMINATED BY '\n' FROM "
				"tResource,tZone,tRRType WHERE tResource.uOwner=%u "
				"AND tRRType.uRRType=tResource.uRRType AND tZone.uZone=tResource.uZone "
				"ORDER BY tZone.cZone",
				cOutFile
				,uClient
			);
		macro_mySQLQueryTextError;
		fprintf(stdout,"Export complete to %s\n",cOutFile);
		exit(0);
	}
	else
	{
		sprintf(gcQuery,"SELECT tZone.cZone,cName,tResource.uTTL,tRRType.cLabel,"
				"cParam1,cParam2,FROM_UNIXTIME(tResource.uCreatedDate) "
				"FROM tResource,tZone,tRRType WHERE tResource.uOwner=%u "
				"AND tRRType.uRRType=tResource.uRRType AND tZone.uZone=tResource.uZone "
				"ORDER BY tZone.cZone",
				uClient
			);
		macro_mySQLRunAndStoreText(res);
		while((field=mysql_fetch_row(res)))
			fprintf(stdout,"\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
				field[0]
				,field[1]
				,field[2]
				,field[3]
				,field[4]
				,field[5]
				,field[6]
				);
		mysql_free_result(res);
		exit(0);
	}


}//void ExportRRCSV(char *cCompany)


void FixBlockOwnership(void)
{
	//This function will go through tBlock and fix all PTR record ownership issues if any.
	unsigned uA=0;
	unsigned uB=0;
	unsigned uC=0;
	unsigned uD=0;
	unsigned uE=0;
	unsigned uNumIPs=0;
	unsigned uBlockEnd=0;
	unsigned uI=0;
	char cZone[256]={""};
	unsigned uCounter=0;
	
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field2;

	sprintf(gcQuery,"SELECT uOwner,cLabel FROM tBlock");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(0);
	}

	res2=mysql_store_result(&gMysql);

	while((field2=mysql_fetch_row(res2)))
	{
		uCounter=0;
		fprintf(stdout,"Block %s\n",field2[1]);
		sscanf(field2[1],"%u.%u.%u.%u/%u",&uA,&uB,&uC,&uD,&uE);
			
		if(!uA)
		{
			fprintf(stdout,"IP Block incorrect format\n");
			continue;
		}
		if((uA>255)||(uB>255)||(uC>255)||(uD>255))
		{
			fprintf(stdout,"IP Block incorrect format\n");
			continue;
		}

		sprintf(cZone,"%u.%u.%u.in-addr.arpa",uC,uB,uA);
		sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",cZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res))
		{
			MYSQL_ROW field;
			uNumIPs=uGetNumIPs(field2[1]);
			uNumIPs+=2; //Add broadcast and router
			uBlockEnd=uD+uNumIPs;

			//Loop is for handling all zone views
			while((field=mysql_fetch_row(res)))
			{
				for(uI=uD;uI<uBlockEnd;uI++)
				{
					sprintf(gcQuery,"UPDATE tResource SET uOwner=%s "
							"WHERE cName='%i' AND uZone='%s' "
							"AND uRRType=7",
							field2[0]
							,uI
							,field[0]
							);
					//fprintf(stdout,"%s\n",gcQuery);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
					uCounter+=mysql_affected_rows(&gMysql);
				}
			}
		}	
		fprintf(stdout,"%u RRs updated\n",uCounter);
	}
}//void FixBlockOwnership(void)


unsigned uGetNSSet(char *cNameServer)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uNSSet=0;

	sprintf(gcQuery,"SELECT tNSSet.uNSSet FROM tNSSet,tNS WHERE"
			" tNSSet.uNSSet=tNS.uNSSet AND"
			" tNS.cFQDN='%s'",cNameServer);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stdout,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uNSSet);
	mysql_free_result(res);

	return(uNSSet);

}//unsigned uGetNSSet(char *cNameServer)


//Import non tZone RRs from dig axfr format file for existing zones
//Ignores all lines with SOA and NS
//Files must be named tZone.cZone and be in /usr/local/idns/axfr/<cView>/ where <cView> is an exsiting tView.cLabel
void ImportAxfr(void)
{
	register int i,j;
	struct dirent **direntViewDir;
	struct dirent **direntZoneFile;
	int uViewDir=0;
	int uZoneFile=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uView=0;
	unsigned uZone=0;
	unsigned uOwner;
	unsigned uNSSet;
	char cuOwner[256]="1";

	GetConfiguration("cuOwner",cuOwner,0);
	sscanf(cuOwner,"%u",&uOwner);

	fprintf(stdout,"ImportAxfr() Start\n");

	uViewDir=scandir("/usr/local/idns/axfr",&direntViewDir,0,0);
	if(uViewDir<0)
	{
		fprintf(stdout,"scandir() error: Does /usr/local/idns/axfr exist?\n");
		return;
	}
	else if(uViewDir==2)
	{
		fprintf(stdout,"No view dirs found in /usr/local/idns/axfr/\n");
		return;
	}

	for(j=0;j<uViewDir;j++)
	{
		//skip the . and .. entries
		if(direntViewDir[j]->d_name[0]=='.') continue;

		//
		//fprintf(stdout,"View %.100s\n",direntViewDir[j]->d_name);

		//if no such tView.cLabel inform and skip
		uView=0;
		sprintf(gcQuery,"SELECT uView FROM tView WHERE cLabel='%s'",direntViewDir[j]->d_name);
		macro_MySQLQueryBasic;
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
			sscanf(field[0],"%u",&uView);
		mysql_free_result(res);
		if(uView==0)
		{
			fprintf(stdout,"View '%.100s' not in tView\n",direntViewDir[j]->d_name);
			continue;
		}

		sprintf(gcQuery,"/usr/local/idns/axfr/%.100s",direntViewDir[j]->d_name);
		uZoneFile=scandir(gcQuery,&direntZoneFile,0,0);
		if(uZoneFile<0)
		{
			fprintf(stdout,"scandir() error: Does /usr/local/idns/axfr/%s/ exist?\n",direntViewDir[j]->d_name);
			return;
		}
		else if(uZoneFile==2)
		{
			fprintf(stdout,"No zone files found in /usr/local/idns/axfr/%s/\n",direntViewDir[j]->d_name);
			continue;
		}

		for(i=0;i<uZoneFile;i++)
		{
			//skip the . and .. entries
			if(direntZoneFile[i]->d_name[0]=='.') continue;

			//if no such tZone.cZone with tView.uView==uView from above inform and skip
			uZone=0;
			uNSSet=0;
			sprintf(gcQuery,"SELECT uZone,uNSSet FROM tZone WHERE cZone='%s' AND uView=%u",
					direntZoneFile[i]->d_name,uView);
			macro_MySQLQueryBasic;
			res=mysql_store_result(&gMysql);
			if((field=mysql_fetch_row(res)))
			{
				sscanf(field[0],"%u",&uZone);
				sscanf(field[1],"%u",&uNSSet);
			}
			mysql_free_result(res);
			if(uZone==0)
			{
				fprintf(stdout,"Zone '%.100s' with view='%s' (uView=%u) not in tZone\n",
					direntZoneFile[i]->d_name,direntViewDir[j]->d_name,uView);
				continue;
			}
			if(uNSSet==0)
			{
				fprintf(stdout,"Zone '%.100s' with view='%s' (uView=%u) has uNSSet==0\n",
					direntZoneFile[i]->d_name,direntViewDir[j]->d_name,uView);
				continue;
			}

			FILE *fp;

			sprintf(gcQuery,"/usr/local/idns/axfr/%.100s/%.100s",direntViewDir[j]->d_name,direntZoneFile[i]->d_name);
			if(!(fp=fopen(gcQuery,"r")))
			{
				fprintf(stdout,"Error: Could not open: %s\n",gcQuery);
				return;
			}

			//Start processing
			fprintf(stdout,"%.100s/%.100s\n",direntViewDir[j]->d_name,direntZoneFile[i]->d_name);

			//Zap existing zone RRs. Be WARNED!
			sprintf(gcQuery,"DELETE FROM tResource WHERE uZone=%u",uZone);
			macro_MySQLQueryBasic;

			while(fgets(gcQuery,254,fp)!=NULL)
			{
				if(strstr(gcQuery,"SOA"))
					continue;
				if(strstr(gcQuery,"NS"))
					continue;
				ProcessRRLine(gcQuery,FQDomainName(direntZoneFile[i]->d_name),uZone,uOwner,uNSSet,1,"ImportAxfr");
			}

			fclose(fp);
	
		}//for each zone file in view dir

	}//for each view dir

	fprintf(stdout,"ImportAxfr() Done\n");

}//void ImportAxfr(void)


//Create new empty zones based on given (in list) base uZone, new uOwner, new uView
void SerialNum(char *cSerialNum);//bind.c
void CloneZonesFromList(void)
{
	unsigned uZone=0;
	unsigned uOwner=0;
	unsigned uView=0;
	FILE *fp;
	char *cp;
	MYSQL_RES *res;
	char cLine[256];

	sprintf(gcQuery,"/usr/local/idns/clonezones.txt");
	if(!(fp=fopen(gcQuery,"r")))
	{
		fprintf(stdout,"Error: Could not open: %s\n",gcQuery);
		return;
	}

	while(fgets(cLine,254,fp)!=NULL)
	{
		//fprintf(stdout,"%s",cLine);

		if(!strncmp(cLine,"#",1))
			continue;
		if(!strncmp(cLine,";",1))
			continue;
		if(!strncmp(cLine,"\n",1))
			continue;
		if(!strncmp(cLine,"\r",1))
			continue;

		if((cp=strchr(cLine,'\n'))) *cp=0;

		if(!strncmp(cLine,"uZone=",6))
		{
			sscanf(cLine+6,"%u",&uZone);
			if(!uZone)
			{
				fprintf(stdout,"Error: uZone==0 %s\n",cLine);
				return;
			}
			else
			{
				fprintf(stdout,"uZone=%u\n",uZone);
			}
			continue;
		}

		if(!strncmp(cLine,"uOwner=",7))
		{
			sscanf(cLine+7,"%u",&uOwner);
			if(!uOwner)
			{
				fprintf(stdout,"Error: uOwner==0 %s\n",cLine);
				return;
			}
			else
			{
				fprintf(stdout,"uOwner=%u\n",uOwner);
			}
			continue;
		}

		if(!strncmp(cLine,"uView=",6))
		{
			sscanf(cLine+6,"%u",&uView);
			if(!uView)
			{
				fprintf(stdout,"Error: uView==0 %s\n",cLine);
				return;
			}
			else
			{
				fprintf(stdout,"uView=%u\n",uView);
			}
			continue;
		}

		if(!uZone)
		{
			fprintf(stdout,"Error: uZone==0\n");
			return;
		}
		if(!uOwner)
		{
			fprintf(stdout,"Error: uOwner==0\n");
			return;
		}
		if(!uView)
		{
			fprintf(stdout,"Error: uView==0\n");
			return;
		}

		//Check valid uZone
		sprintf(gcQuery,"SELECT uZone FROM tZone WHERE uZone=%u",uZone);
		macro_MySQLQueryBasic;
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)==0)
		{
			fprintf(stdout,"Current uZone=%u does not exist\n",uZone);
			continue;
		}
		mysql_free_result(res);
		//Check valid uView
		sprintf(gcQuery,"SELECT uView FROM tView WHERE uView=%u",uView);
		macro_MySQLQueryBasic;
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)==0)
		{
			fprintf(stdout,"Current uView=%u does not exist\n",uView);
			continue;
		}
		mysql_free_result(res);
		//Check valid uOwner
		sprintf(gcQuery,"SELECT uClient FROM tClient WHERE uClient=%u",uOwner);
		macro_MySQLQueryBasic;
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)==0)
		{
			fprintf(stdout,"Current uOwner=%u does not exist\n",uOwner);
			continue;
		}
		mysql_free_result(res);

		char cZone[65]={""};
		sprintf(cZone,"%.64s",WordToLower(cLine));
		//If cZone already exists skip
		sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=%u",cZone,uView);
		macro_MySQLQueryBasic;
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)>0)
		{
			fprintf(stdout,"Zone %s with uView=%u already exists\n",cZone,uView);
			continue;
		}
		mysql_free_result(res);

		fprintf(stdout,"Creating zone %s\n",cZone);
		char cSerial[32];
		SerialNum(cSerial);
		//Source
		sprintf(gcQuery,"INSERT INTO tZoneImport"
				" (cZone,uNSSet,cHostmaster,"
				"uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
				"uMailServers,uView,cMainAddress,uRegistrar,uSecondaryOnly,cOptions,"
				"uOwner,uCreatedBy,uCreatedDate)"
				" SELECT '%s',uNSSet,cHostmaster,%s,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
				"uMailServers,%u,cMainAddress,uRegistrar,uSecondaryOnly,cOptions,"
				"%u,1,UNIX_TIMESTAMP(NOW()) FROM tZone WHERE uZone=%u",
				cZone,cSerial,uView,uOwner,uZone);
		macro_MySQLQueryBasic;
		//Production zone
		sprintf(gcQuery,"INSERT INTO tZone"
				" (cZone,uNSSet,cHostmaster,"
				"uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
				"uMailServers,uView,cMainAddress,uRegistrar,uSecondaryOnly,cOptions,"
				"uOwner,uCreatedBy,uCreatedDate)"
				" SELECT cZone,uNSSet,cHostmaster,uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
				"uMailServers,uView,cMainAddress,uRegistrar,uSecondaryOnly,cOptions,"
				"uOwner,uCreatedBy,uCreatedDate FROM tZoneImport WHERE uZone=%u",(unsigned)mysql_insert_id(&gMysql));
		macro_MySQLQueryBasic;
		//Cleanup
		sprintf(gcQuery,"DELETE FROM tZoneImport WHERE uZone=%u",(unsigned)mysql_insert_id(&gMysql));
		macro_MySQLQueryBasic;
	}
	fclose(fp);

}//void CloneZonesFromList(void)
