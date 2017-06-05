/*
FILE 
	blukop.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See included LICENSE file.
PURPOSE
	vdnsOrg	program file.
	Bulk operations, i.e. multiple zone or other mass change functions.
*/

static char *cMassList={""};

#include "interface.h"

//
//Local only
char *ParseTextAreaLines(char *cTextArea);
void BulkResourceImport(void);
unsigned uGetZoneNameServer(char *cZone);

//zone.c
void ProcessRRLine(const char *cLine,char *cZoneName,const unsigned uZone,
	const unsigned uCustId,const unsigned uNameServer,const unsigned uCreatedBy,const char *cComment);
char *cGetViewLabel(void);
//resource.c
void UpdateSerialNum(char *cZone);


void ProcessBulkOpVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cMassList"))
			cMassList=entries[i].val;
		else if(!strcmp(entries[i].name,"uZone"))
			sscanf(entries[i].val,"%u",&guZone);
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
	htmlHeader("vndsOrg","Header");
	htmlBulkOpPage("vndsOrg","OrgBulkOp.Body");
	htmlFooter("Footer");

}//void htmlBulkOp(void)


void htmlBulkOpPage(char *cTitle, char *cTemplateName)
{
	if(cTemplateName[0])
	{
        	MYSQL_RES *res;
	        MYSQL_ROW field;

		TemplateSelectInterface(cTemplateName,uPLAINSET,uVDNSORGTYPE);
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			struct t_template template;
			char cuZone[16]={""};
			char cZone[256]={""};

			sprintf(cuZone,"%u",guZone);
			sprintf(cZone,"%.255s",ForeignKey("tZone","cZone",guZone));

			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
			
			template.cpName[1]="cCGI";
			template.cpValue[1]="vdnsOrg.cgi";
			
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

			template.cpName[8]="uZone";
			template.cpValue[8]=cuZone;

			template.cpName[9]="gcMessage";
			template.cpValue[9]=gcMessage;

			template.cpName[10]="cZone";
			template.cpValue[10]=cZone;

			template.cpName[11]="cZoneView";
			template.cpValue[11]=cGetViewLabel();

			template.cpName[12]="";

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
	unsigned uDebug=0;
	unsigned uZoneOwner;
	unsigned uNameServer;
	unsigned uResourceCount=0,uImportCount=0;
	static char cMsg[128];
	char cZone[256]={""};
#ifdef BULKIMPORT_JOB		
	unsigned uOnlyOncePerZone=1;
#endif
	//This also sets guView needed before template is spit out.
	//When and where is guZone read. Before here of course, but where?
	//I did not write this crap, but I have to fix it. :/
	//guZone has is supplied via form element, or get URI
	cGetViewLabel();

	sprintf(cZone,"%.255s",ForeignKey("tZone","cZone",guZone));
	//htmlPlainTextError(gcQuery);
	////printf("htmlMassResourceImport() start (uClient=%u)\n",uClient);
	while(1)
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

		if(strncmp(cLine,"cZone=",6))
		{
			//A resource candidate line
			uResourceCount++;
			//If we have no defined zone keep on going.
			if(!guZone) continue;
			uZoneOwner=guOrg;
			ProcessRRLine(cLine,cZone,guZone,uZoneOwner,uNameServer,guLoginClient,
				"vdnsOrg.BulkResourceImport()");
			if(gcMessage[0])
				htmlBulkOp();
			if(mysql_affected_rows(&gMysql)==1)
			{
				uImportCount++;
	
#ifdef BULKIMPORT_JOB		
				if(uOnlyOncePerZone && !uDebug)
				{
					time_t luClock;
					uNameServer=uGetuNameServer(guZone);
					//WTF does this mean:
					//Submit job for first RR. Time for now + 5 minutes
					//This should allow for many more RRs to be added
					//here without complicating the code. A KISS hack?
					UpdateSerialNum(cZone);
					time(&luClock);
					luClock+=300;
					OrgSubmitJob("Modify",uNameServer,cZone,0,luClock);
					uOnlyOncePerZone=0;
				}
#endif

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

