/*
FILE 
	ipauth.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010-2012 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file in main source dir.
PURPOSE
	iDNS Administration (ASP) Interface
	program file.
*/
#include "interface.h"
#include <openisp/ucidr.h>

static char *cMassList={""};
static char cImportMsg[32762]={""}; //A 32k buffer will be enough, if not, truncate the data.
static unsigned uFormat=0;

static unsigned uTotalLines=0;
static unsigned uProcessed=0;
static unsigned uIgnored=0;

char *ParseTextAreaLines(char *cTextArea);//bulkop.c
void RIPEImport(void);

void CommitTransaction(void);

void htmlIPAuthReport(void);

int floorLog2(unsigned int n);


void ProcessIPAuthVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cMassList"))
			cMassList=entries[i].val;
		else if(!strcmp(entries[i].name,"uFormat"))
			sscanf(entries[i].val,"%u",&uFormat);
	}

}//void ProcessIPAuthVars(pentry entries[], int x)


void IPAuthGetHook(entry gentries[],int x)
{
/*	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"cZone"))
			sprintf(gcZone,"%.99s",gentries[i].val);
		else if(!strcmp(gentries[i].name,"uView"))
			sprintf(cuView,"%.15s",gentries[i].val);
	}
*/	
	htmlIPAuth();

}//void IPAuthGetHook(entry gentries[],int x)


void IPAuthCommands(pentry entries[], int x)
{
	if(!strcmp(gcPage,"IPAuth"))
	{
		ProcessIPAuthVars(entries,x);
		if(!strcmp(gcFunction,"IP Auth Import"))
		{
			if(!cMassList[0])
			{
				gcMessage="<blink>Error: </blink>You must enter data at the import panel";
				htmlIPAuth();
			}
			switch(uFormat)
			{
				case 1:
					RIPEImport();
					htmlIPAuthReport();
					break;
				default:
					gcMessage="<blink>Error:</blink> I don't know how to handle that format";
			}
		}
		else if(!strcmp(gcFunction,"Commit IP Auth Import"))
		{
			CommitTransaction();
		}
		htmlIPAuth();
	}

}//void IPAuthCommands(pentry entries[], int x)


void htmlIPAuth(void)
{
	htmlHeader("idnsAdmin","Header");
	htmlIPAuthPage("idnsAdmin","IPAuth.Body");
	htmlFooter("Footer");

}//void htmlIPAuth(void)


void htmlIPAuthReport(void)
{
	htmlHeader("idnsAdmin","Header");
	htmlIPAuthPage("idnsAdmin","IPAuthReport.Body");
	htmlFooter("Footer");

}//void htmlIPAuthReport(void)


void htmlIPAuthDetail(void)
{
	htmlHeader("idnsAdmin","Header");
	htmlIPAuthPage("idnsAdmin","IPAuthDetails.Body");
	htmlFooter("Footer");

}//void htmlIPAuthDetail(void)


void htmlIPAuthPage(char *cTitle, char *cTemplateName)
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

			sprintf(cuResource,"%u",uResource);

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
			template.cpValue[8]=gcZone;

			template.cpName[9]="gcMessage";
			template.cpValue[9]=gcMessage;

			template.cpName[10]="cuView";
			template.cpValue[10]=cuView;

			template.cpName[11]="cImportMsg";
			template.cpValue[11]=cImportMsg;

			template.cpName[12]="cCustomer";
			template.cpValue[12]=gcCustomer;

			template.cpName[13]="uResource";
			template.cpValue[13]=cuResource;

			template.cpName[14]="";

			printf("\n<!-- Start htmlIPAuthPage(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlIPAuthPage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlIPAuthPage()


unsigned CSVFileData(unsigned uClient,char *cName);
unsigned uDefaultClient=0;

void funcIPAuthReport(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cCompanyName[100]={""};
	unsigned uCompanyId=0;

	sprintf(gcQuery,"SELECT cBlock,uClient,cBlockAction,cOwnerAction FROM tTransaction "
			"WHERE (cBlockAction!='None' OR cOwnerAction!='None') ORDER BY uTransaction");
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	
	res=mysql_store_result(&gMysql);

	while((field=mysql_fetch_row(res)))
	{
		sscanf(field[1],"%u",&uCompanyId);
		if(uCompanyId!=uDefaultClient)
			CSVFileData(uCompanyId,cCompanyName);
		else
			sprintf(cCompanyName,"%s",ForeignKey("tClient","cLabel",uDefaultClient));

		fprintf(fp,"<tr><td align=center>%s</td><td align=center>%s</td>"
			"<td align=center>%s</td><td align=center>%s</td><td align=center>%s</td></tr>\n",
			field[0]
			,field[1]
			,cCompanyName
			,field[2]
			,field[3]
			);
	}

}//void funcIPAuthReport(FILE *fp)


//Import functions code begins here
//Will extend in the future for ARIN and LACNIC
//
#define NEW_BLOCK 1 //new block from scratch
#define MOD_BLOCK 2 //update block ownership
#define NA_BLOCK 3 //nothing to do
#define EXPAND_NOMOD 4 //expand block, keep ownership
#define EXPAND_MOD 5 //expand block, update ownership
#define REDUCE_NOMOD 6 //reduce block, keep ownership
#define REDUCE_MOD 7 //reduce block, update ownership

void CreateTransactionTable();
unsigned uGetBlockStatus(char *cBlock,char *cCompany);
unsigned uGetOwnerStatus(char *cCompany);
unsigned uClientCSVCheck(unsigned uClient);
void AddToRejectsTable(char *cLine);


void CreateTransactionTable()
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tTransaction ( uTransaction INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, "
			"cBlock VARCHAR(100) NOT NULL DEFAULT '',unique (cBlock,uOwner), uOwner INT UNSIGNED NOT NULL DEFAULT 0,"
			"index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, "
			"uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, "
			"cBlockAction VARCHAR(100) NOT NULL DEFAULT '', cOwnerAction VARCHAR(100) NOT NULL DEFAULT '',"
			"uClient INT UNSIGNED NOT NULL DEFAULT 0, cCompany VARCHAR(255) NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tIgnoredTransaction (uIgnoredTransaction INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, "
			"cLine VARCHAR(255) NOT NULL DEFAULT '')");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

}//void CreateTransactionTable()


void RIPEImport(void)
{
	char cLine[512]={"ERROR"};
	unsigned uLineNumber=0;
	char cIPBlock[100]={""};
	char cIPBlockStart[64]={""};
	char cIPBlockEnd[64]={""};
	unsigned uCidr=0;
	unsigned uSize=0;
	unsigned uClient=0;
	unsigned uOther=0;
	unsigned uDate=0;
	unsigned uBlockStatus=0;
	unsigned uOwnerStatus=0;
	char *cBlockAction="";
	char *cOwnerAction="";
	char cuDefaultClient[16]={""};
	char cCompany[100]={""};

	CreateTransactionTable();
	
	GetConfiguration("uDefaultClient",cuDefaultClient,1);
	sscanf(cuDefaultClient,"%u",&uDefaultClient);
	if(!uDefaultClient)
	{
		gcMessage="<blink>Error: </blink>Create a tConfiguration entry for uDefaultClient, it must contain the tClient.uClient value for your company";
		htmlIPAuth();
	}

	sprintf(gcQuery,"TRUNCATE tTransaction");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	sprintf(gcQuery,"TRUNCATE tIgnoredTransaction");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	
	while(1)
	{

		uLineNumber++;
		sprintf(cLine,"%.255s",ParseTextAreaLines(cMassList));
		//ParseTextAreaLines() required break;
		if(cLine[0]==0) break;
		//Comments ignore
		if(cLine[0]=='#') continue;
		if(cLine[0]==';') continue;
		uClient=0;
		uTotalLines++;
		if(strstr(cLine,"PKXG-CUST"))
		{
			uProcessed++;
			//80.253.98.0 - 80.253.98.255 256 20060404 PKXG-CUST-1234-01
			sscanf(cLine,"%s - %s %u %u PKXG-CUST-%u-%u",
				cIPBlockStart
				,cIPBlockEnd
				,&uSize
				,&uDate
				,&uClient
				,&uOther);
			if(!uClient)
			{
				uIgnored++;
				strcat(cLine," *** COMPANY NOT FOUND ***");
				AddToRejectsTable(cLine);
				continue;
			}
		}
		else if(strstr(cLine,"PKXG-MRP"))
		{
			uProcessed++;
			//89.167.255.0 - 89.167.255.255 256 20090805 PKXG-MRP-1234-01
			sscanf(cLine,"%s - %s %u %u PKXG-MRP-%u-%u",
				cIPBlockStart
				,cIPBlockEnd
				,&uSize
				,&uDate
				,&uClient
				,&uOther);

		}
		else if(strstr(cLine,"PKXG-INFRA"))
		{
			uProcessed++;
			char cUnused[100]={""};
			sscanf(cLine,"%s - %s %u %u PKXG-INFRA-%s",
				cIPBlockStart
				,cIPBlockEnd
				,&uSize
				,&uDate
				,cUnused);
			uClient=uDefaultClient;
		}
		else if(strstr(cLine,"-DELE"))
		{
			//Managed outside iDNS, skip
			uIgnored++;
			continue;
		}
		else if(1)
		{
			//Invalid, skip
			uIgnored++;
			AddToRejectsTable(cLine);
			continue;
		}

		//Common processing
		if(!uClientCSVCheck(uClient))
		{
			//Add record to rejects table
			strcat(cLine," *** COMPANY NOT FOUND ***");
			AddToRejectsTable(cLine);
			uIgnored++;
			continue;
		}
		//uCidr=(unsigned)(32-log2(uSize));
		uCidr=(unsigned)(32-floorLog2(uSize));
		
		/*printf("cIPBlockStart='%s' cIPBlockEnd='%s' uSize=%u uDate=%u uClient=%u uOther=%u uCidr=%u\n",
			cIPBlockStart
			,cIPBlockEnd
			,uSize
			,uDate
			,uClient
			,uOther
			,uCidr);
		*/
		sprintf(cIPBlock,"%s/%u",cIPBlockStart,uCidr);
		
		if(uClient!=uDefaultClient)
			CSVFileData(uClient,cCompany);
		else
			sprintf(cCompany,"%.99s",ForeignKey("tClient","cLabel",uClient));

		uBlockStatus=uGetBlockStatus(cIPBlock,cCompany);
		uOwnerStatus=uGetOwnerStatus(cCompany);

		switch(uBlockStatus)
		{
			case NEW_BLOCK:
				cBlockAction="New";
			break;

			case MOD_BLOCK:
				cBlockAction="Update Ownership";
			break;

			case EXPAND_NOMOD:
				cBlockAction="Expand Keep Owner";
			break;

			case EXPAND_MOD:
				cBlockAction="Expand Update Owner";
			break;

			case REDUCE_NOMOD:
				cBlockAction="Reduce Keep Owner";
			break;

			case REDUCE_MOD:
				cBlockAction="Reduce Update Owner";
			break;

			case NA_BLOCK:
				cBlockAction="None";
			break;
		}
		switch(uOwnerStatus)
		{
			case NEW_BLOCK:
				cOwnerAction="New";
			break;

			case NA_BLOCK:
				cOwnerAction="None";
			break;
		}
		//printf("IP Block Label=%s uClient=%u cBlockAction=%s cOwnerAction=%s\n"
		//	,cIPBlock,uClient,cBlockAction,cOwnerAction);

		//if(!strcmp(cOwnerAction,"None")&&!strcmp(cBlockAction,"None")) continue; //No record if nothing to do
		
		sprintf(gcQuery,"INSERT INTO tTransaction SET cBlock='%s',cBlockAction='%s',cOwnerAction='%s',"
				"uClient=%u,cCompany='%s',uCreatedBy=%u,uOwner=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
				cIPBlock
				,cBlockAction
				,cOwnerAction
				,uClient
				,cCompany
				,guLoginClient
				,guOrg
				);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(gcQuery);
	}
	//Lock tTransaction for writing?

}//void RIPEImport(void)


void AddToRejectsTable(char *cLine)
{
	sprintf(gcQuery,"INSERT INTO tIgnoredTransaction SET cLine='%s'",cLine);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

}//void AddToRejectsTable(char *cLine)


unsigned uGetBlockStatus(char *cBlock,char *cCompany)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uOwner=0;
	char cIPBlock[64]={""};
	unsigned uCIDR=0;
	unsigned uDbCIDR=0;
	unsigned uUpdateOwner=0;
	unsigned uAction=0;
	char *cp;

#define BLOCK_EXPAND 1
#define BLOCK_REDUCE 2
#define BLOCK_NONE 3

	//This function should check if:
	//Are we creating a brand new block?
	//Are we expanding an existent block and keeping ownership intact?
	//Are we expanding an existent block and updating ownership?
	//Are we reducing an existent block and keeping ownership intact?
	//Are we reducing an existent block and updating ownership?
	
	sprintf(cIPBlock,"%s",cBlock);

	if((cp=strchr(cBlock,'/')))
		sscanf(cp+1,"%u",&uCIDR);
	if((cp=strchr(cIPBlock,'/'))) *cp=0;
	sprintf(gcQuery,"SELECT cLabel,uOwner FROM tBlock WHERE cLabel LIKE '%s/%%'",cIPBlock);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		if((cp=strchr(field[0],'/')))
			sscanf(cp+1,"%u",&uDbCIDR);
		sscanf(field[1],"%u",&uOwner);

		mysql_free_result(res);
		//Does ownership change?
		if(strcmp(ForeignKey("tClient","cLabel",uOwner),cCompany)) uUpdateOwner=1;
		if(uCIDR<uDbCIDR)
			//Block is being expanded
			uAction=BLOCK_EXPAND;
		else if(uCIDR>uDbCIDR)
			uAction=BLOCK_REDUCE;
		else if(uCIDR==uDbCIDR)
			uAction=BLOCK_NONE;
		
		if((uAction==BLOCK_NONE)&&!uUpdateOwner)
			return(NA_BLOCK);
		else if((uAction==BLOCK_NONE)&&uUpdateOwner)
			return(MOD_BLOCK);
		else if((uAction==BLOCK_EXPAND)&&!uUpdateOwner)
			return(EXPAND_NOMOD);
		else if((uAction==BLOCK_EXPAND)&&uUpdateOwner)
			return(EXPAND_MOD);
		else if((uAction==BLOCK_REDUCE)&&!uUpdateOwner)
			return(REDUCE_NOMOD);
		else if((uAction==BLOCK_REDUCE)&&uUpdateOwner)
			return(REDUCE_MOD);
	}
	else
	{
		mysql_free_result(res);
		return(NEW_BLOCK);
	}

	return(0); 

}//unsigned uGetBlockStatus(char *cBlock)


unsigned uGetOwnerStatus(char *cCompany)
{
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s'",cCompany);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	res=mysql_store_result(&gMysql);

	if(!mysql_num_rows(res))
	{
		mysql_free_result(res);
		return(NEW_BLOCK);
	}
	else
	{
		mysql_free_result(res);
		return(NA_BLOCK);
	}

	return(0);

}//unsigned uGetOwnerStatus(unsigned uClient)


unsigned uClientCSVCheck(unsigned uClient)
{
	return(CSVFileData(uClient,NULL));

}

unsigned CSVFileData(unsigned uClient,char *cName)
{
	FILE *fp;
	char cCompanyCSVLocation[100]={"/usr/local/idns/csv/companycode.csv"};
	unsigned uFileClient=0;
	char cLabel[100]={""};

	if(uClient==uDefaultClient) return(1); //exception ;)

	//Open CSV file at fixed location
	fp=fopen(cCompanyCSVLocation,"r");
	if(fp==NULL)
		htmlPlainTextError("Could not open CSV file for companies");
	
	//Search for uClient at CSV file
	while(fgets(gcQuery,2048,fp)!=NULL)
	{
		sscanf(gcQuery,"%u,%s",&uFileClient,cLabel);
		if(cName!=NULL) sprintf(cName,"%.65s",cLabel);
		if(uClient==uFileClient)
		{
			fclose(fp);
			return(1);
		}
	}
	fclose(fp);
	return(0);

}//unsigned uClientCSVCheck(unsigned uClient)

//
//End data processing functions
//

//
//Begin data commit functions
//

void funcReportActions(FILE *fp)
{
	MYSQL_RES *res;
	unsigned uWillDeleteCompanies=0;
	unsigned uWillDeleteBlocks=0;
	unsigned uWillCreateBlocks=0;
	unsigned uWillModBlocks=0;
	unsigned uWillExpandBlocks=0;
	unsigned uWillReduceBlocks=0;
	unsigned uWillCreateCompanies=0;
	unsigned uImportCompanies=0;

	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel NOT IN "
		"(SELECT DISTINCT cCompany FROM tTransaction) AND "
		"uClient!=1 AND uClient!=%u AND (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP')",uDefaultClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	uWillDeleteCompanies=mysql_num_rows(res);
	
	mysql_free_result(res);

	sprintf(gcQuery,"SELECT uBlock FROM tBlock WHERE uOwner IN (SELECT uClient FROM tClient WHERE cLabel NOT IN "
		"(SELECT DISTINCT cCompany FROM tTransaction) AND "
		"uClient!=1 AND uClient!=%u AND (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP'))",uDefaultClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	uWillDeleteBlocks=mysql_num_rows(res);
	
	mysql_free_result(res);
	
	sprintf(gcQuery,"SELECT uTransaction FROM tTransaction WHERE cBlockAction='New'");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	uWillCreateBlocks=mysql_num_rows(res);
	
	mysql_free_result(res);
	
	sprintf(gcQuery,"SELECT uTransaction FROM tTransaction WHERE cBlockAction='Update Ownership'");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	uWillModBlocks=mysql_num_rows(res);
	
	mysql_free_result(res);

	sprintf(gcQuery,"SELECT uTransaction FROM tTransaction WHERE cBlockAction LIKE 'Expand%%'");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	uWillExpandBlocks=mysql_num_rows(res);
	
	mysql_free_result(res);

	sprintf(gcQuery,"SELECT uTransaction FROM tTransaction WHERE cBlockAction LIKE 'Reduce%%'");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	uWillReduceBlocks=mysql_num_rows(res);
	
	mysql_free_result(res);

	sprintf(gcQuery,"SELECT uTransaction FROM tTransaction WHERE cOwnerAction='New'");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	uWillCreateCompanies=mysql_num_rows(res);
	
	mysql_free_result(res);

	sprintf(gcQuery,"SELECT DISTINCT cCompany FROM tTransaction");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	uImportCompanies=mysql_num_rows(res);
	
	mysql_free_result(res);

	fprintf(fp,"After import, %u companies and their contacts will be removed from the database.<br>\n",uWillDeleteCompanies);
	fprintf(fp,"These companies own %u blocks that will also be removed.<br>\n",uWillDeleteBlocks);
	fprintf(fp,"%u companies and a default contact will be added to the database. %u blocks will be created "
			"%u blocks ownership will be updated, %u blocks will be expanded and %u blocks will be reduced.<br>\n",
			uWillCreateCompanies
			,uWillCreateBlocks
			,uWillModBlocks
			,uWillExpandBlocks
			,uWillReduceBlocks
			);
	fprintf(fp,"%u companies were found in the import data.<br>\n",uImportCompanies);
	//fprintf(fp,"From a total of %u lines, %u lines were correctly processed and %u ignored.<br>\n",uTotalLines,uProcessed,uIgnored);

}//void funcReportActions(FILE *fp)


void funcRemovedCompanies(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT uClient,cLabel FROM tClient WHERE cLabel NOT IN "
		"(SELECT DISTINCT cCompany FROM tTransaction) AND "
		"uClient!=1 AND uClient!=%u AND (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP') ORDER BY cLabel",uDefaultClient);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	if(!mysql_num_rows(res))
	{
		fprintf(fp,"<tr><td colspan=2>None</td>\n");
		mysql_free_result(res);
		return;
	}

	while((field=mysql_fetch_row(res)))
		fprintf(fp,"<tr><td>%s</td><td>%s</td>\n",field[0],field[1]);
	
	mysql_free_result(res);

}//void funcRemovedCompanies(FILE *fp)


void funcRemovedBlocks(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT tBlock.cLabel,tClient.cLabel " 
		"FROM tBlock,tClient WHERE tBlock.uOwner IN (SELECT uClient FROM tClient WHERE cLabel NOT IN "
		"(SELECT DISTINCT cCompany FROM tTransaction) AND "
		"uClient!=1 AND uClient!=%u AND (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP') ) AND tClient.uClient=tBlock.uOwner ORDER BY tClient.cLabel",uDefaultClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	if(!mysql_num_rows(res))
	{
		fprintf(fp,"<tr><td colspan=2>None</td>\n");
		mysql_free_result(res);
		return;
	}
	while((field=mysql_fetch_row(res)))
		fprintf(fp,"<tr><td>%s</td><td>%s</td>\n",field[0],field[1]);

	mysql_free_result(res);

}//void funcRemovedBlocks(FILE *fp)


void funcIgnoredLines(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char *cColor="";

	sprintf(gcQuery,"SELECT cLine FROM tIgnoredTransaction");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	if(!mysql_num_rows(res))
	{
		fprintf(fp,"None\n");
		mysql_free_result(res);
		return;
	}
	while((field=mysql_fetch_row(res)))
	{
		if(strstr(field[0],"COMPANY NOT FOUND")) 
			cColor="color=red";
		else
			cColor="";

		fprintf(fp,"<font %s>%s</font><br>\n",cColor,field[0]);
	}
	
	mysql_free_result(res);

}//void funcIgnoredLines(FILE *fp)

/*
void CommitTransaction(void)
{
	//Submit meta job entry for commiting transaction
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT cFQDN,tServer.cLabel,tNSType.cLabel FROM tNS,tNSSet,tServer,tNSType "
			"WHERE (tNS.uNSType=1 OR tNS.uNSType=2) AND tNS.uServer=tServer.uServer "
			"AND tNS.uNSType=tNSType.uNSType "
			"AND tNS.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=1 LIMIT 1");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sprintf(gcQuery,"INSERT INTO tJob SET cJob='IPAuthCommit',uNSSet=1,cTargetServer='%s %s',"
				"uTime=UNIX_TIMESTAMP(NOW()),uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
				field[0]
				,field[2]
				,guOrg
				,guLoginClient
				);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));

	}
	else
		htmlPlainTextError("Couldn't find master or hidden master for uNSSet=1");

}//void CommitTransaction(void)
*/
int floorLog2(unsigned int n)
{
	int pos = 0;
	if (n >= 1<<16) { n >>= 16; pos += 16; }
	if (n >= 1<< 8) { n >>=  8; pos +=  8; }
	if (n >= 1<< 4) { n >>=  4; pos +=  4; }
	if (n >= 1<< 2) { n >>=  2; pos +=  2; }
	if (n >= 1<< 1) {           pos +=  1; }
	return ((n == 0) ? (-1) : pos);
}

void CleanUpBlock(char *cIPBlock);
void CleanUpCompanies(void);
void EncryptPasswd(char *cPasswd);
char *cGetRandomPassword(void);
unsigned ProcessTransaction(char *cIPBlock,char *cCompany,char *cAction);
unsigned ProcessCompanyTransaction(char *cCompany,char *cAction);
unsigned uCreateZone(char *cZone,unsigned uOwner);
void CreateDefaultRR(unsigned uName,char *cParam1,unsigned uZone,unsigned uOwner);
void ResetRR(char *cZone,unsigned uName,char *cParam1,unsigned uOwner);
unsigned GetuZone(char *cLabel, char *cTable);
MYSQL_RES *ZoneQuery(char *cZone);
void UpdateSerialNum(char *cZone,char *cuView);
void RestoreUpdateSerialNum(unsigned uZone); //UpdateSerialNum like in backend, used elsewhere and reused here 
//Action counters
unsigned uBlockAdd=0;
unsigned uBlockMod=0;
unsigned uBlockDel=0;
unsigned uBlockReduce=0;
unsigned uBlockExpand=0;
unsigned uCompanyAdd=0;
unsigned uCompanyDel=0;


void CommitTransaction(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cMsg[100]={""};
	char cuDefaultClient[16]={""};

	GetConfiguration("uDefaultClient",cuDefaultClient,1);
	sscanf(cuDefaultClient,"%u",&uDefaultClient);

	sprintf(gcQuery,"SELECT DISTINCT cCompany FROM tTransaction WHERE cOwnerAction='New' ORDER BY uTransaction");
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	
	res=mysql_store_result(&gMysql);

	while((field=mysql_fetch_row(res)))
	{		
		if(ProcessCompanyTransaction(field[0],"New"))
			uCompanyAdd++;
		/*else
		{
			sprintf(gcQuery,"Failed ProcessCompanyTransaction() for %s",field[0]);
			logfileLine("CommitTransaction()",gcQuery);
		}*/
//		printf("ProcessCompanyTransaction(%s,New)\n",field[0]);
	}

	mysql_free_result(res);	

	sprintf(gcQuery,"SELECT cBlock,cCompany,cBlockAction FROM tTransaction WHERE cBlockAction!='None' ORDER BY uTransaction");
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	
	res=mysql_store_result(&gMysql);
//printf("Content-type: text/plain\n\n");

	while((field=mysql_fetch_row(res)))
	{
//		printf("ProcessTransaction(%s,%s,%s)",field[0],field[1],field[2]);
		ProcessTransaction(field[0],field[1],field[2]);
//		printf("...OK\n");
	}
	mysql_free_result(res);
//	exit(0);
//	printf("CleanUpCompanies()");
	CleanUpCompanies();
//	printf("...OK");
	sprintf(cImportMsg,"Added %u block(s)\n",uBlockAdd);

	sprintf(cMsg,"Modified %u block(s)\n",uBlockMod);
	strcat(cImportMsg,cMsg);

	sprintf(cMsg,"Deleted %u block(s)\n",uBlockDel);
	strcat(cImportMsg,cMsg);
	
	sprintf(cMsg,"Expanded %u block(s)\n",uBlockExpand);
	strcat(cImportMsg,cMsg);

	sprintf(cMsg,"Reduced %u block(s)\n",uBlockReduce);
	strcat(cImportMsg,cMsg);

	if(uCompanyAdd>1)
		sprintf(cMsg,"Added %u companies\n",uCompanyAdd);
	else if(uCompanyAdd==1)
		sprintf(cMsg,"Added %u company\n",uCompanyAdd);
	else if(uCompanyAdd==0)
		sprintf(cMsg,"Didn't add any company\n");
	strcat(cImportMsg,cMsg);

	if(uCompanyDel>1)
		sprintf(cMsg,"Deleted %u companies and their contacts\n",uCompanyDel);
	else if(uCompanyDel==1)
		sprintf(cMsg,"Deleted %u company and their contacts\n",uCompanyDel);
	else if(uCompanyDel==0)
		sprintf(cMsg,"Didn't delete any company\n");
	strcat(cImportMsg,cMsg);


}//void CommitTransaction(void)


unsigned uCreateZone(char *cZone,unsigned uOwner)
{
	//This function creates a new zone
	//sets it serial number
	//and submits a new job
	//
	unsigned uZone=0;
	time_t luClock;

	time(&luClock);

	sprintf(gcQuery,"INSERT INTO tZone SET cZone='%s',uNSSet=1,cHostmaster='%s',"
			"uSerial=0,uExpire=604800,uRefresh=28800,uTTL=86400,"
			"uRetry=7200,uZoneTTL=86400,uMailServers=0,uView=2,uOwner=%u,"
			"uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cZone
			,HOSTMASTER
			,uOwner
			,guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	uZone=mysql_insert_id(&gMysql);
	RestoreUpdateSerialNum(uZone);
	//Submit new job
	if(AdminSubmitJob("New",1,cZone,0,luClock))
			htmlPlainTextError(gcQuery);
	
	return(uZone);

}//unsigned uCreateZone(char *cZone,unsigned uOwner)


void CreateDefaultRR(unsigned uName,char *cParam1,unsigned uZone,unsigned uOwner)
{
	//Avoid dup records
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uResource FROM tResource WHERE cName=%u AND uRRType=7 AND uZone=%u",
			uName
			,uZone
			);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	res=mysql_store_result(&gMysql);

	if(!mysql_num_rows(res))
	{
		sprintf(gcQuery,"INSERT INTO tResource SET "
				"cName=%u,uRRType=7,cParam1='%s',uZone=%u,"
				"uCreatedBy=%u,uOwner=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
				uName
				,cParam1
				,uZone
				,guLoginClient
				,uOwner
				);
		mysql_query(&gMysql,gcQuery);
	
		if(mysql_errno(&gMysql))
			htmlPlainTextError(gcQuery);
	}

}//void CreateDefaultRR(unsigned uName,char *cParam1,unsigned uZone,unsigned uOwner)


MYSQL_RES *ZoneQuery(char *cZone)
{
	static MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=2",cZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	res=mysql_store_result(&gMysql);
	return(res);

}//MYSQL_RES *ZoneQuery(void)


void UpdateBlockOwnership(char *cIPBlock,unsigned uOwner)
{
	sprintf(gcQuery,"UPDATE tBlock SET uOwner=%u,uModBy=%u,"
			"uModDate=UNIX_TIMESTAMP(NOW()) WHERE cLabel='%s'",
			uOwner
			,guLoginClient
			,cIPBlock);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

}//void UpdateBlockOwnership(char *cIPBlock,unsigned uOwner)


unsigned uGetDbBlock(char *cIPBlock)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT cLabel,uOwner FROM tBlock WHERE cLabel LIKE '%s/%%'",cIPBlock);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sprintf(cIPBlock,"%s",field[0]);
	
	return(0);
	
}//unsigned uGetDbBlock(char *cIPBlock)


void CreateBlock(char *cIPBlock,unsigned uClient)
{
	sprintf(gcQuery,"INSERT INTO tBlock SET cLabel='%s',uOwner=%u,"
			"uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cIPBlock
			,uClient
			,guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

}//void CreateBlock(char *cIPBlock,unsigned uClient)


void RemoveOldBlock(char *cIPBlock)
{
	char cLocalIPBlock[100]={""};
	char *cp;
	sprintf(cLocalIPBlock,"%.99s",cIPBlock);
	
	if((cp=strchr(cLocalIPBlock,'/'))) *cp=0;
	sprintf(gcQuery,"DELETE FROM tBlock WHERE cLabel LIKE '%s/%%'",cLocalIPBlock);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

}//void RemoveOldBlock(char *cIPBlock)


unsigned uZoneSetup(char *cZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uZone=0;

	res=ZoneQuery(cZone);
	if(!mysql_num_rows(res))
	{
		uZone=uCreateZone(cZone,uDefaultClient);
	}
	else
	{
		field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uZone);
	}
	mysql_free_result(res);

	return(uZone);

}//unsigned uZoneSetup(char *cZone)


void UpdateBlockOwner(char *cIPBlock,unsigned uClient)
{
	unsigned a,b,c,d,e;
	unsigned uNumNets=0;
	unsigned uNumIPs=0;
	unsigned uZone=0;
	char cZone[100]={""};

	sscanf(cIPBlock,"%u.%u.%u.%u/%u",&a,&b,&c,&d,&e);
	uNumIPs=uGetNumIPs(cIPBlock);
	uNumNets=uGetNumNets(cIPBlock);
	
	if(uNumNets==1)
	{
		sprintf(cZone,"%u.%u.%u.in-addr.arpa",c,b,a);
		uZone=uZoneSetup(cZone);
		sprintf(gcQuery,"UPDATE tResource SET uOwner=%u WHERE uZone=%u AND uOwner!=%u",
				uClient
				,uZone
				,uClient
			);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
	}
	else
	{
		register int x;
		for(x=c;x<(c+uNumNets);x++)
		{
			//
			sprintf(cZone,"%u.%u.%u.in-addr.arpa",x,b,a);
			//printf("cZone=%s\n",cZone);
			uZone=uZoneSetup(cZone);
			sprintf(gcQuery,"UPDATE tResource SET uOwner=%u WHERE uZone=%u AND uOwner!=%u",
					uClient
					,uZone
					,uClient
				);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
		}
	}

}//void UpdateBlockOwner(char *cIPBlock,unsigned uClient)


unsigned ProcessTransaction(char *cIPBlock,char *cCompany,char *cAction)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	register unsigned f;
	unsigned a,b,c,d,e;
	unsigned uNumNets=0;
	unsigned uNumIPs=0;
	unsigned uZone=0;
	unsigned uClient=0;
	unsigned uDbIPs=0;
	unsigned uDBNets=0;
	
	char cDbBlock[64]={""};
	char cZone[100]={""};
	char cParam1[200]={""};
	char cUpdateHost[100]={"packetexchange.net"}; //This will come from tConfiguration, later

	time_t luClock;

	sscanf(cIPBlock,"%u.%u.%u.%u/%u",&a,&b,&c,&d,&e);
	uNumIPs=uGetNumIPs(cIPBlock);
	uNumNets=uGetNumNets(cIPBlock);
	sprintf(cDbBlock,"%u.%u.%u.%u",a,b,c,d);
	uGetDbBlock(cDbBlock);
	//printf("Alleged db block is %s\n",cDbBlock);

	uDbIPs=uGetNumIPs(cDbBlock);
	uDBNets=uGetNumNets(cDbBlock);
	

	time(&luClock);
	//printf("uNumIPs=%u\n",uNumIPs);
	//printf("uNumNets=%u\n",uNumNets);
	
	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel='%s' AND (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP')",cCompany);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uClient);
	else
	{
		mysql_free_result(res);
		return(1);
	}
	mysql_free_result(res);
	//printf("\ncAction=%s cIPBlock=%s\n",cAction,cIPBlock);

	if(!strcmp(cAction,"New"))
	{
		//
		//Create tBlock entry owned by uClient
		CreateBlock(cIPBlock,uClient);

		uBlockAdd++;
		//printf("Created block: %s (%u)\n",cIPBlock,(unsigned)mysql_insert_id(&gMysql));
		if(uNumNets==1)
		{
			//24 and smaller blocks
			sprintf(cZone,"%u.%u.%u.in-addr.arpa",c,b,a);
			uZone=uZoneSetup(cZone);
			
			//Create block default RRs uOwner=uClient
			//
			//printf("d=%u uNumIPs=%u\n",d,uNumIPs);
			if(d==0)d++;
			for(f=d;f<(uNumIPs+d);f++)
			{
				sprintf(cParam1,"%u-%u-%u-%u.%s",f,c,b,a,cUpdateHost);
				CreateDefaultRR(f,cParam1,uZone,uClient);
				//printf("Creating RR cName=%i\n",f);
			}
			//Update zone serial
			RestoreUpdateSerialNum(uZone);
			//printf("Called RestoreUpdateSerialNum()\n");
			//Submit mod job
			//Default uNSSet=1 ONLY
			if(AdminSubmitJob("Mod",1,cZone,0,luClock+300))
					htmlPlainTextError(gcQuery);
			//printf("Submitted job\n");
		}//if(uNumNets==1)
		else
		{
			register int x;
			//Larger than /24 blocks
			for(x=c;x<(c+uNumNets);x++)
			{
				//
				sprintf(cZone,"%u.%u.%u.in-addr.arpa",x,b,a);
				//printf("cZone=%s\n",cZone);
				uZone=uZoneSetup(cZone);
			
				//Create block default RRs uOwner=uClient
				//
				if(d==0)d++;
				for(f=d;f<255;f++)
				{
					sprintf(cParam1,"%u-%u-%u-%u.%s",f,c,b,a,cUpdateHost);
					CreateDefaultRR(f,cParam1,uZone,uClient);
				}
				//Update zone serial
				RestoreUpdateSerialNum(uZone);
				//Submit mod job
				//Default uNSSet=1 ONLY
				if(AdminSubmitJob("Mod",1,cZone,0,luClock+300))
					htmlPlainTextError(gcQuery);
			}//for(f=c;f<((c+uNumNets));f++)
		}
	}
	else if(!strcmp(cAction,"Update Ownership"))
	{
		//
		//Update tBlock uOwner
		UpdateBlockOwnership(cIPBlock,uClient);
		
		uBlockMod++;

		if(uNumNets==1)
		{
			//24 and smaller blocks
			sprintf(cZone,"%u.%u.%u.in-addr.arpa",c,b,a);
			//Check for .arpa zone if it doesn't exist, create it
			//owned by uDefaultClient
			uZone=uZoneSetup(cZone);

			//Update zone RRs
			//to be owned by uClient
			//
			if(d==0)d++;
			for(f=d;f<(uNumIPs+d);f++)
			{
				//Update to default cParam1 or just uOwner update?
				sprintf(cParam1,"%u-%u-%u-%u.%s",f,c,b,a,cUpdateHost);
				ResetRR(cZone,f,cParam1,uClient);
			}
			//Update zone serial
			RestoreUpdateSerialNum(uZone);
			//Submit Mod job for zone
			if(AdminSubmitJob("Mod",1,cZone,0,luClock+300))
				htmlPlainTextError(gcQuery);
		}//if(uNumNets==1)
		else
		{
			register int x;
			//Larger than /24 blocks
			
			for(x=c;x<(c+uNumNets);x++)
			{
				//
				sprintf(cZone,"%u.%u.%u.in-addr.arpa",x,b,a);
				//printf("cZone=%s\n",cZone);
				uZone=uZoneSetup(cZone);
				
				for(f=d;f<254;f++)
				{
					sprintf(cParam1,"%u-%u-%u-%u.%s",f,x,b,a,cUpdateHost);
					ResetRR(cZone,f,cParam1,uClient);
				}
				//Update zone serial
				RestoreUpdateSerialNum(uZone);
				//Submit Mod job for zone
				if(AdminSubmitJob("Mod",1,cZone,0,luClock+300))
					htmlPlainTextError(gcQuery);
			}
		}
	}
	
	if(strstr(cAction,"Expand "))
	{
		unsigned uNetsToAdd=0;
		unsigned uRRToAddCount=0;
		//printf("RemoveOldBlock(%s)\n",cIPBlock);
		RemoveOldBlock(cIPBlock);
		//printf("CreateBlock(%s,%u)\n",cIPBlock,uClient);
		CreateBlock(cIPBlock,uClient);
		
		if(uNumNets==1)
		{
			uRRToAddCount=uNumIPs-uDbIPs;

			sprintf(cZone,"%u.%u.%u.in-addr.arpa",c,b,a);
			uZone=uZoneSetup(cZone);
			
			//Create block default RRs uOwner=uClient
			//
			//printf("d=%u uNumIPs=%u\n",d,uNumIPs);
			for(f=(d+uDbIPs);f<(uNumIPs+d);f++)
			{
				sprintf(cParam1,"%u-%u-%u-%u.%s",f,c,b,a,cUpdateHost);
				CreateDefaultRR(f,cParam1,uZone,uClient);
				//printf("Creating RR cName=%i\n",f);
			}
			//Update zone serial
			RestoreUpdateSerialNum(uZone);
			//printf("Called RestoreUpdateSerialNum()\n");
			//Submit mod job
			//Default uNSSet=1 ONLY
			if(AdminSubmitJob("Mod",1,cZone,0,luClock+300))
					htmlPlainTextError(gcQuery);
		}
		else
		{
			register int x;

			uNetsToAdd=uNumNets-uDBNets;
			/*printf("uNetsToAdd=%u\n",uNetsToAdd);
			printf("uNumNets=%u\n",uNumNets);
			printf("uDBNets=%u\n",uDBNets);
			*/
			//Larger than /24 blocks
			for(x=c;x<(c+uNetsToAdd);x++)
			{
				//
				sprintf(cZone,"%u.%u.%u.in-addr.arpa",x,b,a);
				//printf("cZone=%s\n",cZone);
				uZone=uZoneSetup(cZone);
			
				//Create block default RRs uOwner=uClient
				//
				if(d==0)d++;
				for(f=d;f<255;f++)
				{
					sprintf(cParam1,"%u-%u-%u-%u.%s",f,c,b,a,cUpdateHost);
					CreateDefaultRR(f,cParam1,uZone,uClient);
				}
				//Update zone serial
				RestoreUpdateSerialNum(uZone);
				//Submit mod job
				//Default uNSSet=1 ONLY
				if(AdminSubmitJob("Mod",1,cZone,0,luClock+300))
					htmlPlainTextError(gcQuery);
			}//for(f=c;f<((c+uNumNets));f++)
		}

		//Check if we are keeping owner or not and update as required (the old RRs only)
		if(!strstr(cAction,"Keep Owner"))
			UpdateBlockOwner(cIPBlock,uClient);

		uBlockExpand++;
	}
	else if(strstr(cAction,"Reduce"))
	{
		unsigned uNetsToReduce=0;
		unsigned uRRToReduceCount=0;
		//printf("Content-type: text/plain\n\n");
		//printf("RemoveOldBlock(%s)\n",cIPBlock);
		RemoveOldBlock(cIPBlock);
		CreateBlock(cIPBlock,uClient);
		//printf("CreateBlock(%s,%u)\n",cIPBlock,uClient);
		//printf("uNumNets=%u\n",uNumNets);

		if(uDBNets==1)
		{
			uRRToReduceCount=uDbIPs-uNumIPs;
			printf("uRRToReduceCount=%u\n",uRRToReduceCount);
			sprintf(cZone,"%u.%u.%u.in-addr.arpa",c,b,a);
			for(f=uDbIPs;f<(uDbIPs-d);f--)
			{
				sprintf(cParam1,"%u-%u-%u-%u.%s",f,c,b,a,cUpdateHost);
				ResetRR(cZone,f,cParam1,uDefaultClient);
			}
			//Update zone serial
			RestoreUpdateSerialNum(uZone);
			//Submit mod job
			//Default uNSSet=1 ONLY
			if(AdminSubmitJob("Mod",1,cZone,0,luClock+300))
					htmlPlainTextError(gcQuery);
		}
		else
		{
			register int x;
			uNetsToReduce=uDBNets-uNumNets;
			//printf("uDBNets=%u\nuNumNets=%u\n",uDBNets,uNumNets);
			//printf("uNetsToReduce=%u\n",uNetsToReduce);
			for(x=c+uDBNets;x<(c+uNetsToReduce);x--)
			{
				//
				sprintf(cZone,"%u.%u.%u.in-addr.arpa",x,b,a);
				//printf("cZone=%s\n",cZone);
				uZone=uZoneSetup(cZone);
			
				//Create block default RRs uOwner=uClient
				//
				if(d==0)d++;
				for(f=d;f<255;f++)
				{
					sprintf(cParam1,"%u-%u-%u-%u.%s",f,c,b,a,cUpdateHost);
					ResetRR(cZone,f,cParam1,uDefaultClient);
				}
				//Update zone serial
				RestoreUpdateSerialNum(uZone);
				//Submit mod job
				//Default uNSSet=1 ONLY
				if(AdminSubmitJob("Mod",1,cZone,0,luClock+300))
					htmlPlainTextError(gcQuery);
			}//for(f=c;f<((c+uNumNets));f++)
		}
		//Check if we are keeping owner or not and update as required (the old RRs only)
		if(!strstr(cAction,"Keep Owner"))
			UpdateBlockOwner(cIPBlock,uClient);

		uBlockReduce++;
	}


	return(0);

}//void ProcessTransaction(char *cIPBlock,unsigned uClient,char *cAction)


unsigned ProcessCompanyTransaction(char *cCompany,char *cAction)
{
	unsigned uClient=0;

	if(!strcmp(cAction,"None")) return(1);

	//Default 'New'
	
	unsigned uContact=0;
	char cPasswd[100]={""};
	char cSavePasswd[16]={""};
	//Create tClient record
	sprintf(gcQuery,"INSERT INTO tClient SET cLabel='%s',"
			"cCode='Organization',uOwner=1,uCreatedBy=%u,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cCompany
			,guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	uClient=mysql_insert_id(&gMysql);
	if(!uClient) return(0);

	//Create default contact with same cLabel
	sprintf(gcQuery,"INSERT INTO tClient SET uOwner=%u,cLabel='%s',"
			"cCode='Contact',uCreatedBy=%u,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uClient
			,cCompany
			,guLoginClient
			);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	uContact=mysql_insert_id(&gMysql);
	//Password should be 8 characters random text
	sprintf(cPasswd,"%s",cGetRandomPassword());
	sprintf(cSavePasswd,"%s",cPasswd);
	EncryptPasswd(cPasswd);
	sprintf(gcQuery,"INSERT INTO tAuthorize SET cLabel='%s',uCertClient=%u,"
			"uOwner=%u,cPasswd='%s',cClrPasswd='%s',cIpMask='0.0.0.0',"
			"uPerm=6,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cCompany
			,uContact
			,uClient
			,cPasswd
			,cSavePasswd
			,guLoginClient
			);
	if(!uContact || !uClient)	
		htmlPlainTextError(gcQuery);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	
	return(1);

}//unsigned ProcessCompanyTransaction(unsigned uClient)


char *cGetRandomPassword(void)
{
	static char cPasswd[10]={""};
	MYSQL_RES *res;
	MYSQL_ROW field;

	mysql_query(&gMysql,"DROP FUNCTION IF EXISTS generate_alpha");
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	mysql_query(&gMysql,"CREATE FUNCTION generate_alpha () RETURNS CHAR(1) "
			"RETURN ELT(FLOOR(1 + (RAND() * (50-1))), 'a','b','c','d'"
			",'e','f','g','h','i','j','k','l','m  ','n','o','p','q','r'"
			",'s','t','u','v','w','x','y',  'z','A','B','C','D','E','F',"
			"'G','H','I','J','K','L','M  ','N','O','P','Q','R','S','T','U'"
			",'V','W','X','Y',  'Z' )");
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	mysql_query(&gMysql,"SELECT CONCAT(generate_alpha (),generate_alpha (),generate_alpha (),"
			"generate_alpha (),generate_alpha (),generate_alpha (),generate_alpha (),"
			"generate_alpha ())");
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);
	res=mysql_store_result(&gMysql);
	field=mysql_fetch_row(res); //We will always have a row if the above queries didn't fail
	
	sprintf(cPasswd,"%s",field[0]);

	return(cPasswd);

}//char *cGetRandomPassword(void)


void CleanUpCompanies(void)
{
	//If a company doesn't exist in the tTransaction table:
	//* Company removed
	//* Associated contacts removed
	//* Associated tBlocks (IP Blocks) removed
	//* Associated Forward zones only removed (Reverse zones always left)
	//* Associated Resource Records removed. For reverse zones only ; the IP Address(es)
	// removed need to be replaced with standard reverse record(s) having the following
	// example format (hopefully the .packetexchange.net will not be hardcoded and just a
	//variable etc). Obviously this will differ for different IP Blocks!:
	//4            PTR 4-71-245-83.packetexchange.net.
	//5            PTR 5-71-245-83.packetexchange.net.
	//6            PTR 6-71-245-83.packetexchange.net.
	//7            PTR 7-71-245-83.packetexchange.net.
	MYSQL_RES *res;
	MYSQL_ROW field;

	MYSQL_RES *res2;
	MYSQL_ROW field2;
	char cuDefaultClient[16]={""};

	GetConfiguration("uDefaultClient",cuDefaultClient,1);
	sscanf(cuDefaultClient,"%u",&uDefaultClient);

	sprintf(gcQuery,"SELECT uClient FROM tClient WHERE cLabel NOT IN "
		"(SELECT DISTINCT cCompany FROM tTransaction) AND "
		"uClient!=1 AND uClient!=%u AND (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP')",uDefaultClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		//Delete forward zones and their RRs
		//The query below ensures that the reverse
		//zones RRs are not touched, those will be handled
		//by the CleanUpBlock() function call below
		//printf("Removing tClient.uClient=%s\n",field[0]);

		//Remove forward zones and their RRs
		sprintf(gcQuery,"SELECT uZone FROM tZone WHERE uOwner=%s",field[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(gcQuery);
		res2=mysql_store_result(&gMysql);
		//printf("Company owns %u fwd zones\n",(unsigned)mysql_num_rows(res2));
		while((field2=mysql_fetch_row(res2)))
		{
			sprintf(gcQuery,"DELETE FROM tResource WHERE uZone=%s",field2[0]);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(gcQuery);
		}

		sprintf(gcQuery,"DELETE FROM tZone WHERE uOwner=%s",field[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(gcQuery);
		
		//Delete blocks
		sprintf(gcQuery,"SELECT cLabel FROM tBlock WHERE uOwner=%s",field[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(gcQuery);
		res2=mysql_store_result(&gMysql);
		//printf("Company owns %u blocks\n",(unsigned)mysql_num_rows(res2));
		while((field2=mysql_fetch_row(res2)))
		{
			uBlockDel++;
			//printf("Removing block %s\n",field2[0]);
			CleanUpBlock(field2[0]);
		}
		//Delete contacts
		sprintf(gcQuery,"SELECT uClient FROM tClient WHERE uOwner=%s",field[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(gcQuery);
		res2=mysql_store_result(&gMysql);
		//printf("Company has %u contacts\n",(unsigned)mysql_num_rows(res2));
		while((field2=mysql_fetch_row(res2)))
		{

			sprintf(gcQuery,"DELETE FROM tAuthorize WHERE uCertClient=%s",field2[0]);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(gcQuery);
		}
		sprintf(gcQuery,"DELETE FROM tClient WHERE uOwner=%s",field[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(gcQuery);
		sprintf(gcQuery,"DELETE FROM tClient WHERE uClient=%s",field[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(gcQuery);

		uCompanyDel++;
	}
	
}//void CleanUpCompanies(void)


void CleanUpBlock(char *cIPBlock)
{
	unsigned uNumIPs=0;
	unsigned uNumNets=0;

	unsigned a,b,c,d,e,f;
	char cZone[100]={""};
	char cParam1[200]={""};
	char cUpdateHost[100]={"packetexchange.net"}; //This will come from tConfiguration, later
	time_t luClock;

	time(&luClock);
	
	uNumIPs=uGetNumIPs(cIPBlock);
	uNumNets=uGetNumNets(cIPBlock);

	sscanf(cIPBlock,"%u.%u.%u.%u/%u",&a,&b,&c,&d,&e);

	sprintf(gcQuery,"DELETE FROM tBlock WHERE cLabel='%s'",cIPBlock);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

	if(uNumNets==1)
	{
		//Update RRs
		if(d==0)d++;
		sprintf(cZone,"%u.%u.%u.in-addr.arpa",c,b,a);
		//printf("cZone=%s\n",cZone);

		for(f=d;f<(uNumIPs+d);f++)
		{
			sprintf(cParam1,"%u-%u-%u-%u.%s",f,c,b,a,cUpdateHost);
			//printf("Reset RR cName=%u\n",f);
			ResetRR(cZone,f,cParam1,uDefaultClient);
		}
		//Update zone serial
		UpdateSerialNum(cZone,"2");
		//Submit Mod job for zone
		if(AdminSubmitJob("Mod",1,cZone,0,luClock+300))
			htmlPlainTextError(gcQuery);
	}
	else
	{
		register int x;
		//Larger than /24 blocks
			
		for(x=c;x<(c+uNumNets);x++)
		{
			//
			sprintf(cZone,"%u.%u.%u.in-addr.arpa",x,b,a);
			//printf("cZone=%s\n",cZone);
			for(f=d;f<254;f++)
			{
				sprintf(cParam1,"%u-%u-%u-%u.%s",f,x,b,a,cUpdateHost);
				ResetRR(cZone,f,cParam1,uDefaultClient);
				//printf("Reset RR cName=%u\n",f);

			}
			//Update zone serial
			UpdateSerialNum(cZone,"2");
			//Submit Mod job for zone
			if(AdminSubmitJob("Mod",1,cZone,0,luClock+300))
				htmlPlainTextError(gcQuery);
		}
		

	}

}//void CleanUpBlock(char *cIPBlock)


void ResetRR(char *cZone,unsigned uName,char *cParam1,unsigned uOwner)
{
	unsigned uZone=0;
	uZone=GetuZone(cZone,"tZone");
	
	sprintf(gcQuery,"UPDATE tResource SET cParam1='%s',uOwner=%u,uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE cName='%u' "
			"AND uZone=%u",
			cParam1
			,uOwner
			,guLoginClient
			,uName
			,uZone
			);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(gcQuery);

}//void ResetRR(char *cZone,char *cParam1)


unsigned GetuZone(char *cLabel, char *cTable)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cQuery[512];
	unsigned uServer=0;
	
	sprintf(cQuery,"SELECT _rowid FROM %s WHERE cZone='%s'",cTable,cLabel);
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(0);
	}
	res=mysql_store_result(&gMysql);
        if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uServer);
	mysql_free_result(res);
	return(uServer);

}//unsigned GetuZone(char *cLabel, char *cTable)
