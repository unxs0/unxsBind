/*
FILE 
	zone.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See included LICENSE file.
PURPOSE
	vdnsOrg program file.
	Zone tab functions.
*/

static char cBlock[100]={""};
static char cMainAddress[17]={"0.0.0.0"};

static char cHostmaster[100]={""};
char *cHostmasterStyle="type_fields_off";

static char cNSs[1024]={""};
static char cNSSet[100]={""};
static char cuSerial[16]={""};

static char cuExpire[16]={""};
char *cuExpireStyle="type_fields_off";

static char cuRefresh[16]={""};
char *cuRefreshStyle="type_fields_off";

static char cuTTL[16]={""};
char *cuTTLStyle="type_fields_off";

static char cuRetry[16]={""};
char *cuRetryStyle="type_fields_off";

static char cuZoneTTL[16]={""};
char *cuZoneTTLStyle="type_fields_off";

static char cuNameServer[16]={""};

static char cuZone[16]={"0"};

static char cIPBlock[100]={""};
static unsigned uDelegationTTL=0;
static char *cNSList={""};

#include "interface.h"
#include <openisp/ucidr.h>

unsigned InMyBlocks(char *cIP);//resource.c
extern unsigned guBrowserFirefox;//main.c

//Job queue vars
static unsigned uJob=0;
//uMasterJob: Unique group job ID
static unsigned uMasterJob=0;
//cJob: Job description
static char cJob[101]={""};
//cZone: Zone to be operated on
static char cZone[101]={""};
//uNSSet: Key into tNSSet
static unsigned uNSSet=0;
//cTargetServer: Name of server that should process this job
static char cTargetServer[101]={""};
//uPriority: Job priority
static unsigned uPriority=0;
//uTime: Unix seconds threshold
static long uTime=0;
//cJobData: Aux job data
static char cJobData[101]={""};
static unsigned uOwner=0;
static unsigned uCreatedBy=0;

void Insert_tJob(void);
int SubmitSingleJob(const char *cCommand,const char *cZoneArg, unsigned uNSSetArg,
		const char *cTargetServer, unsigned uPriorityArg, time_t luTimeArg
	       			,unsigned *uMasterJob);
//Locals only :) (surfing in SCali)
void SerialNum(char *cSerial);
int OrgSubmitJob(char *cCommand,unsigned uNameServerArg,char *cZoneArg,
				unsigned uPriorityArg,long unsigned luTimeArg);
unsigned uGetuZone(char *cZone);
unsigned uGetuNameServer(unsigned guZone);
void SetZoneFieldsOn(void);
char *cGetPendingJobs(void);
unsigned uPTRInCIDR(unsigned uZone,char *cIPBlock);
unsigned uPTRInBlock(unsigned uZone,unsigned uStart,unsigned uEnd);
void htmlDelegationTool(void);
void UpdateSerialNum(void);
char *ParseTextAreaLines(char *cTextArea);
void PrepDelToolsTestData(unsigned uNumIPs);
unsigned idnsOnLineZoneCheck(void);
char *cGetViewLabel(void);


void ProcessZoneVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uZone"))
			sscanf(entries[i].val,"%u",&guZone);
		else if(!strcmp(entries[i].name,"cMainAddress"))
			sprintf(cMainAddress,"%.16s",IPNumber(entries[i].val));
		else if(!strcmp(entries[i].name,"cHostmaster"))
			sprintf(cHostmaster,"%.99s",FQDomainName(entries[i].val));
		else if(!strcmp(entries[i].name,"uExpire"))
			sprintf(cuExpire,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"uRefresh"))
			sprintf(cuRefresh,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"uTTL"))
			sprintf(cuTTL,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"uRetry"))
			sprintf(cuRetry,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"uZoneTTL"))
			sprintf(cuZoneTTL,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"uSerial"))
			sprintf(cuSerial,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"uNameServer"))
			sprintf(cuNameServer,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"cNSs"))
			sprintf(cNSs,"%.1023s",entries[i].val);
		else if(!strcmp(entries[i].name,"cNSSet"))
			sprintf(cNSSet,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cIPBlock"))
			sprintf(cIPBlock,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"uDelegationTTL"))
			sscanf(entries[i].val,"%u",&uDelegationTTL);
		else if(!strcmp(entries[i].name,"cNSList"))
			cNSList=entries[i].val;
	}

}//void ProcessZoneVars(pentry entries[], int x)


void ZoneGetHook(entry gentries[],int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uZone"))
			sscanf(gentries[i].val,"%u",&guZone);
	}

	if(guZone)
	{
		SelectZone();
		htmlZone();
	}
	else if(1)
	{
		htmlZone();
	}

}//void ZoneGetHook(entry gentries[],int x)


void ZoneCommands(pentry entries[], int x)
{
	if(!strcmp(gcPage,"Zone"))
	{
		ProcessZoneVars(entries,x);
		if(!strcmp(gcFunction,"Select Zone"))
		{
			SelectZone();
			htmlZone();
		}
		else if(!strcmp(gcFunction,"Modify SOA"))
		{
			SelectZone();//No hidden fields when disabled
			SetZoneFieldsOn();
			sprintf(gcModStep," Confirm");
			gcMessage="Modify data, review, then confirm. Any other action to cancel.";
			gcInputStatus[0]=0;
			htmlZone();
		}
		else if(!strcmp(gcFunction,"Modify SOA Confirm"))
		{
			UpdateZone();
			if(strcmp(gcMessage,"Zone Modified"))
			{
				sprintf(gcModStep," Confirm");
				gcInputStatus[0]=0;
				SetZoneFieldsOn();
			}
			else
			{
				time_t luClock;
				unsigned uNameServer=0;
				char cZone[256]={""};
				time(&luClock);

				sprintf(gcInputStatus,"disabled");
				sscanf(cuNameServer,"%u",&uNameServer);
				sprintf(cZone,"%.255s",ForeignKey("tZone","cZone",guZone));
				if(uNameServer)
				{
					if(OrgSubmitJob("Modify",uNameServer,cZone,0,luClock))
						htmlPlainTextError(mysql_error(&gMysql));
				}
				else
				{
					gcMessage="<blink>Contact admin: uNameServer error";
				}
				SelectZone();
			}
			htmlZone();
		}
		else if(!strcmp(gcFunction,"Delegation Tool"))
		{
			htmlDelegationTool();
		}
		else if(!strcmp(gcFunction,"Delegate Block"))
		{
			sprintf(gcModStep," Confirm");
			htmlDelegationTool();
		}
		else if(!strcmp(gcFunction,"Delegate Block Confirm"))
		{
			unsigned uA,uB,uC,uD,uE,uNumIPs;
			unsigned uMa,uMb,uMc;
			unsigned uIPBlockFormat;
			char cNS[100]={""};
			char cName[100]={""};
			char cParam1[100]={""};
			char cLogEntry[100]={""};
			unsigned uTTL=0;
			unsigned uNameServer=0;
			time_t luClock;
			char cZone[255]={""};

			sprintf(cZone,"%.255s",ForeignKey("tZone","cZone",guZone));

			SelectZone();

			sscanf(cuTTL,"%u",&uTTL);
			sscanf(cuNameServer,"%u",&uNameServer);

			if(!cIPBlock[0])
			{
				gcMessage="<blink>cIPBlock is a required value</blink>";
				sprintf(gcModStep," Confirm");
				htmlDelegationTool();
			}

			if(!cNSList[0])
			{
				gcMessage="<blink>cNSList is a required value</blink>";
				sprintf(gcModStep," Confirm");
				htmlDelegationTool();
			}
				
			//remove extra spaces or any other junk in CIDR
			sscanf(cIPBlock,"%s",gcQuery);
			sprintf(cIPBlock,"%.99s",gcQuery);
			
			sscanf(cZone,"%u.%u.%u.in-addr.arpa",&uMc,&uMb,&uMa);

			if(strchr(cIPBlock,'/'))
			{
				//cIPBlock is in CIDR format
				sscanf(cIPBlock,"%u.%u.%u.%u/%u",&uA,&uB,&uC,&uD,&uE);

				if((uA!=uMa) || (uB!=uMb) || (uC != uMc))
				{
					gcMessage="<blink>Error:</blink> The entered block is not inside the loaded zone.";
					sprintf(gcModStep," Confirm");
					htmlDelegationTool();
				}

				//CIDR only checks and calculations
				if(uE<24)
				{
					gcMessage="<blink>CIDR not supported</blink>";
					sprintf(gcModStep," Confirm");
					htmlDelegationTool();
				}
				
				uNumIPs=2<<(32-uE-1);
				uNumIPs--;

				if((uD+uNumIPs)>255)
				{
					gcMessage="<blink>CIDR range error</blink>";
					sprintf(gcModStep," Confirm");
					htmlDelegationTool();
				}
				
				//This check is superseded by named-checkzone below, we can safely comment it
				//it will say:
				//Error:  The RR has an error: CNAME and other data 
				//In case this condition is reached, clever isn't it?
				/*if(uPTRInCIDR(uZone,cIPBlock))
				{
					gcMessage="<blink>Delegation overlaps existing PTR records. Can't continue</blink>";
					sprintf(gcModStep," Confirm");
					htmlDelegationTool();
				}*/
				
				uIPBlockFormat=IP_BLOCK_CIDR;
			}
			else if(strchr(cIPBlock,'-'))
			{
				//cIPBlock is in dash format
				sscanf(cIPBlock,"%u.%u.%u.%u-%u",&uA,&uB,&uC,&uD,&uE);

				if((uA!=uMa) || (uB!=uMb) || (uC != uMc))
				{
					gcMessage="<blink>Error:</blink> The entered IP range is not inside the loaded zone.";
					sprintf(gcModStep," Confirm");
					htmlDelegationTool();
				}

				if(uE>255)
				{
					gcMessage="<blink>IP Block incorrect format</blink>";
					sprintf(gcModStep," Confirm");
					htmlDelegationTool();
				}
				
				if(uE<uD)
				{
					gcMessage="<blink>IP block range error</blink>";
					sprintf(gcModStep," Confirm");
					htmlDelegationTool();
				}
				/*
				if(uPTRInBlock(uZone,uD,uE))
				{
					gcMessage="<blink>Delegation overlaps existing PTR records. Can't continue</blink>";
					sprintf(gcModStep," Confirm");
					htmlDelegationTool();
				}
				*/	
				uNumIPs=uE-uD;
				uIPBlockFormat=IP_BLOCK_DASH;
			}
			
			//basic sanity check (common)
			
			//add check
			if(!uA)
			{
				gcMessage="<blink>IP Block incorrect format</blink>";
				sprintf(gcModStep," Confirm");
				htmlDelegationTool();
			}
			if((uA>255)||(uB>255)||(uC>255)||(uD>255))
			{
				gcMessage="<blink>IP Block incorrect format</blink>";
				sprintf(gcModStep," Confirm");
				htmlDelegationTool();
			}
			if(uDelegationTTL>uTTL)
			{
				gcMessage="<blink>uDelegationTTL out iof range</blink>";
				sprintf(gcModStep," Confirm");
				htmlDelegationTool();
			}
			if(!uDelegationTTL)
				uDelegationTTL=uTTL;
			
			PrepDelToolsTestData(uNumIPs);
			if(idnsOnLineZoneCheck())
			{
				sprintf(gcModStep," Confirm");
				htmlDelegationTool();
			}
			while(1)
			{
				sprintf(cNS,"%.99s",ParseTextAreaLines(cNSList));

				if(!cNS[0]) break;
				
				if(cNS[strlen(cNS)-1]!='.') strcat(cNS,"."); //Append end dot if not entered by user

				if(uIPBlockFormat==IP_BLOCK_CIDR)
					sprintf(cName,"%u/%u",uD,uE);
				else if(uIPBlockFormat==IP_BLOCK_DASH)
					sprintf(cName,"%u-%u",uD,uE);

				sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%s',uTTL=%u,uRRType=2,"
						"cParam1='%s',cComment='Delegation (%s)',uOwner=%u,uCreatedBy=%u,"
						"uCreatedDate=UNIX_TIMESTAMP(NOW())",
						guZone
						,cName
						,uDelegationTTL
						,cNS
						,cIPBlock
						,guOrg
						,guLoginClient);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
			}

			//$GENERATE 0-255 $ CNAME $.0/24.21.68.217.in-addr.arpa.
			if(uIPBlockFormat==IP_BLOCK_CIDR)
				sprintf(cParam1,"$.%u/%u.%u.%u.%u.in-addr.arpa.",
						uD
						,uE
						,uC
						,uB
						,uA
				       );
			else if(uIPBlockFormat==IP_BLOCK_DASH)
			{
				sprintf(cParam1,"$.%u-%u.%u.%u.%u.in-addr.arpa.",
						uD
						,uE
						,uC
						,uB
						,uA
				       );
			}
			
			sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,"
					"cName='$GENERATE %u-%u $',uRRType=5,cParam1='%s',"
					"cComment='Delegation (%s)',uOwner=%u,uCreatedBy=%u,"
					"uCreatedDate=UNIX_TIMESTAMP(NOW())",
					guZone
					,uD
					,(uD+uNumIPs)
					,cParam1
					,cIPBlock
					,guOrg
					,guLoginClient);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
			UpdateSerialNum();	
			if(OrgSubmitJob("Modify",uNameServer,cZone,0,luClock))
				htmlPlainTextError(mysql_error(&gMysql));
			

			sprintf(cLogEntry,"%s Delegation",cIPBlock);
			iDNSLog(guZone,"tZone",cLogEntry);
			gcMessage="IP block delegation done";
			htmlDelegationTool();
			
		}
		else if(!strcmp(gcFunction,"Remove Del."))
		{
			sprintf(gcNewStep," Confirm");
			htmlDelegationTool();
		}
		else if(!strcmp(gcFunction,"Remove Del. Confirm"))
		{
			unsigned uNameServer=0;
			time_t luClock;
			char cLogEntry[100]={""};
			char cZone[255]={""};

			time(&luClock);
			sprintf(cZone,"%.255s",ForeignKey("tZone","cZone",guZone));

			SelectZone();	
			sscanf(cuNameServer,"%u",&uNameServer);
			
			if(!cIPBlock[0])
			{
				gcMessage="<blink>cIPBlock is a required value</blink>";
				sprintf(gcNewStep," Confirm");
				htmlDelegationTool();
			}
			sprintf(gcQuery,"DELETE FROM tResource WHERE uZone=%u AND cComment='Delegation (%s)'",
				guZone,cIPBlock);
			mysql_query(&gMysql,gcQuery);
			if(!mysql_affected_rows(&gMysql))
			{
				gcMessage="<blink>No delegation removed</blink>";
				htmlDelegationTool();
			}
			
			if(mysql_errno(&gMysql))
				 htmlPlainTextError(mysql_error(&gMysql));

			UpdateSerialNum();
			if(OrgSubmitJob("Modify",uNameServer,cZone,0,luClock))
				htmlPlainTextError(mysql_error(&gMysql));
			
			sprintf(cLogEntry,"%s Delegation Removal",cIPBlock);
			iDNSLog(guZone,"tZone",cLogEntry);
			gcMessage="IP block delegation removed";
			htmlDelegationTool();	
		}
		else if(guZone)
		{
			SelectZone();
			htmlZone();
		}
		htmlZone();
	}

}//void ZoneCommands(pentry entries[], int x)


void htmlZone(void)
{
	htmlHeader("vdnsOrg","Header");
	htmlZonePage("vdnsOrg","VZone.Body");
	htmlFooter("Footer");

}//void htmlZone(void)


void htmlZonePage(char *cTitle, char *cTemplateName)
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
			char cuDelegationTTL[10]={""};
			char cZone[256]={""};

			sprintf(cZone,"%.255s",ForeignKey("tZone","cZone",guZone));

			if(uDelegationTTL)
				sprintf(cuDelegationTTL,"%u",uDelegationTTL);
			
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
			//This is required for hacked on delegation tool.
			if(guZone && (!cuZone[0] || cuZone[0]=='0'))
				sprintf(cuZone,"%u",guZone);
			template.cpValue[8]=cuZone;

			template.cpName[9]="gcMessage";
			template.cpValue[9]=gcMessage;

			template.cpName[10]="gcInputStatus";
			template.cpValue[10]=gcInputStatus;
				

			//
			//Zone record vars
			template.cpName[11]="cNSSet";
			template.cpValue[11]=cNSSet;

			template.cpName[12]="cNSs";
			template.cpValue[12]=cNSs;

			template.cpName[13]="cMainAddress";
			template.cpValue[13]=cMainAddress;

			template.cpName[14]="cZone";
			template.cpValue[14]=cZone;

			template.cpName[15]="cHostmaster";
			template.cpValue[15]=cHostmaster;

			template.cpName[16]="uSerial";
			template.cpValue[16]=cuSerial;

			template.cpName[17]="uExpire";
			template.cpValue[17]=cuExpire;

			template.cpName[18]="uRefresh";
			template.cpValue[18]=cuRefresh;

			template.cpName[19]="uTTL";
			template.cpValue[19]=cuTTL;

			template.cpName[20]="uRetry";
			template.cpValue[20]=cuRetry;

			template.cpName[21]="uZoneTTL";
			template.cpValue[21]=cuZoneTTL;

			template.cpName[22]="cAddRRStatus";
			if(guZone)
				template.cpValue[22]="";
			else
				template.cpValue[22]="disabled";

			template.cpName[23]="cModSOAStatus";
			if(!guZone || strstr(cZone,"in-addr.arpa"))
				template.cpValue[23]="disabled";
			else
				template.cpValue[23]="";

			template.cpName[24]="uNameServer";
			template.cpValue[24]=cuNameServer;
			
			template.cpName[25]="cHostmasterStyle";
			template.cpValue[25]=cHostmasterStyle;

			template.cpName[26]="cuExpireStyle";
			template.cpValue[26]=cuExpireStyle;

			template.cpName[27]="cuRefreshStyle";
			template.cpValue[27]=cuRefreshStyle;

			template.cpName[28]="cuTTLStyle";
			template.cpValue[28]=cuTTLStyle;

			template.cpName[29]="cuRetryStyle";
			template.cpValue[29]=cuRetryStyle;

			template.cpName[30]="cuZoneTTLStyle";
			template.cpValue[30]=cuZoneTTLStyle;
			
			template.cpName[31]="cPendingJobs";
			if(guZone)
				template.cpValue[31]=cGetPendingJobs();
			else
				template.cpValue[31]="No zone selected";

			template.cpName[32]="cIPBlock";
			template.cpValue[32]=cIPBlock;

			template.cpName[33]="uDelegationTTL";
			template.cpValue[33]=cuDelegationTTL;

			template.cpName[34]="cNSList";
			template.cpValue[34]=cNSList;
				
			template.cpName[35]="gcNewStep";
			template.cpValue[35]=gcNewStep;

			template.cpName[36]="cDelToolStatus";
			if(strstr(cZone,"in-addr.arpa"))
				template.cpValue[36]="";
			else
				template.cpValue[36]="disabled";

			template.cpName[37]="cZoneView";
			template.cpValue[37]=cGetViewLabel();

			template.cpName[38]="";

			printf("\n<!-- Start htmlZonePage(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlZonePage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlZonePage()


void funcSelectZone(FILE *fp)
{
	register int i;
	MYSQL_RES *res;
	MYSQL_ROW field;
	MYSQL_RES *res2;
	MYSQL_ROW field2;
	unsigned uCount=1;
	unsigned a=0,b=0,c=0,d=0,e=0;
	unsigned uZone=0;
	char cZone[100];
	char cPrevZone[100]="ERROR";

	fprintf(fp,"<!-- funcSelectZone(fp) Start -->\n");

	//Normal zones
	sprintf(gcQuery,"SELECT uZone,cZone,tView.cLabel FROM tZone,tView WHERE tZone.uView=tView.uView AND "
			"cZone NOT LIKE '%%.arpa' AND (tZone.uOwner=%u OR tZone.uOwner=%u) AND uSecondaryOnly=0 "
			"ORDER BY cZone LIMIT 501",guLoginClient,guOrg);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	fprintf(fp,"<select title='Select the zone you want to load with this dropdown'"
			" name=uZone class=type_textarea onChange=");
	//if(guBrowserFirefox)
	//	fprintf(fp,"'changePage(this.form.cZone)'>\n");
	//else
		fprintf(fp,"'submit()'>\n");
	fprintf(fp,"<option>---</option>");
	while((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&uZone);
		fprintf(fp,"<option value=%s",field[0]);
		if(guZone==uZone)
			fprintf(fp," selected");
		if((uCount++)<=500)
			fprintf(fp,">%s [%s]</option>",field[1],field[2]);
		else
			fprintf(fp,">LIMIT REACHED CONTACT sysadmin</option>");
	}
	mysql_free_result(res);

	//Empty arpa zones with class C that falls into customer block
	//Example Customer block 212.111.47.12/30 should add (if it exists)
	//47.111.212.in-addr.arpa tZone.cZone
	//Plan get a block expand into possible class Cs. Ex.
	//213.52.164.0/24 would expand into 
	//231.52.164.0
	//192.168.0.0/21 would expand into
	//192.168.0.0/24 through 192.168.7.0/24
	//uCount=1;
	sprintf(gcQuery,"SELECT DISTINCT tBlock.cLabel FROM tBlock WHERE "
			"(tBlock.uOwner=%u OR tBlock.uOwner=%u) ORDER BY cLabel LIMIT 301",guLoginClient,guOrg);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u.%u.%u.%u/%u",&a,&b,&c,&d,&e);
		sprintf(cZone,"%u.%u.%u.in-addr.arpa",c,b,a);
		if(strcmp(cZone,cPrevZone))
		{
			sprintf(cPrevZone,"%.99s",cZone);

			if(a==0 || e==0 || b>255 || c>255 || d>255 
					|| a>255 || b>255 || e<21) continue;
			switch(e)
			{
				case 23:
				case 22:
				case 21:
				//Expand these three cases with basic CIDR math
				for(i=0;i<(2<<(24-e-1));i++)
				{
					sprintf(cZone,"%u.%u.%u.in-addr.arpa",c+i,b,a);
					if(!uGetuZone(cZone))
						continue;
					
					sprintf(gcQuery,"SELECT uZone,cZone,tView.cLabel FROM tZone,tView WHERE "
							"tZone.uView=tView.uView AND cZone='%s'",cZone);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
					res2=mysql_store_result(&gMysql);
					while((field2=mysql_fetch_row(res2)))
					{
						sscanf(field2[0],"%u",&uZone);
						fprintf(fp,"<option value=%s",field2[0]);
						if(guZone==uZone)
							fprintf(fp," selected");
						fprintf(fp,">%s [%s]</option>",field2[1],field2[2]);
					}
				}
				break;
				default:
				//24 or smaller see if continue above
				//Single class C rev zone
				sprintf(cZone,"%u.%u.%u.in-addr.arpa",c,b,a);
				if(!uGetuZone(cZone))
						break;
				sprintf(gcQuery,"SELECT uZone,cZone,tView.cLabel FROM tZone,tView WHERE "
						"tZone.uView=tView.uView AND cZone='%s'",cZone);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
				res2=mysql_store_result(&gMysql);
				while((field2=mysql_fetch_row(res2)))
				{
					sscanf(field2[0],"%u",&uZone);
					fprintf(fp,"<option value=%s",field2[0]);
					if(guZone==uZone)
						fprintf(fp," selected");
					fprintf(fp,">%s [%s]</option>",field2[1],field2[2]);
				}
				break;
			}
		}//if distinct
	}
	mysql_free_result(res);

	//ip6.arpa zones
	sprintf(gcQuery,"SELECT uZone,cZone,tView.cLabel FROM tZone,tView WHERE tZone.uView=tView.uView AND "
			"cZone LIKE '%%.ip6.arpa' AND (tZone.uOwner=%u OR tZone.uOwner=%u) AND uSecondaryOnly=0 "
			"ORDER BY cZone LIMIT 501",guLoginClient,guOrg);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&uZone);
		fprintf(fp,"<option value=%s",field[0]);
		if(guZone==uZone)
			fprintf(fp," selected");
		if((uCount++)<=500)
			fprintf(fp,">%s [%s]</option>",field[1],field[2]);
		else
			fprintf(fp,">LIMIT REACHED CONTACT sysadmin</option>");
	}
	mysql_free_result(res);

	fprintf(fp,"</select>\n");

	fprintf(fp,"<!-- funcSelectZone(fp) End -->\n");

}//void funcSelectZone(FILE *fp)


void funcSelectBlock(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uCount=1;

	fprintf(fp,"<!-- funcSelectBlock(fp) Start -->\n");
	sprintf(gcQuery,"SELECT DISTINCT cLabel FROM tBlock WHERE (uOwner=%u OR uOwner=%u) ORDER BY cLabel LIMIT 301",
		guLoginClient,guOrg);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);
	fprintf(fp,"<select name=cBlock class=type_textarea size=%u disabled>\n",
			(unsigned)mysql_num_rows(res));
	while((field=mysql_fetch_row(res)))
	{
		fprintf(fp,"<option ");
		if(!strcmp(cBlock,field[0]))
			fprintf(fp,"selected");
		if((uCount++)<=300)
			fprintf(fp,">%s</option>",field[0]);
		else
			fprintf(fp,">LIMIT REACHED CONTACT sysadmin</option>");
		
	}

	fprintf(fp,"<!-- funcSelectBlock(fp) End -->\n");

	mysql_free_result(res);

}//void funcSelectBlock(FILE *fp)


void funcSelectSecondary(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uCount=1;
	unsigned uRows=0;

	fprintf(fp,"<!-- funcSelectSecondary(fp) Start -->\n");
	sprintf(gcQuery,"SELECT DISTINCT cZone FROM tZone WHERE (uOwner=%u OR uOwner=%u) AND uSecondaryOnly=1"
			" ORDER BY cZone LIMIT 301",guLoginClient,guOrg);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);
	uRows=mysql_num_rows(res);
	if(uRows)
	{
		if(uRows>20) uRows=20;
		fprintf(fp,"<select name=cBlock class=type_textarea size=%u disabled>\n",uRows);
		while((field=mysql_fetch_row(res)))
		{
			fprintf(fp,"<option ");
			if(!strcmp(cBlock,field[0]))
				fprintf(fp,"selected");
			if((uCount++)<=300)
				fprintf(fp,">%s</option>",field[0]);
			else
				fprintf(fp,">LIMIT REACHED CONTACT sysadmin</option>");
		
		}
	}
	else
	{
		fprintf(fp,"You have no secondary service only zones");
	}

	fprintf(fp,"<!-- funcSelectSecondary(fp) End -->\n");

	mysql_free_result(res);

}//void funcSelectSecondary(FILE *fp)


void funcRRs(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cLocalParam1[256]={""};
	
	fprintf(fp,"<!-- funcRRs(fp) Start -->\n");

	sprintf(gcQuery,"SELECT tResource.uResource,tZone.uZone,IF(STRCMP(tResource.cName,''),"
			"tResource.cName,'@'),tResource.uTTL,tRRType.cLabel,tResource.cParam1,"
			"tResource.cParam2,tResource.cComment FROM tResource,tRRType,tZone "
			"WHERE tResource.uZone=tZone.uZone AND tResource.uRRType=tRRType.uRRType "
			"AND (tResource.uOwner=%u OR tResource.uOwner=%u) AND "
			"tZone.uZone='%u' ORDER BY tResource.uRRType,ABS(tResource.cName)",
			guLoginClient
			,guOrg
			,guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{

		if(!field[2][0] || field[2][0]=='\t')
			strcpy(field[2],"@");
		
		if((!strcmp(field[4],"TXT")) && strlen(field[5])>10)
		{
			sprintf(cLocalParam1,"%.10s",field[5]);
			strcat(cLocalParam1,"...");
		}
		else
			sprintf(cLocalParam1,"%.255s",field[5]);
		
		fprintf(fp,"<tr>\n");
		fprintf(fp,"<td valign=top><a class=darkLink href=vdnsOrg.cgi?gcPage=Resource&uResource=%s"
			"&uZone=%s>%s</a></td><td valign=top>%s</td><td valign=top>%s</td><td valign=top>%s</td>"
			"<td valign=top>%s</td><td valign=top>%s</td>\n",
				field[0],
				field[1],
				field[2],
				field[3],
				field[4],
				cLocalParam1,
				field[6],
				field[7]);
		fprintf(fp,"</tr>\n");
		
	}
	mysql_free_result(res);


	fprintf(fp,"<!-- funcRRs(fp) End -->\n");


}//void funcRRs(FILE *fp)

//
//HTML func section ends


void SelectZone(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT tZone.uZone,tZone.cMainAddress,tZone.cHostmaster,'list goes here',"
			"tNSSet.cLabel,tZone.uSerial,tZone.uExpire,tZone.uRefresh,tZone.uTTL,"
			"tZone.uRetry,tZone.uZoneTTL,tNSSet.uNSSet FROM tZone,tNSSet WHERE "
			"tZone.uNSSet=tNSSet.uNSSet AND tZone.uZone='%u' AND tZone.uSecondaryOnly=0 "
			,guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sprintf(cuZone,"%.10s",field[0]);
		sprintf(cMainAddress,"%.16s",field[1]);
		sprintf(cHostmaster,"%.99s",field[2]);
		sprintf(cNSs,"%.1023s",field[3]);
		sprintf(cNSSet,"%.99s",field[4]);
		sprintf(cuSerial,"%.15s",field[5]);
		sprintf(cuExpire,"%.15s",field[6]);
		sprintf(cuRefresh,"%.15s",field[7]);
		sprintf(cuTTL,"%.15s",field[8]);
		sprintf(cuRetry,"%.15s",field[9]);
		sprintf(cuZoneTTL,"%.15s",field[10]);
		sprintf(cuNameServer,"%.15s",field[11]);

		if(!gcMessage[0]) gcMessage="Zone Selected";
	}
	else
	{
		cuZone[0]=0;
		cHostmaster[0]=0;
		cNSs[0]=0;
		cNSSet[0]=0;
		cuSerial[0]=0;
		cuExpire[0]=0;
		cuRefresh[0]=0;
		cuTTL[0]=0;
		cuRetry[0]=0;
		cuZoneTTL[0]=0;
		cuNameServer[0]=0;
		gcMessage="<blink>No Zone Selected</blink>";
	}
	
	mysql_free_result(res);
	
}//void SelectZone(void)


void UpdateZone(void)
{
	unsigned uSerial;
	unsigned uExpire=0;
	unsigned uRefresh=0;
	unsigned uTTL=0;
	unsigned uRetry=0;
	unsigned uZoneTTL=0;
	long unsigned luYearMonDay=0;

	if(!guZone)
	{
		gcMessage="<blink>Unknown/empty zone was not modified</blink>";
		return;
	}

	/*
	if(cMainAddress[0] && cMainAddress[0]!='0' )
	{
		if(!InMyBlocks(cMainAddress))
		{
			gcMessage="Optional main A IP number not in any of your IP blocks";
			return;
		}
	}
	*/

	if(!cHostmaster[0])
	{
		gcMessage="<blink>Must provide SOA Hostmaster Email (@ is first .).</blink>";
		cHostmasterStyle="type_fields_req";
		return;
	}
	if(strlen(cHostmaster)<6)
	{
		gcMessage="<blink>FQDN Hostmaster required. Must specify 'email.' in front.</blink>";
		cHostmasterStyle="type_fields_req";
		return;
	}
	if(!strchr(cHostmaster,'.'))
	{
		gcMessage="<blink>Must use a FQDN for Hostmaster with 'email.' in front</blink>";
		cHostmasterStyle="type_fields_req";
		return;
	}
	if(cHostmaster[strlen(cHostmaster)-1]=='.')
	{
		gcMessage="<blink>email.FQDN Hostmaster should not end with a period.</blink>";
		cHostmasterStyle="type_fields_req";
		return;
	}
	
	

	sscanf(cuSerial,"%u",&uSerial);
	SerialNum(cuSerial);
	sscanf(cuSerial,"%lu",&luYearMonDay);

	//Typical year month day and 99 changes per day max
	//to stay in correct date format. Will still increment.
	if(uSerial>=luYearMonDay)
	{
		uSerial++;
		sprintf(cuSerial,"%u",uSerial);
	}
	else
	{
		sscanf(cuSerial,"%u",&uSerial);
	}


	sscanf(cuExpire,"%u",&uExpire);
	sscanf(cuRefresh,"%u",&uRefresh);
	sscanf(cuTTL,"%u",&uTTL);
	sscanf(cuRetry,"%u",&uRetry);
	sscanf(cuZoneTTL,"%u",&uZoneTTL);

	if(!uExpire || uExpire>2419200)
	{
		gcMessage="<blink>Expire TTL out of range</blink>";
		cuExpireStyle="type_fields_req";
		return;
	}
	if(!uRefresh || uRefresh>100000)
	{
		gcMessage="<blink>Refresh TTL out of range</blink>";
		cuRefreshStyle="type_fields_req";
		return;
	}
	if(!uTTL || uTTL>100000)
	{
		gcMessage="<blink>Default TTL out of range</blink>";
		cuTTLStyle="type_fields_req";
		return;
	}
	if(!uRetry || uRetry>100000)
	{
		gcMessage="<blink>Retry TTL out of range</blink>";
		cuRetryStyle="type_fields_req";
		return;
	}
	if(!uZoneTTL || uZoneTTL>100000)
	{
		gcMessage="<blink>Negative TTL out of range</blink>";
		cuZoneTTLStyle="type_fields_req";
		return;
	}
	//Sanity checks from BIND source db_load.c
	if(  uExpire < (uRefresh + uRetry) )
	{
		gcMessage="<blink>SOA expire value is less than SOA refresh+retry</blink>";
		cuExpireStyle="type_fields_req";
		return;
	}
	if(  uExpire < (uRefresh + (10 * uRetry)) )
	{
		gcMessage="<blink>SOA expire value is less than refresh + 10 * retry</blink>";
		cuExpireStyle="type_fields_req";
		return;
	}
	if(  uExpire < (7 * 24 * 3600) )
	{
		gcMessage="<blink>SOA expire value is less than 7 days</blink>";
		cuExpireStyle="type_fields_req";
		return;
	}
	if(  uExpire > ( 183 * 24 * 3600) )
	{
		gcMessage="<blink>SOA expire value is greater than 6 months</blink>";
		cuExpireStyle="type_fields_req";
		return;
	}
	if(  uRefresh < (uRetry * 2))
	{
		gcMessage="<blink>SOA refresh value is less than 2 * retry</blink>";
		cuRetryStyle="type_fields_req";
		return;
	}

	sprintf(gcQuery,"UPDATE tZone SET uSerial=%u,uExpire=%u,uRefresh=%u,uTTL=%u,uRetry=%u,"
			"uZoneTTL=%u,cHostmaster='%s',uModBy=%u,"
			"uModDate=UNIX_TIMESTAMP(NOW()) WHERE uZone='%u' AND uSecondaryOnly=0",
			uSerial,
			uExpire,
			uRefresh,
			uTTL,
			uRetry,
			uZoneTTL,
			cHostmaster,
			guLoginClient,
			guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	if(mysql_affected_rows(&gMysql)==1)
	{
		gcMessage="Zone Modified";
		iDNSLog(guZone,"tZone","Mod");
	}
	else
	{
		gcMessage="<blink>Zone Not Modified</blink>";
	}
	
}//void UpdateZone(void)


void SerialNum(char *cSerial)
{
	time_t clock;
		
	//Make new today serial number
	time(&clock);
	strftime(cSerial,12,"%Y%m%d00",localtime(&clock));

}//void SerialNum(char *cSerial)


char *cGetPendingJobs(void)
{
	//
	//This zone returns the number of jobs pending for the selected zone
	//We get the selected zone from the global variable guZone
	//A nice idea for this function would be to somehow add an ETA
	//calculation for all the pending jobs 
	
	static char cPendingJobs[20]={""};
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cZone[256]={""};

	sprintf(cZone,"%.255s",ForeignKey("tZone","cZone",guZone));
	sprintf(gcQuery,"SELECT COUNT(uJob) FROM tJob WHERE cZone='%s'",cZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);

	if((field=mysql_fetch_row(res)))
		sprintf(cPendingJobs,"%s pending job(s)",field[0]);
	
	return(cPendingJobs);
			
}//char *cGetPendingJobs(void)


int OrgSubmitJob(char *cCommand,unsigned uNSSetArg,char *cZoneArg,
				unsigned uPriorityArg,long unsigned luTimeArg)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	static unsigned uMasterJob=0;

	sprintf(gcQuery,"SELECT tNS.cFQDN,tNSType.cLabel,tServer.cLabel FROM tNSSet,tNS,tNSType,tServer"
			" WHERE tNSSet.uNSSet=tNS.uNSSet AND tNS.uServer=tServer.uServer AND"
			" tNS.uNSType=tNSType.uNSType AND tNSSet.uNSSet=%u ORDER BY tNSType.uNSType",
			uNSSetArg);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	
	while((field=mysql_fetch_row(res)))
	{
		//cTargetServer is really the target NS with the type qualification
		//Do not confuse with tServer based partions of zones and tNS NSs.
		sprintf(cTargetServer,"%.64s %.32s",field[0],field[1]);
		sprintf(cJobData,"%.99s",field[2]);

		if(SubmitSingleJob(cCommand,cZoneArg,uNSSetArg,
				cTargetServer,uPriorityArg,luTimeArg,&uMasterJob))
					htmlPlainTextError(mysql_error(&gMysql));
	}//if field
	mysql_free_result(res);

	return(0);

}//int SubmitJob()


int SubmitSingleJob(const char *cCommand,const char *cZoneArg, unsigned uNSSetArg,
		const char *cTargetServer, unsigned uPriorityArg, time_t luTimeArg
	       			,unsigned *uMasterJob)
{
	MYSQL_RES *res;
	

	//Don't submit equivalent jobs: Study this issue further:
	//uPriority uTime issues?
	//Have job queue processor handle wierd things like?
	sprintf(gcQuery,"SELECT uJob FROM tJob WHERE cJob='%s' AND cZone='%s' AND uNSSet=%u AND cTargetServer='%s'",
		cCommand
		,cZoneArg
		,uNSSetArg
		,cTargetServer);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res)==0)
	{
		uJob=0;//auto increment
		strcpy(cJob,cCommand);
		strcpy(cZone,cZoneArg);
		uNSSet=uNSSetArg;
		uPriority=uPriorityArg;
		uTime=luTimeArg;
		uCreatedBy=guLoginClient;
		uOwner=guOrg;
		Insert_tJob();
		if(mysql_error(&gMysql)[0])
		{
			mysql_free_result(res);
			return(1);
		}

		if(*uMasterJob == 0)
		{
			uJob=*uMasterJob=mysql_insert_id(&gMysql);
			if(!strstr(cTargetServer,"MASTER"))
				htmlPlainTextError("MASTER NS must be first job submitted");
		}
		else
		{
			uJob=mysql_insert_id(&gMysql);
		}
	
		sprintf(gcQuery,"UPDATE tJob SET uMasterJob=%u WHERE uJob=%u",*uMasterJob,uJob);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
		if(mysql_affected_rows(&gMysql)==0)
		{
			//debug only
			sprintf(gcQuery,"uMasterJob %u",*uMasterJob);
			htmlPlainTextError(gcQuery);
		}
	}
	mysql_free_result(res);

	return(0);

}//int SubmitSingleJob()


void Insert_tJob(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tJob SET uJob=%u,uMasterJob=%u,cJob='%s',cZone='%s',uNSSet=%u,cTargetServer='%s',"
			"uPriority=%u,uTime=%lu,cJobData='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uJob
			,uMasterJob
			,TextAreaSave(cJob)
			,TextAreaSave(cZone)
			,uNSSet
			,TextAreaSave(cTargetServer)
			,uPriority
			,uTime
			,TextAreaSave(cJobData)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tJob(void)


unsigned uGetuZone(char *cZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uZone=0;
	char cQuery[512];

	sprintf(cQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=%u",cZone,guView);
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uZone);
	mysql_free_result(res);

	return(uZone);

}//unsigned uGetuZone(char *cZone)


unsigned uGetuNameServer(unsigned guZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uNameServer=0;
	char cQuery[512];

	sprintf(cQuery,"SELECT uNSSet FROM tZone WHERE uZone=%u AND uView=%u",guZone,guView);
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uNameServer);
	mysql_free_result(res);

	return(uNameServer);

}//unsigned uGetuNameServer(unsigned guZone)


void SetZoneFieldsOn(void)
{
	//
	//Avoid overwriting a red style
	
	if(strcmp(cHostmasterStyle,"type_fields_req"))
		cHostmasterStyle="type_fields";	
	if(strcmp(cuExpireStyle,"type_fields_req"))
		cuExpireStyle="type_fields";
	if(strcmp(cuRefreshStyle,"type_fields_req"))
		cuRefreshStyle="type_fields";
	if(strcmp(cuTTLStyle,"type_fields_req"))
		cuTTLStyle="type_fields";
	if(strcmp(cuRetryStyle,"type_fields_req"))
		cuRetryStyle="type_fields";
	if(strcmp(cuZoneTTLStyle,"type_fields_req"))
		cuZoneTTLStyle="type_fields";
}


void ProcessRRLine(const char *cLine,char *cZoneName,const unsigned uZone,
		const unsigned uCustId,const unsigned uNameServer,const unsigned uCreatedBy,const char *cComment)
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
	char *cp;
	char cResourceImportTable[256]="tResource";

	//GetConfiguration("cResourceImportTable",cResourceImportTable,0);


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

	//Prev NAME not the same as ORIGIN RTFM lol.
	if(cLine[0]=='\t')
	{
		sscanf(cLine,"%100s %255s %255s %255s %255s\n",
			cType,cParam1,cParam2,cParam3,cParam4);
		if(cPrevcName[0])
			sprintf(cName,"%.99s",cPrevcName);
		else	
			strcpy(cName,"\t");
	}
	//Origin
	else if(cLine[0]=='@')
	{
		sscanf(cLine,"@ %100s %255s %255s %255s %255s\n",
			cType,cParam1,cParam2,cParam3,cParam4);
		sprintf(cName,"@");
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
			sprintf(cMsg,"<blink>Error %s: Incorrect A format: %s</blink>",
					cZoneName,cLine);
			gcMessage=cMsg;
			return;
		}
		if(!strcmp(IPNumber(cParam1),"0.0.0.0"))
		{
			sprintf(cMsg,"<blink>Incorrect A IP number format: %s</blink>",cLine);
			gcMessage=cMsg;
			return;
		}
		if(strchr(cName,'_'))
		{
			sprintf(cMsg,"<blink>Error %s: cName (check-names): %s</blink>",
					cZoneName,cLine);
			gcMessage=cMsg;
			return;
		}
	}
	else if(!strcasecmp(cType,"CNAME"))
	{
		MYSQL_RES *res;
		//char cZone[256];
		//char cName[256];
		uRRType=5;
		if(!cParam1[0] || cParam2[0])
		{
			sprintf(cMsg,"<blink>Error %s: Incorrect CNAME format: %s</blink>",
					cZoneName,cLine);
			gcMessage=cMsg;
			return;
		}

		//Do not import non singleton CNAME
		sprintf(gcQuery,"SELECT uResource FROM tResource WHERE cName='%s' AND uZone=%u AND uRRType=5",
							cName,guZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res))
		{
			gcMessage="<blink>CNAME records are singleton type. RR w/the same name found.</blink>";
			return;
		}
		mysql_free_result(res);
		sprintf(gcQuery,"SELECT uResource FROM tResource WHERE cName='%s.%s.' AND uZone=%u AND uRRType=5",
							cName,cZoneName,guZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res))
		{
			gcMessage="<blink>CNAME records are singleton type. RR w/the same FQ name found.</blink>";
			return;
		}
		mysql_free_result(res);

		if(cParam1[strlen(cParam1)-1]!='.')
		{
		//	printf("Warning: Incorrect FQDN missing period: %s\n",cParam1);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam1,cPrevOrigin);
				sprintf(cParam1,"%.255s",gcQuery);
				//printf("Fixed: CNAME RHS fixed from $ORIGIN: %s\n",cParam1);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam1,cZoneName);
				sprintf(cParam1,"%.255s",gcQuery);
				//printf("Fixed: CNAME RHS fixed from cZoneName: %s\n",cParam1);
			}
		}
		//sprintf(cZone,"%.255s.",FQDomainName(cZoneName));
		//sprintf(cName,"%.255s",FQDomainName(cParam1));
		//if(strcmp(cName+strlen(cName)-strlen(cZone),cZone));
		//	printf("Warning: CNAME RHS should probably end with zone: %s\n",cLine);
	}
	else if(!strcasecmp(cType,"NS"))
	{
		MYSQL_RES *res;
		char cNS[100];

		uRRType=2;
		if(!cParam1[0] || cParam2[0])
		{
			sprintf(cMsg,"<blink>Error %s: Incorrect NS format: %s</blink>",
					cZoneName,cLine);
			gcMessage=cMsg;
			return;
		}
		if(cParam1[strlen(cParam1)-1]!='.')
		{
			//sprintf(cMsg,"<blink>Warning %s: Incorrect FQDN missing period: %s\n",
			//		cZoneName,cLine);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam1,cPrevOrigin);
				sprintf(cParam1,"%.255s",gcQuery);
				//printf("Fixed: NS RHS fixed from $ORIGIN: %s\n",cParam1);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam1,cZoneName);
				sprintf(cParam1,"%.255s",gcQuery);
				//printf("Fixed: NS RHS fixed from cZoneName: %s\n",cParam1);
			}
		}

		//Get rid of last period for check
		strcpy(cNS,cParam1);
		cNS[strlen(cNS)-1]=0;
		sprintf(gcQuery,"SELECT uNameServer FROM tNameServer WHERE uNameServer=%u AND"
				" ( (cList LIKE '%s MASTER%%') OR (cList LIKE '%%%s SLAVE%%'))",
					uNameServer,cNS,cNS);

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
		{
			sprintf(cMsg,"<blink>Error %s: %s</blink>",cZoneName,mysql_error(&gMysql));
			gcMessage=cMsg;
			return;
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)) 
		{
			sprintf(cMsg,"<blink>Error %s:NS RR Ignored. Part of uNameServer cList</blink>",
					cZoneName);
			gcMessage=cMsg;
			return;
		}
		mysql_free_result(res);
	}
	else if(!strcasecmp(cType,"MX"))
	{
		unsigned uMX=0;
		uRRType=3;
		if(!cParam1[0] || !cParam2[0] )
		{
			sprintf(cMsg,"<blink>Error %s: Missing MX param: %s</blink>",
					cZoneName,cLine);
			gcMessage=cMsg;
			return;
		}
		sscanf(cParam1,"%u",&uMX);
		if(uMX<1 || uMX>99999)
		{
			sprintf(cMsg,"<blink>Error %s: Incorrect MX format: %s</blink>",
					cZoneName,cLine);
			gcMessage=cMsg;
			return;
		}
		if(cParam2[strlen(cParam2)-1]!='.')
		{
			//
			//How we will show warnings ?
			//sprintf(cMsg,"<blink>Warning %s: Incorrect FQDN missing period: %s\n",
			//		cZoneName,cLine);
			if(cPrevOrigin[0])
			{
				sprintf(gcQuery,"%.255s.%.99s",cParam2,cPrevOrigin);
				sprintf(cParam2,"%.255s",gcQuery);
				//printf("Fixed: MX RHS fixed from $ORIGIN: %s\n",cParam2);
			}
			else
			{
				sprintf(gcQuery,"%.255s.%.99s.",cParam2,cZoneName);
				sprintf(cParam2,"%.255s",gcQuery);
				//printf("Fixed: MX RHS fixed from cZoneName: %s\n",cParam2);
			}
		}
	}
	else if(!strcasecmp(cType,"PTR"))
	{
		unsigned uFirstDigit=0;

		uRRType=7;
		if(!cParam1[0] || cParam2[0])
		{
			sprintf(cMsg,"<blink>Error %s: Incorrect PTR format: %s</blink>",
					cZoneName,cLine);
			gcMessage=cMsg;
			return;
		}
		sscanf(cName,"%u.%*s",&uFirstDigit);
		if(!uFirstDigit)
		{
			//Check this rule again
			sprintf(cMsg,"<blink>Error</blink> %s: Incorrect PTR LHS should start with a non zero digit: %s",
				cZoneName,cLine);
			gcMessage=cMsg;
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
			sprintf(cMsg,"<blink>Error %s: Incorrect TXT format: %s</blink>",
					cZoneName,cParam1);
			gcMessage=cMsg;
			return;
		}
		//debug only
		//printf("TXT: %s\n",cParam1);
	}

	//Special cases that should keep a static var for next line
	else if(!strcasecmp(cName,"$TTL"))
	{
		if(cType[0])
		{
			sscanf(cType,"%u",&uPrevTTL);
			//debug only
			/*printf("$TTL changed: %u (%s %s)\n",
					uPrevTTL,cType,cParam1);*/
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
			/*else
				//debug only
				printf("$ORIGIN Changed: %s\n",
					cPrevOrigin);*/
			return;
		}
	}

	//Unrecognized lines
	//Current missing features -we know about and need: hinfo ignored
	else if(1)
	{
		sprintf(cMsg,"<blink>Error %s: RR Not recognized: %s %s</blink>",cZoneName,cLine,cType);
		gcMessage=cMsg;
		return;
	}

	if(!uTTL && uPrevTTL) uTTL=uPrevTTL;
	if(cPrevOrigin[0]) 
		sprintf(cNamePlus,"%.99s.%.99s",cName,cPrevOrigin);
	else
		sprintf(cNamePlus,"%.99s",cName);
	if(uRRType==6)//TXT special case
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
		sprintf(cMsg,"<blink>Error %s: %s</blink>",cZoneName,mysql_error(&gMysql));
	gcMessage=cMsg;
	return;

}//void ProcessRRLine()


void GetConfiguration(const char *cName, char *cValue, unsigned uHtml)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

        char cQuery[512];

        sprintf(cQuery,"SELECT cValue,cComment FROM tConfiguration WHERE cLabel='%s'",cName);
        mysql_query(&gMysql,cQuery);
        if(mysql_errno(&gMysql))
	{
		if(uHtml)
		{
        	        htmlPlainTextError(mysql_error(&gMysql));
		}
		else
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
	}
        res=mysql_store_result(&gMysql);
        if((field=mysql_fetch_row(res)))
	{
		if(strcmp(field[0],"Value in cComment"))
                	sprintf(cValue,"%.255s",field[0]);
		else
                	sprintf(cValue,"%.1023s",field[1]);
	}
        mysql_free_result(res);

}//void GetConfiguration(const char *cName, char *cValue)


void funcNSSetMembers(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT tNS.uNS,tNS.cFQDN,tServer.cLabel FROM tNSSet,tNS,tServer WHERE"
			" tNS.uNSSet=tNSSet.uNSSet AND tNS.uServer=tServer.uServer AND"
			" tNSSet.uNSSet=%lu ORDER BY tNS.cFQDN",strtoul(cuNameServer,NULL,0));
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	while((field=mysql_fetch_row(res)))
		fprintf(fp,"%s %s\n",field[1],field[2]);

	mysql_free_result(res);

}//void funcNSSetMembers(FILE *fp)


void htmlDelegationTool(void)
{
	htmlHeader("vdnsOrg","Header");
	htmlZonePage("vdnsOrg","DelegationTool.Body");
	htmlFooter("Footer");
	
}//void htmlDelegationTool(void)


unsigned uPTRInCIDR(unsigned uZone,char *cIPBlock)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uA,uB,uC;
	char cIP[100]={""};
	char cZone[256]={""};

	sprintf(cZone,"%.255s",ForeignKey("tZone","cZone",guZone));
	sscanf(cZone,"%u.%u.%u.",&uC,&uB,&uA);
	
	//uRRType=7: PTR
	sprintf(gcQuery,"SELECT cName FROM tResource WHERE uRRType=7 AND uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		sprintf(cIP,"%u.%u.%u.%s",uA,uB,uC,field[0]);
		if(uIpv4InCIDR4(cIP,cIPBlock)) return(1);
	}
	return(0);
	
}//unsigned uPTRInCIDR(unsigned uZone,char *cIPBlock)


unsigned uPTRInBlock(unsigned uZone,unsigned uStart,unsigned uEnd)
{
	MYSQL_RES *res;
	register unsigned uI;

	for(uI=uStart;uI<uEnd;uI++)
	{
		sprintf(gcQuery,"SELECT cName FROM tResource WHERE uRRType=7 AND uZone=%u AND cName='%u'",uZone,uI);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)) return(1);
	}
	
	return(0);
	
}//unsigned uPTRInBlock(unsigned uZone,unsigned uStart,unsigned uEnd)

char *ParseTextAreaLines2(char *cTextArea);
void CreatetResourceTest(void);

void PrepDelToolsTestData(unsigned uNumIPs)
{
	char cNS[100]={""};
	char cName[100]={""};
	char cParam1[100]={""};
	unsigned uA,uB,uC,uD,uE;
	unsigned uIPBlockFormat=0;
	char cNServers[4096]={""};

	CreatetResourceTest();
	sprintf(gcQuery,"DELETE FROM tResourceTest WHERE uZone=%u",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	sprintf(gcQuery,"INSERT INTO tResourceTest (uResource,cName,uOwner,uCreatedBy,uCreatedDate,uModBy,"
			"uModDate,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4,cComment,uZone) "
			"SELECT uResource,cName,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate,uTTL,uRRType,"
			"cParam1,cParam2,cParam3,cParam4,cComment,uZone FROM tResource WHERE "
			"uZone=%u",guZone);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	if(strchr(cIPBlock,'/'))
	{
		sscanf(cIPBlock,"%u.%u.%u.%u/%u",&uA,&uB,&uC,&uD,&uE);
		uIPBlockFormat=IP_BLOCK_CIDR;
	}
	else if(strchr(cIPBlock,'-'))
	{
		sscanf(cIPBlock,"%u.%u.%u.%u-%u",&uA,&uB,&uC,&uD,&uE);
		uIPBlockFormat=IP_BLOCK_DASH;
	}

	sprintf(cNServers,"%.4095s",cNSList);
	
	while(1)
	{
		sprintf(cNS,"%.99s",ParseTextAreaLines2(cNServers));
		if(!cNS[0]) break;
		if(uIPBlockFormat==IP_BLOCK_CIDR)
			sprintf(cName,"%u/%u",uD,uE);
		else if(uIPBlockFormat==IP_BLOCK_DASH)
			sprintf(cName,"%u-%u",uD,uE);

		sprintf(gcQuery,"INSERT INTO tResourceTest SET uZone=%u,cName='%s',uTTL=%u,"
					"uRRType=2,cParam1='%s',cComment='Delegation (%s)',"
					"uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					guZone
					,cName
					,uDelegationTTL
					,cNS
					,cIPBlock
					,uOwner
					,guLoginClient);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
	}

	//$GENERATE 0-255 $ CNAME $.0/24.21.68.217.in-addr.arpa.
	if(uIPBlockFormat==IP_BLOCK_CIDR)
		sprintf(cParam1,"$.%u/%u.%u.%u.%u.in-addr.arpa.",
				uD
				,uE
				,uC
				,uB
				,uA
			       );
	else if(uIPBlockFormat==IP_BLOCK_DASH)
	{
		sprintf(cParam1,"$.%u-%u.%u.%u.%u.in-addr.arpa.",
				uD
				,uE
				,uC
				,uB
				,uA
			       );
	}
	sprintf(gcQuery,"INSERT INTO tResourceTest SET uZone=%u,cName='$GENERATE %u-%u $',"
			"uRRType=5,cParam1='%s',cComment='Delegation (%s)',uOwner=%u,"
			"uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			guZone
			,uD
			,(uD+uNumIPs)
			,cParam1
			,cIPBlock
			,uOwner
			,guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}


char *ParseTextAreaLines2(char *cTextArea)
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
}


char *cGetViewLabel(void)
{
	static char cView[100]={""};
	MYSQL_RES *res;
	MYSQL_ROW field;

	if(!guZone)
	{
		sprintf(cView,"[Unknown view]");
		return(cView);
	}

	sprintf(gcQuery,"SELECT uView FROM tZone WHERE uZone=%u",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&guView);
		sprintf(cView,"[%s]",ForeignKey("tView","cLabel",guView));
	}
	return(cView);

}//char *cGetViewLabel(void)

