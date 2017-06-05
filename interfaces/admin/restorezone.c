/*
FILE 
	restorezone.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file in main source dir.
PURPOSE
	iDNS Administration (ASP) Interface
	program file.
*/

#include "interface.h"

unsigned uDeletedZone=0;
static char cZone[100]={""};
static char cView[32]={""};
static char cMainAddress[17]={"0.0.0.0"};
static char cHostmaster[100]={""};
static char cNSs[1024]={""};
static char cNSSet[100]={""};
static char cuSerial[16]={""};
static char cuExpire[16]={""};
static char cuRefresh[16]={""};
static char cuTTL[16]={""};
static char cuRetry[16]={""};
static char cuZoneTTL[16]={""};
extern char cuNameServer[];
static char cCompany[100]={""};
static unsigned uView=0;
static unsigned uRegistrar=0;
static unsigned uNameServer=0;
static unsigned uOwner=0;
static unsigned uMailServers=0;
static unsigned uSecondaryOnly=0;
static char cOptions[255]={""};
static char cMasterIPs[100]={""};

static char cSearch[100]={""};
static char *cSearchStyle="type_fields";

static char cNavList[16384]="No results.";
static char cRcdFound[100]={""};

extern unsigned uForClient;

void SearchDeletedZone(char *cSearchTerm);
void LoadDeletedZone(unsigned uRowId);
void RestoreZone(unsigned uRowId);
void RestoreRRs(unsigned uRowId);
void DeleteRestoreZone(unsigned uRowId);

#define VAR_LIST_tDeletedResource "tDeletedResource.uDeletedResource,tDeletedResource.uZone,tDeletedResource.cName,tDeletedResource.uTTL,tDeletedResource.uRRType,tDeletedResource.cParam1,tDeletedResource.cParam2,tDeletedResource.cComment,tDeletedResource.uOwner"

void ProcessRestoreZoneVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cSearch"))
			sprintf(cSearch,"%.99s",entries[i].val);
		else if( !strcmp(entries[i].name,"uDeletedZone"))
			sscanf(entries[i].val,"%u",&uDeletedZone);
		else if( !strcmp(entries[i].name,"cZone"))
			sprintf(cZone,"%.99s",entries[i].val);
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
		else if(!strcmp(entries[i].name,"cNavList"))
			sprintf(cNavList,"%.8191s",entries[i].val);
		else if(!strcmp(entries[i].name,"cView"))
			sprintf(cView,"%s",entries[i].val);
		else if(!strcmp(entries[i].name,"cCompany"))
			sprintf(cCompany,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cNSs"))
			sprintf(cNSs,"%.1023s",entries[i].val);
		else if(!strcmp(entries[i].name,"cNSSet"))
			sprintf(cNSSet,"%.99s",entries[i].val);
	}

}//void ProcessRestoreZoneVars(pentry entries[], int x)


void RestoreZoneGetHook(entry gentries[],int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uDeletedZone"))
			sscanf(gentries[i].val,"%u",&uDeletedZone);
	}

	if(uDeletedZone)
		LoadDeletedZone(uDeletedZone);

	htmlRestoreZone();

}//void RestoreZoneGetHook(entry gentries[],int x)


void RestoreZoneCommands(pentry entries[], int x)
{
	if(!strcmp(gcPage,"RestoreZone"))
	{
		ProcessRestoreZoneVars(entries,x);
		
		if(!strcmp(gcFunction,"Restore Zone"))
		{
			if(!uDeletedZone)
			{
				gcMessage="<blink>Error: </blink>You must select a zone to restore.";
				htmlRestoreZone();
			}
			gcMessage="Double check that you want to restore this zone and all its RRs";
			sprintf(gcNewStep,"Confirm ");
		}
		else if(!strcmp(gcFunction,"Confirm Restore Zone"))
		{
			time_t luClock;

			LoadDeletedZone(uDeletedZone);

			RestoreZone(uDeletedZone);
			RestoreRRs(uDeletedZone);
			DeleteRestoreZone(uDeletedZone);
			//
			//Submit job to restore zone all across the cluster
			sscanf(cuNameServer,"%u",&uNameServer);
			time(&luClock);
			if(AdminSubmitJob("New",uNameServer,cZone,0,luClock))
				htmlPlainTextError(mysql_error(&gMysql));
			gcMessage="Zone restored ok. Wait a few minutes so it gets propagated trough the NS cluster.";
			sprintf(gcZone,"%.99s",cZone);
			sprintf(cuView,"%u",uView);

			sprintf(gcCookieZone,"%.99s",cZone);
			guCookieView=uView;
			SetSessionCookie();
			
		}
		if(!strcmp(gcFunction,"Back to Zones Tab"))
		{
			htmlZone();
		}
		else if(!strcmp(gcFunction,"Search"))
		{
			MYSQL_RES *res;
			MYSQL_ROW field;
			char cTmp[512]={""};
			unsigned uRows=0;
			unsigned uDisplayCount=0;

			if(!cSearch[0])
			{
				cSearchStyle="type_fields_req";
				gcMessage="Must provide a valid search term";
				htmlRestoreZone();
			}
			
			SearchDeletedZone(cSearch);
			res=mysql_store_result(&gMysql);

			uRows=mysql_num_rows(res);
			
			if(uRows)
			{
				sprintf(cNavList,"<!-- Search NavList Start -->\n");

				if(uRows==1)
				{
					field=mysql_fetch_row(res);					
					cNavList[0]=0;
					sscanf(field[0],"%u",&uDeletedZone);
					LoadDeletedZone(uDeletedZone);
					htmlRestoreZone();
				}

				while((field=mysql_fetch_row(res)))
				{
					if(strlen(cNavList) > 8000 || (uDisplayCount==20)) break; //avoid buffer overflow
					sprintf(cTmp,"<a href=idnsAdmin.cgi?gcPage=RestoreZone&uDeletedZone=%s>%s [%s]</a><br>\n",
							field[0]
							,field[1]
							,field[2]);
					strcat(cNavList,cTmp);
					uDisplayCount++;
				}
				if(uDisplayCount<uRows)
				{
					sprintf(cTmp,"<br>Only the first %u shown (%u results). If the zone you are looking for is not in the list above please further refine your search.<br>",uDisplayCount,uRows);
					strcat(cNavList,cTmp);
				}
				strcat(cNavList,"<!-- Search NavList End -->\n");
				sprintf(cRcdFound,"%u record(s) found.",uRows);
				gcMessage=cRcdFound;
			}
			else
			{
				gcMessage="No records found";
				htmlRestoreZone();
			}
					
		}
		
		htmlRestoreZone();
	}

}//void RestoreZoneCommands(pentry entries[], int x)


void htmlRestoreZone(void)
{
	htmlHeader("idnsAdmin","Header");
	htmlRestoreZonePage("idnsAdmin","RestoreZone.Body");
	htmlFooter("Footer");

}//void htmlRestoreZone(void)


void htmlRestoreZonePage(char *cTitle, char *cTemplateName)
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
			char cuDeletedZone[16]={""};
			char cuResource[16]={""};

			sprintf(cuResource,"%u",uResource);
			sprintf(cuDeletedZone,"%u",uDeletedZone);

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
			char cZoneView[100];
			sprintf(cZoneView,cZone);
			if(uView)
				sprintf(cZoneView,"%.63s/%.31s",cZone,ForeignKey("tView","cLabel",uView));
			template.cpValue[8]=cZoneView;

			template.cpName[9]="gcMessage";
			template.cpValue[9]=gcMessage;

			//
			//Zone record vars
			template.cpName[10]="cNSSet";
			template.cpValue[10]=cNSSet;

			template.cpName[11]="uNameServer";
			template.cpValue[11]=cuNameServer;

			template.cpName[12]="cMainAddress";
			template.cpValue[12]=cMainAddress;

			template.cpName[13]="uResource";
			template.cpValue[13]=cuResource;

			template.cpName[14]="cHostmaster";
			template.cpValue[14]=cHostmaster;

			template.cpName[15]="uSerial";
			template.cpValue[15]=cuSerial;

			template.cpName[16]="uExpire";
			template.cpValue[16]=cuExpire;

			template.cpName[17]="uRefresh";
			template.cpValue[17]=cuRefresh;

			template.cpName[18]="uTTL";
			template.cpValue[18]=cuTTL;

			template.cpName[19]="uRetry";
			template.cpValue[19]=cuRetry;

			template.cpName[20]="uZoneTTL";
			template.cpValue[20]=cuZoneTTL;

			template.cpName[21]="cSearchStyle"; 
			template.cpValue[21]=cSearchStyle;

			template.cpName[22]="cNavList";
			template.cpValue[22]=cNavList;

			template.cpName[23]="cView";
			template.cpValue[23]=cView;

			template.cpName[24]="cCompany";
			template.cpValue[24]=cCompany;
			
			template.cpName[25]="gcNewStep";
			template.cpValue[25]=gcNewStep;

			template.cpName[26]="uDeletedZone";
			template.cpValue[26]=cuDeletedZone;

			template.cpName[27]="cBtnStatus";
			if(uDeletedZone)
				template.cpValue[27]="";
			else
				template.cpValue[27]="disabled";

			template.cpName[28]="uView";
			template.cpValue[28]=cuView;

			template.cpName[29]="";
						
			printf("\n<!-- Start htmlRestoreZonePage(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlRestoreZonePage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlRestoreZonePage()


void SearchDeletedZone(char *cSearchTerm)
{
	//
	//Will search deleted zones only for valid clients. By valid clients we mean those that have a tClient instance.
	//
	sprintf(gcQuery,"SELECT tDeletedZone.uDeletedZone,tDeletedZone.cZone,tView.cLabel FROM tDeletedZone,tView,tClient"
			" WHERE tView.uView=tDeletedZone.uView AND tDeletedZone.cZone LIKE '%s%%' AND"
			" tDeletedZone.uOwner=tClient.uClient",cSearchTerm);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
}//void SearchDeletedZone(char *cSearchTerm)


void LoadDeletedZone(unsigned uRowId)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	
	sprintf(gcQuery,"SELECT uDeletedZone,tDeletedZone.cZone,cHostmaster,tDeletedZone.uSerial,tDeletedZone.uExpire,"
			"tDeletedZone.uRefresh,tDeletedZone.uTTL,tDeletedZone.uRetry,tDeletedZone.uZoneTTL,tView.cLabel,"
			"tClient.cLabel,tDeletedZone.uNSSet,tNSSet.cLabel,tDeletedZone.uView,tDeletedZone.uRegistrar,"
			"tDeletedZone.uOwner,tDeletedZone.uMailServers,tDeletedZone.uSecondaryOnly,tDeletedZone.cOptions,"
			"tNSSet.cMasterIPs FROM tDeletedZone,tView,tClient,tNSSet WHERE uDeletedZone=%u AND"
			" tView.uView=tDeletedZone.uView AND tClient.uClient=tDeletedZone.uOwner AND"
			" tDeletedZone.uNSSet=tNSSet.uNSSet",uRowId);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);

	if((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&uDeletedZone);
		sprintf(cZone,"%.99s",field[1]);
		sprintf(cHostmaster,"%.99s",field[2]);
		sprintf(cuSerial,"%.15s",field[3]);
		sprintf(cuExpire,"%.15s",field[4]);
		sprintf(cuRefresh,"%.15s",field[5]);
		sprintf(cuTTL,"%.15s",field[6]);
		sprintf(cuRetry,"%.15s",field[7]);
		sprintf(cuZoneTTL,"%.15s",field[8]);
		sprintf(cView,"%s",field[9]);
		sprintf(cCompany,"%.99s",field[10]);
		sprintf(cuNameServer,"%.15s",field[11]);
		sscanf(field[11],"%u",&uNameServer);
		sprintf(cNSSet,"%99s",field[12]);
		sscanf(field[13],"%u",&uView);
		sscanf(field[14],"%u",&uRegistrar);
		sscanf(field[15],"%u",&uOwner);
		sscanf(field[16],"%u",&uMailServers);
		sscanf(field[17],"%u",&uSecondaryOnly);
		sprintf(cOptions,"%.254s",field[18]);
		sprintf(cMasterIPs,"%.99s",field[19]);
		sprintf(cNavList,"<a href='idnsAdmin.cgi?gcPage=RestoreZone&uDeletedZone=%u'>%s [%s]</a><br>\n",uDeletedZone,cZone,cView);
		gcMessage="1 record(s) found.";
	}
	
}//void LoadDeletedZone(unsigned uRowId)


void funcDeletedRRs(FILE *fp,unsigned uShowLinks)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	fprintf(fp,"<!-- funcRRs(fp) Start -->\n");
	if(!uShowLinks)
	{
		sprintf(gcQuery,"SELECT tDeletedResource.uDeletedResource,IF(STRCMP(tDeletedResource.cName,''),"
			"tDeletedResource.cName,'(default)'),tDeletedResource.uTTL,tRRType.cLabel,"
			"tDeletedResource.cParam1,tDeletedResource.cParam2,tDeletedResource.cComment FROM"
			" tDeletedResource,tRRType,tDeletedZone WHERE tDeletedResource.uZone=tDeletedZone.uDeletedZone AND"
			" tDeletedResource.uRRType=tRRType.uRRType AND tDeletedZone.uDeletedZone='%u' AND"
			" tDeletedZone.uSecondaryOnly=0 ORDER BY tDeletedResource.uRRType,tDeletedResource.cName",
				uDeletedZone);
	}
	else
	{
		sprintf(gcQuery,"SELECT tDeletedResource.uDeletedResource,IF(STRCMP(tDeletedResource.cName,''),"
				"tDeletedResource.cName,'(default)'),tDeletedResource.uTTL,tRRType.cLabel,"
				"tDeletedResource.cParam1,tDeletedResource.cParam2,tDeletedResource.cComment FROM"
				" tDeletedResource,tRRType,tZone WHERE tDeletedResource.uZone=tZone.uZone AND"
				" tDeletedResource.uRRType=tRRType.uRRType AND tZone.uZone='%u' AND"
				" tZone.uSecondaryOnly=0 ORDER BY tDeletedResource.uRRType,tDeletedResource.cName",
					uDeletedZone);
	}
	
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{

		if(!field[2][0] || field[2][0]=='\t')
			sprintf(field[2],"@");
		
		fprintf(fp,"<tr>\n");
		if(uShowLinks)
			fprintf(fp,"<td valign=top><a class=darkLink href=idnsAdmin.cgi?gcPage=RestoreResource&"
					"uDeletedResource=%s>%s</a></td>",
				field[0],
				field[1]
			);
		else
			fprintf(fp,"<td valign=top>%s</td>",field[1]);
		
		fprintf(fp,"<td valign=top>%s</td><td valign=top>%s</td><td valign=top>%s</td><td valign=top>%s</td>"
				"<td valign=top>%s</td>\n",
				field[2],
				field[3],
				field[4],
				field[5],
				field[6]
		       );
		fprintf(fp,"</tr>\n");
		
	}
	mysql_free_result(res);


	fprintf(fp,"<!-- funcRRs(fp) End -->\n");

}//void funcDeletedRRs(FILE *fp)


void RestoreZone(unsigned uRowId)
{
	MYSQL_RES *res;
	sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=%u",cZone,uView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{
		gcMessage="<blink>Error: </blink>Error The zone already exists in the system. Can't restore";
		htmlRestoreZone();
	}

	//
	//Restore tZone record
	sprintf(gcQuery,"INSERT INTO tZone (uZone,cZone,uNSSet,cHostmaster,uSerial,uExpire,"
			"uRefresh,uTTL,uRetry,uZoneTTL,uMailServers,uView,cMainAddress,"
			"uRegistrar,uSecondaryOnly,cOptions,uOwner,uCreatedBy,uCreatedDate) "
			"SELECT uDeletedZone,cZone,uNSSet,cHostmaster,uSerial,uExpire,uRefresh,"
			"uTTL,uRetry,uZoneTTL,uMailServers,uView,cMainAddress,uRegistrar,"
			"uSecondaryOnly,cOptions,uOwner,uCreatedBy,UNIX_TIMESTAMP(NOW()) "
			"FROM tDeletedZone WHERE uDeletedZone=%u",uRowId
			);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	iDNSLog(uDeletedZone,"tZone","New (Restore Zone)");
	
}//void RestoreZone(unsigned uRowId)

void RestoreRRs(unsigned uRowId)
{
	//
	//Restore tResource record(s) if available
	
	sprintf(gcQuery,"INSERT INTO tResource (uResource,uZone,cName,uTTL,uRRType,cParam1,cParam2,"
			"cComment,uCreatedBy,uCreatedDate) SELECT uDeletedResource,uZone,cName,uTTL,uRRType,"
			"cParam1,cParam2,cComment,uCreatedBy,UNIX_TIMESTAMP(NOW()) FROM tDeletedResource "
			"WHERE uZone=%u",
			uRowId
			);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

}//void RestoreRRs(unsigned uRowId)


void DeleteRestoreZone(unsigned uRowId)
{	
	//
	//Now remove tDeletedZone and tDeletedResource records
	sprintf(gcQuery,"DELETE FROM tDeletedZone WHERE uDeletedZone=%u",uRowId);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	sprintf(gcQuery,"DELETE FROM tDeletedResource WHERE uZone=%u",uRowId);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

}///void DeleteRestoreZone(unsigned uRowId)


