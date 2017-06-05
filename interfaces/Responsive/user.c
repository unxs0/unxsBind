/*
FILE 
	user.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2010, 2011 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See included LICENSE file.
PURPOSE
	unxsDNS program file.
	User tab functions.
WORK NOTES
	Changing session info and removing get link data input.
*/

#include "interface.h"

extern unsigned guBrowserFirefox;//main.c
extern char gcZone[];
static char cCurrentPasswd[32]={""};
static char cPasswd[32]={""};
static char cPasswd2[32]={""};
char gcInCollapseFive[16]={"out"};

//TOC
void ProcessUserVars(pentry entries[], int x);
void UserGetHook(entry gentries[],int x);
unsigned uNoUpper(const char *cPasswd);
unsigned uNoDigit(const char *cPasswd);
unsigned uNoLower(const char *cPasswd);
unsigned uChangePassword(const char *cPasswd);
void EncryptPasswdWithSalt(char *pw, char *salt);
char *cGetPasswd(char *gcLogin);
extern unsigned guZone;
unsigned uValidPasswd(char *cPasswd,unsigned guLoginClient,char *cUser);
//extern
void unxsvzLog(unsigned uTablePK,char *cTableName,char *cLogEntry,unsigned guPermLevel,unsigned guLoginClient,char *gcLogin,char *gcHost);
void SetZoneFromSearchSet(void);
char *cGetZonename(unsigned uZone);
void htmlOperationsInfo(void);
void htmlLoginInfo(void);
void GetZoneFromSessionGroup(void);//zone.c

void ProcessUserVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cCurrentPasswd"))
			sprintf(cCurrentPasswd,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cPasswd"))
			sprintf(cPasswd,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cPasswd2"))
			sprintf(cPasswd2,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"gcZone"))
			sprintf(gcZone,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"guZone"))
			sscanf(entries[i].val,"%u",&guZone);
	}

}//void ProcessUserVars(pentry entries[], int x)


void UserGetHook(entry gentries[],int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"guZone"))
			sscanf(gentries[i].val,"%u",&guZone);
	}

	if(!strcmp(gcFunction,"OperationsInfo"))
		htmlOperationsInfo();
	if(!strcmp(gcFunction,"LoginInfo"))
		htmlLoginInfo();

	htmlUser();

}//void UserGetHook(entry gentries[],int x)


unsigned uNoUpper(const char *cPasswd)
{
	register int i;
	for(i=0;cPasswd[i];i++)
		if(isupper(cPasswd[i])) return(0);
	return(1);
}//unsigned uNoUpper(const char *cPasswd)


unsigned uNoLower(const char *cPasswd)
{
	register int i;
	for(i=0;cPasswd[i];i++)
		if(islower(cPasswd[i])) return(0);
	return(1);
}//unsigned uNoLower(const char *cPasswd)


unsigned uNoDigit(const char *cPasswd)
{
	register int i;
	for(i=0;cPasswd[i];i++)
		if(isdigit(cPasswd[i])) return(0);
	return(1);
}//unsigned uNoDigit(const char *cPasswd)


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


unsigned uChangePassword(const char *cPasswd)
{
	char cBuffer[100]={""};
	char cSalt[16]={"$1$23abc123$"};//TODO set to random salt;
	(void)srand((int)time((time_t *)NULL));
	to64(&cSalt[3],rand(),8);

	sprintf(cBuffer,"%.99s",cPasswd);
	EncryptPasswdWithSalt(cBuffer,cSalt);

	sprintf(gcQuery,"UPDATE tAuthorize SET cPasswd='%.99s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW())"
			" WHERE cLabel='%s' AND uCertClient=%u",
			cBuffer,guLoginClient,gcLogin,guLoginClient);
	mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
		return(1);
	if(mysql_affected_rows(&gMysql)<1)
		return(1);
	unxsvzLog(guLoginClient,"tClient","Password Changed",guPermLevel,guLoginClient,gcLogin,gcHost);
	return(0);
}//unsigned uChangePassword(const char *cPasswd)


void UserCommands(pentry entries[], int x)
{
	if(!strcmp(gcPage,"User"))
	{
		ProcessUserVars(entries,x);
		if(!strcmp(gcFunction,"Change Password"))
		{
			sprintf(gcInCollapseFive,"in");
			if(!cCurrentPasswd[0] || !gcUser[0])
			{
				gcMessage="Must enter 'Current Password'";
				htmlUser();
			}
			if(!uValidPasswd(cCurrentPasswd,guLoginClient,gcUser))
			{
				gcMessage="Invalid current password entered";
				htmlUser();
			}
			if(!cPasswd[0] || !cPasswd2[0] || strcmp(cPasswd,cPasswd2))
			{
				gcMessage="Must enter new 'Password' twice and they must match";
				htmlUser();
			}
			if(!strcmp(cCurrentPasswd,cPasswd))
			{
				gcMessage="New 'Password' is the same as current password";
				htmlUser();
			}
			if(strlen(cPasswd)<6)
			{
				gcMessage="New 'Password' must be at least 6 chars long";
				htmlUser();
			}
			if(strstr(cPasswd,gcLogin) || strstr(gcLogin,cPasswd) ||
				strstr(cPasswd,gcName) || strstr(gcName,cPasswd) ||
				strstr(gcOrgName,cPasswd) || strstr(cPasswd,gcOrgName))
			{
				gcMessage="New 'Password' must not be related to your login or company";
				htmlUser();
			}
			if(uNoUpper(cPasswd) || uNoLower(cPasswd) || uNoDigit(cPasswd))
			{
				gcMessage="New 'Password' must have some upper and lower case letters,"
						" and at least one number";
				htmlUser();
			}
			if(uChangePassword(cPasswd))
			{
				gcMessage="Password not changed contact system admin";
				htmlUser();
			}
			gcMessage="Password changed you will need to login again";
			htmlUser();
		}
	}

}//void UserCommands(pentry entries[], int x)


void htmlUser(void)
{
	//zone search memory
	GetZoneFromSessionGroup();

	htmlHeader("unxsDNS","UserHeader");
	htmlUserPage("unxsDNS","User.Body");
	htmlFooter("UserFooter");

}//void htmlUser(void)


void htmlUserPage(char *cTitle, char *cTemplateName)
{
	if(cTemplateName[0])
	{
        	MYSQL_RES *res;
	        MYSQL_ROW field;

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
			if(guZone)
				sprintf(gcZone,"%.99s",(char *)cGetZonename(guZone));
			template.cpValue[8]=gcZone;

			char cguZone[16];
			sprintf(cguZone,"%u",guZone);
			template.cpName[9]="guZone";
			template.cpValue[9]=cguZone;

			template.cpName[10]="gcBrand";
			template.cpValue[10]=INTERFACE_HEADER_TITLE;

			template.cpName[11]="gcCopyright";
			template.cpValue[11]=LOCALCOPYRIGHT;

			char cPrivilegedZoneMenu[256]={""};
			template.cpName[12]="cPrivilegedZoneMenu";
			//if(guPermLevel>=6)
			//	sprintf(cPrivilegedZoneMenu,
			//		"<li><a href=\"%1$.32s?gcPage=Repurpose&guZone=%2$u\">Repurpose</a></li>"
			//		"<li><a href=\"%1$.32s?gcPage=Reseller&guZone=%2$u\">Reseller</a></li>"
			//			,template.cpValue[1],guZone);
			template.cpValue[12]=cPrivilegedZoneMenu;

			template.cpName[13]="gcInCollapseFive";//Change passwd div
			template.cpValue[13]=gcInCollapseFive;

			template.cpName[14]="";

			printf("\n<!-- Start htmlUserPage(%s) -->\n",cTemplateName); 
			Template(field[0],&template,stdout);
			printf("\n<!-- End htmlUserPage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlUserPage()


void funcMOTD(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	fprintf(fp,"<!-- funcMOTD(fp) Start -->\n");

	sprintf(gcQuery,"SELECT cComment FROM tConfiguration WHERE cLabel='cOrg_NewMOTD' LIMIT 1");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		fprintf(fp,"%s",field[0]);
	mysql_free_result(res);

	fprintf(fp,"<!-- funcMOTD(fp) End -->\n");

}//void funcMOTD(FILE *fp)


unsigned uValidPasswd(char *cPasswd,unsigned guLoginClient,char *cUser)
{
	char cSalt[16]={""};
	char cPassword[100]={""};
	char cPasswdCopy[100]={""};
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT cPasswd FROM tAuthorize WHERE uCertClient=%u AND cLabel='%s'",guLoginClient,cUser);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		return(0);
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sprintf(cPassword,"%.99s",field[0]);
	mysql_free_result(res);

	sprintf(cPasswdCopy,"%.99s",cPasswd);
	if(cPassword[0])
	{
		//gcMessage=cPassword;
		//MD5 vs DES salt determination
		if(cPassword[0]=='$' && cPassword[2]=='$')
			sprintf(cSalt,"%.12s",cPassword);
		else
			sprintf(cSalt,"%.2s",cPassword);
		EncryptPasswdWithSalt(cPasswdCopy,cSalt);
		if(!strcmp(cPasswdCopy,cPassword))
			return(1);
		else
			return(0);
	}
	return(0);
}//unsigned uValidPasswd()


void htmlOperationsInfo(void)
{

	htmlHeader("unxsDNS","UserHeader");

        MYSQL_RES *res;
	MYSQL_ROW field;
	if(guPermLevel>9)
		sprintf(gcQuery,"SELECT cLabel,FROM_UNIXTIME(uCreatedDate),cHost,cServer FROM tLog WHERE uLogType!=8 AND"
			" (uCreatedBy=%u OR uLoginClient=%u OR uOwner=%u) ORDER BY uCreatedDate DESC LIMIT 50",guLoginClient,guLoginClient,guLoginClient);
	else
		sprintf(gcQuery,"SELECT cLabel,FROM_UNIXTIME(uCreatedDate),cHost,cServer FROM tLog WHERE uLogType!=8 AND"
			" uCreatedBy=%u ORDER BY uCreatedDate DESC LIMIT 50",guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("mysql error<br>");
		return;
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
		printf("%s %s %s %s<br>",field[1],field[0],field[2],field[3]);
	mysql_free_result(res);


	exit(0);

}//void htmlOperationsInfo(void)


void htmlLoginInfo(void)
{

	htmlHeader("unxsDNS","UserHeader");

        MYSQL_RES *res;
	MYSQL_ROW field;
	if(guPermLevel>10)
		sprintf(gcQuery,"SELECT cLabel,FROM_UNIXTIME(uCreatedDate),cHost,cServer FROM tLog WHERE"
			" uLogType=8 ORDER BY uCreatedDate DESC LIMIT 50");
	else
		sprintf(gcQuery,"SELECT cLabel,FROM_UNIXTIME(uCreatedDate),cHost,cServer FROM tLog WHERE uLoginClient=%u"
			" AND uLogType=8 ORDER BY uCreatedDate DESC LIMIT 50",guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("mysql error<br>");
		return;
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
		printf("%s %s %s %s<br>",field[1],field[0],field[2],field[3]);
	mysql_free_result(res);


	exit(0);

}//void htmlLoginInfo(void)
