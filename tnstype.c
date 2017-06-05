/*
FILE
	tNSType source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis 2001-2007
PURPOSE
	Name server type codes. E.g. hidden, primary, secondary etc.
	Still has old DNS server racist labels, like SLAVE, that need to be updated.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uNSType: Primary Key
static unsigned uNSType=0;
//cLabel: Short label
static char cLabel[33]={""};
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



#define VAR_LIST_tNSType "tNSType.uNSType,tNSType.cLabel,tNSType.uOwner,tNSType.uCreatedBy,tNSType.uCreatedDate,tNSType.uModBy,tNSType.uModDate"

 //Local only
void Insert_tNSType(void);
void Update_tNSType(char *cRowid);
void ProcesstNSTypeListVars(pentry entries[], int x);

 //In tNSTypefunc.h file included below
void ExtProcesstNSTypeVars(pentry entries[], int x);
void ExttNSTypeCommands(pentry entries[], int x);
void ExttNSTypeButtons(void);
void ExttNSTypeNavBar(void);
void ExttNSTypeGetHook(entry gentries[], int x);
void ExttNSTypeSelect(void);
void ExttNSTypeSelectRow(void);
void ExttNSTypeListSelect(void);
void ExttNSTypeListFilter(void);
void ExttNSTypeAuxTable(void);

#include "tnstypefunc.h"

 //Table Variables Assignment Function
void ProcesstNSTypeVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uNSType"))
			sscanf(entries[i].val,"%u",&uNSType);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.32s",entries[i].val);
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
	ExtProcesstNSTypeVars(entries,x);

}//ProcesstNSTypeVars()


void ProcesstNSTypeListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uNSType);
                        guMode=2002;
                        tNSType("");
                }
        }
}//void ProcesstNSTypeListVars(pentry entries[], int x)


int tNSTypeCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttNSTypeCommands(entries,x);

	if(!strcmp(gcFunction,"tNSTypeTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tNSTypeList();
		}

		//Default
		ProcesstNSTypeVars(entries,x);
		tNSType("");
	}
	else if(!strcmp(gcFunction,"tNSTypeList"))
	{
		ProcessControlVars(entries,x);
		ProcesstNSTypeListVars(entries,x);
		tNSTypeList();
	}

	return(0);

}//tNSTypeCommands()


void tNSType(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttNSTypeSelectRow();
		else
			ExttNSTypeSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetNSType();
				iDNS("New tNSType table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tNSType WHERE uNSType=%u"
						,uNSType);
				macro_mySQLRunAndStore(res2);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uNSType);
		sprintf(cLabel,"%.32s",field[1]);
		sscanf(field[2],"%u",&uOwner);
		sscanf(field[3],"%u",&uCreatedBy);
		sscanf(field[4],"%lu",&uCreatedDate);
		sscanf(field[5],"%u",&uModBy);
		sscanf(field[6],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tNSType",0);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tNSTypeTools>");
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

        ExttNSTypeButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tNSType Record Data",100);

	if(guMode==2000 || guMode==2002)
		tNSTypeInput(1);
	else
		tNSTypeInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttNSTypeAuxTable();

	Footer_ism3();

}//end of tNSType();


void tNSTypeInput(unsigned uMode)
{

//uNSType
	OpenRow(LANG_FL_tNSType_uNSType,"black");
	printf("<input title='%s' type=text name=uNSType value=%u size=16 maxlength=10 "
,LANG_FT_tNSType_uNSType,uNSType);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uNSType value=%u >\n",uNSType);
	}
//cLabel
	OpenRow(LANG_FL_tNSType_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tNSType_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//uOwner
	OpenRow(LANG_FL_tNSType_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tNSType_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tNSType_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tNSType_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tNSType_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tNSTypeInput(unsigned uMode)


void NewtNSType(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uNSType FROM tNSType\
				WHERE uNSType=%u"
							,uNSType);
	macro_mySQLRunAndStore(res);
	i=mysql_num_rows(res);

	if(i) 
		//tNSType("<blink>Record already exists");
		tNSType(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tNSType();
	//sprintf(gcQuery,"New record %u added");
	uNSType=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tNSType",uNSType);
	iDNSLog(uNSType,"tNSType","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uNSType);
	tNSType(gcQuery);
	}

}//NewtNSType(unsigned uMode)


void DeletetNSType(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tNSType WHERE uNSType=%u AND ( uOwner=%u OR %u>9 )"
					,uNSType,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tNSType WHERE uNSType=%u"
					,uNSType);
#endif
	macro_mySQLQueryHTMLError;
	//tNSType("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uNSType,"tNSType","Del");
#endif
		tNSType(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uNSType,"tNSType","DelError");
#endif
		tNSType(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetNSType(void)


void Insert_tNSType(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tNSType SET uNSType=%u,cLabel='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uNSType
			,TextAreaSave(cLabel)
			,uOwner
			,uCreatedBy
			);

	macro_mySQLQueryHTMLError;

}//void Insert_tNSType(void)


void Update_tNSType(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tNSType SET uNSType=%u,cLabel='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uNSType
			,TextAreaSave(cLabel)
			,uModBy
			,cRowid);

	macro_mySQLQueryHTMLError;

}//void Update_tNSType(void)


void ModtNSType(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	//Mod select gcQuery
	if(guPermLevel<10)
	sprintf(gcQuery,"SELECT tNSType.uNSType,\
				tNSType.uModDate\
				FROM tNSType,tClient\
				WHERE tNSType.uNSType=%u\
				AND tNSType.uOwner=tClient.uClient\
				AND (tClient.uOwner=%u OR tClient.uClient=%u)"
			,uNSType,guLoginClient,guLoginClient);
	else
	sprintf(gcQuery,"SELECT uNSType,uModDate FROM tNSType\
				WHERE uNSType=%u"
						,uNSType);
#else
	sprintf(gcQuery,"SELECT uNSType FROM tNSType\
				WHERE uNSType=%u"
						,uNSType);
#endif

	macro_mySQLRunAndStore(res);
	i=mysql_num_rows(res);

	//if(i<1) tNSType("<blink>Record does not exist");
	if(i<1) tNSType(LANG_NBR_RECNOTEXIST);
	//if(i>1) tNSType("<blink>Multiple rows!");
	if(i>1) tNSType(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tNSType(LANG_NBR_EXTMOD);
#endif

	Update_tNSType(field[0]);
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tNSType",uNSType);
	iDNSLog(uNSType,"tNSType","Mod");
#endif
	tNSType(gcQuery);

}//ModtNSType(void)


void tNSTypeList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttNSTypeListSelect();

	macro_mySQLRunAndStore(res);
	guI=mysql_num_rows(res);

	PageMachine("tNSTypeList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttNSTypeListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uNSType<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		time_t luTime4=strtoul(field[4],NULL,10);
		char cBuf4[32];
		if(luTime4)
			ctime_r(&luTime4,cBuf4);
		else
			sprintf(cBuf4,"---");
		time_t luTime6=strtoul(field[6],NULL,10);
		char cBuf6[32];
		if(luTime6)
			ctime_r(&luTime6,cBuf6);
		else
			sprintf(cBuf6,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[2],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[3],NULL,10))
			,cBuf4
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[5],NULL,10))
			,cBuf6
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tNSTypeList()


void CreatetNSType(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tNSType ( uNSType INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(32) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0 )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetNSType()

