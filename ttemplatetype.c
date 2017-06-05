/*
FILE
	tTemplateType source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
	svn ID removed
PURPOSE
	Template type codes.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uTemplateType: Primary Key
static unsigned uTemplateType=0;
//cLabel: Short label
static char cLabel[33]={""};
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



#define VAR_LIST_tTemplateType "tTemplateType.uTemplateType,tTemplateType.cLabel,tTemplateType.uOwner,tTemplateType.uCreatedBy,tTemplateType.uCreatedDate,tTemplateType.uModBy,tTemplateType.uModDate"

 //Local only
void Insert_tTemplateType(void);
void Update_tTemplateType(char *cRowid);
void ProcesstTemplateTypeListVars(pentry entries[], int x);

 //In tTemplateTypefunc.h file included below
void ExtProcesstTemplateTypeVars(pentry entries[], int x);
void ExttTemplateTypeCommands(pentry entries[], int x);
void ExttTemplateTypeButtons(void);
void ExttTemplateTypeNavBar(void);
void ExttTemplateTypeGetHook(entry gentries[], int x);
void ExttTemplateTypeSelect(void);
void ExttTemplateTypeSelectRow(void);
void ExttTemplateTypeListSelect(void);
void ExttTemplateTypeListFilter(void);
void ExttTemplateTypeAuxTable(void);

#include "ttemplatetypefunc.h"

 //Table Variables Assignment Function
void ProcesstTemplateTypeVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uTemplateType"))
			sscanf(entries[i].val,"%u",&uTemplateType);
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
	ExtProcesstTemplateTypeVars(entries,x);

}//ProcesstTemplateTypeVars()


void ProcesstTemplateTypeListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uTemplateType);
                        guMode=2002;
                        tTemplateType("");
                }
        }
}//void ProcesstTemplateTypeListVars(pentry entries[], int x)


int tTemplateTypeCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttTemplateTypeCommands(entries,x);

	if(!strcmp(gcFunction,"tTemplateTypeTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tTemplateTypeList();
		}

		//Default
		ProcesstTemplateTypeVars(entries,x);
		tTemplateType("");
	}
	else if(!strcmp(gcFunction,"tTemplateTypeList"))
	{
		ProcessControlVars(entries,x);
		ProcesstTemplateTypeListVars(entries,x);
		tTemplateTypeList();
	}

	return(0);

}//tTemplateTypeCommands()


void tTemplateType(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttTemplateTypeSelectRow();
		else
			ExttTemplateTypeSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetTemplateType();
				iDNS("New tTemplateType table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tTemplateType WHERE uTemplateType=%u"
						,uTemplateType);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uTemplateType);
		sprintf(cLabel,"%.32s",field[1]);
		sscanf(field[2],"%u",&uOwner);
		sscanf(field[3],"%u",&uCreatedBy);
		sscanf(field[4],"%lu",&uCreatedDate);
		sscanf(field[5],"%u",&uModBy);
		sscanf(field[6],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tTemplateType",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tTemplateTypeTools>");
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

        ExttTemplateTypeButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tTemplateType Record Data",100);

	if(guMode==2000 || guMode==2002)
		tTemplateTypeInput(1);
	else
		tTemplateTypeInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttTemplateTypeAuxTable();

	Footer_ism3();

}//end of tTemplateType();


void tTemplateTypeInput(unsigned uMode)
{

//uTemplateType
	OpenRow(LANG_FL_tTemplateType_uTemplateType,"black");
	printf("<input title='%s' type=text name=uTemplateType value=%u size=16 maxlength=10 "
,LANG_FT_tTemplateType_uTemplateType,uTemplateType);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uTemplateType value=%u >\n",uTemplateType);
	}
//cLabel
	OpenRow(LANG_FL_tTemplateType_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tTemplateType_cLabel,EncodeDoubleQuotes(cLabel));
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
	OpenRow(LANG_FL_tTemplateType_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tTemplateType_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tTemplateType_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tTemplateType_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tTemplateType_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tTemplateTypeInput(unsigned uMode)


void NewtTemplateType(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uTemplateType FROM tTemplateType\
				WHERE uTemplateType=%u"
							,uTemplateType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tTemplateType("<blink>Record already exists");
		tTemplateType(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tTemplateType();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uTemplateType=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tTemplateType",uTemplateType);
	iDNSLog(uTemplateType,"tTemplateType","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uTemplateType);
	tTemplateType(gcQuery);
	}

}//NewtTemplateType(unsigned uMode)


void DeletetTemplateType(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tTemplateType WHERE uTemplateType=%u AND ( uOwner=%u OR %u>9 )"
					,uTemplateType,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tTemplateType WHERE uTemplateType=%u"
					,uTemplateType);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tTemplateType("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uTemplateType,"tTemplateType","Del");
#endif
		tTemplateType(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uTemplateType,"tTemplateType","DelError");
#endif
		tTemplateType(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetTemplateType(void)


void Insert_tTemplateType(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tTemplateType SET uTemplateType=%u,cLabel='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uTemplateType
			,TextAreaSave(cLabel)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tTemplateType(void)


void Update_tTemplateType(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tTemplateType SET uTemplateType=%u,cLabel='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uTemplateType
			,TextAreaSave(cLabel)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tTemplateType(void)


void ModtTemplateType(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uTemplateType,uModDate FROM tTemplateType WHERE uTemplateType=%u"
			,uTemplateType);
#else
	sprintf(gcQuery,"SELECT uTemplateType FROM tTemplateType WHERE uTemplateType=%u"
			,uTemplateType);
#endif

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tTemplateType("<blink>Record does not exist");
	if(i<1) tTemplateType(LANG_NBR_RECNOTEXIST);
	//if(i>1) tTemplateType("<blink>Multiple rows!");
	if(i>1) tTemplateType(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tTemplateType(LANG_NBR_EXTMOD);
#endif

	Update_tTemplateType(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tTemplateType",uTemplateType);
	iDNSLog(uTemplateType,"tTemplateType","Mod");
#endif
	tTemplateType(gcQuery);

}//ModtTemplateType(void)


void tTemplateTypeList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttTemplateTypeListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tTemplateTypeList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttTemplateTypeListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uTemplateType<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		long luTime4=strtoul(field[4],NULL,10);
		char cBuf4[32];
		if(luTime4)
			ctime_r(&luTime4,cBuf4);
		else
			sprintf(cBuf4,"---");
		long luTime6=strtoul(field[6],NULL,10);
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

}//tTemplateTypeList()


void CreatetTemplateType(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tTemplateType ( uTemplateType INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(32) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0 )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetTemplateType()

