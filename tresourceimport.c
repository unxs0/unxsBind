/*
FILE
	tResourceImport source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
	svn ID removed
PURPOSE
	Import staging table for safety.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uResource: Primary Key
static unsigned uResource=0;
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
//cComment: Optional comment
static char *cComment={""};
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



#define VAR_LIST_tResourceImport "tResourceImport.uResource,tResourceImport.uZone,tResourceImport.cName,tResourceImport.uTTL,tResourceImport.uRRType,tResourceImport.cParam1,tResourceImport.cParam2,tResourceImport.cComment,tResourceImport.uOwner,tResourceImport.uCreatedBy,tResourceImport.uCreatedDate,tResourceImport.uModBy,tResourceImport.uModDate"

 //Local only
void Insert_tResourceImport(void);
void Update_tResourceImport(char *cRowid);
void ProcesstResourceImportListVars(pentry entries[], int x);

 //In tResourceImportfunc.h file included below
void ExtProcesstResourceImportVars(pentry entries[], int x);
void ExttResourceImportCommands(pentry entries[], int x);
void ExttResourceImportButtons(void);
void ExttResourceImportNavBar(void);
void ExttResourceImportGetHook(entry gentries[], int x);
void ExttResourceImportSelect(void);
void ExttResourceImportSelectRow(void);
void ExttResourceImportListSelect(void);
void ExttResourceImportListFilter(void);
void ExttResourceImportAuxTable(void);

#include "tresourceimportfunc.h"

 //Table Variables Assignment Function
void ProcesstResourceImportVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uResource"))
			sscanf(entries[i].val,"%u",&uResource);
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
	ExtProcesstResourceImportVars(entries,x);

}//ProcesstResourceImportVars()


void ProcesstResourceImportListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uResource);
                        guMode=2002;
                        tResourceImport("");
                }
        }
}//void ProcesstResourceImportListVars(pentry entries[], int x)


int tResourceImportCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttResourceImportCommands(entries,x);

	if(!strcmp(gcFunction,"tResourceImportTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tResourceImportList();
		}

		//Default
		ProcesstResourceImportVars(entries,x);
		tResourceImport("");
	}
	else if(!strcmp(gcFunction,"tResourceImportList"))
	{
		ProcessControlVars(entries,x);
		ProcesstResourceImportListVars(entries,x);
		tResourceImportList();
	}

	return(0);

}//tResourceImportCommands()


void tResourceImport(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttResourceImportSelectRow();
		else
			ExttResourceImportSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetResourceImport();
				iDNS("New tResourceImport table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tResourceImport WHERE uResource=%u"
						,uResource);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uResource);
		sscanf(field[1],"%u",&uZone);
		sprintf(cName,"%.100s",field[2]);
		sscanf(field[3],"%u",&uTTL);
		sscanf(field[4],"%u",&uRRType);
		sprintf(cParam1,"%.255s",field[5]);
		sprintf(cParam2,"%.255s",field[6]);
		cComment=field[7];
		sscanf(field[8],"%u",&uOwner);
		sscanf(field[9],"%u",&uCreatedBy);
		sscanf(field[10],"%lu",&uCreatedDate);
		sscanf(field[11],"%u",&uModBy);
		sscanf(field[12],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tResourceImport",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tResourceImportTools>");
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

        ExttResourceImportButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tResourceImport Record Data",100);

	if(guMode==2000 || guMode==2002)
		tResourceImportInput(1);
	else
		tResourceImportInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttResourceImportAuxTable();

	Footer_ism3();

}//end of tResourceImport();


void tResourceImportInput(unsigned uMode)
{

//uResource
	OpenRow(LANG_FL_tResourceImport_uResource,"black");
	printf("<input title='%s' type=text name=uResource value=%u size=16 maxlength=10 "
,LANG_FT_tResourceImport_uResource,uResource);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uResource value=%u >\n",uResource);
	}
//uZone
	OpenRow(LANG_FL_tResourceImport_uZone,IsZero(uZone));
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uZone value=%u >\n",ForeignKey("tZoneImport","cZone",uZone),uZone);
	}
	else
	{
	printf("%s<input type=hidden name=uZone value=%u >\n",ForeignKey("tZoneImport","cZone",uZone),uZone);
	}
//cName
	OpenRow(LANG_FL_tResourceImport_cName,"black");
	printf("<input title='%s' type=text name=cName value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tResourceImport_cName,EncodeDoubleQuotes(cName));
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
	OpenRow(LANG_FL_tResourceImport_uTTL,"black");
	printf("<input title='%s' type=text name=uTTL value=%u size=16 maxlength=10 "
,LANG_FT_tResourceImport_uTTL,uTTL);
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
	OpenRow(LANG_FL_tResourceImport_uRRType,IsZero(uRRType));
	if(guPermLevel>=0 && uMode)
		tTablePullDown("tRRType;cuRRTypePullDown","cLabel","cLabel",uRRType,1);
	else
		tTablePullDown("tRRType;cuRRTypePullDown","cLabel","cLabel",uRRType,0);
//cParam1
	OpenRow(LANG_FL_tResourceImport_cParam1,EmptyString(cParam1));
	printf("<input title='%s' type=text name=cParam1 value=\"%s\" size=80 maxlength=255 "
,LANG_FT_tResourceImport_cParam1,EncodeDoubleQuotes(cParam1));
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
	OpenRow(LANG_FL_tResourceImport_cParam2,"black");
	printf("<input title='%s' type=text name=cParam2 value=\"%s\" size=80 maxlength=255 "
,LANG_FT_tResourceImport_cParam2,EncodeDoubleQuotes(cParam2));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cParam2 value=\"%s\">\n",EncodeDoubleQuotes(cParam2));
	}
//cComment
	OpenRow(LANG_FL_tResourceImport_cComment,"black");
	printf("<textarea title='%s' cols=40 wrap=hard rows=3 name=cComment "
,LANG_FT_tResourceImport_cComment);
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
	OpenRow(LANG_FL_tResourceImport_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tResourceImport_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tResourceImport_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tResourceImport_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tResourceImport_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tResourceImportInput(unsigned uMode)


void NewtResourceImport(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uResource FROM tResourceImport\
				WHERE uResource=%u"
							,uResource);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tResourceImport("<blink>Record already exists");
		tResourceImport(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tResourceImport();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uResource=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tResourceImport",uResource);
	iDNSLog(uResource,"tResourceImport","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uResource);
	tResourceImport(gcQuery);
	}

}//NewtResourceImport(unsigned uMode)


void DeletetResourceImport(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tResourceImport WHERE uResource=%u AND ( uOwner=%u OR %u>9 )"
					,uResource,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tResourceImport WHERE uResource=%u"
					,uResource);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tResourceImport("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uResource,"tResourceImport","Del");
#endif
		tResourceImport(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uResource,"tResourceImport","DelError");
#endif
		tResourceImport(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetResourceImport(void)


void Insert_tResourceImport(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tResourceImport SET uResource=%u,uZone=%u,cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',cParam2='%s',cComment='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uResource
			,uZone
			,TextAreaSave(cName)
			,uTTL
			,uRRType
			,TextAreaSave(cParam1)
			,TextAreaSave(cParam2)
			,TextAreaSave(cComment)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tResourceImport(void)


void Update_tResourceImport(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tResourceImport SET uResource=%u,uZone=%u,cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',cParam2='%s',cComment='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uResource
			,uZone
			,TextAreaSave(cName)
			,uTTL
			,uRRType
			,TextAreaSave(cParam1)
			,TextAreaSave(cParam2)
			,TextAreaSave(cComment)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tResourceImport(void)


void ModtResourceImport(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uResource,uModDate FROM tResourceImport WHERE uResource=%u"
			,uResource);
#else
	sprintf(gcQuery,"SELECT uResource FROM tResourceImport WHERE uResource=%u"
			,uResource);
#endif

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tResourceImport("<blink>Record does not exist");
	if(i<1) tResourceImport(LANG_NBR_RECNOTEXIST);
	//if(i>1) tResourceImport("<blink>Multiple rows!");
	if(i>1) tResourceImport(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tResourceImport(LANG_NBR_EXTMOD);
#endif

	Update_tResourceImport(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tResourceImport",uResource);
	iDNSLog(uResource,"tResourceImport","Mod");
#endif
	tResourceImport(gcQuery);

}//ModtResourceImport(void)


void tResourceImportList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttResourceImportListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tResourceImportList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttResourceImportListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uResource<td><font face=arial,helvetica color=white>uZone<td><font face=arial,helvetica color=white>cName<td><font face=arial,helvetica color=white>uTTL<td><font face=arial,helvetica color=white>uRRType<td><font face=arial,helvetica color=white>cParam1<td><font face=arial,helvetica color=white>cParam2<td><font face=arial,helvetica color=white>cComment<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td><textarea disabled>%s</textarea><td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
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

}//tResourceImportList()


void CreatetResourceImport(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tResourceImport ( uResource INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cName VARCHAR(100) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, uTTL INT UNSIGNED NOT NULL DEFAULT 0, uRRType INT UNSIGNED NOT NULL DEFAULT 0, cParam1 VARCHAR(255) NOT NULL DEFAULT '', cParam2 VARCHAR(255) NOT NULL DEFAULT '', cComment TEXT NOT NULL DEFAULT '', uZone INT UNSIGNED NOT NULL DEFAULT 0,index (uZone), cParam3 VARCHAR(255) NOT NULL DEFAULT '', cParam4 VARCHAR(255) NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetResourceImport()

