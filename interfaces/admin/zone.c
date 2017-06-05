/*
FILE 
	zone.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010-2012 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file in main source dir.
PURPOSE
	idnsAdmin program file.
*/

#include "interface.h"


static char cMainAddress[17]={"0.0.0.0"};
static char *cMainAddressStyle="type_fields_off";

static char cHostmaster[100]={"nsadmin.unixservice.com"};
static char *cHostmasterStyle="type_fields_off";

static char cNSs[1024]={""};
static char cNSSet[100]={""};

static char cuSerial[16]={""};
static char *cuSerialStyle="type_fields_off";

static char cuExpire[16]={"604800"};
static char *cuExpireStyle="type_fields_off";

static char cuRefresh[16]={"10800"};
static char *cuRefreshStyle="type_fields_off";

static char cuTTL[16]={"86400"};
static char *cuTTLStyle="type_fields_off";

static char cuRetry[16]={"3600"};
static char *cuRetryStyle="type_fields_off";

static char cuZoneTTL[16]={"86400"};
static char *cuZoneTTLStyle="type_fields_off";

static char cMasterIPs[256]={""};
static char *cMasterIPsStyle="type_fields_off";

static unsigned uSecondaryOnly=0;

//The default NS must be =1 (for new zones)
char cuNameServer[16]={"1"};

static char cuMailServers[16]={"0"};
static char *cuMailServersStyle="type_fields_off";

static char cuRegistrar[16]={"0"};
static char *cuRegistrarStyle="type_fields_off";

static char cOptions[16385]={""};
static char *cOptionsStyle="type_textarea_off";

static char cSearch[100]={""};
static char *cSearchStyle="type_fields";

static char cuZone[16]={"0"};

static char *cZoneStyle="type_fields_off";

static char cSelectView[100]={""};
static char *cSelectViewStyle="type_fields_off";
char cuView[16]={""};

static char *cRestoreRRStatus="disabled";

extern unsigned uForClient;
extern char *cForClientPullDownStyle;
extern char cForClientPullDown[];

static unsigned uOwner=0;
static unsigned uCreatedBy=0;
static time_t uCreatedDate=0;
static unsigned uModBy=0;
static time_t uModDate=0;

unsigned InMyBlocks(char *cIP);//resource.c
extern unsigned guBrowserFirefox;//main.c

//Locals only :) (surfing in SCali)
void SerialNum(char *cSerial);
int AdminSubmitJob(char *cCommand,unsigned uNameServerArg,char *cZoneArg,
				unsigned uPriorityArg,long unsigned luTimeArg);
char *cGetViewLabel(char *cuView);
unsigned uGetView(char *cView);

void NewZone(void);
void DelZone(void);
void SearchZone(char *cLabel);

void SetZoneFieldsOn(void);
unsigned ValidateZoneInput(void);
void SelectZonesByOwner(unsigned uOwner);
void SaveZone(void);
void ZoneDiagnostics(void);
unsigned uGetZoneOwner(unsigned uZone); //resource.c
unsigned uHasDeletedRR(char *cuZone);


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

void Insert_tJob(void);
int SubmitSingleJob(const char *cCommand,const char *cZoneArg, unsigned uNSSetArg,
		const char *cTargetServer, unsigned uPriorityArg, time_t luTimeArg
	       			,unsigned *uMasterJob);



void ProcessZoneVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if( !strcmp(entries[i].name,"uZone"))
			sprintf(cuZone,"%.99s",entries[i].val);
		else if( !strcmp(entries[i].name,"cZone"))
			sprintf(gcZone,"%.99s",FQDomainName2(entries[i].val));
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
		else if(!strcmp(entries[i].name,"cSearch"))
			sprintf(cSearch,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cView"))
		{
			sprintf(cSelectView,"%.99s",entries[i].val);
			sprintf(cuView,"%u",uGetView(cSelectView));
		}
		else if(!strcmp(entries[i].name,"cForClientPullDown"))
		{
                        uForClient=ReadPullDown("tClient","cLabel",entries[i].val);
			sprintf(cForClientPullDown,"%.99s",entries[i].val);
		}
		else if(!strcmp(entries[i].name,"uForClient"))
			sscanf(entries[i].val,"%u",&uForClient);
		else if(!strcmp(entries[i].name,"cNSs"))
			sprintf(cNSs,"%.1023s",entries[i].val);
		else if(!strcmp(entries[i].name,"cMasterIPs"))
			sprintf(cMasterIPs,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"uSecondaryOnly"))
			sscanf(entries[i].val,"%u",&uSecondaryOnly);
		else if(!strcmp(entries[i].name,"uMailServers"))
			sprintf(cuMailServers,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"uRegistrar"))
			sprintf(cuRegistrar,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"cOptions"))
			sprintf(cOptions,"%.16385s",entries[i].val);
		else if(!strcmp(entries[i].name,"uOwner"))
			sscanf(entries[i].val,"%u",&uOwner);
		else if(!strcmp(entries[i].name,"uCreatedBy"))
			sscanf(entries[i].val,"%u",&uCreatedBy);
		else if(!strcmp(entries[i].name,"uCreatedDate"))
			sscanf(entries[i].val,"%lu",&uCreatedDate);
		else if(!strcmp(entries[i].name,"uModBy"))
			sscanf(entries[i].val,"%u",&uModBy);
		else if(!strcmp(entries[i].name,"uModDate"))
			sscanf(entries[i].val,"%lu",&uModDate);

	}

}//void ProcessZoneVars(pentry entries[], int x)


void ZoneGetHook(entry gentries[],int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"cCustomer"))
		{
			sprintf(gcCustomer,"%.99s",gentries[i].val);
			uForClient=uGetClient(gcCustomer);
		}
	}
	if(gcCookieCustomer[0] && uForClient==0) uForClient=uGetClient(gcCookieCustomer);
	if(gcCookieZone[0] && gcZone[0]==0) sprintf(gcZone,"%.99s",gcCookieZone);
	if(guCookieView && cuView[0]==0) sprintf(cuView,"%u",guCookieView);

	if(gcZone[0] && cuView[0])
	{
		SelectZone(1);
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
			SelectZone(1);
			htmlZone();
		}
		else if(!strcmp(gcFunction,"New"))
		{
		//	SelectZone();//No hidden fields when disabled
			sprintf(gcNewStep,"Confirm ");
			gcMessage="Modify data, review, then confirm. Any other action to cancel.";
			gcInputStatus[0]=0;
			SetZoneFieldsOn();
			cuSerial[0]=0;
			htmlZone();
		}
		else if(!strcmp(gcFunction,"Confirm New"))
		{
			if(ValidateZoneInput())				
				NewZone();

			if(strcmp(gcMessage,"Zone Created"))
			{
				sprintf(gcNewStep,"Confirm ");
				gcInputStatus[0]=0;
				SetZoneFieldsOn();
			}
			else
			{
				
				time_t luClock;
				unsigned uNameServer=0;

				time(&luClock);

				sprintf(gcInputStatus,"disabled");
				sscanf(cuNameServer,"%u",&uNameServer);
	
				if(uNameServer)
				{
					if(AdminSubmitJob("New",uNameServer,gcZone,0,luClock))
						htmlPlainTextError(mysql_error(&gMysql));
				}
				else
				{
					gcMessage="<blink>Error: </blink>Contact admin: uNameServer error";
				}
				SelectZone(1);
			}
			htmlZone();		
		}		
		else if(!strcmp(gcFunction,"Modify"))
		{
			SelectZone(1);//No hidden fields when disabled
			sprintf(gcModStep,"Confirm ");
			gcMessage="Modify data, review, then confirm. Any other action to cancel.";
			gcInputStatus[0]=0;
			SetZoneFieldsOn();
			if(uOwner!=uForClient)
			{
				uForClient=uOwner; //Don't mess zone ownership by mistake if loaded via another company
				sprintf(gcCustomer,"%s",ForeignKey("tClient","cLabel",uOwner));
			}
			htmlZone();
		}
		else if(!strcmp(gcFunction,"Confirm Modify"))
		{
			if(ValidateZoneInput())
				UpdateZone();
			
			if(strcmp(gcMessage,"Zone Modified"))
			{
				sprintf(gcModStep,"Confirm ");
				gcInputStatus[0]=0;
				SetZoneFieldsOn();				
			}
			else
			{
				time_t luClock;
				unsigned uNameServer=0;

				time(&luClock);

				sprintf(gcInputStatus,"disabled");
				sscanf(cuNameServer,"%u",&uNameServer);
				if(uNameServer)
				{
					if(AdminSubmitJob("Modify",uNameServer,gcZone,0,luClock))
						htmlPlainTextError(mysql_error(&gMysql));
				}
				else
				{
					gcMessage="<blink>Error: </blink>Contact admin: uNameServer error";
				}
				SelectZone(1);
			}
			htmlZone();
		}
		else if(!strcmp(gcFunction,"Delete"))
		{
			SelectZone(1);//No hidden fields when disabled
			sprintf(gcDelStep,"Confirm ");			
			htmlZone();
		}
		else if(!strcmp(gcFunction,"Confirm Delete"))
		{
			SelectZone(1);//No hidden fields when disabled
			SaveZone();
			DelZone();
			if(!strcmp(gcMessage,"Zone Deleted"))
			{
				time_t luClock;
				unsigned uNameServer=0;

				time(&luClock);

				sprintf(gcInputStatus,"disabled");
				sscanf(cuNameServer,"%u",&uNameServer);
				if(uNameServer)
				{
					if(AdminSubmitJob("Delete",uNameServer,gcZone,0,luClock))
						htmlPlainTextError(mysql_error(&gMysql));
				}
				else
				{
					gcMessage="<blink>Error: </blink>Contact admin: uNameServer error";
				}
			}
			htmlZone();
			
		}
		else if(!strcmp(gcFunction,"Bulk Import"))
		{
			htmlBulkOp();
		}
		else if(!strcmp(gcFunction,"Restore Zones"))
		{
			htmlRestoreZone();
		}
		else if(!strcmp(gcFunction,"Restore RRs"))
		{
			htmlRestoreResource(cuZone);
		}
		else if(!strcmp(gcFunction,"Zone Diagnostics"))
		{
			ZoneDiagnostics();
		}
		else if(gcZone[0])
		{
			SelectZone(1);
			htmlZone();
		}
		htmlZone();
	}

}//void ZoneCommands(pentry entries[], int x)


void htmlZone(void)
{
	htmlHeader("idnsAdmin","Header");
	htmlZonePage("idnsAdmin","Admin.Zone.Body");
	htmlFooter("Footer");

}//void htmlZone(void)


void htmlZonePage(char *cTitle, char *cTemplateName)
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
			char cuForClient[16]={""};
			char cuSecondaryOnly[2]={""};
			char cuResource[16]={""};
			//These vars will render the data in human readable form
			char cuCreatedDate[100]={""};
			char cuModDate[100]={""};
			char cuOwner[100]={""};
			char cuCreatedBy[100]={""};
			char cuModBy[100]={""};
			//These vars are for the form hidden fields
			char cuCreatedDateForm[100]={""};
			char cuModDateForm[100]={""};
			char cuOwnerForm[100]={""};
			char cuCreatedByForm[100]={""};
			char cuModByForm[100]={""};

			sprintf(cuResource,"%u",uResource);
			sprintf(cuCreatedDateForm,"%lu",uCreatedDate);
			sprintf(cuModDateForm,"%lu",uModDate);
			sprintf(cuOwnerForm,"%u",uOwner);
			sprintf(cuCreatedByForm,"%u",uCreatedBy);
			sprintf(cuModByForm,"%u",uModBy);

			if(uOwner)
				sprintf(cuOwner,"%s",ForeignKey(TCLIENT,"tClient.cLabel",uOwner));
			else
				sprintf(cuOwner,"---");

			if(uCreatedBy)
				sprintf(cuCreatedBy,"%s",ForeignKey(TCLIENT,"tClient.cLabel",uCreatedBy));
			else
				sprintf(cuCreatedBy,"---");

			if(uCreatedDate)
				sprintf(cuCreatedDate,"%s",ctime(&uCreatedDate));
			else
				sprintf(cuCreatedDate,"---");

			if(uModBy)
				sprintf(cuModBy,"%s",ForeignKey(TCLIENT,"tClient.cLabel",uModBy));
			else
				sprintf(cuModBy,"---");

			if(uModDate)
				sprintf(cuModDate,"%s",ctime(&uModDate));
			else
				sprintf(cuModDate,"---");
				
			sprintf(cuResource,"%u",uResource);
			sprintf(cuForClient,"%u",uForClient);
			sprintf(cuSecondaryOnly,"%u",uSecondaryOnly);
			
			sprintf(cNSSet,"%s",ForeignKey("tNSSet","cLabel",strtoul(cuNameServer,NULL,10)));

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

			template.cpName[8]="uZone";
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
			template.cpValue[14]=gcZone;

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
			if(gcZone[0])
				template.cpValue[22]="";
			else
				template.cpValue[22]="disabled";

			template.cpName[23]="cModSOAStatus";
			if(!gcZone[0] || strstr(gcZone,"in-addr.arpa"))
				template.cpValue[23]="disabled";
			else
				template.cpValue[23]="";

			template.cpName[24]="uNameServer";
			template.cpValue[24]=cuNameServer;

			template.cpName[25]="unused";
			template.cpValue[25]="delme";

			template.cpName[26]="cMainAddressStyle";
			template.cpValue[26]=cMainAddressStyle;

			template.cpName[27]="cHostmasterStyle";
			template.cpValue[27]=cHostmasterStyle;

			template.cpName[28]="cuSerialStyle";
			template.cpValue[28]=cuSerialStyle;

			template.cpName[29]="cuExpireStyle";
			template.cpValue[29]=cuExpireStyle;

			template.cpName[30]="cuRefreshStyle";
			template.cpValue[30]=cuRefreshStyle;

			template.cpName[31]="cuTTLStyle";
			template.cpValue[31]=cuTTLStyle;

			template.cpName[32]="cuRetryStyle";
			template.cpValue[32]=cuRetryStyle;

			template.cpName[33]="cSearchStyle";
			template.cpValue[33]=cSearchStyle;
			
			template.cpName[34]="cuZoneTTLStyle";
			template.cpValue[34]=cuZoneTTLStyle;

			template.cpName[35]="gcNewStep";
			template.cpValue[35]=gcNewStep;

			template.cpName[36]="gcDelStep";
			template.cpValue[36]=gcDelStep;

			template.cpName[37]="cZoneStyle";
			template.cpValue[37]=cZoneStyle;

			template.cpName[38]="cuView";
			template.cpValue[38]=cuView;
			
			template.cpName[39]="uForClient";
                        template.cpValue[39]=cuForClient;
			
			template.cpName[40]="cMasterIPsStyle";
			template.cpValue[40]=cMasterIPsStyle;
			
			template.cpName[41]="cMasterIPs";
			template.cpValue[41]=cMasterIPs;
			
			template.cpName[42]="uSecondaryOnly";
			template.cpValue[42]=cuSecondaryOnly;

			template.cpName[43]="cOptions";
			template.cpValue[43]=cOptions;

			template.cpName[44]="cOptionsStyle";
			template.cpValue[44]=cOptionsStyle;

			template.cpName[45]="cRestoreRRStatus";
			template.cpValue[45]=cRestoreRRStatus;

			template.cpName[46]="cDiagStatus";
			if(gcZone[0] && cuView[0])
				template.cpValue[46]="";
			else
				template.cpValue[46]="disabled";

			template.cpName[47]="cLabel";
			template.cpValue[47]=gcCustomer;

			template.cpName[48]="cResourcesTabStatus";
			if(!gcZone[0])
				template.cpValue[48]="onclick='return false'";
			else
				template.cpValue[48]="";

			template.cpName[49]="uView";
			template.cpValue[49]=cuView;

			template.cpName[50]="cCustomer";
			template.cpValue[50]=gcCustomer;

			template.cpName[51]="uResource";
			template.cpValue[51]=cuResource;

			template.cpName[52]="gcZone";
			template.cpValue[52]=gcZone;

			//template.cpName[53]="";
			template.cpName[53]="uOwner";
			template.cpValue[53]=cuOwner;

			template.cpName[54]="uCreatedBy";
			template.cpValue[54]=cuCreatedBy;

			template.cpName[55]="uCreatedDate";
			template.cpValue[55]=cuCreatedDate;

			template.cpName[56]="uModBy";
			template.cpValue[56]=cuModBy;

			template.cpName[57]="uModDate";
			template.cpValue[57]=cuModDate;

			template.cpName[58]="uOwnerForm";
			template.cpValue[58]=cuOwnerForm;

			template.cpName[59]="uCreatedByForm";
			template.cpValue[59]=cuCreatedByForm;

			template.cpName[60]="uCreatedDateForm";
			template.cpValue[60]=cuCreatedDateForm;

			template.cpName[61]="uModByForm";
			template.cpValue[61]=cuModByForm;

			template.cpName[62]="uModDateForm";
			template.cpValue[62]=cuModDateForm;

			template.cpName[63]="cZoneGetLink";
			char cZoneGetLink[256];
			sprintf(cZoneGetLink,"&cZone=%.99s&uView=%.16s&cCustomer=%.32s",gcZone,cuView,gcCustomer);
			template.cpValue[63]=cZoneGetLink;

			template.cpName[64]="cZoneView";
			char cZoneView[256];
			sprintf(cZoneView,"%.99s",gcZone);
			if(guCookieView)
				sprintf(cZoneView,"%.99s/%.31s",gcZone,ForeignKey("tView","cLabel",guCookieView));
			template.cpValue[64]=cZoneView;

			template.cpName[65]="";

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


void funcSelectView(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	fprintf(fp,"<!-- funcSelectView(fp) Start -->\n");
	fprintf(fp,"<input type=hidden name=cView value='%s'>\n",cSelectView);
	sprintf(gcQuery,"SELECT tView.cLabel FROM tView ORDER BY tView.cLabel");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	fprintf(fp,"<select title='Select the DNS view for the Zone' name=cView class=%s %s>\n",cSelectViewStyle,gcInputStatus);
	while((field=mysql_fetch_row(res)))
	{
		fprintf(fp,"<option ");
		if(!strcmp(cSelectView,field[0]))
			fprintf(fp,"selected");
		fprintf(fp,">%s</option>\n",field[0]);
	}

	fprintf(fp,"</select>\n");
	mysql_free_result(res);

	fprintf(fp,"<!-- funcSelectView(fp) End -->\n");
	
}//void funcSelectView(FILE *fp)


void funcSelectMailServers(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	fprintf(fp,"<!-- funcSelectMailServers(fp) Start -->\n");
	fprintf(fp,"<input type=hidden name=uMailServers value='%s'>\n",cuMailServers);
	sprintf(gcQuery,"SELECT uMailServer,cLabel FROM tMailServer ORDER BY cLabel");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	fprintf(fp,"<select title='Select the mailservers set for the Zone' name=uMailServer class=%s %s>\n",cuMailServersStyle,gcInputStatus);
	fprintf(fp,"<option value=0>---</option>\n");
	while((field=mysql_fetch_row(res)))
	{
		fprintf(fp,"<option value=%s ",field[0]);
		if(!strcmp(cuMailServers,field[0]))
			fprintf(fp,"selected");
		fprintf(fp,">%s</option>\n",field[1]);
	}

	fprintf(fp,"</select>\n");
	mysql_free_result(res);

	fprintf(fp,"<!-- funcSelectMailServers(fp) End -->\n");
	
}//void funcSelectMailServers(FILE *fp)


void funcSelectRegistrar(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	fprintf(fp,"<!-- funcSelectRegistrar(fp) Start -->\n");
	fprintf(fp,"<input type=hidden name=uRegistrar value='%s'>\n",cuRegistrar);
	sprintf(gcQuery,"SELECT uRegistrar,cLabel FROM tRegistrar ORDER BY cLabel");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	fprintf(fp,"<select title='Select the registrar of the Zone' name=uRegistrar class=%s %s>\n",cuMailServersStyle,gcInputStatus);
	fprintf(fp,"<option value=0>---</option>\n");
	while((field=mysql_fetch_row(res)))
	{
		fprintf(fp,"<option value=%s ",field[0]);
		if(!strcmp(cuRegistrar,field[0]))
			fprintf(fp,"selected");
		fprintf(fp,">%s</option>\n",field[1]);
	}

	fprintf(fp,"</select>\n");
	mysql_free_result(res);

	fprintf(fp,"<!-- funcSelectRegistrar(fp) End -->\n");
	
}//void funcSelectRegistrar(FILE *fp)


void funcSelectSecondaryYesNo(FILE *fp)
{
	fprintf(fp,"<!-- funcSelectSecondaryYesNo(fp) Start -->\n");
	fprintf(fp,"<input type=hidden name=uSecondaryOnly value=%u>\n",uSecondaryOnly);
	fprintf(fp,"<select name=uSecondaryOnly disabled class=type_fields_off>\n");

	fprintf(fp,"<option value=0 ");
	if(uSecondaryOnly==0)
		fprintf(fp,"selected");
	fprintf(fp,">No</option>\n");
	fprintf(fp,"<option value=1 ");
	if(uSecondaryOnly)
		fprintf(fp,"selected");
	fprintf(fp,">Yes</option>\n");
	fprintf(fp,"</select>\n");

	fprintf(fp,"<!-- funcSelectSecondaryYesNo(fp) End -->\n");
	
}//void funcSelectSecondaryYesNo(FILE *fp)


void funcRRs(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cParam1[65]={""};

	fprintf(fp,"<!-- funcRRs(fp) Start -->\n");

	sprintf(gcQuery,"SELECT tResource.uResource,tZone.cZone,IF(STRCMP(tResource.cName,''),"
			"tResource.cName,'(default)'),tResource.uTTL,tRRType.cLabel,tResource.cParam1,"
			"tResource.cParam2,tResource.cComment FROM tResource,tRRType,tZone WHERE "
			"tResource.uZone=tZone.uZone AND tResource.uRRType=tRRType.uRRType AND "
			"tZone.uView='%s' AND tZone.cZone='%s' ORDER BY tResource.uRRType,ABS(tResource.cName)",
			cuView,gcZone);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{

		if(!field[2][0] || field[2][0]=='\t')
			sprintf(field[2],"@");
		
		//Ticket #60 fix. The bottom table width extended all the above tables.
		if(strlen(field[5])>50)
			sprintf(cParam1,"%.32s[...]",field[5]);
		else
			sprintf(cParam1,"%.64s",field[5]);

		fprintf(fp,"<tr>\n");
		fprintf(fp,"<td valign=top><a class=darkLink href=\"idnsAdmin.cgi?gcPage=Resource"
			"&uResource=%s&cZone=%s&uView=%s&cCustomer=%s\">%s</a></td><td valign=top>%s</td>"
			"<td valign=top>%s</td><td valign=top>%s</td><td valign=top>%s</td><td valign=top>%s</td>\n",
				field[0],
				field[1],
				cuView,
				gcCustomer,
				field[2],
				field[3],
				field[4],
				cParam1,
				field[6],
				field[7]);
		fprintf(fp,"</tr>\n");
		
	}
	mysql_free_result(res);


	fprintf(fp,"<!-- funcRRs(fp) End -->\n");


}//void funcRRs(FILE *fp)


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

//
//HTML func section ends

void SelectZone(unsigned uSetCookie)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	sprintf(gcQuery,"SELECT uZone,cMainAddress,cHostmaster,tZone.uModDate,tNSSet.cLabel,uSerial,uExpire,"
			"uRefresh,uTTL,uRetry,uZoneTTL,uView,tNSSet.uNSSet,tZone.uOwner,uSecondaryOnly,tNSSet.cMasterIPs,"
			"uMailServers,uRegistrar,cOptions,tZone.uOwner,tZone.uCreatedBy,tZone.uCreatedDate,"
			"tZone.uModBy FROM tZone,tNSSet WHERE tZone.uNSSet=tNSSet.uNSSet AND "
			"tZone.cZone='%s' AND tZone.uView='%s'",gcZone,cuView);
	
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		unsigned uView=0;

		sprintf(cuZone,"%.10s",field[0]);
		sprintf(cMainAddress,"%.16s",field[1]);
		sprintf(cHostmaster,"%.99s",field[2]);
		sscanf(field[3],"%lu",&uModDate);
		sprintf(cNSSet,"%.99s",field[4]);
		sprintf(cuSerial,"%.15s",field[5]);
		sprintf(cuExpire,"%.15s",field[6]);
		sprintf(cuRefresh,"%.15s",field[7]);
		sprintf(cuTTL,"%.15s",field[8]);
		sprintf(cuRetry,"%.15s",field[9]);
		sprintf(cuZoneTTL,"%.15s",field[10]);
		sprintf(cSelectView,"%s",cGetViewLabel(field[11]));
		sscanf(field[11],"%u",&uView);
		sprintf(cuNameServer,"%.15s",field[12]);
		//sscanf(field[13],"%u",&uForClient);
		//sprintf(gcCustomer,"%.99s",cClientLabel(uForClient));
		sscanf(field[14],"%u",&uSecondaryOnly);
		sprintf(cMasterIPs,"%.255s",field[15]);
		sprintf(cuMailServers,"%.15s",field[16]);
		sprintf(cuRegistrar,"%.15s",field[17]);
		sprintf(cOptions,"%.16385s",field[18]);
		sscanf(field[19],"%u",&uOwner);
		sscanf(field[20],"%u",&uCreatedBy);
		sscanf(field[21],"%lu",&uCreatedDate);
		sscanf(field[22],"%u",&uModBy);
		if(!gcMessage[0]) gcMessage="Zone Selected";
		if(uHasDeletedRR(cuZone))
			cRestoreRRStatus="";
		if(uSetCookie && (strcmp(gcCookieZone,gcZone) || guCookieView!=uView))
		{
			sprintf(gcCookieZone,"%.99s",gcZone);
			guCookieView=uView;
			guCookieResource=0;
			SetSessionCookie();
		}
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
		cSelectView[0]=0;
		cuNameServer[0]=0;
		gcZone[0]=0;
		cuMailServers[0]=0;
		cuRegistrar[0]=0;
		cOptions[0]=0;
		gcMessage="<blink>Error: </blink>No Zone Selected";
	}
	
	mysql_free_result(res);
	
}//void SelectZone(void)


void NewZone(void)
{

	unsigned uSerial;
	unsigned uExpire=0;
	unsigned uRefresh=0;
	unsigned uTTL=0;
	unsigned uRetry=0;
	unsigned uZoneTTL=0;
	long unsigned luYearMonDay=0;

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

	//
	//uNameServer=1 has to be the local cluster/server
	
	sprintf(gcQuery,"INSERT INTO tZone SET cZone='%s',uSerial=%u,uExpire=%u,uRefresh=%u,"
			"uTTL=%u,uRetry=%u,uZoneTTL=%u,uView=%u,cMainAddress='%s',cHostmaster='%s',"
			"uSecondaryOnly=0,uNSSet=1,uMailServers=%s,uRegistrar=%s,cOptions='%s',"
			"uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			gcZone,
			uSerial,
			uExpire,
			uRefresh,
			uTTL,
			uRetry,
			uZoneTTL,
			(guCookieView=uGetView(cSelectView)),
			cMainAddress,
			cHostmaster,
			cuMailServers,
			cuRegistrar,
			cOptions,
			uForClient,
			guLoginClient
			);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
//		htmlPlainTextError(mysql_error(&gMysql));
		strcat(gcQuery,"<br>");
		strcat(gcQuery,mysql_error(&gMysql));
		htmlPlainTextError(gcQuery);
	}

	if(mysql_affected_rows(&gMysql)==1)
	{
		gcMessage="Zone Created";
		iDNSLog(mysql_insert_id(&gMysql),"tZone","New");
	}
	else
	{
		gcMessage="<blink>Error: </blink>Zone Not Created Modified";
	}
	
	//Set session cookie
	if(strcmp(cForClientPullDown,gcCustomer))
		sprintf(gcCustomer,"%.99s",cForClientPullDown);
	uResource=0;

	guCookieResource=0;
	sprintf(gcCookieZone,"%.99s",gcZone);
	SetSessionCookie();

}//void NewZone(void)


void DelZone(void)
{
	sprintf(gcQuery,"DELETE FROM tZone WHERE cZone='%s' AND uView=%s", gcZone,cuView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	if(mysql_affected_rows(&gMysql)==1)
	{
		gcMessage="Zone Deleted";
		iDNSLog(uGetuZone(gcZone,cuView),"tZone","Del");
		sprintf(gcQuery,"DELETE FROM tResource WHERE uZone='%s'",cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));

	}
	else
	{
		gcMessage="<blink>Error: </blink>Zone Not Deleted";
	}

	//Set session cookie
	gcZone[0]=0;
	cuView[0]=0;
	uResource=0;

	gcCookieZone[0]=0;
	guCookieResource=0;
	guCookieView=0;
	SetSessionCookie();

}//void DelZone(void)


void UpdateZone(void)
{
	unsigned uSerial;
	unsigned uExpire=0;
	unsigned uRefresh=0;
	unsigned uTTL=0;
	unsigned uRetry=0;
	unsigned uZoneTTL=0;
	long unsigned luYearMonDay=0;

	if(!gcZone[0] || gcZone[0]=='-')
	{
		gcMessage="<blink>Error: </blink>Unknown/empty zone was not modified";
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
	
	sprintf(gcQuery,"UPDATE tZone SET uSerial=%u,uExpire=%u,uRefresh=%u,uTTL=%u,uRetry=%u,"
			"uZoneTTL=%u,uView=%u,cMainAddress='%s',cHostmaster='%s',uMailServers=%s,"
			"uRegistrar=%s,cOptions='%s',uOwner=%u,uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE uZone='%s'",
			uSerial,
			uExpire,
			uRefresh,
			uTTL,
			uRetry,
			uZoneTTL,
			uGetView(cSelectView),
			cMainAddress,
			cHostmaster,
			cuMailServers,
			cuRegistrar,
			cOptions,
			uForClient,
			guLoginClient,
			cuZone);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	if(mysql_affected_rows(&gMysql)==1)
	{
		gcMessage="Zone Modified";
		iDNSLog(uGetuZone(gcZone,cuView),"tZone","Mod");
	}
	else
	{
		gcMessage="<blink>Error: </blink>Zone Not Modified";
	}

	//Update tResource.uOwner; only for forward zones!
	//.arpa zones may have RR owned by several companies
	if(strstr(gcZone,"in-addr.arpa")==NULL)
	{
		sprintf(gcQuery,"UPDATE tResource SET uOwner=%u,uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE uZone='%s'",
				uForClient
				,guLoginClient
				,cuZone);
		mysql_query(&gMysql,gcQuery);
	
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
	}

	//Set session cookie
	if(strcmp(cForClientPullDown,gcCustomer))
		sprintf(gcCustomer,"%.99s",cForClientPullDown);
	uResource=0;

	guCookieResource=0;
	SetSessionCookie();

}//void UpdateZone(void)


void SerialNum(char *cSerial)
{
	time_t clock;
		
	//Make new today serial number
	time(&clock);
	strftime(cSerial,12,"%Y%m%d00",localtime(&clock));

}//void SerialNum(char *cSerial)


int AdminSubmitJob(char *cCommand,unsigned uNSSetArg,char *cZoneArg,
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
	sprintf(gcQuery,"INSERT INTO tJob SET uJob=%u,uMasterJob=%u,cJob='%s',cZone='%s',uNSSet=%u,cTargetServer='%s',uPriority=%u,uTime=%lu,cJobData='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
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


unsigned uGetuZone(char *cZone, char *cuView)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uZone=0;
	char cQuery[512];

	sprintf(cQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView='%s'",cZone,cuView);
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uZone);
	mysql_free_result(res);

	return(uZone);

}//unsigned uGetuZone(char *cZone)


char *cGetViewLabel(char *cuView)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	static char cLabel[100];
	char cQuery[512];

	sprintf(cQuery,"SELECT cLabel FROM tView WHERE uView=%s",cuView);
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sprintf(cLabel,"%.99s",field[0]);
	mysql_free_result(res);

	return(cLabel);
	
}//char cGetViewLabel(char *cuView)


unsigned uGetView(char *cView)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uView=0;
	char cQuery[512];

	sprintf(cQuery,"SELECT uView FROM tView WHERE cLabel='%s'",cView);
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uView);
	mysql_free_result(res);

	return(uView);

}//unsigned uGetView(char *cView)

	
void SetZoneFieldsOn(void)
{
	cuExpireStyle="type_fields";
	cuRefreshStyle="type_fields";
	cuTTLStyle="type_fields";
	cuRetryStyle="type_fields";
	cuZoneTTLStyle="type_fields";
	cHostmasterStyle="type_fields";	
	cZoneStyle="type_fields";
	cSelectViewStyle="type_fields";
	cForClientPullDownStyle="type_fields";
	cuMailServersStyle="type_fields";
	cuRegistrarStyle="type_fields";
	cOptionsStyle="type_textarea";
	
//	cSelectSecondaryYesNoStyle="type_fields";

}//void SetZoneFieldsOn(void)


unsigned ValidateZoneInput(void)
{
	unsigned uValid=1;
	unsigned uExpire=0;
	unsigned uRefresh=0;
	unsigned uTTL=0;
	unsigned uRetry=0;
	unsigned uZoneTTL=0;
	MYSQL_RES *res;

	if(!gcZone[0])
	{
		cZoneStyle="type_fields_req";
		gcMessage="<blink>Error: </blink>No zone name specified!";
		return(0);
	}
	//Remove trailing period/dot from zone FQDN to adhere to our internal model.
	if(gcZone[strlen(gcZone)-1]=='.')
		gcZone[strlen(gcZone)-1]=0;

	if(!strcmp(gcFunction,"Confirm New"))
	{
		sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=%u",gcZone,uGetView(cSelectView));
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res))
		{
			cZoneStyle="type_fields_req";
			gcMessage="<blink>Error: </blink>Zone name already in use!";
			return(0);
		}
		mysql_free_result(res);
	}
	else if(!strcmp(gcFunction,"Confirm Modify"))
	{
		MYSQL_ROW field;

		sprintf(gcQuery,"SELECT cZone FROM tZone WHERE uZone='%s'",cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			if(strcmp(gcZone,field[0]))
			{
				cZoneStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>You can't change a zone name, delete it and create it again";
				return(0);
			}
		}
		mysql_free_result(res);
	}
	if(uSecondaryOnly)
	{
		//
		//Don't check for zone headers. Only for cMasterIPs value.
		if(!cMasterIPs[0])
		{
			cMasterIPsStyle="type_fields_req";
			return(0);
		}
		return(1);
	}
	
	if(!cHostmaster[0])
	{
		gcMessage="<blink>Error: </blink>Must provide SOA Hostmaster Email (@ is first .).";
		uValid=0;
		cHostmasterStyle="type_fields_req";
	}
	if(strlen(cHostmaster)<6)
	{
		gcMessage="<blink>Error: </blink>FQDN Hostmaster required. Must specify 'email.' in front.";
		uValid=0;
	}
	if(strchr(cHostmaster,'@'))
	{
		gcMessage="<blink>Error: </blink>ISC BIND Hostmaster format does not allow the @ character. Use a . instead.";
		uValid=0;
		cHostmasterStyle="type_fields_req";
	}
	if(!strchr(cHostmaster,'.'))
	{
		gcMessage="<blink>Error: </blink>Must use a FQDN for Hostmaster with 'email.' in front";
		uValid=0;
		cHostmasterStyle="type_fields_req";
	}
	if(cHostmaster[strlen(cHostmaster)-1]=='.')
	{
		gcMessage="<blink>Error: </blink>email.FQDN Hostmaster should not end with a period.";
		uValid=0;
		cHostmasterStyle="type_fields_req";
	}
	

	sscanf(cuExpire,"%u",&uExpire);
	sscanf(cuRefresh,"%u",&uRefresh);
	sscanf(cuTTL,"%u",&uTTL);
	sscanf(cuRetry,"%u",&uRetry);
	sscanf(cuZoneTTL,"%u",&uZoneTTL);

	if(!uExpire || uExpire>2419200)
	{
		gcMessage="<blink>Error: </blink>Expire TTL out of range";
		uValid=0;
		cuExpireStyle="type_fields_req";		
	}
	if(!uRefresh || uRefresh>100000)
	{
		gcMessage="<blink>Error: </blink>Refresh TTL out of range";
		uValid=0;
		cuRefreshStyle="type_fields_req";
	}
	if(!uTTL || uTTL>100000)
	{
		gcMessage="<blink>Error: </blink>Default TTL out of range";
		uValid=0;
		cuTTLStyle="type_fields_req";		
	}
	if(!uRetry || uRetry>100000)
	{
		gcMessage="<blink>Error: </blink>Retry TTL out of range";
		uValid=0;
		cuRetryStyle="type_fields_req";
	}
	if(!uZoneTTL || uZoneTTL>100000)
	{
		gcMessage="<blink>Error: </blink>Negative TTL out of range";
		uValid=0;
		cuZoneTTLStyle="type_fields_req";
	}
	//Sanity checks from BIND source db_load.c
	if(  uExpire < (uRefresh + uRetry) )
	{
		gcMessage="<blink>Error: </blink>SOA expire value is less than SOA refresh+retry";
		uValid=0;
		cuExpireStyle="type_fields_req";
		cuRetryStyle="type_fields_req";
		cuRefreshStyle="type_fields_req";		
	}
	if(  uExpire < (uRefresh + (10 * uRetry)) )
	{
		gcMessage="<blink>Error: </blink>SOA expire value is less than refresh + 10 * retry";
		uValid=0;
		cuExpireStyle="type_fields_req";
		cuRetryStyle="type_fields_req";
		cuRefreshStyle="type_fields_req";
	}
	if(  uExpire < (7 * 24 * 3600) )
	{
		gcMessage="<blink>Error: </blink>SOA expire value is less than 7 days";
		uValid=0;
		cuExpireStyle="type_fields_req";
	}
	if(  uExpire > ( 183 * 24 * 3600) )
	{
		gcMessage="<blink>Error: </blink>SOA expire value is greater than 6 months";
		uValid=0;
		cuExpireStyle="type_fields_req";
	}
	if(  uRefresh < (uRetry * 2))
	{
		gcMessage="<blink>Error: </blink>SOA refresh value is less than 2 * retry";
		uValid=0;
		cuRetryStyle="type_fields_req";
		cuRefreshStyle="type_fields_req";
	}

	if(!uForClient)
	{
		gcMessage="<blink>Error: </blink>Must select a Company";
		uValid=0;
		cForClientPullDownStyle="type_fields_req";
	}

	return(uValid);

}//unsigned ValidateZoneInput(void)



unsigned uGetuNameServer(char *cZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uNameServer=0;
	char cQuery[512];

	sprintf(cQuery,"SELECT uNSSet FROM tZone WHERE cZone='%s' AND uView=%s",cZone,cuView);
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uNameServer);
	mysql_free_result(res);

	return(uNameServer);

}//unsigned uGetuNameServer(char *cZone)


void SaveZone(void)
{
	//This function will copy a deleted zone and its resource records to tDeletedZone adn tDeletedResource respectively

	//Avoid any kind of DUP records error!
	sprintf(gcQuery,"DELETE FROM tDeletedZone WHERE uDeletedZone='%s'",cuZone);;
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	sprintf(gcQuery,"DELETE FROM tDeletedResource WHERE uZone='%s'",cuZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));


	sprintf(gcQuery,"INSERT INTO tDeletedZone (uDeletedZone,cZone,uNSSet,cHostmaster,"
			"uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,uMailServers,uView,"
			"cMainAddress,uRegistrar,uSecondaryOnly,cOptions,uOwner,uCreatedDate,"
			"uCreatedBy) SELECT uZone,cZone,uNSSet,cHostmaster,uSerial,uExpire,"
			"uRefresh,uTTL,uRetry,uZoneTTL,uMailServers,uView,cMainAddress,uRegistrar,"
			"uSecondaryOnly,cOptions,uOwner,uCreatedDate,uCreatedBy FROM tZone "
			"WHERE uZone='%s'",
			cuZone
			);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	//
	//Copy the tResource records to tDeletedResource
		
	sprintf(gcQuery,"INSERT INTO tDeletedResource (uDeletedResource,uZone,cName,uTTL,uRRType,cParam1,cParam2,"
			"cComment,uCreatedBy,uCreatedDate) SELECT uResource,uZone,cName,uTTL,uRRType,"
			"cParam1,cParam2,cComment,uCreatedBy,UNIX_TIMESTAMP(NOW()) FROM tResource "
			"WHERE uZone=%s",
			cuZone
			);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

}//void SaveZone(void)

unsigned uHasDeletedRR(char *cuZone)
{
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uDeletedResource FROM tDeletedResource WHERE uZone=%s",cuZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	return((unsigned)mysql_num_rows(res));
	
}//unsigned uHasDeletedRR(unsigned uZone)


void ZoneDiagnostics(void)
{
	char cBinDir[100]={"/usr/sbin"};
	char *cp;
	char cView[100]={""};
	char cZoneFile[256]={""};
	MYSQL_RES *res;
	MYSQL_ROW field;

	FILE *fp;
	
	register int i;
	
	GetConfiguration("cBinDir",cBinDir,1);

	printf("Content-type: text/plain\n\n");
	
	sprintf(cView,"%.99s",cGetViewLabel(cuView));
	for(i=0;i<strlen(cView);i++)
		cView[i]=tolower(cView[i]);
	
	sprintf(cZoneFile,"/usr/local/idns/named.d/master/%.31s/%c/%.99s",cView,gcZone[0],gcZone);

	sprintf(gcQuery,"%.99s/named-checkzone %.99s %.255s",cBinDir,gcZone,cZoneFile);
	printf("Testing with:%s\n\n",gcQuery);
	
	if((fp=popen(gcQuery,"r")))
	{
		while(fgets(gcQuery,512,fp))
			printf("%s",gcQuery);
		pclose(fp);
	}
	else
		perror("popen");

	printf("\n\n");	
	//'Guess' dig binary location
	if((cp=strstr(cBinDir,"/sbin")))
		*cp=0;

	//Get master FQDN
	sprintf(gcQuery,"SELECT cFQDN FROM tNS WHERE uNSType=1 OR uNSType=2 AND uNSSet="
			"(SELECT uNSSet FROM tZone WHERE cZone='%s' AND uView='%s') LIMIT 1",
			gcZone
			,cuView
			);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	if((field=mysql_fetch_row(res)))
	{
		sprintf(gcQuery,"%s/bin/dig @%s soa %s",cBinDir,field[0],gcZone);
		printf("Testing with dig:%s\n",gcQuery);
		if((fp=popen(gcQuery,"r")))
		{
			while(!feof(fp))
			{
				fgets(gcQuery,512,fp);
				printf("%s",gcQuery);
			}
			pclose(fp);
		}
		else
			perror("popen");
	}
	else
		printf("Error: It seems you don't have any master NS configured to test against.\n");
	
	printf("\n\n");

	sprintf(gcQuery,"%s/bin/dig soa %s",cBinDir,gcZone);
	printf("Testing with :%s\n",gcQuery);
	if((fp=popen(gcQuery,"r")))
	{
		while(fgets(gcQuery,512,fp))
			printf("%s",gcQuery);
		pclose(fp);
	}
	else
		perror("popen");
	exit(0);	
}//void ZoneDiagnostic(void)


#ifdef EXPERIMENTAL
void funcZoneStatus(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,
	"SELECT tZone.cZone,tZone.uView FROM tZone WHERE uZone IN (SELECT uTablePK FROM tLog WHERE cMessage='Zone with errors') ORDER BY tZone.cZone");

	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);

	if(mysql_num_rows(res))
	{
		fprintf(fp,"<b><u>Zones with errors</u></b><br><br>\n");
		while((field=mysql_fetch_row(res)))
			fprintf(fp,"<a href=idnsAdmin.cgi?gcPage=Zone&cZone=%s&uView=%s>%s</a><br>\n",field[0],field[1],field[0]);
	}

}//void funcZoneStatus(FILE *fp)
#endif


void funcOptionalGraph(FILE *fp)
{
	//This function will check if there's a tConfiguration record with the loaded zone name
	//If so, it will use it to display a graph with the zone usage info.
	char cOptionalGraph[100]={""};
	
	if(!gcZone[0]) return;
	
	GetConfiguration(gcZone,cOptionalGraph,1);

	if(!cOptionalGraph[0]) return;
	
	fprintf(fp,"<tr>\n"
		"<td valign=top>"
		"<a class=inputLink href=\"#\" onClick=\"javascript:window.open('idnsAdmin.cgi?gcPage=Glossary&cLabel=Optional Zone Graph','Glossary',"
		"'height=600,width=500,status=yes,toolbar=no,menubar=no,location=no,scrollbars=1')\"><strong>Optional Zone Graph</strong></a>\n"
		"</td>\n"
		"<td>\n"
		"<a title='Click to enlarge' href='%1$s'><img src='%1$s' border=0 height=154 width=400></a>\n</td>\n</tr>\n",cOptionalGraph);
	
}//void funcOptionalGraph(FILE *fp)


void RequestStatGraph(void)
{
	MYSQL_RES *res;
	char cOptionalGraph[100]={""};
	
	if(!gcZone[0]) return;
	//Check if job for this zone already exists
	sprintf(gcQuery,"SELECT uJob FROM tJob WHERE cZone='%s' AND cTargetServer='graphics.server'",gcZone);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	if(mysql_num_rows(res))
	{
		mysql_free_result(res);
		return;
	}
	
	//Check if the zone already has a graph configured
	GetConfiguration(gcZone,cOptionalGraph,1);
	if(cOptionalGraph[0]) return;
	
	//Now job, cool, will create a new one then
	sprintf(gcQuery,"INSERT INTO tJob SET cJob='StatGraph',cZone='%s',uNameServer=0,cTargetServer='graphics.server',"
			"uOwner=1,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			gcZone
			,guLoginClient
			);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	if(mysql_insert_id(&gMysql))
		gcMessage="Stat graph request submitted.";
	
}//void RequestStatGraph(void)


void funcRequestGraphBtn(FILE *fp)
{
	char cOptionalGraph[100]={""};
	
	if(!gcZone[0]) return;

	GetConfiguration(gcZone,cOptionalGraph,1);
	if(cOptionalGraph[0]) return;

	fprintf(fp,"<input class=largeButton type=submit name=gcFunction value='Request Stats Graph' title='Submits a job for requesting stats graph'>\n");
	
}//void funcRequestGraphBtn(FILE *fp)


void funcZoneNavList(FILE *fp,unsigned uSetCookie)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uResults=0;
	
	if(!cSearch[0]) return;

	if(gcCookieCustomer[0])
		sprintf(gcQuery,"SELECT tZone.cZone,tZone.uView,tView.cLabel,tClient.cLabel,tZone.uOwner FROM tZone,tView,tClient "
			"WHERE tZone.uView=tView.uView AND tZone.uOwner=tClient.uClient AND tZone.uOwner=%u AND tZone.cZone LIKE '%s%%'",
				uGetClient(gcCookieCustomer),cSearch);
	else
		sprintf(gcQuery,"SELECT tZone.cZone,tZone.uView,tView.cLabel,tClient.cLabel,tZone.uOwner FROM tZone,tView,tClient "
			"WHERE tZone.uView=tView.uView AND tZone.uOwner=tClient.uClient AND tZone.cZone LIKE '%s%%'",cSearch);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);

	uResults=mysql_num_rows(res);

	if(uResults==1)
	{
		//Load single record, free result, return
		if((field=mysql_fetch_row(res)))
		{
			sprintf(gcZone,"%.99s",field[0]);
			sprintf(cuView,"%s",field[1]);
			sscanf(field[4],"%u",&uForClient);
			sprintf(gcCustomer,"%s",field[3]);
			SelectZone(uSetCookie);
			mysql_free_result(res);

			fprintf(fp,"<a class=darkLink href=\"idnsAdmin.cgi?gcPage=Zone&cZone=%s&uView=%s&cCustomer=%s\">%s [%s]</a><br>\n",
				field[0]
				,field[1]
				,field[3]
				,field[0]
				,field[2]);
				
			return;
		}
	}
	else if(uResults>1)
	{
		//Display bunch of records, free result, return
		unsigned uCount=0;
		
		while((field=mysql_fetch_row(res)))
		{
			uCount++;
			fprintf(fp,"<a class=darkLink href=\"idnsAdmin.cgi?gcPage=Zone&cZone=%s&uView=%s&cCustomer=%s\">%s [%s]</a><br>\n",
				field[0]
				,field[1]
				,field[3]
				,field[0]
				,field[2]);
			if(uCount==MAX_RESULTS) break;
		}
		if(uCount<uResults)
			fprintf(fp,"Only the first %u shown (%u results). If the zone you are looking for is not in the list above "
				"please further refine your search.<br>",uCount,uResults);
		//We free result and return outside this if
		
	}
	else if(!uResults)
	{
		//Show no rcds found msg, free result, return}
		fprintf(fp,"No records found.<br>\n");
	}

	mysql_free_result(res);
	
}//void funcZoneNavList(FILE *fp)

