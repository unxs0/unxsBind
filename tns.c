/*
FILE
	tNS source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis 2001-2007
PURPOSE
	Name server label table.
	Used with tServer, tNSSet and tNSStype.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uNS: Primary Key
static unsigned uNS=0;
//cFQDN: FQDN of a NS
static char cFQDN[101]={""};
//uNSType: Key into tNSType
static unsigned uNSType=0;
static char cuNSTypePullDown[256]={""};
//uNSSet: Key into tNSSet
static unsigned uNSSet=0;
static char cuNSSetPullDown[256]={""};
//uServer: Key into tServer
static unsigned uServer=0;
static char cuServerPullDown[256]={""};
//uOwner: Record owner
static unsigned uOwner=0;
//uCreatedBy: uClient for last insert
static unsigned uCreatedBy=0;
//uCreatedDate: Unix seconds date last insert
static time_t uCreatedDate=0;
//uModBy: uClient for last update
static unsigned uModBy=0;
//uModDate: Unix seconds date last update
static time_t uModDate=0;



#define VAR_LIST_tNS "tNS.uNS,tNS.cFQDN,tNS.uNSType,tNS.uNSSet,tNS.uServer,tNS.uOwner,tNS.uCreatedBy,tNS.uCreatedDate,tNS.uModBy,tNS.uModDate"

 //Local only
void Insert_tNS(void);
void Update_tNS(char *cRowid);
void ProcesstNSListVars(pentry entries[], int x);

 //In tNSfunc.h file included below
void ExtProcesstNSVars(pentry entries[], int x);
void ExttNSCommands(pentry entries[], int x);
void ExttNSButtons(void);
void ExttNSNavBar(void);
void ExttNSGetHook(entry gentries[], int x);
void ExttNSSelect(void);
void ExttNSSelectRow(void);
void ExttNSListSelect(void);
void ExttNSListFilter(void);
void ExttNSAuxTable(void);

#include "tnsfunc.h"

 //Table Variables Assignment Function
void ProcesstNSVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uNS"))
			sscanf(entries[i].val,"%u",&uNS);
		else if(!strcmp(entries[i].name,"cFQDN"))
			sprintf(cFQDN,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"uNSType"))
			sscanf(entries[i].val,"%u",&uNSType);
		else if(!strcmp(entries[i].name,"cuNSTypePullDown"))
		{
			sprintf(cuNSTypePullDown,"%.255s",entries[i].val);
			uNSType=ReadPullDown("tNSType","cLabel",cuNSTypePullDown);
		}
		else if(!strcmp(entries[i].name,"uNSSet"))
			sscanf(entries[i].val,"%u",&uNSSet);
		else if(!strcmp(entries[i].name,"cuNSSetPullDown"))
		{
			sprintf(cuNSSetPullDown,"%.255s",entries[i].val);
			uNSSet=ReadPullDown("tNSSet","cLabel",cuNSSetPullDown);
		}
		else if(!strcmp(entries[i].name,"uServer"))
			sscanf(entries[i].val,"%u",&uServer);
		else if(!strcmp(entries[i].name,"cuServerPullDown"))
		{
			sprintf(cuServerPullDown,"%.255s",entries[i].val);
			uServer=ReadPullDown("tServer","cLabel",cuServerPullDown);
		}
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
	ExtProcesstNSVars(entries,x);

}//ProcesstNSVars()


void ProcesstNSListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uNS);
                        guMode=2002;
                        tNS("");
                }
        }
}//void ProcesstNSListVars(pentry entries[], int x)


int tNSCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttNSCommands(entries,x);

	if(!strcmp(gcFunction,"tNSTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tNSList();
		}

		//Default
		ProcesstNSVars(entries,x);
		tNS("");
	}
	else if(!strcmp(gcFunction,"tNSList"))
	{
		ProcessControlVars(entries,x);
		ProcesstNSListVars(entries,x);
		tNSList();
	}

	return(0);

}//tNSCommands()


void tNS(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttNSSelectRow();
		else
			ExttNSSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetNS();
				iDNS("New tNS table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tNS WHERE uNS=%u"
						,uNS);
				macro_mySQLRunAndStore(res2);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uNS);
		sprintf(cFQDN,"%.100s",field[1]);
		sscanf(field[2],"%u",&uNSType);
		sscanf(field[3],"%u",&uNSSet);
		sscanf(field[4],"%u",&uServer);
		sscanf(field[5],"%u",&uOwner);
		sscanf(field[6],"%u",&uCreatedBy);
		sscanf(field[7],"%lu",&uCreatedDate);
		sscanf(field[8],"%u",&uModBy);
		sscanf(field[9],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tNS",0);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tNSTools>");
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

        ExttNSButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tNS Record Data",100);

	if(guMode==2000 || guMode==2002)
		tNSInput(1);
	else
		tNSInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttNSAuxTable();

	Footer_ism3();

}//end of tNS();


void tNSInput(unsigned uMode)
{

//uNS
	OpenRow(LANG_FL_tNS_uNS,"black");
	printf("<input title='%s' type=text name=uNS value=%u size=16 maxlength=10 "
,LANG_FT_tNS_uNS,uNS);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uNS value=%u >\n",uNS);
	}
//cFQDN
	OpenRow(LANG_FL_tNS_cFQDN,"black");
	printf("<input title='%s' type=text name=cFQDN value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tNS_cFQDN,EncodeDoubleQuotes(cFQDN));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cFQDN value=\"%s\">\n",EncodeDoubleQuotes(cFQDN));
	}
//uNSType
	OpenRow(LANG_FL_tNS_uNSType,"black");
	if(guPermLevel>=10 && uMode)
		tTablePullDown("tNSType;cuNSTypePullDown","cLabel","cLabel",uNSType,1);
	else
		tTablePullDown("tNSType;cuNSTypePullDown","cLabel","cLabel",uNSType,0);
//uNSSet
	OpenRow(LANG_FL_tNS_uNSSet,"black");
	if(guPermLevel>=10 && uMode)
		tTablePullDown("tNSSet;cuNSSetPullDown","cLabel","cLabel",uNSSet,1);
	else
		tTablePullDown("tNSSet;cuNSSetPullDown","cLabel","cLabel",uNSSet,0);
//uServer
	OpenRow(LANG_FL_tNS_uServer,"black");
	if(guPermLevel>=10 && uMode)
		tTablePullDown("tServer;cuServerPullDown","cLabel","cLabel",uServer,1);
	else
		tTablePullDown("tServer;cuServerPullDown","cLabel","cLabel",uServer,0);
//uOwner
	OpenRow(LANG_FL_tNS_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tNS_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tNS_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tNS_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tNS_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tNSInput(unsigned uMode)


void NewtNS(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uNS FROM tNS WHERE uNS=%u",uNS);
	macro_mySQLRunAndStore(res);
	i=mysql_num_rows(res);

	if(i) 
		//tNS("<blink>Record already exists");
		tNS(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tNS();
	//sprintf(gcQuery,"New record %u added");
	uNS=mysql_insert_id(&gMysql);
	uCreatedDate=luGetCreatedDate("tNS",uNS);
	iDNSLog(uNS,"tNS","New");

	if(!uMode)
	{
		sprintf(gcQuery,LANG_NBR_NEWRECADDED,uNS);
		tNS(gcQuery);
	}

}//NewtNS(unsigned uMode)


void DeletetNS(void)
{
	sprintf(gcQuery,"DELETE FROM tNS WHERE uNS=%u AND ( uOwner=%u OR %u>9 )"
					,uNS,guLoginClient,guPermLevel);
	macro_mySQLQueryHTMLError;
	//tNS("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
		iDNSLog(uNS,"tNS","Del");
		tNS(LANG_NBR_RECDELETED);
	}
	else
	{
		iDNSLog(uNS,"tNS","DelError");
		tNS(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetNS(void)


void Insert_tNS(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tNS SET uNS=%u,cFQDN='%s',uNSType=%u,uNSSet=%u,uServer=%u,uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uNS
			,TextAreaSave(cFQDN)
			,uNSType
			,uNSSet
			,uServer
			,uOwner
			,uCreatedBy
			);

	macro_mySQLQueryHTMLError;

}//void Insert_tNS(void)


void Update_tNS(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tNS SET uNS=%u,cFQDN='%s',uNSType=%u,uNSSet=%u,uServer=%u,uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uNS
			,TextAreaSave(cFQDN)
			,uNSType
			,uNSSet
			,uServer
			,uModBy
			,cRowid);

	macro_mySQLQueryHTMLError;

}//void Update_tNS(void)


void ModtNS(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uPreModDate=0;

	//Mod select gcQuery
	if(guPermLevel<10)
	sprintf(gcQuery,"SELECT tNS.uNS,\
				tNS.uModDate\
				FROM tNS,tClient\
				WHERE tNS.uNS=%u\
				AND tNS.uOwner=tClient.uClient\
				AND (tClient.uOwner=%u OR tClient.uClient=%u)"
			,uNS,guLoginClient,guLoginClient);
	else
		sprintf(gcQuery,"SELECT uNS,uModDate FROM tNS WHERE uNS=%u"
						,uNS);
	macro_mySQLRunAndStore(res);
	i=mysql_num_rows(res);

	//if(i<1) tNS("<blink>Record does not exist");
	if(i<1) tNS(LANG_NBR_RECNOTEXIST);
	//if(i>1) tNS("<blink>Multiple rows!");
	if(i>1) tNS(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tNS(LANG_NBR_EXTMOD);

	Update_tNS(field[0]);
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
	uModDate=luGetModDate("tNS",uNS);
	iDNSLog(uNS,"tNS","Mod");
	tNS(gcQuery);

}//ModtNS(void)


void tNSList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttNSListSelect();

	macro_mySQLRunAndStore(res);
	guI=mysql_num_rows(res);

	PageMachine("tNSList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttNSListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uNS<td><font face=arial,helvetica color=white>cFQDN<td><font face=arial,helvetica color=white>uNSType<td><font face=arial,helvetica color=white>uNSSet<td><font face=arial,helvetica color=white>uServer<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,ForeignKey("tNSType","cLabel",strtoul(field[2],NULL,10))
			,ForeignKey("tNSSet","cLabel",strtoul(field[3],NULL,10))
			,ForeignKey("tServer","cLabel",strtoul(field[4],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[5],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[6],NULL,10))
			,cBuf7
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[8],NULL,10))
			,cBuf9
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tNSList()


void CreatetNS(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tNS ( uNS INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cFQDN VARCHAR(100) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, uNSType INT UNSIGNED NOT NULL DEFAULT 0, uNSSet INT UNSIGNED NOT NULL DEFAULT 0,index (uNSSet), uServer INT UNSIGNED NOT NULL DEFAULT 0 )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetNS()

