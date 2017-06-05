/*
FILE 
	bulkop.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file in main source dir.
PURPOSE
	iDNS Administration (ASP) Interface
	program file.
*/

static char *cMassList={""};
extern char cuView[];//zone.c
static char cImportMsg[32762]={""}; //A 32k buffer will be enough, if not, truncate the data.
static unsigned uDebug=0;
#include "interface.h"
#include <openisp/ucidr.h>

//
//Local only
char *ParseTextAreaLines(char *cTextArea);
void BulkResourceImport(void);
unsigned uGetZoneNSSet(char *cZone);
unsigned ProcessRRLine(unsigned uLineNumber,const char *cLine,char *cZoneName,const unsigned uZone, unsigned uCustId,const unsigned uNSSet,const unsigned uCreatedBy,const char *cComment);
unsigned uGetPTROwner(char *cZone, char *cFirstDigit);

//resource.c
void UpdateSerialNum(char *cZone,char *cuView);
unsigned uGetZoneOwner(unsigned uZone);

//zone.c
void SerialNum(char *cSerial);

//main.c
char *FQDomainName2(char *cInput);

void ProcessBulkOpVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cMassList"))
			cMassList=entries[i].val;
		else if(!strcmp(entries[i].name,"cZone"))
			sprintf(gcZone,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"uView"))
			sprintf(cuView,"%.9s",entries[i].val);
		else if(!strcmp(entries[i].name,"uDebug"))
			sscanf(entries[i].val,"%u",&uDebug);
	}

}//void ProcessBulkOpVars(pentry entries[], int x)


void BulkOpGetHook(entry gentries[],int x)
{

	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"cZone"))
			sprintf(gcZone,"%.99s",gentries[i].val);
		else if(!strcmp(gentries[i].name,"uView"))
			sprintf(cuView,"%.15s",gentries[i].val);
	}
	htmlBulkOp();

}//void BulkOpGetHook(entry gentries[],int x)


void BulkOpCommands(pentry entries[], int x)
{
	if(!strcmp(gcPage,"BulkOp"))
	{
		ProcessBulkOpVars(entries,x);
		
		if(!strcmp(gcFunction,"Bulk Resource Import"))
		{
			if(!cMassList[0])
			{
				gcMessage="<blink>Error: </blink>cMassList empty. Read manual for list format.";
				htmlBulkOp();
			}
			
			BulkResourceImport();
			
		}
		if(!strcmp(gcFunction,"Back to Zones Tab"))
		{
			htmlZone();
		}
		htmlBulkOp();
	}
}//void BulkOpCommands(pentry entries[], int x)


void htmlBulkOp(void)
{
	htmlHeader("idnsAdmin","Header");
	htmlBulkOpPage("idnsAdmin","AdminBulkOp.Body");
	htmlFooter("Footer");

}//void htmlBulkOp(void)


void htmlBulkOpPage(char *cTitle, char *cTemplateName)
{
	if(cTemplateName[0])
	{
        	MYSQL_RES *res;
	        MYSQL_ROW field;

		TemplateSelectInterface(cTemplateName,uPLAINSET,uIDNSADMINTYPE);
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			struct t_template template;
			char cuResource[16]={""};

			sprintf(cuResource,"%u",uResource);

			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
			
			template.cpName[1]="cCGI";
			template.cpValue[1]="idnsAdmin.cgi";
			
			template.cpName[2]="gcLogin";
			template.cpValue[2]=gcUser;

			template.cpName[3]="gcName";
			template.cpValue[3]=gcName;

			template.cpName[4]="gcOrgName";
			template.cpValue[4]=gcOrgName;

			template.cpName[5]="cUserLevel";
			template.cpValue[5]=(char *)cUserLevel(guPermLevel);

			template.cpName[6]="gcHost";
			template.cpValue[6]=gcHost;

			template.cpName[7]="gcModStep";
			template.cpValue[7]=gcModStep;

			template.cpName[8]="cZone";
			template.cpValue[8]=gcZone;

			template.cpName[9]="gcMessage";
			template.cpValue[9]=gcMessage;

			template.cpName[10]="cuView";
			template.cpValue[10]=cuView;

			template.cpName[11]="cImportMsg";
			template.cpValue[11]=cImportMsg;

			template.cpName[12]="cCustomer";
			template.cpValue[12]=gcCustomer;

			template.cpName[13]="uResource";
			template.cpValue[13]=cuResource;

			template.cpName[14]="cZoneGetLink";
			char cZoneGetLink[256];
			sprintf(cZoneGetLink,"&cZone=%.99s&uView=%.16s&cCustomer=%.32s",gcZone,cuView,gcCustomer);
			template.cpValue[14]=cZoneGetLink;

			template.cpName[15]="cZoneView";
			char cZoneView[256];
			sprintf(cZoneView,"%.99s",gcZone);
			if(guCookieView)
				sprintf(cZoneView,"%.99s/%.31s",gcZone,ForeignKey("tView","cLabel",guCookieView));
			template.cpValue[15]=cZoneView;

			template.cpName[16]="";

			printf("\n<!-- Start htmlBulkOpPage(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlBulkOpPage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlBulkOpPage()


void BulkResourceImport(void)
{
	char cLine[512]={"ERROR"};
	char *cp;
	unsigned uZone;
	unsigned uZoneOwner;
	unsigned uNSSet;
	unsigned uView=2;
	unsigned uZoneCount=0,uResourceCount=0,uImportCount=0,uZoneFoundCount=0;
	unsigned uOnlyOncePerZone=1;
	static char cMsg[128];
	unsigned uLineNumber=0;
	unsigned uCreateZones=0;

	MYSQL_RES *res;
	MYSQL_ROW field;

	if(guCookieView) uView=guCookieView;	
	sprintf(cuView,"%u",uView);
	uZone=uGetuZone(gcCookieZone,cuView);
	

	if(!uZone)
	{	
		sprintf(cImportMsg,"Bulk Resource Record aborted. Zone=%s/%u/%s\n",gcCookieZone,uZone,cuView);
		htmlBulkOp();
	}

	sprintf(cImportMsg,"Bulk resource record import begin. Zone=%s/%u/%s\n",gcCookieZone,uZone,cuView);

	sprintf(gcZone,"%.99s",gcCookieZone);
	
	while(1)
	{

		uLineNumber++;
		sprintf(cLine,"%.255s",ParseTextAreaLines(cMassList));
		//ParseTextAreaLines() required break;
		if(cLine[0]==0) break;
		//Comments ignore
		if(cLine[0]=='#') continue;
		if(cLine[0]==';') continue;
		

		//Debug only
		//printf("cLine=(%s)\n",cLine);
		//Debug only line everything after this will
		//have no jobs created. Later add uDebug to ProcessRRLine()
		if(!strncmp(cLine,"cMode=debug;",12))
		{
			uDebug=1;
			sprintf(cMsg,"%u: uDebug=1\n",uLineNumber);
			strcat(cImportMsg,cMsg);
			continue;
		}
		if(!strncmp(cLine,"uCreateZones=1;",16))
		{
			uCreateZones=1;
			sprintf(cMsg,"%u: uCreateZones=1\n",uLineNumber);
			strcat(cImportMsg,cMsg);
			continue;
		}
		
		if(!strncmp(cLine,"uView=",6))
		{
			if((cp=strchr(cLine,';')))
				*cp=0;
			sscanf(cLine+6,"%u",&uView);
			if(uView>2 || uView<1)
				uView=2;
			sprintf(cMsg,"%u: uView=%u\n",uLineNumber,uView);
			sprintf(cuView,"%u",uView);
			strcat(cImportMsg,cMsg);
			continue;
		}
		
		if(!strncmp(cLine,"cZone=",6))
		{
			uOnlyOncePerZone=0;
			uZone=0;
			uZoneOwner=0;
			uNSSet=0;
			uZoneCount++;
			char cuSerial[100]={""};
			if((cp=strchr(cLine,';')))
				*cp=0;
			sprintf(gcZone,"%.99s",cLine+6);
			//strip trailing tabs or other white space.
			FQDomainName2(gcZone);	
			//Debug only
			sprintf(cMsg,"%u: cZone=(%s)\n",uLineNumber,gcZone);
			strcat(cImportMsg,cMsg);
			
			//First check tZone
			sprintf(gcQuery,"SELECT uZone,uNSSet,uOwner FROM tZone WHERE cZone='%s' AND uSecondaryOnly=0 AND uView=%u",gcZone,uView);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				sprintf(cMsg,"%u: %s\n",uLineNumber,mysql_error(&gMysql));
				strcat(cImportMsg,cMsg);
				continue;
			}
			res=mysql_store_result(&gMysql);
			if((field=mysql_fetch_row(res)))
			{
				sscanf(field[0],"%u",&uZone);
				sscanf(field[1],"%u",&uNSSet);
				sscanf(field[2],"%u",&uZoneOwner);
			}
			else if(uCreateZones)
			{
				time_t luClock;
				//
				//If we set the parameter to create non-existant zones, create it :) (using the default from zone.c)
				SerialNum(cuSerial);
				uNSSet=1;
				sprintf(gcQuery,"INSERT INTO tZone SET cZone='%s',uNSSet=%u,cHostmaster='nsadmin.unixservice.com',"
						"uSerial='%s',uExpire=604800,uRefresh=10800,uTTL=86400,uRetry=3600,uZoneTTL=86400,"
						"uMailServers=0,cMainAddress='',uView=%u,uRegistrar=0,cOptions='',uSecondaryOnly=0,"
						"uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
						gcZone
						,uNSSet
						,cuSerial
						,uView
						,uGetClient(gcOrgName) // Will create the zones for the ASP. Manual ownership update may be required later.
						,guLoginClient);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
				{
					sprintf(cMsg,"%u: %s\n",uLineNumber,mysql_error(&gMysql));
					strcat(cImportMsg,cMsg);
					continue;
				}
				uZone=mysql_insert_id(&gMysql);
				if(uZone)
				{
					iDNSLog(uZone,"tZone","New");
					sprintf(cMsg,"%u: Added zone %s\n",uLineNumber,gcZone);
					strcat(cImportMsg,cMsg);
				}
				else
					sprintf(cMsg,"%u: Zone add failure\n",uLineNumber);
				uZoneOwner=uGetClient(gcOrgName);

				time(&luClock);
				if(AdminSubmitJob("New",uNSSet,gcZone,0,luClock))
					htmlPlainTextError(mysql_error(&gMysql));

			}
			mysql_free_result(res);
			if(!uZone || !uZoneOwner || !uNSSet)
			{
				sprintf(cMsg,"%u: Valid %s not found. Skipping.\n",uLineNumber,gcZone);
				strcat(cImportMsg,cMsg);
				continue;

			}
			uOnlyOncePerZone=1;
			uZoneFoundCount++;
		}
		else
		{
			unsigned uResource=0;
			//A resource candidate line
			uResourceCount++;
			
			//If we have no defined zone keep on going.
			if(!uZone) continue;
			uZoneOwner=uGetZoneOwner(uZone);
			uResource=ProcessRRLine(uLineNumber,cLine,gcZone,uZone,uZoneOwner,uNSSet,guLoginClient,
				"idnsAdmin.BulkResourceImport()");
			if(uResource && (mysql_affected_rows(&gMysql)==1))
			{
				uImportCount++;
				
				if(uOnlyOncePerZone && !uDebug)
				{
					time_t luClock;
					uNSSet=uGetuNameServer(gcZone);
					if(!uNSSet)
						strcat(cImportMsg,"uGetuNameServer() failed!\n");
					//Submit job for first RR. Time for now + 5 minutes
					//This should allow for many more RRs to be added
					//here without complicating the code. A KISS hack?
					UpdateSerialNum(gcZone,cuView);
					if(mysql_affected_rows(&gMysql)<1)
						strcat(cImportMsg,"UpdateSerialNum() failed!\n");
					time(&luClock);
					luClock+=300;
					if(AdminSubmitJob("Modify",uNSSet,gcZone,0,luClock))
						strcat(cImportMsg,"AdminSubmitJob() failed!\n");
					else
						strcat(cImportMsg,"AdminSubmitJob() ok.\n");
					uOnlyOncePerZone=0;
				}
				iDNSLog(uResource,"tResource","New");
			}
		}

	}

	sprintf(cMsg,"Bulk resource record import ends.\n");
	strcat(cImportMsg,cMsg);
	sprintf(cMsg,"Zone creation: %u of %u tZone.cZone found.\nResource import: %u resource lines found %u imported.\n",
		uZoneFoundCount,uZoneCount,uResourceCount,uImportCount);
	strcat(cImportMsg,cMsg);
	htmlBulkOp();

}//void BulkResourceImport(void)


//Does not allow empty lines...this may need reviewing ;) to say the least.
char *ParseTextAreaLines(char *cTextArea)
{
	static unsigned uEnd=0;
	static unsigned uStart=0;
	static char cRetVal[512];

	uStart=uEnd;
	while(cTextArea[uEnd++])
	{
		if(cTextArea[uEnd]=='\n' || cTextArea[uEnd]=='\r' || cTextArea[uEnd]==0
				|| cTextArea[uEnd]==10 || cTextArea[uEnd]==13 )
		{
			if(cTextArea[uEnd]==0)
				break;

			cTextArea[uEnd]=0;
			sprintf(cRetVal,"%.511s",cTextArea+uStart);

			if(cRetVal[0]=='\n' || cRetVal[0]==13)
			{
				uStart=uEnd=0;
				return("");
			}

			if(cTextArea[uEnd+1]==10)
				uEnd+=2;
			else
				uEnd++;

			return(cRetVal);
		}
	}

	if(uStart!=uEnd)
	{
		sprintf(cRetVal,"%.511s",cTextArea+uStart);
		return(cRetVal);
	}

	uStart=uEnd=0;
	return("");

}//char *ParseTextAreaLines(char *cTextArea)


unsigned ProcessRRLine(unsigned uLineNumber,const char *cLine,char *cZoneName,const unsigned uZone,unsigned uCustId,const unsigned uNSSet,const unsigned uCreatedBy,const char *cComment)
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
	static char cMsg[128];
	char cResourceImportTable[256]={""};
	char *cp;
	
	sprintf(cResourceImportTable,"tResource");

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
			sprintf(cName,"\t");
	}
	else if(cLine[0]=='@')
	{
		sscanf(cLine,"@ %100s %255s %255s %255s %255s\n",
			cType,cParam1,cParam2,cParam3,cParam4);
		if(cPrevcName[0])
			sprintf(cName,"%.99s",cPrevcName);
		else	
			sprintf(cName,"\t");
	}
	else
	{
		sscanf(cLine,"%99s %100s %255s %255s %255s %255s\n",
			cName,cType,cParam1,cParam2,cParam3,cParam4);
		sprintf(cPrevcName,"%.99s",cName);
	}

	if(!cLine[0] || cLine[0]==';')
			return(0);

	if(cName[0]!='$')
	{
		//Shift left on inline TTL NOT $TTL directive
		sscanf(cType,"%u",&uTTL);
		if(uTTL>1 && uTTL<800000)
		{
			sprintf(cType,"%.255s",cParam1);
			sprintf(cParam1,"%.255s",cParam2);
			sprintf(cParam2,"%.255s",cParam3);
			sprintf(cParam3,"%.255s",cParam4);
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
			sprintf(cMsg,"%u: Error %s: Incorrect A format: %s\n",
					uLineNumber,cZoneName,cLine);
			strcat(cImportMsg,cMsg);
			return(0);
		}
		if(!strcmp(IPNumber(cParam1),"0.0.0.0"))
		{
			sprintf(cMsg,"%u: Incorrect A IP number format: %s\n",uLineNumber,cLine);
			strcat(cImportMsg,cMsg);
			return(0);
		}
		if(strchr(cName,'_'))
		{
			sprintf(cMsg,"%u: Error %s: cName (check-names): %s\n",
					uLineNumber,cZoneName,cLine);
			strcat(cImportMsg,cMsg);
			return(0);
		}

	}
	else if(!strcasecmp(cType,"CNAME"))
	{
		char cZone[254];
		char cName[254];
		uRRType=5;
		if(!cParam1[0] || cParam2[0])
		{
			sprintf(cMsg,"%u: error at %s Incorrect CNAME format: %s\n",
					uLineNumber,cZoneName,cLine);
			strcat(cImportMsg,cMsg);
			return(0);
		}
		if(cParam1[strlen(cParam1)-1]!='.')
		{
			sprintf(cMsg,"%u: warning Incorrect FQDN missing period: %s\n",uLineNumber,cParam1);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam1,cPrevOrigin);
				sprintf(cParam1,"%.255s",gcQuery);
				sprintf(cMsg,"%u: fixed CNAME RHS fixed from $ORIGIN: %s\n",uLineNumber,cParam1);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam1,cZoneName);
				sprintf(cParam1,"%.255s",gcQuery);
				sprintf(cMsg,"%u: fixed CNAME RHS fixed from cZoneName: %s\n",uLineNumber,cParam1);
			}
		}
		sprintf(cZone,"%.255s.",FQDomainName(cZoneName));
		sprintf(cName,"%.99s",FQDomainName(cParam1));
		if(strcmp(cName+strlen(cName)-strlen(cZone),cZone));
			sprintf(cMsg,"%u: warning CNAME RHS should probably end with zone\n",uLineNumber);
	}
	else if(!strcasecmp(cType,"NS"))
	{
		//MYSQL_RES *res;
		//char cNS[100];

		uRRType=2;
		if(!cParam1[0] || cParam2[0])
		{
			sprintf(cMsg,"%u: error %s: Incorrect NS format\n",
					uLineNumber,cZoneName);
			strcat(cImportMsg,cMsg);
			return(0);
		}
		if(cParam1[strlen(cParam1)-1]!='.')
		{
			sprintf(cMsg,"%u: warning %s Incorrect FQDN missing period\n",
					uLineNumber,cZoneName);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam1,cPrevOrigin);
				sprintf(cParam1,"%.255s",gcQuery);
				sprintf(cMsg,"%u: fixed NS RHS fixed from $ORIGIN: %s\n",uLineNumber,cParam1);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam1,cZoneName);
				sprintf(cParam1,"%.255s",gcQuery);
				sprintf(cMsg,"%u: fixed NS RHS fixed from cZoneName: %s\n",uLineNumber,cParam1);
			}
		}

		//Get rid of last period for check
		/*
		strcpy(cNS,cParam1);
		cNS[strlen(cNS)-1]=0;
		sprintf(gcQuery,"SELECT uNSSet FROM tNSSet WHERE uNSSet=%u AND ( (cList LIKE '%s MASTER%%') OR (cList LIKE '%%%s SLAVE%%'))",uNSSet,cNS,cNS);

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
		{
			sprintf(cMsg,"%u: error %s: %s\n",uLineNumber,cZoneName,mysql_error(&gMysql));
			strcat(cImportMsg,cMsg);
			return(0);
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)) 
		{
			sprintf(cMsg,"%u: error %s:NS RR Ignored. Part of uNSSet cList\n",
					uLineNumber,cZoneName);
			strcat(cImportMsg,cMsg);
			return(0);
		}
		mysql_free_result(res);
		*/
	}
	else if(!strcasecmp(cType,"MX"))
	{
		unsigned uMX=0;
		uRRType=3;
		if(!cParam1[0] || !cParam2[0] )
		{
			sprintf(cMsg,"%u: error %s: Missing MX param\n",
					uLineNumber,cZoneName);
			strcat(cImportMsg,cMsg);
			return(0);
		}
		sscanf(cParam1,"%u",&uMX);
		if(uMX<1 || uMX>99999)
		{
			sprintf(cMsg,"%u: error %s: Incorrect MX format\n",
					uLineNumber,cZoneName);
			strcat(cImportMsg,cMsg);
			return(0);
		}
		if(cParam2[strlen(cParam2)-1]!='.')
		{
			sprintf(cMsg,"%u: warning %s: Incorrect FQDN missing period\n",
					uLineNumber,cZoneName);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam2,cPrevOrigin);
				sprintf(cParam2,"%.255s",gcQuery);
				sprintf(cMsg,"%u: fixed: MX RHS fixed from $ORIGIN: %s\n",uLineNumber,cParam2);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam2,cZoneName);
				sprintf(cParam2,"%.255s",gcQuery);
				sprintf(cMsg,"%u: fixed: MX RHS fixed from cZoneName: %s\n",uLineNumber,cParam2);
			}
		}
	}
	else if(!strcasecmp(cType,"PTR"))
	{
		unsigned uFirstDigit=0;

		uRRType=7;
		if(!cParam1[0] || cParam2[0])
		{
			sprintf(cMsg,"%u: error %s: Incorrect PTR format\n",
					uLineNumber,cZoneName);
			strcat(cImportMsg,cMsg);
			return(0);
		}
		sscanf(cName,"%u.%*s",&uFirstDigit);
		if(!uFirstDigit)
		{
			//Check this rule again
			sprintf(cMsg,"%u: error %s: Incorrect PTR LHS should start with a non zero digit\n",
					uLineNumber,cZoneName);
			strcat(cImportMsg,cMsg);
			return(0);
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
			sprintf(cMsg,"%u: error %s: Incorrect TXT format: %s\n",
					uLineNumber,cZoneName,cParam1);
			strcat(cImportMsg,cMsg);
			return(0);
		}
		//debug only
		//printf("TXT: %s\n",cParam1);
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
			sprintf(cMsg,"%u: error %s: Incorrect SPF format: %s\n",
					uLineNumber,cZoneName,cParam1);
			strcat(cImportMsg,cMsg);
			return(0);
		}
		//debug only
		//printf("SPF: %s\n",cParam1);
	}

	//Special cases that should keep a static var for next line
	else if(!strcasecmp(cName,"$TTL"))
	{
		if(cType[0])
		{
			sscanf(cType,"%u",&uPrevTTL);
			//debug only
			sprintf(cMsg,"%u: $TTL changed: %u (%s %s)\n",
					uLineNumber,uPrevTTL,cType,cParam1);
			strcat(cImportMsg,cMsg);
			return(0);
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
			{
				//debug only
				sprintf(cMsg,"%u: $ORIGIN Changed: %s\n",
					uLineNumber,cPrevOrigin);
				strcat(cImportMsg,cMsg);
			}
			return(0);
		}
	}

	//Unrecognized lines
	//Current missing features -we know about and need: hinfo ignored
	else if(1)
	{
		sprintf(cMsg,"%u: error %s RR Not recognized %s\n",uLineNumber,cZoneName,cType);
		strcat(cImportMsg,cMsg);
		return(0);
	}
	
	if(!uTTL && uPrevTTL) uTTL=uPrevTTL;
	if(cPrevOrigin[0]) 
		sprintf(cNamePlus,"%.99s.%.99s",cName,cPrevOrigin);
	else
		sprintf(cNamePlus,"%.99s",cName);
	
	//
	//If we are importing a PTR, uZoneOwner=1, however, we must set uCustId based on tBlock data
	//
	//Loop trough the tBlock entries until we find a match. This logic will be unusable in case we have
	//lots of blocks. Must develop a function if possible like cIPv4InBlock(char *cIPV4)
	//Temporarily this code will be under the function used below:
	if(strstr(cZoneName,"in-addr.arpa"))
	{
		uCustId=uGetPTROwner(cZoneName,cName);
		if(!uCustId) uCustId=1; //Default to Root if no tBlock match ?
		//sprintf(cMsg,"%u: debug info: uCustId=%u\n",uLineNumber,uCustId);
		//strcat(cImportMsg,cMsg);
	}	
	
	if(uRRType==6 || uRRType==11)//TXT or SPF special case
	sprintf(gcQuery,"INSERT INTO %s SET  uZone=%u,cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',cComment='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())"
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
	sprintf(gcQuery,"INSERT INTO %s SET uZone=%u,cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',cParam2='%s',cComment='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())"
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
	{
		sprintf(cMsg,"%u: error %s: %s\n",uLineNumber,cZoneName,mysql_error(&gMysql));
		strcat(cImportMsg,cMsg);
	}
	return(mysql_insert_id(&gMysql));
}//void ProcessRRLine()


unsigned uGetPTROwner(char *cZone, char *cFirstDigit)
{
	//cZone=62.209.32.in-addr.arpa (e.g.)
	unsigned a,b,c,d,unused;
	unsigned uOwner=0;
	char cTmp[64]={""};
	char cIP[64]={""};
	MYSQL_RES *res;
	MYSQL_ROW field;

	if(strstr(cFirstDigit,"/"))
	{
		//1/10	records
		sscanf(cFirstDigit,"%u/%u",&d,&unused);
	}
	else if(strstr(cFirstDigit,"-"))
	{
		//5-25 records
		sscanf(cFirstDigit,"%u-%u",&d,&unused);
	}
	else
		sscanf(cFirstDigit,"%u",&d);
	
	sscanf(cZone,"%u.%u.%u.%s",&a,&b,&c,cTmp);
	sprintf(cIP,"%u.%u.%u.%u",c,b,a,d);

	sprintf(gcQuery,"SELECT cLabel,uOwner FROM tBlock");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	while((field=mysql_fetch_row(res)))
	{
		if(uIpv4InCIDR4(cIP,field[0]))
		{
			sscanf(field[1],"%u",&uOwner);
			return(uOwner);
		}
	}
	return(0);//No match

}//unsigned uGetPTROwner(char *cZone, char *cFirstDigit)

