/*
FILE
	tDeletedResource source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Undo tResource table for certian undoable ops.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;

#include "mysqlrad.h"

//Table Variables
//Table Variables
//uDeletedResource: Primary Key
static unsigned uDeletedResource=0;
//uZone: Belongs to this tZone
static unsigned uZone=0;
//cName: Host name relative, full or emtpy=zone
static char cName[101]={""};
//uTTL: Optional TTL for this record only
static unsigned uTTL=0;
//uRRType: Pulldown of resource record type table
static unsigned uRRType=0;
static char cuRRTypePullDown[256]={""};
//cParam1: Parameter 1 field
static char cParam1[256]={""};
//cParam2: Parameter 2 field
static char cParam2[256]={""};
//cParam3: Parameter 3 field
static char cParam3[256]={""};
//cParam4: Parameter 4 field
static char cParam4[256]={""};
//cComment: Optional comment
static char *cComment={""};
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



#define VAR_LIST_tDeletedResource "tDeletedResource.uDeletedResource,tDeletedResource.uZone,tDeletedResource.cName,tDeletedResource.uTTL,tDeletedResource.uRRType,tDeletedResource.cParam1,tDeletedResource.cParam2,tDeletedResource.cParam3,tDeletedResource.cParam4,tDeletedResource.cComment,tDeletedResource.uOwner,tDeletedResource.uCreatedBy,tDeletedResource.uCreatedDate,tDeletedResource.uModBy,tDeletedResource.uModDate"

 //Local only
void Insert_tDeletedResource(void);
void Update_tDeletedResource(char *cRowid);
void ProcesstDeletedResourceListVars(pentry entries[], int x);

 //In tDeletedResourcefunc.h file included below
void ExtProcesstDeletedResourceVars(pentry entries[], int x);
void ExttDeletedResourceCommands(pentry entries[], int x);
void ExttDeletedResourceButtons(void);
void ExttDeletedResourceNavBar(void);
void ExttDeletedResourceGetHook(entry gentries[], int x);
void ExttDeletedResourceSelect(void);
void ExttDeletedResourceSelectRow(void);
void ExttDeletedResourceListSelect(void);
void ExttDeletedResourceListFilter(void);
void ExttDeletedResourceAuxTable(void);

#include "tdeletedresourcefunc.h"

 //Table Variables Assignment Function
void ProcesstDeletedResourceVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uDeletedResource"))
			sscanf(entries[i].val,"%u",&uDeletedResource);
		else if(!strcmp(entries[i].name,"uZone"))
			sscanf(entries[i].val,"%u",&uZone);
		else if(!strcmp(entries[i].name,"cName"))
			sprintf(cName,"%.100s",FQDomainName(entries[i].val));
		else if(!strcmp(entries[i].name,"uTTL"))
			sscanf(entries[i].val,"%u",&uTTL);
		else if(!strcmp(entries[i].name,"uRRType"))
			sscanf(entries[i].val,"%u",&uRRType);
		else if(!strcmp(entries[i].name,"cuRRTypePullDown"))
		{
			sprintf(cuRRTypePullDown,"%.255s",entries[i].val);
			uRRType=ReadPullDown("tRRType","cLabel",cuRRTypePullDown);
		}
		else if(!strcmp(entries[i].name,"cParam1"))
			sprintf(cParam1,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam2"))
			sprintf(cParam2,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"cComment"))
			cComment=entries[i].val;
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
	ExtProcesstDeletedResourceVars(entries,x);

}//ProcesstDeletedResourceVars()


void ProcesstDeletedResourceListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uDeletedResource);
                        guMode=2002;
                        tDeletedResource("");
                }
        }
}//void ProcesstDeletedResourceListVars(pentry entries[], int x)


int tDeletedResourceCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttDeletedResourceCommands(entries,x);

	if(!strcmp(gcFunction,"tDeletedResourceTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tDeletedResourceList();
		}

		//Default
		ProcesstDeletedResourceVars(entries,x);
		tDeletedResource("");
	}
	else if(!strcmp(gcFunction,"tDeletedResourceList"))
	{
		ProcessControlVars(entries,x);
		ProcesstDeletedResourceListVars(entries,x);
		tDeletedResourceList();
	}

	return(0);

}//tDeletedResourceCommands()


void tDeletedResource(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttDeletedResourceSelectRow();
		else
			ExttDeletedResourceSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetDeletedResource();
				iDNS("New tDeletedResource table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tDeletedResource WHERE uDeletedResource=%u"
						,uDeletedResource);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uDeletedResource);
		sscanf(field[1],"%u",&uZone);
		sprintf(cName,"%.100s",field[2]);
		sscanf(field[3],"%u",&uTTL);
		sscanf(field[4],"%u",&uRRType);
		sprintf(cParam1,"%.255s",field[5]);
		sprintf(cParam2,"%.255s",field[6]);
		sprintf(cParam3,"%.255s",field[7]);
		sprintf(cParam4,"%.255s",field[8]);
		cComment=field[9];
		sscanf(field[10],"%u",&uOwner);
		sscanf(field[11],"%u",&uCreatedBy);
		sscanf(field[12],"%lu",&uCreatedDate);
		sscanf(field[13],"%u",&uModBy);
		sscanf(field[14],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tDeletedResource",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tDeletedResourceTools>");
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

        ExttDeletedResourceButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tDeletedResource Record Data",100);

	if(guMode==2000 || guMode==2002)
		tDeletedResourceInput(1);
	else
		tDeletedResourceInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttDeletedResourceAuxTable();

	Footer_ism3();

}//end of tDeletedResource();


void tDeletedResourceInput(unsigned uMode)
{

//uDeletedResource
	OpenRow(LANG_FL_tDeletedResource_uDeletedResource,"black");
	printf("<input title='%s' type=text name=uDeletedResource value=%u size=16 maxlength=10 "
		,LANG_FT_tDeletedResource_uDeletedResource,uDeletedResource);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uDeletedResource value=%u >\n",uDeletedResource);
	}
//uZone
	OpenRow(LANG_FL_tDeletedResource_uZone,IsZero(uZone));
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uZone value=%u >\n",ForeignKey("tZone","cZone",uZone),uZone);
	}
	else
	{
	printf("%s<input type=hidden name=uZone value=%u >\n",ForeignKey("tZone","cZone",uZone),uZone);
	}
//cName
	OpenRow(LANG_FL_tDeletedResource_cName,"black");
	printf("<input title='%s' type=text name=cName value=\"%s\" size=40 maxlength=100 "
		,LANG_FT_tDeletedResource_cName,EncodeDoubleQuotes(cName));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cName value=\"%s\">\n",EncodeDoubleQuotes(cName));
	}
//uTTL
	OpenRow(LANG_FL_tDeletedResource_uTTL,"black");
	printf("<input title='%s' type=text name=uTTL value=%u size=16 maxlength=10 "
		,LANG_FT_tDeletedResource_uTTL,uTTL);
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uTTL value=%u >\n",uTTL);
	}
//uRRType
	OpenRow(LANG_FL_tDeletedResource_uRRType,IsZero(uRRType));
	if(guPermLevel>=0 && uMode)
		tTablePullDown("tRRType;cuRRTypePullDown","cLabel","cLabel",uRRType,1);
	else
		tTablePullDown("tRRType;cuRRTypePullDown","cLabel","cLabel",uRRType,0);
//cParam1
	OpenRow(LANG_FL_tDeletedResource_cParam1,EmptyString(cParam1));
	printf("<input title='%s' type=text name=cParam1 value=\"%s\" size=80 maxlength=255 "
		,LANG_FT_tDeletedResource_cParam1,EncodeDoubleQuotes(cParam1));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam1 value=\"%s\">\n",EncodeDoubleQuotes(cParam1));
	}
//cParam2
	OpenRow(LANG_FL_tDeletedResource_cParam2,"black");
	printf("<input title='%s' type=text name=cParam2 value=\"%s\" size=80 maxlength=255 "
		,LANG_FT_tDeletedResource_cParam2,EncodeDoubleQuotes(cParam2));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam2 value=\"%s\">\n",EncodeDoubleQuotes(cParam2));
	}
//cParam3
	OpenRow(LANG_FL_tResource_cParam3,"black");
	printf("<input title='%s' type=text name=cParam3 value=\"%s\" size=80 maxlength=255 "
		,LANG_FT_tResource_cParam3,EncodeDoubleQuotes(cParam3));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam3 value=\"%s\">\n",EncodeDoubleQuotes(cParam3));
	}
//cParam4
	OpenRow(LANG_FL_tResource_cParam4,"black");
	printf("<input title='%s' type=text name=cParam4 value=\"%s\" size=80 maxlength=255 "
		,LANG_FT_tResource_cParam4,EncodeDoubleQuotes(cParam4));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam4 value=\"%s\">\n",EncodeDoubleQuotes(cParam4));
	}
//cComment
	OpenRow(LANG_FL_tDeletedResource_cComment,"black");
	printf("<textarea title='%s' cols=40 wrap=hard rows=3 name=cComment "
		,LANG_FT_tDeletedResource_cComment);
	if(guPermLevel>=0 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",cComment);
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",cComment);
		printf("<input type=hidden name=cComment value=\"%s\" >\n",EncodeDoubleQuotes(cComment));
	}
//uOwner
	OpenRow(LANG_FL_tDeletedResource_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tDeletedResource_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tDeletedResource_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tDeletedResource_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tDeletedResource_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tDeletedResourceInput(unsigned uMode)


void NewtDeletedResource(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uDeletedResource FROM tDeletedResource\
				WHERE uDeletedResource=%u"
							,uDeletedResource);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tDeletedResource("<blink>Record already exists");
		tDeletedResource(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tDeletedResource();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uDeletedResource=mysql_insert_id(&gMysql);
	uCreatedDate=luGetCreatedDate("tDeletedResource",uDeletedResource);
	iDNSLog(uDeletedResource,"tDeletedResource","New");

	if(!uMode)
	{
		sprintf(gcQuery,LANG_NBR_NEWRECADDED,uDeletedResource);
		tDeletedResource(gcQuery);
	}

}//NewtDeletedResource(unsigned uMode)


void DeletetDeletedResource(void)
{
	sprintf(gcQuery,"DELETE FROM tDeletedResource WHERE uDeletedResource=%u AND ( uOwner=%u OR %u>9 )"
					,uDeletedResource,guLoginClient,guPermLevel);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tDeletedResource("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
		iDNSLog(uDeletedResource,"tDeletedResource","Del");
		tDeletedResource(LANG_NBR_RECDELETED);
	}
	else
	{
		iDNSLog(uDeletedResource,"tDeletedResource","DelError");
		tDeletedResource(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetDeletedResource(void)


void Insert_tDeletedResource(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tDeletedResource SET uDeletedResource=%u,uZone=%u,cName='%s',uTTL=%u,uRRType=%u,"
			"cParam1='%s',cParam2='%s',cParam3='%s',cParam4='%s',cComment='%s',uOwner=%u,uCreatedBy=%u,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uDeletedResource
			,uZone
			,TextAreaSave(cName)
			,uTTL
			,uRRType
			,TextAreaSave(cParam1)
			,TextAreaSave(cParam2)
			,TextAreaSave(cParam3)
			,TextAreaSave(cParam4)
			,TextAreaSave(cComment)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tDeletedResource(void)


void Update_tDeletedResource(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tDeletedResource SET uDeletedResource=%u,uZone=%u,cName='%s',uTTL=%u,uRRType=%u,"
			"cParam1='%s',cParam2='%s',cParam3='%s',cParam4='%s',cComment='%s',uModBy=%u,"
			"uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uDeletedResource
			,uZone
			,TextAreaSave(cName)
			,uTTL
			,uRRType
			,TextAreaSave(cParam1)
			,TextAreaSave(cParam2)
			,TextAreaSave(cParam3)
			,TextAreaSave(cParam4)
			,TextAreaSave(cComment)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tDeletedResource(void)


void ModtDeletedResource(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uDeletedResource,uModDate FROM tDeletedResource WHERE uDeletedResource=%u"
			,uDeletedResource);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tDeletedResource("<blink>Record does not exist");
	if(i<1) tDeletedResource(LANG_NBR_RECNOTEXIST);
	//if(i>1) tDeletedResource("<blink>Multiple rows!");
	if(i>1) tDeletedResource(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tDeletedResource(LANG_NBR_EXTMOD);

	Update_tDeletedResource(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
	uModDate=luGetModDate("tDeletedResource",uDeletedResource);
	iDNSLog(uDeletedResource,"tDeletedResource","Mod");
	tDeletedResource(gcQuery);

}//ModtDeletedResource(void)


void tDeletedResourceList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttDeletedResourceListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tDeletedResourceList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttDeletedResourceListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uDeletedResource"
		"<td><font face=arial,helvetica color=white>uZone<td><font face=arial,helvetica color=white>cName"
		"<td><font face=arial,helvetica color=white>uTTL<td><font face=arial,helvetica color=white>uRRType"
		"<td><font face=arial,helvetica color=white>cParam1<td><font face=arial,helvetica color=white>cParam2"
		"<td><font face=arial,helvetica color=white>cComment<td><font face=arial,helvetica color=white>uOwner"
		"<td><font face=arial,helvetica color=white>uCreatedBy"
		"<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy"
		"<td><font face=arial,helvetica color=white>uModDate</tr>");

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
		long luTime10=strtoul(field[10],NULL,10);
		char cBuf10[32];
		if(luTime10)
			ctime_r(&luTime10,cBuf10);
		else
			sprintf(cBuf10,"---");
		long luTime12=strtoul(field[12],NULL,10);
		char cBuf12[32];
		if(luTime12)
			ctime_r(&luTime12,cBuf12);
		else
			sprintf(cBuf12,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s"
			"<td><textarea disabled>%s</textarea><td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,ForeignKey("tZone","cZone",strtoul(field[1],NULL,10))
			,field[2]
			,field[3]
			,ForeignKey("tRRType","cLabel",strtoul(field[4],NULL,10))
			,field[5]
			,field[6]
			,field[7]
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[8],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[9],NULL,10))
			,cBuf10
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[11],NULL,10))
			,cBuf12
				);
	}

	printf("</table></form>\n");
	Footer_ism3();

}//tDeletedResourceList()


void CreatetDeletedResource(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tDeletedResource ("
			" uDeletedResource INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
			" cName VARCHAR(100) NOT NULL DEFAULT '',"
			" uOwner INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uOwner),"
			" uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0,"
			" uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0,"
			" uModBy INT UNSIGNED NOT NULL DEFAULT 0,"
			" uModDate INT UNSIGNED NOT NULL DEFAULT 0,"
			" uTTL INT UNSIGNED NOT NULL DEFAULT 0,"
			" uRRType INT UNSIGNED NOT NULL DEFAULT 0,"
			" cParam1 VARCHAR(255) NOT NULL DEFAULT '',"
			" cParam2 VARCHAR(255) NOT NULL DEFAULT '',"
			" cComment TEXT NOT NULL DEFAULT '',"
			" uZone INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uZone),"
			" cParam3 VARCHAR(255) NOT NULL DEFAULT '',"
			" cParam4 VARCHAR(255) NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetDeletedResource()

