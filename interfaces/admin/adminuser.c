/*
FILE 
	adminuser.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file in main source dir.
PURPOSE
	iDNS Admin (Owner) Interface
	program file.
*/

#include "interface.h"

#define BO_ROOT         "Backend Root"
#define BO_ADMIN        "Backend Admin"

static unsigned uClient=0;
static char cuClient[16]={""};

static unsigned uStep=0;

static char cUserName[100]={""};
static char *cUserNameStyle="type_fields_off";

static char cPassword[100]={""};
static char *cPasswordStyle="type_fields_off";

static char cClearPassword[100]={""};
static unsigned uSaveClrPassword=0;

static char cClientName[100]={""};
static char *cClientNameStyle="type_fields_off";

static char cEmail[100]={""};
static char *cEmailStyle="type_fields_off";

static char cInfo[513];
static char *cInfoStyle="type_textarea_off";

static char cNavList[8192]={""};

static char cSearch[100]={""};
static char *cSearchStyle="type_fields";

extern unsigned uForClient;
extern char *cForClientPullDownStyle;
extern char cForClientPullDown[];

extern unsigned guContactPerm;
extern char *cPermPullDownStyle;

static unsigned uOwner=0;
static unsigned uCreatedBy=0;
static time_t uCreatedDate=0;
static unsigned uModBy=0;
static time_t uModDate=0;

extern unsigned guBrowserFirefox;//main.c
void EncryptPasswd(char *pw);
unsigned EmailExists(char *cEmail);//customercontact.c

//
//Local only
unsigned ValidateAdminUserInput(void);
void NewAdminUser(void);
void ModAdminUser(void);
void DelAdminUser(void);
void SelectAdminUser(char *cLabel,unsigned uMode);
void htmlAdminUserWizard(unsigned uStep);
char *cPermLevel(unsigned uPerm);
	
void SetAdminUserFieldsOn(void);
void LoadAdmin(void);



void ProcessAdminUserVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cUserName"))
			sprintf(cUserName,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cPassword"))
			sprintf(cPassword,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cEmail"))
			sprintf(cEmail,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cClientName"))
			sprintf(cClientName,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cInfo"))
			sprintf(cInfo,"%.256s",entries[i].val);	
		else if(!strcmp(entries[i].name,"uStep"))
			sscanf(entries[i].val,"%u",&uStep);
		else if(!strcmp(entries[i].name,"cSearch"))
			 sprintf(cSearch,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"uClient"))
			sprintf(cuClient,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"cNavList"))
			sprintf(cNavList,"%.8191s",entries[i].val);		
		else if(!strcmp(entries[i].name,"cClearPassword"))
			sprintf(cClearPassword,"%.99s",entries[i].val);		
		else if(!strcmp(entries[i].name,"uSaveClrPassword"))
			sscanf(entries[i].val,"%u",&uSaveClrPassword);	
		else if(!strcmp(entries[i].name,"cuPerm"))
			sscanf(entries[i].val,"%u",&guContactPerm);
		else if(!strcmp(entries[i].name,"cForClientPullDown"))
		{
                        uForClient=ReadPullDown("tClient","cLabel",entries[i].val);
			sprintf(cForClientPullDown,"%.99s",entries[i].val);
			sprintf(gcCustomer,"%.99s",entries[i].val);
		}
		else if(!strcmp(entries[i].name,"uForClient"))
			sscanf(entries[i].val,"%u",&uForClient);
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


void AdminUserGetHook(entry gentries[],int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uClient"))
		{
			sprintf(cuClient,"%s",gentries[i].val);
			sscanf(gentries[i].val,"%u",&uClient);
		}
	}

	if(uClient)
		LoadAdmin();
	
	htmlAdminUser();
	
}//void AdminUserGetHook(entry gentries[],int x)


void AdminUserCommands(pentry entries[], int x)
{

	if(!strcmp(gcPage,"Administrator"))
	{
		ProcessAdminUserVars(entries,x);
		/*if(!gcCustomer[0])
			GetConfiguration("cASP",gcCustomer,1);
		*/	
		if(!strcmp(gcFunction,"New"))
		{			
			sprintf(gcNewStep,"Confirm ");
			gcMessage="Enter/modify data, review, then confirm. Any other action to cancel.";
			gcInputStatus[0]=0;
			SetAdminUserFieldsOn();
			gcPermInputStatus[0]=0;
			htmlAdminUser();
		}
		else if(!strcmp(gcFunction,"Confirm New"))
		{
			//If login not supplied create same as contact name
			if(cClientName[0] && !cUserName[0])
				sprintf(cUserName,"%.99s",cClientName);
														
			if(!ValidateAdminUserInput())
			{
				sprintf(gcNewStep,"Confirm ");
				gcInputStatus[0]=0;			
				htmlAdminUser();
			}
			NewAdminUser();
			htmlAdminUser();

		}
		else if(!strcmp(gcFunction,"Modify"))
		{
			sprintf(gcModStep,"Confirm ");
			gcMessage="Enter/modify data, review, then confirm. Any other action to cancel.";
			gcInputStatus[0]=0;
			SetAdminUserFieldsOn();
			htmlAdminUser();
		}
		else if(!strcmp(gcFunction,"Confirm Modify"))
		{
			if(!cuClient[0])
			{
				gcMessage="<blink>Error: </blink>Can't modify. No record selected";
				htmlAdminUser();
			}
			if(!ValidateAdminUserInput())
			{
				sprintf(gcModStep,"Confirm ");
				gcInputStatus[0]=0;
				htmlAdminUser();
			}
			ModAdminUser();

			htmlAdminUser();
		}
		else if(!strcmp(gcFunction,"Delete"))
		{
			sprintf(gcDelStep,"Confirm ");
			gcMessage="Double check you have selected the correct record to delete. Then confirm. Any other action to cancel.";
			htmlAdminUser();
		}		
		else if(!strcmp(gcFunction,"Confirm Delete"))
		{
			if(!cuClient[0])
			{
				gcMessage="<blink>Error: </blink>No record selected";
				htmlAdminUser();
			}
			DelAdminUser();

			htmlAdminUser();
		}
		else if(!strcmp(gcFunction,"Add Admin User Wizard"))
		{
			SetAdminUserFieldsOn();
			htmlAdminUserWizard(1);
		}
		else if(!strcmp(gcFunction,"Next"))
		{
			MYSQL_RES *res;
			SetAdminUserFieldsOn();
			gcInputStatus[0]=0;

			switch(uStep)
			{
				case 1:
					if(!cClientName[0])
					{
						cClientNameStyle="type_fields_req";
						gcMessage="<blink>Error: </blink>Contact Name must be provided";
						htmlAdminUserWizard(1);
					}
					else
					{
					
						SelectAdminUser(cClientName,0);
						res=mysql_store_result(&gMysql);
			
						if(mysql_num_rows(res))
						{
							mysql_free_result(res);
							gcMessage="<blink>Error: </blink>Contact Name already exists";
							cClientNameStyle="type_fields_req";
							htmlAdminUserWizard(1);
						}
					}
					if(cEmail[0])
					{
						if(strstr(cEmail,"@")==NULL || strstr(cEmail,".")==NULL)
						{
							cEmailStyle="type_fields_req";
							gcMessage="<blink>Error: </blink>Email format is incorrect";
							htmlAdminUserWizard(1);
						}
					}
					break;
				case 2:
					if(!cUserName[0])
					{
						cUserNameStyle="type_fields_req";
						gcMessage="Review your input data";
						htmlAdminUserWizard(2);
					}
					else
					{
						sprintf(gcQuery,"SELECT uAuthorize FROM tAuthorize WHERE cLabel='%s'",cUserName);
						mysql_query(&gMysql,gcQuery);
						
						if(mysql_errno(&gMysql))
							htmlPlainTextError(mysql_error(&gMysql));
									
						res=mysql_store_result(&gMysql);
						
						if(mysql_num_rows(res))
						{
							cUserNameStyle="type_fields_req";
							gcMessage="<blink>Error: </blink>Login User already exists";
							htmlAdminUserWizard(2);
						}
					}
					if(!cPassword[0] || (strlen(cPassword)<5))
					{
						cPasswordStyle="type_fields_req";
						gcMessage="<blink>Error: </blink>Password with at least 5 chars is required";
						htmlAdminUserWizard(2);
					}
					break;
			}//switch(uStep)
			uStep++;
                        htmlAdminUserWizard(uStep);
		}			
		else if(!strcmp(gcFunction,"Finish"))
		{
			MYSQL_RES *res;
			
			SetAdminUserFieldsOn();

			SelectAdminUser(cClientName,0);
                        res=mysql_store_result(&gMysql);

			if(mysql_num_rows(res))
			{
				cClientNameStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>Client already exists";				
				htmlAdminUserWizard(4);
				
			}
									
			if(!ValidateAdminUserInput())
			{
				htmlAdminUserWizard(4);
			}
			NewAdminUser();
			htmlAdminUser();
		}
		else if(!strcmp(gcFunction,"Cancel Wizard"))
		{
			cUserName[0]=0;
			guContactPerm=0;
			cuClient[0]=0;
			uClient=0;
			cPassword[0]=0;
			cClearPassword[0]=0;
			uSaveClrPassword=0;
			uForClient=0;
			cForClientPullDown[0]=0;
			cClientName[0]=0;
			cEmail[0]=0;
			cInfo[0]=0;

			htmlAdminUser();
		}
			
		htmlAdminUser();
	}

}//void AdminUserCommands(pentry entries[], int x)


void htmlAdminUserWizard(unsigned uStep)
{
	htmlHeader("idnsAdmin","Header");
	sprintf(gcQuery,"AdminUserWizard.%u",uStep);
	htmlAdminUserPage("idnsAdmin",gcQuery);
	htmlFooter("Footer");

}//void htmlAdminUserWizard(unsigned uStep)



void htmlAdminUser(void)
{
	htmlHeader("idnsAdmin","Header");
	htmlAdminUserPage("idnsAdmin","AdminUser.Body");
	htmlFooter("Footer");

}//void htmlAdminUser(void)


void htmlAdminUserPage(char *cTitle, char *cTemplateName)
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

			sprintf(cuForClient,"%u",uForClient);
			
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

			template.cpName[21]="uForClient";
			template.cpValue[21]=cuForClient;

			template.cpName[22]="cNavList";
			template.cpValue[22]=cNavList;

			template.cpName[23]="uClient";
			template.cpValue[23]=cuClient;

			template.cpName[24]="cSearchStyle";
			template.cpValue[24]=cSearchStyle;

			template.cpName[25]="cSearch";
			template.cpValue[25]=cSearch;

			template.cpName[26]="cInfoStyle";
			template.cpValue[26]=cInfoStyle;

			template.cpName[27]="cLabel";
			template.cpValue[27]=gcCustomer;

			template.cpName[28]="cClearPassword";
			template.cpValue[28]=cClearPassword;

			template.cpName[29]="cZone";
			template.cpValue[29]=gcZone;

			template.cpName[30]="cCustomer";
			template.cpValue[30]=gcCustomer;

			template.cpName[31]="uResource";
			template.cpValue[31]=cuResource;

			template.cpName[32]="uView";
			template.cpValue[32]=cuView;

			template.cpName[33]="uOwner";
			template.cpValue[33]=cuOwner;

			template.cpName[34]="uCreatedBy";
			template.cpValue[34]=cuCreatedBy;

			template.cpName[35]="uCreatedDate";
			template.cpValue[35]=cuCreatedDate;

			template.cpName[36]="uModBy";
			template.cpValue[36]=cuModBy;

			template.cpName[37]="uModDate";
			template.cpValue[37]=cuModDate;

			template.cpName[38]="uOwnerForm";
			template.cpValue[38]=cuOwnerForm;

			template.cpName[39]="uCreatedByForm";
			template.cpValue[39]=cuCreatedByForm;

			template.cpName[40]="uCreatedDateForm";
			template.cpValue[40]=cuCreatedDateForm;

			template.cpName[41]="uModByForm";
			template.cpValue[41]=cuModByForm;

			template.cpName[42]="uModDateForm";
			template.cpValue[42]=cuModDateForm;

			template.cpName[43]="";

			printf("\n<!-- Start htmlAdminUserPage(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlAdminUserPage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlAdminUserPage()


void SelectAdminUser(char *cLabel,unsigned uMode)
{
	if(!uMode)
	{
		if(strstr(cLabel,"%")==NULL)
			sprintf(gcQuery,"SELECT %s,%s FROM tClient,tAuthorize WHERE tClient.cLabel='%s' AND tAuthorize.uCertClient=tClient.uClient",
					VAR_LIST_tClient,
					VAR_LIST_tAuthorize,
					cLabel);
		else
			sprintf(gcQuery,"SELECT %s,%s FROM tClient,tAuthorize WHERE tClient.cLabel LIKE '%s' AND tAuthorize.uCertClient=tClient.uClient AND (tClient.uOwner=%u OR tClient.uClient=1) ORDER BY tClient.cLabel",
					VAR_LIST_tClient,
					VAR_LIST_tAuthorize,
					cLabel,
					uGetClient(gcCustomer)
					);
	}
	else
	{
			sprintf(gcQuery,"SELECT %s,%s FROM tClient,tAuthorize WHERE tAuthorize.uAuthorize=%s AND tAuthorize.uCertClient=tClient.uClient",
					VAR_LIST_tClient,
					VAR_LIST_tAuthorize,
					cLabel);
	}

	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

}//void SelectAdminUser(char *cLabel,unsigned uMode)


void NewAdminUser(void)
{
	sprintf(gcQuery,"INSERT INTO tClient SET cLabel='%s',cInfo='%s',cEmail='%s',cCode='Contact',"
			"uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cClientName
			,cInfo
			,cEmail
			,uForClient
			,guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	uClient=mysql_insert_id(&gMysql);
	sprintf(cuClient,"%.15u",uClient);

	if(strncmp(cPassword,"..",2) && strncmp(cPassword,"$1$",3) && uSaveClrPassword)
		sprintf(cClearPassword,"%.99s",cPassword);

	EncryptPasswd(cPassword);
	
	
	//
	//uPerm=10 Back-end admin
	sprintf(gcQuery,"INSERT INTO tAuthorize SET cLabel='%s',cPasswd='%s',cClrPasswd='%s',"
			"uPerm=%u,uCertClient=%u,uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cUserName
			,cPassword
			,cClearPassword
			,guContactPerm
			,uClient
			,uForClient
			,guLoginClient);
	if(!guContactPerm || !uClient || !uForClient)	
		htmlPlainTextError(gcQuery);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	if(mysql_insert_id(&gMysql) && uClient)
	{
		iDNSLog(uClient,"tClient","New");
		gcMessage="Admin User created OK";
	}
	else
	{
		iDNSLog(uClient,"tClient","New Fail");
		gcMessage="<blink>Error: </blink>Admin User NOT created";
	}
	time(&uCreatedDate);
	uOwner=uForClient;
	uCreatedBy=guLoginClient;


}//void NewAdminUser(void)


void ModAdminUser(void)
{
	unsigned uWasMod=0;
	sprintf(gcQuery,"UPDATE tClient SET cLabel='%s',cEmail='%s',cInfo='%s',uOwner=%u,"
			"uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE uClient=%s",
			cClientName
			,cEmail
			,cInfo
			,uForClient
			,guLoginClient
			,cuClient);
	mysql_query(&gMysql,gcQuery);
	
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	sscanf(cuClient,"%u",&uClient);
	
	uWasMod=mysql_affected_rows(&gMysql);

	
	if(strncmp(cPassword,"..",2) && uSaveClrPassword)
		sprintf(cClearPassword,"%.99s",cPassword);
	else
		cClearPassword[0]=0;

	if(strncmp(cPassword,"..",2) && strncmp(cPassword,"$1$",3))
		EncryptPasswd(cPassword);
	
	sprintf(gcQuery,"UPDATE tAuthorize SET cLabel='%s',cPasswd='%s',cClrPasswd='%s',"
			"uPerm=%u,uOwner=%u,uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE uCertClient='%s'",
			cUserName
			,cPassword
			,cClearPassword
			,guContactPerm
			,uForClient
			,guLoginClient
			,cuClient);
	 mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	uWasMod+=mysql_affected_rows(&gMysql);

	if(uWasMod==2)
	{
		iDNSLog(uClient,"tClient","Mod");
		gcMessage="Admin User modified OK";
	}
	else
	{
		iDNSLog(uClient,"tClient","Mod Fail");
		gcMessage="<blink>Error: </blink>Administrator User NOT modified";
	}
	uModBy=guLoginClient;
	time(&uModDate);

}//void ModAdminUser(void)


void DelAdminUser(void)
{
	sprintf(gcQuery,"DELETE FROM tClient WHERE uClient=%s",cuClient);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	sscanf(cuClient,"%u",&uClient);
	

	sprintf(gcQuery,"DELETE FROM tAuthorize WHERE uCertClient='%s'",cuClient);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	if(mysql_affected_rows(&gMysql))
	{
		iDNSLog(uClient,"tClient","Del");
		gcMessage="Admin User deleted OK";
	}
	else
	{
		iDNSLog(uClient,"tClient","Del Fail");
		gcMessage="<blink>Error: </blink>Admin User NOT deleted";
	}

}//void DelAdminUser(void)


unsigned ValidateAdminUserInput(void)
{
	if(!cClientName[0])
	{
		gcMessage="<blink>Error: </blink>'Contact Name' must be provided.";
		SetAdminUserFieldsOn();
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
				SetAdminUserFieldsOn();
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
				SetAdminUserFieldsOn();
				cClientNameStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>'Contact Name' already exists for selected company,"
					" perhaps you wanted to create it for another company.";
				return(0);
			}
		}
		else if(!strcmp(gcFunction,"Confirm Modify"))
		{
			unsigned uRowId=0;

			sscanf(cuClient,"%u",&uRowId);

			if(strcmp(TextAreaSave(cClientName),
				ForeignKey("tClient","cLabel",uRowId)))
			{
				sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s' AND uOwner=%u",
						TextAreaSave(cClientName)
						,uForClient
						);
				macro_mySQLRunAndStore(res);
				if(mysql_num_rows(res))
				{
					SetAdminUserFieldsOn();
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
			SetAdminUserFieldsOn();
			cEmailStyle="type_fields_req";
			return(0);
		}
		if(!strcmp(gcFunction,"Confirm New"))
		{
			if(EmailExists(cEmail))
			{
				SetAdminUserFieldsOn();
				cEmailStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>The entered email address is already used.";
				return(0);
			}
		}
			
	}
		
	if(!cUserName[0])
	{
		gcMessage="<blink>Error: </blink>Login is required.";
		SetAdminUserFieldsOn();
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
				SetAdminUserFieldsOn();
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
				SetAdminUserFieldsOn();
				cUserNameStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>Login already in use";
				return(0);
			}
		}
	}
	if(!cPassword[0])
	{
		gcMessage="<blink>Error: </blink>Password must be provided.";
		SetAdminUserFieldsOn();
		cPasswordStyle="type_fields_req";
		return(0);
	}
	else
	{
		if(strlen(cPassword)<5)
		{
			gcMessage="<blink>Error: </blink>Password must be at least 5 characters.";
			SetAdminUserFieldsOn();
			cPasswordStyle="type_fields_req";
			return(0);
		}
	}
	
	if(!uForClient)
	{
		gcMessage="<blink>Error: </blink>Please select a Company to create the Contact for.";
		SetAdminUserFieldsOn();
		cForClientPullDownStyle="type_fields_req";
		return(0);
	}
	return(1);
		
}//unsigned ValidateAdminInput(void)


void SetAdminUserFieldsOn(void)
{
	cUserNameStyle="type_fields";
	cPasswordStyle="type_fields";
	cClientNameStyle="type_fields";
	cEmailStyle="type_fields";
	cInfoStyle="type_textarea";
	cForClientPullDownStyle="type_fields";
	cPermPullDownStyle="type_fields";
}//void SetAdminUserFieldsOn(void)


void LoadAdmin(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT tClient.uClient,tClient.cLabel,tClient.cInfo,tClient.cEmail,tClient.uOwner,"
			"tAuthorize.cLabel,tAuthorize.uPerm,tAuthorize.cPasswd,tAuthorize.cClrPasswd, "
			"tClient.uCreatedBy,tClient.uCreatedDate,tClient.uModBy,tClient.uModDate FROM "
			"tClient,tAuthorize WHERE tClient.uClient=%u AND tAuthorize.uCertClient=tClient.uClient",uClient);
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
		sprintf(cUserName,"%.100s",field[5]);
		sscanf(field[6],"%u",&guContactPerm);
		sprintf(cPassword,"%.100s",field[7]);
		sprintf(cClearPassword,"%.100s",field[8]);
		sprintf(cForClientPullDown,"%.100s",cClientLabel(uForClient));
		sprintf(gcCustomer,"%.100s",cForClientPullDown);
		sscanf(field[4],"%u",&uOwner);
		sscanf(field[9],"%u",&uCreatedBy);
		sscanf(field[10],"%lu",&uCreatedDate);
		sscanf(field[11],"%u",&uModBy);
		sscanf(field[12],"%lu",&uModDate);
	}
	//else
	//	gcMessage="No records found";

//	mysql_free_result(res);

}//void LoadAdmin(void)


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
			htmlPlainTextError(mysql_error(&gMysql));
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


void funcAdmLast7DaysAct(FILE *fp, unsigned uArgClient)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cLink[500]={""};
	time_t luClock;
	struct t_template template;

	template.cpName[0]="";

	if(uArgClient) uClient=uArgClient;

	if(!uClient) return;
	
	sprintf(gcQuery,"SELECT tLog.cLabel,uTablePK,cTableName,tLog.uCreatedDate,"
			"IF(cTableName='tResource',(SELECT cZone FROM tZone,tResource WHERE tZone.uZone=tResource.uZone "
			"AND tResource.uResource=uTablePK),''),IF(cTableName='tResource',(SELECT tClient.cLabel FROM "
			"tClient,tResource WHERE tClient.uClient=tResource.uOwner AND tResource.uResource=uTablePK),''), "
			"IF(cTableName='tResource',(SELECT uView FROM tZone,tResource WHERE tZone.uZone=tResource.uZone "
			"AND tResource.uResource=uTablePK),''),IF(cTableName='tZone',"
			"(SELECT cZone FROM tZone WHERE uZone=uTablePK),''),"
			"IF(cTableName='tZone',(SELECT tClient.cLabel FROM"
			" tClient,tZone WHERE tZone.uOwner=tClient.uClient AND uZone=uTablePK),''),"
			"IF(cTableName='tZone',(SELECT uView FROM tZone WHERE uZone=uTablePK),'') "
			"FROM tLog WHERE (uLogType=1 OR uLogType=2 OR uLogType=3) AND "
			"uLoginClient=%u AND tLog.uCreatedDate>(UNIX_TIMESTAMP(NOW())-604800)"
			" ORDER BY tLog.uCreatedDate DESC LIMIT 100",uClient);
//	fprintf(fp,"%s",gcQuery);return;
	/*
	0: tLog.cLabel
	1: tLog.uTablePK
	2: tLog.cTableName
	3: tLog.FROM_UNIXTIME(uCreatedDate)
	4: IF(cTableName='tResource') cZone else ''
	5: IF(cTableName='tResource') Company label else ''
	6: IF(cTableName='tResource') tZone.uView else ''
	7: IF(cTableName='tZone' cZone else ''
	8: IF(cTableName='tZone' Company label else ''
	9: IF(cTableName='tZone' tZone.uView else ''
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
	fpTemplate(fp,"ActTableRowLabelAdmin",&template);

	while((field=mysql_fetch_row(res)))
	{
		sscanf(field[3],"%lu",&luClock);
		if(!strcmp(field[2],"tResource"))
		{
			//Check if the RR was deleted.
			if(field[4]!=NULL)
				sprintf(cLink,"<a title='Load this RR' class=darkLink "
					"href=\"idnsAdmin.cgi?gcPage=Resource&uResource=%s&cCustomer=%s"
					"&cZone=%s&uView=%s\">%s</a>",
						field[1]
						,field[5]
						,field[4]
						,field[6]
						,field[1]);
			else
				sprintf(cLink,"<a href=idnsAdmin.cgi?gcPage=RestoreResource&uDeletedResource=%s>%s</a>"
						" (<font color=red>Deleted</font>)",
							field[1],field[1]);
		}
		else if(!strcmp(field[2],"tZone"))
		{
			//Check if the zone was deleted.
			if(field[7]!=NULL)
				sprintf(cLink,"<a title='Load this Zone' class=darkLink "
					"href=\"idnsAdmin.cgi?gcPage=Zone&cZone=%s,uView=%s&cCustomer=%s\">%s</a>",
					field[7]
					,field[9]
					,field[8]
					,field[7]
					);
			else
				sprintf(cLink,"<a href=idnsAdmin.cgi?gcPage=RestoreZone&uDeletedZone=%s>%s</a>"
						" (<font color=red>Deleted</font>)",
							field[1],field[1]);
		}
		else if(1)
			sprintf(cLink,"<a class=darkLink title='Load record into the iDNS backend' "
					"href=iDNS.cgi?gcFunction=%s&u%s=%s target=_blank>%s</a>",
						field[2],field[2]+1,field[1],field[1]);
		
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

}//void funcAdmLast7DaysAct(FILE *fp)


void funcAdminUsers(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uPerm=0;

	if(gcCustomer[0]) //If we have a set company, will show admin users for that company only. Otherwise will show all uPerm>6 users
	{
		unsigned uCompany=0;
		sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s'",gcCustomer);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
	
		if(!mysql_num_rows(res))
			return;

		if((field=mysql_fetch_row(res)))
			sscanf(field[0],"%u",&uCompany);
		mysql_free_result(res);
		
		sprintf(gcQuery,"SELECT tClient.uClient,tAuthorize.cLabel,tAuthorize.uPerm,tClient.cLabel FROM "
				"tAuthorize,tClient WHERE tAuthorize.uPerm>=10 AND tAuthorize.uCertClient=tClient.uClient "
				"AND tClient.uOwner=%u ORDER BY tAuthorize.cLabel",uCompany);
	}
	else
		sprintf(gcQuery,"SELECT tClient.uClient,tAuthorize.cLabel,tAuthorize.uPerm,tClient.cLabel FROM "
				 "tAuthorize,tClient WHERE tAuthorize.uPerm>=10 AND tAuthorize.uCertClient=tClient.uClient ORDER BY tAuthorize.cLabel");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);
        while((field=mysql_fetch_row(res)))
	{
		sscanf(field[2],"%u",&uPerm);
		fprintf(fp,"<tr><td><a class=darkLink title='Click to load into the Adminstrators tab' "
			"href=\"idnsAdmin.cgi?uClient=%s&cZone=%s&uView=%s&uResource=%u&cCustomer=%s&gcPage=Administrator\">%s</a></td>"
			"<td>%s</td><td>%s</td></tr>\n",
			field[0]
			,gcZone
			,cuView
			,uResource
			,gcCustomer
			,field[1]
			,cUserLevel(uPerm)
			,field[3]
			);
	}
	mysql_free_result(res);

}//void funcAdminUsers(FILE *fp)

