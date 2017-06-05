/*
FILE 
	zone.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2010-2015 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See included LICENSE file.
PURPOSE
	unxsDNS program file.
	Zone and related tab functions.
WORK NOTES
	Moving from get arg for zone and zone search pattern to backend tGroup,tGroupGlue and tGroupType
	saved data.
	Current zone: tGroupType.uGroupType=21 "selected zone"
	Current zone search pattern: A tConfiguration entry for uOwner=guLoginClient
	Nothing comes from get link passed data.

	Session data:
		guZone
		gcSearch
		
*/

#include "interface.h"

extern unsigned guBrowserFirefox;//main.c
char gcZone[100]={""};
static char gcSearch[100]={""};
static char gcSearchAux[32]={""};
//static char *gcSearchName={""};
unsigned guStatus=0;
unsigned guNewZone=0;
unsigned guReseller=0;
static char gcNewZoneTZ[64]={"PST8PDT"};
//static unsigned guNode=0;
//static unsigned guDatacenter=0;
//Zone details
//static unsigned guFlag=0;
static char gcLabel[33]={""};
static char gcNewHostname[33]={""};
static char gcNewHostParam0[33]={""};
static char gcNewHostParam1[33]={""};
//static char gcServer[32]={""};
static char *gcBulkData={""};
static char gcCustomerName[33]={""};
static char gcCustomerLimit[16]={""};
static char gcNewLogin[33]={"John Doe"};
static char gcNewEmail[65]={""};
static char gcNewPasswd[33]={""};
static char gcAuthCode[33]={""};
static char *gcShowDetails="";
char gcAdminPort[16]=":3321";
char gcInCollapseSearch[16]="out";

unsigned guMode;
unsigned guZoneSubmit=0;
unsigned guSearchSubmit=0;



void EncryptPasswdWithSalt(char *pw, char *salt);

//TOC incomplete TODO
void ProcessZoneVars(pentry entries[], int x);
void htmlZoneList(void);
void htmlZoneQOS(void);
void htmlZoneBulk(void);
void ZoneGetHook(entry gentries[],int x);
char *cGetZonename(unsigned uZone);
char *cGetImageHost(unsigned uZone);
void SelectZone(void);
char *CustomerName(char *cInput);
char *NameToLower(char *cInput);
char *CustomerEmail(char *cInput);
char *cNumbersOnly(char *cInput);
void SetZoneStatus(unsigned uZone,unsigned uStatus);
void funcZoneList(FILE *fp);
void SendAlertEmail(char *cMsg);
void funcZoneBulk(FILE *fp);
char *ParseTextAreaLines(char *cTextArea);
char *random_pw(char *dest);
void GetConfigurationValue(char const *cName,char *cValue,unsigned uDatacenter,unsigned uNode,unsigned uZone);
unsigned uGetPrimaryZoneGroup(unsigned uZone);
void GetZonePropertyValue(char const *cName,char *cValue,unsigned uZone);
void htmlSearchCollapseText(FILE *fp);

void htmlAuxPage(char *cTitle, char *cTemplateName);
void htmlAbout(void);
void htmlContact(void);

//session related
void GetZoneFromSessionGroup(void);
unsigned uGetSessionGroup(const char *gcUser,unsigned uGroupType);
unsigned uGetSessionGroupZone(void);
unsigned uGetZoneFromZoneGroup(unsigned uGroup);
unsigned uGetZoneSearch(unsigned guLoginClient);
void UpdateSessionZone(unsigned guZone);
void UpdateSessionZoneSearch(char *gcSearch);

void htmlResourceRecords(void);
void htmlSearchCollapse(void);


unsigned uGetClientPermLevel(uClient)
{
	unsigned uRetVal=0;
        MYSQL_RES *res;
	MYSQL_ROW field;
	sprintf(gcQuery,"SELECT uPerm FROM tAuthorize WHERE uCertClient=%u",uClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		return(uRetVal);
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uRetVal);
	mysql_free_result(res);
	return(uRetVal);
}//unsigned uGetClientPermLevel(uClient)


unsigned uPower10(unsigned uI)
{
	static unsigned uTotal=1;
	register int i;

	for(i=0;uI>0;uI--)
		uTotal=uTotal*10;

	return(uTotal);

}//unsigned uPower10(unsigned uI)


void ProcessZoneVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"guZone"))
		{
			sscanf(entries[i].val,"%u",&guZone);
			UpdateSessionZone(guZone);
		}
		else if(!strcmp(entries[i].name,"guZoneSubmit"))
		{
			sscanf(entries[i].val,"%u",&guZoneSubmit);
			if(guZoneSubmit)
			{
				guZone=guZoneSubmit;
				UpdateSessionZone(guZoneSubmit);
			}
		}
		else if(!strcmp(entries[i].name,"gcZone"))
			sprintf(gcZone,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"gcSearchSubmit"))
		{
			sprintf(gcSearch,"%.99s",entries[i].val);
			guSearchSubmit=1;
			UpdateSessionZoneSearch(gcSearch);
			sprintf(gcInCollapseSearch,"%.15s","in");
			//Need to use function in template connected to in state
		}
		else if(!strcmp(entries[i].name,"gcSearch"))
			sprintf(gcSearch,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"gcSearchAux"))
			sprintf(gcSearchAux,"%.31s",entries[i].val);
		else if(!strcmp(entries[i].name,"guNewZone"))
			sscanf(entries[i].val,"%u",&guNewZone);
		else if(!strcmp(entries[i].name,"guReseller"))
			sscanf(entries[i].val,"%u",&guReseller);
		else if(!strcmp(entries[i].name,"gcNewHostname"))
			sprintf(gcNewHostname,"%.32s",NameToLower(entries[i].val));
		else if(!strcmp(entries[i].name,"gcNewZoneTZ"))
			sprintf(gcNewZoneTZ,"%.63s",entries[i].val);
		else if(!strcmp(entries[i].name,"gcNewHostParam0"))
			sprintf(gcNewHostParam0,"%.32s",NameToLower(entries[i].val));
		else if(!strcmp(entries[i].name,"gcNewHostParam1"))
			sprintf(gcNewHostParam1,"%.32s",CustomerName(entries[i].val));
		else if(!strcmp(entries[i].name,"gcCustomerName"))
			sprintf(gcCustomerName,"%.32s",CustomerName(entries[i].val));
		else if(!strcmp(entries[i].name,"gcCustomerLimit"))
			sprintf(gcCustomerLimit,"%.15s",CustomerName(entries[i].val));
		else if(!strcmp(entries[i].name,"gcAuthCode"))
			sprintf(gcAuthCode,"%.32s",CustomerName(entries[i].val));
		else if(!strcmp(entries[i].name,"gcShowDetails"))
			gcShowDetails="checked";
		else if(!strcmp(entries[i].name,"gcNewLogin"))
			sprintf(gcNewLogin,"%.31s",CustomerName(entries[i].val));
		else if(!strcmp(entries[i].name,"gcNewPasswd"))
			sprintf(gcNewPasswd,"%.31s",CustomerName(entries[i].val));
		else if(!strcmp(entries[i].name,"gcBulkData"))
			gcBulkData=entries[i].val;
		else if(!strcmp(entries[i].name,"gcNewEmail"))
			sprintf(gcNewEmail,"%.99s",CustomerEmail(entries[i].val));
	}

}//void ProcessZoneVars(pentry entries[], int x)


void ZoneGetHook(entry gentries[],int x)
{
	if(!strcmp(gcPage,"About"))
		htmlAbout();
	else if(!strcmp(gcPage,"Contact"))
		htmlContact();

	if(!strcmp(gcFunction,"ResourceRecords"))
			htmlResourceRecords();
	if(!strcmp(gcFunction,"SearchCollapse"))
			htmlSearchCollapse();

	htmlZone();

}//void ZoneGetHook(entry gentries[],int x)


void ZoneCommands(pentry entries[], int x)
{
	if(!strcmp(gcPage,"Zone"))
	{
		ProcessZoneVars(entries,x);
		htmlZone();
	}

}//void ZoneCommands(pentry entries[], int x)


void htmlAbout(void)
{
	htmlHeader("unxsDNS","ZoneHeader");
	GetZoneFromSessionGroup();
	uGetZoneSearch(guLoginClient);
	htmlAuxPage("unxsDNS","About.Body");
	htmlFooter("ZoneFooter");

}//void htmlAbout(void)


void htmlContact(void)
{
	htmlHeader("unxsDNS","ZoneHeader");
	htmlAuxPage("unxsDNS","Contact.Body");
	htmlFooter("ZoneFooter");

}//void htmlContact(void)

#define uSELECTEDZONE 21

void GetZoneFromSessionGroup(void)
{
	unsigned uGroup=0;
	if((uGroup=uGetSessionGroup(gcUser,uSELECTEDZONE)))
	{
		guZone=uGetZoneFromZoneGroup(uGroup);
	}
}//void GetZoneFromSessionGroup(void)


void htmlZone(void)
{
	//zone search memory
	GetZoneFromSessionGroup();
	uGetZoneSearch(guLoginClient);

	htmlHeader("unxsDNS","ZoneHeader");
	if(guZone)
		SelectZone();
	htmlZonePage("unxsDNS","Zone.Body");
	htmlFooter("ZoneFooter");

}//void htmlZone(void)


void htmlZoneList(void)
{
	htmlHeader("unxsDNS","ZoneHeader");
	htmlZonePage("unxsDNS","ZoneList.Body");
	htmlFooter("ZoneFooter");

}//void htmlZoneList(void)


void htmlAuxPage(char *cTitle, char *cTemplateName)
{
	if(cTemplateName[0])
	{
        	MYSQL_RES *res;
	        MYSQL_ROW field;
		unsigned uNoShow=0;

		TemplateSelectInterface(cTemplateName,uPLAINSET,uDNS);
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			struct t_template template;

			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
			
			template.cpName[1]="cCGI";
			template.cpValue[1]="unxsDNS.cgi";
			
			template.cpName[2]="gcLogin";
			template.cpValue[2]=gcUser;

			template.cpName[3]="gcName";
			template.cpValue[3]=gcName;

			template.cpName[4]="gcOrgName";
			template.cpValue[4]=gcOrgName;

			template.cpName[5]="cUserLevel";
			template.cpValue[5]=(char *)cUserLevel(guPermLevel);//Safe?

			template.cpName[6]="gcHost";
			template.cpValue[6]=gcHost;

			template.cpName[7]="gcMessage";
			template.cpValue[7]=gcMessage;

			template.cpName[8]="gcZone";
			template.cpValue[8]=gcZone;
			if(!uNoShow)
				template.cpValue[8]=(char *)cGetZonename(guZone) ;
			else
				template.cpValue[8]="no zone selected";

			template.cpName[9]="gcSearch";
			template.cpValue[9]=gcSearch;

			template.cpName[10]="guZone";
			char cguZone[16];
			sprintf(cguZone,"%u",guZone);
			template.cpValue[10]=cguZone;

			template.cpName[11]="gcLabel";
			template.cpValue[11]=gcLabel;

			template.cpName[12]="gcCopyright";
			template.cpValue[12]=LOCALCOPYRIGHT;

			template.cpName[13]="gcBrand";
			template.cpValue[13]=INTERFACE_HEADER_TITLE;

			char cContactEmail[256]={""};
			sprintf(cContactEmail,"support@unixservice.com");
			//GetConfiguration("cOrg_ContactEmail",cContactEmail,0);
			template.cpName[14]="cContactEmail";
			template.cpValue[14]=cContactEmail;

			char cCtHostnameLink[128]={"no zone selected"};
			sprintf(cCtHostnameLink,"<a href=https://%s%s/admin >%s</a>",template.cpValue[8],gcAdminPort,template.cpValue[8]);
			template.cpName[15]="cCtHostnameLink";
			template.cpValue[15]=cCtHostnameLink;

			char cPrivilegedZoneMenu[256]={""};
			template.cpName[16]="cPrivilegedZoneMenu";
			//if(guPermLevel>=6)
				//sprintf(cPrivilegedZoneMenu,
				//	"<li><a href=\"%1$.32s?gcPage=Repurpose&guZone=%2$u\">Repurpose</a></li>"
				//	"<li><a href=\"%1$.32s?gcPage=Reseller&guZone=%2$u\">Reseller</a></li>"
				//		,template.cpValue[1],guZone);
			template.cpValue[16]=cPrivilegedZoneMenu;

			template.cpName[17]="gcOTPSecretExists";
			char cExists[16]={"No"};
			if(gcOTPSecret[0])
				sprintf(cExists,"Yes");
			template.cpValue[17]=cExists;

			template.cpName[18]="guLoginClient";
			char cguLoginClient[16];
			sprintf(cguLoginClient,"%u",guLoginClient);
			template.cpValue[18]=cguLoginClient;

			template.cpName[19]="";

			printf("\n<!-- Start htmlAuxPage(%s) -->\n",cTemplateName); 
			Template(field[0],&template,stdout);
			printf("\n<!-- End htmlAuxPage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlAuxPage()


void htmlZonePage(char *cTitle, char *cTemplateName)
{
	if(cTemplateName[0])
	{
        	MYSQL_RES *res;
	        MYSQL_ROW field;
		unsigned uNoShow=0;

		TemplateSelectInterface(cTemplateName,uPLAINSET,uDNS);
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			struct t_template template;

			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
			
			template.cpName[1]="cCGI";
			template.cpValue[1]="unxsDNS.cgi";
			
			template.cpName[2]="gcLogin";
			template.cpValue[2]=gcUser;

			template.cpName[3]="gcName";
			template.cpValue[3]=gcName;

			template.cpName[4]="gcOrgName";
			template.cpValue[4]=gcOrgName;

			template.cpName[5]="cUserLevel";
			template.cpValue[5]=(char *)cUserLevel(guPermLevel);//Safe?

			template.cpName[6]="gcHost";
			template.cpValue[6]=gcHost;

			template.cpName[7]="gcMessage";
			template.cpValue[7]=gcMessage;

			template.cpName[8]="gcZone";
			template.cpValue[8]=gcZone;
			if(!uNoShow)
				template.cpValue[8]=(char *)cGetZonename(guZone) ;
			else
				template.cpValue[8]="no zone selected";

			template.cpName[9]="gcSearch";
			template.cpValue[9]=gcSearch;

			template.cpName[10]="guZone";
			char cguZone[16];
			sprintf(cguZone,"%u",guZone);
			template.cpValue[10]=cguZone;

			template.cpName[11]="gcLabel";
			template.cpValue[11]=gcLabel;

			template.cpName[12]="cDisabled";
			if(guZone)
				template.cpValue[12]="";
			else
				template.cpValue[12]="disabled";

			template.cpName[13]="gcSearchAux";
			template.cpValue[13]=gcSearchAux;

			//MYSQL_RES *res0;
			//MYSQL_ROW field0;
			//char cQuery[128];
			//sprintf(cQuery,"SELECT cValue FROM tProperty WHERE uType=3 AND uKey=%u AND cName='cOrg_AdminPort'",guZone);
			//mysql_query(&gMysql,cQuery);
			//if(mysql_errno(&gMysql))
			//	htmlPlainTextError(mysql_error(&gMysql));
			//res0=mysql_store_result(&gMysql);
			//if((field0=mysql_fetch_row(res0)))
			//	sprintf(gcAdminPort,":%.14s",field0[0]);
			//mysql_free_result(res0);
			template.cpName[14]="gcAdminPort";
			template.cpValue[14]=gcAdminPort;

			template.cpName[15]="gcBrand";
			template.cpValue[15]=INTERFACE_HEADER_TITLE;

			template.cpName[16]="gcCopyright";
			template.cpValue[16]=LOCALCOPYRIGHT;

			template.cpName[17]="gcInCollapseSearch";
			template.cpValue[17]=gcInCollapseSearch;

			char cCtHostnameLink[128]={"no zone selected"};
			sprintf(cCtHostnameLink,"<a href=https://%s%s/admin >%s</a>",template.cpValue[8],gcAdminPort,template.cpValue[8]);
			template.cpName[18]="cCtHostnameLink";
			template.cpValue[18]=cCtHostnameLink;

			char cPrivilegedZoneMenu[512]={""};
			template.cpName[19]="cPrivilegedZoneMenu";
			//if(guPermLevel>=6)
			//{
			//	sprintf(cPrivilegedZoneMenu,
			//		"<li><a href=\"%1$.32s?gcPage=Repurpose&guZone=%2$u\">Repurpose</a></li>"
			//		"<li><a href=\"%1$.32s?gcPage=Reseller&guZone=%2$u\">Reseller</a></li>"
			//			,template.cpValue[1],guZone);
			//}
			template.cpValue[19]=cPrivilegedZoneMenu;

			template.cpName[20]="";

			printf("\n<!-- Start htmlZonePage(%s) -->\n",cTemplateName); 
			Template(field[0],&template,stdout);
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


void funcZoneImageTag(FILE *fp)
{
	if(!guZone)
		return;

}//void funcZoneImageTag(FILE *fp)


void funcSearchSubmit(FILE *fp)
{
	fprintf(fp,"<!-- funcSearchSubmit(fp) Start -->\n");
	if(gcInCollapseSearch[0]=='i')
	{
		htmlSearchCollapseText(fp);
	}
	else
	{
		fprintf(fp,"loading...\n");
	}
	fprintf(fp,"<!-- funcSearchSubmit(fp) End -->\n");
}//void funcSearchSubmit(FILE *fp)


void funcSelectZone(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uCount=1;
	unsigned uZone=0;

	fprintf(fp,"<!-- funcSelectZone(fp) Start -->\n");

	if(guPermLevel>5)
	{
		if(gcSearch[0])
			sprintf(gcQuery,"SELECT tZone.uZone,tZone.cZone FROM tZone WHERE"
			" (tZone.uOwner IN"
			" (SELECT uClient FROM tClient WHERE"
			" 	((uOwner IN (SELECT uClient FROM tClient WHERE uOwner=%u) OR uOwner=%u) AND cCode='Organization'))"
			" OR tZone.uOwner=%u) AND"
			" tZone.uView=2 AND"
			" tZone.cZone LIKE '%s%%'"
			" ORDER BY tZone.cZone LIMIT 301",guOrg,guOrg,guOrg,gcSearch);
		else
			sprintf(gcQuery,"SELECT tZone.uZone,tZone.cZone FROM tZone WHERE"
			" (tZone.uOwner IN"
			" (SELECT uClient FROM tClient WHERE ((uOwner IN (SELECT uClient FROM tClient WHERE uOwner=%u)"
			" 	OR uOwner=%u) AND " " 	cCode='Organization'))"
			" OR tZone.uOwner=%u) AND"
			" tZone.uView=2 "
			" ORDER BY tZone.cZone LIMIT 301",guOrg,guOrg,guOrg);
	}
	else
	{
		if(gcSearch[0])
			sprintf(gcQuery,"SELECT tZone.uZone,tZone.cZone FROM tZone WHERE"
			" tZone.uCreatedBy=%u AND tZone.cZone LIKE '%s%%' AND"
			" tZone.uView=2 "
			" ORDER BY tZone.cZone LIMIT 301",guLoginClient,gcSearch);
		else
			sprintf(gcQuery,"SELECT tZone.uZone,tZone.cZone FROM tZone WHERE"
			" tZone.uCreatedBy=%u AND "
			" tZone.uView=2 "
			" ORDER BY tZone.cZone LIMIT 301",guLoginClient);
	}

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	fprintf(fp,"<select type='hostnameSelect' id='SelectZone' class='form-control' title='Select the zone you want to load with this dropdown'"
			" name='guZoneSubmit' onChange='submit()'>\n");
	fprintf(fp,"<option value=0>---</option>");
	while((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&uZone);
		fprintf(fp,"<option value=%s",field[0]);
		if(guZone==uZone)
			fprintf(fp," selected");
		if((uCount++)<=300)
			fprintf(fp,">%s</option>",field[1]);
		else
			fprintf(fp,">Limit reached! Use better filter</option>");
	}
	mysql_free_result(res);

	fprintf(fp,"</select>\n");

	fprintf(fp,"<!-- funcSelectZone(fp) End -->\n");

}//void funcSelectZone(FILE *fp)


char *cGetZonename(unsigned uZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	static char cZone[100]={""};

	sprintf(gcQuery,"SELECT cZone FROM tZone WHERE uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		return((char *)mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sprintf(cZone,"%.99s",field[0]);
	return(cZone);
}//char *cGetZonename(unsigned uZone)


void SelectZone(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT cZone FROM tZone WHERE uZone=%u",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sprintf(gcLabel,"%.32s",field[0]);

		if(!gcMessage[0]) gcMessage="Zone Selected";
	}
	else
	{
		gcLabel[0]=0;
		gcMessage="<blink>No Zone Selected</blink>";
	}
	
	mysql_free_result(res);
	
}//void SelectZone(void)



void funcZoneInfo(FILE *fp)
{
	if(!guZone)
		return;

	MYSQL_RES *res;
	MYSQL_ROW field;

	fprintf(fp,"<!-- funcZoneInfo (fp) Start -->\n");

	//Temporary hack using uCreatedBy for access control
	sprintf(gcQuery,"SELECT uZone FROM tZone WHERE"
				" ("
				" uCreatedBy=%u OR uCreatedBy=%u OR"
				" (uOwner IN (SELECT uClient FROM tClient WHERE"
				" ((uOwner IN (SELECT uClient FROM tClient WHERE uOwner=%u) OR uOwner=%u) AND "
				" cCode='Organization')) OR uOwner=%u)"
				" ) AND"
				" uZone=%u",
					guLoginClient,guOrg,guOrg,guOrg,guOrg,guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res)<1)
		return;

	//Serial
	sprintf(gcQuery,"SELECT uSerial FROM tZone WHERE uZone=%u",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		printf("<li class=list-group-item ><span class=badge >%s</span>Serial Number</li>\n",field[0]);
	}

	//View
	sprintf(gcQuery,"SELECT tView.cLabel FROM tZone,tView WHERE tZone.uView=tView.uView AND"
			" tZone.uZone=%u",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		printf("<li class=list-group-item ><span class=badge >%s</span>View</li>\n",field[0]);
	}

	//TTL
	sprintf(gcQuery,"SELECT uTTL FROM tZone WHERE uZone=%u",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		printf("<li class=list-group-item ><span class=badge >%s</span>TTL</li>\n",field[0]);
	}

	//Owner
	sprintf(gcQuery,"SELECT tClient.cLabel FROM tZone,tClient WHERE tZone.uOwner=tClient.uClient AND"
			" tZone.uZone=%u",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		printf("<li class=list-group-item ><span class=badge >%s</span>Owner</li>\n",field[0]);
	}

	//CreatedBy
	sprintf(gcQuery,"SELECT tClient.cLabel FROM tZone,tClient WHERE tZone.uCreatedBy=tClient.uClient AND"
			" tZone.uZone=%u",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		printf("<li class=list-group-item ><span class=badge >%s</span>Created By</li>\n",field[0]);
	}

	//CreatedDate
	sprintf(gcQuery,"SELECT FROM_UNIXTIME(uCreatedDate,'%%a %%b %%d %%T %%Y') FROM tZone WHERE uZone=%u",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		printf("<li class=list-group-item ><span class=badge >%s</span>Date Created</li>\n",field[0]);
	}

	//ModBy
	sprintf(gcQuery,"SELECT tClient.cLabel FROM tZone,tClient WHERE tZone.uModBy>0 AND tZone.uModBy=tClient.uClient AND"
			" tZone.uZone=%u",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		printf("<li class=list-group-item ><span class=badge >%s</span>Modified By</li>\n",field[0]);
	}

	//ModDate
	sprintf(gcQuery,"SELECT FROM_UNIXTIME(uModDate,'%%a %%b %%d %%T %%Y') FROM tZone WHERE uZone=%u AND uModDate>0",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		printf("<li class=list-group-item ><span class=badge >%s</span>Date Modified</li>\n",field[0]);
	}

	//Show very little to unprivilged users
	if(guPermLevel<6)
	{
		fprintf(fp,"<!-- funcZoneInfo(fp) low permissions end-->\n");
		return;
	}

	fprintf(fp,"<!-- funcZoneInfo(fp) End -->\n");

}//void funcZoneInfo(FILE *fp)


char *cGetImageHost(unsigned uZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	static char cHostname[100]={""};
	char cOrg_ImageNodeDomain[64]={""};

	sprintf(gcQuery,"SELECT cValue FROM tConfiguration WHERE uDatacenter=0"
			" AND uZone=0 AND uNode=0 AND cLabel='cOrg_ImageNodeDomain'");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		return((char *)mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sprintf(cOrg_ImageNodeDomain,"%.63s",field[0]);

	if(!cOrg_ImageNodeDomain[0])
		return(cHostname);

	sprintf(gcQuery,"SELECT tNode.cLabel FROM tZone,tNode WHERE tZone.uNode=tNode.uNode AND uZone=%u",
		uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		return((char *)mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sprintf(cHostname,"https://%.32s.%.63s",field[0],cOrg_ImageNodeDomain);

	return(cHostname);

}//char *cGetImageHost(unsigned uZone)


void funcNewZone(FILE *fp)
{

}//void funcNewZone(FILE *fp)


char *CustomerName(char *cInput)
{
	register int i;

	for(i=0;cInput[i];i++)
		if(!isalnum(cInput[i]) && cInput[i]!=' ' 
			&& cInput[i]!='.' && cInput[i]!=',' && cInput[i]!='-') break;
	cInput[i]=0;

	return(cInput);

}//char *CustomerName(char *cInput)


char *NameToLower(char *cInput)
{
	register int i;

	for(i=0;cInput[i];i++)
	{
	
		if(!isalnum(cInput[i]) && cInput[i]!='-') break;
		if(isupper(cInput[i])) cInput[i]=tolower(cInput[i]);
	}
	cInput[i]=0;

	return(cInput);

}//char *NameToLower(char *cInput)


char *CustomerEmail(char *cInput)
{
	register int i;

	for(i=0;cInput[i];i++)
	{
	
		if(!isalnum(cInput[i]) && cInput[i]!='-' && cInput[i]!='.' && cInput[i]!='@') break;
		if(isupper(cInput[i])) cInput[i]=tolower(cInput[i]);
	}
	cInput[i]=0;

	return(cInput);

}//char *CustomerEmail(char *cInput)


char *cNumbersOnly(char *cInput)
{
	register int i;

	for(i=0;cInput[i];i++)
	{
	
		if(!isdigit(cInput[i]) ) break;
	}
	cInput[i]=0;

	return(cInput);

}//char *cNumbersOnly(char *cInput)


void SetZoneStatus(unsigned uZone,unsigned uStatus)
{
	sprintf(gcQuery,"UPDATE tZone SET uStatus=%u,uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW())"
			" WHERE uZone=%u",
					uStatus,guLoginClient,uZone);
	mysql_query(&gMysql,gcQuery);

}//void SetZoneStatus(unsigned uZone,unsigned uStatus)


//functions that operate on loaded zone
void funcZone(FILE *fp)
{

}//void funcZone(FILE *fp)


void funcZoneList(FILE *fp)
{

}//void funcZoneList(FILE *fp)


void SendAlertEmail(char *cMsg)
{
	FILE *pp;
	pid_t pidChild;

	pidChild=fork();
	if(pidChild!=0)
		return;

	pp=popen("/usr/lib/sendmail -t","w");
	if(pp==NULL)
	{
		//logfileLine("SendAlertEmail","popen() /usr/lib/sendmail");
		return;
	}
			
	//should be defined in local.h
	fprintf(pp,"To: %s\n",cMAILTO);
	if(cBCC!=NULL)
	{
		char cBcc[512]={""};
		sprintf(cBcc,"%.511s",cBCC);
		if(cBcc[0])
			fprintf(pp,"Bcc: %s\n",cBcc);
	}
	fprintf(pp,"From: %s\n",cFROM);
	fprintf(pp,"Subject: %s\n",cSUBJECT);

	fprintf(pp,"%s\n",cMsg);

	fprintf(pp,".\n");

	pclose(pp);

	//logfileLine("SendAlertEmail","email attempt ok");

}//void SendAlertEmail(char *cMsg)


//functions that operate on loaded zone
void funcZoneBulk(FILE *fp)
{

}//void funcZoneBulk(FILE *fp)


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
			char cSave[1];

			/*
			if(cTextArea[uEnd+1]=='\n' || cTextArea[uEnd+1]=='\r' || cTextArea[uEnd+1]==0
				|| cTextArea[uEnd+1]==10 || cTextArea[uEnd+1]==13 )
			{
				uEnd++;
				memcpy(cSave,cTextArea+uEnd,1);
				cTextArea[uEnd]=0;
				sprintf(cRetVal,"%.511s",cTextArea+uStart);
				memcpy(cTextArea+uEnd,cSave,1);
				return(cRetVal);
			}
			*/

			if(cTextArea[uEnd]==0)
				break;

			memcpy(cSave,cTextArea+uEnd,1);
			cTextArea[uEnd]=0;
			sprintf(cRetVal,"%.511s",cTextArea+uStart);
			memcpy(cTextArea+uEnd,cSave,1);

			if(cTextArea[uEnd+1]==10)
				uEnd+=2;
			else
				uEnd++;

			//empty line call again recursively
			//or comment line anything that starts without a number
			if(cRetVal[0]=='\n' || cRetVal[0]==13 || !isdigit(cRetVal[0]) )
			{
				ParseTextAreaLines(cTextArea);
			}

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


//(C) issue: public domain code from unix.com

static int srand_called=0;

char *random_pw(char *dest)
{
    size_t len=0;
    char *p=dest;
    int three_in_a_row=0;
    int arr[128]={0x0};

	/* be sure to have called srand exactly one time */
	if(!srand_called)
	{
		srandom(time(NULL));
		srand_called=1;
	}
	*dest=0x0; /* int the destination string*/
	for(len=6 + rand()%3; len; len--, p++) /* gen characters */
	{
		char *q=dest;
		*p=(rand()%2)? rand()%26 + 97: rand()%10 + 48;
		p[1]=0x0;
		arr[(int) *p]++;                         /* check values */
		if(arr[(int) *p]==3)
		{
			for(q=dest; q[2]>0 && !three_in_a_row; q++)	
				if(*q==q[1] && q[1]==q[2])
			   		three_in_a_row=1;
		}
		if(three_in_a_row || arr[(int) *p]> 3 )
			return random_pw(dest);        /* values do not pass try again */
	}
    return dest;
}//char *random_pw(char *dest)


void GetConfigurationValue(char const *cName,char *cValue,unsigned uDatacenter,unsigned uNode,unsigned uZone)
{
        MYSQL_RES *res;
	MYSQL_ROW field;

	cValue[0]=0;
	sprintf(gcQuery,"SELECT cValue FROM tConfiguration WHERE uDatacenter=%u"
			" AND uNode=%u AND uZone=%u AND cLabel='%s'",uDatacenter,uNode,uZone,cName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		return;
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sprintf(cValue,"%.255s",field[0]);

}//void GetConfigurationValue()


//DEFINITION
//	The primary group is the first group that was assigned to
//	a zone based on the smallest uGroupGlue value.
unsigned uGetPrimaryZoneGroup(unsigned uZone)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	unsigned uGroup=0;

	//requires mysql >= 5.?
	sprintf(gcQuery,"SELECT uGroup FROM tGroupGlue WHERE uGroupGlue=(SELECT MIN(tGroupGlue.uGroupGlue) FROM tGroupGlue,tGroup WHERE"
				" tGroupGlue.uGroup=tGroup.uGroup AND tGroup.uGroupType=1 AND tGroupGlue.uZone=%u)",uZone);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
        res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		if(field[0]!=NULL)
			sscanf(field[0],"%u",&uGroup);
	}
	mysql_free_result(res);

	return(uGroup);

}//unsigned uGetPrimaryZoneGroup(unsigned uZone)


char *ToLower(char *cInput)
{
	register int i;

	for(i=0;cInput[i];i++)
		if(isupper(cInput[i])) cInput[i]=tolower(cInput[i]);
	cInput[i]=0;

	return(cInput);

}//char *ToLower(char *cInput)


unsigned uGetSessionGroup(const char *gcUser,unsigned uGroupType)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	unsigned uGroup=0;

	sprintf(gcQuery,"SELECT uGroup FROM tGroup WHERE cLabel='%s' AND uGroupType=%u",gcUser,uGroupType);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
		return(0);
        res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		if(field[0]!=NULL)
			sscanf(field[0],"%u",&uGroup);
	}
	mysql_free_result(res);

	return(uGroup);

}//unsigned uGetSessionGroup()


unsigned uGetZoneFromZoneGroup(unsigned uGroup)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	unsigned uZone=0;

	sprintf(gcQuery,"SELECT uZone FROM tGroupGlue WHERE uGroup=%u LIMIT 1",uGroup);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
        res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		if(field[0]!=NULL)
			sscanf(field[0],"%u",&uZone);
	}
	mysql_free_result(res);

	return(uZone);

}//unsigned uGetZoneFromZoneGroup(unsigned uGroup)


//set global gcSearch from tConfiguration
unsigned uGetZoneSearch(unsigned guLoginClient)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	unsigned uRetVal=1;//error

	sprintf(gcQuery,"SELECT cValue FROM tConfiguration WHERE cLabel='gcSearch' AND uOwner=%u",guLoginClient);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
        res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		if(field[0]!=NULL)
		{
			if(field[0][0])
			{
				sprintf(gcSearch,"%.99s",field[0]);
				uRetVal=0;
			}
		}
	}
	mysql_free_result(res);

	return(uRetVal);

}//unsigned uGetZoneSearch(unsigned guLoginClient)


void UpdateSessionZoneSearch(char *gcSearch)
{
	if(!gcSearch[0]) return;

	MYSQL_RES *res;
        MYSQL_ROW field;

	sprintf(gcQuery,"SELECT uConfiguration FROM tConfiguration WHERE cLabel='gcSearch' AND uOwner=%u",guLoginClient);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
        res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sprintf(gcQuery,"UPDATE tConfiguration"
				" SET cValue='%s',uModBy=%u,"
				" uModDate=UNIX_TIMESTAMP(NOW()) WHERE uConfiguration=%s",gcSearch,guLoginClient,field[0]);
        	mysql_query(&gMysql,gcQuery);
        	if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
	}
	else
	{
		sprintf(gcQuery,"INSERT INTO tConfiguration"
				" SET cValue='%s',cLabel='gcSearch',uOwner=%u,uCreatedBy=%u,"
				" uCreatedDate=UNIX_TIMESTAMP(NOW())",gcSearch,guLoginClient,guLoginClient);
        	mysql_query(&gMysql,gcQuery);
        	if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
	}

}//void UpdateSessionZoneSearch(unsigned guZone)


void UpdateSessionZone(unsigned guZone)
{
	if(!guZone) return;

	unsigned uSessionZoneGroup=0;
	//If search group exists check to see if a zone exists
	if((uSessionZoneGroup=uGetSessionGroup(gcUser,uSELECTEDZONE)))
	{
		unsigned guZoneFromGroup=0;
		guZoneFromGroup=uGetZoneFromZoneGroup(uSessionZoneGroup);
		if(guZoneFromGroup==guZone) return;
		sprintf(gcQuery,"DELETE FROM tGroupGlue WHERE uGroup=%u",uSessionZoneGroup);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			return;
	}
	//else create the initial search group
	else
	{
		sprintf(gcQuery,"INSERT INTO tGroup SET cLabel='%s',uGroupType=%u"
						",uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							gcUser,uSELECTEDZONE,guOrg,guLoginClient);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			return;
		if((uSessionZoneGroup=mysql_insert_id(&gMysql))==0)
			return;
	}
	sprintf(gcQuery,"INSERT INTO tGroupGlue SET uGroup=%u,uZone=%u",uSessionZoneGroup,guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		return;

}//void UpdateSessionZone(unsigned guZone)


void htmlResourceRecords(void)
{

	htmlHeader("unxsDNS","ZoneHeader");
	
	GetZoneFromSessionGroup();

	if(!guZone)
		exit(0);

	MYSQL_RES *res;
	MYSQL_ROW field;

	printf("<!-- htmlResourceRecords() Start -->\n");

	//Resource records
	sprintf(gcQuery,"SELECT cName,cParam1,tRRType.cLabel FROM tResource,tRRType WHERE uZone=%u AND tResource.uRRType=tRRType.uRRType",guZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		exit(0);
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		printf("<li class=list-group-item ><span class=badge >%.32s %.32s</span>%s</li>\n",field[0],field[1],field[2]);
	}

	//Show very little to unprivilged users
	if(guPermLevel<6)
	{
		printf("<!-- htmlResourceRecords() low permissions end-->\n");
		exit(0);
	}

	printf("<!-- htmlResourceRecords() End -->\n");
	exit(0);

}//void htmlResourceRecords(void)


void htmlSearchCollapseText(FILE *fp)
{
	fprintf(fp,"<!-- htmlSearchCollapse() Start -->\n");

	fprintf(fp,"<div class='list-group'>");
	fprintf(fp,"<h5>%s</h5>",gcMessage);
	fprintf(fp,"<form method=post action=unxsDNS.cgi style='margin:0px;'>");
	fprintf(fp,"<input type=hidden name=gcPage value=Zone >");
	fprintf(fp,"<input type=hidden name=guZone value=%u >",guZone);
	fprintf(fp,"<input type=hidden name=gcZone value=%s >",gcZone);
	fprintf(fp,"<input type=hidden name=gcSearch value=%s >",gcSearch);

	funcSelectZone(stdout);

	fprintf(fp,"<input type=hostname class=form-control id=searchZone"
		" title='Enter first letter(s) of container hostname, or you can use the SQL wildcard \"%%\""
		" and the single place \"_\" pattern matching chars'"
		" name=gcSearchSubmit value='%s' placeholder='Hostname search pattern' size=32 maxlength=32 onChange='submit()'>",gcSearch);
	fprintf(fp,"</form>");
	fprintf(fp,"</div>");

	fprintf(fp,"<!-- htmlSearchCollapse() End -->\n");
}//void htmlSearchCollapseText(FILE *fp)


void htmlSearchCollapse(void)
{

	htmlHeader("unxsDNS","ZoneHeader");

	GetZoneFromSessionGroup();
	uGetZoneSearch(guLoginClient);

	if(!guZone)
		exit(0);

	htmlSearchCollapseText(stdout);


	exit(0);

}//void htmlSearchCollapse(void)
