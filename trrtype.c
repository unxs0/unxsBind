/*
FILE
	tRRType source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Resource records type codes. A, AAAA, MX, etc.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uRRType: Primary Key
static unsigned uRRType=0;
//cLabel: Short label
static char cLabel[33]={""};
//uParam1: Requires parameter 1
static unsigned uParam1=0;
static char cYesNouParam1[32]={""};
//uParam2: Requires parameter 2
static unsigned uParam2=0;
static char cYesNouParam2[32]={""};
//cParam1Func: Input validation function
static char cParam1Func[33]={""};
//cParam2Func: Input validation function
static char cParam2Func[33]={""};
//cParam3Func: Input validation function
static char cParam3Func[33]={""};
//cParam4Func: Input validation function
static char cParam4Func[33]={""};
//cParam1Label: Label for interfaces
static char cParam1Label[33]={""};
//cParam2Label: Label for interfaces
static char cParam2Label[33]={""};
//cParam3Label: Label for interfaces
static char cParam3Label[33]={""};
//cParam4Label: Label for interfaces
static char cParam4Label[33]={""};
//cParam1Tip: Html title tool-tip for interfaces
static char cParam1Tip[101]={""};
//cParam2Tip: Html title tool-tip for interfaces
static char cParam2Tip[101]={""};
//cParam3Tip: Html title tool-tip for interfaces
static char cParam3Tip[101]={""};
//cParam4Tip: Html title tool-tip for interfaces
static char cParam4Tip[101]={""};
//uParam3: Requires parameter 3
static unsigned uParam3=0;
static char cYesNouParam3[32]={""};
//uParam4: Requires parameter 4
static unsigned uParam4=0;
static char cYesNouParam4[32]={""};
//uName: Requires tResource.cName
static unsigned uName=0;
static char cYesNouName[32]={""};
//cNameFunc: Input validation function
static char cNameFunc[33]={""};
//cNameLabel: Label for interfaces
static char cNameLabel[33]={""};
//cNameTip: Html title tool-tip for interfaces
static char cNameTip[101]={""};
//uOwner: Record owner
static unsigned uOwner=0;
//uCreatedBy: uClient for last insert
static unsigned uCreatedBy=0;
#define ISM3FIELDS
//uCreatedDate: Unix seconds date last insert
static long uCreatedDate=0;
//uModBy: uClient for last update
static unsigned uModBy=0;
//uModDate: Unix seconds date last update
static long uModDate=0;



#define VAR_LIST_tRRType "tRRType.uRRType,tRRType.cLabel,tRRType.uParam1,tRRType.uParam2,tRRType.cParam1Func,tRRType.cParam2Func,tRRType.cParam3Func,tRRType.cParam4Func,tRRType.cParam1Label,tRRType.cParam2Label,tRRType.cParam3Label,tRRType.cParam4Label,tRRType.cParam1Tip,tRRType.cParam2Tip,tRRType.cParam3Tip,tRRType.cParam4Tip,tRRType.uParam3,tRRType.uParam4,tRRType.uName,tRRType.cNameFunc,tRRType.cNameLabel,tRRType.cNameTip,tRRType.uOwner,tRRType.uCreatedBy,tRRType.uCreatedDate,tRRType.uModBy,tRRType.uModDate"

 //Local only
void Insert_tRRType(void);
void Update_tRRType(char *cRowid);
void ProcesstRRTypeListVars(pentry entries[], int x);

 //In tRRTypefunc.h file included below
void ExtProcesstRRTypeVars(pentry entries[], int x);
void ExttRRTypeCommands(pentry entries[], int x);
void ExttRRTypeButtons(void);
void ExttRRTypeNavBar(void);
void ExttRRTypeGetHook(entry gentries[], int x);
void ExttRRTypeSelect(void);
void ExttRRTypeSelectRow(void);
void ExttRRTypeListSelect(void);
void ExttRRTypeListFilter(void);
void ExttRRTypeAuxTable(void);

#include "trrtypefunc.h"

 //Table Variables Assignment Function
void ProcesstRRTypeVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uRRType"))
			sscanf(entries[i].val,"%u",&uRRType);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"uParam1"))
			sscanf(entries[i].val,"%u",&uParam1);
		else if(!strcmp(entries[i].name,"cYesNouParam1"))
		{
			sprintf(cYesNouParam1,"%.31s",entries[i].val);
			uParam1=ReadYesNoPullDown(cYesNouParam1);
		}
		else if(!strcmp(entries[i].name,"uParam2"))
			sscanf(entries[i].val,"%u",&uParam2);
		else if(!strcmp(entries[i].name,"cYesNouParam2"))
		{
			sprintf(cYesNouParam2,"%.31s",entries[i].val);
			uParam2=ReadYesNoPullDown(cYesNouParam2);
		}
		else if(!strcmp(entries[i].name,"cParam1Func"))
			sprintf(cParam1Func,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam2Func"))
			sprintf(cParam2Func,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam3Func"))
			sprintf(cParam3Func,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam4Func"))
			sprintf(cParam4Func,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam1Label"))
			sprintf(cParam1Label,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam2Label"))
			sprintf(cParam2Label,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam3Label"))
			sprintf(cParam3Label,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam4Label"))
			sprintf(cParam4Label,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam1Tip"))
			sprintf(cParam1Tip,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam2Tip"))
			sprintf(cParam2Tip,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam3Tip"))
			sprintf(cParam3Tip,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam4Tip"))
			sprintf(cParam4Tip,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"uParam3"))
			sscanf(entries[i].val,"%u",&uParam3);
		else if(!strcmp(entries[i].name,"cYesNouParam3"))
		{
			sprintf(cYesNouParam3,"%.31s",entries[i].val);
			uParam3=ReadYesNoPullDown(cYesNouParam3);
		}
		else if(!strcmp(entries[i].name,"uParam4"))
			sscanf(entries[i].val,"%u",&uParam4);
		else if(!strcmp(entries[i].name,"cYesNouParam4"))
		{
			sprintf(cYesNouParam4,"%.31s",entries[i].val);
			uParam4=ReadYesNoPullDown(cYesNouParam4);
		}
		else if(!strcmp(entries[i].name,"uName"))
			sscanf(entries[i].val,"%u",&uName);
		else if(!strcmp(entries[i].name,"cYesNouName"))
		{
			sprintf(cYesNouName,"%.31s",entries[i].val);
			uName=ReadYesNoPullDown(cYesNouName);
		}
		else if(!strcmp(entries[i].name,"cNameFunc"))
			sprintf(cNameFunc,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cNameLabel"))
			sprintf(cNameLabel,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cNameTip"))
			sprintf(cNameTip,"%.100s",entries[i].val);
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

	//After so we can overwrite form data if needed.
	ExtProcesstRRTypeVars(entries,x);

}//ProcesstRRTypeVars()


void ProcesstRRTypeListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uRRType);
                        guMode=2002;
                        tRRType("");
                }
        }
}//void ProcesstRRTypeListVars(pentry entries[], int x)


int tRRTypeCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttRRTypeCommands(entries,x);

	if(!strcmp(gcFunction,"tRRTypeTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tRRTypeList();
		}

		//Default
		ProcesstRRTypeVars(entries,x);
		tRRType("");
	}
	else if(!strcmp(gcFunction,"tRRTypeList"))
	{
		ProcessControlVars(entries,x);
		ProcesstRRTypeListVars(entries,x);
		tRRTypeList();
	}

	return(0);

}//tRRTypeCommands()


void tRRType(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttRRTypeSelectRow();
		else
			ExttRRTypeSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetRRType();
				iDNS("New tRRType table created");
                	}
			else
			{
				htmlPlainTextError(mysql_error(&gMysql));
			}
        	}

		res=mysql_store_result(&gMysql);
		if((guI=mysql_num_rows(res)))
		{
			if(guMode==6)
			{
			sprintf(gcQuery,"SELECT _rowid FROM tRRType WHERE uRRType=%u"
						,uRRType);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uRRType);
		sprintf(cLabel,"%.32s",field[1]);
		sscanf(field[2],"%u",&uParam1);
		sscanf(field[3],"%u",&uParam2);
		sprintf(cParam1Func,"%.32s",field[4]);
		sprintf(cParam2Func,"%.32s",field[5]);
		sprintf(cParam3Func,"%.32s",field[6]);
		sprintf(cParam4Func,"%.32s",field[7]);
		sprintf(cParam1Label,"%.32s",field[8]);
		sprintf(cParam2Label,"%.32s",field[9]);
		sprintf(cParam3Label,"%.32s",field[10]);
		sprintf(cParam4Label,"%.32s",field[11]);
		sprintf(cParam1Tip,"%.100s",field[12]);
		sprintf(cParam2Tip,"%.100s",field[13]);
		sprintf(cParam3Tip,"%.100s",field[14]);
		sprintf(cParam4Tip,"%.100s",field[15]);
		sscanf(field[16],"%u",&uParam3);
		sscanf(field[17],"%u",&uParam4);
		sscanf(field[18],"%u",&uName);
		sprintf(cNameFunc,"%.32s",field[19]);
		sprintf(cNameLabel,"%.32s",field[20]);
		sprintf(cNameTip,"%.100s",field[21]);
		sscanf(field[22],"%u",&uOwner);
		sscanf(field[23],"%u",&uCreatedBy);
		sscanf(field[24],"%lu",&uCreatedDate);
		sscanf(field[25],"%u",&uModBy);
		sscanf(field[26],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tRRType",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tRRTypeTools>");
	printf("<input type=hidden name=gluRowid value=%lu>",gluRowid);
	if(guI)
	{
		if(guMode==6)
			//printf(" Found");
			printf(LANG_NBR_FOUND);
		else if(guMode==5)
			//printf(" Modified");
			printf(LANG_NBR_MODIFIED);
		else if(guMode==4)
			//printf(" New");
			printf(LANG_NBR_NEW);
		printf(LANG_NBRF_SHOWING,gluRowid,guI);
	}
	else
	{
		if(!cResult[0])
		//printf(" No records found");
		printf(LANG_NBR_NORECS);
	}
	if(cResult[0]) printf("%s",cResult);
	printf("</td></tr>");
	printf("<tr><td valign=top width=25%%>");

        ExttRRTypeButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tRRType Record Data",100);

	if(guMode==2000 || guMode==2002)
		tRRTypeInput(1);
	else
		tRRTypeInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttRRTypeAuxTable();

	Footer_ism3();

}//end of tRRType();


void tRRTypeInput(unsigned uMode)
{

//uRRType
	OpenRow(LANG_FL_tRRType_uRRType,"black");
	printf("<input title='%s' type=text name=uRRType value=%u size=16 maxlength=10 "
,LANG_FT_tRRType_uRRType,uRRType);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uRRType value=%u >\n",uRRType);
	}
//cLabel
	OpenRow(LANG_FL_tRRType_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=60 maxlength=32 "
,LANG_FT_tRRType_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//uParam1
	OpenRow(LANG_FL_tRRType_uParam1,"black");
	if(guPermLevel>=0 && uMode)
		YesNoPullDown("uParam1",uParam1,1);
	else
		YesNoPullDown("uParam1",uParam1,0);
//uParam2
	OpenRow(LANG_FL_tRRType_uParam2,"black");
	if(guPermLevel>=0 && uMode)
		YesNoPullDown("uParam2",uParam2,1);
	else
		YesNoPullDown("uParam2",uParam2,0);
//cParam1Func
	OpenRow(LANG_FL_tRRType_cParam1Func,"black");
	printf("<input title='%s' type=text name=cParam1Func value=\"%s\" size=60 maxlength=32 "
,LANG_FT_tRRType_cParam1Func,EncodeDoubleQuotes(cParam1Func));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam1Func value=\"%s\">\n",EncodeDoubleQuotes(cParam1Func));
	}
//cParam2Func
	OpenRow(LANG_FL_tRRType_cParam2Func,"black");
	printf("<input title='%s' type=text name=cParam2Func value=\"%s\" size=60 maxlength=32 "
,LANG_FT_tRRType_cParam2Func,EncodeDoubleQuotes(cParam2Func));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam2Func value=\"%s\">\n",EncodeDoubleQuotes(cParam2Func));
	}
//cParam3Func
	OpenRow(LANG_FL_tRRType_cParam3Func,"black");
	printf("<input title='%s' type=text name=cParam3Func value=\"%s\" size=60 maxlength=32 "
,LANG_FT_tRRType_cParam3Func,EncodeDoubleQuotes(cParam3Func));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam3Func value=\"%s\">\n",EncodeDoubleQuotes(cParam3Func));
	}
//cParam4Func
	OpenRow(LANG_FL_tRRType_cParam4Func,"black");
	printf("<input title='%s' type=text name=cParam4Func value=\"%s\" size=60 maxlength=32 "
,LANG_FT_tRRType_cParam4Func,EncodeDoubleQuotes(cParam4Func));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam4Func value=\"%s\">\n",EncodeDoubleQuotes(cParam4Func));
	}
//cParam1Label
	OpenRow(LANG_FL_tRRType_cParam1Label,"black");
	printf("<input title='%s' type=text name=cParam1Label value=\"%s\" size=60 maxlength=32 "
,LANG_FT_tRRType_cParam1Label,EncodeDoubleQuotes(cParam1Label));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam1Label value=\"%s\">\n",EncodeDoubleQuotes(cParam1Label));
	}
//cParam2Label
	OpenRow(LANG_FL_tRRType_cParam2Label,"black");
	printf("<input title='%s' type=text name=cParam2Label value=\"%s\" size=60 maxlength=32 "
,LANG_FT_tRRType_cParam2Label,EncodeDoubleQuotes(cParam2Label));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam2Label value=\"%s\">\n",EncodeDoubleQuotes(cParam2Label));
	}
//cParam3Label
	OpenRow(LANG_FL_tRRType_cParam3Label,"black");
	printf("<input title='%s' type=text name=cParam3Label value=\"%s\" size=60 maxlength=32 "
,LANG_FT_tRRType_cParam3Label,EncodeDoubleQuotes(cParam3Label));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam3Label value=\"%s\">\n",EncodeDoubleQuotes(cParam3Label));
	}
//cParam4Label
	OpenRow(LANG_FL_tRRType_cParam4Label,"black");
	printf("<input title='%s' type=text name=cParam4Label value=\"%s\" size=60 maxlength=32 "
,LANG_FT_tRRType_cParam4Label,EncodeDoubleQuotes(cParam4Label));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam4Label value=\"%s\">\n",EncodeDoubleQuotes(cParam4Label));
	}
//cParam1Tip
	OpenRow(LANG_FL_tRRType_cParam1Tip,"black");
	printf("<input title='%s' type=text name=cParam1Tip value=\"%s\" size=60 maxlength=100 "
,LANG_FT_tRRType_cParam1Tip,EncodeDoubleQuotes(cParam1Tip));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam1Tip value=\"%s\">\n",EncodeDoubleQuotes(cParam1Tip));
	}
//cParam2Tip
	OpenRow(LANG_FL_tRRType_cParam2Tip,"black");
	printf("<input title='%s' type=text name=cParam2Tip value=\"%s\" size=60 maxlength=100 "
,LANG_FT_tRRType_cParam2Tip,EncodeDoubleQuotes(cParam2Tip));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam2Tip value=\"%s\">\n",EncodeDoubleQuotes(cParam2Tip));
	}
//cParam3Tip
	OpenRow(LANG_FL_tRRType_cParam3Tip,"black");
	printf("<input title='%s' type=text name=cParam3Tip value=\"%s\" size=60 maxlength=100 "
,LANG_FT_tRRType_cParam3Tip,EncodeDoubleQuotes(cParam3Tip));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam3Tip value=\"%s\">\n",EncodeDoubleQuotes(cParam3Tip));
	}
//cParam4Tip
	OpenRow(LANG_FL_tRRType_cParam4Tip,"black");
	printf("<input title='%s' type=text name=cParam4Tip value=\"%s\" size=60 maxlength=100 "
,LANG_FT_tRRType_cParam4Tip,EncodeDoubleQuotes(cParam4Tip));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam4Tip value=\"%s\">\n",EncodeDoubleQuotes(cParam4Tip));
	}
//uParam3
	OpenRow(LANG_FL_tRRType_uParam3,"black");
	if(guPermLevel>=0 && uMode)
		YesNoPullDown("uParam3",uParam3,1);
	else
		YesNoPullDown("uParam3",uParam3,0);
//uParam4
	OpenRow(LANG_FL_tRRType_uParam4,"black");
	if(guPermLevel>=0 && uMode)
		YesNoPullDown("uParam4",uParam4,1);
	else
		YesNoPullDown("uParam4",uParam4,0);
//uName
	OpenRow(LANG_FL_tRRType_uName,"black");
	if(guPermLevel>=0 && uMode)
		YesNoPullDown("uName",uName,1);
	else
		YesNoPullDown("uName",uName,0);
//cNameFunc
	OpenRow(LANG_FL_tRRType_cNameFunc,"black");
	printf("<input title='%s' type=text name=cNameFunc value=\"%s\" size=60 maxlength=32 "
,LANG_FT_tRRType_cNameFunc,EncodeDoubleQuotes(cNameFunc));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cNameFunc value=\"%s\">\n",EncodeDoubleQuotes(cNameFunc));
	}
//cNameLabel
	OpenRow(LANG_FL_tRRType_cNameLabel,"black");
	printf("<input title='%s' type=text name=cNameLabel value=\"%s\" size=60 maxlength=32 "
,LANG_FT_tRRType_cNameLabel,EncodeDoubleQuotes(cNameLabel));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cNameLabel value=\"%s\">\n",EncodeDoubleQuotes(cNameLabel));
	}
//cNameTip
	OpenRow(LANG_FL_tRRType_cNameTip,"black");
	printf("<input title='%s' type=text name=cNameTip value=\"%s\" size=60 maxlength=100 "
,LANG_FT_tRRType_cNameTip,EncodeDoubleQuotes(cNameTip));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cNameTip value=\"%s\">\n",EncodeDoubleQuotes(cNameTip));
	}
//uOwner
	OpenRow(LANG_FL_tRRType_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tRRType_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tRRType_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tRRType_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tRRType_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tRRTypeInput(unsigned uMode)


void NewtRRType(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uRRType FROM tRRType\
				WHERE uRRType=%u"
							,uRRType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tRRType("<blink>Record already exists");
		tRRType(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tRRType();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uRRType=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tRRType",uRRType);
	iDNSLog(uRRType,"tRRType","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uRRType);
	tRRType(gcQuery);
	}

}//NewtRRType(unsigned uMode)


void DeletetRRType(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tRRType WHERE uRRType=%u AND ( uOwner=%u OR %u>9 )"
					,uRRType,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tRRType WHERE uRRType=%u"
					,uRRType);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tRRType("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uRRType,"tRRType","Del");
#endif
		tRRType(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uRRType,"tRRType","DelError");
#endif
		tRRType(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetRRType(void)


void Insert_tRRType(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tRRType SET uRRType=%u,cLabel='%s',uParam1=%u,uParam2=%u,cParam1Func='%s',cParam2Func='%s',cParam3Func='%s',cParam4Func='%s',cParam1Label='%s',cParam2Label='%s',cParam3Label='%s',cParam4Label='%s',cParam1Tip='%s',cParam2Tip='%s',cParam3Tip='%s',cParam4Tip='%s',uParam3=%u,uParam4=%u,uName=%u,cNameFunc='%s',cNameLabel='%s',cNameTip='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uRRType
			,TextAreaSave(cLabel)
			,uParam1
			,uParam2
			,TextAreaSave(cParam1Func)
			,TextAreaSave(cParam2Func)
			,TextAreaSave(cParam3Func)
			,TextAreaSave(cParam4Func)
			,TextAreaSave(cParam1Label)
			,TextAreaSave(cParam2Label)
			,TextAreaSave(cParam3Label)
			,TextAreaSave(cParam4Label)
			,TextAreaSave(cParam1Tip)
			,TextAreaSave(cParam2Tip)
			,TextAreaSave(cParam3Tip)
			,TextAreaSave(cParam4Tip)
			,uParam3
			,uParam4
			,uName
			,TextAreaSave(cNameFunc)
			,TextAreaSave(cNameLabel)
			,TextAreaSave(cNameTip)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tRRType(void)


void Update_tRRType(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tRRType SET uRRType=%u,cLabel='%s',uParam1=%u,uParam2=%u,cParam1Func='%s',cParam2Func='%s',cParam3Func='%s',cParam4Func='%s',cParam1Label='%s',cParam2Label='%s',cParam3Label='%s',cParam4Label='%s',cParam1Tip='%s',cParam2Tip='%s',cParam3Tip='%s',cParam4Tip='%s',uParam3=%u,uParam4=%u,uName=%u,cNameFunc='%s',cNameLabel='%s',cNameTip='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uRRType
			,TextAreaSave(cLabel)
			,uParam1
			,uParam2
			,TextAreaSave(cParam1Func)
			,TextAreaSave(cParam2Func)
			,TextAreaSave(cParam3Func)
			,TextAreaSave(cParam4Func)
			,TextAreaSave(cParam1Label)
			,TextAreaSave(cParam2Label)
			,TextAreaSave(cParam3Label)
			,TextAreaSave(cParam4Label)
			,TextAreaSave(cParam1Tip)
			,TextAreaSave(cParam2Tip)
			,TextAreaSave(cParam3Tip)
			,TextAreaSave(cParam4Tip)
			,uParam3
			,uParam4
			,uName
			,TextAreaSave(cNameFunc)
			,TextAreaSave(cNameLabel)
			,TextAreaSave(cNameTip)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tRRType(void)


void ModtRRType(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uPreModDate=0;

	//Mod select gcQuery
	sprintf(gcQuery,"SELECT uRRType,uModDate FROM tRRType WHERE uRRType=%u"
						,uRRType);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tRRType("<blink>Record does not exist");
	if(i<1) tRRType(LANG_NBR_RECNOTEXIST);
	//if(i>1) tRRType("<blink>Multiple rows!");
	if(i>1) tRRType(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tRRType(LANG_NBR_EXTMOD);
#endif

	Update_tRRType(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tRRType",uRRType);
	iDNSLog(uRRType,"tRRType","Mod");
#endif
	tRRType(gcQuery);

}//ModtRRType(void)


void tRRTypeList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttRRTypeListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tRRTypeList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttRRTypeListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uRRType<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>uParam1<td><font face=arial,helvetica color=white>uParam2<td><font face=arial,helvetica color=white>cParam1Func<td><font face=arial,helvetica color=white>cParam2Func<td><font face=arial,helvetica color=white>cParam3Func<td><font face=arial,helvetica color=white>cParam4Func<td><font face=arial,helvetica color=white>cParam1Label<td><font face=arial,helvetica color=white>cParam2Label<td><font face=arial,helvetica color=white>cParam3Label<td><font face=arial,helvetica color=white>cParam4Label<td><font face=arial,helvetica color=white>cParam1Tip<td><font face=arial,helvetica color=white>cParam2Tip<td><font face=arial,helvetica color=white>cParam3Tip<td><font face=arial,helvetica color=white>cParam4Tip<td><font face=arial,helvetica color=white>uParam3<td><font face=arial,helvetica color=white>uParam4<td><font face=arial,helvetica color=white>uName<td><font face=arial,helvetica color=white>cNameFunc<td><font face=arial,helvetica color=white>cNameLabel<td><font face=arial,helvetica color=white>cNameTip<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



	mysql_data_seek(res,guStart-1);

	for(guN=0;guN<(guEnd-guStart+1);guN++)
	{
		field=mysql_fetch_row(res);
		if(!field)
		{
			printf("<tr><td><font face=arial,helvetica>End of data</table>");
			Footer_ism3();
		}
			if(guN % 2)
				printf("<tr bgcolor=#BBE1D3>");
			else
				printf("<tr>");
		long unsigned luYesNo2=strtoul(field[2],NULL,10);
		char cBuf2[4];
		if(luYesNo2)
			sprintf(cBuf2,"Yes");
		else
			sprintf(cBuf2,"No");
		long unsigned luYesNo3=strtoul(field[3],NULL,10);
		char cBuf3[4];
		if(luYesNo3)
			sprintf(cBuf3,"Yes");
		else
			sprintf(cBuf3,"No");
		long unsigned luYesNo16=strtoul(field[16],NULL,10);
		char cBuf16[4];
		if(luYesNo16)
			sprintf(cBuf16,"Yes");
		else
			sprintf(cBuf16,"No");
		long unsigned luYesNo17=strtoul(field[17],NULL,10);
		char cBuf17[4];
		if(luYesNo17)
			sprintf(cBuf17,"Yes");
		else
			sprintf(cBuf17,"No");
		long unsigned luYesNo18=strtoul(field[18],NULL,10);
		char cBuf18[4];
		if(luYesNo18)
			sprintf(cBuf18,"Yes");
		else
			sprintf(cBuf18,"No");
		long luTime24=strtoul(field[24],NULL,10);
		char cBuf24[32];
		if(luTime24)
			ctime_r(&luTime24,cBuf24);
		else
			sprintf(cBuf24,"---");
		long luTime26=strtoul(field[26],NULL,10);
		char cBuf26[32];
		if(luTime26)
			ctime_r(&luTime26,cBuf26);
		else
			sprintf(cBuf26,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,cBuf2
			,cBuf3
			,field[4]
			,field[5]
			,field[6]
			,field[7]
			,field[8]
			,field[9]
			,field[10]
			,field[11]
			,field[12]
			,field[13]
			,field[14]
			,field[15]
			,cBuf16
			,cBuf17
			,cBuf18
			,field[19]
			,field[20]
			,field[21]
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[22],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[23],NULL,10))
			,cBuf24
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[25],NULL,10))
			,cBuf26
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tRRTypeList()


void CreatetRRType(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tRRType ( uRRType INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(32) NOT NULL DEFAULT '', UNIQUE (cLabel), uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, uParam1 INT UNSIGNED NOT NULL DEFAULT 0, uParam2 INT UNSIGNED NOT NULL DEFAULT 0, cParam2Func VARCHAR(32) NOT NULL DEFAULT '', cParam1Func VARCHAR(32) NOT NULL DEFAULT '', cParam1Label VARCHAR(32) NOT NULL DEFAULT '', cParam2Label VARCHAR(32) NOT NULL DEFAULT '', cParam1Tip VARCHAR(100) NOT NULL DEFAULT '', cParam2Tip VARCHAR(100) NOT NULL DEFAULT '', cNameLabel VARCHAR(32) NOT NULL DEFAULT '', cNameTip VARCHAR(100) NOT NULL DEFAULT '', cNameFunc VARCHAR(32) NOT NULL DEFAULT '', uName INT UNSIGNED NOT NULL DEFAULT 0, cParam3Func VARCHAR(32) NOT NULL DEFAULT '', cParam4Func VARCHAR(32) NOT NULL DEFAULT '', cParam3Label VARCHAR(32) NOT NULL DEFAULT '', cParam4Label VARCHAR(32) NOT NULL DEFAULT '', cParam3Tip VARCHAR(100) NOT NULL DEFAULT '', cParam4Tip VARCHAR(100) NOT NULL DEFAULT '', uParam3 INT UNSIGNED NOT NULL DEFAULT 0, uParam4 INT UNSIGNED NOT NULL DEFAULT 0 )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetRRType()

