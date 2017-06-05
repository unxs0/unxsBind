/*
FILE
	tMailServer source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	tZone mail server FK table. Being deprecated as very weird.
	Was used to save DB space back in the day.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uMailServer: Primary Key
static unsigned uMailServer=0;
//cLabel: Group of mail servers label
static char cLabel[33]={""};
//cList: List of mail servers
static char *cList={""};
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



#define VAR_LIST_tMailServer "tMailServer.uMailServer,tMailServer.cLabel,tMailServer.cList,tMailServer.uOwner,tMailServer.uCreatedBy,tMailServer.uCreatedDate,tMailServer.uModBy,tMailServer.uModDate"

 //Local only
void Insert_tMailServer(void);
void Update_tMailServer(char *cRowid);
void ProcesstMailServerListVars(pentry entries[], int x);

 //In tMailServerfunc.h file included below
void ExtProcesstMailServerVars(pentry entries[], int x);
void ExttMailServerCommands(pentry entries[], int x);
void ExttMailServerButtons(void);
void ExttMailServerNavBar(void);
void ExttMailServerGetHook(entry gentries[], int x);
void ExttMailServerSelect(void);
void ExttMailServerSelectRow(void);
void ExttMailServerListSelect(void);
void ExttMailServerListFilter(void);
void ExttMailServerAuxTable(void);

#include "tmailserverfunc.h"

 //Table Variables Assignment Function
void ProcesstMailServerVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uMailServer"))
			sscanf(entries[i].val,"%u",&uMailServer);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cList"))
			cList=entries[i].val;
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
	ExtProcesstMailServerVars(entries,x);

}//ProcesstMailServerVars()


void ProcesstMailServerListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uMailServer);
                        guMode=2002;
                        tMailServer("");
                }
        }
}//void ProcesstMailServerListVars(pentry entries[], int x)


int tMailServerCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttMailServerCommands(entries,x);

	if(!strcmp(gcFunction,"tMailServerTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tMailServerList();
		}

		//Default
		ProcesstMailServerVars(entries,x);
		tMailServer("");
	}
	else if(!strcmp(gcFunction,"tMailServerList"))
	{
		ProcessControlVars(entries,x);
		ProcesstMailServerListVars(entries,x);
		tMailServerList();
	}

	return(0);

}//tMailServerCommands()


void tMailServer(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttMailServerSelectRow();
		else
			ExttMailServerSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetMailServer();
				iDNS("New tMailServer table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tMailServer WHERE uMailServer=%u"
						,uMailServer);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uMailServer);
		sprintf(cLabel,"%.32s",field[1]);
		cList=field[2];
		sscanf(field[3],"%u",&uOwner);
		sscanf(field[4],"%u",&uCreatedBy);
		sscanf(field[5],"%lu",&uCreatedDate);
		sscanf(field[6],"%u",&uModBy);
		sscanf(field[7],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tMailServer",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tMailServerTools>");
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

        ExttMailServerButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tMailServer Record Data",100);

	if(guMode==2000 || guMode==2002)
		tMailServerInput(1);
	else
		tMailServerInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttMailServerAuxTable();

	Footer_ism3();

}//end of tMailServer();


void tMailServerInput(unsigned uMode)
{

//uMailServer
	OpenRow(LANG_FL_tMailServer_uMailServer,"black");
	printf("<input title='%s' type=text name=uMailServer value=%u size=16 maxlength=10 "
,LANG_FT_tMailServer_uMailServer,uMailServer);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uMailServer value=%u >\n",uMailServer);
	}
//cLabel
	OpenRow(LANG_FL_tMailServer_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tMailServer_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//cList
	OpenRow(LANG_FL_tMailServer_cList,"black");
	printf("<textarea title='%s' cols=60 wrap=hard rows=5 name=cList "
,LANG_FT_tMailServer_cList);
	if(guPermLevel>=0 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",cList);
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",cList);
		printf("<input type=hidden name=cList value=\"%s\" >\n",EncodeDoubleQuotes(cList));
	}
//uOwner
	OpenRow(LANG_FL_tMailServer_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tMailServer_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tMailServer_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tMailServer_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tMailServer_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tMailServerInput(unsigned uMode)


void NewtMailServer(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uMailServer FROM tMailServer\
				WHERE uMailServer=%u"
							,uMailServer);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tMailServer("<blink>Record already exists");
		tMailServer(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tMailServer();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uMailServer=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tMailServer",uMailServer);
	iDNSLog(uMailServer,"tMailServer","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uMailServer);
	tMailServer(gcQuery);
	}

}//NewtMailServer(unsigned uMode)


void DeletetMailServer(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tMailServer WHERE uMailServer=%u AND ( uOwner=%u OR %u>9 )"
					,uMailServer,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tMailServer WHERE uMailServer=%u"
					,uMailServer);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tMailServer("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uMailServer,"tMailServer","Del");
#endif
		tMailServer(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uMailServer,"tMailServer","DelError");
#endif
		tMailServer(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetMailServer(void)


void Insert_tMailServer(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tMailServer SET uMailServer=%u,cLabel='%s',cList='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uMailServer
			,TextAreaSave(cLabel)
			,TextAreaSave(cList)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tMailServer(void)


void Update_tMailServer(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tMailServer SET uMailServer=%u,cLabel='%s',cList='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uMailServer
			,TextAreaSave(cLabel)
			,TextAreaSave(cList)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tMailServer(void)


void ModtMailServer(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uMailServer,uModDate FROM tMailServer WHERE uMailServer=%u"
			,uMailServer);
#else
	sprintf(gcQuery,"SELECT uMailServer FROM tMailServer WHERE uMailServer=%u"
			,uMailServer);
#endif

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tMailServer("<blink>Record does not exist");
	if(i<1) tMailServer(LANG_NBR_RECNOTEXIST);
	//if(i>1) tMailServer("<blink>Multiple rows!");
	if(i>1) tMailServer(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tMailServer(LANG_NBR_EXTMOD);
#endif

	Update_tMailServer(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tMailServer",uMailServer);
	iDNSLog(uMailServer,"tMailServer","Mod");
#endif
	tMailServer(gcQuery);

}//ModtMailServer(void)


void tMailServerList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttMailServerListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tMailServerList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttMailServerListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uMailServer<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>cList<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		long luTime5=strtoul(field[5],NULL,10);
		char cBuf5[32];
		if(luTime5)
			ctime_r(&luTime5,cBuf5);
		else
			sprintf(cBuf5,"---");
		long luTime7=strtoul(field[7],NULL,10);
		char cBuf7[32];
		if(luTime7)
			ctime_r(&luTime7,cBuf7);
		else
			sprintf(cBuf7,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td><textarea disabled>%s</textarea><td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,field[2]
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[3],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[4],NULL,10))
			,cBuf5
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[6],NULL,10))
			,cBuf7
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tMailServerList()


void CreatetMailServer(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tMailServer ( uMailServer INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cList TEXT NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, cLabel VARCHAR(32) NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetMailServer()

