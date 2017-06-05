/*
FILE
	tTemplate source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	HTML/CSS templates for web interfaces.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uTemplate: Primary Key
static unsigned uTemplate=0;
//cLabel: Short label
static char cLabel[33]={""};
//uTemplateSet: Short label
static unsigned uTemplateSet=0;
static char cuTemplateSetPullDown[256]={""};
//uTemplateType: Short label
static unsigned uTemplateType=0;
static char cuTemplateTypePullDown[256]={""};
//cComment: About the template
static char *cComment={""};
//cTemplate: Template itself
static char *cTemplate={""};
//uOwner: Record owner
static unsigned uOwner=0;
//uCreatedBy: uClient for last insert
static unsigned uCreatedBy=0;
//uCreatedDate: Unix seconds date last insert
static long uCreatedDate=0;
//uModBy: uClient for last update
static unsigned uModBy=0;
//uModDate: Unix seconds date last update
static long uModDate=0;



#define VAR_LIST_tTemplate "tTemplate.uTemplate,tTemplate.cLabel,tTemplate.uTemplateSet,tTemplate.uTemplateType,tTemplate.cComment,tTemplate.cTemplate,tTemplate.uOwner,tTemplate.uCreatedBy,tTemplate.uCreatedDate,tTemplate.uModBy,tTemplate.uModDate"

 //Local only
void Insert_tTemplate(void);
void Update_tTemplate(char *cRowid);
void ProcesstTemplateListVars(pentry entries[], int x);

 //In tTemplatefunc.h file included below
void ExtProcesstTemplateVars(pentry entries[], int x);
void ExttTemplateCommands(pentry entries[], int x);
void ExttTemplateButtons(void);
void ExttTemplateNavBar(void);
void ExttTemplateGetHook(entry gentries[], int x);
void ExttTemplateSelect(void);
void ExttTemplateSelectRow(void);
void ExttTemplateListSelect(void);
void ExttTemplateListFilter(void);
void ExttTemplateAuxTable(void);

#include "ttemplatefunc.h"

 //Table Variables Assignment Function
void ProcesstTemplateVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uTemplate"))
			sscanf(entries[i].val,"%u",&uTemplate);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"uTemplateSet"))
			sscanf(entries[i].val,"%u",&uTemplateSet);
		else if(!strcmp(entries[i].name,"cuTemplateSetPullDown"))
		{
			sprintf(cuTemplateSetPullDown,"%.255s",entries[i].val);
			uTemplateSet=ReadPullDown("tTemplateSet","cLabel",cuTemplateSetPullDown);
		}
		else if(!strcmp(entries[i].name,"uTemplateType"))
			sscanf(entries[i].val,"%u",&uTemplateType);
		else if(!strcmp(entries[i].name,"cuTemplateTypePullDown"))
		{
			sprintf(cuTemplateTypePullDown,"%.255s",entries[i].val);
			uTemplateType=ReadPullDown("tTemplateType","cLabel",cuTemplateTypePullDown);
		}
		else if(!strcmp(entries[i].name,"cComment"))
			cComment=entries[i].val;
		else if(!strcmp(entries[i].name,"cTemplate"))
			cTemplate=entries[i].val;
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
	ExtProcesstTemplateVars(entries,x);

}//ProcesstTemplateVars()


void ProcesstTemplateListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uTemplate);
                        guMode=2002;
                        tTemplate("");
                }
        }
}//void ProcesstTemplateListVars(pentry entries[], int x)


int tTemplateCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttTemplateCommands(entries,x);

	if(!strcmp(gcFunction,"tTemplateTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tTemplateList();
		}

		//Default
		ProcesstTemplateVars(entries,x);
		tTemplate("");
	}
	else if(!strcmp(gcFunction,"tTemplateList"))
	{
		ProcessControlVars(entries,x);
		ProcesstTemplateListVars(entries,x);
		tTemplateList();
	}

	return(0);

}//tTemplateCommands()


void tTemplate(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttTemplateSelectRow();
		else
			ExttTemplateSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetTemplate();
				iDNS("New tTemplate table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tTemplate WHERE uTemplate=%u"
						,uTemplate);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uTemplate);
		sprintf(cLabel,"%.32s",field[1]);
		sscanf(field[2],"%u",&uTemplateSet);
		sscanf(field[3],"%u",&uTemplateType);
		cComment=field[4];
		cTemplate=field[5];
		sscanf(field[6],"%u",&uOwner);
		sscanf(field[7],"%u",&uCreatedBy);
		sscanf(field[8],"%lu",&uCreatedDate);
		sscanf(field[9],"%u",&uModBy);
		sscanf(field[10],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tTemplate",1);

	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tTemplateTools>");
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

        ExttTemplateButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tTemplate Record Data",100);

	if(guMode==2000 || guMode==2002)
		tTemplateInput(1);
	else
		tTemplateInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttTemplateAuxTable();

	Footer_ism3();

}//end of tTemplate();


void tTemplateInput(unsigned uMode)
{

//uTemplate
	OpenRow(LANG_FL_tTemplate_uTemplate,"black");
	printf("<input title='%s' type=text name=uTemplate value=%u size=16 maxlength=10 "
,LANG_FT_tTemplate_uTemplate,uTemplate);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uTemplate value=%u >\n",uTemplate);
	}
//cLabel
	OpenRow(LANG_FL_tTemplate_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tTemplate_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//uTemplateSet
	OpenRow(LANG_FL_tTemplate_uTemplateSet,"black");
	if(guPermLevel>=7 && uMode)
		tTablePullDown("tTemplateSet;cuTemplateSetPullDown","cLabel","cLabel",uTemplateSet,1);
	else
		tTablePullDown("tTemplateSet;cuTemplateSetPullDown","cLabel","cLabel",uTemplateSet,0);
//uTemplateType
	OpenRow(LANG_FL_tTemplate_uTemplateType,"black");
	if(guPermLevel>=7 && uMode)
		tTablePullDown("tTemplateType;cuTemplateTypePullDown","cLabel","cLabel",uTemplateType,1);
	else
		tTablePullDown("tTemplateType;cuTemplateTypePullDown","cLabel","cLabel",uTemplateType,0);
//cComment
	OpenRow(LANG_FL_tTemplate_cComment,"black");
	printf("<textarea title='%s' cols=80 wrap=hard rows=16 name=cComment "
,LANG_FT_tTemplate_cComment);
	if(guPermLevel>=7 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",TransformAngleBrackets(cComment));
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",TransformAngleBrackets(cComment));
		printf("<input type=hidden name=cComment value=\"%s\" >\n",EncodeDoubleQuotes(cComment));
	}
//cTemplate
	OpenRow(LANG_FL_tTemplate_cTemplate,"black");
	printf("<textarea title='%s' cols=80 wrap=off rows=16 name=cTemplate "
	,LANG_FT_tTemplate_cTemplate);
	if(guPermLevel>=7 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",TransformAngleBrackets(cTemplate));
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",TransformAngleBrackets(cTemplate));
		printf("<input type=hidden name=cTemplate value=\"%s\" >\n",EncodeDoubleQuotes(cTemplate));
	}
//uOwner
	OpenRow(LANG_FL_tTemplate_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tTemplate_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tTemplate_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tTemplate_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tTemplate_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tTemplateInput(unsigned uMode)


void NewtTemplate(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uTemplate FROM tTemplate\
				WHERE uTemplate=%u"
							,uTemplate);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tTemplate("<blink>Record already exists");
		tTemplate(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tTemplate();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uTemplate=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tTemplate",uTemplate);
	iDNSLog(uTemplate,"tTemplate","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uTemplate);
	tTemplate(gcQuery);
	}

}//NewtTemplate(unsigned uMode)


void DeletetTemplate(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tTemplate WHERE uTemplate=%u AND ( uOwner=%u OR %u>9 )"
					,uTemplate,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tTemplate WHERE uTemplate=%u"
					,uTemplate);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tTemplate("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uTemplate,"tTemplate","Del");
#endif
		tTemplate(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uTemplate,"tTemplate","DelError");
#endif
		tTemplate(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetTemplate(void)


void Insert_tTemplate(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tTemplate SET uTemplate=%u,cLabel='%s',uTemplateSet=%u,uTemplateType=%u,cComment='%s',cTemplate='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uTemplate
			,TextAreaSave(cLabel)
			,uTemplateSet
			,uTemplateType
			,TextAreaSave(cComment)
			,TextAreaSave(cTemplate)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tTemplate(void)


void Update_tTemplate(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tTemplate SET uTemplate=%u,cLabel='%s',uTemplateSet=%u,uTemplateType=%u,cComment='%s',cTemplate='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uTemplate
			,TextAreaSave(cLabel)
			,uTemplateSet
			,uTemplateType
			,TextAreaSave(cComment)
			,TextAreaSave(cTemplate)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tTemplate(void)


void ModtTemplate(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uTemplate,uModDate FROM tTemplate WHERE uTemplate=%u"
			,uTemplate);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tTemplate("<blink>Record does not exist");
	if(i<1) tTemplate(LANG_NBR_RECNOTEXIST);
	//if(i>1) tTemplate("<blink>Multiple rows!");
	if(i>1) tTemplate(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tTemplate(LANG_NBR_EXTMOD);

	Update_tTemplate(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
	uModDate=luGetModDate("tTemplate",uTemplate);
	iDNSLog(uTemplate,"tTemplate","Mod");
	tTemplate(gcQuery);

}//ModtTemplate(void)


void tTemplateList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttTemplateListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tTemplateList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttTemplateListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uTemplate<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>uTemplateSet<td><font face=arial,helvetica color=white>uTemplateType<td><font face=arial,helvetica color=white>cComment<td><font face=arial,helvetica color=white>cTemplate<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		long luTime8=strtoul(field[8],NULL,10);
		char cBuf8[32];
		if(luTime8)
			ctime_r(&luTime8,cBuf8);
		else
			sprintf(cBuf8,"---");
		long luTime10=strtoul(field[10],NULL,10);
		char cBuf10[32];
		if(luTime10)
			ctime_r(&luTime10,cBuf10);
		else
			sprintf(cBuf10,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td><textarea disabled><pre>%s</pre></textarea><td><textarea disabled><pre>%s</pre></textarea><td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,ForeignKey("tTemplateSet","cLabel",strtoul(field[2],NULL,10))
			,ForeignKey("tTemplateType","cLabel",strtoul(field[3],NULL,10))
			,TransformAngleBrackets(field[4])
			,TransformAngleBrackets(field[5])
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[6],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[7],NULL,10))
			,cBuf8
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[9],NULL,10))
			,cBuf10
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tTemplateList()


void CreatetTemplate(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tTemplate ( uTemplate INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(32) NOT NULL DEFAULT '', INDEX (cLabel), uOwner INT UNSIGNED NOT NULL DEFAULT 0, INDEX (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, cComment TEXT NOT NULL DEFAULT '', cTemplate TEXT NOT NULL DEFAULT '', uTemplateSet INT UNSIGNED NOT NULL DEFAULT 0, uTemplateType INT UNSIGNED NOT NULL DEFAULT 0 )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetTemplate()

