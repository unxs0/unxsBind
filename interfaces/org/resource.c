/*
FILE 
	resource.c
	svn ID removed
AUTHOR
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice
PURPOSE
	idnsOrg
	program file.
*/

#include "interface.h"
#include <openisp/ucidr.h>

extern char gcZone[];
static unsigned uResource=0;
static unsigned uZone=0;

static char cName[256]={""};
static char *cNameStyle="type_fields_off";

static char cuTTL[16]={""};
static char *cuTTLStyle="type_fields_off";

static char cRRType[100]={""};
static char *cRRTypeStyle="type_fields_off";

static char cParam1[256]={""};
static char *cParam1Style="type_fields_off";

static char cParam2[256]={""};
static char *cParam2Style="type_fields_off";

static char cParam3[256]={""};
static char *cParam3Style="type_textarea_off";

static char cParam4[256]={""};
static char *cParam4Style="type_textarea_off";

static char cComment[256]={""};
static char *cCommentStyle="type_fields_off";


static char cuNameServer[16]={""};
static char cParam1Label[33]={""};
static char cParam1Tip[100]={""};
static char cParam2Label[33]={""};
static char cParam2Tip[100]={""};
static char cParam3Label[33]={""};
static char cParam3Tip[100]={""};
static char cParam4Label[33]={""};
static char cParam4Tip[100]={""};
static char cNameLabel[33]={""};
static char cNameTip[100]={""};
static unsigned uStep=0;

//Local only
void SelectResource(void);
void UpdateResource(void);
void NewResource(void);
void DelResource(void);
unsigned SelectRRType(char *cRRType);
void LoadRRTypeLabels(void);
unsigned RRCheck(void);
unsigned InMyBlocks(char *cIP);
void UpdateSerialNum(char *cZone);
void LoadRRNoType(void);
void MasterFunctionSelect(void);
unsigned uRRExists(char *cZone,char *cRRType,char *cValue,char *cParam);
void ResourceSetFieldsOn(void);
void SaveResource(void);
char *cPrintNSList(FILE *zfp,char *cuNameServer);
void PrintMXList(FILE *zfp,char *cuMailServers);
char *GetRRType(unsigned uRRType);
unsigned idnsOnLineZoneCheck(void);
void CreatetResourceTest(void);
void PrepareTestData(void);
unsigned uPerRRTypeCheck(void);


void ProcessResourceVars(pentry entries[], int x)
{
	register int i;
	char *cp;
	char *cp2;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cZone"))
			sprintf(gcZone,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"uResource"))
			sscanf(entries[i].val,"%u",&uResource);
		else if(!strcmp(entries[i].name,"cName"))
			sprintf(cName,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"uTTL"))
			sprintf(cuTTL,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"cRRType"))
		{
			if(guBrowserFirefox)
			{
				//From pimped out JS select hack
				if((cp=strstr(entries[i].val,"cRRType=")))
				{
					if((cp2=strchr(entries[i].val+9,'&')))
						*cp2=0;
					sprintf(cRRType,"%.32s",entries[i].val+9);
				}
				else
				{
					sprintf(cRRType,"%.32s",entries[i].val);
				}
			}
			else
			{
				sprintf(cRRType,"%.99s",entries[i].val);
			}
		}
		else if(!strcmp(entries[i].name,"cParam1"))
			sprintf(cParam1,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam2"))
			sprintf(cParam2,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam3"))
			sprintf(cParam3,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam4"))
			sprintf(cParam4,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"cComment"))
			sprintf(cComment,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"uNameServer"))
			sprintf(cuNameServer,"%.15s",entries[i].val);
		else if(!strcmp(entries[i].name,"uStep"))
			sscanf(entries[i].val,"%u",&uStep);
	}

	if(!guBrowserFirefox && cRRType[0])
	{
		//'&cRRType=HINFO&cZone=bogus.net&gcFunction=Modify&uResource=46758'
		//From pimped out JS select hack but for MSIE! What a mess.
		if((cp=strstr(cRRType,"cRRType=")))
		{
			if((cp2=strchr(cRRType+8,'&')))
				*cp2=0;
			sprintf(cRRType,"%.32s",cp+8);
		}
		if((cp=strstr(cp2+1,"cZone=")))
		{
			if((cp2=strchr(cp+6,'&')))
				*cp2=0;
			sprintf(gcZone,"%.255s",cp+6);
		}
		if((cp=strstr(cp2+1,"gcFunction=")))
		{
			if((cp2=strchr(cp+11,'&')))
				*cp2=0;
			sprintf(gcFunction,"%.99s",cp+11);
		}
		if((cp=strstr(cp2+1,"uResource=")))
		{
			sscanf(cp+10,"%u",&uResource);
		}

		MasterFunctionSelect();

	}//if(!guBrowserFirefox)

}//void ProcessResourceVars(pentry entries[], int x)


void ResourceCommands(pentry entries[], int x)
{
	if(!strcmp(gcPage,"Resource"))
	{
		ProcessResourceVars(entries,x);

		MasterFunctionSelect();

		if(gcZone[0])
		{
			SelectResource();
			htmlResource();
		}
		else if(1)
		{
			gcMessage="<blink>Redirected to 'Zones.' You must select a zone first</blink>";
			htmlZone();
		}
	}

}//void ResourceCommands(pentry entries[], int x)


void ResourceGetHook(entry gentries[],int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"cZone"))
			sprintf(gcZone,"%.99s",gentries[i].val);
		else if(!strcmp(gentries[i].name,"uResource"))
			sscanf(gentries[i].val,"%u",&uResource);
		else if(!strcmp(gentries[i].name,"cRRType"))
			sprintf(cRRType,"%.32s",gentries[i].val);
		else if(!strcmp(gentries[i].name,"gcFunction"))
			sprintf(gcFunction,"%.99s",gentries[i].val);
		else if(!strcmp(gentries[i].name,"uStep"))
			sscanf(gentries[i].val,"%u",&uStep);
	}

	if(cRRType[0] && gcFunction[0] && gcZone[0])
	{
		if(!strcmp(gcFunction,"Modify") || !strcmp(gcFunction,"Modify Confirm"))
		{
			LoadRRTypeLabels();//Get labels and tips directly from cRRType
			LoadRRNoType();//Get data again except RRType
			ResourceSetFieldsOn();
			sprintf(gcModStep," Confirm");
			gcMessage="You changed the resource type! Modify data, review, then confirm. Any other action to cancel.";
			gcInputStatus[0]=0;
			htmlResource();
		}
		else if(!strcmp(gcFunction,"New") || !strcmp(gcFunction,"New Confirm"))
		{
			LoadRRTypeLabels();//Get labels and tips directly from cRRType
			LoadRRNoType();//Get data again except RRType
			ResourceSetFieldsOn();
			sprintf(gcNewStep," Confirm");
			gcMessage="You changed the resource type! Modify data, review, then confirm. Any other action to cancel.";
			gcInputStatus[0]=0;
			htmlResource();
		}
		else if(!strcmp(gcFunction,"Next"))
		{
			LoadRRTypeLabels();//Get labels and tips directly from cRRType
			LoadRRNoType();//Get data again except RRType
			gcMessage="You changed the resource type! Modify data, review, then confirm. Any other action to cancel.";
			ResourceSetFieldsOn();
			htmlResourceWizard(uStep);
		}

	}
	else if(gcZone[0])
	{
		SelectResource();
		htmlResource();
	}
	else if(1)
	{
		gcMessage="<blink>Redirected to 'Zones.' You must select a zone first</blink>";
		htmlZone();
	}

}//void ResourceGetHook(entry gentries[],int x)


void htmlResource(void)
{
	htmlHeader("idnsOrg","Header");
	htmlResourcePage("idnsOrg","Resource.Body");
	htmlFooter("Footer");

}//void htmlResource(void)


void htmlResourcePage(char *cTitle, char *cTemplateName)
{
	if(cTemplateName[0])
	{
        	MYSQL_RES *res;
	        MYSQL_ROW field;

		TemplateSelectInterface(cTemplateName,uPLAINSET,uIDNSORGTYPE);
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			struct t_template template;
			char cuResource[16];
			
			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
			
			template.cpName[1]="cCGI";
			template.cpValue[1]="idnsOrg.cgi";
			
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

			template.cpName[8]="gcMessage";
			template.cpValue[8]=gcMessage;

			template.cpName[9]="gcInputStatus";
			template.cpValue[9]=gcInputStatus;

			//Form rows
			template.cpName[10]="cName";
			template.cpValue[10]=cName;

			template.cpName[11]="uTTL";
			template.cpValue[11]=cuTTL;

			template.cpName[12]="cRRType";
			template.cpValue[12]=cRRType;

			template.cpName[13]="cParam1";
			template.cpValue[13]=cParam1;

			template.cpName[14]="cParam2";
			template.cpValue[14]=cParam2;

			template.cpName[15]="cComment";
			template.cpValue[15]=cComment;

			template.cpName[16]="cZone";
			template.cpValue[16]=gcZone;

			template.cpName[17]="uResource";
			sprintf(cuResource,"%u",uResource);
			template.cpValue[17]=cuResource;

			template.cpName[18]="uNameServer";
			template.cpValue[18]=cuNameServer;

			template.cpName[19]="gcNewStep";
			template.cpValue[19]=gcNewStep;

			template.cpName[20]="gcDelStep";
			template.cpValue[20]=gcDelStep;

			template.cpName[21]="cParam1Label";
			template.cpValue[21]=cParam1Label;

			template.cpName[22]="cParam1Tip";
			template.cpValue[22]=cParam1Tip;

			template.cpName[23]="cParam2Label";
			template.cpValue[23]=cParam2Label;

			template.cpName[24]="cParam2Tip";
			template.cpValue[24]=cParam2Tip;

			template.cpName[25]="cNameLabel";
			template.cpValue[25]=cNameLabel;

			template.cpName[26]="cNameTip";
			template.cpValue[26]=cNameTip;

			template.cpName[27]="cCommentStyle";
			template.cpValue[27]=cCommentStyle;
			
			template.cpName[28]="cNameStyle";
			template.cpValue[28]=cNameStyle;

			template.cpName[29]="cuTTLStyle";
			template.cpValue[29]=cuTTLStyle;

			template.cpName[30]="cRRTypeStyle";
			template.cpValue[30]=cRRTypeStyle;

			template.cpName[31]="cParam1Style";
			template.cpValue[31]=cParam1Style;			 						 
			
			template.cpName[32]="";

			printf("\n<!-- Start htmlResourcePage(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlResourcePage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlResourcePage()


void funcMetaParam(FILE *fp)
{
	//This function will display the extra parameter inputs based on RRType
	MYSQL_RES *res;
	MYSQL_ROW field;

	struct t_template template;
	unsigned uParam2=0;
	unsigned uParam3=0;
	unsigned uParam4=0;

	sprintf(gcQuery,"SELECT uParam2,uParam3,uParam4 FROM tRRType WHERE cLabel='%s'",TextAreaSave(cRRType));
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&uParam2);
		sscanf(field[1],"%u",&uParam3);
		sscanf(field[2],"%u",&uParam4);
	}
	mysql_free_result(res);

	template.cpName[0]="cParam2Label";
	template.cpValue[0]=cParam2Label;

	template.cpName[1]="cParam2Tip";
	template.cpValue[1]=cParam2Tip;

	template.cpName[2]="cParam2";
	template.cpValue[2]=cParam2;
	
	template.cpName[3]="cParam2Style";
	template.cpValue[3]=cParam2Style;

	template.cpName[4]="cParam3Label";
	template.cpValue[4]=cParam3Label;

	template.cpName[5]="cParam3Tip";
	template.cpValue[5]=cParam3Tip;

	template.cpName[6]="cParam3";
	template.cpValue[6]=cParam3;
	
	template.cpName[7]="cParam3Style";
	template.cpValue[7]=cParam3Style;
	
	template.cpName[8]="cParam4Label";
	template.cpValue[8]=cParam4Label;

	template.cpName[9]="cParam4Tip";
	template.cpValue[9]=cParam4Tip;

	template.cpName[10]="cParam4";
	template.cpValue[10]=cParam4;
	
	template.cpName[11]="cParam4Style";
	template.cpValue[11]=cParam4Style;
	
	//RR wizard fix
	if(uStep) gcInputStatus[0]=0;
	template.cpName[12]="gcInputStatus";
	template.cpValue[12]=gcInputStatus;
	
	template.cpName[13]="";

	if(uParam2)
		fpTemplate(fp,"InputParam2",&template);
	
	if(uParam3)
		fpTemplate(fp,"InputParam3",&template);
	
	if(uParam4)
		fpTemplate(fp,"InputParam4",&template);

}//void funcMetaParam(FILE *fp)


void SelectResource(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	if(uResource)
		sprintf(gcQuery,"SELECT tResource.cName,tResource.uTTL,tRRType.cLabel,"
				"tResource.cParam1,tResource.cParam2,tResource.cComment,"
				"tZone.uNSSet,tResource.uResource,tResource.uZone,tRRType.cParam1Label,"
				"tRRType.cParam1Tip,tRRType.cParam2Label,tRRType.cParam2Tip,tRRType.cParam3Label,"
				"tRRType.cParam3Tip,tRRType.cParam4Label,tRRType.cParam4Tip,tRRType.cNameLabel,"
				"tRRType.cNameTip,tResource.cParam3,tResource.cParam4 FROM tResource,tRRType,"
				"tZone WHERE tZone.uZone=tResource.uZone AND tResource.uRRType=tRRType.uRRType "
				"AND tZone.cZone='%s' AND tZone.uView=2 AND tResource.uResource=%u",
				gcZone,uResource);
	else
		sprintf(gcQuery,"SELECT tResource.cName,tResource.uTTL,tRRType.cLabel,tResource.cParam1,"
				"tResource.cParam2,tResource.cComment,tZone.uNSSet,tResource.uResource,"
				"tResource.uZone,tRRType.cParam1Label,tRRType.cParam1Tip,tRRType.cParam2Label,"
				"tRRType.cParam2Tip,tRRType.cParam3Label,tRRType.cParam3Tip,tRRType.cParam4Label,"
				"tRRType.cParam4Tip,tRRType.cNameLabel,tRRType.cNameTip,tResource.cParam3,"
				"tResource.cParam4 FROM tResource,tRRType,tZone WHERE tZone.uZone=tResource.uZone "
				"AND tResource.uRRType=tRRType.uRRType AND tZone.cZone='%s' AND tZone.uView=2 AND "
				"(tResource.uOwner=%u OR tResource.uOwner=%u) ORDER BY tRRType.uRRType,"
				"tResource.cName LIMIT 1",gcZone,guLoginClient,guOrg);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sprintf(cName,"%.99s",field[0]);
		sprintf(cuTTL,"%.15s",field[1]);
		sprintf(cRRType,"%.32s",field[2]);
		sprintf(cParam1,"%.255s",field[3]);
		sprintf(cParam2,"%.255s",field[4]);
		sprintf(cComment,"%.255s",field[5]);
		sprintf(cuNameServer,"%.15s",field[6]);
		sscanf(field[7],"%u",&uResource);
		sscanf(field[8],"%u",&uZone);
		sprintf(cParam1Label,"%.32s",field[9]);
		sprintf(cParam1Tip,"%.99s",field[10]);
		sprintf(cParam2Label,"%.32s",field[11]);
		sprintf(cParam2Tip,"%.99s",field[12]);
		sprintf(cParam3Label,"%.32s",field[13]);
		sprintf(cParam3Tip,"%.99s",field[14]);
		sprintf(cParam4Label,"%.32s",field[15]);
		sprintf(cParam4Tip,"%.99s",field[16]);
		sprintf(cNameLabel,"%.32s",field[17]);
		sprintf(cNameTip,"%.99s",field[18]);
		sprintf(cParam3,"%.255s",field[19]);
		sprintf(cParam4,"%.255s",field[20]);
		if(!gcMessage[0]) gcMessage="Zone Resource Selected";
	}
	else
	{
		mysql_free_result(res);
		if(!strstr(gcZone,"in-addr.arpa"))//A RR and has rights via uOwner
			sprintf(gcQuery,"SELECT tRRType.cLabel,tZone.uNSSet,tZone.uZone,tRRType.cParam1Label,"
					"tRRType.cParam1Tip,tRRType.cParam2Label,tRRType.cParam2Tip,tRRType.cParam3Label,"
					"tRRType.cParam3Tip,tRRType.cParam4Label,tRRType.cParam4Tip,tRRType.cNameLabel,"
					"tRRType.cNameTip FROM tRRType,tZone WHERE tRRType.uRRType=1 AND tZone.cZone='%s' "
					"AND tZone.uView=2 AND (tZone.uOwner=%u OR tZone.uOwner=%u)",
					gcZone,guLoginClient,guOrg);
		else	//PTR RR and has rights via zone.c 
			//(low grade cross-site scrpting security issue for registered login)
			sprintf(gcQuery,"SELECT tRRType.cLabel,tZone.uNSSet,tZone.uZone,tRRType.cParam1Label,"
					"tRRType.cParam1Tip,tRRType.cParam2Label,tRRType.cParam2Tip,tRRType.cParam3Label,"
					"tRRType.cParam3Tip,tRRType.cParam4Label,tRRType.cParam4Tip,tRRType.cNameLabel,"
					"tRRType.cNameTip FROM tRRType,tZone WHERE tRRType.uRRType=7 AND tZone.cZone='%s' "
					"AND tZone.uView=2 AND (tZone.uOwner=1 OR tZone.uOwner=%u)",
					gcZone,guOrg);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			sprintf(cRRType,"%.32s",field[0]);
			sprintf(cuNameServer,"%.15s",field[1]);
			sscanf(field[2],"%u",&uZone);
			sprintf(cParam1Label,"%.32s",field[3]);
			sprintf(cParam1Tip,"%.99s",field[4]);
			sprintf(cParam2Label,"%.32s",field[5]);
			sprintf(cParam2Tip,"%.99s",field[6]);
			sprintf(cParam3Label,"%.32s",field[7]);
			sprintf(cParam3Tip,"%.99s",field[8]);
			sprintf(cParam4Label,"%.32s",field[9]);
			sprintf(cParam4Tip,"%.99s",field[10]);
			sprintf(cNameLabel,"%.32s",field[11]);
			sprintf(cNameTip,"%.99s",field[12]);

			cName[0]=0;
			cuTTL[0]=0;
			cParam1[0]=0;
			cParam2[0]=0;
			cParam3[0]=0;
			cParam4[0]=0;
			uResource=0;
			cComment[0]=0;
			gcMessage="Zone with no resources. You may use [New] to add.";
		}
		else
		{
			LoadRRTypeLabels();
		}
	}
	mysql_free_result(res);
	
}//void SelectResource(void)


void UpdateResource(void)
{
	unsigned uTTL=0;
	unsigned uRRType=0;

	sscanf(cuTTL,"%u",&uTTL);
	uRRType=SelectRRType(cRRType);

	//Check here
	if(RRCheck())
	{
		sprintf(gcModStep," Confirm");
		gcInputStatus[0]=0;
		return;
	}
	
	sprintf(gcQuery,"UPDATE tResource SET cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',cParam2='%s',"
			"cParam3='%s',cParam4='%s',cComment='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) "
			"WHERE uResource=%u",
			TextAreaSave(cName),
			uTTL,
			uRRType,
			TextAreaSave(cParam1),
			TextAreaSave(cParam2),
			TextAreaSave(cParam3),
			TextAreaSave(cParam4),
			TextAreaSave(cComment),
			guLoginClient,uResource);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	if(mysql_affected_rows(&gMysql)==1)
	{
		gcMessage="Zone Resource Modified";
		iDNSLog(uResource,"tResource","Mod");
	}
	else
	{
		gcMessage="<blink>Zone Resource Not Modified</blink>";
	}
	
}//void UpdateResource(void)


void NewResource(void)
{
	unsigned uTTL=0;
	unsigned uRRType=0;

	sscanf(cuTTL,"%u",&uTTL);
	uRRType=SelectRRType(cRRType);

	//Check here
	if(RRCheck())
	{
		sprintf(gcNewStep," Confirm");
		gcInputStatus[0]=0;
		return;
	}

	uZone=uGetuZone(gcZone);
	
	sprintf(gcQuery,"INSERT INTO tResource SET cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',"
			"cParam2='%s',cParam3='%s',cParam4='%s',cComment='%s',uOwner=%u,"
			"uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW()),uZone=%u",
			TextAreaSave(cName),
			uTTL,
			uRRType,
			TextAreaSave(cParam1),
			TextAreaSave(cParam2),
			TextAreaSave(cParam3),
			TextAreaSave(cParam4),
			TextAreaSave(cComment),
			guOrg,
			guLoginClient,
			uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	if(mysql_affected_rows(&gMysql)==1)
	{
		gcMessage="Zone Resource Created";
		uResource=mysql_insert_id(&gMysql);
		iDNSLog(uResource,"tResource","New");
	}
	else
	{
		gcMessage="<blink>Zone Resource Not Created</blink>";
	}
	
}//void NewResource(void)


void DelResource(void)
{
	sprintf(gcQuery,"DELETE FROM tResource WHERE uResource=%u AND (uOwner=%u OR uOwner=%u)",
			uResource,guLoginClient,guOrg);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	if(mysql_affected_rows(&gMysql)==1)
	{
		gcMessage="Zone Resource Deleted";
		iDNSLog(uResource,"tResource","Del");
	}
	else
	{
		gcMessage="<blink>Zone Resource Not Deleted</blink>";
	}
	
}//void DelResource(void)


unsigned SelectRRType(char *cRRType)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uRRType=0;

	sprintf(gcQuery,"SELECT uRRType from tRRType WHERE cLabel='%s'",cRRType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uRRType);
	mysql_free_result(res);

	return(uRRType);
	
}//unsigned SelectRRType(char *cRRType)


unsigned uRRExists(char *cZone,char *cRRType,char *cValue,char *cParam)
{
	MYSQL_RES *res;

	//
	//If we are modifying a record, bypass this check
	if(!strcmp(gcFunction,"Modify Confirm"))
		return(0);
	
	sprintf(gcQuery,"SELECT uResource FROM tResource WHERE uZone=%u AND uRRType=%u AND cName='%s' AND cParam1='%s'",
			uGetuZone(cZone)
			,SelectRRType(cRRType)
			,cValue
			,cParam
	       );
	mysql_query(&gMysql,gcQuery);
	//htmlPlainTextError(gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	if(mysql_num_rows(res))
		return(1);

	return(0);
	
}//unsigned uRRExists(char *cZone,char *cRRType,char *cValue)


unsigned RRCheck(void)
{
	register int i;
	
	//For all types check here
	//

	if(!cuTTL[0])
		sprintf(cuTTL,"0");
	if(!isdigit(cuTTL[strlen(cuTTL)-1]))
	{
		gcMessage="<blink>Resource TTL should be specified in seconds only</blink>";
		return(17);
	}

	//Remove any extra characters(E.g.: cParam1=ns.somedns.net ."
	if(cName[0])
	{
		sscanf(cName,"%s",gcQuery);
		sprintf(cName,"%.255s",gcQuery);
	}
	
	if(cParam1[0] && strcmp(cRRType,"TXT") && strcmp(cRRType,"HINFO"))
	{
		sscanf(cParam1,"%s",gcQuery);
		sprintf(cParam1,"%.99s",gcQuery);
	}
	
	if(cParam2[0] && strcmp(cRRType,"HINFO"))
	{
		sscanf(cParam2,"%s",gcQuery);
		sprintf(cParam2,"%.99s",gcQuery);
	}
		
	//General cName checks
	//1-. Allow empty cName unless PTR record
	if(!cName[0] && !strcmp(cRRType,"PTR"))
	{
		gcMessage="<blink>Resource name is required by us for PTR type records</blink>";
		return(1);
	}


	//2-. If it has a period must be full qually time
	if(cName[strlen(cName)-1]=='.')
	{
		//Another bug may allow zones to be added with trailing dot.
		if(gcZone[strlen(gcZone)-1]=='.')
			sprintf(gcQuery,"%.4095s",gcZone);
		else
			sprintf(gcQuery,"%.4095s.",gcZone);
		if(strcmp(gcQuery,cName))
		{
			//Another bug may allow zones to be added with trailing dot.
			if(gcZone[strlen(gcZone)-1]=='.')
				sprintf(gcQuery,".%.4095s",gcZone);
			else
				sprintf(gcQuery,".%.4095s.",gcZone);
			if(!strstr(cName+(strlen(cName)-strlen(gcQuery)),gcQuery))
			{
				if(strstr(cName+strlen(cName)-strlen(gcZone),gcZone))
				{
					strcat(cName,".");
					gcMessage="<blink>We have added a final period. If this correct confirm</blink>";
					cNameStyle="type_fields_req";
					return(2);
				}
				gcMessage="<blink>If Name is fully qualified it must end with the zone and final period</blink>";
				cNameStyle="type_fields_req";
				return(2);
			}
		}
	}
	//More cName validation. Do not allow .. or .- or -. in cName
	if(strstr(cName,".."))
	{
		gcMessage="<blink>Error: </blink>Name can't contain '..'";
		cNameStyle="type_fields_req";
		return(2);
	}
	if(strstr(cName,".-"))
	{
		gcMessage="<blink>Error: </blink>Name can't contain '.-'";
		cNameStyle="type_fields_req";
		return(2);
	}
	if(strstr(cName,"-."))
	{
		gcMessage="<blink>Error: </blink>Name can't contain '-.'";
		cNameStyle="type_fields_req";
		return(2);
	}
	//More cName validation. If @ present it must be only char.
	if(strchr(cName,'@') && strlen(cName)>1)
	{
		gcMessage="<blink>Error: </blink>If @ is used it must be the only char";
		cNameStyle="type_fields_req";
		return(2);
	}

	//3-. Can only have digits, letters, dash and dots the ampersand and the asterix wild card.
	//This is mostly for the (default) problem we have experienced after deployment :(
	for(i=0;cName[i];i++)
	{
		if(!isalnum(cName[i]) && cName[i]!='-' && cName[i]!='.' && cName[i]!='@'
				&& cName[i]!='*' && cName[i]!='_')
		{
			gcMessage="<blink>Name can be empty or have only letters, numbers, the default origin @ symbol."
					" Or dashes (-) and periods (.)</blink>";
			cNameStyle="type_fields_req";
			return(2);
		}
	}

	FQDomainName(cName);
	
	//Per Type checks
	if(uPerRRTypeCheck())
		return(3);

	PrepareTestData();

	if(idnsOnLineZoneCheck())
	{
		return(18);
	}

	return(0);

}//void RRCheck(int uMode)


unsigned uPerRRTypeCheck(void)
{
	unsigned a=0,b=0,c=0,d=0;	
	
	if(!strcmp(cRRType,"CNAME"))
	{
		MYSQL_RES *res;
		//cParam2 not used. Erased.
		cParam2[0]=0;

		////Ticket #323 CNAME record pointing to itself
		if(!cName[0] && !strcmp(gcZone,cParam1))
		{
			gcMessage="<blink>Can't create a CNAME record pointing to itself</blink>";
			cParam1Style="type_fields_req";
			cNameStyle="type_fields_req";
			return(3);
		}

		//don't allow same name CNAME records
		if(strcmp(gcFunction,"Modify Confirm")) 
		{
			sprintf(gcQuery,"SELECT uResource FROM tResource WHERE cName='%s' AND uZone=%u AND uRRType=5",cName,uGetuZone(gcZone));
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
			res=mysql_store_result(&gMysql);
			if(mysql_num_rows(res))
			{
				gcMessage="<blink>CNAME records are singleton type. RR w/the same name found.</blink>";
				cNameStyle="type_fields_req";
				return(3);
			}
			mysql_free_result(res);
		}
		
		if(!cParam1[0])
		{
			sprintf(cParam1,"%.99s.",gcZone);
			sprintf(gcQuery,"<blink>%s is required. Common CNAME default entry made for you, check/change if needed</blink>",cParam1Label);
			gcMessage=gcQuery;
			cParam1Style="type_fields_req";
			return(3);
		}
		else
		{
			char cParam1Save[101]={""};
			char cParam1Temp[101]={""};

			sprintf(cParam1Save,"%.100s",cParam1);
			//converts, eliminates illegal chars.
			//helpers try to format for you...lol
			FQDomainName(cParam1);

			//Ticket #323 CNAME record pointing to itself
			if(!strcmp(cName,cParam1) || !strcmp(gcZone,cParam1))
			{
				gcMessage="<blink>Can't create a CNAME record pointing to itself</blink>";
				cParam1Style="type_fields_req";
				cNameStyle="type_fields_req";
				return(3);
			}
			
			if(strchr(cParam1,'.'))
			{
				//FQDN
				//Missing trailing dot
				if(cParam1[strlen(cParam1)-1]!='.')
				{
					sprintf(cParam1Temp,"%.100s.",cParam1);
					sprintf(cParam1,"%.99s",cParam1Temp);
				}
				if(strcmp(cParam1,cParam1Save))
				{
					sprintf(gcQuery,
						"<blink>%s was changed check/fix</blink>",
							cParam1Label);
					gcMessage=gcQuery;
					cParam1Style="type_fields_req";
					return(4);
				}
			}
			else
			{
				//Not FQDN, but internal zone member or another zone?

				//TODO
				//Advanced help not implemented yet.
				//Check in this zone for A record. Should not be another CNAME.
				//Check in tZone

				if(cParam1[strlen(cParam1)-1]!='.')
				{
					sprintf(cParam1Temp,"%.49s.%.49s.",cParam1,gcZone);
					sprintf(cParam1,"%.99s",cParam1Temp);
				}
				if(strcmp(cParam1,cParam1Save))
				{
					sprintf(gcQuery,
						"<blink>%s was changed check/fix</blink>",
							cParam1Label);
					gcMessage=gcQuery;
					cParam1Style="type_fields_req";
					return(5);
				}
			}

		}
	}
	else if(!strcmp(cRRType,"A"))
	{

		if(!strcmp(gcZone+strlen(gcZone)-5,".arpa"))
		{
			gcMessage="<blink>Can not add A records to arpa zones</blink>";
			return(6);
		}
		sscanf(cParam1,"%u.%u.%u.%u",&a,&b,&c,&d);
		if(a>255) a=0;
		if(b>255) b=0;
		if(c>255) c=0;  
		if(d>255) d=0;  

		sprintf(cParam1,"%u.%u.%u.%u",a,b,c,d);

		cParam2[0]=0;
		if(!a || !d)
		{
			sprintf(gcQuery,
				"<blink>Invalid IP Number for %s</blink>",
							cParam1Label);
			gcMessage=gcQuery;
			cParam1Style="type_fields_req";
			return(7);
		}

		if(uRRExists(gcZone,cRRType,cName,cParam1))
		{
			gcMessage="<blink>Resource record already exists</blink>";
			cParam1Style="type_fields_req";
			cNameStyle="type_fields_req";
			cRRTypeStyle="type_fields_req";
			return(7);
		}

		if(strchr(cName,'_'))
		{
			gcMessage="<blink>A RR cName/Hostname can't contain '_'</blink>";
			cNameStyle="type_fields_req";
			return(7);
		}
	}
	else if(!strcmp(cRRType,"PTR"))
	{
		unsigned uPtr=0;
		unsigned uPtrLen=strlen(cName);
		//We only allow simple classC in-addr PTR
		if(strstr(gcZone,"in-addr.arpa"))
		{
			sscanf(cName,"%u",&uPtr);
			sprintf(cName,"%u",uPtr);

			if(!uPtr)
			{
			sprintf(gcQuery,"<blink>Class-C in-addr.arpa zone PTR must be the last octet of rev dns IP</blink>");
			gcMessage=gcQuery;
			cNameStyle="type_fields_req";			
			return(8);
			}

			if(uPtr>255)
			{
			sprintf(gcQuery,"<blink>PTR value out of range (1-255 allowed only)</blink>");
			gcMessage=gcQuery;
			cNameStyle="type_fields_req";			
			return(8);
			}

			if(uPtrLen!=strlen(cName))
			{
			sprintf(gcQuery,"<blink>PTR was changed check</blink>");
			cNameStyle="type_fields_req";
			gcMessage=gcQuery;
			return(8);
			}

			sscanf(gcZone,"%u.%u.%u.in-adddr.arpa",&c,&b,&a);
			if(!a)
			{
			sprintf(gcQuery,
				"<blink>Unexpected in-addr.arpa zone name format</blink>");
			gcMessage=gcQuery;
			cNameStyle="type_fields_req";
			return(8);
			}

			if(uRRExists(gcZone,cRRType,cName,cParam1))
			{
			gcMessage="<blink>Resource record already exists</blink>";
			cNameStyle="type_fields_req";
			cParam1Style="type_fields_req";
			cRRTypeStyle="type_fields_req";
			return(8);
			}

			sprintf(cParam2,"%u.%u.%u.%u",a,b,c,uPtr);
			if(!InMyBlocks(cParam2))
			{
			gcMessage="<blink>IP Number not in any of your IP blocks</blink>";
			cNameStyle="type_fields_req";
			return(18);
			}
			cParam2[0]=0;//Was just a temp place holder
		}

		//Non arpa PTR records. Should we allow?
		cParam2[0]=0;
		FQDomainName(cParam1);
		if(!cParam1[0])
		{
			sprintf(gcQuery,"<blink>%s is required</blink>",cParam1Label);
			gcMessage=gcQuery;
			cParam1Style="type_fields_req";
			return(8);
		}
		if(cParam1[strlen(cParam1)-1]!='.') strcat(cParam1,".");
	}
	else if(!strcmp(cRRType,"MX"))
	{
		if(!strcmp(gcZone+strlen(gcZone)-5,".arpa"))
		{
			gcMessage="<blink>Can not add MX records to arpa zones</blink>";
			return(9);
		}
		sscanf(cParam1,"%u",&a);
		sprintf(cParam1,"%u",a);
		if(!a || a>1000)
		{
			sprintf(gcQuery,"<blink>Invalid MX Priority Number for %s</blink>",
					cParam1Label);
			gcMessage=gcQuery;
			cParam1Style="type_fields_req";
			return(10);
		}
		FQDomainName(cParam2);
		if(!cParam2[0])
		{
			sprintf(gcQuery,"<blink>%s is required</blink>",cParam2Label);
			gcMessage=gcQuery;
			cParam2Style="type_fields_req";
			return(11);
		}
		if(cParam2[strlen(cParam2)-1]!='.') strcat(cParam2,".");
		
	}
	else if(!strcmp(cRRType,"NS"))
	{
		//All cases
		cParam2[0]=0;
		if(!cParam1[0])
		{
			sprintf(gcQuery,"<blink>%s is required</blink>",cParam1Label);
			gcMessage=gcQuery;
			cParam1Style="type_fields_req";
			return(12);
		}
		FQDomainName(cParam1);
		if(cParam1[strlen(cParam1)-1]!='.') strcat(cParam1,".");

		if(strcmp(gcZone+strlen(gcZone)-5,".arpa"))
		{
			sprintf(cName,"%.255s.",gcZone);
		}
		//else no other rules for arpa zone for now TODO
	}
	else if(!strcmp(cRRType,"HINFO"))
	{
		if(!cParam1[0])
		{
			sprintf(gcQuery,"<blink>%s is required</blink>",cParam1Label);
			gcMessage=gcQuery;
			cParam1Style="type_fields_req";
			return(13);
		}
		if(cParam1[0]!='"' && cParam1[strlen(cParam1)-1]!='"')
		{
			sprintf(gcQuery,"\"%.4095s\"",cParam1);
			strcpy(cParam1,gcQuery);
			cParam1Style="type_fields_req";
		}

		if(!cParam2[0])
		{
			sprintf(gcQuery,"<blink>%s is required</blink>",cParam2Label);
			gcMessage=gcQuery;
			cParam2Style="type_fields_req";
			return(14);
		}
		if(cParam2[0]!='"' && cParam1[strlen(cParam2)-1]!='"')
		{
			sprintf(gcQuery,"\"%.4095s\"",cParam2);
			sprintf(cParam2,"%.99s",gcQuery);
		}
	}
	else if(!strcmp(cRRType,"TXT"))
	{
		char *cp;

		cParam2[0]=0;
		if(!cParam1[0])
		{
			sprintf(gcQuery,"<blink>%s is required</blink>",cParam1Label);
			gcMessage=gcQuery;
			cParam1Style="type_fields_req";
			return(15);
		}
		if(cParam1[0]!='"' && cParam1[strlen(cParam1)-1]!='"')
		{
			sprintf(gcQuery,"\"%.4095s\"",cParam1);
			sprintf(cParam1,"%.99s",gcQuery);
		}
		else if(cParam1[0]!='"' && cParam1[strlen(cParam1)-1]=='"')
		{
			sprintf(gcQuery,"\"%.4095s",cParam1);
			sprintf(cParam1,"%.99s",gcQuery);
		}
		else if(cParam1[0]=='"' && cParam1[strlen(cParam1)-1]!='"')
		{
			sprintf(gcQuery,"%.4095s\"",cParam1);
			sprintf(cParam1,"%.99s",gcQuery);
		}
		else if((cp=strchr(cParam1+1,'"')))
		{
			if(cp!=cParam1+strlen(cParam1)-1)
			{
				sprintf(gcQuery,"<blink>Stray \" in TXT value</blink>");
				gcMessage=gcQuery;
				cParam1Style="type_fields_req";
				return(15);
			}
		}

	}
	else if(!strcmp(cRRType,"SRV"))
	{
		unsigned uI=0;
		if(!cName[0])
		{
			gcMessage="<blink>Service protocol and domain required</blink>";
			cNameStyle="type_fields_req";
			return(16);
		}
		else
		{
			register int x=0;
			
			//All lowercase
			for(x=0;x<strlen(cName);x++)
				cName[x]=tolower(cName[x]);
			if((strstr(cName,"_tcp")==NULL)&&(strstr(cName,"_udp")==NULL))
			{
				gcMessage="<blink>Error: </blink>Service protocol required";
				cNameStyle="type_fields_req";
				return(16);
			}
		}
		if(!cParam1[0])
		{
			gcMessage="<blink>Priority required</blink>";
			cParam1Style="type_fields_req";
			return(16);
		}
		if(!cParam2[0])
		{
			gcMessage="<blink>Weight required</blink>";
			cParam2Style="type_fields_req";
			return(16);
		}
		if(!cParam3[0])
		{
			gcMessage="<blink>Port required</blink>";
			cParam3Style="type_textarea_req";
			return(16);
		}
		if(!cParam4[0])
		{
			gcMessage="<blink>Target host required</blink>";
			cParam4Style="type_textarea_req";
			return(16);                          
		}

		if((strstr(cName,gcZone)==NULL))
		{
			gcMessage="Must include zone name in service parameter. E.g.: _sip._tcp.example.com.</blink>";
			cNameStyle="type_fields_req";
			return(17);
		}
		sscanf(cParam1,"%u",&uI);
		if(!uI)
		{
			gcMessage="<blink>Must specify numerical priority</blink>";
			cParam1Style="type_fields_req";
			return(17);
		}
		uI=0;
		sscanf(cParam2,"%u",&uI);
		if(!uI)
		{
			gcMessage="<blink>Must specify numerical weight</blink>";
			cParam2Style="type_fields_req";
			return(17);
		}
		uI=0;
		sscanf(cParam3,"%u",&uI);
		if((!uI) || (uI>65535))
		{
			gcMessage="<blink>Invalid port number</blink>";
			cParam3Style="type_textarea_req";
			return(17);
		}
		FQDomainName(cParam4);
		if(cParam4[strlen(cParam4)-1]!='.') strcat(cParam4,".");
		if(cName[strlen(cName)-1]!='.') strcat(cName,".");
	}
	else if(!strcmp(cRRType,"AAAA"))
	{
		register int i;
		unsigned h1=0;
		unsigned h2=0;
		unsigned h3=0;
		unsigned h4=0;
		unsigned h5=0;
		unsigned h6=0;
		unsigned h7=0;
		unsigned h8=0;
		char *cp;
		unsigned uColonCount=0;
		unsigned uRead=0;

		//Insure these are empty
		cParam2[0]=0;

		if(strchr(cName,'_'))
		{
			gcMessage="<blink>AAAA RR cName/Hostname can't contain '_'</blink>";
			cNameStyle="type_fields_req";
			return(18);
		}

		if(strlen(cParam1)<4)
		{
			cParam1Style="type_fields_req";
			gcMessage="<blink>Error: </blink>IPv6 number must be at least 4 chars long (e.g. 1::a)";
			return(18);
		}

		//if cParam1 has no consecutive colons we can simply:
		if((cp=strstr(cParam1,"::")))
		{
			if(strstr(cp+2,"::"))
			{
				cParam1Style="type_fields_req";				
				gcMessage="<blink>Error: </blink>IPv6 number can not have more than one double colon!";
				return(18);
			}
		}

		//Now for the hard work
		for(i=0;cParam1[i];i++)
		{
			if(cParam1[i]==':')
				uColonCount++;
			if(cParam1[i]!=':' && !isxdigit(cParam1[i]))
			{
				cParam1Style="type_fields_req";
				gcMessage="<blink>Error: </blink>IPv6 number can only have hexadecimal digits and colons!";
				return(18);
			}
		}

		switch(uColonCount)
		{
			case 0:
			case 1:
				cParam1Style="type_fields_req";
				gcMessage="<blink>Error: </blink>IPv6 too few colons: Min is 2!";
				return(18);
			break;

			case 2:
				uRead=sscanf(cParam1,"%x::%x",&h1,&h8);
				if(uRead!=2)
				{
					cParam1Style="type_fields_req";
					gcMessage="<blink>Error: </blink>IPv6 format-2 error!";
					return(18);
				}
			break;

			case 3:
				uRead=sscanf(cParam1,"%x::%x:%x",&h1,&h7,&h8);
				if(uRead!=3)
				{
					uRead=sscanf(cParam1,"%x:%x::%x",&h1,&h2,&h8);
					if(uRead!=3)
					{
						cParam1Style="type_fields_req";
						gcMessage="<blink>Error: </blink>IPv6 format-3 error!";
						return(18);
					}
				}
			break;

			case 4:
				uRead=sscanf(cParam1,"%x::%x:%x:%x",&h1,&h6,&h7,&h8);
				if(uRead!=4)
				{
					uRead=sscanf(cParam1,"%x:%x::%x:%x",&h1,&h2,&h7,&h8);
					if(uRead!=4)
					{
						uRead=sscanf(cParam1,"%x:%x:%x::%x",&h1,&h2,&h3,&h8);
						if(uRead!=4)
						{
							cParam1Style="type_fields_req";
							gcMessage="<blink>Error: </blink>IPv6 format-4 error!";
							return(18);
						}
					}
				}
			break;

			case 5:
				uRead=sscanf(cParam1,"%x::%x:%x:%x:%x",&h1,&h5,&h6,&h7,&h8);
				if(uRead!=5)
				{
					uRead=sscanf(cParam1,"%x:%x::%x:%x:%x",&h1,&h2,&h6,&h7,&h8);
					if(uRead!=5)
					{
						uRead=sscanf(cParam1,"%x:%x:%x::%x:%x",&h1,&h2,&h3,&h7,&h8);
						if(uRead!=5)
						{
							uRead=sscanf(cParam1,"%x:%x:%x:%x::%x",&h1,&h2,&h3,&h4,&h8);
							if(uRead!=5)
							{
								cParam1Style="type_fields_req";
								gcMessage="<blink>Error: </blink>IPv6 format-5 error!";
								return(18);
							}
						}
					}
				}
			break;

			case 6:
				uRead=sscanf(cParam1,"%x::%x:%x:%x:%x:%x",&h1,&h4,&h5,&h6,&h7,&h8);
				if(uRead!=6)
				{
					uRead=sscanf(cParam1,"%x:%x::%x:%x:%x:%x",&h1,&h2,&h5,&h6,&h7,&h8);
					if(uRead!=6)
					{
						uRead=sscanf(cParam1,"%x:%x:%x::%x:%x:%x",&h1,&h2,&h3,&h6,&h7,&h8);
						if(uRead!=6)
						{
							uRead=sscanf(cParam1,"%x:%x:%x:%x::%x:%x",&h1,&h2,&h3,&h4,&h7,&h8);
							if(uRead!=6)
							{
								uRead=sscanf(cParam1,"%x:%x:%x:%x:%x::%x",
											&h1,&h2,&h3,&h4,&h5,&h8);
								if(uRead!=6)
								{
									cParam1Style="type_fields_req";
									gcMessage="<blink>Error: </blink>IPv6 format-6 error!";
									return(18);
								}
							}
						}
					}
				}
			break;

			case 7:
				uRead=sscanf(cParam1,"%x:%x:%x:%x:%x:%x:%x:%x",&h1,&h2,&h3,&h4,&h5,&h6,&h7,&h8);
				if(uRead!=8)
				{
					cParam1Style="type_fields_req";
					gcMessage="<blink>Error: </blink>IPv6 format-7 error!";
					return(18);
				}
			break;

			default:
				cParam1Style="type_fields_req";
				gcMessage="<blink>Error: </blink>IPv6 too many colons: Max is 7!";
				return(18);
			
		}

		//First basic checks for AAAA hosts
		if(!h1)
		{
			cParam1Style="type_fields_req";
			gcMessage="<blink>Error: </blink>IPv6 number can not have a 0 in first 16 bit hex word.";
			return(18);
		}

		if(!h8)
		{
			cParam1Style="type_fields_req";
			gcMessage=malloc(256);
			sprintf(gcMessage,"<blink>Error: </blink>IPv6 number can not have a 0 in last 16 bit hex word:"
					" %x:%x:%x:%x:%x:%x:%x:%x",h1,h2,h3,h4,h5,h6,h7,h8);
			return(18);
		}

		//Mandatory rewrite in shortest possible IPv6 format.
		//This is needed to speed up DNSSEC and reduce BIND zone file size.
		//This may not be a good idea. Need to research further: If someone wants to
		//write a bunch of 0's why not?
		//Compress empty words: Double colon. Can only be used once.
		//Trying KISS method here. sprintf does the leading 0 removal for us.
		//6 consecutive 0 case
		if(!h2 && !h3 && !h4 && !h5 && !h6 && !h7)
			sprintf(cParam1,"%x::%x",h1,h8);
		//5 consecutive 0 cases
		else if(!h2 && !h3 && !h4 && !h5 && !h6)
			sprintf(cParam1,"%x::%x:%x",h1,h7,h8);
		else if(!h3 && !h4 && !h5 && !h6 && !h7)
			sprintf(cParam1,"%x:%x::%x",h1,h2,h8);
		//4 consecutive 0 cases
		else if(!h2 && !h3 && !h4 && !h5)
			sprintf(cParam1,"%x::%x:%x:%x",h1, h6,h7,h8);
		else if(!h3 && !h4 && !h5 && !h6)
			sprintf(cParam1,"%x:%x::%x:%x",h1,h2, h7,h8);
		else if(!h4 && !h5 && !h6 && !h7)
			sprintf(cParam1,"%x:%x:%x::%x",h1,h2,h3, h8);
		//3 consecutive 0 cases
		else if(!h2 && !h3 && !h4)
			sprintf(cParam1,"%x::%x:%x:%x:%x",h1, h5,h6,h7,h8);
		else if(!h3 && !h4 && !h5)
			sprintf(cParam1,"%x:%x::%x:%x:%x",h1,h2, h6,h7,h8);
		else if(!h4 && !h5 && !h6)
			sprintf(cParam1,"%x:%x:%x::%x:%x",h1,h2,h3, h7,h8);
		else if(!h5 && !h6 && !h7)
			sprintf(cParam1,"%x:%x:%x:%x::%x",h1,h2,h3,h4, h8);
		//2 consecutive 0 cases
		else if(!h2 && !h3)
			sprintf(cParam1,"%x::%x:%x:%x:%x:%x",h1, h4,h5,h6,h7,h8);
		else if(!h3 && !h4)
			sprintf(cParam1,"%x:%x::%x:%x:%x:%x",h1,h2, h5,h6,h7,h8);
		else if(!h4 && !h5)
			sprintf(cParam1,"%x:%x:%x::%x:%x:%x",h1,h2,h3, h6,h7,h8);
		else if(!h5 && !h6)
			sprintf(cParam1,"%x:%x:%x:%x::%x:%x",h1,h2,h3,h4, h7,h8);
		else if(!h6 && !h7)
			sprintf(cParam1,"%x:%x:%x:%x:%x::%x",h1,h2,h3,h4,h5, h8);
		//0 consecutive 0 case, i.e. no double colon case
		else if(1)
			sprintf(cParam1,"%x:%x:%x:%x:%x:%x:%x:%x",h1,h2,h3,h4,h5,h6,h7,h8);
	}
	else if(!strcmp(cRRType,"NAPTR"))
	{
		register int i;
		unsigned uI=0;

		if(!cName[0])
		{
			cNameStyle="type_fields_req";
			gcMessage="<blink>Error: </blink>cName: Resource name required";
		}
		else
		{
			register int x=0;
			
			//All lowercase
			for(x=0;x<strlen(cName);x++)
				cName[x]=tolower(cName[x]);
		}	
		if(!cParam1[0])
		{
			cParam1Style="type_fields_req";
			gcMessage="<blink>Error: </blink>cParam1: Order value required";
		}
		if(!cParam2[0])
		{
			cParam2Style="type_fields_req";
			gcMessage="<blink>Error: </blink>cParam2: Preference value required";
		}
		if(!cParam3[0])
		{
			cParam3Style="type_textarea_req";
			gcMessage="<blink>Error: </blink>cParam3: Flags and ENUM double quoted strings required";
		}
		if(!cParam4[0])
		{
			cParam4Style="type_textarea_req";
			gcMessage="<blink>Error: </blink>cParam4: Double quoted regex string and optional SRV target required.";
		}

		sscanf(cParam1,"%u",&uI);
		if(!uI && !(isdigit(cParam1[0])))
		{
			cParam1Style="type_fields_req";
			gcMessage="<blink>Error: </blink>cParam1: Must specify numerical order";
		}

		uI=0;
		sscanf(cParam2,"%u",&uI);
		if(!uI && (!isdigit(cParam2[0])))
		{
			cParam2Style="type_fields_req";
			gcMessage="<blink>Error: </blink>cParam2: Must specify numerical preference";
		}

		//Check for double quotes
		uI=0;
		for(i=0;cParam3[i];i++)
			if(cParam3[i]=='\"') uI++;
		if(uI!=4)
		{
			cParam3Style="type_textarea_req";
			gcMessage="<blink>Error: </blink>cParam3: Must double quote both flags and ENUM string."
					" Ex: \"U\" \"E2U+sip\"";
		}

		uI=0;
		for(i=0;cParam4[i];i++)
			if(cParam4[i]=='\"') uI++;
		if(uI<2)
		{
			cParam4Style="type_textarea_req";
				gcMessage="<blink>Error: </blink>Must double quote REGEX."
					" Ex: \"!^.*$!sip:customer-service@example.com!\" _sip._udp.example.com";
		}


	}
	
	else if(1)
	{
		gcMessage="<blink>Must select valid Resource Type (A,MX,PTR,TXT,NS,CNAME,HINFO,NAPTR,AAAA)</blink>";
		cRRTypeStyle="type_fields_req";
		return(16);
	}
	
	return(0);

}//unsigned uPerRRTypeCheck(void)


unsigned InMyBlocks(char *cIP)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT cLabel FROM tBlock WHERE uOwner=%u OR uOwner=%u",
			guLoginClient,guOrg);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(uIpv4InCIDR4(cIP,field[0])==1)
		{
			mysql_free_result(res);
			return(1);
		}
	}
	return(0);

}//unsigned InMyBlocks(char *cIP)


void UpdateSerialNum(char *cZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	long unsigned luYearMonDay=0;
	unsigned uSerial=0;
	unsigned uZone=0;
	char cSerial[16]={""};


	sprintf(gcQuery,"SELECT uSerial,uZone FROM tZone WHERE cZone='%s' AND uView=2",cZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&uSerial);
		sscanf(field[1],"%u",&uZone);
	}
	mysql_free_result(res);
	
	SerialNum(cSerial);
	sscanf(cSerial,"%lu",&luYearMonDay);


	//Typical year month day and 99 changes per day max
	//to stay in correct date format. Will still increment even if>99 changes in one day
	//but will be stuck until 1 day goes by with no changes.
	if(uSerial<luYearMonDay)
		sprintf(gcQuery,"UPDATE tZone SET uSerial=%s WHERE uZone=%u",cSerial,uZone);
	else
		sprintf(gcQuery,"UPDATE tZone SET uSerial=uSerial+1 WHERE uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

}//void UpdateSerialNum()


void funcSelectRRType(FILE *fp, unsigned uUseStatus)
{

	fprintf(fp,"<!-- funcSelectRRType(fp) Start -->\n");
	
	if(uUseStatus)
		fprintf(fp,"<select %s name=cRRType class=%s ",gcInputStatus,cRRTypeStyle);
	else 
		fprintf(fp,"<select name=cRRType class=type_textarea ");

	if(guBrowserFirefox)
		fprintf(fp,"onChange='changePage(this.form.cRRType)'>\n");
	else
		fprintf(fp,"onChange='submit()'>\n");

	//Only allow PTR RRs
	if(strstr(gcZone,"in-addr.arpa"))
	{
		if(guBrowserFirefox)
		{
	if(uStep)
		fprintf(fp,"<option value='&cRRType=PTR&cZone=%s&gcFunction=%s&uResource=%u&uStep=%u'",gcZone,gcFunction,uResource,uStep);
	else
		fprintf(fp,"<option value='&cRRType=PTR&cZone=%s&gcFunction=%s&uResource=%u'",gcZone,gcFunction,uResource);
		}
		else
		{
			if(!strcmp(gcFunction,"New"))
				sprintf(gcFunction,"New Confirm");
			else if(!strcmp(gcFunction,"Modify"))
				sprintf(gcFunction,"Modify Confirm");
	if(uStep)
		fprintf(fp,"<option value='&cRRType=PTR&cZone=%s&gcFunction=%s&uResource=%u&uStep=%u'",gcZone,gcFunction,uResource,uStep);
	else
		fprintf(fp,"<option value='&cRRType=PTR&cZone=%s&gcFunction=%s&uResource=%u'",gcZone,gcFunction,uResource);
}
		if(!strcmp(cRRType,"PTR"))
			fprintf(fp,"selected");
		fprintf(fp,">PTR</option>");
	}
	else
	//Normal zones NO PTR and NO NS RRs allowed
	{
		MYSQL_RES *res;
		MYSQL_ROW field;

		sprintf(gcQuery,"SELECT cLabel FROM tRRType WHERE uRRType!=7 AND uRRType!=2");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
	
		while((field=mysql_fetch_row(res)))
		{

			if(guBrowserFirefox)			
			{
	if(uStep)
		fprintf(fp,"<option value='&cRRType=%s&cZone=%s&gcFunction=%s&uResource=%u&uStep=%u'",field[0],gcZone,gcFunction,uResource,uStep);
	else
		fprintf(fp,"<option value='&cRRType=%s&cZone=%s&gcFunction=%s&uResource=%u'",field[0],gcZone,gcFunction,uResource);
			}
			else
			{
				if(!strcmp(gcFunction,"New"))
					sprintf(gcFunction,"New Confirm");
				else if(!strcmp(gcFunction,"Modify"))
					sprintf(gcFunction,"Modify Confirm");
	if(uStep)				
		fprintf(fp,"<option value='&cRRType=%s&cZone=%s&gcFunction=%s&uResource=%u&uStep=%u'",field[0],gcZone,gcFunction,uResource,uStep);
	else
		fprintf(fp,"<option value='&cRRType=%s&cZone=%s&gcFunction=%s&uResource=%u'",field[0],gcZone,gcFunction,uResource);
			}
			if(!strcmp(cRRType,field[0]))
				fprintf(fp,"selected");
			fprintf(fp,">%s</option>\n",field[0]);
		}
		mysql_free_result(res);
	}

	fprintf(fp,"</select>\n");
	fprintf(fp,"<!-- funcSelectRRType(fp) End -->\n");

}//void funcSelectRRType(FILE *fp)


void LoadRRTypeLabels(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	if(!cRRType[0])
		sprintf(cRRType,"A");

	sprintf(gcQuery,"SELECT cParam1Label,cParam1Tip,cParam2Label,cParam2Tip,cNameLabel,"
			"cNameTip,cParam3Label,cParam3Tip,cParam4Label,cParam4Tip FROM "
			"tRRType WHERE cLabel='%s'",cRRType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sprintf(cParam1Label,"%.32s",field[0]);
		sprintf(cParam1Tip,"%.99s",field[1]);
		sprintf(cParam2Label,"%.32s",field[2]);
		sprintf(cParam2Tip,"%.99s",field[3]);
		sprintf(cNameLabel,"%.32s",field[4]);
		sprintf(cNameTip,"%.99s",field[5]);
		sprintf(cParam3Label,"%.32s",field[6]);
		sprintf(cParam3Tip,"%.99s",field[7]);
		sprintf(cParam4Label,"%.32s",field[8]);
		sprintf(cParam4Tip,"%.99s",field[9]);
	}
	mysql_free_result(res);
	
}//void LoadRRTypeLabels(void)


void LoadRRNoType(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT cName,uTTL,cParam1,cParam2,cParam3,cParam4,cComment FROM tResource WHERE uResource=%u",uResource);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sprintf(cName,"%.99s",field[0]);
		sprintf(cuTTL,"%.15s",field[1]);
		sprintf(cParam1,"%.255s",field[2]);
		sprintf(cParam2,"%.255s",field[3]);
		sprintf(cParam3,"%.255s",field[4]);
		sprintf(cParam4,"%.255s",field[5]);
		sprintf(cComment,"%.255s",field[6]);
	}
	mysql_free_result(res);
	
}//void LoadRRNoType(void)


void MasterFunctionSelect(void)
{
	if(!strcmp(gcFunction,"Modify"))
	{
		SelectResource();
		if(strstr(cComment,"Delegation"))
		{
			gcMessage="<blink>Modification of delegation records is not allowed</blink>";
			htmlResource();
		}
		sprintf(gcModStep," Confirm");
		ResourceSetFieldsOn();
		gcMessage="Modify data, review, then confirm. Any other action to cancel.";
		gcInputStatus[0]=0;
		htmlResource();
	}
	else if(!strcmp(gcFunction,"New"))
	{
		SelectResource();
		sprintf(gcNewStep," Confirm");
		ResourceSetFieldsOn();
		gcMessage="Enter/modify data, review, then confirm. Any other action to cancel.";
		gcInputStatus[0]=0;
		htmlResource();
	}
	else if(!strcmp(gcFunction,"Delete"))
	{
		SelectResource();
		sprintf(gcDelStep," Confirm");
		gcMessage="Double check you have selected the correct record to delete. Then confirm. Any other action to cancel.";
		htmlResource();
	}
	else if(!strcmp(gcFunction,"Modify Confirm"))
	{
		LoadRRTypeLabels();//Get labels and tips directly from cRRType
		UpdateResource();	
		if(strcmp(gcMessage,"Zone Resource Modified"))
		{
			sprintf(gcModStep," Confirm");
			gcInputStatus[0]=0;
			ResourceSetFieldsOn();
		}
		else
		{
			time_t luClock;
			unsigned uNameServer=0;

			time(&luClock);

			sprintf(gcInputStatus,"disabled");
			if(cuNameServer[0])
			{
				sscanf(cuNameServer,"%u",&uNameServer);
			}
			else if(gcZone[0])
			{
				uNameServer=uGetuNameServer(gcZone);
			}
			if(uNameServer)
			{
				UpdateSerialNum(gcZone);
				if(OrgSubmitJob("Modify",uNameServer,gcZone,0,luClock))
					htmlPlainTextError(mysql_error(&gMysql));
			}
			else
			{
				gcMessage="<blink>Contact admin: uNameServer error (mod)</blink>";
			}
		}
		htmlResource();
	}
	else if(!strcmp(gcFunction,"New Confirm"))
	{
		LoadRRTypeLabels();//Get labels and tips directly from cRRType
		NewResource();	
		if(strcmp(gcMessage,"Zone Resource Created"))
		{
			sprintf(gcNewStep," Confirm");
			gcInputStatus[0]=0;
			ResourceSetFieldsOn();
		}
		else
		{
			time_t luClock;
			unsigned uNameServer=0;

			time(&luClock);

			sprintf(gcInputStatus,"disabled");
			if(cuNameServer[0])
			{
				sscanf(cuNameServer,"%u",&uNameServer);
			}
			else if(gcZone[0])
			{
				uNameServer=uGetuNameServer(gcZone);
			}
			if(uNameServer)
			{
				UpdateSerialNum(gcZone);
				if(OrgSubmitJob("Modify",uNameServer,gcZone,0,luClock))
					htmlPlainTextError(mysql_error(&gMysql));
			}
			else
			{
				gcMessage="<blink>Contact admin: uNameServer error (new)</blink>";
			}
		}
		htmlResource();
	}
	else if(!strcmp(gcFunction,"Delete Confirm"))
	{
		SaveResource();
		DelResource();	
		if(strcmp(gcMessage,"Zone Resource Deleted"))
		{
			sprintf(gcDelStep," Confirm");
			gcInputStatus[0]=0;
		}
		else
		{
			time_t luClock;
			unsigned uNameServer=0;

			time(&luClock);

			sprintf(gcInputStatus,"disabled");
			if(cuNameServer[0])
			{
				sscanf(cuNameServer,"%u",&uNameServer);
			}
			else if(gcZone[0])
			{
				uNameServer=uGetuNameServer(gcZone);
			}
			if(uNameServer)
			{
				UpdateSerialNum(gcZone);
				if(OrgSubmitJob("Modify",uNameServer,gcZone,0,luClock))
					htmlPlainTextError(mysql_error(&gMysql));
			}
			else
			{
				gcMessage="<blink>Contact admin: uNameServer error (del)</blink>";
			}
		}
		LoadRRTypeLabels();
		htmlResource();
	}
	else if(!strcmp(gcFunction,"Add Resource Wizard"))
	{
		ResourceSetFieldsOn();
		htmlResourceWizard(1);
	}
	else if(!strcmp(gcFunction,"Next"))
	{
		register int i=0;

		LoadRRTypeLabels();
		ResourceSetFieldsOn();
		//
		//Validation switch per wizard step
		//
		switch(uStep)
		{
			case 1:
				
				if(!strcmp(cRRType,"SRV"))
				{
					gcMessage="<blink>Error: </blink>SRV records are not supported by this wizard.";
					htmlResourceWizard(uStep);
				}
				//remove extra chars
				if(cName[0])
				{
					sscanf(cName,"%s",gcQuery);
					sprintf(cName,"%.255s",gcQuery);
				}
	
				//
				//Validate cName
				if(!cName[0] && !strcmp(cRRType,"PTR"))
				{
					gcMessage="<blink>Resource name is required by us for PTR type records</blink>";
					cNameStyle="type_fields_req";
					htmlResourceWizard(uStep);
				}

				//2-. If it has a period must be full qually time
				if(strchr(cName,'.'))
				{
					sprintf(gcQuery,"%s.",gcZone);
					if(strcmp(gcQuery,cName))
					{
						sprintf(gcQuery,".%s.",gcZone);
						if(!strstr(cName+(strlen(cName)-strlen(gcQuery)),gcQuery))
						{
							if(strstr(cName+strlen(cName)-strlen(gcZone),gcZone))
							{
								strcat(cName,".");
								gcMessage="<blink>We have added a final period."
										" If this correct confirm</blink>";
								cNameStyle="type_fields_req";
								htmlResourceWizard(uStep);
							}
							gcMessage="<blink>If Name is fully qualified it must end with the zone"
									" and final period</blink>";
							htmlResourceWizard(uStep);
						}
					}
				}

				//2b-.
				//More cName validation. Do not allow .. or .- or -. in cName
				if(strstr(cName,".."))
				{
					gcMessage="<blink>Error: </blink>Name can't contain '..'";
					htmlResourceWizard(uStep);
				}
				if(strstr(cName,".-"))
				{
					gcMessage="<blink>Error: </blink>Name can't contain '.-'";
					htmlResourceWizard(uStep);
				}
				if(strstr(cName,"-."))
				{
					gcMessage="<blink>Error: </blink>Name can't contain '-.'";
					htmlResourceWizard(uStep);
				}


				//3-. Can only have digits, letters, dash and dots the ampersand and the asterix wild card.
				//This is mostly for the (default) problem we have experienced after deployment :(
				for(i=0;cName[i];i++)
				{
					if(!isalnum(cName[i]) && cName[i]!='-' && cName[i]!='.' && cName[i]!='@'&& cName[i]!='*' &&
						cName[i]!='_')
					{
						gcMessage="<blink>Name can be empty or have only letters, numbers, the default origin @ symbol. Or dashes (-) and periods (.)</blink>";
						cNameStyle="type_fields_req";
						htmlResourceWizard(uStep);
					}
				}

				FQDomainName(cName);
				break;
			case 2:
				
				//remove extra chars
				if(cParam1[0] && strcmp(cRRType,"TXT") && strcmp(cRRType,"HINFO"))
				{
					sscanf(cParam1,"%s",gcQuery);
					sprintf(cParam1,"%.99s",gcQuery);
				}

				if(cParam2[0] && strcmp(cRRType,"HINFO"))
				{
					sscanf(cParam2,"%s",gcQuery);
					sprintf(cParam2,"%.99s",gcQuery);
				}
				
				if(uPerRRTypeCheck())
					htmlResourceWizard(uStep);

				break;
		}//switch(uStep)
		uStep++;
		htmlResourceWizard(uStep);
	}
	else if(!strcmp(gcFunction,"Back"))
	{
		LoadRRTypeLabels();
		ResourceSetFieldsOn();
		uStep--;
		htmlResourceWizard(uStep);
	}
	else if(!strcmp(gcFunction,"Finish"))
	{
		LoadRRTypeLabels();//Get labels and tips directly from cRRType
		NewResource();	
		if(!strcmp(gcMessage,"Zone Resource Created"))
		{
			time_t luClock;
			unsigned uNameServer=0;

			time(&luClock);

			sprintf(gcInputStatus,"disabled");
			if(cuNameServer[0])
			{
				sscanf(cuNameServer,"%u",&uNameServer);
			}
			else if(gcZone[0])
			{
				uNameServer=uGetuNameServer(gcZone);
			}
			if(uNameServer)
			{
				UpdateSerialNum(gcZone);
				if(OrgSubmitJob("Modify",uNameServer,gcZone,0,luClock))
					htmlPlainTextError(mysql_error(&gMysql));
			}
			else
			{
				gcMessage="<blink>Contact admin: uNameServer error (new)</blink>";
			}
		}
		htmlResource();
	}
	else if(!strcmp(gcFunction,"Bulk Resource Record Entry"))
	{
		htmlBulkOp();
	}
}//


void htmlResourceWizard(unsigned uStep)
{
	char cTmp[16]={""};
	htmlHeader("idnsOrg","Header");
	sprintf(cTmp,"ResourceWizard.%.1u",uStep);
	htmlResourcePage("idnsOrg",cTmp);
	htmlFooter("Footer");
	
}//void htmlResourceWizard(unsigned uStep)


void ResourceSetFieldsOn(void)
{
	if(strcmp(cNameStyle,"type_fields_req"))
		cNameStyle="type_fields";
	if(strcmp(cuTTLStyle,"type_fields_req"))
		cuTTLStyle="type_fields";
	if(strcmp(cRRTypeStyle,"type_fields_req"))
		cRRTypeStyle="type_fields";
	if(strcmp(cParam1Style,"type_fields_req"))
		cParam1Style="type_fields";
	if(strcmp(cParam2Style,"type_fields_req"))
		cParam2Style="type_fields";
	if(strcmp(cCommentStyle,"type_fields_req"))
		cCommentStyle="type_fields";
	if(strcmp(cParam3Style,"type_fields_req"))
		cParam3Style="type_textarea";
	if(strcmp(cParam4Style,"type_fields_req"))
		cParam4Style="type_textarea";

}//void ResourceSetFieldsOn(void)


void SaveResource(void)
{
	//This function will sabe the deleted RRs into tDeletedResource
	unsigned uZone=0;
	unsigned uRRType=0;
	
	uZone=uGetuZone(gcZone);
	uRRType=SelectRRType(cRRType);
	sprintf(gcQuery,"INSERT INTO tDeletedResource SET uDeletedResource='%u',uZone='%u',"
			"cName='%s',uTTL='%s',uRRType='%u',cParam1='%s',cParam2='%s',cParam3='%s',"
			"cParam4='%s',cComment='%s',uOwner='%u',uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())"
			" ON DUPLICATE KEY UPDATE uDeletedResource='%u',uZone='%u',"
			"cName='%s',uTTL='%s',uRRType='%u',cParam1='%s',cParam2='%s',cParam3='%s',"
			"cParam4='%s',cComment='%s',uOwner='%u',uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uResource,
			uZone,
			TextAreaSave(cName),
			cuTTL,
			uRRType,
			TextAreaSave(cParam1),
			TextAreaSave(cParam2),
			TextAreaSave(cParam3),
			TextAreaSave(cParam4),
			cComment,
			guOrg,
			uResource,
			uZone,
			TextAreaSave(cName),
			cuTTL,
			uRRType,
			TextAreaSave(cParam1),
			TextAreaSave(cParam2),
			TextAreaSave(cParam3),
			TextAreaSave(cParam4),
			cComment,
			guOrg);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

}//void SaveResource(void)


unsigned idnsOnLineZoneCheck(void)
{
	//This function will create a zonefile online and run named-checkzone
	MYSQL_RES *res;
	MYSQL_ROW field;
	MYSQL_RES *res2;
	MYSQL_ROW field2;
	FILE *zfp;
	FILE *dnfp;
	unsigned uZone=0;
	unsigned uRRType=0;
	char cTTL[50]={""};
	char cZoneFile[100]={""};

	sprintf(cZoneFile,"/tmp/%s",gcZone);

	if((zfp=fopen(cZoneFile,"w"))==NULL)
		htmlPlainTextError("fopen() failed for temp zonefile");

	if((dnfp=fopen("/dev/null","w"))==NULL)
		htmlPlainTextError("fopen() failed for /dev/null");

	sprintf(gcQuery,"SELECT tZone.cZone,tZone.uZone,tZone.uNSSet,tZone.cHostmaster,"
			"tZone.uSerial,tZone.uTTL,tZone.uExpire,tZone.uRefresh,tZone.uRetry,tZone.uZoneTTL,"
			"tZone.uMailServers,tZone.cMainAddress,tView.cLabel FROM tZone,tNSSet,tNS,tView"
			" WHERE tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND"
			" tZone.uView=tView.uView AND tZone.cZone='%s' AND tZone.uView=2",gcZone);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	if((field=mysql_fetch_row(res)))
	{
		char *cp;
		char cFirstNS[100]={""};
		
		//0 cZone
		//1 uZone
		//2 uNameServer
		//3 cHostmaster
		//4 uSerial
		//
		//5 uTTL default TTL macro directive $TTL. See bind 8.2.x docs
		//
		//6 uExpire
		//7 uRefresh
		//8 uRetry
		//9 uZoneTTL is the negative response caching value in the SOA.
		//  See bind 8.2.x docs
		//
		//10 uMailServers
		//11 cMainAddress
		//12 tView.cLabel
		//13 tView.cMaster
		//14 tView.uOrder
		sscanf(field[1],"%u",&uZone);
	
		if((cp=strchr(field[3],' '))) *cp=0;

		fprintf(zfp,"; %s\n",field[0]);
		fprintf(zfp,"$TTL %s\n",field[5]);
	
		//MASTER HIDDEN support
		sprintf(cFirstNS,"%.99s",cPrintNSList(dnfp,field[2]));
		if(cFirstNS[0])
			fprintf(zfp,
			"@ IN SOA %s. %s. (\n",cFirstNS,field[3]);
/*		else
			fprintf(zfp,
			"@ IN SOA %s. %s. (\n",cMasterNS,field[3]);*/
		fprintf(zfp,"\t\t\t%s\t;serial\n",field[4]);
		fprintf(zfp,"\t\t\t%s\t\t;slave refresh\n",field[7]);
		fprintf(zfp,"\t\t\t%s\t\t;slave retry\n",field[8]);
		fprintf(zfp,"\t\t\t%s\t\t;slave expiration\n",field[6]);
		fprintf(zfp,"\t\t\t%s )\t\t;negative ttl\n\n",
			field[9]);

		//ns
		cPrintNSList(zfp,field[2]);

		//mx1
		PrintMXList(zfp,field[10]);
		//in a 0.0.0.0 is the null IP number
		if(field[11][0] && field[11][0]!='0')
			fprintf(zfp,"\t\tA %s\n;\n",field[11]);
		fprintf(zfp,";\n");

		//TODO
		if(!strcmp(field[0]+strlen(field[0])-5,".arpa"))
			sprintf(gcQuery,"SELECT cName,uTTL,uRRType,cParam1,cParam2 FROM tResourceTest WHERE uZone=%u ORDER BY uResource",uZone);
		else
			sprintf(gcQuery,"SELECT cName,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4 FROM tResourceTest WHERE uZone=%u ORDER BY cName",uZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
		res2=mysql_store_result(&gMysql);
		while((field2=mysql_fetch_row(res2)))
		{
			char cRRType[9]="";

			sscanf(field2[2],"%u",&uRRType);
			sprintf(cRRType,"%.8s",GetRRType(uRRType));

			if(field2[1][0]!='0') strcpy(cTTL,field2[1]);

			//Do not write TTL if cName is a $GENERATE line
			if(strstr(field2[0],"$GENERATE")==NULL)
			{
				if(!strcmp(cRRType,"SRV"))
					fprintf(zfp,"%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
							field2[0],
							cTTL,
							cRRType,
							field2[3],
							field2[4],
							field2[5],
							field2[6]);
				else if(!strcmp(cRRType,"NAPTR"))
					fprintf(zfp,"%s\t%s\t%s\t%s\t%s\t(%s\t%s)\n",
							field2[0],
							cTTL,
							cRRType,
							field2[3],
							field2[4],
							field2[5],
							field2[6]);
				else if(1)
					fprintf(zfp,"%s\t%s\t%s\t%s\t%s\n",
							field2[0],
							cTTL,
							cRRType,
							field2[3],
							field2[4]);
			}
			else
			{
				fprintf(zfp,"%s\t%s\t%s\t%s\n",
						field2[0],
						cRRType,
						field2[3],
						field2[4]);
			}
		}
	
		mysql_free_result(res2);
		fclose(zfp);

		//**** IMPORTANT NOTE *****
		//Check the named-checkzone binary location for your setup
		//
		sprintf(gcQuery,"/usr/sbin/named-checkzone %s %s 2>&1 > /dev/null",field[0],cZoneFile);
		if(system(gcQuery))
		{
			unsigned uMessageSet=0;
			char cLine[1024]={""};
			sprintf(gcQuery,"/usr/sbin/named-checkzone %s %s 2>&1",field[0],cZoneFile);
			if((zfp=popen(gcQuery,"r"))==NULL)
				htmlPlainTextError("popen() failed");
			
			//zone clonetest.com/IN: loading master file /tmp/clonetest.com: CNAME and other data
			while(fgets(cLine,sizeof cLine,zfp)!=NULL)
			{
				if(strstr(cLine,"zone"))
				{
					char *cp;
					if((cp=strstr(cLine,cZoneFile)))
					{
						cp=cp+strlen(cZoneFile)+2; //2 more chars ': '
						gcMessage=malloc(256);
						sprintf(gcMessage,"<blink>Error: </blink> The RR has an error: %s",cp);
						uMessageSet=1;
					}
				}
				else if(strstr(cLine,"sh: /usr/sbin/named-checkzone: Permission denied"))
				{
					gcMessage="<blink>Error: </blink> Can't execute named-checkzone. Fix your permissions";
					uMessageSet=1;
				}
			}
			pclose(zfp);
			unlink(cZoneFile);
			
			if(!uMessageSet)
				gcMessage="No message was set but named-checkzone failed. "
					"Tipically this indicates a setup problem. Contact support ASAP!";
			return(1);
		}
	}
	unlink(cZoneFile);

	return(0);

}//unsigned idnsOnLineZoneCheck(void)


char *GetRRType(unsigned uRRType)
{
	MYSQL_RES *res;
	static char cRRType[100];
	
	strcpy(cRRType,"unknown");

	sprintf(gcQuery,"SELECT cLabel FROM tRRType WHERE uRRType=%u",uRRType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	
	if(mysql_num_rows(res)==1) 
	{
		MYSQL_ROW field;
		if((field=mysql_fetch_row(res)))
			strcpy(cRRType,field[0]);

	}
	mysql_free_result(res);
	return(cRRType);

}//char *GetRRType(unsigned uRRType)


char *cPrintNSList(FILE *zfp,char *cuNSSet)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	static char cFirstNS[100]={""};
	unsigned uFirst=1;

	//Do not include HIDDEN NSs. 2 is the fixed HIDDEN TYPE
	//This ORDER BY implies that people will use a NS scheme that is alphabetical
	//in nature like ns1, ns2 etc. This will need to be looked at later ;)
	sprintf(gcQuery,"SELECT tNS.cFQDN,tNS.uNSType FROM tNSSet,tNS WHERE tNSSet.uNSSet=tNS.uNSSet AND"
			" tNS.uNSType!=2 AND tNSSet.uNSSet=%s ORDER BY tNS.cFQDN",cuNSSet);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
			fprintf(zfp,"\t\tNS %s.\n",FQDomainName(field[0]));
			if(uFirst)
			{
				sprintf(cFirstNS,"%.99s",field[0]);
				uFirst=0;
			}
	}
	mysql_free_result(res);

	return(cFirstNS);

}//char *cPrintNSList()


//Old cList based list. Will be deprecated to tMXSet based sub-schema for 2.8 release
void PrintMXList(FILE *zfp,char *cuMailServers)
{
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT cList FROM tMailServer WHERE uMailServer=%s",
			cuMailServers);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	
	if(mysql_num_rows(res)==1) 
	{
		MYSQL_ROW field;
		register int i=0,j=0,uMX=10;

		if((field=mysql_fetch_row(res)))
		{
			//parse out
			while(field[0][i])
			{
				if(field[0][i]=='\n' || field[0][i]==0)
				{
					field[0][i]=0;
					if(strlen(FQDomainName(field[0]+j)))
					{	
						fprintf(zfp,"\t\tMX %d %s.\n",uMX,
							FQDomainName(field[0]+j));
					}
					i++;
					j=i;
					uMX+=10;
				}
				i++;
			}
			if(field[0][j] && strlen(FQDomainName(field[0]+j)))
			{	
				fprintf(zfp,"\t\tMX %d %s.\n",uMX,
					FQDomainName(field[0]+j));
			}
		}
	}
	mysql_free_result(res);
}//PrintMXList(FILE *fp,char *cuMailServers)


void CreatetResourceTest(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tResourceTest ( uResource INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cName VARCHAR(100) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, uTTL INT UNSIGNED NOT NULL DEFAULT 0, uRRType INT UNSIGNED NOT NULL DEFAULT 0, cParam1 VARCHAR(255) NOT NULL DEFAULT '', cParam2 VARCHAR(255) NOT NULL DEFAULT '', cComment TEXT NOT NULL DEFAULT '', uZone INT UNSIGNED NOT NULL DEFAULT 0,index (uZone), cParam3 VARCHAR(255) NOT NULL DEFAULT '', cParam4 VARCHAR(255) NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//void CreatetRestResource(void)


void PrepareTestData(void)
{
	unsigned uZone=uGetuZone(gcZone);
	unsigned uRRType=SelectRRType(cRRType);

	CreatetResourceTest();
	sprintf(gcQuery,"TRUNCATE tResourceTest");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	sprintf(gcQuery,"INSERT INTO tResourceTest (uResource,cName,uOwner,uCreatedBy,uCreatedDate,uModBy,"
			"uModDate,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4,cComment,uZone) "
			"SELECT uResource,cName,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate,uTTL,uRRType,"
			"cParam1,cParam2,cParam3,cParam4,cComment,uZone FROM tResource WHERE "
			"uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	if(!strcmp(gcFunction,"New Confirm") || !strcmp(gcFunction,"Finish"))
		sprintf(gcQuery,"INSERT INTO tResourceTest SET cName='%s',uTTL=%s,uRRType=%u,cParam1='%s'"
				",cParam2='%s',cParam3='%s',cParam4='%s',cComment='%s',uOwner=%u,uCreatedBy=%u,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW()),uZone=%u",
				cName,
				cuTTL,
				uRRType,
				cParam1,
				cParam2,
				cParam3,
				cParam4,
				TextAreaSave(cComment),
				guOrg,
				guLoginClient,
				uZone);
	else if(!strcmp(gcFunction,"Modify Confirm"))
		sprintf(gcQuery,"UPDATE tResourceTest SET cName='%s',uTTL=%s,uRRType=%u,cParam1='%s',cParam2='%s',"
				"cParam3='%s',cParam4='%s',cComment='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) "
				"WHERE uResource=%u",
				cName,
				cuTTL,
				uRRType,
				cParam1,
				cParam2,
				cParam3,
				cParam4,
				TextAreaSave(cComment),
				guLoginClient,
				uResource);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

}//void PrepareTestData(void)

