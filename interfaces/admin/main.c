/*
FILE 
	main.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file in main source dir.
PURPOSE
	iDNS Admin (Owner) Interface
REQUIRES
	OpenISP libtemplates.a and templates.h
*/

#include "interface.h"

//Global vars
//libtemplate.a required
MYSQL gMysql;
//Multipurpose buffer
char gcQuery[4096]={""};

//
//Template vars
char *gcMessage="&nbsp;";
char gcInputStatus[32]={"disabled"};
char gcPermInputStatus[32]={"disabled"};
char gcModStep[32]={""};
char gcNewStep[32]={""};
char gcDelStep[32]={""};
char gcZone[256]={""};
char gcCustomer[256]={""};
//SSLLoginCookie()
char gcCookie[1024]={""};
char gcLogin[100]={""};
char cLogKey[16]={"Ksdj458jssdUjf79"};
char gcPasswd[100]={""};
unsigned guSSLCookieLogin=0;

int guPermLevel=0;
char gcuPermLevel[4]={""};
unsigned guLoginClient=0;
unsigned guOrg=0;
unsigned guASPContact=0;
char gcUser[100]={""};
char gcName[100]={""};
char gcOrgName[100]={""};
char gcHost[100]={""};
char gcHostname[100]={""};

char gcFunction[100]={""};
char gcPage[100]={""};
unsigned guBrowserFirefox=0;

//new cookie cleanup
unsigned guCookieResource=0;
unsigned guCookieView=0;
char gcView[32]={""};
unsigned guCookieContact=0;
char gcCookieZone[100]={""};
char gcCookieCustomer[100]={""};

//
//Local only
int main(int argc, char *argv[]);
int iValidLogin(int mode);
void SSLCookieLogin(void);
void SetLogin(void);
void GetPLAndClient(char *cUser);
void htmlLogin(void);
void htmlLoginPage(char *cTitle, char *cTemplateName);
char *cShortenText(char *cText);
void SetSessionCookie(void);
void GetSessionCookie(void);
char *FQDomainName2(char *cInput);

int main(int argc, char *argv[])
{
	pentry entries[MAXPOSTVARS];
	entry gentries[MAXGETVARS];
	char *gcl;
	register int i;
	int cl=0;


	ConnectDb();

	if(getenv("REMOTE_ADDR")!=NULL)
		sprintf(gcHost,"%.99s",getenv("REMOTE_ADDR"));

	if(getenv("HTTP_USER_AGENT")!=NULL)
	{
		if(strstr(getenv("HTTP_USER_AGENT"),"Firefox"))
			guBrowserFirefox=1;
	}

	gethostname(gcHostname, 98);

	if(strcmp(getenv("REQUEST_METHOD"),"POST"))
	{
		//Get	
		gcl = getenv("QUERY_STRING");
		for(i=0;gcl[0] != '\0' && i<MAXGETVARS;i++)
		{
			getword(gentries[i].val,gcl,'&');
			plustospace(gentries[i].val);
			unescape_url(gentries[i].val);
			getword(gentries[i].name,gentries[i].val,'=');

			if(!strcmp(gentries[i].name,"gcFunction"))
				sprintf(gcFunction,"%.99s",gentries[i].val);
			else if(!strcmp(gentries[i].name,"gcPage"))
				sprintf(gcPage,"%.99s",gentries[i].val);
			else if(!strcmp(gentries[i].name,"cZone"))
				sprintf(gcZone,"%.99s",gentries[i].val);
			else if(!strcmp(gentries[i].name,"uView"))
				sprintf(cuView,"%.15s",gentries[i].val);
			else if(!strcmp(gentries[i].name,"uResource"))
				sscanf(gentries[i].val,"%u",&uResource);
			else if(!strcmp(gentries[i].name,"cCustomer"))
				sprintf(gcCustomer,"%.99s",gentries[i].val);
		}
		SSLCookieLogin();
		GetSessionCookie();
		//Required to be logged in GET section
		if(gcPage[0])
		{
			if(!strcmp(gcPage,"Customer"))
				CustomerGetHook(gentries,i);
			else if(!strcmp(gcPage,"CustomerUser"))
				CustomerContactGetHook(gentries,i);
			else if(!strcmp(gcPage,"Administrator"))
				AdminUserGetHook(gentries,i);
			else if(!strcmp(gcPage,"Glossary"))
				GlossaryGetHook(gentries,i);
			else if(!strcmp(gcPage,"Zone"))
				ZoneGetHook(gentries,i);
			else if(!strcmp(gcPage,"Resource"))
				ResourceGetHook(gentries,i);
			else if(!strcmp(gcPage,"Report"))
				ReportGetHook(gentries,i);
			else if(!strcmp(gcPage,"BulkOp"))
				BulkOpGetHook(gentries,i);
			else if(!strcmp(gcPage,"RestoreZone"))
				RestoreZoneGetHook(gentries,i);
			else if(!strcmp(gcPage,"RestoreResource"))
				RestoreResourceGetHook(gentries,i);
			else if(!strcmp(gcPage,"IPAuth"))
				IPAuthGetHook(gentries,i);
			else if(!strcmp(gcPage,"IPAuthDetail"))
				htmlIPAuthDetail();
			else if(!strcmp(gcPage,"Dashboard"))
				htmlDashBoard();
			else if(1)
				htmlDashBoard();

		}
	}
	else
	{
		//Post
		GetSessionCookie();
		cl = atoi(getenv("CONTENT_LENGTH"));
		for(i=0;cl && (!feof(stdin)) && i<MAXPOSTVARS ;i++)
		{
			entries[i].val = fmakeword(stdin,'&',&cl);
			plustospace(entries[i].val);
			unescape_url(entries[i].val);
			entries[i].name = makeword(entries[i].val,'=');
			
			if(!strcmp(entries[i].name,"gcFunction"))
				sprintf(gcFunction,"%.99s",entries[i].val);
			else if(!strcmp(entries[i].name,"gcPage"))
				sprintf(gcPage,"%.99s",entries[i].val);
			else if(!strcmp(entries[i].name,"gcLogin"))
				sprintf(gcLogin,"%.99s",entries[i].val);
			else if(!strcmp(entries[i].name,"gcPasswd"))
				sprintf(gcPasswd,"%.99s",entries[i].val);
			else if(!strcmp(entries[i].name,"cZone"))
				sprintf(gcZone,"%.99s",entries[i].val);
			else if(!strcmp(entries[i].name,"uView"))
				sprintf(cuView,"%.15s",entries[i].val);
			else if(!strcmp(entries[i].name,"uResource"))
				sscanf(entries[i].val,"%u",&uResource);
			else if(!strcmp(entries[i].name,"cCustomer"))
				sprintf(gcCustomer,"%.99s",entries[i].val);
		}
	}

	//Not required to be logged in gcFunction section
	if(gcFunction[0])
	{
		if(!strncmp(gcFunction,"Logout",5))
		{
			printf("Set-Cookie: idnsAdminLogin=; expires=\"Mon, 01-Jan-1971 00:10:10 GMT\"\n");
			printf("Set-Cookie: idnsAdminPasswd=; expires=\"Mon, 01-Jan-1971 00:10:10 GMT\"\n");
			printf("Set-Cookie: iDNSSessionCookie=; expires=\"Mon, 01-Jan-1971 00:10:10 GMT\"\n");
			sprintf(gcQuery,"INSERT INTO tLog SET cLabel='logout %.99s',uLogType=7,uPermLevel=%u,"
				"uLoginClient=%u,cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=%u,"
				"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname,guOrg);
			mysql_query(&gMysql,gcQuery);
        		guPermLevel=0;
			gcUser[0]=0;
			guLoginClient=0;
			htmlLogin();
		}
	}

        if(!strcmp(gcFunction,"Login")) SetLogin();
	
        if(!guPermLevel || !gcUser[0] || !guLoginClient)
                SSLCookieLogin();

	//First page after valid login
	if(!strcmp(gcFunction,"Login"))
	{
		if(guPermLevel>9)
		{
			htmlDashBoard();
		}
		else
		{
			//This should not be needed but can't find the error. And this works for now.
			printf("Set-Cookie: idnsAdminLogin=; expires=\"Mon, 01-Jan-1971 00:10:10 GMT\"\n");
			printf("Set-Cookie: idnsAdminPasswd=; expires=\"Mon, 01-Jan-1971 00:10:10 GMT\"\n");
			printf("Set-Cookie: iDNSSessionCookie=; expires=\"Mon, 01-Jan-1971 00:10:10 GMT\"\n");
			sprintf(gcQuery,"INSERT INTO tLog SET cLabel='perm logout %.99s',uLogType=7,uPermLevel=%u,"
				"uLoginClient=%u,cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=%u,"
				"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname,guOrg);
			mysql_query(&gMysql,gcQuery);
        		guPermLevel=0;
			gcUser[0]=0;
			guLoginClient=0;
			htmlLogin();
		}
	}

	//Per page command tree
	CustomerCommands(entries,i);
	CustomerContactCommands(entries,i);
	AdminUserCommands(entries,i);
	ZoneCommands(entries,i);
	ResourceCommands(entries,i);
	BulkOpCommands(entries,i);
	ReportCommands(entries,i);
	RestoreZoneCommands(entries,i);
	RestoreResourceCommands(entries,i);
	IPAuthCommands(entries,i);

	//default logged in page
	htmlCustomer();
	return(0);

}//end of main()


void htmlLogin(void)
{
	htmlHeader("idnsAdmin","Header");
	htmlLoginPage("idnsAdmin","AdminLogin.Body");
	htmlFooter("Footer");

}//void htmlLogin(void)


void htmlLoginPage(char *cTitle, char *cTemplateName)
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
			
			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
			
			template.cpName[1]="cCGI";
			template.cpValue[1]="idnsAdmin.cgi";
			
			template.cpName[2]="cMessage";
			template.cpValue[2]=gcMessage;

			template.cpName[3]="";

			printf("\n<!-- Start htmlLoginPage(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlLoginPage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlLoginPage()


void htmlPlainTextError(const char *cError)
{
	char cQuery[1024];

	printf("Content-type: text/plain\n\n");
	printf("Please report this idnsAdmin fatal error ASAP:\n%s\n",cError);
	//Attempt to report error in tLog
	sprintf(cQuery,"INSERT INTO tLog SET cLabel='htmlPlainTextError',uLogType=4,uPermLevel=%u,uLoginClient=%u,"
			"cLogin='%s',cHost='%s',cMessage=\"%s\",cServer='%s',uOwner=%u,uCreatedBy=%u,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW())",
				guPermLevel,guLoginClient,gcLogin,gcHost,cError,gcHostname,guOrg,guLoginClient);
	mysql_query(&gMysql,cQuery);
        if(mysql_errno(&gMysql))
		printf("Another error occurred while attempting to log: %s\n",
				mysql_error(&gMysql));
	exit(0);

}//void htmlPlainTextError(const char *cError)


void htmlHeader(char *cTitle, char *cTemplateName)
{
	printf("Content-type: text/html\n\n");

	if(cTemplateName[0])
	{
		MYSQL_RES *res;
	        MYSQL_ROW field;

		TemplateSelectInterface(cTemplateName,uPLAINSET,uIDNSADMINTYPE);
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			struct t_template template;
			template.cpName[0]="cTitle";
			template.cpValue[0]="idnsAdmin";
		
			template.cpName[1]="";

			printf("\n<!-- Start htmlHeader(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlHeader(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr><center><font size=1>%s %s</font>",cTitle,cTemplateName);
		}
		mysql_free_result(res);
	}
	else
	{
		printf("<html><head><title>%s</title></head><body bgcolor=white><font face=Arial,Helvetica",
					cTitle);
	}

}//void htmlHeader()


void htmlFooter(char *cTemplateName)
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
			template.cpName[0]="cIspName";
			template.cpValue[0]=ISPNAME;
			template.cpName[1]="cIspUrl";
			template.cpValue[1]=ISPURL;
			template.cpName[2]="cCopyright";
			template.cpValue[2]=LOCALCOPYRIGHT;
			template.cpName[3]="";

			printf("\n<!-- Start htmlFooter(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlFooter(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}
	else
	{
		printf("<hr>");
		printf("<center><font size=1><a href=%s>%s</a>\n",
				ISPURL,ISPNAME);
		printf("</body></html>");
	}

	exit(0);

}//void htmlFooter()


void fpTemplate(FILE *fp,char *cTemplateName,struct t_template *template)
{
	if(cTemplateName[0])
	{	
        	MYSQL_RES *res;
	        MYSQL_ROW field;

		TemplateSelectInterface(cTemplateName,uPLAINSET,uIDNSADMINTYPE);
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			fprintf(fp,"\n<!-- Start fpTemplate(%s) -->\n",cTemplateName); 
			Template(field[0], template, fp);
			fprintf(fp,"\n<!-- End fpTemplate(%s) -->\n",cTemplateName); 
		}
		else
		{
			fprintf(fp,"<hr>");
			fprintf(fp,"<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void fpTemplate(FILE *fp,char *cTemplateName,struct t_template *template)


//libtemplate.a required
void AppFunctions(FILE *fp,char *cFunction)
{
	if(!strcmp(cFunction,"funcPermLevelDropDown"))
		funcPermLevelDropDown(fp,1);
	else if(!strcmp(cFunction,"funcPermLevelDropDown2"))
		funcPermLevelDropDown(fp,0);
	else if(!strcmp(cFunction,"funcTablePullDownResellers"))
		funcTablePullDownResellers(fp,1);				
	else if(!strcmp(cFunction,"funcTablePullDownResellers2"))
		funcTablePullDownResellers(fp,0);
	else if(!strcmp(cFunction,"funcCustomerContacts"))
		funcCustomerContacts(fp);
	else if(!strcmp(cFunction,"funcRRs"))
                funcRRs(fp);
	else if(!strcmp(cFunction,"funcSelectView"))
		funcSelectView(fp);
	else if(!strcmp(cFunction,"funcSelectRRType"))
                funcSelectRRType(fp,1);
	else if(!strcmp(cFunction,"funcSelectRRTypeWiz"))
		funcSelectRRType(fp,0);
	else if(!strcmp(cFunction,"funcReportResults"))
		funcReportResults(fp);
	else if(!strcmp(cFunction,"funcReportHitsTop20"))
		funcReportHitsTop20(fp);
	else if(!strcmp(cFunction,"funcSelectHitMonth"))
		funcSelectHitMonth(fp);
	else if(!strcmp(cFunction,"funcReportOverallChanges"))
		funcReportOverallChanges(fp);
	else if(!strcmp(cFunction,"funcSelectSecondaryYesNo"))
		funcSelectSecondaryYesNo(fp);
	else if(!strcmp(cFunction,"funcDeletedRRs"))
		funcDeletedRRs(fp,0);
	else if(!strcmp(cFunction,"funcDeletedRRs2"))
		funcDeletedRRs(fp,1);
	else if(!strcmp(cFunction,"funcAvailableZones"))
		funcAvailableZones(fp);
#ifdef EXPERIMENTAL
	else if(!strcmp(cFunction,"funcZoneStatus"))
		funcZoneStatus(fp);
#endif
	else if(!strcmp(cFunction,"funcContactNavList"))
		funcContactNavList(fp,0);
	else if(!strcmp(cFunction,"funcZoneList"))
		funcZoneList(fp);
	else if(!strcmp(cFunction,"funcTopTenidnsOrgUsers"))
		funcTopTenidnsOrgUsers(fp);
	else if(!strcmp(cFunction,"funcTopTenidnsAdminUsers"))
		funcTopTenidnsAdminUsers(fp);
	else if(!strcmp(cFunction,"funcTopTenZoneMods"))
		funcTopTenZoneMods(fp);
	else if(!strcmp(cFunction,"funcTopTenTraffZones"))
		funcTopTenTraffZones(fp);
	else if(!strcmp(cFunction,"funcRequestGraphBtn"))
		funcRequestGraphBtn(fp);
	else if(!strcmp(cFunction,"funcContactLast7DaysActivity"))
		funcContactLast7DaysActivity(fp);
	else if(!strcmp(cFunction,"funcAdmLast7DaysAct"))
		funcAdmLast7DaysAct(fp,0);
	else if(!strcmp(cFunction,"funcOptionalGraph"))
		funcOptionalGraph(fp);
	else if(!strcmp(cFunction,"funcTopInfo"))
		funcTopInfo(fp);
	else if(!strcmp(cFunction,"funcUsageGraph"))
		funcUsageGraph(fp);
	else if(!strcmp(cFunction,"funcZoneNavList"))
		funcZoneNavList(fp,0);
	else if(!strcmp(cFunction,"funcAdminUsers"))
		funcAdminUsers(fp);
	else if(!strcmp(cFunction,"funcCompanyNavList"))
		funcCompanyNavList(fp,0);
	else if(!strcmp(cFunction,"funcMetaParam"))
		funcMetaParam(fp);
	else if(!strcmp(cFunction,"funcNSSetMembers"))
		funcNSSetMembers(fp);
	else if(!strcmp(cFunction,"funcRRMetaParam"))
		funcRRMetaParam(fp);
	else if(!strcmp(cFunction,"funcIPAuthReport"))
		funcIPAuthReport(fp);
	else if(!strcmp(cFunction,"funcReportActions"))
		funcReportActions(fp);
	else if(!strcmp(cFunction,"funcRemovedCompanies"))
		funcRemovedCompanies(fp);
	else if(!strcmp(cFunction,"funcRemovedBlocks"))
		funcRemovedBlocks(fp);
	else if(!strcmp(cFunction,"funcIgnoredLines"))
		funcIgnoredLines(fp);

}//void AppFunctions(FILE *fp,char *cFunction)


//
//Login functions section
char *cGetPasswd(char *gcLogin)
{
	static char cPasswd[100]={""};
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
	char *cp;

	//SQL injection code
	if((cp=strchr(gcLogin,'\''))) *cp=0;

	sprintf(gcQuery,"SELECT cPasswd FROM tAuthorize WHERE cLabel='%s'",
			gcLogin);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		htmlPlainTextError(mysql_error(&gMysql));
	}
	mysqlRes=mysql_store_result(&gMysql);
	cPasswd[0]=0;
	if((mysqlField=mysql_fetch_row(mysqlRes)))
		sprintf(cPasswd,"%.99s",mysqlField[0]);
	mysql_free_result(mysqlRes);

	
	return(cPasswd);

}//char *cGetPasswd(char *gcLogin)


void SSLCookieLogin(void)
{
	char *ptr,*ptr2;

	//Parse out login and passwd from cookies
#ifdef SSLONLY
	if(getenv("HTTPS")==NULL) 
		htmlPlainTextError("Non SSL access denied");
#endif

	if(getenv("HTTP_COOKIE")!=NULL)
		sprintf(gcCookie,"%.1022s",getenv("HTTP_COOKIE"));
	
	if(gcCookie[0])
	{

	if((ptr=strstr(gcCookie,"idnsAdminLogin=")))
	{
		ptr+=strlen("idnsAdminLogin=");
		if((ptr2=strchr(ptr,';')))
		{
			*ptr2=0;
			sprintf(gcLogin,"%.99s",ptr);
			*ptr2=';';
		}
		else
		{
			sprintf(gcLogin,"%.99s",ptr);
		}
	}
	if((ptr=strstr(gcCookie,"idnsAdminPasswd=")))
	{
		ptr+=strlen("idnsAdminPasswd=");
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
		htmlLogin();

	sprintf(gcUser,"%.41s",gcLogin);
	GetPLAndClient(gcUser);
	if(!guPermLevel || !guLoginClient)
		htmlPlainTextError("Unexpected guPermLevel or guLoginClient value");
	
	if(guPermLevel<10)
		htmlLogin();

	gcPasswd[0]=0;
	guSSLCookieLogin=1;

}//SSLCookieLogin()


void GetPLAndClient(char *cUser)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
	char cASP[100]={""};

	sprintf(gcQuery,"SELECT tAuthorize.uPerm,tAuthorize.uCertClient,tAuthorize.uOwner FROM"
				" tAuthorize,tClient WHERE tAuthorize.cLabel='%s'",
		cUser);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	mysqlRes=mysql_store_result(&gMysql);
	if((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[0],"%d",&guPermLevel);
		sscanf(mysqlField[1],"%u",&guLoginClient);
		sscanf(mysqlField[2],"%u",&guOrg);
	}
	mysql_free_result(mysqlRes);
	
	sprintf(gcQuery,"SELECT tClient.cLabel FROM tClient WHERE tClient.uClient=%u",guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	mysqlRes=mysql_store_result(&gMysql);
	if((mysqlField=mysql_fetch_row(mysqlRes)))
		sprintf(gcName,"%s",mysqlField[0]);
	mysql_free_result(mysqlRes);

	if(guOrg==1)
		GetConfiguration("cASP",gcOrgName,1);
	else
	{
		sprintf(gcQuery,"SELECT cLabel FROM tClient WHERE uClient=%u",guOrg);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		mysqlRes=mysql_store_result(&gMysql);
		if((mysqlField=mysql_fetch_row(mysqlRes)))
			sprintf(gcOrgName,"%.100s",mysqlField[0]);
		mysql_free_result(mysqlRes);
	}

	//Ticket #80
	GetConfiguration("cASP",cASP,1);
	if(!strcmp(gcOrgName,cASP)) guASPContact=1;

}//void GetPLAndClient()


void EncryptPasswdWithSalt(char *pw, char *salt)
{
	char passwd[102]={""};
	char *cpw;
			
	sprintf(passwd,"%.101s",pw);
				
	cpw=crypt(passwd,salt);

	sprintf(pw,"%s",cpw);

}//void EncryptPasswdWithSalt(char *pw, char *salt)


int iValidLogin(int mode)
{
	char cSalt[16]={""};
	char cPassword[100]={""};

	//Notes:
	//Mode=1 means we have encrypted passwd from cookie

	sprintf(cPassword,"%.99s",cGetPasswd(gcLogin));
	if(cPassword[0])
	{
		if(!mode)
		{
			//MD5 vs DES salt determination
			if(cPassword[0]=='$' && cPassword[2]=='$')
				sprintf(cSalt,"%.12s",cPassword);
			else
				sprintf(cSalt,"%.2s",cPassword);
			EncryptPasswdWithSalt(gcPasswd,cSalt);
			if(!strcmp(gcPasswd,cPassword))
					return 1;
		}
		else
		{
			if(!strcmp(gcPasswd,cPassword))
					return 1;
		}
	}
	if(!mode)
	{
		sprintf(gcQuery,"INSERT INTO tLog SET cLabel='login failed %.99s',uLogType=7,uPermLevel=%u,uLoginClient=%u,"
		"cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
		gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname);
		mysql_query(&gMysql,gcQuery);
	}
	return 0;

}//iValidLogin()


void SetLogin(void)
{
	if( iValidLogin(0) )
	{
		printf("Set-Cookie: idnsAdminLogin=%s; Secure; HttpOnly\n",gcLogin);
		printf("Set-Cookie: idnsAdminPasswd=%s; Secure; HttpOnly\n",gcPasswd);
		sprintf(gcUser,"%.41s",gcLogin);
		GetPLAndClient(gcUser);
		guSSLCookieLogin=1;
		//tLogType.cLabel='admin login'->uLogType=7
		sprintf(gcQuery,"INSERT INTO tLog SET cLabel='login ok %.99s',uLogType=7,uPermLevel=%u,uLoginClient=%u,"
				"cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=%u,uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())",
				gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname,guOrg);
		mysql_query(&gMysql,gcQuery);
	}
	else
	{
		guSSLCookieLogin=0;
		SSLCookieLogin();
	}
				
}//void SetLogin(void)

//
//End login fuctions section
//
//RAD / mysqlBind functions

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

#define BO_CUSTOMER	"Backend Customer"
#define BO_RESELLER	"Backend Reseller"
#define BO_ADMIN 	"Backend Admin"
#define BO_ROOT 	"Backend Root"

#define ORG_CUSTOMER	"Organization Customer"
#define ORG_WEBMASTER	"Organization Webmaster"
#define ORG_SALES	"Organization Sales Force"
#define ORG_SERVICE	"Organization Customer Service"
#define ORG_ACCT	"Organization Bookkeeper"
#define ORG_ADMIN	"Organization Admin"
const char *cUserLevel(unsigned uPermLevel)
{
	switch(uPermLevel)
	{
		case 12:
			return(BO_ROOT);
		break;
		case 10:
			return(BO_ADMIN);
		break;
		case 8:
			return(BO_RESELLER);
		break;
		case 7:
			return(BO_CUSTOMER);
		break;
		case 6:
			return(ORG_ADMIN);
		break;
		case 5:
			return(ORG_ACCT);
		break;
		case 4:
			return(ORG_SERVICE);
		break;
		case 3:
			return(ORG_SALES);
		break;
		case 2:
			return(ORG_WEBMASTER);
		break;
		case 1:
			return(ORG_CUSTOMER);
		break;
		default:
			return("---");
		break;
	}

}//const char *cUserLevel(unsigned uPermLevel)


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


char *FQDomainName(char *cInput)
{
	register int i;

	for(i=0;cInput[i];i++)
	{
//		if(!isalnum(cInput[i]) && cInput[i]!='.'  && cInput[i]!='-' )
//			break;
		if(isupper(cInput[i])) cInput[i]=tolower(cInput[i]);
	}
	cInput[i]=0;

	return(cInput);

}//char *FQDomainName(char *cInput)


char *FQDomainName2(char *cInput)
{
	register int i;

	for(i=0;cInput[i];i++)
	{
		if(!isalnum(cInput[i]) && cInput[i]!='.'  && cInput[i]!='-' )
			break;
		if(isupper(cInput[i])) cInput[i]=tolower(cInput[i]);
	}
	cInput[i]=0;

	return(cInput);

}//char *FQDomainName2(char *cInput)


void iDNSLog(unsigned uTablePK, char *cTableName, char *cLogEntry)
{
        char cQuery[512];

	//uLogType==2 is this org interface
        sprintf(cQuery,"INSERT INTO tLog SET cLabel='%.63s',uLogType=3,uPermLevel=%u,uLoginClient=%u,"
			"cLogin='%.99s',cHost='%.99s',uTablePK='%u',cTableName='%.31s',"
			"cHash=MD5(CONCAT('%s','%u','%u','%s','%s','%u','%s','%s')),uOwner=%u,"
			"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cLogEntry,
			guPermLevel,
			guLoginClient,
			gcLogin,
			gcHost,
			uTablePK,
			cTableName,
			cLogEntry,
			guPermLevel,
			guLoginClient,
			gcLogin,
			gcHost,
			uTablePK,
			cTableName,
			cLogKey,
			guOrg);

        mysql_query(&gMysql,cQuery);

}//void iDNSLog(unsigned uTablePK, char *cTableName, char *cLogEntry)


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


char *cShortenText(char *cText)
{
	//Return the first n word from cText
	//will use the spaces for word counting.
	unsigned uCount=0;
	register int i=0;
	static char cResult[100];
	
	for(i=0;i<strlen(cText);i++)
	{
		cResult[i]=cText[i];
		if(cText[i]==' ')
			uCount++;
		if(uCount>=8) break;
	}

	cResult[i]='\0';
	return(cResult);
	
}//char *cShortenText(char *cText)


const char *ForeignKey(const char *cTableName, const char *cFieldName, unsigned uKey)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
	char cQuery[512]={""};

        sprintf(cQuery,"SELECT %s FROM %s WHERE _rowid=%u",
                        cFieldName,cTableName,uKey);
        mysql_query(&gMysql,cQuery);
        if(mysql_errno(&gMysql)) return(mysql_error(&gMysql));

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
		sprintf(gcQuery,"%u",uKey);
        	return(gcQuery);
	}

}//const char *ForeignKey(const char *cTableName, const char *cFieldName, unsigned uKey)


void funcTopInfo(FILE *fp)
{
	//<font size=+1>{{cZone}} :: {{cLabel}} :: iDNS Admin Interface</font>
	
	char cOutput[512]={""};
	char cResource[100]={""};
	char cContact[100]={""};

	if(guCookieView)
		sprintf(gcView,"%.31s",ForeignKey("tView","cLabel",guCookieView));
	if(guCookieResource)
		sprintf(cResource,"%.99s",ForeignKey("tResource","cName",guCookieResource));
	if(guCookieContact)
		sprintf(cContact,"%.31s",ForeignKey("tClient","cLabel",guCookieContact));

	sprintf(cOutput,"<br><br><b>%.99s %.31s %.99s %.31s %.31s :: iDNS Admin Interface</b>",
				cResource,gcView,gcCookieZone,gcCookieCustomer,cContact);

	fprintf(fp,"%s",cOutput);

}//void funcTopInfo(FILE *fp)


void ConvertToEnglishDate(char *cDate)
{
	//This function parses the cDate argument, which is a date time string
	//as returned by the MySQL FROM_UNIXTIME() function
	unsigned uDay=0;
	unsigned uMonth=0;
	unsigned uYear=0;
	char cTime[65]={""};

	sscanf(cDate,"%u-%u-%u %s",&uYear,&uMonth,&uDay,cTime);

#ifdef ARG_DATE
	sprintf(cDate,"%02u/%02u/%u %s",uDay,uMonth,uYear,cTime);
#else
	sprintf(cDate,"%02u/%02u/%u %s",uMonth,uDay,uYear,cTime);
#endif

}//void ConvertToEnglishDate(char *cDate)


void SetSessionCookie(void)
{
	printf("Set-Cookie: iDNSSessionCookie=cCustomer=%s|cZone=%s|uView=%u|uContact=%u|uResource=%u|; Secure; HttpOnly\n",
			gcCookieCustomer,gcCookieZone,guCookieView,guCookieContact,guCookieResource);
};


void GetSessionCookie(void)
{
	char *ptr;
	char *ptr2;
	char ciDNSSessionCookie[512]={""};

	if(getenv("HTTP_COOKIE")!=NULL)
		sprintf(gcCookie,"%.1022s",getenv("HTTP_COOKIE"));
	if(gcCookie[0])
	{
		if((ptr=strstr(gcCookie,"iDNSSessionCookie=")))
		{
			ptr+=strlen("iDNSSessionCookie=");
			if((ptr2=strchr(ptr,';')))
			{
				*ptr2=0;
				sprintf(ciDNSSessionCookie,"%.511s",ptr);
			}
			else
				 sprintf(ciDNSSessionCookie,"%.511s",ptr);

		}
	}


	if(ciDNSSessionCookie[0])
	{
		if((ptr=strstr(ciDNSSessionCookie,"cCustomer=")))
		{
			ptr+=strlen("cCustomer=");
			if((ptr2=strchr(ptr,'|')))
			{
				*ptr2=0;
				sprintf(gcCookieCustomer,"%s",ptr);
				*ptr2='|';
			}
		}
		if((ptr=strstr(ciDNSSessionCookie,"cZone=")))
		{
			ptr+=strlen("cZone=");
			if((ptr2=strchr(ptr,'|')))
			{
				*ptr2=0;
				sprintf(gcCookieZone,"%s",ptr);
				*ptr2='|';
			}
		}
		if((ptr=strstr(ciDNSSessionCookie,"uView=")))
		{
			ptr+=strlen("uView=");
			if((ptr2=strchr(ptr,'|')))
			{
				*ptr2=0;
				sscanf(ptr,"%u",&guCookieView);
				*ptr2='|';
			}
		}
		if((ptr=strstr(ciDNSSessionCookie,"uContact=")))
		{
			ptr+=strlen("uContact=");
			if((ptr2=strchr(ptr,'|')))
			{
				*ptr2=0;
				sscanf(ptr,"%u",&guCookieContact);
				*ptr2='|';
			}
		}
		if((ptr=strstr(ciDNSSessionCookie,"uResource=")))
		{
			ptr+=strlen("uResource=");
			if((ptr2=strchr(ptr,'|')))
			{
				*ptr2=0;
				sscanf(ptr,"%u",&guCookieResource);
				*ptr2='|';
			}
		}
//If debug
//	printf("Content-type: text/plain\n\nciDNSSessionCookie='%s'"
//			" guCookieResource=%u guCookieContact=%u guCookieView=%u gcCookieZone=%s gcCookieCustomer=%s\n",
//				ciDNSSessionCookie,guCookieResource,guCookieContact,guCookieView,gcCookieZone,gcCookieCustomer);
//	exit(0);
	}
//	htmlPlainTextError(gcZone);
}//void GetSessionCookie(void)


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


void ExtSelect(const char *cTable,const char *cVarList,unsigned uMaxResults)
{
	if(guLoginClient==1 && guPermLevel>11)//Root can read access all
		sprintf(gcQuery,"SELECT %1$s FROM %2$s ORDER BY %2$s._rowid",
					cVarList,cTable);
	else 
		sprintf(gcQuery,"SELECT %1$s FROM %3$s," TCLIENT
				" WHERE %3$s.uOwner=" TCLIENT ".uClient"
				" AND (" TCLIENT ".uClient=%2$u OR " TCLIENT ".uOwner"
				" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%2$u OR uClient=%2$u))"
				" ORDER BY %3$s._rowid",
					cVarList,guOrg,
					cTable);
	if(uMaxResults)
	{
		char cLimit[33]={""};
		sprintf(cLimit," LIMIT %u",uMaxResults);
		strcat(gcQuery,cLimit);
	}

}//void ExtSelect(...)

