/*
FILE 
	customercontact.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file in main source dir.
PURPOSE
	Contacts TAB.
	idnsAdmin interface program file.
*/

#include "interface.h"

#define ORG_CUSTOMER	"Organization Customer"
#define ORG_WEBMASTER	"Organization Webmaster"
#define ORG_SALES	"Organization Sales Force"
#define ORG_SERVICE	"Organization Customer Service"
#define ORG_ACCT	"Organization Bookkeeper"
#define ORG_ADMIN       "Organization Admin"

#define BO_ROOT		"Backend Root"
#define BO_ADMIN	"Backend Admin"


static unsigned uStep=0;

unsigned guContactPerm=0;
char *cPermPullDownStyle="type_fields_off";//has to be referenced from adminuser.c

static char cUserName[100]={""};
static char *cUserNameStyle="type_fields_off";

static char cPassword[100]={""};
static char *cPasswordStyle="type_fields_off";

static char cClearPassword[100]={""};
static unsigned uSaveClrPassword=0;

unsigned uForClient=0;//has to be referenced from adminuser.c
char *cForClientPullDownStyle="type_fields_off";//has to be referenced from adminuser.c
char cForClientPullDown[100]={""};//has to be referenced from adminuser.c

static char cClientName[100]={""};
static char *cClientNameStyle="type_fields_off";

static char cEmail[100]={""};
static char *cEmailStyle="type_fields_off";

static char cInfo[513];
static char *cInfoStyle="type_textarea_off";

static char cSearch[100]={""};

static unsigned uOwner=0;
static unsigned uCreatedBy=0;
static time_t uCreatedDate=0;
static unsigned uModBy=0;
static time_t uModDate=0;

extern unsigned guBrowserFirefox;//main.c
void EncryptPasswdWithSalt(char *cPasswd,char *cSalt);//main.c
//
//Local only
unsigned ValidateCustomerContactInput(void);
void NewCustomerContact(void);
void ModCustomerContact(void);
void DelCustomerContact(void);
void htmlCustomerContactWizard(unsigned uStep);
char *cPermLevel(unsigned uPerm);
unsigned EmailExists(char *cEmail);

void EncryptPasswd(char *pw);
void SetCustomerContactFieldsOn(void);
void LoadCustomerContact(void);
	
void ProcessCustomerContactVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cCustomer"))
			sprintf(gcCustomer,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cUserName"))
			sprintf(cUserName,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cPassword"))
			sprintf(cPassword,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cuPerm"))
			sscanf(entries[i].val,"%u",&guContactPerm);
		else if(!strcmp(entries[i].name,"cForClientPullDown"))
		{
                        uForClient=ReadPullDown("tClient","cLabel",entries[i].val);
			sprintf(cForClientPullDown,"%.99s",entries[i].val);
		}
		else if(!strcmp(entries[i].name,"cEmail"))
			sprintf(cEmail,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cClientName"))
			sprintf(cClientName,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cInfo"))
			sprintf(cInfo,"%.512s",entries[i].val);	
		else if(!strcmp(entries[i].name,"uStep"))
			sscanf(entries[i].val,"%u",&uStep);
		else if(!strcmp(entries[i].name,"cSearch"))
			 sprintf(cSearch,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"uForClient"))
			sscanf(entries[i].val,"%u",&uForClient);
		else if(!strcmp(entries[i].name,"uClient"))
			sscanf(entries[i].val,"%u",&guCookieContact);
		else if(!strcmp(entries[i].name,"uSaveClrPassword"))
			sscanf(entries[i].val,"%u",&uSaveClrPassword);
		else if(!strcmp(entries[i].name,"cClearPassword"))
			sprintf(cClearPassword,"%.99s",entries[i].val);
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

}//void ProcessUserVars(pentry entries[], int x)


char *cPermLevel(unsigned uPerm)
{
	static char cRet[32];
	switch(uPerm)
	{
		case 6:
			sprintf(cRet,"%s",ORG_ADMIN);
			break;
		case 5:
			sprintf(cRet,"%s",ORG_ACCT);
			break;
		case 4:
			sprintf(cRet,"%s",ORG_SERVICE);
			break;
		case 3:
			sprintf(cRet,"%s",ORG_SALES);
			break;
		case 2:
			sprintf(cRet,"%s",ORG_WEBMASTER);
			break;
		case 1:
			sprintf(cRet,"%s",ORG_CUSTOMER);
			break;
	}
	return(cRet);
	
}//char *cPermLevel(unsigned uPerm)


void CustomerContactGetHook(entry gentries[],int x)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	register int i;
	unsigned uClient=0;
	unsigned uOwner=0;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uClient"))
			sscanf(gentries[i].val,"%u",&uClient);
	}

	if(guCookieContact && !uClient)
	{
		LoadCustomerContact();
	}
	else if(gcCookieCustomer[0] && !guCookieContact)
	{
		//
		//Load the first contact as defualt
		sprintf(gcQuery,"SELECT uClient FROM tClient WHERE uOwner=%u ORDER BY cLabel LIMIT 1",uGetClient(gcCookieCustomer));
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			sscanf(field[0],"%u",&guCookieContact);
			SetSessionCookie();
			LoadCustomerContact();
		}
		htmlCustomerContact();
	}
	else if(uClient)
	{
		sprintf(gcQuery,"SELECT uOwner FROM tClient WHERE uClient=%u",uClient);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
			sscanf(field[0],"%u",&uOwner);
		mysql_free_result(res);

		if(uOwner)
		{
			sprintf(gcQuery,"SELECT cLabel FROM tClient WHERE uClient=%u",uOwner);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
			res=mysql_store_result(&gMysql);
			if((field=mysql_fetch_row(res)))
				sprintf(gcCookieCustomer,"%.31s",field[0]);
			mysql_free_result(res);

			guCookieContact=uClient;
			SetSessionCookie();
			LoadCustomerContact();
		}
	}
	htmlCustomerContact();
	

}//void CustomerContactGetHook(entry gentries[],int x)


void CustomerContactCommands(pentry entries[], int x)
{

	if(!strcmp(gcPage,"CustomerUser"))
	{
		ProcessCustomerContactVars(entries,x);
		if(!strcmp(gcFunction,"New"))
		{
			cClientName[0]=0;
			cUserName[0]=0;
			cPassword[0]=0;
			sprintf(gcNewStep,"Confirm ");
			gcMessage="Enter/modify data, review, then confirm. Any other action to cancel.";
			gcInputStatus[0]=0;
			SetCustomerContactFieldsOn();
			gcPermInputStatus[0]=0;
			htmlCustomerContact();
		}
		else if(!strcmp(gcFunction,"Confirm New"))
		{
			//If login not supplied create same as contact name
			if(cClientName[0] && !cUserName[0])
				sprintf(cUserName,"%.99s",cClientName);
			if(!ValidateCustomerContactInput())
			{
				sprintf(gcNewStep,"Confirm ");
				gcInputStatus[0]=0;
				htmlCustomerContact();
			}
			NewCustomerContact();
		}
		else if(!strcmp(gcFunction,"Modify"))
		{
			sprintf(gcModStep,"Confirm ");
			gcMessage="Enter/modify data, review, then confirm. Any other action to cancel.";
			gcInputStatus[0]=0;
			gcPermInputStatus[0]=0;
			SetCustomerContactFieldsOn();
			htmlCustomerContact();
		}
		else if(!strcmp(gcFunction,"Confirm Modify"))
		{
			if(!guCookieContact)
			{
				gcMessage="<blink>Error: </blink>Can't modify. No record selected";
				htmlCustomerContact();
			}
			if(!ValidateCustomerContactInput())
			{
				sprintf(gcModStep,"Confirm ");
				gcInputStatus[0]=0;
				htmlCustomerContact();
			}
			ModCustomerContact();
		}
		else if(!strcmp(gcFunction,"Delete"))
		{
			sprintf(gcDelStep,"Confirm ");
			gcMessage="Double check you have selected the correct record to delete. Then confirm. Any other action to cancel.";
			htmlCustomerContact();
		}		
		else if(!strcmp(gcFunction,"Confirm Delete"))
		{
			if(!guCookieContact)
			{
				gcMessage="<blink>Error: </blink>No record selected";
				htmlCustomerContact();
			}
			DelCustomerContact();
		}
		else if(!strcmp(gcFunction,"Add Contact Wizard"))
		{			
			guCookieContact=0;
			SetSessionCookie();
			htmlCustomerContactWizard(1);
		}
		else if(!strcmp(gcFunction,"Next"))
		{
			MYSQL_RES *res;
			SetCustomerContactFieldsOn();
			
			switch(uStep)
			{
				case 1:
					if(!uForClient)
					{
						gcMessage="<blink>Error: </blink>Customer must be specified";
						cForClientPullDownStyle="type_fields_req";
						htmlCustomerContactWizard(1);
					}
					break;
				case 2:						

					sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s' AND uOwner=%u",
							TextAreaSave(cClientName)
							,uGetClient(gcCustomer));
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));

					res=mysql_store_result(&gMysql);
			
					if(mysql_num_rows(res))
					{
						mysql_free_result(res);
						gcMessage="<blink>Error: </blink>Contact already exists";
						cClientNameStyle="type_fields_req";
						htmlCustomerContactWizard(2);
					}
					
					if(!cClientName[0])
					{
						cClientNameStyle="type_fields_req";
						gcMessage="<blink>Error: </blink>Contact name is required";
						htmlCustomerContactWizard(2);
					}
					if(cEmail[0])
					{
						if(strstr(cEmail,"@")==NULL || strstr(cEmail,".")==NULL)
						{
							cEmailStyle="type_fields_req";
							gcMessage="<blink>Error: </blink>Email format is incorrect";
							htmlCustomerContactWizard(2);
						}
					}
					break;
				case 3:
					if(!cUserName[0])
					{
						cUserNameStyle="type_fields_req";
						gcMessage="<blink>Error: </blink>Login name is required";
						htmlCustomerContactWizard(3);
					}
					else
					{
						sprintf(gcQuery,"SELECT uAuthorize FROM tAuthorize WHERE cLabel='%s'",
							cUserName);
						mysql_query(&gMysql,gcQuery);
						
						if(mysql_errno(&gMysql))
							htmlPlainTextError(mysql_error(&gMysql));
									
						res=mysql_store_result(&gMysql);
						
						if(mysql_num_rows(res))
						{
							cUserNameStyle="type_fields_req";
							gcMessage="<blink>Error: </blink>Login User already exists";
							htmlCustomerContactWizard(3);
						}
					}
					if(!cPassword[0] || (strlen(cPassword)<5))
					{
						cPasswordStyle="type_fields_req";
						gcMessage="<blink>Error: </blink>Password with at least 5 chars is required";
						htmlCustomerContactWizard(3);
					}
					if(guContactPerm<1 || guContactPerm>6)
					{
						char gcQuery[128];
						cPermPullDownStyle="type_fields_req";
						sprintf(gcQuery,"<blink>Error: </blink>Permission level must be set correctly (%u)",guContactPerm);
						gcMessage=gcQuery;
						htmlCustomerContactWizard(3);
					}
					gcInputStatus[0]=0;
					break;
			}//switch(uStep)
			uStep++;
                        htmlCustomerContactWizard(uStep);
		}			
		else if(!strcmp(gcFunction,"Finish"))
		{
			MYSQL_RES *res;
			
			SetCustomerContactFieldsOn();

			sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s' AND uOwner=%u",
					TextAreaSave(cClientName)
					,uGetClient(gcCustomer));
			mysql_query(&gMysql,gcQuery);
			
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
                        res=mysql_store_result(&gMysql);

			if(mysql_num_rows(res))
			{
				cClientNameStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>Client already exists";				
				htmlCustomerContactWizard(4);
				
			}
									
			if(!ValidateCustomerContactInput())
			{
				htmlCustomerContactWizard(4);
			}
			NewCustomerContact();
			htmlCustomerContact();			
		}
		else if(!strcmp(gcFunction,"Cancel Wizard"))
		{
			cUserName[0]=0;
			guContactPerm=0;
			guCookieContact=0;
			cPassword[0]=0;
			cClearPassword[0]=0;
			uSaveClrPassword=0;
			uForClient=0;
			cForClientPullDown[0]=0;
			cClientName[0]=0;
			cEmail[0]=0;
			cInfo[0]=0;
			
			htmlCustomerContact();
		}
		if(gcCustomer[0])
		{
		//	SelectCustomerContact();
			htmlCustomerContact();
		}
		else if(1)
		{
			//Old behavior
			//gcMessage="<blink>Error: </blink>Redirected to 'Companies'. You must select a Company first";
			//htmlCustomer();
			htmlCustomerContact();
		}
	}

}//void CustomerContactCommands(pentry entries[], int x)


void htmlCustomerContactWizard(unsigned uStep)
{
	htmlHeader("idnsAdmin","Header");
	sprintf(gcQuery,"CustomerUserWizard.%u",uStep);
	htmlCustomerContactPage("idnsAdmin",gcQuery);
	htmlFooter("Footer");

}//void htmlCustomerContactWizard(unsigned uStep)



void htmlCustomerContact(void)
{
	htmlHeader("idnsAdmin","Header");
	htmlCustomerContactPage("idnsAdmin","CustomerUser.Body");
	htmlFooter("Footer");

}//void htmlCustomerContact(void)


void htmlCustomerContactPage(char *cTitle, char *cTemplateName)
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

			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
			
			template.cpName[1]="cCGI";
			template.cpValue[1]="idnsAdmin.cgi";
			
			template.cpName[2]="gcLogin";
			template.cpValue[2]=gcLogin;

			template.cpName[3]="gcName";
			template.cpValue[3]=gcName;

			template.cpName[4]="gcOrgName";
			template.cpValue[4]=gcOrgName;

			template.cpName[5]="cUserLevel";
			template.cpValue[5]=(char *)cUserLevel(guPermLevel);

			template.cpName[6]="gcHost";
			template.cpValue[6]=gcHost;

			template.cpName[7]="gcMessage";
			template.cpValue[7]=gcMessage;

			template.cpName[8]="gcInputStatus";
			template.cpValue[8]=gcInputStatus;
				
			template.cpName[9]="gcNewStep";
			template.cpValue[9]=gcNewStep;

			template.cpName[10]="gcModStep";
			template.cpValue[10]=gcModStep;
			
			template.cpName[11]="gcDelStep";
			template.cpValue[11]=gcDelStep;

			template.cpName[12]="cUserNameStyle";
			template.cpValue[12]=cUserNameStyle;

			template.cpName[13]="cPasswordStyle";
			template.cpValue[13]=cPasswordStyle;

			template.cpName[14]="cEmailStyle";
			template.cpValue[14]=cEmailStyle;
				
			template.cpName[15]="cClientNameStyle";
			template.cpValue[15]=cClientNameStyle;
			
			template.cpName[16]="cUserName";
			template.cpValue[16]=cUserName;

			template.cpName[17]="cEmail";
			template.cpValue[17]=cEmail;

			template.cpName[18]="cClientName";
			template.cpValue[18]=cClientName;

			template.cpName[19]="cInfo";
			template.cpValue[19]=cInfo;

			template.cpName[20]="cPassword";
			template.cpValue[20]=cPassword;
			
			template.cpName[21]="cBtnStatus";
			if(guCookieContact)
				template.cpValue[21]="";
			else
				template.cpValue[21]="disabled";

			template.cpName[22]="cClearPassword";
			template.cpValue[22]=cClearPassword;
			
			template.cpName[23]="cCustomer";
			template.cpValue[23]=gcCookieCustomer;

			template.cpName[24]="gcZone";
			template.cpValue[24]=gcZone;

			template.cpName[25]="uView";
			template.cpValue[25]=cuView;

			template.cpName[26]="cSearch";
			template.cpValue[26]=cSearch;

			template.cpName[27]="cInfoStyle";
			template.cpValue[27]=cInfoStyle;

			template.cpName[28]="cLabel";
			template.cpValue[28]=gcCustomer;

			template.cpName[29]="uResource";
			template.cpValue[29]=cuResource;

			template.cpName[30]="uCreatedBy";
			template.cpValue[30]=cuCreatedBy;

			template.cpName[31]="uCreatedDate";
			template.cpValue[31]=cuCreatedDate;

			template.cpName[32]="uModBy";
			template.cpValue[32]=cuModBy;

			template.cpName[33]="uModDate";
			template.cpValue[33]=cuModDate;

			template.cpName[34]="uOwnerForm";
			template.cpValue[34]=cuOwnerForm;

			template.cpName[35]="uCreatedByForm";
			template.cpValue[35]=cuCreatedByForm;

			template.cpName[36]="uCreatedDateForm";
			template.cpValue[36]=cuCreatedDateForm;

			template.cpName[37]="uModByForm";
			template.cpValue[37]=cuModByForm;

			template.cpName[38]="uModDateForm";
			template.cpValue[38]=cuModDateForm;

			template.cpName[39]="uOwner";
			template.cpValue[39]=cuOwner;

			template.cpName[40]="";
			
			printf("\n<!-- Start htmlCustomerContactPage(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlCustomerContactPage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlCustomerContactPage()


void NewCustomerContact(void)
{
	sprintf(gcQuery,"INSERT INTO tClient SET cLabel='%s',cInfo='%s',cEmail='%s',uOwner=%u,uCreatedBy=%u,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW()),cCode='Contact'",
			cClientName
			,cInfo
			,cEmail
			,uForClient
			,guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	guCookieContact=mysql_insert_id(&gMysql);

	if(strncmp(cPassword,"..",2) && strncmp(cPassword,"$1$",3) && uSaveClrPassword)
		sprintf(cClearPassword,"%.99s",cPassword);
	
	EncryptPasswd(cPassword);

	sprintf(gcQuery,"INSERT INTO tAuthorize SET cLabel='%s',cPasswd='%s',cClrPasswd='%s',uPerm=%u,"
			"uCertClient=%u,uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cUserName
			,cPassword
			,cClearPassword
			,guContactPerm
			,guCookieContact
			,uForClient
			,guLoginClient);
	if(!guContactPerm || !guCookieContact || !uForClient)	
		htmlPlainTextError(gcQuery);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	if(mysql_insert_id(&gMysql) && guCookieContact)
	{
		iDNSLog(guCookieContact,"tClient","New");
		gcMessage="Company contact created OK";
	}
	else
	{
		iDNSLog(guCookieContact,"tClient","New Fail");
		gcMessage="<blink>Error: </blink>Company contact NOT created";
	}
	
	time(&uCreatedDate);
	uOwner=uForClient;
	uCreatedBy=guLoginClient;
	SetSessionCookie();

}//void NewCustomerContact(void)


void ModCustomerContact(void)
{
	unsigned uWasMod=0;

	if(!uForClient && gcCookieCustomer[0])
		uForClient=uGetClient(gcCookieCustomer);

	sprintf(gcQuery,"UPDATE tClient SET cLabel='%s',cEmail='%s',cInfo='%s',uModBy=%u,uOwner=%u,"
			"uModDate=UNIX_TIMESTAMP(NOW()) WHERE uClient=%u",
			cClientName
			,cEmail
			,cInfo
			,guLoginClient
			,uForClient
			,guCookieContact);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	uWasMod=mysql_affected_rows(&gMysql);

	if(!strstr(cInfo,"LDAP"))
	{

		if(strncmp(cPassword,"..",2) && strncmp(cPassword,"$1$",3) && uSaveClrPassword)
			sprintf(cClearPassword,"%.99s",cPassword);
		else
			cClearPassword[0]=0;

		if(strncmp(cPassword,"..",2) && strncmp(cPassword,"$1$",3))
			EncryptPasswd(cPassword);
	
		sprintf(gcQuery,"UPDATE tAuthorize SET cLabel='%s',cPasswd='%s',cClrPasswd='%s',uPerm=%u,"
			"uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE uCertClient=%u",
			cUserName
			,cPassword
			,cClearPassword
			,guContactPerm
			,guLoginClient
			,guCookieContact);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		uWasMod+=mysql_affected_rows(&gMysql);
	}

	if(uWasMod==2 && !strstr(cInfo,"LDAP"))
	{
		iDNSLog(guCookieContact,"tClient","Mod");
		gcMessage="Company contact modified OK";
	}
	else if(uWasMod==1 && !strstr(cInfo,"LDAP"))
	{
		iDNSLog(guCookieContact,"tClient","Mod");
		gcMessage="Company contact modified but tAuthorize not updated! Contact sysadmin.";
	}
	else if(uWasMod==1 && strstr(cInfo,"LDAP"))
	{
		iDNSLog(guCookieContact,"tClient","Mod");
		gcMessage="Company contact modified OK for LDAP account";
	}
	else if(uWasMod==0)
	{
		iDNSLog(guCookieContact,"tClient","Mod Fail");
		gcMessage="<blink>Error: </blink>Company contact NOT modified!";
	}
	time(&uModDate);
	uModBy=guLoginClient;

}//void ModCustomerContact(void)


void DelCustomerContact(void)
{
	sprintf(gcQuery,"DELETE FROM tClient WHERE uClient=%u",guCookieContact);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));


	sprintf(gcQuery,"DELETE FROM tAuthorize WHERE uCertClient=%u",guCookieContact);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	if(mysql_affected_rows(&gMysql))
	{
		iDNSLog(guCookieContact,"tClient","Del");
		gcMessage="Company contact deleted OK";
	}
	else
	{
		iDNSLog(guCookieContact,"tClient","Del Fail");
		gcMessage="<blink>Error: </blink>Company contact NOT deleted";
	}
	guCookieContact=0;
	SetSessionCookie();

}//void DelCustomerContact(void)


unsigned ValidateCustomerContactInput(void)
{
	if(!cClientName[0])
	{
		gcMessage="<blink>Error: </blink>'Contact Name' must be provided.";
		SetCustomerContactFieldsOn();
		cClientNameStyle="type_fields_req";
		return(0);
	}
	else
	{
		MYSQL_RES *res;
		register int i;

		for(i=0;cClientName[i];i++)
		{
			if(!isalnum(cClientName[i]) && cClientName[i]!='.'
				&& cClientName[i]!=' ' && cClientName[i]!='-' && cClientName[i]!=',')
			{
				SetCustomerContactFieldsOn();
				cClientNameStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>'Contact Name' has invalid chars";
				return(0);
			}
		}
		
		if(!strcmp(gcFunction,"Confirm New"))
		{
			sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s' AND uOwner=%u",
					TextAreaSave(cClientName)
					,uForClient
					);
			macro_mySQLRunAndStore(res);
			if(mysql_num_rows(res))
			{
				SetCustomerContactFieldsOn();
				cClientNameStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>'Contact Name' already exists for selected company,"
					" perhaps you wanted to create it for another company.";
				return(0);
			}
		}
		else if(!strcmp(gcFunction,"Confirm Modify"))
		{
			if(strcmp(TextAreaSave(cClientName),
				ForeignKey("tClient","cLabel",guCookieContact)))
			{
				sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s' AND uOwner=%u",
						TextAreaSave(cClientName)
						,uForClient
						);
				macro_mySQLRunAndStore(res);
				if(mysql_num_rows(res))
				{
					SetCustomerContactFieldsOn();
					cClientNameStyle="type_fields_req";
					gcMessage="<blink>Error: </blink>'Contact Name' already exists for selected company,"
						" perhaps you wanted to create it for another company.";
					return(0);
				}
			}
		}
	}

	if(cEmail[0])
	{
		if(strstr(cEmail,"@")==NULL || strstr(cEmail,".")==NULL)
		{
			gcMessage="<blink>Error: </blink>'Email Address' must be provided.";
			SetCustomerContactFieldsOn();
			cEmailStyle="type_fields_req";
			return(0);
		}
		if(!strcmp(gcFunction,"Confirm New"))
		{
			if(EmailExists(cEmail))
			{
				SetCustomerContactFieldsOn();
				cEmailStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>The entered email address is already used.";
				return(0);
			}
		}
			
	}
		
	if(!cUserName[0] && !strstr(cInfo,"LDAP"))
	{
		gcMessage="<blink>Error: </blink>Login is required, unless LDAP is in the additional info field.";
		SetCustomerContactFieldsOn();
		cUserNameStyle="type_fields_req";
		return(0);
	}
	else
	{
		//
		//Check for valid characters, no punctuation symbols allowed except '.' in Login
		register int i;
		MYSQL_RES *res;

		for(i=0;cUserName[i];i++)
		{
			if(!isalnum(cUserName[i]) && cUserName[i]!='.' && cUserName[i]!=' ' && cUserName[i]!='-')
			{
				SetCustomerContactFieldsOn();
				cUserNameStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>Login contains invalid chars.";
				return(0);
			}
		}

		if(!strcmp(gcFunction,"Confirm New"))
		{
			sprintf(gcQuery,"SELECT uAuthorize FROM tAuthorize WHERE cLabel='%s'",
					TextAreaSave(cUserName)
					);
			macro_mySQLRunAndStore(res);
			if(mysql_num_rows(res))
			{
				SetCustomerContactFieldsOn();
				cUserNameStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>Login already in use";
				return(0);
			}
		}
	}
	if(!cPassword[0]  && !strstr(cInfo,"LDAP"))
	{
		gcMessage="<blink>Error: </blink>Password must be provided.";
		SetCustomerContactFieldsOn();
		cPasswordStyle="type_fields_req";
		return(0);
	}
	else if( !strstr("LDAP",cInfo))
	{
		if(strlen(cPassword)<5)
		{
			gcMessage="<blink>Error: </blink>Password must be at least 5 characters.";
			SetCustomerContactFieldsOn();
			cPasswordStyle="type_fields_req";
			return(0);
		}
	}
	
	if(!uForClient && !gcCookieCustomer[0])
	{
		gcMessage="<blink>Error: </blink>Please select a Company to create the Contact for.";
		SetCustomerContactFieldsOn();
		cForClientPullDownStyle="type_fields_req";
		return(0);
	}

	if(!guContactPerm)
	{
		gcMessage="<blink>Error: </blink>Please select a permission level for contact.";
		SetCustomerContactFieldsOn();
		cPermPullDownStyle="type_fields_req";
		return(0);
	}

	return(1);
		
}//unsigned ValidateCustomerContactInput(void)


void funcTablePullDownResellers(FILE *fp,unsigned uUseStatus)
{
        MYSQL_RES *res;         
        MYSQL_ROW field;

        register int i,n; 
	char *cTitle;

	if(gcCookieCustomer[0])
		uUseStatus=1;//disable select forced to use cookie value

	if(!strcmp(gcPage,"CustomerUser"))
		cTitle="Select the Company you want to create the Company Contact for";
	else if(!strcmp(gcPage,"Zone"))
		cTitle="Select the Company you want to own the zone record";
	else 
		cTitle="Select the Company you want to create the Administrator for";

	sprintf(gcQuery,"SELECT tClient.uClient,tClient.cLabel FROM tClient WHERE"
			" (uOwner=1 OR uOwner=%u) AND (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP') ORDER BY cLabel",guOrg);

        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
        {
                printf("%s",mysql_error(&gMysql));
                return;
        }
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	fprintf(fp,"<input type=hidden name=uForClient value=%u>\n",uForClient);
	if(gcCookieCustomer[0]) sprintf(gcCustomer,"%.32s",gcCookieCustomer);
	fprintf(fp,"<input type=hidden name=cForClientPullDown value='%s'>\n",gcCustomer);
        if(i>0)
        {
		if(!strcmp(gcPage,"AdminUser") || !strcmp(gcPage,"Zone"))
		{
			fprintf(fp,"<select title='%s' name=cForClientPullDown class=%s %s>\n",
					cTitle,cForClientPullDownStyle,gcInputStatus);
		}
		else
		{
			if(uUseStatus)
	                	fprintf(fp,"<select title='%s' name=cForClientPullDown class=%s disabled>\n",
					cTitle,cForClientPullDownStyle);
			else
				fprintf(fp,"<select title='%s' name=cForClientPullDown class=type_textarea>\n",
					cTitle);
		}

                //Default no selection
                fprintf(fp,"<option>---</option>\n");

                for(n=0;n<i;n++)
                {
                        int unsigned field0=0;

                        field=mysql_fetch_row(res);
                        sscanf(field[0],"%u",&field0);

                        if(strcmp(field[1],gcCustomer))
                        {
                                fprintf(fp,"<option>%s</option>\n",field[1]);
                        }
                        else
                        {
                                fprintf(fp,"<option selected>%s</option>\n",field[1]);
                        }
                }
        }
        else
        {
	        fprintf(fp,"<select name=cForClientPullDown class=type_textarea><option>---</option></select>\n");
        }
        fprintf(fp,"</select>\n");

}//funcTablePullDownResellers()


void funcPermLevelDropDown(FILE *fp,unsigned uUseStatus)
{
	char *cTitle;

	if(!strcmp(gcPage,"CustomerUser"))
		cTitle="for the Company Contact";
	else
		cTitle="for the Administrator";

	fprintf(fp,"<input type=hidden name=uPerm value='%u'>\n",guContactPerm);

	if(uUseStatus)
		fprintf(fp,"<select name=cuPerm class=%s %s>\n",cPermPullDownStyle,gcInputStatus);
	else
		fprintf(fp,"<select title='Select the permission level %s' name=cuPerm class=%s>\n",cTitle,cPermPullDownStyle);

	
	if(!strcmp(gcPage,"CustomerUser"))
	{
		fprintf(fp,"<option value=1");
		if(guContactPerm==1)
			fprintf(fp,"selected");
		fprintf(fp,">%s</option>\n",ORG_CUSTOMER);
		
		fprintf(fp,"<option value=2");
		if(guContactPerm==2)
			fprintf(fp,"selected");
		fprintf(fp,">%s</option>\n",ORG_WEBMASTER);

		if(guContactPerm==3)
			fprintf(fp,"selected");
		fprintf(fp,">%s</option>\n",ORG_SALES);
		
		fprintf(fp,"<option value=4 ");
		if(guContactPerm==4)
			fprintf(fp,"selected");
		fprintf(fp,">%s</option>\n",ORG_SERVICE);

		fprintf(fp,"<option value=5 ");
		if(guContactPerm==5)
			fprintf(fp,"selected");
		fprintf(fp,">%s</option>\n",ORG_ACCT);

		fprintf(fp,"<option value=6 ");
		if(guContactPerm==6)
			fprintf(fp,"selected");
		fprintf(fp,">%s</option>\n",ORG_ADMIN);
	}
	else
	{
	fprintf(fp,"<option value=10 ");
		if(guContactPerm==10)
			fprintf(fp,"selected");
		fprintf(fp,">%s</option>\n",BO_ADMIN);

		fprintf(fp,"<option value=12 ");
		if(guContactPerm==12)
			fprintf(fp,"selected");
		fprintf(fp,">%s</option>\n",BO_ROOT);
	}
	fprintf(fp,"</select>\n");
	

}//void funcPermLevelDropDown(FILE *fp)


void SetCustomerContactFieldsOn(void)
{
	cUserNameStyle="type_fields";
	cPasswordStyle="type_fields";
	cForClientPullDownStyle="type_fields";
	cClientNameStyle="type_fields";
	cEmailStyle="type_fields";
	cInfoStyle="type_textarea";
	cPermPullDownStyle="type_fields";

}//void SetCustomerContactFieldsOn(void)


char *cClientLabel(unsigned uClient)
{
	static char cLabel[100]={""};
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT cLabel FROM tClient WHERE uClient=%u",uClient);
	mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	if((field=mysql_fetch_row(res)))
		sprintf(cLabel,"%s",field[0]);

	mysql_free_result(res);

	return(cLabel);

}//char *cClientLabel(unsigned uClient)


unsigned uGetClient(char *cLabel)
{
	unsigned uClient=0;
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s'",cLabel);
	mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uClient);

	mysql_free_result(res);

	return(uClient);

}//unsigned uGetClient(char *cLabel)


void LoadCustomerContact(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT tClient.uClient,tClient.cLabel,tClient.cInfo,tClient.cEmail,tClient.uOwner,"
			" tAuthorize.cLabel,tAuthorize.uPerm,tAuthorize.cPasswd,tAuthorize.cClrPasswd,"
			" tClient.uCreatedBy,tClient.uCreatedDate,tClient.uModBy,tClient.uModDate"
			" FROM tClient LEFT JOIN tAuthorize ON tAuthorize.uCertClient=tClient.uClient WHERE tClient.uClient=%u"
			,guCookieContact);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sprintf(cClientName,"%.100s",field[1]);
		sprintf(cInfo,"%.512s",field[2]);
		sprintf(cEmail,"%.100s",field[3]);
		sscanf(field[4],"%u",&uForClient);

		//LEFT JOIN
		if(field[5]!=NULL)
			sprintf(cUserName,"%.100s",field[5]);
		if(field[6]!=NULL)
			sscanf(field[6],"%u",&guContactPerm);
		if(field[7]!=NULL)
			sprintf(cPassword,"%.100s",field[7]);
		if(field[8]!=NULL)
			sprintf(cClearPassword,"%.100s",field[8]);

		sprintf(cForClientPullDown,"%.100s",cClientLabel(uForClient));
		sscanf(field[4],"%u",&uOwner);
		sscanf(field[9],"%u",&uCreatedBy);
		sscanf(field[10],"%lu",&uCreatedDate);
		sscanf(field[11],"%u",&uModBy);
		sscanf(field[12],"%lu",&uModDate);
	}
	else
		gcMessage="No records found";

	mysql_free_result(res);

}//void LoadCustomerContact(void)


unsigned EmailExists(char *cEmail)
{
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cEmail='%s'",TextAreaSave(cEmail));
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);

	return((unsigned)mysql_num_rows(res));

}//unsigned EmailExists(char *cEmail)


void funcContactNavList(FILE *fp,unsigned uSetCookie)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uResults=0;
	
	if(!cSearch[0]) return;

	if(guASPContact)
	{
		if(gcCookieCustomer[0])
			sprintf(gcQuery,"SELECT uClient,cLabel"
				" FROM tClient"
				" WHERE cLabel LIKE '%1$s%%'"
				" AND tClient.uOwner=%2$u "
				"ORDER BY cLabel",cSearch,uGetClient(gcCookieCustomer));
		else
			sprintf(gcQuery,"SELECT uClient,cLabel"
				" FROM tClient"
				" WHERE cLabel LIKE '%1$s%%'"
				"ORDER BY cLabel",cSearch);
	}
	else
	{
		if(gcCookieCustomer[0])
			sprintf(gcQuery,"SELECT uClient,cLabel"
				" FROM tClient"
				" WHERE cLabel LIKE '%1$s%%'"
				" AND tClient.uOwner=%2$u "
				"ORDER BY cLabel",cSearch,uGetClient(gcCookieCustomer));
		else
			sprintf(gcQuery,"SELECT uClient,cLabel"
				" FROM tClient"
				" WHERE cLabel LIKE '%1$s%%'"
				" AND tClient.uOwner=%2$u "
				"ORDER BY cLabel",cSearch,guOrg);
	}
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
			sscanf(field[0],"%u",&guCookieContact);
			if(uSetCookie)
				SetSessionCookie();
			LoadCustomerContact();
			mysql_free_result(res);

			fprintf(fp,"<a class=darkLink href=idnsAdmin.cgi?gcPage=CustomerUser&uClient=%s>%s</a><br>\n",
				field[0]
				,field[1]);
				
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
			fprintf(fp,"<a class=darkLink href=idnsAdmin.cgi?gcPage=CustomerUser&uClient=%s>%s</a><br>\n",
				field[0]
				,field[1]);

			if(uCount==MAX_RESULTS) break;
		}
		if(uCount<uResults)
			fprintf(fp,"Only the first %u shown (%u results). If the contact you are looking for is not in the list above "
				"please further refine your search.<br>",uCount,uResults);
		//We free result and return outside this if
		
	}
	else if(!uResults)
	{
		//Show no rcds found msg, free result, return}
		//fprintf(fp,"No records found.<br>%s\n",gcQuery);
		fprintf(fp,"No records found.<br>\n");
	}

	mysql_free_result(res);
	
}//void funcContactNavList(FILE *fp)


void funcContactLast7DaysActivity(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cLink[500]={""};
	time_t luClock;
	struct t_template template;

	template.cpName[0]="";

	if(!guCookieContact) return;

	if(guPermLevel>=6)
	{
		funcAdmLast7DaysAct(fp,guCookieContact);
		return;
	}
	
	sprintf(gcQuery,"SELECT cLabel,uTablePK,cTableName,uCreatedDate,"
			"IF(cTableName='tResource',(SELECT cZone FROM tZone,tResource WHERE tZone.uZone=tResource.uZone "
			"AND tResource.uResource=uTablePK),''),IF(cTableName='tResource',(SELECT tClient.cLabel FROM "
			"tClient,tResource WHERE tClient.uClient=tResource.uOwner AND tResource.uResource=uTablePK),''), "
			"IF(cTableName='tResource',(SELECT uView FROM tZone,tResource WHERE tZone.uZone=tResource.uZone "
			"AND tResource.uResource=uTablePK),''),IF(cTableName='tZone',(SELECT cZone FROM tZone WHERE uZone=uTablePK),''),"
			"IF(cTableName='tZone',(SELECT uView FROM tZone WHERE uZone=uTablePK),''),IF(cTableName='tZone',(SELECT "
			"tClient.cLabel FROM tClient,tZone WHERE tClient.uClient=tZone.uOwner=tClient.uClient AND uZone=uTablePK),'') "
			"FROM tLog WHERE (uLogType=1 OR uLogType=2 OR uLogType=3) AND "
			"uLoginClient=%u AND uCreatedDate>(UNIX_TIMESTAMP(NOW())-604800) ORDER BY tLog.uCreatedDate DESC LIMIT 100",guCookieContact);
	/*
	0: tLog.cLabel
	1: tLog.uTablePK
	2: tLog.cTableName
	3: tLog.FROM_UNIXTIME(uCreatedDate)
	4: IF(cTableName='tResource') cZone else ''
	5: IF(cTableName='tResource') Company label else ''
	6: IF(cTableName='tResource') tZone.uView else ''
	7: IF(cTableName='tZone') cZone else ''
	8: IF(cTableName='tZone') tZone.uView else ''
	9: IF(cTableName='tZone') Company label else ''
	*/
	//printf("%s",gcQuery);return;
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);
	if(!mysql_num_rows(res))
	{
		mysql_free_result(res);
		return;
	}

	fpTemplate(fp,"LastWWActTableTop",&template);
	fpTemplate(fp,"ActTableRowLabelContact",&template);	
		
	while((field=mysql_fetch_row(res)))
	{
		sscanf(field[3],"%lu",&luClock);
		if(!strcmp(field[2],"tResource"))
			sprintf(cLink,"<a title='Load this RR' class=darkLink "
				"href=\"idnsAdmin.cgi?gcPage=Resource&uResource=%s&cCustomer=%s&cZone=%s&uView=%s\">%s (%s)</a>",
				field[1]
				,field[5]
				,field[4]
				,field[6]
				,field[1]
				,field[4]);
		else if(!strcmp(field[2],"tZone"))
			sprintf(cLink,"<a title='Load this zone' class=darkLink "
				"href=\"idnsAdmin.cgi?gcPage=Zone&cZone=%s&cCustomer=%s&uView=%s\">%s</a>",
				field[7]
				,field[8]
				,field[9]
				,field[7]);
		else if(1)
			sprintf(cLink,"%s",field[1]);
		
		template.cpName[0]="cAction";
		template.cpValue[0]=field[0];

		template.cpName[1]="cRowId";
		template.cpValue[1]=cLink;

		template.cpName[2]="cTable";
		template.cpValue[2]=field[2];

		template.cpName[3]="cDate";
		template.cpValue[3]=ctime(&luClock);

		template.cpName[4]="";
		
		fpTemplate(fp,"ActTableRowContact",&template);

	}

	template.cpName[0]="";
	fpTemplate(fp,"LastWWActTableFooter",&template);
	
	mysql_free_result(res);

}//void funcContactLast7DaysActivity(FILE *fp)


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

