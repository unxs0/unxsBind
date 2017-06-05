/*
FILE 
	zone.c
	svn ID removed
AUTHOR
	(C) 2006-2009 Gary Wallis  and Hugo Urquiza for Unixservice
PURPOSE
	idnsOrg	
	program file.
*/

static char *cMassList={""};

#include "interface.h"

//
//Local only
char *ParseTextAreaLines(char *cTextArea);
void BulkResourceImport(void);
unsigned uGetZoneNameServer(char *cZone);


//zone.c
void ProcessRRLine(const char *cLine,char *cZoneName,const unsigned uZone,const unsigned uCustId,const unsigned uNameServer,const unsigned uCreatedBy,const char *cComment);
//resource.c
void UpdateSerialNum(char *cZone);

void ProcessBulkOpVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cMassList"))
			cMassList=entries[i].val;
		else if(!strcmp(entries[i].name,"cZone"))
			sprintf(gcZone,entries[i].val);
	}

}//void ProcessBulkOpVars(pentry entries[], int x)


void BulkOpGetHook(entry gentries[],int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		//if(!strcmp(gentries[i].name,"cBulkOp"))
		//	sprintf(gcBulkOp,"%.99s",gentries[i].val);
	}


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
				gcMessage="<blink>cMassList empty.</blink> Read manual for list format.";
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
	htmlHeader("idnsOrg","Header");
	htmlBulkOpPage("idnsOrg","OrgBulkOp.Body");
	htmlFooter("Footer");

}//void htmlBulkOp(void)


void htmlBulkOpPage(char *cTitle, char *cTemplateName)
{
	if(cTemplateName[0])
	{
        	MYSQL_RES *res;
	        MYSQL_ROW field;

		TemplateSelectInterface(cTemplateName,uPLAINSET,uIDNSORGTYPE);
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			struct t_template template;
			
			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
			
			template.cpName[1]="cCGI";
			template.cpValue[1]="idnsOrg.cgi";
			
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

			template.cpName[10]="";

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
	unsigned uDebug=0;
	unsigned uZone;
	unsigned uView=2;//external
	unsigned uZoneOwner;
	unsigned uNameServer;
	unsigned uResourceCount=0,uImportCount=0;
	unsigned uOnlyOncePerZone=1;
	static char cMsg[128];
	
	uZone=uGetuZone(gcZone);
	//htmlPlainTextError(gcQuery);
	////printf("htmlMassResourceImport() start (uClient=%u)\n",uClient);
	while(gcZone[0])
	{

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
			//printf("uDebug=1\n");
			continue;
		}

		if(!strncmp(cLine,"uView=",6))
		{
			if((cp=strchr(cLine,';')))
				*cp=0;
			sscanf(cLine+6,"%u",&uView);
			if(uView>2 || uView<1)
				uView=2;
			//printf("uView=%u\n",uView);
			continue;
		}
		if(strncmp(cLine,"cZone=",6))
		{
			//A resource candidate line
			uResourceCount++;
			//If we have no defined zone keep on going.
			if(!uZone) continue;
			uZoneOwner=guOrg;
			ProcessRRLine(cLine,gcZone,uZone,uZoneOwner,uNameServer,guLoginClient,
				"idnsOrg.BulkResourceImport()");
			if(gcMessage[0])
				htmlBulkOp();
			if(mysql_affected_rows(&gMysql)==1)
			{
				uImportCount++;
				
				if(uOnlyOncePerZone && !uDebug)
				{
					time_t luClock;
					uNameServer=uGetuNameServer(gcZone);
					//Submit job for first RR. Time for now + 5 minutes
					//This should allow for many more RRs to be added
					//here without complicating the code. A KISS hack?
					UpdateSerialNum(gcZone);
					time(&luClock);
					luClock+=300;
					OrgSubmitJob("Modify",uNameServer,gcZone,0,luClock);
					uOnlyOncePerZone=0;
				}

			}
		}

	}
	sprintf(cMsg,"%u resource lines found %u imported.\n",uResourceCount,uImportCount);
	//exit(0);
	gcMessage=cMsg;
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

