/*
FILE
	tTemplateSet source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
	svn ID removed
PURPOSE
	Template set codes.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uTemplateSet: Primary Key
static unsigned uTemplateSet=0;
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



#define VAR_LIST_tTemplateSet "tTemplateSet.uTemplateSet,tTemplateSet.cLabel,tTemplateSet.uOwner,tTemplateSet.uCreatedBy,tTemplateSet.uCreatedDate,tTemplateSet.uModBy,tTemplateSet.uModDate"

 //Local only
void Insert_tTemplateSet(void);
void Update_tTemplateSet(char *cRowid);
void ProcesstTemplateSetListVars(pentry entries[], int x);

 //In tTemplateSetfunc.h file included below
void ExtProcesstTemplateSetVars(pentry entries[], int x);
void ExttTemplateSetCommands(pentry entries[], int x);
void ExttTemplateSetButtons(void);
void ExttTemplateSetNavBar(void);
void ExttTemplateSetGetHook(entry gentries[], int x);
void ExttTemplateSetSelect(void);
void ExttTemplateSetSelectRow(void);
void ExttTemplateSetListSelect(void);
void ExttTemplateSetListFilter(void);
void ExttTemplateSetAuxTable(void);

#include "ttemplatesetfunc.h"

 //Table Variables Assignment Function
void ProcesstTemplateSetVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uTemplateSet"))
			sscanf(entries[i].val,"%u",&uTemplateSet);
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
	ExtProcesstTemplateSetVars(entries,x);

}//ProcesstTemplateSetVars()


void ProcesstTemplateSetListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uTemplateSet);
                        guMode=2002;
                        tTemplateSet("");
                }
        }
}//void ProcesstTemplateSetListVars(pentry entries[], int x)


int tTemplateSetCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttTemplateSetCommands(entries,x);

	if(!strcmp(gcFunction,"tTemplateSetTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tTemplateSetList();
		}

		//Default
		ProcesstTemplateSetVars(entries,x);
		tTemplateSet("");
	}
	else if(!strcmp(gcFunction,"tTemplateSetList"))
	{
		ProcessControlVars(entries,x);
		ProcesstTemplateSetListVars(entries,x);
		tTemplateSetList();
	}

	return(0);

}//tTemplateSetCommands()


void tTemplateSet(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttTemplateSetSelectRow();
		else
			ExttTemplateSetSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetTemplateSet();
				iDNS("New tTemplateSet table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tTemplateSet WHERE uTemplateSet=%u"
						,uTemplateSet);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uTemplateSet);
		sprintf(cLabel,"%.32s",field[1]);
		sscanf(field[2],"%u",&uOwner);
		sscanf(field[3],"%u",&uCreatedBy);
		sscanf(field[4],"%lu",&uCreatedDate);
		sscanf(field[5],"%u",&uModBy);
		sscanf(field[6],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tTemplateSet",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tTemplateSetTools>");
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

        ExttTemplateSetButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tTemplateSet Record Data",100);

	if(guMode==2000 || guMode==2002)
		tTemplateSetInput(1);
	else
		tTemplateSetInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttTemplateSetAuxTable();

	Footer_ism3();

}//end of tTemplateSet();


void tTemplateSetInput(unsigned uMode)
{

//uTemplateSet
	OpenRow(LANG_FL_tTemplateSet_uTemplateSet,"black");
	printf("<input title='%s' type=text name=uTemplateSet value=%u size=16 maxlength=10 "
,LANG_FT_tTemplateSet_uTemplateSet,uTemplateSet);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uTemplateSet value=%u >\n",uTemplateSet);
	}
//cLabel
	OpenRow(LANG_FL_tTemplateSet_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tTemplateSet_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//uOwner
	OpenRow(LANG_FL_tTemplateSet_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tTemplateSet_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tTemplateSet_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tTemplateSet_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tTemplateSet_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tTemplateSetInput(unsigned uMode)


void NewtTemplateSet(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uTemplateSet FROM tTemplateSet\
				WHERE uTemplateSet=%u"
							,uTemplateSet);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tTemplateSet("<blink>Record already exists");
		tTemplateSet(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tTemplateSet();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uTemplateSet=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tTemplateSet",uTemplateSet);
	iDNSLog(uTemplateSet,"tTemplateSet","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uTemplateSet);
	tTemplateSet(gcQuery);
	}

}//NewtTemplateSet(unsigned uMode)


void DeletetTemplateSet(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tTemplateSet WHERE uTemplateSet=%u AND ( uOwner=%u OR %u>9 )"
					,uTemplateSet,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tTemplateSet WHERE uTemplateSet=%u"
					,uTemplateSet);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tTemplateSet("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uTemplateSet,"tTemplateSet","Del");
#endif
		tTemplateSet(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uTemplateSet,"tTemplateSet","DelError");
#endif
		tTemplateSet(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetTemplateSet(void)


void Insert_tTemplateSet(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tTemplateSet SET uTemplateSet=%u,cLabel='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uTemplateSet
			,TextAreaSave(cLabel)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tTemplateSet(void)


void Update_tTemplateSet(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tTemplateSet SET uTemplateSet=%u,cLabel='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uTemplateSet
			,TextAreaSave(cLabel)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tTemplateSet(void)


void ModtTemplateSet(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uTemplateSet,uModDate FROM tTemplateSet WHERE uTemplateSet=%u"
			,uTemplateSet);
#else
	sprintf(gcQuery,"SELECT uTemplateSet FROM tTemplateSet WHERE uTemplateSet=%u"
			,uTemplateSet);
#endif

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tTemplateSet("<blink>Record does not exist");
	if(i<1) tTemplateSet(LANG_NBR_RECNOTEXIST);
	//if(i>1) tTemplateSet("<blink>Multiple rows!");
	if(i>1) tTemplateSet(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tTemplateSet(LANG_NBR_EXTMOD);
#endif

	Update_tTemplateSet(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tTemplateSet",uTemplateSet);
	iDNSLog(uTemplateSet,"tTemplateSet","Mod");
#endif
	tTemplateSet(gcQuery);

}//ModtTemplateSet(void)


void tTemplateSetList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttTemplateSetListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tTemplateSetList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttTemplateSetListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uTemplateSet<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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

}//tTemplateSetList()


void CreatetTemplateSet(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tTemplateSet ( uTemplateSet INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(32) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0 )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetTemplateSet()

