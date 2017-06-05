/*
FILE 
	customer.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file in main source dir.
PURPOSE
	Companies TAB.
	idnsAdmin interface program file.
*/


#define BO_ADMIN 	"Backend Admin"
#define BO_ROOT 	"Backend Root"

#include "interface.h"

static unsigned uClient=0;

static char cCompanyName[100]={""};
static char *cLabelStyle="type_fields_off";

static char cEmail[100]={""};
static char *cEmailStyle="type_fields_off";

static char cInfo[513]={""};
static char *cInfoStyle="type_textarea_off";

static char cNavList[8192]={"No results."};

static char cSearch[100]={""};
static char *cSearchStyle="type_fields";
static unsigned uStep=0;

static unsigned uOwner=0;
static unsigned uCreatedBy=0;
static time_t uCreatedDate=0;
static unsigned uModBy=0;
static time_t uModDate=0;

//
//Local only
unsigned ValidateCustomerInput(void);
void SelectCustomer(char *cLabel, unsigned uMode);
void NewCustomer(void);
void DelCustomer(void);
void ModCustomer(void);

unsigned uHasZones(char *cLabel);
unsigned uHasBlocks(char *cLabel);
unsigned uHasContacts(char *cLabel);

void SetCustomerFieldsOn(void);

void LoadCustomer(void);
	

void ProcessCustomerVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cInfo"))
			sprintf(cInfo,"%.513s",entries[i].val);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cCompanyName,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cEmail"))
			sprintf(cEmail,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cSearch"))
			sprintf(cSearch,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"uClient"))
			sscanf(entries[i].val,"%u",&uClient);
		else if(!strcmp(entries[i].name,"uStep"))
			sscanf(entries[i].val,"%u",&uStep);
		else if(!strcmp(entries[i].name,"cNavList"))
			sprintf(cNavList,"%.8191s",entries[i].val);			
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


void CustomerGetHook(entry gentries[],int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uClient"))
			sscanf(gentries[i].val,"%u",&uClient);
	}

	if(gcCookieCustomer[0] && !uClient)
		uClient=uGetClient(gcCookieCustomer);
	
	LoadCustomer();
	htmlCustomer();

}//void CustomerGetHook(entry gentries[],int x)


void LoadCustomer(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT uClient,cLabel,cInfo,cEmail,uOwner,uCreatedBy,uCreatedDate,"
				"uModBy,uModDate FROM tClient WHERE uClient=%u",uClient);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&uClient);
		sprintf(cCompanyName,"%.100s",field[1]);			
		sprintf(cInfo,"%.513s",field[2]);
		sprintf(cEmail,"%.100s",field[3]);
		sscanf(field[4],"%u",&uOwner);
		sscanf(field[5],"%u",&uCreatedBy);
		sscanf(field[6],"%lu",&uCreatedDate);
		sscanf(field[7],"%u",&uModBy);
		sscanf(field[8],"%lu",&uModDate);

		if(strcmp(gcCookieCustomer,cCompanyName))
		{
			sprintf(gcCookieCustomer,"%.99s",cCompanyName);
			gcCookieZone[0]=0;
			guCookieResource=0;
			guCookieView=0;
			guCookieContact=0;
			SetSessionCookie();
		}
		//gcMessage="1 record(s) found";
	}
	else
	{
		if(gcCookieCustomer[0])
			gcMessage="<blink>Error: </blink>No records found.";
	}

	mysql_free_result(res);

}//void LoadCustomer(char *cuClient)


void CustomerCommands(pentry entries[], int x)
{
	if(!strcmp(gcPage,"Customer"))
	{
		ProcessCustomerVars(entries,x);
		if(!strcmp(gcFunction,"New"))
		{			
			sprintf(gcNewStep,"Confirm ");
			gcMessage="Enter/modify data, review, then confirm. Any other action to cancel.";
			gcInputStatus[0]=0;
			SetCustomerFieldsOn();
		}
		else if(!strcmp(gcFunction,"Confirm New"))
		{
			if(!ValidateCustomerInput())
			{
				sprintf(gcNewStep,"Confirm ");
				gcInputStatus[0]=0;
				htmlCustomer();
			}
			NewCustomer();
		}
		else if(!strcmp(gcFunction,"Modify"))
		{
			sprintf(gcModStep,"Confirm ");
			gcMessage="Enter/modify data, review, then confirm. Any other action to cancel.";
			SetCustomerFieldsOn();
			gcInputStatus[0]=0;
		}
		else if(!strcmp(gcFunction,"Confirm Modify"))
		{
			if(!uClient)
			{
				gcMessage="<blink>Error: </blink>Can't modify. No record selected";
				htmlCustomer();
			}
			if(!ValidateCustomerInput())
			{
				sprintf(gcModStep,"Confirm ");
				gcInputStatus[0]=0;
				htmlCustomer();
			}
			ModCustomer();
		}
		else if(!strcmp(gcFunction,"Delete"))
		{
			if(uHasZones(cCompanyName))
			{
				gcMessage="<blink>Error: </blink>Can't delete a Company  with zones";
				htmlCustomer();
			}
			else if(uHasBlocks(cCompanyName))
			{
				gcMessage="<blink>Error: </blink>Can't delete a Company with blocks";
				htmlCustomer();
			}
			else if(uHasContacts(cCompanyName))
			{
				gcMessage="<blink>Error: </blink>Can't delete a Company with contacts";
				htmlCustomer();
			}
			
			sprintf(gcDelStep,"Confirm ");
			gcMessage="Double check you have selected the correct record to delete."
					" Then confirm. Any other action to cancel.";
		}
		else if(!strcmp(gcFunction,"Confirm Delete"))
		{
			if(!uClient)
			{
				gcMessage="<blink>Error: </blink>No record selected";
				htmlCustomer();
			}
			DelCustomer();
		}
		else if(!strcmp(gcFunction,"Add Company Wizard"))
		{
			SetCustomerFieldsOn();
			htmlCustomerWizard(1);
		}
		else if(!strcmp(gcFunction,"Next"))
		{
			MYSQL_RES *res;
			
			SetCustomerFieldsOn();
			
			switch(uStep)
			{
				case 1:
					if(!cCompanyName[0])
					{
						gcMessage="<blink>Error: </blink>Company name can't be empty";
						cLabelStyle="type_fields_req";
						htmlCustomerWizard(1);
					}

					SelectCustomer(cCompanyName,0);
					res=mysql_store_result(&gMysql);
			
					if(mysql_num_rows(res))
					{
						mysql_free_result(res);
						gcMessage="<blink>Error: </blink>Company already exists";
						cLabelStyle="type_fields_req";
						htmlCustomerWizard(1);
					}
					
					break;
				case 2:
					if(cEmail[0])
					{
						if(strstr(cEmail,"@")==NULL || strstr(cEmail,".")==NULL)
						{
							cEmailStyle="type_fields_req";
							gcMessage="<blink>Error: </blink>Email has to be a valid email address";
							htmlCustomerWizard(2);
						}
					}
					break;
			}
			uStep++;
			htmlCustomerWizard(uStep);			
		}
		else if(!strcmp(gcFunction,"Finish"))
		{
			MYSQL_RES *res;
			SelectCustomer(cCompanyName,0);
			res=mysql_store_result(&gMysql);
			
			if(mysql_num_rows(res)==0)
			{
				SetCustomerFieldsOn();
				if(!ValidateCustomerInput())
				{					
					htmlCustomerWizard(4);
				}
				NewCustomer();
				htmlCustomer();
			}
			else
			{
				gcMessage="<blink>Error: </blink>Company already exists!";
				cLabelStyle="type_fields_req";
			}
			mysql_free_result(res);

			SetCustomerFieldsOn();
			htmlCustomerWizard(4);		
		}
		else if(!strcmp(gcFunction,"Cancel Wizard"))
		{
			cCompanyName[0]=0;
			cEmail[0]=0;
			cInfo[0]=0;
		}
		htmlCustomer();
	}

}//void CustomerCommands(pentry entries[], int x)


void htmlCustomerWizard(unsigned uStep)
{
	htmlHeader("idnsAdmin","Header");
	sprintf(gcQuery,"CustomerWizard.%u",uStep);
	htmlCustomerPage("idnsAdmin",gcQuery);
	htmlFooter("Footer");
	
}//void htmlCustomerWizard(unsigned uStep)


void htmlCustomer(void)
{
	if(cSearch[0])
	{
		FILE *fp;

		if((fp=fopen("/dev/null","w")))
		{
			funcCompanyNavList(fp,1);
			fclose(fp);
		}
	}

	htmlHeader("idnsAdmin","Header");
	htmlCustomerPage("idnsAdmin","Customer.Body");
	htmlFooter("Footer");

}//void htmlCustomer(void)


void htmlCustomerPage(char *cTitle, char *cTemplateName)
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
			char cuClient[16]={""};
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
			
			sprintf(cuClient,"%u",uClient);
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

			template.cpName[12]="cLabel";
			template.cpValue[12]=cCompanyName;

			template.cpName[13]="cEmail";
			template.cpValue[13]=cEmail;
			
			template.cpName[14]="cInfo";
			template.cpValue[14]=cInfo;

			template.cpName[15]="uModByForm";
			template.cpValue[15]=cuModByForm;

			template.cpName[16]="cLabelStyle";
			template.cpValue[16]=cLabelStyle;

			template.cpName[17]="cEmailStyle";
			template.cpValue[17]=cEmailStyle;

			template.cpName[18]="cNavList";
			template.cpValue[18]=cNavList;

			template.cpName[19]="cSearchStyle";
			template.cpValue[19]=cSearchStyle;
			
			template.cpName[20]="cuClient";
			template.cpValue[20]=cuClient;

			template.cpName[21]="uModDateForm";
			template.cpValue[21]=cuModDateForm;

			template.cpName[22]="cSearch";
			template.cpValue[22]=cSearch;

			template.cpName[23]="cInfoStyle";
			template.cpValue[23]=cInfoStyle;

			template.cpName[24]="cBtnStatus";
			if(uClient)
				template.cpValue[24]="";
			else
				template.cpValue[24]="disabled";

			template.cpName[25]="cCustomer";
			template.cpValue[25]=cCompanyName;

			template.cpName[26]="gcZone";
			template.cpValue[26]=gcZone;

			template.cpName[27]="uView";
			template.cpValue[27]=cuView;

			template.cpName[28]="uResource";
			template.cpValue[28]=cuResource;

			template.cpName[29]="uOwner";
			template.cpValue[29]=cuOwner;

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

			template.cpName[37]="";

			printf("\n<!-- Start htmlCustomerPage(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlCustomerPage(%s)  cuModBy=%s -->\n",cTemplateName,cuModByForm); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlCustomerPage()


void funcCustomerContacts(FILE *fp)
{
	MYSQL_RES *res;
        MYSQL_ROW field;
	unsigned uPerm=0;
	char *cPage="";

	fprintf(fp,"<!-- funcCustomerContacts(fp) Start -->\n");

	if(!uClient && !gcCookieCustomer[0]) return;

	if(!uClient && gcCookieCustomer[0])
		uClient=uGetClient(gcCookieCustomer);


	sprintf(gcQuery,"SELECT tClient.uClient,tClient.cLabel,tClient.cEmail,tClient.cInfo,"
			" tAuthorize.uPerm"
			" FROM tClient LEFT JOIN tAuthorize ON tAuthorize.uCertClient=tClient.uClient"
			" WHERE tClient.uOwner=%u"
			" ORDER BY cLabel",uClient);
		
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
       	while((field=mysql_fetch_row(res)))
	{
		if(field[4]!=NULL)
			sscanf(field[4],"%u",&uPerm);
		if(uPerm>7)
			cPage="Administrator";
		else
			cPage="CustomerUser";

		fprintf(fp,"<tr><td><a class=darkLink href=\"idnsAdmin.cgi?gcPage=%s"
			"&uClient=%s&cCustomer=%s&cZone=%s&uView=%s&uResource=%u\">%s</a></td><td>%s</td><td>%s</td></tr>",
			cPage,field[0],cCompanyName,gcZone,cuView,uResource,field[1],field[2],field[3]);
	}
	mysql_free_result(res);

	fprintf(fp,"<!-- funcCustomerContacts(fp)End -->\n");
	
}//void funcCustomerContacts(FILE *fp)


void SelectCustomer(char *cLabel,unsigned uMode)
{
	if(!uMode)
	{
		if(strstr(cLabel,"%")==NULL)
			sprintf(gcQuery,"SELECT %s FROM tClient WHERE cLabel='%s' AND (uOwner=1 OR uOwner=%u)",
					VAR_LIST_tClient,cLabel,guOrg);
		else
			sprintf(gcQuery,"SELECT %s FROM tClient WHERE cLabel LIKE '%s' AND (uOwner=1 OR uOwner=%u)"
					" ORDER BY cLabel",
					VAR_LIST_tClient,cLabel,guOrg);
	}
	else
	{
		sprintf(gcQuery,"SELECT %s FROM tClient WHERE uClient=%s",
				VAR_LIST_tClient,cLabel);
	}

	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	
}//void SelectCustomer(char *cLabel)


void NewCustomer(void)
{
	time(&uCreatedDate);

	sprintf(gcQuery,"INSERT INTO tClient SET cLabel='%s',cEmail='%s',cInfo='%s',cCode='Organization',"
				"uOwner=%u,uCreatedBy=%u,uCreatedDate=%lu",
			cCompanyName
			,cEmail
			,cInfo
			,guOrg
			,guLoginClient
			,uCreatedDate);

	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	uClient=mysql_insert_id(&gMysql);

	if(uClient)
	{
		gcMessage="New Company created OK";
		iDNSLog(uClient,"tClient","New");
	}
	else
	{
		gcMessage="New Company NOT created";
		iDNSLog(uClient,"tClient","New Fail");
	}
	uOwner=guOrg;
	uCreatedBy=guLoginClient;
	sprintf(gcCustomer,"%.99s",cCompanyName);
	
	//Set session cookie after creating new company
	sprintf(gcCookieCustomer,"%.99s",cCompanyName);
	//If there's a zone or a RR selected, will unselect
	gcZone[0]=0;
	cuView[0]=0;
	uResource=0;
	guCookieContact=0;
	SetSessionCookie();

}//void NewCustomer(void)


void ModCustomer(void)
{
	time(&uModDate);

	sprintf(gcQuery,"UPDATE tClient SET cLabel='%s',cEmail='%s',cInfo='%s',"
				"uModBy=%u,uModDate=%lu WHERE uClient=%u",
			cCompanyName
			,cEmail
			,cInfo
			,guLoginClient
			,uModDate
			,uClient);
	mysql_query(&gMysql,gcQuery);
	
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	if(mysql_affected_rows(&gMysql))
	{
		gcMessage="Company modified OK";
		iDNSLog(uClient,"tClient","Mod");
	}
	else
	{
		gcMessage="Company NOT modified";
		iDNSLog(uClient,"tClient","Mod Fail");
	}

	uModBy=guLoginClient;
	sprintf(gcCustomer,"%.99s",cCompanyName);

	sprintf(gcCookieCustomer,"%.99s",cCompanyName);
	SetSessionCookie();

}//void ModCustomer(void)


void DelCustomer(void)
{
	sprintf(gcQuery,"DELETE FROM tClient WHERE uClient=%u",uClient);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	if(mysql_affected_rows(&gMysql))
	{
		gcMessage="Company deleted OK";
		iDNSLog(uClient,"tClient","Del");
	}
	else
	{
		gcMessage="Company NOT deleted";
		iDNSLog(uClient,"tClient","Del Fail");
	}
	
	gcCustomer[0]=0;

	gcCookieCustomer[0]=0;
	gcCookieZone[0]=0;
	guCookieView=0;
	guCookieResource=0;
	guCookieContact=0;
	SetSessionCookie();

}//void DelCustomer(void)


unsigned uHasZones(char *cLabel)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	
	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s'",cLabel);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		mysql_free_result(res);

		sprintf(gcQuery,"SELECT uZone FROM tZone WHERE uOwner=%s",field[0]);
		mysql_query(&gMysql,gcQuery);

		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);

		if(mysql_num_rows(res))
		{
			mysql_free_result(res);
			return(1);
		}
	}
	mysql_free_result(res);

	return(0);	
				
}//unsigned uHasZones(char *cLabel)


unsigned uHasBlocks(char *cLabel)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	
	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s'",cLabel);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		mysql_free_result(res);

		sprintf(gcQuery,"SELECT uBlock FROM tBlock WHERE uOwner=%s",field[0]);
		mysql_query(&gMysql,gcQuery);

		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);

		if(mysql_num_rows(res))
		{
			mysql_free_result(res);
			return(1);
		}
	}
	mysql_free_result(res);

	return(0);
	
}//unsigned uHasBlocks(char *cLabel)


unsigned uHasContacts(char *cLabel)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	
	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s'",cLabel);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		mysql_free_result(res);

		sprintf(gcQuery,"SELECT uClient FROM tClient WHERE uOwner=%s",field[0]);
		mysql_query(&gMysql,gcQuery);

		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);

		if(mysql_num_rows(res))
		{
			mysql_free_result(res);
			return(1);
		}
	}
	mysql_free_result(res);

	return(0);
	
}//unsigned uHasContacts(char *cLabel)


unsigned ValidateCustomerInput(void)
{
	if(!cCompanyName[0])
	{
		SetCustomerFieldsOn();
		cLabelStyle="type_fields_req";
		gcMessage="<blink>Error: </blink>Customer Name can't be empty";
		return(0);
	}
	else
	{
		MYSQL_RES *res;
		//
		//Check for valid characters, no punctuation symbols allowed except '.' in Customer Name
		register int i;

		for(i=0;cCompanyName[i];i++)
		{
			if(!isalnum(cCompanyName[i]) && cCompanyName[i]!='.' && cCompanyName[i]!=' ' && cCompanyName[i]!='-')
			{
				SetCustomerFieldsOn();
				cLabelStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>Customer Name contains invalid characters";
				return(0);
			}
		}
		if(!strcmp(gcFunction,"Confirm New"))
		{
			sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s'",cCompanyName);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
			res=mysql_store_result(&gMysql);
			if(mysql_num_rows(res))
			{
				cLabelStyle="type_fields_req";
				gcMessage="<blink>Error: </blink>Customer Name already in use";
				return(0);
			}
			mysql_free_result(res);
		}
	}
	if(cEmail[0])
	{
		if(strstr(cEmail,"@")==NULL || strstr(cEmail,".")==NULL)
		{
			SetCustomerFieldsOn();
			cEmailStyle="type_fields_req";
			gcMessage="<blink>Error: </blink>Email has to be a valid email address";
			return(0);
		}
	}

	return(1);
		
}//unsigned ValidateInput(void)


void SetCustomerFieldsOn(void)
{
	cLabelStyle="type_fields";
	cEmailStyle="type_fields";
	cInfoStyle="type_textarea";

}//void SetCustomerFieldsOn(void)

char *cZoneLink(unsigned uZone,unsigned uOwner);
char *cGetViewLabel(char *cuView);

void funcZoneList(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uCount=0;
	unsigned uTotal=0;
	unsigned uHeader=0;
	char cZone[100]={""};
	char cPrevZone[100]={"ERROR"};
	unsigned uZone=0;
	unsigned a=0,b=0,c=0,d=0,e=0;
	register int i;         
	
	if(!uClient) return;
	
	fprintf(fp,"<!-- funcZoneList(fp) Start -->\n");

	sprintf(gcQuery,"SELECT tZone.cZone,tView.cLabel,tView.uView FROM tZone,tView WHERE "
			"tZone.uView=tView.uView AND tZone.uOwner='%u' AND cZone NOT LIKE '%%in-addr.arpa' "
			"ORDER BY tZone.cZone",uClient);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);
	
	uTotal=mysql_num_rows(res);
	
	if(!uTotal)
	{
		mysql_free_result(res);
		goto NextList;
	}
	fprintf(fp,"<a title='List of zones owned by the loaded company' class=inputLink href=\"#\" "
			"onClick=\"open_popup('?gcPage=Glossary&cLabel=Zone List')\"><strong>Zone List</strong></a><br>\n");
	uHeader=1;

	while((field=mysql_fetch_row(res)))
	{
		uCount++;
		fprintf(fp,"<a href=\"idnsAdmin.cgi?gcPage=Zone&cZone=%s&uView=%s&cCustomer=%s\" class=darkLink>%s[%s]</a><br>\n",
			field[0]
			,field[2]
			,cCompanyName
			,field[0]
			,field[1]
			);
		if(uCount>50)
		{
			fprintf(fp,"<br>Only the first 50 zones displayed (%u zones)\n",uTotal);
			break;
		}
	}
	
	mysql_free_result(res);

NextList:	
	sprintf(gcQuery,"SELECT DISTINCT tBlock.cLabel FROM tBlock WHERE tBlock.uOwner=%u ORDER BY cLabel LIMIT 301",uClient);
	
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	if(mysql_num_rows(res) && !uHeader)
		fprintf(fp,"<a title='List of zones owned by the loaded company' class=inputLink href=\"#\" "
				"onClick=\"open_popup('?gcPage=Glossary&cLabel=Block Zone List')\"><strong>Block Assigned Zone List</strong></a><br>\n");
	while((field=mysql_fetch_row(res)))
	{
		if(5!=sscanf(field[0],"%u.%u.%u.%u/%u",&a,&b,&c,&d,&e))
			continue;
		sprintf(cZone,"%u.%u.%u.in-addr.arpa",c,b,a);
		if(strcmp(cZone,cPrevZone))
		{
			sprintf(cPrevZone,"%.99s",cZone);

			if(a==0 || e==0 || b>255 || c>255 || d>255
					|| a>255 || b>255 || e<21) continue;
			switch(e)
			{
				case 23:
				case 22:
				case 21:
				//Expand these three cases with basic CIDR math
				for(i=0;i<(2<<(24-e-1));i++)
				{
					sprintf(cZone,"%u.%u.%u.in-addr.arpa",c+i,b,a);
					uZone=uGetuZone(cZone,"2");
					if(!uZone)
						continue;
					fprintf(fp,"%s",cZoneLink(uZone,uClient));
				}
				break;
				default:
				//24 or smaller see if continue above
				//Single class C rev zone
				sprintf(cZone,"%u.%u.%u.in-addr.arpa",c,b,a);
				uZone=uGetuZone(cZone,"2");
				if(!uZone)
						break;
				fprintf(fp,"%s",cZoneLink(uZone,uClient));
			}
		}//if distinct
	}
	mysql_free_result(res);
	fprintf(fp,"<!-- funcZoneList(fp) End -->\n");
	return;

}//void funcZoneList(FILE *fp)


char *cZoneLink(unsigned uZone,unsigned uOwner)
{
	static char cRet[512]={"error"};
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT cZone,uView FROM tZone WHERE uZone=%u",uZone);
	macro_mySQLRunAndStore(res);
	if((field=mysql_fetch_row(res)))
	{
		sprintf(cRet,"<a href=\"idnsAdmin.cgi?gcPage=Zone&cZone=%s&uView=%s&cCustomer=%s\" class=darkLink>%s[%s]</a><br>",
			field[0]
			,field[1]
			,ForeignKey("tClient","cLabel",uOwner)
			,field[0]
			,cGetViewLabel(field[1]));
	}
	else
		cRet[0]=0;
	
	mysql_free_result(res);
	return(cRet);

}//char *cZoneLink(unsigned uZone,unsigned uOwner)


void funcCompanyNavList(FILE *fp,unsigned uSetCookie)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uResults=0;
	
	if(!cSearch[0]) return;
	
	if(!guASPContact)
	        sprintf(gcQuery,"SELECT uClient,cLabel FROM tClient WHERE uClient!=1 AND "
				"cLabel LIKE '%s%%' AND (uClient=%u OR uOwner=%u) AND (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP') ORDER BY cLabel",
					cSearch,guOrg,guOrg);
	else
		sprintf(gcQuery,"SELECT uClient,cLabel FROM tClient WHERE uClient!=1 AND "
				"cLabel LIKE '%s%%' AND (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP') ORDER BY cLabel",cSearch);

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
			sscanf(field[0],"%u",&uClient);
			LoadCustomer();
			mysql_free_result(res);

			fprintf(fp,"<a class=darkLink href=\"idnsAdmin.cgi?gcPage=Customer&uClient=%s\">%s</a><br>\n",
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
			fprintf(fp,"<a class=darkLink href=\"idnsAdmin.cgi?gcPage="
					"Customer&uClient=%1$s\">%2$s</a><br>\n",field[0],field[1]);

			if(uCount==MAX_RESULTS) break;
		}
		if(uCount<uResults)
			fprintf(fp,"Only the first %u shown (%u results). If the company"
					" you are looking for is not in the list above "
					"please further refine your search.<br>",uCount,uResults);
		//We free result and return outside this if
	}
	else if(!uResults)
	{
		//Show no rcds found msg, free result, return}
		fprintf(fp,"No records found.<br>\n");
	}

	mysql_free_result(res);

}//void funcCompanyNavList(FILE *fp)
