/*
FILE
	tLog source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Audit log table.
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
//cMessage: Message text
static char cMessage[256]={""};
//cServer: Server name
static char cServer[65]={""};
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



#define VAR_LIST_tLog "tLog.uLog,tLog.cLabel,tLog.uLogType,tLog.cHash,tLog.uPermLevel,tLog.uLoginClient,tLog.cLogin,tLog.cHost,tLog.uTablePK,tLog.cTableName,tLog.cMessage,tLog.cServer,tLog.uOwner,tLog.uCreatedBy,tLog.uCreatedDate,tLog.uModBy,tLog.uModDate"

 //Local only
void Insert_tLog(void);
void Update_tLog(char *cRowid);
void ProcesstLogListVars(pentry entries[], int x);

 //In tLogfunc.h file included below
void ExtProcesstLogVars(pentry entries[], int x);
void ExttLogCommands(pentry entries[], int x);
void ExttLogButtons(void);
void ExttLogNavBar(void);
void ExttLogGetHook(entry gentries[], int x);
void ExttLogSelect(void);
void ExttLogSelectRow(void);
void ExttLogListSelect(void);
void ExttLogListFilter(void);
void ExttLogAuxTable(void);

#include "tlogfunc.h"

 //Table Variables Assignment Function
void ProcesstLogVars(pentry entries[], int x)
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
		else if(!strcmp(entries[i].name,"cMessage"))
			sprintf(cMessage,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"cServer"))
			sprintf(cServer,"%.64s",entries[i].val);
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
	ExtProcesstLogVars(entries,x);

}//ProcesstLogVars()


void ProcesstLogListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uLog);
                        guMode=2002;
                        tLog("");
                }
        }
}//void ProcesstLogListVars(pentry entries[], int x)


int tLogCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttLogCommands(entries,x);

	if(!strcmp(gcFunction,"tLogTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tLogList();
		}

		//Default
		ProcesstLogVars(entries,x);
		tLog("");
	}
	else if(!strcmp(gcFunction,"tLogList"))
	{
		ProcessControlVars(entries,x);
		ProcesstLogListVars(entries,x);
		tLogList();
	}

	return(0);

}//tLogCommands()


void tLog(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttLogSelectRow();
		else
			ExttLogSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetLog();
				iDNS("New tLog table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tLog WHERE uLog=%u"
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
		sprintf(cMessage,"%.255s",field[10]);
		sprintf(cServer,"%.64s",field[11]);
		sscanf(field[12],"%u",&uOwner);
		sscanf(field[13],"%u",&uCreatedBy);
		sscanf(field[14],"%lu",&uCreatedDate);
		sscanf(field[15],"%u",&uModBy);
		sscanf(field[16],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tLog",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tLogTools>");
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

        ExttLogButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tLog Record Data",100);

	if(guMode==2000 || guMode==2002)
		tLogInput(1);
	else
		tLogInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttLogAuxTable();

	Footer_ism3();

}//end of tLog();


void tLogInput(unsigned uMode)
{

//uLog
	OpenRow(LANG_FL_tLog_uLog,"black");
	printf("<input title='%s' type=text name=uLog value=%u size=16 maxlength=10 "
,LANG_FT_tLog_uLog,uLog);
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
	OpenRow(LANG_FL_tLog_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=64 maxlength=64 "
,LANG_FT_tLog_cLabel,EncodeDoubleQuotes(cLabel));
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
	OpenRow(LANG_FL_tLog_uLogType,"black");
	if(guPermLevel>=7 && uMode)
		tTablePullDown("tLogType;cuLogTypePullDown","cLabel","cLabel",uLogType,1);
	else
		tTablePullDown("tLogType;cuLogTypePullDown","cLabel","cLabel",uLogType,0);
//cHash
	OpenRow(LANG_FL_tLog_cHash,"black");
	printf("<input title='%s' type=text name=cHash value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tLog_cHash,EncodeDoubleQuotes(cHash));
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
	OpenRow(LANG_FL_tLog_uPermLevel,"black");
	printf("<input title='%s' type=text name=uPermLevel value=%u size=16 maxlength=32 "
,LANG_FT_tLog_uPermLevel,uPermLevel);
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
	OpenRow(LANG_FL_tLog_uLoginClient,"black");
	printf("<input title='%s' type=text name=uLoginClient value=%u size=16 maxlength=32 "
,LANG_FT_tLog_uLoginClient,uLoginClient);
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
	OpenRow(LANG_FL_tLog_cLogin,"black");
	printf("<input title='%s' type=text name=cLogin value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tLog_cLogin,EncodeDoubleQuotes(cLogin));
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
	OpenRow(LANG_FL_tLog_cHost,"black");
	printf("<input title='%s' type=text name=cHost value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tLog_cHost,EncodeDoubleQuotes(cHost));
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
	OpenRow(LANG_FL_tLog_uTablePK,"black");
	printf("<input title='%s' type=text name=uTablePK value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tLog_uTablePK,EncodeDoubleQuotes(uTablePK));
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
	OpenRow(LANG_FL_tLog_cTableName,"black");
	printf("<input title='%s' type=text name=cTableName value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tLog_cTableName,EncodeDoubleQuotes(cTableName));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cTableName value=\"%s\">\n",EncodeDoubleQuotes(cTableName));
	}
//cMessage
	OpenRow(LANG_FL_tLog_cMessage,"black");
	printf("<input title='%s' type=text name=cMessage value=\"%s\" size=40 maxlength=255 "
,LANG_FT_tLog_cMessage,EncodeDoubleQuotes(cMessage));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cMessage value=\"%s\">\n",EncodeDoubleQuotes(cMessage));
	}
//cServer
	OpenRow(LANG_FL_tLog_cServer,"black");
	printf("<input title='%s' type=text name=cServer value=\"%s\" size=40 maxlength=64 "
,LANG_FT_tLog_cServer,EncodeDoubleQuotes(cServer));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cServer value=\"%s\">\n",EncodeDoubleQuotes(cServer));
	}
//uOwner
	OpenRow(LANG_FL_tLog_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tLog_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tLog_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tLog_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tLog_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tLogInput(unsigned uMode)


void NewtLog(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uLog FROM tLog\
				WHERE uLog=%u"
							,uLog);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tLog("<blink>Record already exists");
		tLog(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tLog();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uLog=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tLog",uLog);
	iDNSLog(uLog,"tLog","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uLog);
	tLog(gcQuery);
	}

}//NewtLog(unsigned uMode)


void DeletetLog(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tLog WHERE uLog=%u AND ( uOwner=%u OR %u>9 )"
					,uLog,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tLog WHERE uLog=%u"
					,uLog);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tLog("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uLog,"tLog","Del");
#endif
		tLog(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uLog,"tLog","DelError");
#endif
		tLog(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetLog(void)


void Insert_tLog(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tLog SET uLog=%u,cLabel='%s',uLogType=%u,cHash='%s',uPermLevel=%u,uLoginClient=%u,cLogin='%s',cHost='%s',uTablePK='%s',cTableName='%s',cMessage='%s',cServer='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uLog
			,TextAreaSave(cLabel)
			,uLogType
			,TextAreaSave(cHash)
			,uPermLevel
			,uLoginClient
			,TextAreaSave(cLogin)
			,TextAreaSave(cHost)
			,TextAreaSave(uTablePK)
			,TextAreaSave(cTableName)
			,TextAreaSave(cMessage)
			,TextAreaSave(cServer)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tLog(void)


void Update_tLog(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tLog SET uLog=%u,cLabel='%s',uLogType=%u,cHash='%s',uPermLevel=%u,uLoginClient=%u,cLogin='%s',cHost='%s',uTablePK='%s',cTableName='%s',cMessage='%s',cServer='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uLog
			,TextAreaSave(cLabel)
			,uLogType
			,TextAreaSave(cHash)
			,uPermLevel
			,uLoginClient
			,TextAreaSave(cLogin)
			,TextAreaSave(cHost)
			,TextAreaSave(uTablePK)
			,TextAreaSave(cTableName)
			,TextAreaSave(cMessage)
			,TextAreaSave(cServer)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tLog(void)


void ModtLog(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uLog,uModDate FROM tLog WHERE uLog=%u"
			,uLog);
#else
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE uLog=%u"
			,uLog);
#endif

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tLog("<blink>Record does not exist");
	if(i<1) tLog(LANG_NBR_RECNOTEXIST);
	//if(i>1) tLog("<blink>Multiple rows!");
	if(i>1) tLog(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tLog(LANG_NBR_EXTMOD);
#endif

	Update_tLog(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tLog",uLog);
	iDNSLog(uLog,"tLog","Mod");
#endif
	tLog(gcQuery);

}//ModtLog(void)


void tLogList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttLogListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tLogList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttLogListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uLog<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>uLogType<td><font face=arial,helvetica color=white>cHash<td><font face=arial,helvetica color=white>uPermLevel<td><font face=arial,helvetica color=white>uLoginClient<td><font face=arial,helvetica color=white>cLogin<td><font face=arial,helvetica color=white>cHost<td><font face=arial,helvetica color=white>uTablePK<td><font face=arial,helvetica color=white>cTableName<td><font face=arial,helvetica color=white>cMessage<td><font face=arial,helvetica color=white>cServer<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		long luTime14=strtoul(field[14],NULL,10);
		char cBuf14[32];
		if(luTime14)
			ctime_r(&luTime14,cBuf14);
		else
			sprintf(cBuf14,"---");
		long luTime16=strtoul(field[16],NULL,10);
		char cBuf16[32];
		if(luTime16)
			ctime_r(&luTime16,cBuf16);
		else
			sprintf(cBuf16,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
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
			,field[10]
			,field[11]
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[12],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[13],NULL,10))
			,cBuf14
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[15],NULL,10))
			,cBuf16
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tLogList()


void CreatetLog(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tLog ( uTablePK VARCHAR(32) NOT NULL DEFAULT '', cHost VARCHAR(32) NOT NULL DEFAULT '', uLoginClient INT UNSIGNED NOT NULL DEFAULT 0, cLogin VARCHAR(32) NOT NULL DEFAULT '', uPermLevel INT UNSIGNED NOT NULL DEFAULT 0, cTableName VARCHAR(32) NOT NULL DEFAULT '', uLog INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(64) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, cHash VARCHAR(32) NOT NULL DEFAULT '', uLogType INT UNSIGNED NOT NULL DEFAULT 0,index (uLogType), cMessage VARCHAR(255) NOT NULL DEFAULT '', cServer VARCHAR(64) NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetLog()

