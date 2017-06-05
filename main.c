/*
FILE 
	main.c
	svn ID removed
PURPOSE
	Main cgi interface and common functions used for all the other
	table tx.c files and their schema independent txfunc.h files -until
	you mess with them in non standard ways...lol.
LEGAL
	(C) Gary Wallis and Hugo Urquiza 2001-2009 for Unixservice. All Rights Reserved.
	GPL (see fsf.org) LICENSE file should be included in distribution.
OTHER
	Only CentOS5 Linux is supported and tested by openisp.net/Unixservice LLC. 
HELP
	support @ openisp . net
	supportgrp @ unixservice . com

*/

#include "mysqlrad.h"
#include <ctype.h>
#include "language.h"
#include "local.h"

//Global vars

#define SHOWPAGE 30
MYSQL gMysql;
MYSQL gMysql2;
unsigned long gluRowid;
unsigned guStart;
unsigned guEnd;
unsigned guI;
unsigned guN=SHOWPAGE;
char gcCommand[100];
char gcFilter[100];
char gcFind[100];
unsigned guMode;
unsigned guJS=0;

int guPermLevel=0;
unsigned guLoginClient=0;
unsigned guReseller=0;
unsigned guCompany=0;
char gcUser[100]={""};
char gcHost[100]={""};
char gcHostname[100]={""};
char gcCompany[100]={""};

//SSLLoginCookie()
char gcCookie[1024]={""};
char gcLogin[100]={""};
char gcPasswd[100]={""};
unsigned guSSLCookieLogin=0;
unsigned guRequireOTPLogin=0;
unsigned guOTPExpired=0;
char gcOTP[16]={""};
char gcOTPInfo[100]={"Nothing yet"};
char gcOTPSecret[65]={""};

char gcFunction[100]={""};
unsigned guListMode=0;
char gcQuery[8192]={""};
char *gcQstr=gcQuery;
char *gcBuildInfo="GitVersion:"GitVersion;
char *gcRADStatus="";

//Local
void Footer_ism3(void);
void Header_ism3(char *cMsg, int iJs);
const char *ForeignKey(const char *cTableName, const char *cFieldName, unsigned uKey);
char *cEmailInput(char *cInput);
void GetClientOwner(unsigned uClient, unsigned *uOwner);

//Ext
int iExtMainCommands(pentry entries[], int x);
void DashBoard(const char *cOptionalMsg);
void ExtMainContent(void);
void ExtMainShell(int iArgc, char *cArgv[]);

//Only local
void ConnectDb(void);
void NoSuchFunction(void);
void iDNS(const char *cResult);
const char *cUserLevel(unsigned uPermLevel);

int iValidLogin(int iMode);
void SSLCookieLogin(void);
void SetLogin(void);
void EncryptPasswdWithSalt(char *gcPasswd, char *cSalt);
void EncryptPasswd(char *pw);
void GetPLAndClient(char *cUser);
void htmlSSLLogin(void);
void UpdateOTPExpire(unsigned uAuthorize,unsigned uClient);
char *cGetPasswd(char *gcLogin,char *cOTPSecret,unsigned long *luOTPExpire,unsigned long *luSQLNow,unsigned *uAuthorize);

//mainfunc.h for symbolic links to this program
void CalledByAlias(int iArgc,char *cArgv[]);
void MonthUsageData(unsigned uSimile);
void DayUsageData(unsigned uLogType);

pentry entries[256];
int x;

//mainfunc.h
void voidRestoreBackup(char *cFile);
void voidDeleteBackup(char *cFile);


int main(int iArgc, char *cArgv[])
{
	entry gentries[16];
	char *gcl;
	int cl=0;

	gethostname(gcHostname, 98);

	if(!strstr(cArgv[0],"iDNS.cgi"))
		CalledByAlias(iArgc,cArgv);

	if(getenv("REMOTE_HOST")!=NULL)
		sprintf(gcHost,"%.99s",getenv("REMOTE_HOST"));

	else if(getenv("REMOTE_ADDR")!=NULL)
	{
		ConnectDb();
		sprintf(gcHost,"%.99s",getenv("REMOTE_ADDR"));
	}
	else
	{
		ExtMainShell(iArgc,cArgv);
	}


	if(strcmp(getenv("REQUEST_METHOD"),"POST"))
	{

		SSLCookieLogin();

	        gcl = getenv("QUERY_STRING");

	        for(x=0;gcl[0] != '\0' && x<8;x++)
		{
               		getword(gentries[x].val,gcl,'&');
               		plustospace(gentries[x].val);
               		unescape_url(gentries[x].val);
               		getword(gentries[x].name,gentries[x].val,'=');

			//basic anti hacker
			escape_shell_cmd(gentries[x].val);

			//Local vars
			if(!strcmp(gentries[x].name,"gcFunction")) 
				sprintf(gcFunction,"%.99s",gentries[x].val);
			else if(!strcmp(gentries[x].name,"delBackup")) 
				voidDeleteBackup(gentries[x].val);
			else if(!strcmp(gentries[x].name,"resBackup")) 
				voidRestoreBackup(gentries[x].val);
		}

		if(gcFunction[0])
		{
			if(!strcmp(gcFunction,"Main"))
				iDNS("");
			else if(!strcmp(gcFunction,"Logout"))
			{
				printf("Set-Cookie: iDNSLogin=; discard; expires=\"Mon, 01-Jan-1971 00:10:10 GMT\"\n");
				printf("Set-Cookie: iDNSPasswd=; discard; expires=\"Mon, 01-Jan-1971 00:10:10 GMT\"\n");
				sprintf(gcQuery,"INSERT INTO tLog SET cLabel='logout %.99s',uLogType=6,uPermLevel=%u,"
				" uLoginClient=%u,cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=%u,uCreatedBy=1,"
				" uCreatedDate=UNIX_TIMESTAMP(NOW()) ON DUPLICATE KEY UPDATE "
				" cLabel='logout %.99s',uLogType=6,uPermLevel=%u,"
				" uLoginClient=%u,cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=%u,uCreatedBy=1,"
				" uCreatedDate=UNIX_TIMESTAMP(NOW())",
					gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname,guCompany,
					gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname,guCompany);
				MYSQL_RUN;
				if(gcOTPSecret[0])
				{
					UpdateOTPExpire(0,guLoginClient);
					guRequireOTPLogin=1;
				}
				gcCookie[0]=0;
                                guPermLevel=0;
                                guLoginClient=0;
                                gcUser[0]=0;
                                gcCompany[0]=0;
                                guSSLCookieLogin=0;
                                htmlSSLLogin();
			}

			else if(!strcmp(gcFunction,"tZone"))
				ExttZoneGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tResource"))
				ExttResourceGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tRRType"))
				ExttRRTypeGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tJob"))
				ExttJobGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tMailServer"))
				ExttMailServerGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tNSType"))
				ExttNSTypeGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tNSSet"))
				ExttNSSetGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tNS"))
				ExttNSGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tServer"))
				ExttServerGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tConfiguration"))
				ExttConfigurationGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tTemplate"))
				ExttTemplateGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tTemplateSet"))
				ExttTemplateSetGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tTemplateType"))
				ExttTemplateTypeGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tLog"))
				ExttLogGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tLogType"))
				ExttLogTypeGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tBlock"))
				ExttBlockGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tView"))
				ExttViewGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tRegistrar"))
				ExttRegistrarGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tGlossary"))
				ExttGlossaryGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tGroup"))
				ExttGroupGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tGroupGlue"))
				ExttGroupGlueGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tGroupType"))
				ExttGroupTypeGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tZoneImport"))
				ExttZoneImportGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tResourceImport"))
				ExttResourceImportGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tMonth"))
				ExttMonthGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tMonthHit"))
				ExttMonthHitGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tLogMonth"))
				ExttLogMonthGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tHit"))
				ExttHitGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tHitMonth"))
				ExttHitMonthGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tDeletedZone"))
				ExttDeletedZoneGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tDeletedResource"))
				ExttDeletedResourceGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tClient"))
				ExttClientGetHook(gentries,x);
			else if(!strcmp(gcFunction,"tAuthorize"))
				ExttAuthorizeGetHook(gentries,x);
			else if(!strcmp(gcFunction,"SimileMonthUsage"))
			{
				MonthUsageData(2);
				exit(0);
			}
			else if(!strcmp(gcFunction,"Dashboard"))
				iDNS("DashBoard");

		}

		iDNS("");

	}//end get method interface section

	//Post method interface
	cl = atoi(getenv("CONTENT_LENGTH"));
	for(x=0;cl && (!feof(stdin)) && x<256 ;x++)
	{
		entries[x].val = fmakeword(stdin,'&',&cl);
		plustospace(entries[x].val);
		unescape_url(entries[x].val);
		entries[x].name = makeword(entries[x].val,'=');

		//basic anti hacker
		//Allow posted page cursors. See PageMachine()
		//escape_shell_cmd(entries[x].val);

		if(!strcmp(entries[x].name,"gcFunction")) 
			sprintf(gcFunction,"%.99s",entries[x].val);
		else if(!strcmp(entries[x].name,"guListMode")) 
			sscanf(entries[x].val,"%u",&guListMode);
		else if(!strcmp(entries[x].name,"username"))
			sprintf(gcLogin,"%.99s",entries[x].val);
                else if(!strcmp(entries[x].name,"password"))
			sprintf(gcPasswd,"%.99s",entries[x].val);
                else if(!strcmp(entries[x].name,"gcOTP"))
			sprintf(gcOTP,"%.15s",entries[x].val);
	}

	//SSLCookieLogin()
        if(!strcmp(gcFunction,"Login")) SetLogin();

        if(!guPermLevel || !gcUser[0] || !guLoginClient)
                SSLCookieLogin();

	//Main Post Menu
	tZoneCommands(entries,x);
	tResourceCommands(entries,x);
	tRRTypeCommands(entries,x);
	tJobCommands(entries,x);
	tMailServerCommands(entries,x);
	tNSTypeCommands(entries,x);
	tNSSetCommands(entries,x);
	tNSCommands(entries,x);
	tServerCommands(entries,x);
	tConfigurationCommands(entries,x);
	tTemplateCommands(entries,x);
	tTemplateSetCommands(entries,x);
	tTemplateTypeCommands(entries,x);
	tLogCommands(entries,x);
	tLogTypeCommands(entries,x);
	tBlockCommands(entries,x);
	tViewCommands(entries,x);
	tRegistrarCommands(entries,x);
	tGlossaryCommands(entries,x);
	tGroupCommands(entries,x);
	tGroupGlueCommands(entries,x);
	tGroupTypeCommands(entries,x);
	tZoneImportCommands(entries,x);
	tResourceImportCommands(entries,x);
	tMonthCommands(entries,x);
	tMonthHitCommands(entries,x);
	tLogMonthCommands(entries,x);
	tHitCommands(entries,x);
	tHitMonthCommands(entries,x);
	tDeletedZoneCommands(entries,x);
	tDeletedResourceCommands(entries,x);
	tClientCommands(entries,x);
	tAuthorizeCommands(entries,x);
	iExtMainCommands(entries,x);

	NoSuchFunction();

	return(0);

}//end of main()

static char *cGitVersion="GitVersion:"GitVersion;
#include "mainfunc.h"

void iDNS(const char *cResult)
{
        if(cResult[0])
	{
        	if(!strncmp(cResult,"Database server",14))
		{
			Header_ism3("Main",guJS);
			printf("%s",cResult);
		}
		else
		{
			char cuDashboardType[256]={""};

			GetConfiguration("uDashboardType",cuDashboardType,0);
			if(cuDashboardType[0]) sscanf(cuDashboardType,"%u",&guJS);
			//This guJS is a test hack for now
			Header_ism3("Dashboard",guJS);
			DashBoard(cResult);
		}
	}
        else
	{
		Header_ism3("Main",guJS);
                ExtMainContent();
	}

	Footer_ism3();

}//void iDNS(const char *cResult)


void htmlStyleSheet(void)
{

	printf("<!-- StyleSheet() -->\n");
	printf("<link rel=stylesheet type=text/css href=/css/radBackend.css />\n");


}//void htmlStyleSheet(void)


void jsCalendarHeader(void)
{
        printf("<link rel='stylesheet' type='text/css' media='all' href='/js/calendar-blue.css'/>\n");
        printf("<script type='text/javascript' src='/js/calendar.js'></script>\n");
        printf("<script type='text/javascript' src='/js/calendar-en.js'></script>\n");
        printf("<script type='text/javascript' src='/js/calendar-setup.js'></script>\n");

}//void jsCalendarHeader(void)


void jsCalendarInput(char *cInputName,char *cValue,unsigned uMode)
{
        char cMode[16]={""};
        if(!uMode)
                sprintf(cMode,"disabled");

        printf("<input id='%s' class='field_input' type='text' name='%s' value='%s' size=40 style='display: ; \
                vertical-align: middle; ' %s >\n",cInputName,cInputName,cValue,cMode);

        if(uMode)
        {
		printf("<img date_trigger='1' class='record_button' date_field='%s' id='date_trigger_%s_501'"
			"src='/images/calendar.gif' onmouseout='swapClass(event); this.src='/images/calendar.gif'"
			"' onmouseover='swapClass(event); this.src='/images/calendar_mo.gif'"
			"onmousedown='this.style.top = '1px'; this.style.left = '1px''"
			"onmouseup='this.style.top = '0px'; this.style.left = '0px''"
                        "style='position: relative; vertical-align: middle; display: ;"
			"' title='Date selector'/>\n",cInputName,cInputName);

                printf("<script type='text/javascript'>\n Calendar.setup({\n "
                        "inputField     :    '%s',\n "
                        "ifFormat : '%%Y-%%m-%%d',\n "
                        "button         :    'date_trigger_%s_501',\n "
                        "align          :    'bR',\n "
                        "singleClick    :    true,\n "
                        "weekNumbers    :    false,\n "
                        "step           :    1,\n "
                        "timeFormat : 12\n });</script>\n",cInputName,cInputName);
        }
        else
                printf("<input type=hidden name='%s' value='%s'>\n",cInputName,cValue);

}//void jsCalendarInput(char *cInputName,char *cValue,unsigned uMode)


void jsToggleCheckboxes(void)
{
        printf("<script>"
		"function checkAll(checkname, toggle)"
		"{"
		"	for (i = 0; i < checkname.length; i++)"
		"	if( checkname[i].name.indexOf(\"NoCA\")==(-1) )"
		"	{"
		"		checkname[i].checked = toggle.checked? true:false"
		"	}"
		"}"
		"</script>");
}//void jsToggleCheckboxes(void)


void Header_ism3(char *title, int iJs)
{
	printf("Content-type: text/html\n\n");
	printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\""
			" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	printf("<link rel=\"shortcut icon\" type=image/x-icon href=/images/dns.ico?v=4>\n");
        printf("<html><head><title>unxsBind %s %s </title>",gcHostname,title);
	printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n");
	htmlStyleSheet();
        if(iJs==1)
                jsCalendarHeader();
        else if(iJs==2)
		jsToggleCheckboxes();

	printf("</head><body><form name=formMain action=iDNS.cgi method=post autocomplete=on><blockquote>\n");
	//printf("<img src=/images/unxsbind.jpg>&nbsp;&nbsp;\n");

	//ModuleRAD3NavBars()
	if(!strcmp(gcFunction,"tZone") || !strcmp(gcFunction,"tZoneTools") ||
			!strcmp(gcFunction,"tZoneList"))
		ExttZoneNavBar();
	else if(!strcmp(gcFunction,"tResource") || !strcmp(gcFunction,"tResourceTools") ||
			!strcmp(gcFunction,"tResourceList"))
		ExttResourceNavBar();
	else if(!strcmp(gcFunction,"tRRType") || !strcmp(gcFunction,"tRRTypeTools") ||
			!strcmp(gcFunction,"tRRTypeList"))
		ExttRRTypeNavBar();
	else if(!strcmp(gcFunction,"tJob") || !strcmp(gcFunction,"tJobTools") ||
			!strcmp(gcFunction,"tJobList"))
		ExttJobNavBar();
	else if(!strcmp(gcFunction,"tMailServer") || !strcmp(gcFunction,"tMailServerTools") ||
			!strcmp(gcFunction,"tMailServerList"))
		ExttMailServerNavBar();
	else if(!strcmp(gcFunction,"tNSType") || !strcmp(gcFunction,"tNSTypeTools") ||
			!strcmp(gcFunction,"tNSTypeList"))
		ExttNSTypeNavBar();
	else if(!strcmp(gcFunction,"tNSSet") || !strcmp(gcFunction,"tNSSetTools") ||
			!strcmp(gcFunction,"tNSSetList"))
		ExttNSSetNavBar();
	else if(!strcmp(gcFunction,"tNS") || !strcmp(gcFunction,"tNSTools") ||
			!strcmp(gcFunction,"tNSList"))
		ExttNSNavBar();
	else if(!strcmp(gcFunction,"tServer") || !strcmp(gcFunction,"tServerTools") ||
			!strcmp(gcFunction,"tServerList"))
		ExttServerNavBar();
	else if(!strcmp(gcFunction,"tConfiguration") || !strcmp(gcFunction,"tConfigurationTools") ||
			!strcmp(gcFunction,"tConfigurationList"))
		ExttConfigurationNavBar();
	else if(!strcmp(gcFunction,"tTemplate") || !strcmp(gcFunction,"tTemplateTools") ||
			!strcmp(gcFunction,"tTemplateList"))
		ExttTemplateNavBar();
	else if(!strcmp(gcFunction,"tTemplateSet") || !strcmp(gcFunction,"tTemplateSetTools") ||
			!strcmp(gcFunction,"tTemplateSetList"))
		ExttTemplateSetNavBar();
	else if(!strcmp(gcFunction,"tTemplateType") || !strcmp(gcFunction,"tTemplateTypeTools") ||
			!strcmp(gcFunction,"tTemplateTypeList"))
		ExttTemplateTypeNavBar();
	else if(!strcmp(gcFunction,"tLog") || !strcmp(gcFunction,"tLogTools") ||
			!strcmp(gcFunction,"tLogList"))
		ExttLogNavBar();
	else if(!strcmp(gcFunction,"tLogType") || !strcmp(gcFunction,"tLogTypeTools") ||
			!strcmp(gcFunction,"tLogTypeList"))
		ExttLogTypeNavBar();
	else if(!strcmp(gcFunction,"tBlock") || !strcmp(gcFunction,"tBlockTools") ||
			!strcmp(gcFunction,"tBlockList"))
		ExttBlockNavBar();
	else if(!strcmp(gcFunction,"tView") || !strcmp(gcFunction,"tViewTools") ||
			!strcmp(gcFunction,"tViewList"))
		ExttViewNavBar();
	else if(!strcmp(gcFunction,"tRegistrar") || !strcmp(gcFunction,"tRegistrarTools") ||
			!strcmp(gcFunction,"tRegistrarList"))
		ExttRegistrarNavBar();
	else if(!strcmp(gcFunction,"tGlossary") || !strcmp(gcFunction,"tGlossaryTools") ||
			!strcmp(gcFunction,"tGlossaryList"))
		ExttGlossaryNavBar();
	else if(!strcmp(gcFunction,"tGroup") || !strcmp(gcFunction,"tGroupTools") ||
			!strcmp(gcFunction,"tGroupList"))
		ExttGroupNavBar();
	else if(!strcmp(gcFunction,"tGroupGlue") || !strcmp(gcFunction,"tGroupGlueTools") ||
			!strcmp(gcFunction,"tGroupGlueList"))
		ExttGroupGlueNavBar();
	else if(!strcmp(gcFunction,"tGroupType") || !strcmp(gcFunction,"tGroupTypeTools") ||
			!strcmp(gcFunction,"tGroupTypeList"))
		ExttGroupTypeNavBar();
	else if(!strcmp(gcFunction,"tZoneImport") || !strcmp(gcFunction,"tZoneImportTools") ||
			!strcmp(gcFunction,"tZoneImportList"))
		ExttZoneImportNavBar();
	else if(!strcmp(gcFunction,"tResourceImport") || !strcmp(gcFunction,"tResourceImportTools") ||
			!strcmp(gcFunction,"tResourceImportList"))
		ExttResourceImportNavBar();
	else if(!strcmp(gcFunction,"tMonth") || !strcmp(gcFunction,"tMonthTools") ||
			!strcmp(gcFunction,"tMonthList"))
		ExttMonthNavBar();
	else if(!strcmp(gcFunction,"tMonthHit") || !strcmp(gcFunction,"tMonthHitTools") ||
			!strcmp(gcFunction,"tMonthHitList"))
		ExttMonthHitNavBar();
	else if(!strcmp(gcFunction,"tLogMonth") || !strcmp(gcFunction,"tLogMonthTools") ||
			!strcmp(gcFunction,"tLogMonthList"))
		ExttLogMonthNavBar();
	else if(!strcmp(gcFunction,"tHit") || !strcmp(gcFunction,"tHitTools") ||
			!strcmp(gcFunction,"tHitList"))
		ExttHitNavBar();
	else if(!strcmp(gcFunction,"tHitMonth") || !strcmp(gcFunction,"tHitMonthTools") ||
			!strcmp(gcFunction,"tHitMonthList"))
		ExttHitMonthNavBar();
	else if(!strcmp(gcFunction,"tDeletedZone") || !strcmp(gcFunction,"tDeletedZoneTools") ||
			!strcmp(gcFunction,"tDeletedZoneList"))
		ExttDeletedZoneNavBar();
	else if(!strcmp(gcFunction,"tDeletedResource") || !strcmp(gcFunction,"tDeletedResourceTools") ||
			!strcmp(gcFunction,"tDeletedResourceList"))
		ExttDeletedResourceNavBar();
	else if(!strcmp(gcFunction,"tClient") || !strcmp(gcFunction,"tClientTools") ||
			!strcmp(gcFunction,"tClientList"))
		ExttClientNavBar();
	else if(!strcmp(gcFunction,"tAuthorize") || !strcmp(gcFunction,"tAuthorizeTools") ||
			!strcmp(gcFunction,"tAuthorizeList"))
		ExttAuthorizeNavBar();

	//Login info
	printf("<font size=3><b>unxsBind</b></font> \n ");
	if(!guPermLevel)
	{
		printf("&nbsp;&nbsp;&nbsp;<font color=red>Your IP address %s has been logged</font>",gcHost);
		//printf("&nbsp;&nbsp;&nbsp;<font color=red>%s ",gcUser);
		//if(strcmp(gcUser,gcCompany)) printf("(%s) ",gcCompany);
		//printf("logged in from %s [%s/%u (%s)]</font>",gcHost,cUserLevel(guPermLevel),guReseller,gcOTPInfo);
	}
	else
	{
		printf("&nbsp;&nbsp;&nbsp;<font color=red>%s ",gcUser);
		if(strcmp(gcUser,gcCompany)) printf("(%s) ",gcCompany);
		printf("logged in from %s [%s]</font>",gcHost,cUserLevel(guPermLevel));
		//printf("logged in from %s [%s/%u (%s)]</font>",gcHost,cUserLevel(guPermLevel),guReseller,gcOTPInfo);
	}

	//Logout link
	if(guSSLCookieLogin)
	{
		printf(" <a title='Erase login cookies' href=?gcFunction=Logout>Logout</a> ");
		printf(" <a title='Quick dashboard link' href=?gcFunction=Dashboard>Dashboard</a> ");
	}

	//Generate Menu Items
	printf("\n<!-- tab menu -->\n");
	printf("<div id=menuholder>\n");
	printf("\t<div id=menutab>\n");

	printf("\t\t<div id=topline>\n");
	printf("\t\t\t<ol>\n");


	//Main always exists for RAD
	printf("\t\t\t<li");
	if(strncmp(gcFunction,"Main",4))
		printf(">\n");
	else
		printf(" id=current>\n");
		printf("\t\t\t<a title='Home start page' href=?gcFunction=Main>Main</a>\n");
	printf("\t\t\t</li>\n");

	if(guSSLCookieLogin)
	{

		//tZone
		if(guPermLevel>=7)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tZone") && strcmp(gcFunction,"tZoneTools") &&
				strcmp(gcFunction,"tZoneList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='DNS Zones' href=?gcFunction=tZone>tZone</a>\n");
		}
		//tResource
		if(guPermLevel>=20 
			|| strcmp(gcFunction,"tResource")==0 || strcmp(gcFunction,"tResourceTools")==0 || strcmp(gcFunction,"tResourceList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tResource") && strcmp(gcFunction,"tResourceTools") &&
				strcmp(gcFunction,"tResourceList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Resource Records for a given Zone' href=?gcFunction=tResource>tResource</a>\n");
		}
		//tRRType
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tRRType")==0 || strcmp(gcFunction,"tRRTypeList")==0 || strcmp(gcFunction,"tRRTypeTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tRRType") && strcmp(gcFunction,"tRRTypeTools") &&
				strcmp(gcFunction,"tRRTypeList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Resource Record Types' href=?gcFunction=tRRType>tRRType</a>\n");
		}
		//tMailServer
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tMailServer")==0 || strcmp(gcFunction,"tMailServerList")==0 || strcmp(gcFunction,"tMailServerTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tMailServer") && strcmp(gcFunction,"tMailServerTools") &&
				strcmp(gcFunction,"tMailServerList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Mail server groups' href=?gcFunction=tMailServer>tMailServer</a>\n");
		}
		//tNSType
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tNSType")==0 || strcmp(gcFunction,"tNSTypeList")==0 || strcmp(gcFunction,"tNSTypeTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tNSType") && strcmp(gcFunction,"tNSTypeTools") &&
				strcmp(gcFunction,"tNSTypeList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Type of name server' href=?gcFunction=tNSType>tNSType</a>\n");
		}
		//tNSSet
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tNSSet")==0 || strcmp(gcFunction,"tNSSetList")==0 || strcmp(gcFunction,"tNSSetTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tNSSet") && strcmp(gcFunction,"tNSSetTools") &&
				strcmp(gcFunction,"tNSSetList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='For grouping related NSs to a zone' href=?gcFunction=tNSSet>tNSSet</a>\n");
		}
		//tNS
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tNS")==0 || strcmp(gcFunction,"tNSList")==0 || strcmp(gcFunction,"tNSTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tNS") && strcmp(gcFunction,"tNSTools") &&
				strcmp(gcFunction,"tNSList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Individual NS set members' href=?gcFunction=tNS>tNS</a>\n");
		}
		//tServer
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tServer")==0 || strcmp(gcFunction,"tServerList")==0 || strcmp(gcFunction,"tServerTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tServer") && strcmp(gcFunction,"tServerTools") &&
				strcmp(gcFunction,"tServerList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='For grouping NS sets to a server' href=?gcFunction=tServer>tServer</a>\n");
		}
		//tConfiguration
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tConfiguration")==0 || strcmp(gcFunction,"tConfigurationList")==0 
			|| strcmp(gcFunction,"tConfigurationTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tConfiguration") && strcmp(gcFunction,"tConfigurationTools") &&
				strcmp(gcFunction,"tConfigurationList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Runtime configuration variables' href=?gcFunction=tConfiguration>tConfiguration</a>\n");
		}
		//tTemplate
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tTemplate")==0 || strcmp(gcFunction,"tTemplateList")==0 || strcmp(gcFunction,"tTemplateTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tTemplate") && strcmp(gcFunction,"tTemplateTools") &&
				strcmp(gcFunction,"tTemplateList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='tTemplate' href=?gcFunction=tTemplate>tTemplate</a>\n");
		}
		//tTemplateSet
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tTemplateSet")==0 || strcmp(gcFunction,"tTemplateSetList")==0 || strcmp(gcFunction,"tTemplateSetTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tTemplateSet") && strcmp(gcFunction,"tTemplateSetTools") &&
				strcmp(gcFunction,"tTemplateSetList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='tTemplateSet' href=?gcFunction=tTemplateSet>tTemplateSet</a>\n");
		}
		//tTemplateType
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tTemplateType")==0 || strcmp(gcFunction,"tTemplateTypeList")==0 || strcmp(gcFunction,"tTemplateTypeTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tTemplateType") && strcmp(gcFunction,"tTemplateTypeTools") &&
				strcmp(gcFunction,"tTemplateTypeList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='tTemplateType' href=?gcFunction=tTemplateType>tTemplateType</a>\n");
		}
		//tLog
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tLog")==0 || strcmp(gcFunction,"tLogList")==0 || strcmp(gcFunction,"tLogTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tLog") && strcmp(gcFunction,"tLogTools") &&
				strcmp(gcFunction,"tLogList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Audit Log' href=?gcFunction=tLog>tLog</a>\n");
		}
		//tLogType
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tLogType")==0 || strcmp(gcFunction,"tLogTypeList")==0 || strcmp(gcFunction,"tLogTypeTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tLogType") && strcmp(gcFunction,"tLogTypeTools") &&
				strcmp(gcFunction,"tLogTypeList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Audit Log Type' href=?gcFunction=tLogType>tLogType</a>\n");
		}
		//tBlock
		if(guPermLevel>=7)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tBlock") && strcmp(gcFunction,"tBlockTools") &&
				strcmp(gcFunction,"tBlockList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='CIDR IP Block Control' href=?gcFunction=tBlock>tBlock</a>\n");
		}
		//tView
		if(guPermLevel>=10)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tView") && strcmp(gcFunction,"tViewTools") &&
				strcmp(gcFunction,"tViewList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='View details for tZone.uView' href=?gcFunction=tView>tView</a>\n");
		}
		//tRegistrar
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tRegistrar")==0 || strcmp(gcFunction,"tRegistrarList")==0 || strcmp(gcFunction,"tRegistrarTools")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tRegistrar") && strcmp(gcFunction,"tRegistrarTools") &&
				strcmp(gcFunction,"tRegistrarList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='tRegistrar' href=?gcFunction=tRegistrar>tRegistrar</a>\n");
		}
		//tGlossary
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tGlossary")==0 || strcmp(gcFunction,"tGlossaryTools")==0 || strcmp(gcFunction,"tGlossaryList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tGlossary") && strcmp(gcFunction,"tGlossaryTools") &&
				strcmp(gcFunction,"tGlossaryList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Stores the Glossary definitions' href=?gcFunction=tGlossary>tGlossary</a>\n");
		}
		//tGroup
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tGroup")==0 || strcmp(gcFunction,"tGroupTools")==0 || strcmp(gcFunction,"tGroupList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tGroup") && strcmp(gcFunction,"tGroupTools") &&
				strcmp(gcFunction,"tGroupList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='tGroup' href=?gcFunction=tGroup>tGroup</a>\n");
		}
		//tGroupGlue
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tGroupGlue")==0 || strcmp(gcFunction,"tGroupGlueTools")==0 || strcmp(gcFunction,"tGroupGlueList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tGroupGlue") && strcmp(gcFunction,"tGroupGlueTools") &&
				strcmp(gcFunction,"tGroupGlueList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='tGroupGlue' href=?gcFunction=tGroupGlue>tGroupGlue</a>\n");
		}
		//tGroupType
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tGroupType")==0 || strcmp(gcFunction,"tGroupTypeTools")==0 || strcmp(gcFunction,"tGroupTypeList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tGroupType") && strcmp(gcFunction,"tGroupTypeTools") &&
				strcmp(gcFunction,"tGroupTypeList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='tGroupType' href=?gcFunction=tGroupType>tGroupType</a>\n");
		}
		//tZoneImport
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tZoneImport")==0 || strcmp(gcFunction,"tZoneImportTools")==0 || strcmp(gcFunction,"tZoneImportList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tZoneImport") && strcmp(gcFunction,"tZoneImportTools") &&
				strcmp(gcFunction,"tZoneImportList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='DNS Imported Zones' href=?gcFunction=tZoneImport>tZoneImport</a>\n");
		}
		//tResourceImport
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tResourceImport")==0 || strcmp(gcFunction,"tResourceImportTools")==0 
			|| strcmp(gcFunction,"tResourceImportList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tResourceImport") && strcmp(gcFunction,"tResourceImportTools") &&
				strcmp(gcFunction,"tResourceImportList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Resource Records for a given import zone' href=?gcFunction=tResourceImport>tResourceImport</a>\n");
		}
		//tMonth
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tMonth")==0 || strcmp(gcFunction,"tMonthTools")==0 || strcmp(gcFunction,"tMonthList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tMonth") && strcmp(gcFunction,"tMonthTools") &&
				strcmp(gcFunction,"tMonthList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='tMonth' href=?gcFunction=tMonth>tMonth</a>\n");
		}
		//tMonthHit
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tMonthHit")==0 || strcmp(gcFunction,"tMonthHitTools")==0 || strcmp(gcFunction,"tMonthHitList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tMonthHit") && strcmp(gcFunction,"tMonthHitTools") &&
				strcmp(gcFunction,"tMonthHitList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='tMonthHit' href=?gcFunction=tMonthHit>tMonthHit</a>\n");
		}
		//tLogMonth
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tLogMonth")==0 || strcmp(gcFunction,"tLogMonthTools")==0 || strcmp(gcFunction,"tLogMonthList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tLogMonth") && strcmp(gcFunction,"tLogMonthTools") &&
				strcmp(gcFunction,"tLogMonthList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Archived Audit Log' href=?gcFunction=tLogMonth>tLogMonth</a>\n");
		}
		//tHit
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tHit")==0 || strcmp(gcFunction,"tHitTools")==0 || strcmp(gcFunction,"tHitList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tHit") && strcmp(gcFunction,"tHitTools") &&
				strcmp(gcFunction,"tHitList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='tHit' href=?gcFunction=tHit>tHit</a>\n");
		}
		//tHitMonth
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tHitMonth")==0 || strcmp(gcFunction,"tHitMonthTools")==0 || strcmp(gcFunction,"tHitMonthList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tHitMonth") && strcmp(gcFunction,"tHitMonthTools") &&
				strcmp(gcFunction,"tHitMonthList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='tHitMonth' href=?gcFunction=tHitMonth>tHitMonth</a>\n");
		}
		//tDeletedZone
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tDeletedZone")==0 || strcmp(gcFunction,"tDeletedZoneTools")==0 || strcmp(gcFunction,"tDeletedZoneList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tDeletedZone") && strcmp(gcFunction,"tDeletedZoneTools") &&
				strcmp(gcFunction,"tDeletedZoneList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Deleted DNS Zones' href=?gcFunction=tDeletedZone>tDeletedZone</a>\n");
		}
		//tDeletedResource
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tDeletedResource")==0 || strcmp(gcFunction,"tDeletedResourceTools")==0 
			|| strcmp(gcFunction,"tDeletedResourceList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tDeletedResource") && strcmp(gcFunction,"tDeletedResourceTools") &&
				strcmp(gcFunction,"tDeletedResourceList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Deleted Resource Records for a given Zone' href=?gcFunction=tDeletedResource>tDeletedResource</a>\n");
		}
		//tClient
		if(guPermLevel>=7)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tClient") && strcmp(gcFunction,"tClientTools") &&
				strcmp(gcFunction,"tClientList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Clients' href=?gcFunction=tClient>tClient</a>\n");
		}
		//tAuthorize
		if(guPermLevel>=20
			|| strcmp(gcFunction,"tAuthorize")==0 || strcmp(gcFunction,"tAuthorizeTools")==0 || strcmp(gcFunction,"tAuthorizeList")==0)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tAuthorize") && strcmp(gcFunction,"tAuthorizeTools") &&
				strcmp(gcFunction,"tAuthorizeList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Login Authorization' href=?gcFunction=tAuthorize>tAuthorize</a>\n");
		}
		//tJob
		if(guPermLevel>=10)
		{
		  printf("\t\t\t<li");
		  if(strcmp(gcFunction,"tJob") && strcmp(gcFunction,"tJobTools") &&
				strcmp(gcFunction,"tJobList"))
			  printf(">\n");
		  else
			  printf(" id=current>\n");
		  printf("\t\t\t<a title='Job Queue' href=?gcFunction=tJob>tJob</a>\n");
		}
	
	}//if logged in
	
	printf("\t\t\t</ol>\n");

	printf("\t\t\t<br class=clearall >\n");
	printf("\t\t</div>\n");
	printf("\t</div>\n");
	printf("</div>\n");

}//Header_ism3(char *title, int js)


void Footer_ism3(void)
{
	printf("</blockquote></form>");

	exit(0);

}//Footer_ism3(void)


void NoSuchFunction(void)
{
	 
	sprintf(gcQuery,"[%s] Not Recognized",gcFunction);
	htmlPlainTextError(gcQuery);
}

void ProcessControlVars(pentry entries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"gcFilter"))
			sprintf(gcFilter,entries[i].val);
		else if(!strcmp(entries[i].name,"gcCommand"))
			sprintf(gcCommand,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"gcFind"))
			sprintf(gcFind,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"gluRowid"))
			sscanf(entries[i].val,"%lu",&gluRowid);
	}
}

//guMode=0 page guMode, guMode=1 list auto header mode
void PageMachine(char *cFuncName, int iLmode, char *cMsg)
{
	
if(iLmode)
{
	//List mode
	
	if(gluRowid<1) gluRowid=1;
	if(gluRowid>(guN=((guI/SHOWPAGE)+1))) gluRowid=guN;

	if(!strcmp(gcFind," >"))
	{
		//If NOT on last page show next page
		if( gluRowid >= guN-1 )
		{
			//If on last page adjust end
			guStart= ((guI/SHOWPAGE)*SHOWPAGE) + 1;
			guEnd=guI;
		}
		else
		{
			guStart=((gluRowid)*SHOWPAGE)+1;
			guEnd=guStart+SHOWPAGE-1;
			gluRowid++;
		}
	}
	else if(!strcmp(gcFind,"< "))
	{
		if(gluRowid>1 )
		{
			gluRowid--;
			guStart=(gluRowid)*SHOWPAGE-SHOWPAGE+1;
			guEnd=guStart+SHOWPAGE-1;
		}
		else
		{
			guStart=1;
			if(guI > SHOWPAGE)
			{
				guEnd=SHOWPAGE;
			}
			else
			{
				guEnd=guI;
			}
			gluRowid=1;
		}
	}
	else if(!strcmp(gcFind,">>"))
	{
		guStart= ((guI/SHOWPAGE)*SHOWPAGE) + 1;
		guEnd=guI;
		gluRowid=guN;
	}
	else if(1)
	{
		guStart=1;
		if(guI > SHOWPAGE)
		{
			guEnd=SHOWPAGE;
		}
		else
		{
			guEnd=guI;
		}
		gluRowid=1;
	}

	guListMode=1;
	Header_ism3(cFuncName,0);

	if(!guI) 
        {
                printf(LANG_PAGEMACHINE_HINT);

        }
 

        printf(LANG_PAGEMACHINE_SHOWING,1+(guStart/SHOWPAGE),guN,guStart,guEnd,guI);


	printf("<input type=hidden name=gluRowid value=%lu>",gluRowid);
	printf("<input type=hidden name=gcFunction value=%s >",cFuncName);
	printf("<input type=hidden name=guListMode value=1 >\n");
}
else
{
	//Page mode
	//on entry guI has number of rows
	//on entry gluRowid has current position unless guI=1
	//if guI=1 then we need to figure out real guI

	guN=guI;

	if(gluRowid<1)
	{
		gluRowid=1;
		return;
	}
	
	if(!strcmp(gcFind," >"))
	{
		//If on last page stay there
		if( gluRowid >= guI )
		{
			//If on last page adjust guEnd
			gluRowid=guI;
		}
		else
		{
			gluRowid++;
		}
	}
	else if(!strcmp(gcFind,"< "))
	{
		if(gluRowid>1 )
		{
			gluRowid--;
		}
		else
		{
			gluRowid=1;
		}
	}
	else if(!strcmp(gcFind,">>"))
	{
		gluRowid=guI;
	}
	else if(!strcmp(gcFind,"<<"))
	{
		gluRowid=1;
	}
	else if(1)
	{
		//If on last page stay there
		if( gluRowid >= guI )
		{
			gluRowid=guI;
		}
	}

}//guEnd iLmode

}//PageMachine()


void OpenFieldSet(char *cLabel, unsigned uWidth)
{
	printf("<fieldset><legend><b>%s</b></legend><table width=%u%%>\n",cLabel,uWidth);

}//void OpenFieldSet()


void CloseFieldSet(void)
{
	printf("</table></fieldset>\n");

}//void CloseFieldSet(void)


void OpenRow(const char *cFieldLabel, const char *cColor)
{
	printf("<tr><td width=20%% valign=top><a href=?gcFunction=tGlossary&cSearch=%s"
		" class=darkLink><font color=%.32s>%.32s</a></td><td>",cFieldLabel,cColor,cFieldLabel);

}//void OpenRow()


void tTablePullDownOwner(const char *cTableName, const char *cFieldName,
                        const char *cOrderby, unsigned uSelector, unsigned uMode)
{
        register int i,n;
        char cLabel[128];
        MYSQL_RES *mysqlRes;         
        MYSQL_ROW mysqlField;

        char cSelectName[34]={""};
	char cHidden[100]={""};
        char cLocalTableName[256]={""};
        char *cp;
	char *cMode="";

	if(!uMode)
		cMode="disabled";
      
        if(!cTableName[0] || !cFieldName[0] || !cOrderby[0])
        {
                printf("Invalid input tTablePullDown()");
                return;
        }

        //Extended functionality
        strncpy(cLocalTableName,cTableName,255);
        if((cp=strchr(cLocalTableName,';')))
        {
                strncpy(cSelectName,cp+1,32);
                cSelectName[32]=0;
                *cp=0;
        }


	if(guLoginClient==1)
        	sprintf(gcQuery,"SELECT _rowid,%s FROM %s ORDER BY %s",
                                cFieldName,cLocalTableName,cOrderby);
	else
        	sprintf(gcQuery,"SELECT _rowid,%s FROM %s WHERE uOwner=%u OR uOwner IN"
				" (SELECT uClient FROM " TCLIENT " WHERE uOwner=%u) ORDER BY %s",
				cFieldName,cLocalTableName,guCompany,guCompany,cOrderby);

	macro_mySQLRunAndStoreTextVoidRet(mysqlRes);
	i=mysql_num_rows(mysqlRes);

	if(cSelectName[0])
                sprintf(cLabel,"%s",cSelectName);
        else
                sprintf(cLabel,"%s_%sPullDown",cLocalTableName,cFieldName);

        if(i>0)
        {
                printf("<select name=%s %s>\n",cLabel,cMode);

                //Default no selection
                printf("<option title='No selection'>---</option>\n");

                for(n=0;n<i;n++)
                {
                        int unsigned field0=0;

                        mysqlField=mysql_fetch_row(mysqlRes);
                        sscanf(mysqlField[0],"%u",&field0);

                        if(uSelector != field0)
                        {
                             printf("<option>%s</option>\n",mysqlField[1]);
                        }
                        else
                        {
                             printf("<option selected>%s</option>\n",mysqlField[1]);
			     if(!uMode)
			     sprintf(cHidden,"<input type=hidden name=%.32s value='%.32s'>\n",
			     		cLabel,mysqlField[1]);
                        }
                }
        }
        else
        {
		printf("<select name=%s %s><option title='No selection'>---</option></select>\n"
                        ,cLabel,cMode);
		if(!uMode)
		sprintf(cHidden,"<input type=hidden name=%.32s value='0'>\n",cLabel);
        }
        printf("</select>\n");
	if(cHidden[0])
		printf("%s",cHidden);

}//tTablePullDownOwner()


void tTablePullDown(const char *cTableName, const char *cFieldName,
                        const char *cOrderby, unsigned uSelector, unsigned uMode)
{
        register int i,n;
        char cLabel[128];
        MYSQL_RES *mysqlRes;         
        MYSQL_ROW mysqlField;

        char cSelectName[34]={""};
	char cHidden[100]={""};
        char cLocalTableName[256]={""};
        char *cp;
	char *cMode="";

	if(!uMode)
		cMode="disabled";
      
        if(!cTableName[0] || !cFieldName[0] || !cOrderby[0])
        {
                printf("Invalid input tTablePullDown()");
                return;
        }

        //Extended functionality
        strncpy(cLocalTableName,cTableName,255);
        if((cp=strchr(cLocalTableName,';')))
        {
                strncpy(cSelectName,cp+1,32);
                cSelectName[32]=0;
                *cp=0;
        }


        sprintf(gcQuery,"SELECT _rowid,%s FROM %s ORDER BY %s",
                                cFieldName,cLocalTableName,cOrderby);

        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
        {
                printf("%s",mysql_error(&gMysql));
                return;
        }
	mysqlRes=mysql_store_result(&gMysql);
	i=mysql_num_rows(mysqlRes);

	if(cSelectName[0])
                sprintf(cLabel,"%s",cSelectName);
        else
                sprintf(cLabel,"%s_%sPullDown",cLocalTableName,cFieldName);

        if(i>0)
        {
                printf("<select name=%s %s>\n",cLabel,cMode);

                //Default no selection
                printf("<option title='No selection'>---</option>\n");

                for(n=0;n<i;n++)
                {
                        int unsigned field0=0;

                        mysqlField=mysql_fetch_row(mysqlRes);
                        sscanf(mysqlField[0],"%u",&field0);

                        if(uSelector != field0)
                        {
                             printf("<option>%s</option>\n",mysqlField[1]);
                        }
                        else
                        {
                             printf("<option selected>%s</option>\n",mysqlField[1]);
			     if(!uMode)
			     sprintf(cHidden,"<input type=hidden name=%.32s value='%.32s'>\n",
			     		cLabel,mysqlField[1]);
                        }
                }
        }
        else
        {
		printf("<select name=%s %s><option title='No selection'>---</option></select>\n"
                        ,cLabel,cMode);
		if(!uMode)
		sprintf(cHidden,"<input type=hidden name=%.32s value='0'>\n",cLabel);
        }
        printf("</select>\n");
	if(cHidden[0])
		printf("%s",cHidden);

}//tTablePullDown()


int ReadPullDown(const char *cTableName,const char *cFieldName,const char *cLabel)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;

        unsigned int iRowid=0;//Not found

        sprintf(gcQuery,"select _rowid from %s where %s='%s'",
                        cTableName,cFieldName,TextAreaSave((char *) cLabel));
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&iRowid);
        mysql_free_result(mysqlRes);
        return(iRowid);

}//ReadPullDown(char *cTableName,char *cLabel)


char *TextAreaSave(char *cField)
{
	register unsigned int i=0,j=0,uNum=0,uCtrlM=0;
	char *cCopy=NULL;

	for(i=0;cField[i];i++)
	{
		if(cField[i]=='\'' || cField[i]=='\\') uNum++;
		if(cField[i]=='\r') uCtrlM++;
	}
	if(!uNum && !uCtrlM) return(cField);

	if(uNum)
		cCopy=(char *)malloc( ( (strlen(cField)) + (uNum*2) + 1 ));
	else
		cCopy=(char *)cField;

	if(!cCopy) htmlPlainTextError("TextAreaInput() malloc error");

	i=0;
	while(cField[i])
	{
		if( cField[i]=='\'' )
		{
			cCopy[j++]='\\';
			cCopy[j++]='\'';
		}
		else if( cField[i]=='\\')
		{
			cCopy[j++]='\\';
			cCopy[j++]='\\';
		}
		
		//Remove nasty ctrl-m's. fsck /u Bill!
		else if(cField[i]!='\r')
		{
			cCopy[j++]=cField[i];
		}
		i++;
	}

	cCopy[j]=0;
	return(cCopy);

}//char *TextAreaSave(char *cField)


char *TransformAngleBrackets(char *cField)
{
	register unsigned int i=0,j=0,uNum=0;
	char *cCopy=NULL;

	for(i=0;cField[i];i++)
		if(cField[i]=='<' || cField[i]=='>') uNum++;
	if(!uNum) return(cField);
	cCopy=(char *)malloc( ( (strlen(cField)) + (uNum*4) + 1 ));

	if(!cCopy) htmlPlainTextError("TransformAngleBrackets() malloc error");

	i=0;
	while(cField[i])
	{
		//Expand angle brackets into HTML codes
		if( cField[i]=='<' )
		{
			cCopy[j++]='&';
			cCopy[j++]='l';
			cCopy[j++]='t';
			cCopy[j++]=';';
		}
		else if( cField[i]=='>' )
		{
			cCopy[j++]='&';
			cCopy[j++]='g';
			cCopy[j++]='t';
			cCopy[j++]=';';
		}
		else if(1)
                {
                        cCopy[j++]=cField[i];
                }
		i++;
	}

	cCopy[j]=0;
	return(cCopy);

}//char *TransformAngleBrackets(char *cField)


char *EncodeDoubleQuotes(char *cField)
{
	register unsigned int i=0,j=0,uNum=0;
	char *cCopy=NULL;

	for(i=0;cField[i];i++)
		if(cField[i]=='"') uNum++;
	if(!uNum) return(cField);
	cCopy=(char *)malloc( ( (strlen(cField)) + (uNum*5) + 1 ));

	if(!cCopy) htmlPlainTextError("EncodeDoubleQuotes() malloc error");

	i=0;
	while(cField[i])
	{
		//Expand double quote into HTML codes
		if( cField[i]=='"' )
		{
			cCopy[j++]='&';
			cCopy[j++]='q';
			cCopy[j++]='u';
			cCopy[j++]='o';
			cCopy[j++]='t';
			cCopy[j++]=';';
		}
		else if(1)
                {
                        cCopy[j++]=cField[i];
                }
		i++;
	}

	cCopy[j]=0;
	return(cCopy);

}//char *EncodeDoubleQuotes(char *cField)


void YesNo(unsigned uSelect)
{
        if(uSelect)
                printf("Yes");
        else
                printf("No");

}//YesNo()


void YesNoPullDown(char *cFieldName, unsigned uSelect, unsigned uMode)
{
	char cHidden[100]={""};
	char *cMode="";

	if(!uMode)
		cMode="disabled";
      
	printf("<select name=cYesNo%s %s>\n",cFieldName,cMode);

        if(uSelect==0)
                printf("<option selected>No</option>\n");
        else
                printf("<option>No</option>\n");

        if(uSelect==1)
	{
                printf("<option selected>Yes</option>\n");
		if(!uMode)
			sprintf(cHidden,"<input type=hidden name=cYesNo%s value='Yes'>\n",
			     		cFieldName);
	}
        else
	{
                printf("<option>Yes</option>\n");
	}

        printf("</select>\n");
	if(cHidden[0])
		printf("%s",cHidden);

}//YesNoPullDown()


int ReadYesNoPullDown(const char *cLabel)
{
        if(!strcmp(cLabel,"Yes"))
                return(1);
        else
                return(0);

}//ReadYesNoPullDown(char *cLabel)


const char *ForeignKey(const char *cTableName, const char *cFieldName, unsigned uKey)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;

	static char cQuery[512];
	static char cKey[16];

        sprintf(cQuery,"SELECT %s FROM %s WHERE _rowid=%u",
                        cFieldName,cTableName,uKey);
        mysql_query(&gMysql,cQuery);
        if(mysql_errno(&gMysql))
		return(mysql_error(&gMysql));

        mysqlRes=mysql_store_result(&gMysql);
        if(mysql_num_rows(mysqlRes)==1)
        {
                mysqlField=mysql_fetch_row(mysqlRes);
                return(mysqlField[0]);
        }
	
	if(!uKey)
	{
        	return("---");
	}
	else
	{
		sprintf(cKey,"%u",uKey);
        	return(cKey);
	}

}//const char *ForeignKey(const char *cTableName, const char *cFieldName, unsigned uKey)



 //tValidFunc functions: Form validation feedback

const char *IsZero(unsigned uInput)
{
        if(uInput)
                return("black");
        else
                return("red");

}//const char *IsZero(unsigned uInput)

 
const char *BadIPNum(const char *cInput)
{ 
        if( cInput!=NULL && cInput[0] && strcmp(cInput,"0.0.0.0"))
                return("black");
        else
                return("red");

}//const char *BadIPNum(const char *cInput)


const char *EmptyString(const char *cInput)
{
        if(cInput!=NULL && cInput[0])
                return("black");
        else
                return("red");

}//const char *EmptyString(const char *cInput) 


//tInputFunc functions: Convert data on cgi form post

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


char *IPv4CIDR(char *cInput)
{
	unsigned a=0,b=0,c=0,d=0,e=0;

	sscanf(cInput,"%u.%u.%u.%u/%u",&a,&b,&c,&d,&e);

	if(a>255) a=0;
	if(b>255) b=0;
	if(c>255) c=0;
	if(d>255) d=0;
	if(e>32) e=32;
	if(e<20) e=20;

	//TODO ggw 8/2003 CIDR alignment math for a.b.c.d vs CIDR range 20-32
	//16 class C's - one IP

	sprintf(cInput,"%u.%u.%u.%u/%u",a,b,c,d,e);

	return(cInput);

}//char *IPv4CIDR(char *cInput)


char *IPv4Range(char *cInput)
{
	unsigned a=0,b=0,c=0,d=0,e=0;

	sscanf(cInput,"%u.%u.%u.%u-%u",&a,&b,&c,&d,&e);

	if(a>255) a=0;
	if(b>255) b=0;
	if(c>255) c=0;
	if(d>255) d=0;
	if(e>255) e=0;
	if(e<d) e=d;

	sprintf(cInput,"%u.%u.%u.%u-%u",a,b,c,d,e);

	return(cInput);

}//char *IPv4Range(char *cInput)


char *IPv4All(char *cInput)
{
	if(strchr(cInput,'-'))
		IPv4Range(cInput);
	else if(strchr(cInput,'/'))
		IPv4CIDR(cInput);
	else if(1)
		IPNumber(cInput);

	return(cInput);

}//char *IPv4All(char *cInput)


char *EmailInput(char *cInput)
{
	register int i;

	for(i=0;cInput[i];i++)
	{
	
		if(!isalnum(cInput[i]) && cInput[i]!='.'  && cInput[i]!='-' 
				&& cInput[i]!='@' && cInput[i]!='_')
			break;
		if(isupper(cInput[i])) cInput[i]=tolower(cInput[i]);
	}
	cInput[i]=0;

	return(cInput);

}//char *EmailInput(char *cInput)


char *cMoneyInput(char *cInput)
{
	register int i,j;
	char cOutput[32];

	//Allow only certain chars nums $ , and . Ex. $250,000.00
	for(i=0;cInput[i];i++)
	{
		if(!isdigit(cInput[i]) && cInput[i]!='.'  && cInput[i]!=',' 
				&& cInput[i]!='$')
			break;
	}
	cInput[i]=0;

	//Strip non internal chars
	for(i=0,j=0;cInput[i];i++)
	{
		if(cInput[i]==',' || cInput[i]=='$')
		{
			;
		}
		else
		{
			cOutput[j++]=cInput[i];
		}
	}
	cOutput[j]=0;
	sprintf(cInput,"%.31s",cOutput);

	//Only allow one .
	unsigned uCountPeriods=0;
	for(i=strlen(cInput);i>0;i--)
	{
		if(cInput[i]=='.')
		{
			uCountPeriods++;
			if(uCountPeriods>1)
				cInput[i]=',';
		}
	}
	if(uCountPeriods>1)
		cMoneyInput(cInput);

	return(cInput);

}//char *cMoneyInput(char *cInput)

	
char *cMoneyDisplay(char *cInput)
{
	double fBuffer;
	int i;
	//
	//We need to convert to double before calling strfmon
	fBuffer=atof(cInput);
	i=strlen(cInput);
	i+=3;
	//Of course you may change the locale if appropiate.
        setlocale(LC_MONETARY, "en_US");
	strfmon(cInput,i, "%n",fBuffer); 
	
	return(cInput);

}//char *cMoneyDisplay(char *cInput)


char *FQDomainName(char *cInput)
{
	register int i;

	for(i=0;cInput[i];i++)
	{
	
		if(!isalnum(cInput[i]) && cInput[i]!='.'  && cInput[i]!='-' 
			&& cInput[i]!='_' && cInput[i]!='@' && cInput[i]!='/' && cInput[i]!='*')
			break;
		if(isupper(cInput[i])) cInput[i]=tolower(cInput[i]);
	}
	cInput[i]=0;

	return(cInput);

}//char *FQDomainName(char *cInput)


char *WordToLower(char *cInput)
{
	register int i;

	for(i=0;cInput[i];i++)
	{
	
		if(!isalnum(cInput[i]) && cInput[i]!='_' && cInput[i]!='-'
				&& cInput[i]!='@' && cInput[i]!='.' ) break;
		if(isupper(cInput[i])) cInput[i]=tolower(cInput[i]);
	}
	cInput[i]=0;

	return(cInput);

}//char *WordToLower(char *cInput)



void EncryptPasswdWithSalt(char *gcPasswd, char *cSalt)
{
	char cPasswd[100]={""};
	char *cp;
			
	sprintf(cPasswd,"%.99s",gcPasswd);
	cp=crypt(cPasswd,cSalt);
	sprintf(gcPasswd,"%.99s",cp);

//Debug only
//printf("Content-type: text/html\n\n");
//printf("gcPasswd=(%s),cSalt=(%s)",gcPasswd,cSalt);
//exit(0);

}//void EncryptPasswdWithSalt(char *gcPasswd, char *cSalt)


void GetPLAndClient(char *cUser)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;

	//SQL FROM the defined external tables must provide db.tAuthorize and db.tClient for the other SQL
	// to work.
	sprintf(gcQuery,"SELECT tAuthorize.uPerm,tAuthorize.uCertClient,tAuthorize.uOwner,"
				"tClient.cLabel"
				" FROM " TAUTHORIZE "," TCLIENT
				" WHERE tAuthorize.uOwner=tClient.uClient"
				" AND tAuthorize.cLabel='%s'",cUser);
	macro_mySQLRunAndStore(mysqlRes);
	if(mysql_num_rows(mysqlRes))
	{
		mysqlField=mysql_fetch_row(mysqlRes);
		sscanf(mysqlField[0],"%d",&guPermLevel);
		sscanf(mysqlField[1],"%u",&guLoginClient);
		sscanf(mysqlField[2],"%u",&guCompany);
		sprintf(gcCompany,"%.99s",mysqlField[3]);
	}
	mysql_free_result(mysqlRes);

}//void GetPLAndClient()


void GetClientOwner(unsigned uClient, unsigned *uOwner)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;

        char cQuery[254];

	sprintf(cQuery,"SELECT uOwner FROM " TCLIENT " WHERE uClient=%u",uClient);

        mysql_query(&gMysql,cQuery);
        if(mysql_errno(&gMysql))
                htmlPlainTextError(mysql_error(&gMysql));
        mysqlRes=mysql_store_result(&gMysql);
        *uOwner=0;
        if((mysqlField=mysql_fetch_row(mysqlRes)))
                sscanf(mysqlField[0],"%u",uOwner);
        mysql_free_result(mysqlRes);

}//void GetClientOwner(unsigned uClient, unsigned *uOwner)


void iDNSLog(unsigned uTablePK, char *cTableName, char *cLogEntry)
{
        char cQuery[512];

	//uLogType==1 is this back-end cgi by default tLogType install
        sprintf(cQuery,"INSERT INTO tLog SET cLabel='%.63s',uLogType=1,uPermLevel=%u,uLoginClient=%u,cLogin='%.99s',cHost='%.99s',uTablePK=%u,cTableName='%.31s',uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",cLogEntry,guPermLevel,guLoginClient,gcLogin,gcHost,uTablePK,cTableName);

        mysql_query(&gMysql,cQuery);

}//void iDNSLog(unsigned uTablePK, char *cTableName, char *cLogEntry)


long luGetCreatedDate(char *cTableName, unsigned uTablePK)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
        char cQuery[254];
	long luCreatedDate=0;

        sprintf(cQuery,"SELECT uCreatedDate FROM %s WHERE _rowid=%u",
						cTableName,uTablePK);
        mysql_query(&gMysql,cQuery);
        if(mysql_errno(&gMysql))
                htmlPlainTextError(mysql_error(&gMysql));
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
                sscanf(mysqlField[0],"%lu",&luCreatedDate);
        mysql_free_result(mysqlRes);

	return(luCreatedDate);

}//long luGetCreatedDate(char *cTableName, unsigned uTablePK)


long luGetModDate(char *cTableName, unsigned uTablePK)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
        char cQuery[254];
	long luModDate=0;

        sprintf(cQuery,"SELECT uModDate FROM %s WHERE _rowid=%u",
						cTableName,uTablePK);
        mysql_query(&gMysql,cQuery);
        if(mysql_errno(&gMysql))
                htmlPlainTextError(mysql_error(&gMysql));
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
                sscanf(mysqlField[0],"%lu",&luModDate);
        mysql_free_result(mysqlRes);

	return(luModDate);

}//long luGetModDate(char *cTableName, unsigned uTablePK)


void htmlPlainTextError(const char *cError)
{
	char cQuery[1024];

	printf("Content-type: text/plain\n\n");
	printf("Please report this iDNS error message ASAP:\n%s\n",cError);

	//Attempt to report error in tLog
        sprintf(cQuery,"INSERT INTO tLog SET cLabel='htmlPlainTextError',uLogType=4,uPermLevel=%u,uLoginClient=%u,cLogin='%s',cHost='%s',cMessage=\"%s\",cServer='%s',uOwner=1,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",guPermLevel,guLoginClient,gcLogin,gcHost,cError,gcHostname,guLoginClient);
        mysql_query(&gMysql,cQuery);
        if(mysql_errno(&gMysql))
		printf("Another error occurred while attempting to log: %s\n",
				mysql_error(&gMysql));
	exit(0);

}//void htmlPlainTextError(const char *cError)

//This algo is based on GPL work in php-4.0.4p11 url.c and FSF face file urlencode.c
//Then modified to suit our needs and readability and UK style guidelines
char *cURLEncode(char *cURL)
{
        register int x,y;
        unsigned char *cp;
        int len=strlen(cURL);

        static unsigned char hexchars[] = "0123456789ABCDEF";

        cp=(unsigned char *)malloc(3*strlen(cURL)+1);
        for(x=0,y=0;len--;x++,y++)
        {
                cp[y]=(unsigned char)cURL[x];
                if(cp[y]==' ')
                {
                        cp[y]='+';
                }
                else if( (cp[y]<'0' && cp[y]!='-' && cp[y]!='.') ||
                                (cp[y]<'A' && cp[y]>'9') ||
                                (cp[y]>'Z' && cp[y]<'a' && cp[y]!='_') ||
                                (cp[y]>'z') )
                {
                        cp[y++]='%';
                        cp[y++]=hexchars[(unsigned char) cURL[x] >> 4];
                        cp[y]=hexchars[(unsigned char) cURL[x] & 15];
                }
        }

        cp[y]='\0';
        return((char *)cp);

}//char *cURLEncode(char *cURL)


//Starting cleanup of current client contact role permissions model
//Description
//1-. Root user == 1. Initial install super user, sets up root tClient ASP company
//	and thus the uOwner of this tClient entry is root. There should be only one
//	such company but this is not mandatory. This company will have the only 
//	contacts that may have access to everything.
//	Root user if owner of a record blocks delete operations done by others.
//2-. If record uOwner is 0 something is broken and only Root can delete or modify.
//3-. Any contact of a company with perm level >= admin can mod any record
//	owned by parent company. For delete with perm level >= root.
//4-. Any company contact with perm level >= root can delete or mod any record
//	of companies owned by parent company.
//5-. Any contact of perm level >=user that created a record and is owned by parent company can mod that record.
//	Similarly for delete if user is admin level
//
//7 user level
//10 admin level
//12 root level
unsigned uAllowDel(const unsigned uOwner, const unsigned uCreatedBy)
{
	if(guListMode) return(0);

	if(uOwner) GetClientOwner(uOwner,&guReseller);//Get owner of the owner
	
	if( (guPermLevel>9 && uOwner==guCompany) //r3
				|| (guPermLevel>9 && guCompany==guReseller) //r4
				|| (guPermLevel>9 && uCreatedBy==guLoginClient && uOwner==guCompany) //r5
				|| (guPermLevel>11 && guCompany==1) //r1
				|| (guPermLevel>11 && guLoginClient==1) )//r2
			return(1);
	return(0);
}//unsigned uAllowDel(...)


unsigned uAllowMod(const unsigned uOwner, const unsigned uCreatedBy)
{

	if(guListMode) return(0);

	if(uOwner) GetClientOwner(uOwner,&guReseller);//Get owner of the owner

	if( (guPermLevel>9 && uOwner==guCompany) //r3
				|| (guPermLevel>9 && guCompany==guReseller) //r4
				|| (guPermLevel>6 && uCreatedBy==guLoginClient && uOwner==guCompany) //r5
				|| (guPermLevel>11 && guCompany==1) //r1
				|| (guPermLevel>11 && guLoginClient==1) )//r2
			return(1);
	return(0);
}//unsigned uAllowMod(...)


void ExtListSelect(const char *cTable,const char *cVarList)
{
	if(guPermLevel>11)//Root can read access all
		sprintf(gcQuery,"SELECT %s FROM %s",
					cVarList,cTable);
	else 
		sprintf(gcQuery,"SELECT %1$s FROM %3$s," TCLIENT
				" WHERE %3$s.uOwner=" TCLIENT ".uClient"
				" AND (" TCLIENT ".uClient=%2$u OR " TCLIENT ".uOwner"
				" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%2$u OR uClient=%2$u))",
					cVarList,guCompany,cTable);
}//void ExtListSelect(...)


void ExtSelect(const char *cTable,const char *cVarList,unsigned uMaxResults)
{
	if(guPermLevel>11)//Root can read access all
		sprintf(gcQuery,"SELECT %1$s FROM %2$s ORDER BY %2$s._rowid",
					cVarList,cTable);
	else 
		sprintf(gcQuery,"SELECT %1$s FROM %3$s," TCLIENT
				" WHERE %3$s.uOwner=" TCLIENT ".uClient"
				" AND (" TCLIENT ".uClient=%2$u OR " TCLIENT ".uOwner"
				" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%2$u OR uClient=%2$u))"
				" ORDER BY %3$s._rowid",
					cVarList,guCompany,
					cTable);
	if(uMaxResults)
	{
		char cLimit[33]={""};
		sprintf(cLimit," LIMIT %u",uMaxResults);
		strcat(gcQuery,cLimit);
	}

}//void ExtSelect(...)


void ExtSelectSearch(const char *cTable,const char *cVarList,const char *cSearchField,
				const char *cSearch,const char *cExtraCond,unsigned uMaxResults)
{
	if(guPermLevel>11)//Root can read access all
	{
		if(cExtraCond!=NULL && cSearchField!=NULL)
			sprintf(gcQuery,"SELECT %1$s FROM %2$s WHERE %3$s LIKE '%4$s%%' AND %5$s ORDER BY %3$s",
						cVarList,cTable,cSearchField,cSearch,cExtraCond);
		else if(cExtraCond!=NULL && cSearchField==NULL)
			sprintf(gcQuery,"SELECT %1$s FROM %2$s WHERE %3$s",
						cVarList,cTable,cExtraCond);
		else if(cExtraCond==NULL && cSearchField!=NULL)
			sprintf(gcQuery,"SELECT %1$s FROM %2$s WHERE %3$s LIKE '%4$s%%' ORDER BY %3$s",
						cVarList,cTable,cSearchField,cSearch);
		else if(cExtraCond==NULL && cSearchField==NULL)
			sprintf(gcQuery,"SELECT %1$s FROM %2$s",cVarList,cTable);
	}
	else
	{
		if(cExtraCond!=NULL && cSearchField!=NULL)
			sprintf(gcQuery,"SELECT %1$s FROM %3$s," TCLIENT
				 	" WHERE %4$s LIKE '%5$s%%' AND %6$s AND %3$s.uOwner=" TCLIENT ".uClient"
					" AND (" TCLIENT ".uClient=%2$u OR " TCLIENT ".uOwner"
					" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%2$u OR uClient=%2$u))"
					" ORDER BY %4$s",
						cVarList,guCompany,
						cTable,cSearchField,cSearch,cExtraCond);
		else if(cExtraCond==NULL && cSearchField!=NULL)
			sprintf(gcQuery,"SELECT %1$s FROM %3$s," TCLIENT
				 	" WHERE %4$s LIKE '%5$s%%' AND %3$s.uOwner=" TCLIENT ".uClient"
					" AND (" TCLIENT ".uClient=%2$u OR " TCLIENT ".uOwner"
					" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%2$u OR uClient=%2$u))"
					" ORDER BY %4$s",
						cVarList,guCompany,
						cTable,cSearchField,cSearch);
		else if(cExtraCond!=NULL && cSearchField==NULL)
			sprintf(gcQuery,"SELECT %1$s FROM %3$s," TCLIENT
				 	" WHERE %4$s AND %3$s.uOwner=" TCLIENT ".uClient"
					" AND (" TCLIENT ".uClient=%2$u OR " TCLIENT ".uOwner"
					" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%2$u OR uClient=%2$u))",
						cVarList,guCompany,
						cTable,cExtraCond);
		else if(cExtraCond!=NULL && cSearchField!=NULL)
			sprintf(gcQuery,"SELECT %1$s FROM %3$s," TCLIENT
				 	" WHERE %4$s LIKE '%5$s%%' AND  %3$s.uOwner=" TCLIENT ".uClient"
					" AND (" TCLIENT ".uClient=%2$u OR " TCLIENT ".uOwner"
					" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%2$u OR uClient=%2$u))"
					" ORDER BY %4$s",
						cVarList,guCompany,
						cTable,cSearchField,cSearch);
		else if(cExtraCond==NULL && cSearchField==NULL)
			sprintf(gcQuery,"SELECT %1$s FROM %3$s," TCLIENT
				 	" WHERE %3$s.uOwner=" TCLIENT ".uClient"
					" AND (" TCLIENT ".uClient=%2$u OR " TCLIENT ".uOwner"
					" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%2$u OR uClient=%2$u))",
						cVarList,guCompany,cTable);
	}

	if(uMaxResults)
	{
		char cLimit[33]={""};
		sprintf(cLimit," LIMIT %u",uMaxResults);
		strncat(gcQuery,cLimit,32);
	}
	//debug only
	//iDNS(gcQuery);

}//void ExtSelectSearch(...)


void ExtSelectRow(const char *cTable,const char *cVarList,unsigned uRow)
{
	if(guPermLevel>11)//Root can read access all
		sprintf(gcQuery,"SELECT %s FROM %s WHERE %s._rowid=%u",
					cVarList,cTable,cTable,uRow);
	else 
		sprintf(gcQuery,"SELECT %1$s FROM %3$s," TCLIENT
				" WHERE %3$s.uOwner=" TCLIENT ".uClient"
                                " AND %3$s._rowid=%4$u"
				" AND (" TCLIENT ".uClient=%2$u OR " TCLIENT ".uOwner"
				" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%2$u OR uClient=%2$u))",
					cVarList,guCompany,
					cTable,uRow);
}//void ExtSelectRow(...)


//For those tables you anybody to be able to read. Like tStatus.
void ExtSelectRowPublic(const char *cTable,const char *cVarList,unsigned uRow)
{
		sprintf(gcQuery,"SELECT %s FROM %s WHERE u%s=%u",cVarList,cTable,cTable+1,uRow);
}//void ExtSelectRowPublic(...)


void ExtListSelectPublic(const char *cTable,const char *cVarList)
{
		sprintf(gcQuery,"SELECT %s FROM %s",cVarList,cTable);
}//void ExtListSelectPublic(...)


void ExtSelectPublic(const char *cTable,const char *cVarList)
{
		sprintf(gcQuery,"SELECT %s FROM %s",cVarList,cTable);

}//void ExtSelectPublic(...)


//Passwd stuff
static unsigned char itoa64[] =         /* 0 ... 63 => ascii - 64 */
        "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void to64(s, v, n)
  register char *s;
  register long v;
  register int n;
{
    while (--n >= 0) {
        *s++ = itoa64[v&0x3f];
        v >>= 6;
    }
}//void to64(s, v, n)


void EncryptPasswd(char *pw)
{
	//Notes:
	//	We should change time based salt 
	//	(could be used for faster dictionary attack)
	//	to /dev/random based system.

        char salt[3];
        char passwd[102]={""};
        char *cpw;
	char cMethod[16] ={""}; 

	GetConfiguration("cCryptMethod",cMethod,0);
	if(!strcmp(cMethod,"MD5"))
	{
		char cSalt[] = "$1$01234567$";
	    	(void)srand((int)time((time_t *)NULL));
    		to64(&cSalt[3],rand(),8);
		cpw = crypt(pw,cSalt);
		// error not verified, str NULL ("") returned	
	}
	else
	{
		// default DES method
	        sprintf(passwd,"%.99s",pw);
    		(void)srand((int)time((time_t *)NULL));
    		to64(&salt[0],rand(),2);
		cpw=crypt(passwd,salt);
	}	
	sprintf(pw,"%.99s",cpw);

}//void EncryptPasswd(char *pw)


int ReadPullDownOwner(const char *cTableName,const char *cFieldName,
				const char *cLabel,const unsigned uOwner)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;

        unsigned int iRowid=0;//Not found

        sprintf(gcQuery,"SELECT _rowid FROM %s WHERE %s='%s' AND (uOwner=%u OR uOwner=%u)",
                        cTableName,cFieldName,TextAreaSave((char *) cLabel),uOwner,guCompany);
        MYSQL_RUN_STORE(mysqlRes);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&iRowid);
        mysql_free_result(mysqlRes);
        return(iRowid);

}//ReadPullDownOwner()


//
//
//OTP and login logic

void SetLogin(void)
{
	if( iValidLogin(0) )
	{
		printf("Set-Cookie: iDNSLogin=%s; secure;\n",gcLogin);
		printf("Set-Cookie: iDNSPasswd=%s; secure;\n",gcPasswd);
		strncpy(gcUser,gcLogin,41);
		GetPLAndClient(gcUser);
		guSSLCookieLogin=1;
		if(guPermLevel>6)
			iDNS("DashBoard");
		else if(guPermLevel==6)
			iDNS("Welcome Organization Admin");
		else
			iDNS("Welcome");
	}
	else
	{
		guSSLCookieLogin=0;
		SSLCookieLogin();
	}
				
}//void SetLogin(void)


unsigned uValidOTP(char *cOTPSecret,char *cOTP)
{
#ifdef LIBOATH
	char *secret;
	size_t secretlen = 0;
	int rc;
	char otp[10];
	time_t now;

	rc=oath_init();
	if(rc!=OATH_OK)
		return(0);

	now=time(NULL);

	rc=oath_base32_decode(cOTPSecret,strlen(cOTPSecret),&secret,&secretlen);
	if(rc!=OATH_OK)
		goto gotoFail;

	//2 min time skew window
	rc=oath_totp_generate(secret,secretlen,now-60,30,0,6,otp);
	if(rc!=OATH_OK)
		goto gotoFail;
	if(!strcmp(cOTP,otp))
		goto gotoMatch;

	rc=oath_totp_generate(secret,secretlen,now-30,30,0,6,otp);
	if(rc!=OATH_OK)
		goto gotoFail;
	if(!strcmp(cOTP,otp))
		goto gotoMatch;

	rc=oath_totp_generate(secret,secretlen,now,30,0,6,otp);
	if(rc!=OATH_OK)
		goto gotoFail;
	if(!strcmp(cOTP,otp))
		goto gotoMatch;

	rc=oath_totp_generate(secret,secretlen,now+30,30,0,6,otp);
	if(rc!=OATH_OK)
		goto gotoFail;
	if(!strcmp(cOTP,otp))
		goto gotoMatch;

	rc=oath_totp_generate(secret,secretlen,now+60,30,0,6,otp);
	if(rc!=OATH_OK)
		goto gotoFail;
	if(!strcmp(cOTP,otp))
		goto gotoMatch;

gotoFail:
	free(secret);
	oath_done();
	return(0);

gotoMatch:
	free(secret);
	oath_done();
	return(1);
#else
	return(1);//fake match
#endif

}//unsigned uValidOTP(char *cOTPSecret,char *cOTP)


//with uAuthorize==0 it expires the OTP for a given guLoginClient
void UpdateOTPExpire(unsigned uAuthorize,unsigned uClient)
{

#ifdef LIBOATH

	//OTP login OK for 4 more hours. Change to configurable TODO.
	if(!uAuthorize)
		sprintf(gcQuery,"UPDATE " TAUTHORIZE " SET uOTPExpire=0 WHERE uCertClient=%u",
			uClient);
	else
		sprintf(gcQuery,"UPDATE " TAUTHORIZE " SET uOTPExpire=(UNIX_TIMESTAMP(NOW())+28800) WHERE uAuthorize=%u",
			uAuthorize);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
#endif
}//void UpdateOTPExpire()


char *cGetPasswd(char *gcLogin,char *cOTPSecret,unsigned long *luOTPExpire,unsigned long *luSQLNow,unsigned *uAuthorize)
{
	static char cPasswd[100]={""};
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
	char *cp;

	//SQL injection code
	if((cp=strchr(gcLogin,'\''))) *cp=0;

#ifdef LIBOATH
	sprintf(gcQuery,"SELECT cPasswd,cOTPSecret,uOTPExpire,UNIX_TIMESTAMP(NOW()),uAuthorize"
				" FROM " TAUTHORIZE " WHERE cLabel='%s'",gcLogin);
#else
	sprintf(gcQuery,"SELECT cPasswd,UNIX_TIMESTAMP(NOW()),uAuthorize"
				" FROM " TAUTHORIZE " WHERE cLabel='%s'",gcLogin);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
	mysqlRes=mysql_store_result(&gMysql);
	cPasswd[0]=0;
	if((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sprintf(cPasswd,"%.99s",mysqlField[0]);

		if(mysqlField[1])
		{
#ifdef LIBOATH
			sprintf(cOTPSecret,"%.64s",mysqlField[1]);
			sscanf(mysqlField[2],"%lu",luOTPExpire);
			sscanf(mysqlField[3],"%lu",luSQLNow);
			sscanf(mysqlField[4],"%u",uAuthorize);
#else
			sscanf(mysqlField[1],"%lu",luSQLNow);
			sscanf(mysqlField[2],"%u",uAuthorize);
#endif
		}
	}
	mysql_free_result(mysqlRes);
	
	return(cPasswd);

}//char *cGetPasswd()


int iValidLogin(int iMode)
{
	//private function
	void UpdateLogLoginOk(void)
	{
		sprintf(gcQuery,"INSERT INTO tLog SET cLabel='login ok %.99s',uLogType=6,uPermLevel=%u,"
			" uLoginClient=%u,cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=%u,"
			" uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()) ON DUPLICATE KEY UPDATE"
			" cLabel='login ok %.99s',uLogType=6,uPermLevel=%u,"
			" uLoginClient=%u,cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=%u,"
			" uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
				gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname,guCompany,
				gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname,guCompany);
		MYSQL_RUN;
		//LoginFirewallJobs(guLoginClient);
	}

	char cSalt[16]={""};
	char cPassword[100]={""};

	//Notes:
	//iMode=1 means we have encrypted passwd from cookie

	unsigned uAuthorize=0;
	long unsigned luOTPExpire=0;
	long unsigned luSQLNow=0;
	strncpy(cPassword,cGetPasswd(gcLogin,gcOTPSecret,&luOTPExpire,&luSQLNow,&uAuthorize),99);
	//If user has OTP secret then they must login with OTP every so often.
	if(luOTPExpire<=luSQLNow && gcOTPSecret[0])
		guOTPExpired=1;
	sprintf(gcOTPInfo,"{%s}/[%s] %u unexpected case %s/%s %u %u",gcOTPSecret,gcOTP,guOTPExpired,gcLogin,cPassword,uAuthorize,iMode);
	//debug only
	//iDNS(gcOTPInfo);
	if(cPassword[0])
	{
		//No cookies!
		if(!iMode)
		{

			//MD5 vs DES salt determination
			if(cPassword[0]=='$' && cPassword[2]=='$')
				sprintf(cSalt,"%.12s",cPassword);
			else
				sprintf(cSalt,"%.2s",cPassword);
			EncryptPasswdWithSalt(gcPasswd,cSalt);
			if(!strcmp(gcPasswd,cPassword)) 
			{
				guCompany=1;//If next line does not work
				GetPLAndClient(gcLogin);
				if(guOTPExpired && gcOTP[0] && gcOTPSecret[0])
				{
					if(!uValidOTP(gcOTPSecret,gcOTP))
					{
						guRequireOTPLogin=1;
						sprintf(gcOTPInfo,"{%s}/[%s] %u login invalid gcOTP",gcOTPSecret,gcOTP,guOTPExpired);
						//LogoutFirewallJobs(guLoginClient);
						return(0);
					}
					else
					{
						guRequireOTPLogin=0;
						guOTPExpired=0;
						UpdateOTPExpire(uAuthorize,0);
						sprintf(gcOTPInfo,"{%s}/[%s] %u login valid gcOTP",gcOTPSecret,gcOTP,guOTPExpired);
						UpdateLogLoginOk();
						return(1);
					}
				}
				else if(guOTPExpired)
				{
					guRequireOTPLogin=1;
					sprintf(gcOTPInfo,"{%s}/[%s] %u login valid but expired",gcOTPSecret,gcOTP,guOTPExpired);
					//LogoutFirewallJobs(guLoginClient);
					return(0);
				}
				sprintf(gcOTPInfo,"{%s}/[%s] %u login valid",gcOTPSecret,gcOTP,guOTPExpired);
				UpdateLogLoginOk();
				return(1);
			}
		}
		//Cookies supplied gcPasswd
		else
		{
			if(!strcmp(gcPasswd,cPassword))
			{
				if(guOTPExpired && gcOTP[0] && gcOTPSecret[0])
				{
					if(!uValidOTP(gcOTPSecret,gcOTP))
					{
						guRequireOTPLogin=1;
						sprintf(gcOTPInfo,"{%s}/[%s] %u cookie login expired invalid gcOTP (%s)",
							gcOTPSecret,gcOTP,guOTPExpired,gcUser);
						//LogoutFirewallJobs(guLoginClient);
						return(0);
					}
					else
					{
						guOTPExpired=0;
						guRequireOTPLogin=0;
						UpdateOTPExpire(uAuthorize,0);
						sprintf(gcOTPInfo,"{%s}/[%s] %u cookie login valid gcOTP",gcOTPSecret,gcOTP,guOTPExpired);
						return(1);
					}
				}
				else if(guOTPExpired)
				{
					guRequireOTPLogin=1;
					sprintf(gcOTPInfo,"{%s}/[%s] %u cookie login expired no gcOTP",gcOTPSecret,gcOTP,guOTPExpired);
					//LogoutFirewallJobs(guLoginClient);
					return(0);
				}
				sprintf(gcOTPInfo,"{%s}/[%s] %u cookie login valid",gcOTPSecret,gcOTP,guOTPExpired);
				return(1);
			}
		}
	}

	//No cookies and passwords did not match OR no password at all
	if(!iMode)
	{
		guCompany=1;//If next line does not work
		GetPLAndClient(gcLogin);
		//guPermLevel=0;
		//guLoginClient=0;
		sprintf(gcQuery,"INSERT INTO tLog SET cLabel='login failed %.99s',uLogType=6,uPermLevel=%u,"
				" uLoginClient=%u,cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=%u,"
				" uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()) ON DUPLICATE KEY UPDATE"
				" cLabel='login failed %.99s',uLogType=6,uPermLevel=%u,"
				" uLoginClient=%u,cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=%u,"
				" uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname,guCompany,
					gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname,guCompany);
		MYSQL_RUN;
		//LogoutFirewallJobs(guLoginClient);
	}

	if(guOTPExpired)
		guRequireOTPLogin=1;
	sprintf(gcOTPInfo,"{%s}/[%s] %u invalid login",gcOTPSecret,gcOTP,guOTPExpired);
	return 0;

}//iValidLogin()


void SSLCookieLogin(void)
{
	char *ptr,*ptr2;

	//Parse out login and passwd from cookies
	 
		

	if(getenv("HTTP_COOKIE")!=NULL)
		strncpy(gcCookie,getenv("HTTP_COOKIE"),1022);
	
	if(gcCookie[0])
	{

	if((ptr=strstr(gcCookie,"iDNSLogin=")))
	{
		ptr+=strlen("iDNSLogin=");
		if((ptr2=strchr(ptr,';')))
		{
			*ptr2=0;
			strncpy(gcLogin,ptr,99);
			*ptr2=';';
		}
		else
		{
			strncpy(gcLogin,ptr,99);
		}
	}
	if((ptr=strstr(gcCookie,"iDNSPasswd=")))
	{
		ptr+=strlen("iDNSPasswd=");
		if((ptr2=strchr(ptr,';')))
		{
			*ptr2=0;
			sprintf(gcPasswd,"%.99s",ptr);
			*ptr2=';';
		}
		else
		{
			sprintf(gcPasswd,"%.99s",ptr);
		}
	}
	
	}//if gcCookie[0] time saver

	if(!iValidLogin(1))
		htmlSSLLogin();

	strncpy(gcUser,gcLogin,41);
	GetPLAndClient(gcUser);
	if(!guPermLevel || !guLoginClient)
		iDNS("Access denied");
	gcPasswd[0]=0;
	guSSLCookieLogin=1;

}//SSLCookieLogin()


void htmlSSLLogin(void)
{
        Header_ism3("",0);

	printf("<p>\n");
	printf("Login: <input type=text title='Enter your login name' size=20 maxlength=98 id=\"username\" name=\"username\" >\n");
	printf(" Passwd: <input type=password title='Enter your password' size=20 maxlength=20 id=\"password\" name=\"password\" >\n");
	if(guRequireOTPLogin)
		printf(" Validation code: <input type=text size=8 maxlength=8"
			" title='Enter your 6 digit one time password. Download google authenticator"
			" or similar. Ask your admin for the barcode or secret.' name=gcOTP autocomplete=off >\n");
	printf("<font size=1> <input id=login type=submit name=gcFunction value=Login >\n");

        Footer_ism3();

}//void htmlSSLLogin(void)

