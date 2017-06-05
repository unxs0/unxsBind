/*
FILE
	tGlossary source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Glossary table for terms used herein.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uGlossary: Primary Key
static unsigned uGlossary=0;
//cLabel: Short label
static char cLabel[33]={""};
//cText: Definition of the label referenced in cLabel
static char *cText={""};
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



#define VAR_LIST_tGlossary "tGlossary.uGlossary,tGlossary.cLabel,tGlossary.cText,tGlossary.uOwner,tGlossary.uCreatedBy,tGlossary.uCreatedDate,tGlossary.uModBy,tGlossary.uModDate"

 //Local only
void Insert_tGlossary(void);
void Update_tGlossary(char *cRowid);
void ProcesstGlossaryListVars(pentry entries[], int x);

 //In tGlossaryfunc.h file included below
void ExtProcesstGlossaryVars(pentry entries[], int x);
void ExttGlossaryCommands(pentry entries[], int x);
void ExttGlossaryButtons(void);
void ExttGlossaryNavBar(void);
void ExttGlossaryGetHook(entry gentries[], int x);
void ExttGlossarySelect(void);
void ExttGlossarySelectRow(void);
void ExttGlossaryListSelect(void);
void ExttGlossaryListFilter(void);
void ExttGlossaryAuxTable(void);

#include "tglossaryfunc.h"

 //Table Variables Assignment Function
void ProcesstGlossaryVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uGlossary"))
			sscanf(entries[i].val,"%u",&uGlossary);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cText"))
			cText=entries[i].val;
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
	ExtProcesstGlossaryVars(entries,x);

}//ProcesstGlossaryVars()


void ProcesstGlossaryListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uGlossary);
                        guMode=2002;
                        tGlossary("");
                }
        }
}//void ProcesstGlossaryListVars(pentry entries[], int x)


int tGlossaryCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttGlossaryCommands(entries,x);

	if(!strcmp(gcFunction,"tGlossaryTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tGlossaryList();
		}

		//Default
		ProcesstGlossaryVars(entries,x);
		tGlossary("");
	}
	else if(!strcmp(gcFunction,"tGlossaryList"))
	{
		ProcessControlVars(entries,x);
		ProcesstGlossaryListVars(entries,x);
		tGlossaryList();
	}

	return(0);

}//tGlossaryCommands()


void tGlossary(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttGlossarySelectRow();
		else
			ExttGlossarySelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetGlossary();
				iDNS("New tGlossary table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tGlossary WHERE uGlossary=%u"
						,uGlossary);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uGlossary);
		sprintf(cLabel,"%.32s",field[1]);
		cText=field[2];
		sscanf(field[3],"%u",&uOwner);
		sscanf(field[4],"%u",&uCreatedBy);
		sscanf(field[5],"%lu",&uCreatedDate);
		sscanf(field[6],"%u",&uModBy);
		sscanf(field[7],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tGlossary",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tGlossaryTools>");
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

        ExttGlossaryButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("Record Data",100);

	if(guMode==2000 || guMode==2002)
		tGlossaryInput(1);
	else
		tGlossaryInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttGlossaryAuxTable();

	Footer_ism3();

}//end of tGlossary();


void tGlossaryInput(unsigned uMode)
{

//uGlossary
	OpenRow(LANG_FL_tGlossary_uGlossary,"black");
	printf("<input title='%s' type=text name=uGlossary value=%u size=16 maxlength=10 "
,LANG_FT_tGlossary_uGlossary,uGlossary);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uGlossary value=%u >\n",uGlossary);
	}
//cLabel
	OpenRow(LANG_FL_tGlossary_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tGlossary_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//cText
	OpenRow(LANG_FL_tGlossary_cText,"black");
	printf("<textarea title='%s' cols=80 wrap=hard rows=16 name=cText "
,LANG_FT_tGlossary_cText);
	if(guPermLevel>=7 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",cText);
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",cText);
		printf("<input type=hidden name=cText value=\"%s\" >\n",EncodeDoubleQuotes(cText));
	}
//uOwner
	OpenRow(LANG_FL_tGlossary_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tGlossary_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tGlossary_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tGlossary_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tGlossary_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tGlossaryInput(unsigned uMode)


void NewtGlossary(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uGlossary FROM tGlossary\
				WHERE uGlossary=%u"
							,uGlossary);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tGlossary("<blink>Record already exists");
		tGlossary(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tGlossary();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uGlossary=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tGlossary",uGlossary);
	iDNSLog(uGlossary,"tGlossary","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uGlossary);
	tGlossary(gcQuery);
	}

}//NewtGlossary(unsigned uMode)


void DeletetGlossary(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tGlossary WHERE uGlossary=%u AND ( uOwner=%u OR %u>9 )"
					,uGlossary,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tGlossary WHERE uGlossary=%u"
					,uGlossary);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tGlossary("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uGlossary,"tGlossary","Del");
#endif
		tGlossary(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uGlossary,"tGlossary","DelError");
#endif
		tGlossary(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetGlossary(void)


void Insert_tGlossary(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tGlossary SET uGlossary=%u,cLabel='%s',cText='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uGlossary
			,TextAreaSave(cLabel)
			,TextAreaSave(cText)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tGlossary(void)


void Update_tGlossary(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tGlossary SET uGlossary=%u,cLabel='%s',cText='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uGlossary
			,TextAreaSave(cLabel)
			,TextAreaSave(cText)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tGlossary(void)


void ModtGlossary(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uGlossary,uModDate FROM tGlossary WHERE uGlossary=%u"
			,uGlossary);
#else
	sprintf(gcQuery,"SELECT uGlossary FROM tGlossary WHERE uGlossary=%u"
			,uGlossary);
#endif

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tGlossary("<blink>Record does not exist");
	if(i<1) tGlossary(LANG_NBR_RECNOTEXIST);
	//if(i>1) tGlossary("<blink>Multiple rows!");
	if(i>1) tGlossary(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tGlossary(LANG_NBR_EXTMOD);
#endif

	Update_tGlossary(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tGlossary",uGlossary);
	iDNSLog(uGlossary,"tGlossary","Mod");
#endif
	tGlossary(gcQuery);

}//ModtGlossary(void)


void tGlossaryList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttGlossaryListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tGlossaryList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttGlossaryListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uGlossary<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>cText<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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

}//tGlossaryList()


void CreatetGlossary(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tGlossary ( uGlossary INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(32) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, cText TEXT NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetGlossary()

