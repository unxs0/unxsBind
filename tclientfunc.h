/*
FILE
	svn ID removed
PURPOSE
	Non-schema dependent tclient.c expansion.
AUTHOR
	GPL License applies, see www.fsf.org for details
	See LICENSE file in this distribution
	(C) 2001-20010 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
*/

#define BO_CUSTOMER	"Back-Office Customer"
#define BO_RESELLER	"Back-Office Reseller"
#define BO_ADMIN 	"Back-Office Admin"
#define BO_ROOT 	"Back-Office Root"
#define ORG_CUSTOMER	"Organization Customer"
#define ORG_WEBMASTER	"Organization Webmaster"
#define ORG_SALES	"Organization Sales Force"
#define ORG_SERVICE	"Organization Customer Service"
#define ORG_ACCT	"Organization Bookkeeper"
#define ORG_ADMIN	"Organization Admin"

#define BO_ROOT_VAL		12
#define BO_ADMIN_VAL		10
#define BO_RESELLER_VAL		8	
#define BO_CUSTOMER_VAL		7
#define ORG_CUSTOMER_VAL	1
#define ORG_WEBMASTER_VAL	2
#define ORG_SALES_VAL		3
#define ORG_SERVICE_VAL		4
#define ORG_ACCT_VAL		5
#define ORG_ADMIN_VAL		6

static char cPasswd[36]={""};
static char cLogin[33]={""};
static char cExtLabel[16]={""};
static char cuPerm[33]={ORG_ADMIN};
static unsigned uPerm=0;
static unsigned uOnlyASPs=0;
static char cSearch[100]={""};

//Aux drop/pull downs
static char cForClientPullDown[256]={"---"};
static unsigned uForClient=0;


unsigned IsAuthUser(char *cLabel, unsigned uOwner, unsigned uCertClient);
void PermLevelDropDown(char *cuPerm);

void EncryptPasswdWithSalt(char *cPasswd,char *cSalt);
void EncryptPasswd(char *cPasswd);//main.c
void GetClientMaxParams(unsigned uClient,unsigned *uMaxSites,unsigned *uMaxIPs);
const char *cUserLevel(unsigned uPermLevel);
unsigned uMaxClientsReached(unsigned uClient);
void tTablePullDownResellers(unsigned uSelector,unsigned uMode);
void ContactsNavList(void);
void htmlRecordContext(void);
void tClientNavList(void);
void BasictClientCheck(void);
void htmlAuthorizeLinksFromClient(unsigned uCertClient);
void tAuthorizeNavListForLoadedOwner(void);
void tAuthorizeLink(void);


void ExtProcesstClientVars(pentry entries[], int x)
{

	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cPasswd"))
			sprintf(cPasswd,"%.35s",entries[i].val);
		else if(!strcmp(entries[i].name,"cLogin"))
			sprintf(cLogin,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cExtLabel"))
			sprintf(cExtLabel,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cuPerm"))
		{
			sprintf(cuPerm,"%.32s",entries[i].val);
			sprintf(cuPerm,"%.32s",entries[i].val);
			if(!strcmp(cuPerm,ORG_ADMIN))
				uPerm=ORG_ADMIN_VAL;
			else if(!strcmp(cuPerm,BO_ADMIN))
				uPerm=BO_ADMIN_VAL;
			else if(!strcmp(cuPerm,BO_ROOT))
				uPerm=BO_ROOT_VAL;
			else if(!strcmp(cuPerm,BO_CUSTOMER))
				uPerm=BO_CUSTOMER_VAL;
			else if(!strcmp(cuPerm,BO_RESELLER))
				uPerm=BO_RESELLER_VAL;
		}
		else if(!strcmp(entries[i].name,"cForClientPullDown"))
		{
			strcpy(cForClientPullDown,entries[i].val);
			uForClient=ReadPullDown(TCLIENT,"cLabel",
					cForClientPullDown);
		}
		else if(!strcmp(entries[i].name,"cSearch"))
			sprintf(cSearch,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"uOnlyASPs"))
			uOnlyASPs=1;
	}

}//void ExtProcesstClientVars(pentry entries[], int x)


void ExttClientCommands(pentry entries[], int x)
{
        MYSQL_RES *res;

	if(!strcmp(gcFunction,"tClientTools"))
	{
		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=10)
			{
				ProcesstClientVars(entries,x);
				if(guLoginClient!=1 && uMaxClientsReached(guCompany))
				{
					guMode=0;
					tClient("Your reseller maximum of customers has been reached");
				}
				guMode=2000;
				//These just for GUI cleanup
				cCode[0]=0;
				uModDate=0;
				uModBy=0;
				tClient(LANG_NB_CONFIRMNEW);
			}
			else
				tClient("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=10)
			{
				ProcesstClientVars(entries,x);

				//Validate
				guMode=2000;
				BasictClientCheck();
				if(guLoginClient!=1 && uMaxClientsReached(guCompany))
				{
					guMode=0;
					tClient("Your maximum of customers has been reached");
				}
				if(strlen(cLabel)<3)
					tClient("<blink>Error</blink>: Invalid cLabel!");
				guMode=0;

				if(!uForClient)
				{
					uOwner=guCompany;
					sprintf(cCode,"Organization");
				}
				else
				{
					uOwner=uForClient;
					sprintf(cCode,"Contact");
				}
				uClient=0;//Update .c this is dumb
				uCreatedBy=guLoginClient;
				//These just for GUI cleanup
				uModDate=0;
				uModBy=0;
				NewtClient(0);
			}
			else
				tClient("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstClientVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy) && (guPermLevel>10 || uCreatedBy==guLoginClient))
			{
                        	guMode=2001;
                        	tClient(LANG_NB_CONFIRMDEL);
			}
			else
				tClient("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstClientVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy) && (guPermLevel>10 || uCreatedBy==guLoginClient))
			{
                        	guMode=2001;
				//This must be customized
				sprintf(gcQuery,"SELECT uZone FROM tZone WHERE uOwner=%u OR uCreatedBy=%u",uClient,uClient);
				mysql_query(&gMysql,gcQuery);
        			if(mysql_errno(&gMysql))
                			tClient(mysql_error(&gMysql));
        			res=mysql_store_result(&gMysql);
				if(mysql_num_rows(res))
					tClient("Can't delete client with resources");
				mysql_free_result(res);
				
				if(!strcmp(cCode,"Contact") || !strncmp(cCode,"CONT",4))
				{
					sprintf(gcQuery,"DELETE FROM " TAUTHORIZE 
						" WHERE (cLabel='%s' OR uCertClient=%u)",
						cLabel,uClient);
					mysql_query(&gMysql,gcQuery);
        				if(mysql_errno(&gMysql))
                				tClient(mysql_error(&gMysql));
				}
				else if(!strcmp(cCode,"Organization") || !strncmp(cCode,"COMP",4))
				{
					sprintf(gcQuery,"DELETE FROM " TCLIENT 
						" WHERE uOwner=%u",uClient);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						tClient(mysql_error(&gMysql));
					sprintf(gcQuery,"DELETE FROM " TAUTHORIZE
						" WHERE uOwner=%u",uClient);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						tClient(mysql_error(&gMysql));
				}
                        	guMode=5;
                        	DeletetClient();
			}
			else
				tClient("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstClientVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy) && (guPermLevel>10 || uClient==guLoginClient || uCreatedBy==guLoginClient))
			{
				guMode=2002;
				tClient(LANG_NB_CONFIRMMOD);
			}
			else
				tClient("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstClientVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy) && (guPermLevel>10 || uClient==guLoginClient || uCreatedBy==guLoginClient))
			{
				//Validate
				guMode=2002;
				if(strlen(cLabel)<3)
					tClient("<blink>Error</blink>: Invalid cLabel!");
				guMode=0;

				uModBy=guLoginClient;
				if(uForClient && uClient>1)
				{
					sprintf(gcQuery,"UPDATE " TCLIENT " SET uOwner=%u WHERE uClient=%u",
						uForClient,uClient);
					mysql_query(&gMysql,gcQuery);
        				if(mysql_errno(&gMysql))
                				htmlPlainTextError(mysql_error(&gMysql));
					uOwner=uForClient;
				}
				ModtClient();
			}
			else
				tClient("<blink>Error</blink>: Denied by permissions settings");
                }

                else if(!strcmp(gcCommand,"Authorize"))
                {
                        ProcesstClientVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=3000;
				tClient("Enter login, passwd, user level and confirm."
					" To authorize a new user to use an interface or this back-office.");
			}
			else
				tClient("<blink>Error</blink>: Denied by permissions settings");
		}
                else if(!strcmp(gcCommand,"Confirm Authorize"))
                {
                        ProcesstClientVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				time_t clock;
				char cClrPasswd[33]={""};

			        time(&clock);
				
				if(strlen(cLogin)<4)
				{
					guMode=3000;
					tClient("Login must be at least 4 chars!");
				}

				if(strlen(cPasswd)<6)
				{
					guMode=3000;
					tClient("Passwd must be at least 6 chars!");
				}

				if(uPerm<1 || uPerm>12)
				{
					guMode=3000;
					sprintf(gcQuery,"uPerm level error:%u",uPerm);
					tClient(gcQuery);
				}
		
				//sprintf(cClrPasswd,"%.32s",cPasswd);
				EncryptPasswd(cPasswd);
				if(uPerm==12) uClient=1;//uCertClient root alias temp hack
				sprintf(gcQuery,"INSERT INTO " TAUTHORIZE " SET cLabel='%s',uPerm=%u,"
					"uCertClient=%u,cPasswd='%s',uOwner=%u,uCreatedBy=%u,"
					"uCreatedDate=UNIX_TIMESTAMP(NOW()),cIPMask='0.0.0.0',cClrPasswd='%s'",
						cLogin,uPerm,uClient,cPasswd,uOwner,guLoginClient,cClrPasswd);

				mysql_query(&gMysql,gcQuery);
        			if(mysql_errno(&gMysql))
				{
					printf("Content-type: text/plain\n\n");
                			printf("mysql_error: %s\n",mysql_error(&gMysql));
					exit(0);
				}
				tClient("Contact Authorized");
			}
			else
				tClient("<blink>Error</blink>: Denied by permissions settings");

		}//Confirm auth

	}

}//void ExttClientCommands(pentry entries[], int x)


void ExttClientButtons(void)
{
	unsigned uDefault=0;

	OpenFieldSet("tClient Aux Panel",100);
	 
	switch(guMode)
        {
                case 2000:
			printf("<u>New: Step 1 Tips</u><br>");
			printf("Here you would usually enter a new company name into cLabel. Optionally some standardized company info in cInfo,"
				" like addresses phone numbers and such. A main company email is usually helpful, cCode is used internally."
				" <br>If you are creating a contact for an existing company select that company from the drop down select"
				" below and use cLabel for the contact name (Ex. Anne Flechter) and the cInfo would be the contacts personal"
				" phone numbers and or address etc.");
			if(guPermLevel>7)
			{
				if(uOwner==1)
					tTablePullDownResellers(uClient,0);
				else
					tTablePullDownResellers(uOwner,0);
			}
                        printf(LANG_NBB_CONFIRMNEW);
			printf("<br>\n");
                break;

                case 2001:
                        printf(LANG_NBB_CONFIRMDEL);
			printf("<br>Note: Will also delete tAuthorize entries related to this uClient. Probably not a good idea"
				" to delete Root owned tClient records this way, even if you have the permissions to do so.\n");
                break;

                case 2002:
			printf("<u>Modify: Step 1 Tips</u><br>");
			printf("Here you can modify the contact or company name. In the later case still keeping all associated contacts."
				" You can update the cInfo text area. Add an email or company or contact code.<br>A much more advanced"
				" operation and one that must be done with care is the use of the 'Change or Create' drop down select:"
				" It is meant primarily to associate or change the association of contacts or other orphan tClient records"
				" with a given company. In any case the 'Change or Create' feature must be used with caution since it may"
				" affect many other tClient and tAuthorize records indirectly, especially if used on a company record.");

			htmlRecordContext();
			if(guPermLevel>7)
			{
				tTablePullDownResellers(uForClient,0);
			}
                        printf("<p>");
                        printf(LANG_NBB_CONFIRMMOD);
                break;

                case 3000:
			printf("<u>Authorize: Step 1 Tips</u><br>Depending on the user level you may authorize a contact to access interfaces"
				" (like the organization/contact portal.) Or even to use this back-office. In the 'Login' you would enter a"
				" login (that can be the same as the tClient.cLabel) for this contact and a password. The most common user"
				" permission level is 'Organization Admin' that would allow this contact to login to the idnsOrg.cgi interface"
				" and have full control over the companies DNS resource records. The second most common user level is"
				" 'Back-Office Root' that will allow the user full access to this back-office iDNS.cgi interface.<p>\n");
			if(guPermLevel>7)
				PermLevelDropDown(cuPerm);
			printf("<br>Login <input type=text title='Login to use' name=cLogin value='%s'"
				" size=20 maxlength=32>\n",cLabel);
			printf("<br>Passwd <input type=text title='Login passwd' name=cPasswd size=20 maxlength=15>");
			printf("<input class=largeButton title='Confirm authorization' type=submit name=gcCommand value='Confirm Authorize'>");
                break;

		default:
			if(guPermLevel>9)
			{
				uDefault=1;
				printf("<u>Table Tips</u><br>");
				printf("Here you can create new companies (organizations in general) or add new contacts"
					" to existing companies. Once a new contact is made you can authorize her to use"
					" interfaces or even this backoffice web interface."
					" The system wide hierachical model used is: A single root user, under this root"
					" user application service provider companies (ASPs) exist, at least one ASP needs"
					" to be created. Under (or as part of) the ASP at least one admin level contact must"
					" be created, this contact can then create or reseller companies or end user"
					" companies. The reseller companies may in turn create other end user companies"
					" that they can manage. End user companies only manage their own resources, via"
					" their contacts. Finally the contacts are assigned a role that limits the"
					" operations they can perform for their company and the interfaces they can use.");
				printf("<p><u>Search Tools</u><br>");
				printf("Enter the complete or the first part of a company or contact name below."
					" Not case sensitive. You can use %% and _ SQL LIKE matching chars. The check box further limits your search.<br>");
				printf("<input type=text title='cLabel search. Use %% . and _ for pattern matching.'"
					" name=cSearch value=\"%s\" maxlength=99 size=20><br>",cSearch);
				printf("Only ASPs <input title='Limit search to Root owned tClient records that in this model"
					" are the controlling ASP companies' type=checkbox name=uOnlyASPs ");
				if(uOnlyASPs)
					printf("checked><br>");
				else
					printf("><br>");
				tClientNavList();
				htmlRecordContext();
			}

			//COMP if from an old customer fork e.g. COMP12345
			if(strcmp(cCode,"Organization") && strncmp(cCode,"COMP",4) && uClient)
				htmlAuthorizeLinksFromClient(uClient);

			if( strcmp(cCode,"Organization") && strncmp(cCode,"COMP",4) && uClient && guPermLevel>9 && uClient!=guLoginClient 
				&& !IsAuthUser(cLabel,uOwner,uClient) &&guMode!=5 && uOwner!=1)
				printf("<p><input class=largeButton title='Authorize %s to manage his company resources'"
					" type=submit name=gcCommand value='Authorize'>",cLabel);

			ContactsNavList();

	}

	if(!uDefault)
	{
		if(cSearch[0])
			printf("<input type=hidden name=cSearch value=\"%s\">",cSearch);
		if(uOnlyASPs)
			printf("<input type=hidden name=uOnlyASPs value=1>");
	}
	CloseFieldSet();


}//void ExttClientButtons(void)


void ExttClientAuxTable(void)
{

}//void ExttClientAuxTable(void)


void ExttClientGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uClient"))
		{
			sscanf(gentries[i].val,"%u",&uClient);
			guMode=6;
		}
		else if(!strcmp(gentries[i].name,"uOnlyASPs"))
		{
			sscanf(gentries[i].val,"%u",&uOnlyASPs);
		}
		else if(!strcmp(gentries[i].name,"cSearch"))
		{
			sprintf(cSearch,"%.99s",gentries[i].val);
		}
	}
	tClient("");

}//void ExttClientGetHook(entry gentries[], int x)


void ExttClientSelect(void)
{
        if(cSearch[0])
        {
                if(uOnlyASPs)
                {
                        if(guLoginClient==1 && guPermLevel>11)//Root can read access all
                                sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM " TCLIENT " WHERE tClient.cLabel LIKE '%s%%'"
                                                " AND tClient.uOwner=1 ORDER BY cLabel",cSearch);
                        else
                                sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM " TCLIENT
                                        " WHERE (uClient=%1$u OR uOwner"
                                        " IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%1$u OR uClient=%1$u))"
                                        " AND tClient.cLabel LIKE '%2$s%%' AND tClient.uOwner=1"
                                        " ORDER BY cLabel",guCompany,cSearch);
                }
                else
                {
                        if(guLoginClient==1 && guPermLevel>11)//Root can read access all
                                sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM " TCLIENT " WHERE tClient.cLabel LIKE '%s%%'"
                                                " ORDER BY cLabel",cSearch);
                        else
                                sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM " TCLIENT
                                        " WHERE (uClient=%1$u OR uOwner"
                                        " IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%1$u OR uClient=%1$u))"
                                        " AND tClient.cLabel LIKE '%2$s%%'"
                                        " ORDER BY cLabel",guCompany,cSearch);
                }
        }
        else if(!cSearch[0] && uOnlyASPs)
        {
                if(guLoginClient==1 && guPermLevel>11)//Root can read access all
                        sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM " TCLIENT " WHERE"
                                        " tClient.uOwner=1 ORDER BY cLabel");
                        else
                                sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM " TCLIENT
                                        " WHERE (uClient=%1$u OR uOwner"
                                        " IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%1$u OR uClient=%1$u))"
                                        " AND tClient.uOwner=1"
                                        " ORDER BY cLabel",guCompany);
        }
        else if(1)
        {
                if(guLoginClient==1 && guPermLevel>11)//Root can read access all
                        sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM " TCLIENT " ORDER BY uClient");
                else
                        sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM " TCLIENT
                                        " WHERE (uClient=%1$u OR uOwner"
                                        " IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%1$u OR uClient=%1$u))"
                                        " ORDER BY uClient",guCompany);
        }

}//void ExttClientSelect(void)


void ExttClientSelectRow(void)
{
	ExtSelectRow("tClient",VAR_LIST_tClient,uClient);
	if(guLoginClient==1 && guPermLevel>11)//Root can read access all
		sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM tClient WHERE uClient=%u ORDER BY uClient",
			uClient);
	else 
		sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM " TCLIENT
				" WHERE uClient=%2$u AND (uClient=%1$u OR uOwner"
				" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%1$u OR uClient=%1$u))"
				" ORDER BY uClient",guCompany,uClient);

}//void ExttClientSelectRow(void)


void ExttClientListSelect(void)
{
	char cCat[512];

	ExtListSelect("tClient",VAR_LIST_tClient);
	if(guLoginClient==1 && guPermLevel>11)//Root can read access all
		sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM " TCLIENT);
	else 
		sprintf(gcQuery,"SELECT " VAR_LIST_tClient " FROM " TCLIENT
				" WHERE (uClient=%1$u OR uOwner"
				" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%1$u OR uClient=%1$u))"
								,guCompany);

	//Changes here must be reflected below in ExttClientListFilter()
        if(!strcmp(gcFilter,"uClient"))
        {
                sscanf(gcCommand,"%u",&uClient);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"uClient=%u ORDER BY uClient",
						uClient);
		strcat(gcQuery,cCat);
        }
	else if(!strcmp(gcFilter,"cLabel"))
        {
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"cLabel LIKE '%s' ORDER BY cLabel",gcCommand);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uClient");
        }

}//void ExttClientListSelect(void)


void ExttClientListFilter(void)
{
        //Filter
        printf("<td align=right >Select ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uClient"))
                printf("<option>uClient</option>");
        else
                printf("<option selected>uClient</option>");
        if(strcmp(gcFilter,"cLabel"))
                printf("<option>cLabel</option>");
        else
                printf("<option selected>cLabel</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttClientListFilter(void)


void ExttClientNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(guPermLevel>=10 && !guListMode)
		printf(LANG_NBB_NEW);

	if(uAllowMod(uOwner,uCreatedBy) && (guPermLevel>10 || uClient==guLoginClient || uCreatedBy==guLoginClient))
		printf(LANG_NBB_MODIFY);

	if(uAllowDel(uOwner,uCreatedBy) && (guPermLevel>10 || uCreatedBy==guLoginClient))
		printf(LANG_NBB_DELETE);

	if(uOwner)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttClientNavBar(void)


unsigned IsAuthUser(char *cLabel, unsigned uOwner, unsigned uCertClient)
{
	unsigned uRetVal=0;
        MYSQL_RES *res;
        MYSQL_ROW field;

	if(uOwner==1)
		return(0);

        sprintf(gcQuery,"SELECT uPerm FROM " TAUTHORIZE 
				" WHERE (cLabel='%s' OR uCertClient=%u ) AND uOwner=%u ",cLabel,uCertClient,uOwner);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
                printf("%s",mysql_error(&gMysql));
                return(0);
        }
        res=mysql_store_result(&gMysql);
        if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uRetVal);
        mysql_free_result(res);
	
	return(uRetVal);

}//unsigned IsAuthUser()


unsigned uMaxClientsReached(unsigned uClient)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	unsigned uRetVal=1;
	unsigned uMaxClients=2;
	unsigned uClients=0;

	sprintf(gcQuery,"SELECT cInfo FROM " TCLIENT " WHERE uClient=%u",uClient);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
                tClient(mysql_error(&gMysql));

        res=mysql_store_result(&gMysql);
        if((field=mysql_fetch_row(res)))
        {
		char *cp;

		if((cp=strstr(field[0],"uMaxClients=")))
			sscanf(cp+12,"%u",&uMaxClients);
	}
	mysql_free_result(res);

	sprintf(gcQuery,"SELECT COUNT(uClient) FROM " TCLIENT " WHERE uOwner=%u",uClient);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
                tClient(mysql_error(&gMysql));

        res=mysql_store_result(&gMysql);
        if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uClients);
	mysql_free_result(res);

	if(uClients<uMaxClients)
		uRetVal=0;

	return(uRetVal);

}//unsigned uMaxClientsReached(unsigned uClient)


void tTablePullDownResellers(unsigned uSelector,unsigned uMode)
{
	if(guPermLevel<10)
		return;

        MYSQL_RES *res;         
        MYSQL_ROW field;

        register int i,n;
    
	if(guPermLevel>11)
	{
		sprintf(gcQuery,"SELECT uClient,cLabel FROM " TCLIENT
				" WHERE (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP') AND uClient!=1"
				" ORDER BY cLabel");
	}
	else
	{
		sprintf(gcQuery,"SELECT uClient,cLabel FROM " TCLIENT
				" WHERE cLabel!='%s'"
				" AND (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP')" 
				" AND (uClient=%u OR uOwner"
				" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%u OR uClient=%u))"
				" ORDER BY cLabel",
					gcUser,
					guCompany,
					guCompany,
					guCompany);
	}

        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
        {
                printf("%s",mysql_error(&gMysql));
                return;
        }
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

        if(i>0)
        {
		if(!uMode)
			printf("<p><u>Optionally Select an Existing Company</u><br>");
                printf("<select name=cForClientPullDown>\n");

                //Default no selection
                printf("<option>---</option>\n");

                for(n=0;n<i;n++)
                {
                        unsigned ufield0=0;

                        field=mysql_fetch_row(res);
                        sscanf(field[0],"%u",&ufield0);

                        if(uSelector != ufield0)
                        {
                                printf("<option>%s</option>\n",field[1]);
                        }
                        else
                        {
                                printf("<option selected>%s</option>\n",field[1]);
                        }
                }
        	printf("</select><p>\n");
        }

}//tTablePullDownResellers()


void PermLevelDropDown(char *cuPerm)
{
	printf("User Level ");

	printf("<select name=cuPerm>\n");

	//Allow resellers to add their customers
	if(guPermLevel>7)
	{
		printf("<option ");
		if(!strcmp(cuPerm,BO_CUSTOMER))
			printf("selected>");
		else
			printf(">");
		printf("%s</option>\n",BO_CUSTOMER);
	}

	//Allow admins to add resellers
	//and organization admins
	if(guPermLevel>9)
	{
		printf("<option ");
		if(!strcmp(cuPerm,BO_RESELLER))
			printf("selected>");
		else
			printf(">");
		printf("%s</option>\n",BO_RESELLER);

		printf("<option ");
		if(!strcmp(cuPerm,BO_ADMIN))
			printf("selected>");
		else
			printf(">");
		printf("%s</option>\n",BO_ADMIN);

		printf("<option ");
		if(!strcmp(cuPerm,ORG_ADMIN))
			printf("selected>");
		else
			printf(">");
		printf("%s</option>\n",ORG_ADMIN);
	}

	//Allow root to add admin and other root users
	if(guPermLevel>11)
	{
		printf("<option ");
		if(!strcmp(cuPerm,BO_ADMIN))
			printf("selected>");
		else
			printf(">");
		printf("%s</option>\n",BO_ADMIN);

		printf("<option ");
		if(!strcmp(cuPerm,BO_ROOT))
			printf("selected>");
		else
			printf(">");
		printf("%s</option>\n",BO_ROOT);
	}

	printf("</select>\n");

}//void PermLevelDropDown(char *cuPerm)


const char *cUserLevel(unsigned uPermLevel)
{
	switch(uPermLevel)
	{
		case BO_ROOT_VAL:
			return(BO_ROOT);
		break;
		case BO_ADMIN_VAL:
			return(BO_ADMIN);
		break;
		case BO_RESELLER_VAL:
			return(BO_RESELLER);
		break;
		case BO_CUSTOMER_VAL:
			return(BO_CUSTOMER);
		break;
		case ORG_ADMIN_VAL:
			return(ORG_ADMIN);
		break;
		case ORG_ACCT_VAL:
			return(ORG_ACCT);
		break;
		case ORG_SERVICE_VAL:
			return(ORG_SERVICE);
		break;
		case ORG_SALES_VAL:
			return(ORG_SALES);
		break;
		case ORG_WEBMASTER_VAL:
			return(ORG_WEBMASTER);
		break;
		case ORG_CUSTOMER_VAL:
			return(ORG_CUSTOMER);
		break;
		default:
			return("---");
		break;
	}

}//const char *cUserLevel(unsigned uPermLevel)


void ContactsNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	if(guPermLevel<10 || !uClient)
		return;

	if(uOwner!=1)
		tAuthorizeLink();

	//Login info
	if(uOwner!=1 && strcmp(cCode,"Organization") && strncmp(cCode,"COMP",4))
		tAuthorizeNavListForLoadedOwner();

	//NavList proper
	sprintf(gcQuery,"SELECT uClient,cLabel FROM " TCLIENT " WHERE uOwner=%u AND uOwner!=1",uClient);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>Controlled Companies or Contacts NavList</u><br>\n");
                printf("%s<br>\n",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{
        	printf("<p><u>Controlled Companies or Contacts NavList</u><br>\n");
		while((field=mysql_fetch_row(res)))
        	{
			printf("<a class=darkLink href=?gcFunction=tClient&uClient=%s&uOnlyASPs=%u"
				,field[0],uOnlyASPs);
			if(cSearch[0])
			{
				spacetoplus(cSearch);
				printf("&cSearch=%s",cSearch);
			}
			printf(">%s</a><br>",field[1]);
        	}
	}
        mysql_free_result(res);

}//void ContactsNavList(void)


void htmlRecordContext(void)
{
	printf("<p><u>Record Context Info</u><br>");
	if(uOwner>1 && strcmp(cCode,"Contact") && strncmp(cCode,"CONT",4))
		printf("'%s' appears to be a reseller or ASP owned company or organization",cLabel);
	else if(uOwner>1 && strcmp(cCode,"Organization") && strncmp(cCode,"COMP",4))
		printf("'%s' appears to be a contact of <a class=darkLink href=?gcFunction=tClient&uClient=%u>'%s'</a>",
				cLabel,uOwner,ForeignKey(TCLIENT,"cLabel",uOwner));
	else if(uOwner==1 && strcmp(cLabel,"Root"))
		printf("'%s' appears to be an ASP root company",cLabel);
	else if(uOwner==1 && !strcmp(cLabel,"Root"))
		printf("'Root' is the system created root user. This user is the only user that can"
				" create ASP level companies. Make sure the passwd is changed"
				" regularly via the tAuthorize table.");
}//void htmlRecordContext(void)


void tClientNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

        printf("<p><u>tClientNavList</u><br>\n");

        if(!cSearch[0] && !uOnlyASPs)
        {
                printf("Must restrict with cSearch or uOnlyASPs");
                return;
        }

        ExttClientSelect();
        strcat(gcQuery," LIMIT 20");

        macro_mySQLRunAndStore(res);

        if(mysql_num_rows(res))
        {
                while((field=mysql_fetch_row(res)))
                        printf("<a class=darkLink href=?gcFunction=tClient&uClient=%s&cSearch=%s>%s</a><br>\n",
                                field[0]
                                ,cURLEncode(cSearch)
                                ,field[1]
                                );
        }
        else
                printf("No records found.\n");

        mysql_free_result(res);

}//void tClientNavList(void)


void BasictClientCheck(void)
{
	if(guMode==2000)
	{
		if(uOwner==1)
		{
			MYSQL_RES *res;
			sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s' AND uOwner=1",TextAreaSave(cLabel));
			macro_mySQLRunAndStore(res);

			if(mysql_num_rows(res))
			{
				guMode=2000;
				tClient("<blink>Error: </blink>ASP company with that cLabel already exists.");
			}
		}
	}
}//void BasictClientCheck(void)


void htmlAuthorizeLinksFromClient(unsigned uCertClient)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	if(!uCertClient) return;

        sprintf(gcQuery,"SELECT uAuthorize,cLabel,uPerm FROM "TAUTHORIZE" WHERE uCertClient=%u",uCertClient);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
                printf("%s",mysql_error(&gMysql));
                return;
        }
        res=mysql_store_result(&gMysql);
        while((field=mysql_fetch_row(res)))
		printf("<br><a class=darkLink href=?gcFunction=tAuthorize&uAuthorize=%s>"
				"%s/%s/%s</a>\n",field[0],cLabel,field[1],field[2]);
        mysql_free_result(res);

}//void htmlAuthorizeLinksFromClient()


void tAuthorizeNavListForLoadedOwner(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	unsigned uNum=0;
	unsigned uCount=0;

	sprintf(gcQuery,"SELECT uAuthorize,cLabel,uPerm,uCertClient FROM tAuthorize "
			" WHERE uOwner=%u OR uOwner IN (SELECT uClient FROM " TCLIENT
			" WHERE uOwner=%u) ORDER BY cLabel",uOwner,uOwner);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if((uNum=mysql_num_rows(res)))
	{
        	printf("<p><u>tAuthorizeNavList(%u)</u><br>\n",uNum);
        	while((field=mysql_fetch_row(res)))
		{
			printf("<a class=darkLink href=?gcFunction=tAuthorize&uAuthorize=%s>"
				"%s/%s/%s</a><br>\n",field[0],field[1],field[2],field[3]);
			if((++uCount)>50) break;
		}
		if(uCount>50)
			printf("Only first 50 shown\n");
				
	}
        mysql_free_result(res);

}//void tAuthorizeNavListForLoadedOwner(void)


void tAuthorizeLink(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	sprintf(gcQuery,"SELECT uAuthorize,cLabel,uPerm,uCertClient FROM tAuthorize "
			" WHERE (uCertClient=%u AND uCertClient!=1) OR cLabel='%s'",uClient,cLabel);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if((mysql_num_rows(res)>0))
	{
        	printf("<p><u>tAuthorizeLink()</u><br>\n");
        	while((field=mysql_fetch_row(res)))
		{
			printf("<a class=darkLink href=?gcFunction=tAuthorize&uAuthorize=%s>"
				"%s/%s/%s</a><br>\n",field[0],field[1],field[2],field[3]);
		}
				
	}
        mysql_free_result(res);

}//void tAuthorizeLink(void)

