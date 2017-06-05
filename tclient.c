/*
FILE
	tClient source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	User and owner interface and backend client login table.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uClient: Primary Key
static unsigned uClient=0;
//cLabel: Short label
static char cLabel[101]={""};
//cInfo: Unformatted info/address etc.
static char *cInfo={""};
//cEmail: Main Email
static char cEmail[101]={""};
//cCode: Enterprise wide customer/contact tracking
static char cCode[33]={""};
//uOwner: Record owner
static unsigned uOwner=0;
//uCreatedBy: uClient for last insert
static unsigned uCreatedBy=0;
#define ISM3FIELDS
//uCreatedDate: Unix seconds date last insert
static time_t uCreatedDate=0;
//uModBy: uClient for last update
static unsigned uModBy=0;
//uModDate: Unix seconds date last update
static time_t uModDate=0;



#define VAR_LIST_tClient "tClient.uClient,tClient.cLabel,tClient.cInfo,tClient.cEmail,tClient.cCode,tClient.uOwner,tClient.uCreatedBy,tClient.uCreatedDate,tClient.uModBy,tClient.uModDate"

 //Local only
void Insert_tClient(void);
void Update_tClient(char *cRowid);
void ProcesstClientListVars(pentry entries[], int x);

 //In tClientfunc.h file included below
void ExtProcesstClientVars(pentry entries[], int x);
void ExttClientCommands(pentry entries[], int x);
void ExttClientButtons(void);
void ExttClientNavBar(void);
void ExttClientGetHook(entry gentries[], int x);
void ExttClientSelect(void);
void ExttClientSelectRow(void);
void ExttClientListSelect(void);
void ExttClientListFilter(void);
void ExttClientAuxTable(void);

#include "tclientfunc.h"

 //Table Variables Assignment Function
void ProcesstClientVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uClient"))
			sscanf(entries[i].val,"%u",&uClient);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"cInfo"))
			cInfo=entries[i].val;
		else if(!strcmp(entries[i].name,"cEmail"))
			sprintf(cEmail,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"cCode"))
			sprintf(cCode,"%.32s",entries[i].val);
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
	ExtProcesstClientVars(entries,x);

}//ProcesstClientVars()


void ProcesstClientListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uClient);
                        guMode=2002;
                        tClient("");
                }
        }
}//void ProcesstClientListVars(pentry entries[], int x)


int tClientCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttClientCommands(entries,x);

	if(!strcmp(gcFunction,"tClientTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tClientList();
		}

		//Default
		ProcesstClientVars(entries,x);
		tClient("");
	}
	else if(!strcmp(gcFunction,"tClientList"))
	{
		ProcessControlVars(entries,x);
		ProcesstClientListVars(entries,x);
		tClientList();
	}

	return(0);

}//tClientCommands()


void tClient(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttClientSelectRow();
		else
			ExttClientSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetClient();
				iDNS("New tClient table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tClient WHERE uClient=%u"
						,uClient);
				macro_mySQLRunAndStore(res2);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uClient);
		sprintf(cLabel,"%.100s",field[1]);
		cInfo=field[2];
		sprintf(cEmail,"%.100s",field[3]);
		sprintf(cCode,"%.32s",field[4]);
		sscanf(field[5],"%u",&uOwner);
		sscanf(field[6],"%u",&uCreatedBy);
		sscanf(field[7],"%lu",&uCreatedDate);
		sscanf(field[8],"%u",&uModBy);
		sscanf(field[9],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tClient",0);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tClientTools>");
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

        ExttClientButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tClient Record Data",100);

	if(guMode==2000 || guMode==2002)
		tClientInput(1);
	else
		tClientInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttClientAuxTable();

	Footer_ism3();

}//end of tClient();


void tClientInput(unsigned uMode)
{

//uClient
	OpenRow(LANG_FL_tClient_uClient,"black");
	printf("<input title='%s' type=text name=uClient value=%u size=16 maxlength=10 "
,LANG_FT_tClient_uClient,uClient);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uClient value=%u >\n",uClient);
	}
//cLabel
	OpenRow(LANG_FL_tClient_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tClient_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//cInfo
	OpenRow(LANG_FL_tClient_cInfo,"black");
	printf("<textarea title='%s' cols=80 wrap=hard rows=16 name=cInfo "
,LANG_FT_tClient_cInfo);
	if(guPermLevel>=7 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",cInfo);
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",cInfo);
		printf("<input type=hidden name=cInfo value=\"%s\" >\n",EncodeDoubleQuotes(cInfo));
	}
//cEmail
	OpenRow(LANG_FL_tClient_cEmail,"black");
	printf("<input title='%s' type=text name=cEmail value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tClient_cEmail,EncodeDoubleQuotes(cEmail));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cEmail value=\"%s\">\n",EncodeDoubleQuotes(cEmail));
	}
//cCode
	OpenRow(LANG_FL_tClient_cCode,"black");
	printf("<input title='%s' type=text name=cCode value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tClient_cCode,EncodeDoubleQuotes(cCode));
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cCode value=\"%s\">\n",EncodeDoubleQuotes(cCode));
	}
//uOwner
	OpenRow(LANG_FL_tClient_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey("tClient","cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey("tClient","cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tClient_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey("tClient","cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey("tClient","cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tClient_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tClient_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey("tClient","cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey("tClient","cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tClient_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tClientInput(unsigned uMode)


void NewtClient(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uClient FROM tClient\
				WHERE uClient=%u"
							,uClient);
	macro_mySQLRunAndStore(res);
	i=mysql_num_rows(res);

	if(i) 
		//tClient("<blink>Record already exists");
		tClient(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tClient();
	//sprintf(gcQuery,"New record %u added");
	uClient=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tClient",uClient);
	iDNSLog(uClient,"tClient","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uClient);
	tClient(gcQuery);
	}

}//NewtClient(unsigned uMode)


void DeletetClient(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tClient WHERE uClient=%u AND ( uOwner=%u OR %u>9 )"
					,uClient,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tClient WHERE uClient=%u"
					,uClient);
#endif
	macro_mySQLQueryHTMLError;
	//tClient("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uClient,"tClient","Del");
#endif
		tClient(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uClient,"tClient","DelError");
#endif
		tClient(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetClient(void)


void Insert_tClient(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tClient SET uClient=%u,cLabel='%s',cInfo='%s',cEmail='%s',cCode='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uClient
			,TextAreaSave(cLabel)
			,TextAreaSave(cInfo)
			,TextAreaSave(cEmail)
			,TextAreaSave(cCode)
			,uOwner
			,uCreatedBy
			);

	macro_mySQLQueryHTMLError;

}//void Insert_tClient(void)


void Update_tClient(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tClient SET uClient=%u,cLabel='%s',cInfo='%s',cEmail='%s',cCode='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uClient
			,TextAreaSave(cLabel)
			,TextAreaSave(cInfo)
			,TextAreaSave(cEmail)
			,TextAreaSave(cCode)
			,uModBy
			,cRowid);

	macro_mySQLQueryHTMLError;

}//void Update_tClient(void)


void ModtClient(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	//Mod select gcQuery
	if(guPermLevel<10)
	sprintf(gcQuery,"SELECT tClient.uClient,\
				tClient.uModDate\
				FROM tClient,tClient\
				WHERE tClient.uClient=%u\
				AND tClient.uOwner=tClient.uClient\
				AND (tClient.uOwner=%u OR tClient.uClient=%u)"
			,uClient,guLoginClient,guLoginClient);
	else
	sprintf(gcQuery,"SELECT uClient,uModDate FROM tClient\
				WHERE uClient=%u"
						,uClient);
#else
	sprintf(gcQuery,"SELECT uClient FROM tClient\
				WHERE uClient=%u"
						,uClient);
#endif

	macro_mySQLRunAndStore(res);
	i=mysql_num_rows(res);

	//if(i<1) tClient("<blink>Record does not exist");
	if(i<1) tClient(LANG_NBR_RECNOTEXIST);
	//if(i>1) tClient("<blink>Multiple rows!");
	if(i>1) tClient(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tClient(LANG_NBR_EXTMOD);
#endif

	Update_tClient(field[0]);
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tClient",uClient);
	iDNSLog(uClient,"tClient","Mod");
#endif
	tClient(gcQuery);

}//ModtClient(void)


void tClientList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttClientListSelect();
	macro_mySQLRunAndStore(res);
	guI=mysql_num_rows(res);

	PageMachine("tClientList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttClientListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uClient<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>cInfo<td><font face=arial,helvetica color=white>cEmail<td><font face=arial,helvetica color=white>cCode<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
                time_t luTime7=strtoul(field[7],NULL,10);
                char cBuf7[32];
                if(luTime7)
                        ctime_r(&luTime7,cBuf7);
                else
                        sprintf(cBuf7,"---");
                time_t luTime9=strtoul(field[9],NULL,10);
                char cBuf9[32];
                if(luTime9)
                        ctime_r(&luTime9,cBuf9);
                else
                        sprintf(cBuf9,"---");
		printf("<td><a class=darkLink href=?gcFunction=tClient&uClient=%s>"
                	" %s<td>%s<td><textarea disabled>%s</textarea><td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
                        ,field[0]
                        ,field[0]
                        ,field[1]
                        ,field[2]
                        ,field[3]
                        ,field[4]
                        ,ForeignKey(TCLIENT,"cLabel",strtoul(field[5],NULL,10))
                        ,ForeignKey(TCLIENT,"cLabel",strtoul(field[6],NULL,10))
                        ,cBuf7
                        ,ForeignKey(TCLIENT,"cLabel",strtoul(field[8],NULL,10))
                        ,cBuf9
                                );


	}

	printf("</table></form>\n");
	Footer_ism3();

}//tClientList()


void CreatetClient(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tClient ( uClient INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(100) NOT NULL DEFAULT '',unique (cLabel,uOwner), uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, cInfo TEXT NOT NULL DEFAULT '', cEmail VARCHAR(100) NOT NULL DEFAULT '', cCode VARCHAR(32) NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetClient()

