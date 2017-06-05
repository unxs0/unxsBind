/*
FILE
	tLogMonth source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Monthly log names for archiving.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uLog: Primary Key
static unsigned uLog=0;
//cLabel: Short label
static char cLabel[65]={""};
//uLogType: Log Type
static unsigned uLogType=0;
static char cuLogTypePullDown[256]={""};
//cHash: Security hash to complicate tampering
static char cHash[33]={""};
//uPermLevel: User Perm Level
static unsigned uPermLevel=0;
//uLoginClient: Client Number
static unsigned uLoginClient=0;
//cLogin: Login name
static char cLogin[33]={""};
//cHost: Ip Address
static char cHost[33]={""};
//uTablePK: Primar Key of the Table
static char uTablePK[33]={""};
//cTableName: Name of the Table
static char cTableName[33]={""};
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



#define VAR_LIST_tLogMonth "tLogMonth.uLog,tLogMonth.cLabel,tLogMonth.uLogType,tLogMonth.cHash,tLogMonth.uPermLevel,tLogMonth.uLoginClient,tLogMonth.cLogin,tLogMonth.cHost,tLogMonth.uTablePK,tLogMonth.cTableName,tLogMonth.uOwner,tLogMonth.uCreatedBy,tLogMonth.uCreatedDate,tLogMonth.uModBy,tLogMonth.uModDate"

 //Local only
void Insert_tLogMonth(void);
void Update_tLogMonth(char *cRowid);
void ProcesstLogMonthListVars(pentry entries[], int x);

 //In tLogMonthfunc.h file included below
void ExtProcesstLogMonthVars(pentry entries[], int x);
void ExttLogMonthCommands(pentry entries[], int x);
void ExttLogMonthButtons(void);
void ExttLogMonthNavBar(void);
void ExttLogMonthGetHook(entry gentries[], int x);
void ExttLogMonthSelect(void);
void ExttLogMonthSelectRow(void);
void ExttLogMonthListSelect(void);
void ExttLogMonthListFilter(void);
void ExttLogMonthAuxTable(void);

#include "tlogmonthfunc.h"

 //Table Variables Assignment Function
void ProcesstLogMonthVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uLog"))
			sscanf(entries[i].val,"%u",&uLog);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.64s",entries[i].val);
		else if(!strcmp(entries[i].name,"uLogType"))
			sscanf(entries[i].val,"%u",&uLogType);
		else if(!strcmp(entries[i].name,"cuLogTypePullDown"))
		{
			sprintf(cuLogTypePullDown,"%.255s",entries[i].val);
			uLogType=ReadPullDown("tLogType","cLabel",cuLogTypePullDown);
		}
		else if(!strcmp(entries[i].name,"cHash"))
			sprintf(cHash,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"uPermLevel"))
			sscanf(entries[i].val,"%u",&uPermLevel);
		else if(!strcmp(entries[i].name,"uLoginClient"))
			sscanf(entries[i].val,"%u",&uLoginClient);
		else if(!strcmp(entries[i].name,"cLogin"))
			sprintf(cLogin,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cHost"))
			sprintf(cHost,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"uTablePK"))
			sprintf(uTablePK,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cTableName"))
			sprintf(cTableName,"%.32s",entries[i].val);
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
	ExtProcesstLogMonthVars(entries,x);

}//ProcesstLogMonthVars()


void ProcesstLogMonthListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uLog);
                        guMode=2002;
                        tLogMonth("");
                }
        }
}//void ProcesstLogMonthListVars(pentry entries[], int x)


int tLogMonthCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttLogMonthCommands(entries,x);

	if(!strcmp(gcFunction,"tLogMonthTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tLogMonthList();
		}

		//Default
		ProcesstLogMonthVars(entries,x);
		tLogMonth("");
	}
	else if(!strcmp(gcFunction,"tLogMonthList"))
	{
		ProcessControlVars(entries,x);
		ProcesstLogMonthListVars(entries,x);
		tLogMonthList();
	}

	return(0);

}//tLogMonthCommands()


void tLogMonth(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttLogMonthSelectRow();
		else
			ExttLogMonthSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetLogMonth();
				iDNS("New tLogMonth table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tLogMonth WHERE uLog=%u"
						,uLog);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uLog);
		sprintf(cLabel,"%.64s",field[1]);
		sscanf(field[2],"%u",&uLogType);
		sprintf(cHash,"%.32s",field[3]);
		sscanf(field[4],"%u",&uPermLevel);
		sscanf(field[5],"%u",&uLoginClient);
		sprintf(cLogin,"%.32s",field[6]);
		sprintf(cHost,"%.32s",field[7]);
		sprintf(uTablePK,"%.32s",field[8]);
		sprintf(cTableName,"%.32s",field[9]);
		sscanf(field[10],"%u",&uOwner);
		sscanf(field[11],"%u",&uCreatedBy);
		sscanf(field[12],"%lu",&uCreatedDate);
		sscanf(field[13],"%u",&uModBy);
		sscanf(field[14],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tLogMonth",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tLogMonthTools>");
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

        ExttLogMonthButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tLogMonth Record Data",100);

	if(guMode==2000 || guMode==2002)
		tLogMonthInput(1);
	else
		tLogMonthInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttLogMonthAuxTable();

	Footer_ism3();

}//end of tLogMonth();


void tLogMonthInput(unsigned uMode)
{

//uLog
	OpenRow(LANG_FL_tLogMonth_uLog,"black");
	printf("<input title='%s' type=text name=uLog value=%u size=16 maxlength=10 "
,LANG_FT_tLogMonth_uLog,uLog);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uLog value=%u >\n",uLog);
	}
//cLabel
	OpenRow(LANG_FL_tLogMonth_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=64 maxlength=64 "
,LANG_FT_tLogMonth_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//uLogType
	OpenRow(LANG_FL_tLogMonth_uLogType,"black");
	if(guPermLevel>=7 && uMode)
		tTablePullDown("tLogType;cuLogTypePullDown","cLabel","cLabel",uLogType,1);
	else
		tTablePullDown("tLogType;cuLogTypePullDown","cLabel","cLabel",uLogType,0);
//cHash
	OpenRow(LANG_FL_tLogMonth_cHash,"black");
	printf("<input title='%s' type=text name=cHash value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tLogMonth_cHash,EncodeDoubleQuotes(cHash));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cHash value=\"%s\">\n",EncodeDoubleQuotes(cHash));
	}
//uPermLevel
	OpenRow(LANG_FL_tLogMonth_uPermLevel,"black");
	printf("<input title='%s' type=text name=uPermLevel value=%u size=16 maxlength=32 "
,LANG_FT_tLogMonth_uPermLevel,uPermLevel);
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uPermLevel value=%u >\n",uPermLevel);
	}
//uLoginClient
	OpenRow(LANG_FL_tLogMonth_uLoginClient,"black");
	printf("<input title='%s' type=text name=uLoginClient value=%u size=16 maxlength=32 "
,LANG_FT_tLogMonth_uLoginClient,uLoginClient);
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uLoginClient value=%u >\n",uLoginClient);
	}
//cLogin
	OpenRow(LANG_FL_tLogMonth_cLogin,"black");
	printf("<input title='%s' type=text name=cLogin value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tLogMonth_cLogin,EncodeDoubleQuotes(cLogin));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLogin value=\"%s\">\n",EncodeDoubleQuotes(cLogin));
	}
//cHost
	OpenRow(LANG_FL_tLogMonth_cHost,"black");
	printf("<input title='%s' type=text name=cHost value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tLogMonth_cHost,EncodeDoubleQuotes(cHost));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cHost value=\"%s\">\n",EncodeDoubleQuotes(cHost));
	}
//uTablePK
	OpenRow(LANG_FL_tLogMonth_uTablePK,"black");
	printf("<input title='%s' type=text name=uTablePK value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tLogMonth_uTablePK,EncodeDoubleQuotes(uTablePK));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uTablePK value=\"%s\">\n",EncodeDoubleQuotes(uTablePK));
	}
//cTableName
	OpenRow(LANG_FL_tLogMonth_cTableName,"black");
	printf("<input title='%s' type=text name=cTableName value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tLogMonth_cTableName,EncodeDoubleQuotes(cTableName));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cTableName value=\"%s\">\n",EncodeDoubleQuotes(cTableName));
	}
//uOwner
	OpenRow(LANG_FL_tLogMonth_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tLogMonth_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tLogMonth_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tLogMonth_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tLogMonth_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tLogMonthInput(unsigned uMode)


void tLogMonthList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttLogMonthListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tLogMonthList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttLogMonthListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uLog<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>uLogType<td><font face=arial,helvetica color=white>cHash<td><font face=arial,helvetica color=white>uPermLevel<td><font face=arial,helvetica color=white>uLoginClient<td><font face=arial,helvetica color=white>cLogin<td><font face=arial,helvetica color=white>cHost<td><font face=arial,helvetica color=white>uTablePK<td><font face=arial,helvetica color=white>cTableName<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		long luTime12=strtoul(field[12],NULL,10);
		char cBuf12[32];
		if(luTime12)
			ctime_r(&luTime12,cBuf12);
		else
			sprintf(cBuf12,"---");
		long luTime14=strtoul(field[14],NULL,10);
		char cBuf14[32];
		if(luTime14)
			ctime_r(&luTime14,cBuf14);
		else
			sprintf(cBuf14,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,ForeignKey("tLogType","cLabel",strtoul(field[2],NULL,10))
			,field[3]
			,field[4]
			,field[5]
			,field[6]
			,field[7]
			,field[8]
			,field[9]
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[10],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[11],NULL,10))
			,cBuf12
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[13],NULL,10))
			,cBuf14
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tLogMonthList()


void CreatetLogMonth(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tLogMonth ( uTablePK VARCHAR(32) NOT NULL DEFAULT '', cHost VARCHAR(32) NOT NULL DEFAULT '', uLoginClient INT UNSIGNED NOT NULL DEFAULT 0, cLogin VARCHAR(32) NOT NULL DEFAULT '', uPermLevel INT UNSIGNED NOT NULL DEFAULT 0, cTableName VARCHAR(32) NOT NULL DEFAULT '', uLog INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(64) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, cHash VARCHAR(32) NOT NULL DEFAULT '', uLogType INT UNSIGNED NOT NULL DEFAULT 0,index (uLogType) )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetLogMonth()

