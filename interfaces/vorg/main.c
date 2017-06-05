/*
FILE 
	main.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See included LICENSE file.
PURPOSE
	vdnsOrg Interface
	program file
REQUIRES
	OpenISP libtemplates.a and templates.h
	See makefile for more information.
CURRENT WORK
	Changing template system to require set and type.
	Set is for skin changes.
	Type identifies templates by interface or other function.
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
char gcModStep[32]={""};
char gcNewStep[32]={""};
char gcDelStep[32]={""};
unsigned guZone=0;
unsigned guView=0;

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
char gcUser[100]={""};
char gcName[100]={""};
char gcOrgName[100]={""};
char gcHost[100]={""};
char gcHostname[100]={""};

char gcFunction[100]={""};
char gcPage[100]={""};
unsigned guBrowserFirefox=0;

//
//Local only
int main(int argc, char *argv[]);
int iValidLogin(int mode);
void SSLCookieLogin(void);
void SetLogin(void);
void GetPLAndClient(char *cUser);
#ifdef cLDAPURI
void GetPLAndClientLDAP(const char *cLogin,const char *cOrganization);
#endif
void htmlLogin(void);
void htmlLoginPage(char *cTitle, char *cTemplateName);



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
		SSLCookieLogin();
		
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
		}
		if(gcPage[0])
		{
			if(!strcmp(gcPage,"Resource"))
				ResourceGetHook(gentries,i);
			else if(!strcmp(gcPage,"Zone"))
				ZoneGetHook(gentries,i);
			else if(!strcmp(gcPage,"Glossary"))
				GlossaryGetHook(gentries,i);
			else if(!strcmp(gcPage,"BulkOp"))
				BulkOpGetHook(gentries,i);
		}
	}
	else
	{
		//Post
		
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
		}
	}

	//Not required to be logged in gcFunction section
	if(gcFunction[0])
	{
		if(!strncmp(gcFunction,"Logout",5))
		{
			//8 idnsOrg log type, need to globally add 9 for vdnsOrg
			printf("Set-Cookie: vdnsOrgLogin=; expires=\"Mon, 01-Jan-1971 00:10:10 GMT\"\n");
			printf("Set-Cookie: vdnsOrgPasswd=; expires=\"Mon, 01-Jan-1971 00:10:10 GMT\"\n");
			sprintf(gcQuery,"INSERT INTO tLog SET cLabel='logout %.99s',uLogType=8,uPermLevel=%u,"
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

        if(!strcmp(gcFunction,"Login")) 
		SetLogin();

        if(!guPermLevel || !gcUser[0] || !guLoginClient)
                SSLCookieLogin();

	//First page after valid login
	if(!strcmp(gcFunction,"Login"))
		htmlZone();

	//Per page command tree
	ZoneCommands(entries,i);
	ResourceCommands(entries,i);
	BulkOpCommands(entries,i);
	
	//default logged in page
	htmlZone();
	return(0);

}//end of main()


void htmlLogin(void)
{
	htmlHeader("vdnsOrg","Header");
	htmlLoginPage("vdnsOrg","ZLogin.Body");
	htmlFooter("Footer");

}//void htmlLogin(void)


void htmlLoginPage(char *cTitle, char *cTemplateName)
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
			
			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
			
			template.cpName[1]="cCGI";
			template.cpValue[1]="vdnsOrg.cgi";
			
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
	printf("Please report this vdnsOrg fatal error ASAP:\n%s\n",cError);

	//Attempt to report error in tLog
        sprintf(cQuery,"INSERT INTO tLog SET cLabel='htmlPlainTextError',uLogType=4,uPermLevel=%u,uLoginClient=%u,"
			"cLogin='%s',cHost='%s',cMessage=\"%s\",cServer='%s',uOwner=1,uCreatedBy=%u,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW())",
				guPermLevel,guLoginClient,gcLogin,gcHost,cError,gcHostname,guLoginClient);
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

		TemplateSelectInterface(cTemplateName,uPLAINSET,uVDNSORGTYPE);
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			struct t_template template;
			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
		
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
		printf("<html><head><title>%s</title></head><body bgcolor=white><font face=Arial,Helvetica",cTitle);
	}

}//void htmlHeader()


void htmlFooter(char *cTemplateName)
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
			template.cpName[0]="cIspName";
			template.cpValue[0]=ISPNAME;
			template.cpName[1]="cIspUrl";
			template.cpValue[1]=ISPURL;
			template.cpName[2]="cCopyright";
			template.cpValue[2]="&copy; 2009-2010 Unixservice, LLC. All Rights Reserved.";
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


//libtemplate.a required
void AppFunctions(FILE *fp,char *cFunction)
{
	if(!strcmp(cFunction,"funcSelectZone"))
		funcSelectZone(fp);
	else if(!strcmp(cFunction,"funcSelectBlock"))
                funcSelectBlock(fp);
	else if(!strcmp(cFunction,"funcSelectSecondary"))
                funcSelectSecondary(fp);
	else if(!strcmp(cFunction,"funcRRs"))
                funcRRs(fp);
	else if(!strcmp(cFunction,"funcSelectRRType"))
                funcSelectRRType(fp,1);
	else if(!strcmp(cFunction,"funcSelectRRTypeWiz"))
		funcSelectRRType(fp,0);
	else if(!strcmp(cFunction,"funcNSSetMembers"))
		funcNSSetMembers(fp);
	else if(!strcmp(cFunction,"funcMetaParam"))
		funcMetaParam(fp);
	//Not implemented yet
	//else if(!strcmp(cFunction,"funcZoneStatus"))
	//funcZoneStatus(fp);
	
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

	sprintf(gcQuery,"SELECT cPasswd,uPerm FROM tAuthorize WHERE cLabel='%s'",
			gcLogin);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		htmlPlainTextError(mysql_error(&gMysql));
	}
	mysqlRes=mysql_store_result(&gMysql);
	cPasswd[0]=0;
	if((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sprintf(cPasswd,"%.99s",mysqlField[0]);
		sscanf(mysqlField[1],"%u",&guPermLevel);
		//Do not allow admin users to use this end user interface
		if(guPermLevel>=10) htmlLogin();
	}
	mysql_free_result(mysqlRes);
	
	return(cPasswd);

}//char *cGetPasswd(char *gcLogin)


void SSLCookieLogin(void)
{
	char *cP,*cP2;

	//Parse out login and passwd from cookies
#ifdef SSLONLY
	if(getenv("HTTPS")==NULL) 
		htmlPlainTextError("Non SSL access denied");
#endif

	if(getenv("HTTP_COOKIE")!=NULL)
		sprintf(gcCookie,"%.1022s",getenv("HTTP_COOKIE"));
	
	if(gcCookie[0])
	{

		if((cP=strstr(gcCookie,"vdnsOrgLogin=")))
		{
			cP+=strlen("vdnsOrgLogin=");
			if((cP2=strchr(cP,';')))
			{
				*cP2=0;
				sprintf(gcLogin,"%.99s",cP);
				*cP2=';';
			}
			else
			{
				sprintf(gcLogin,"%.99s",cP);
			}
		}
		if((cP=strstr(gcCookie,"vdnsOrgPasswd=")))
		{
			cP+=strlen("vdnsOrgPasswd=");
			if((cP2=strchr(cP,';')))
			{
				*cP2=0;
				sprintf(gcPasswd,"%.99s",cP);
				*cP2=';';
			}
			else
			{
				sprintf(gcPasswd,"%.99s",cP);
			}
		}
	
	}//if gcCookie[0] time saver

	//First try tClient/tAuthorize system
	if(!iValidLogin(1))
	{
#ifdef cLDAPURI
		//Then LDAP system
		if(!iValidLDAPLogin(gcLogin,gcPasswd,gcOrgName))
		{
			htmlLogin();
		}
		else
		{
			sprintf(gcUser,"%.41s",gcLogin);
			GetPLAndClientLDAP(gcUser,gcOrgName);
		}
#else
		htmlLogin();
#endif
	}
	else
	{
		sprintf(gcUser,"%.41s",gcLogin);
		GetPLAndClient(gcUser);
	}

	//Don't allow Root login at this interface (Ticket #81)
	if(!guPermLevel || !guLoginClient || guPermLevel>=10 || guLoginClient==1)
	{
        	guPermLevel=0;
		gcUser[0]=0;
		guLoginClient=0;
		htmlLogin();
	}
	
        //if(!strcmp(gcFunction,"Login"))
	//	SetLogin();
	gcPasswd[0]=0;
	guSSLCookieLogin=1;

}//SSLCookieLogin()


void GetPLAndClient(char *cUser)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;

	sprintf(gcQuery,"SELECT tAuthorize.uPerm,tAuthorize.uCertClient,tClient.cLabel,tClient.uOwner FROM"
			" tAuthorize,tClient WHERE tAuthorize.uCertClient=tClient.uClient AND tAuthorize.cLabel='%s'",
				cUser);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	mysqlRes=mysql_store_result(&gMysql);
	if((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[0],"%d",&guPermLevel);
		sscanf(mysqlField[1],"%u",&guLoginClient);
		sprintf(gcName,"%.100s",mysqlField[2]);
		sscanf(mysqlField[3],"%u",&guOrg);
	}
	mysql_free_result(mysqlRes);

	if(guOrg==1)
		sprintf(gcOrgName,"ASP Provider");
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

}//void GetPLAndClient()


#ifdef cLDAPURI
void GetPLAndClientLDAP(const char *cLogin,const char *cOrganization)
{
        MYSQL_RES *mysqlRes;
        MYSQL_RES *mysqlRes2;
        MYSQL_ROW mysqlField;
        MYSQL_ROW mysqlField2;

	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s'",cOrganization);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	mysqlRes=mysql_store_result(&gMysql);
	if((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[0],"%u",&guOrg);
		guLoginClient=guOrg;
		sprintf(gcName,"%.100s",cLogin);
		//Fixed LDAP perm level. Could be extended via optional LDAP attr
		guPermLevel=8;

		//Also add this LDAP cLogin to cOriganization if does already exist
		sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s'",cLogin);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		mysqlRes2=mysql_store_result(&gMysql);
		if((mysqlField2=mysql_fetch_row(mysqlRes2)))
		{
			sscanf(mysqlField2[0],"%u",&guLoginClient);
		}
		else
		{
			//Could extend this to add cInfo from optional LDAP attr. Same for cEmail
			sprintf(gcQuery,"INSERT INTO tClient SET cLabel='%s',cInfo='LDAP',cCode='Contact',"
				"uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					cLogin,guOrg);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
			guLoginClient=mysql_insert_id(&gMysql);
		}
		mysql_free_result(mysqlRes2);
	}
	mysql_free_result(mysqlRes);

}//void GetPLAndClientLDAP()
#endif

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
		sprintf(gcQuery,"INSERT INTO tLog SET cLabel='login failed %.99s',uLogType=8,uPermLevel=%u,uLoginClient=%u,"
		"cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname);
		mysql_query(&gMysql,gcQuery);
	}
	return 0;

}//iValidLogin()


void SetLogin(void)
{
	if(iValidLogin(0))
	{
		printf("Set-Cookie: vdnsOrgLogin=%s;\n",gcLogin);
		printf("Set-Cookie: vdnsOrgPasswd=%s;\n",gcPasswd);
		sprintf(gcUser,"%.41s",gcLogin);
		GetPLAndClient(gcUser);
		guSSLCookieLogin=1;
		//tLogType.cLabel='org login'->uLogType=8
		sprintf(gcQuery,"INSERT INTO tLog SET cLabel='login ok %.99s',uLogType=8,uPermLevel=%u,uLoginClient=%u,"
				"cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=%u,uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())",
				gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname,guOrg);
		mysql_query(&gMysql,gcQuery);
	}
#ifdef cLDAPURI
	else if(iValidLDAPLogin(gcLogin,gcPasswd,gcOrgName))
	{
		printf("Set-Cookie: vdnsOrgLogin=%s;\n",gcLogin);
		printf("Set-Cookie: vdnsOrgPasswd=%s;\n",gcPasswd);
		sprintf(gcUser,"%.41s",gcLogin);
		GetPLAndClientLDAP(gcUser,gcOrgName);
		guSSLCookieLogin=1;
		//tLogType.cLabel='org login'->uLogType=8
		sprintf(gcQuery,"INSERT INTO tLog SET cLabel='login ok %.99s',uLogType=8,uPermLevel=%u,uLoginClient=%u,"
				"cLogin='%.99s',cHost='%.99s',cServer='%.99s',uOwner=%u,uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())",
				gcLogin,guPermLevel,guLoginClient,gcLogin,gcHost,gcHostname,guOrg);
		mysql_query(&gMysql,gcQuery);
	}
#endif
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
//This check is performed in resource.c, would case a malfunctioning if not commented here
//		if(!isalnum(cInput[i]) && cInput[i]!='.'  && cInput[i]!='-' )
//			break;
		if(isupper(cInput[i])) cInput[i]=tolower(cInput[i]);
	}
	cInput[i]=0;

	return(cInput);

}//char *FQDomainName(char *cInput)


void iDNSLog(unsigned uTablePK, char *cTableName, char *cLogEntry)
{
        char cQuery[512];

	//uLogType==2 is this org interface
        sprintf(cQuery,"INSERT INTO tLog SET cLabel='%.63s',uLogType=2,uPermLevel=%u,uLoginClient=%u,cLogin='%.99s',"
			"cHost='%.99s',uTablePK=%u,cTableName='%.31s',"
			"cHash=MD5(CONCAT('%s','%u','%u','%s','%s','%u','%s','%s')),uOwner=1,uCreatedBy=1,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW())",
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
			cLogKey);

        mysql_query(&gMysql,cQuery);

}//void iDNSLog(unsigned uTablePK, char *cTableName, char *cLogEntry)


void fpTemplate(FILE *fp,char *cTemplateName,struct t_template *template)
{
	if(cTemplateName[0])
	{	
        	MYSQL_RES *res;
	        MYSQL_ROW field;

		TemplateSelectInterface(cTemplateName,uPLAINSET,uVDNSORGTYPE);
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
