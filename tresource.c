/*
FILE
	tResource source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Zone db resource records. Excluding zone SOA records that we save via tZone.
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



#define VAR_LIST_tResource "tResource.uResource,tResource.uZone,tResource.cName,tResource.uTTL,tResource.uRRType,tResource.cParam1,tResource.cParam2,tResource.cParam3,tResource.cParam4,tResource.cComment,tResource.uOwner,tResource.uCreatedBy,tResource.uCreatedDate,tResource.uModBy,tResource.uModDate"

 //Local only
void Insert_tResource(void);
void Update_tResource(char *cRowid);
void ProcesstResourceListVars(pentry entries[], int x);
static char cZoneSearch[64]={""};
static char cNameSearch[64]={""};
static char cParam1Search[64]={""};
static char cParam2Search[64]={""};
static char cParam3Search[64]={""};
static char cParam4Search[64]={""};
static char cCommentSearch[64]={""};
static unsigned uForClient=0;
static char cForClientPullDown[256]={""};
static unsigned uView=0;
static char cuViewPullDown[256]={""};
void tResourceSearchSet(unsigned uStep);

 //In tResourcefunc.h file included below
void ExtProcesstResourceVars(pentry entries[], int x);
void ExttResourceCommands(pentry entries[], int x);
void ExttResourceButtons(void);
void ExttResourceNavBar(void);
void ExttResourceGetHook(entry gentries[], int x);
void ExttResourceSelect(void);
void ExttResourceSelectRow(void);
void ExttResourceListSelect(void);
void ExttResourceListFilter(void);
void ExttResourceAuxTable(void);

#include "tresourcefunc.h"

 //Table Variables Assignment Function
void ProcesstResourceVars(pentry entries[], int x)
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
		else if(!strcmp(entries[i].name,"cParam3"))
			sprintf(cParam3,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam4"))
			sprintf(cParam4,"%.255s",entries[i].val);
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
		else if(!strcmp(entries[i].name,"cNameSearch"))
			sprintf(cNameSearch,"%.63s",entries[i].val);
		else if(!strcmp(entries[i].name,"cZoneSearch"))
			sprintf(cZoneSearch,"%.63s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam1Search"))
			sprintf(cParam1Search,"%.63s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam2Search"))
			sprintf(cParam2Search,"%.63s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam3Search"))
			sprintf(cParam3Search,"%.63s",entries[i].val);
		else if(!strcmp(entries[i].name,"cParam4Search"))
			sprintf(cParam4Search,"%.63s",entries[i].val);
		else if(!strcmp(entries[i].name,"cCommentSearch"))
			sprintf(cCommentSearch,"%.63s",entries[i].val);
		else if(!strcmp(entries[i].name,"cForClientPullDown"))
		{
			strcpy(cForClientPullDown,entries[i].val);
			uForClient=ReadPullDown(TCLIENT,"cLabel",cForClientPullDown);
		}
		else if(!strcmp(entries[i].name,"cuViewPullDown"))
		{
			sprintf(cuViewPullDown,"%.255s",entries[i].val);
			uView=ReadPullDown("tView","cLabel",cuViewPullDown);
		}

	}

	//After so we can overwrite form data if needed.
	ExtProcesstResourceVars(entries,x);

}//ProcesstResourceVars()


void ProcesstResourceListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uResource);
                        guMode=2002;
                        tResource("");
                }
        }
}//void ProcesstResourceListVars(pentry entries[], int x)


int tResourceCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttResourceCommands(entries,x);

	if(!strcmp(gcFunction,"tResourceTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tResourceList();
		}

		//Default
		ProcesstResourceVars(entries,x);
		tResource("");
	}
	else if(!strcmp(gcFunction,"tResourceList"))
	{
		ProcessControlVars(entries,x);
		ProcesstResourceListVars(entries,x);
		tResourceList();
	}

	return(0);

}//tResourceCommands()


void tResource(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttResourceSelectRow();
		else
			ExttResourceSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetResource();
				iDNS("New tResource table created");
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
				sprintf(gcQuery,"SELECT _rowid FROM tResource WHERE uResource=%u"
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

	Header_ism3(":: tResource",2);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tResourceTools>");
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

        ExttResourceButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tResource Record Data",100);

	//Custom right panel for creating search sets
	if(guMode==12001)
		tResourceSearchSet(1);
	else if(guMode==12002)
		tResourceSearchSet(2);
	else if(guMode==2000 || guMode==2002)
		tResourceInput(1);
	else if(1)
		tResourceInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttResourceAuxTable();

	Footer_ism3();

}//end of tResource();


void tResourceSearchSet(unsigned uStep)
{
	printf("<tr><td><u>Set search parameters</u></td></tr>");

	OpenRow("cZone pattern","black");
	//Usability: Transfer from main tContainer page any current search pattern
	if(cSearch[0])
		sprintf(cZoneSearch,"%.31s",cSearch);
	printf("<input title='SQL search pattern %% and _ allowed' type=text name=cZoneSearch"
			" value=\"%s\" size=40 maxlength=63 >",cZoneSearch);

	OpenRow("NSSet","black");
	tTablePullDown("tNSSet;cuNSSetPullDown","cLabel","cLabel",uSelectNSSet,1);

	OpenRow("View","black");
	tTablePullDown("tView;cuViewPullDown","cLabel","cLabel",uView,1);

	OpenRow("cName pattern","black");
	printf("<input title='SQL search pattern %% and _ allowed' type=text name=cNameSearch"
			" value=\"%s\" size=40 maxlength=63 >",cNameSearch);

	OpenRow("RRType","black");
	tTablePullDown("tRRType;cuRRTypePullDown","cLabel","cLabel",uRRType,1);

	OpenRow("cParam1 pattern","black");
	printf("<input title='SQL search pattern %% and _ allowed' type=text name=cParam1Search"
			" value=\"%s\" size=40 maxlength=63 >",cParam1Search);

	OpenRow("cParam2 pattern","black");
	printf("<input title='SQL search pattern %% and _ allowed' type=text name=cParam2Search"
			" value=\"%s\" size=40 maxlength=63 >",cParam2Search);

	OpenRow("cParam3 pattern","black");
	printf("<input title='SQL search pattern %% and _ allowed' type=text name=cParam3Search"
			" value=\"%s\" size=40 maxlength=63 >",cParam3Search);

	OpenRow("cParam4 pattern","black");
	printf("<input title='SQL search pattern %% and _ allowed' type=text name=cParam4Search"
			" value=\"%s\" size=40 maxlength=63 >",cParam4Search);

	OpenRow("cComment pattern","black");
	printf("<input title='SQL search pattern %% and _ allowed' type=text name=cCommentSearch"
			" value=\"%s\" size=40 maxlength=63 >",cCommentSearch);

	OpenRow("Owner","black");
	tTablePullDownResellers(uForClient,1);

	if(uStep==1)
	{
		;
	}
	else if(uStep==2)
	{
		;
	}

}//void tResourceSearchSet(unsigned uStep)


void tResourceInput(unsigned uMode)
{

//uResource
	OpenRow(LANG_FL_tResource_uResource,"black");
	printf("<input title='%s' type=text name=uResource value=%u size=16 maxlength=10 "
		,LANG_FT_tResource_uResource,uResource);
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
	OpenRow(LANG_FL_tResource_uZone,IsZero(uZone));
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uZone value=%u >\n",ForeignKey("tZone","cZone",uZone),uZone);
	}
	else
	{
	printf("%s<input type=hidden name=uZone value=%u >\n",ForeignKey("tZone","cZone",uZone),uZone);
	}
//cName
	OpenRow(LANG_FL_tResource_cName,"black");
	printf("<input title='%s' type=text name=cName value=\"%s\" size=40 maxlength=100 ",
			LANG_FT_tResource_cName,EncodeDoubleQuotes(cName));
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
	OpenRow(LANG_FL_tResource_uTTL,"black");
	printf("<input title='%s' type=text name=uTTL value=%u size=16 maxlength=10 "
			,LANG_FT_tResource_uTTL,uTTL);
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
	OpenRow(LANG_FL_tResource_uRRType,IsZero(uRRType));
	if(guPermLevel>=0 && uMode)
		tTablePullDown("tRRType;cuRRTypePullDown","cLabel","cLabel",uRRType,1);
	else
		tTablePullDown("tRRType;cuRRTypePullDown","cLabel","cLabel",uRRType,0);
//cParam1
	OpenRow(LANG_FL_tResource_cParam1,EmptyString(cParam1));
	printf("<input title='%s' type=text name=cParam1 value=\"%s\" size=80 maxlength=255 "
		,LANG_FT_tResource_cParam1,EncodeDoubleQuotes(cParam1));
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
	OpenRow(LANG_FL_tResource_cParam2,"black");
	printf("<input title='%s' type=text name=cParam2 value=\"%s\" size=80 maxlength=255 "
		,LANG_FT_tResource_cParam2,EncodeDoubleQuotes(cParam2));
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
	printf("<textarea title='%s' cols=77 wrap=soft rows=3 name=cParam3 "
		,LANG_FT_tResource_cParam3);
	if(guPermLevel>=0 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",cParam3);
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",cParam3);
		printf("<input type=hidden name=cParam3 value=\"%s\" >\n",EncodeDoubleQuotes(cParam3));
	}
//cParam4
	OpenRow(LANG_FL_tResource_cParam4,"black");
	printf("<textarea title='%s' cols=77 wrap=soft rows=3 name=cParam4 "
		,LANG_FT_tResource_cParam4);
	if(guPermLevel>=0 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",cParam4);
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",cParam4);
		printf("<input type=hidden name=cParam4 value=\"%s\" >\n",EncodeDoubleQuotes(cParam4));
	}
//cComment
	OpenRow(LANG_FL_tResource_cComment,"black");
	printf("<textarea title='%s' cols=40 wrap=hard rows=3 name=cComment "
		,LANG_FT_tResource_cComment);
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
	OpenRow(LANG_FL_tResource_uOwner,"black");
	if(guPermLevel>=20 && uMode)
		printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	else
		printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
//uCreatedBy
	OpenRow(LANG_FL_tResource_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
		printf("%s<input type=hidden name=uCreatedBy value=%u >\n",
				ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	else
		printf("%s<input type=hidden name=uCreatedBy value=%u >\n",
				ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
//uCreatedDate
	OpenRow(LANG_FL_tResource_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tResource_uModBy,"black");
	if(guPermLevel>=20 && uMode)
		printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	else
		printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
//uModDate
	OpenRow(LANG_FL_tResource_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tResourceInput(unsigned uMode)


void NewtResource(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uResource FROM tResource WHERE uResource=%u",uResource);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tResource("<blink>Record already exists");
		tResource(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tResource();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uResource=mysql_insert_id(&gMysql);
	uCreatedDate=luGetCreatedDate("tResource",uResource);
	iDNSLog(uResource,"tResource","New");

	if(!uMode)
	{
		sprintf(gcQuery,LANG_NBR_NEWRECADDED,uResource);
		tResource(gcQuery);
	}

}//NewtResource(unsigned uMode)


void DeletetResource(void)
{
	sprintf(gcQuery,"DELETE FROM tResource WHERE uResource=%u AND ( uOwner=%u OR %u>9 )"
					,uResource,guLoginClient,guPermLevel);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tResource("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
		iDNSLog(uResource,"tResource","Del");
		tResource(LANG_NBR_RECDELETED);
	}
	else
	{
		iDNSLog(uResource,"tResource","DelError");
		tResource(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetResource(void)


void Insert_tResource(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tResource SET uResource=%u,uZone=%u,cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',cParam2='%s',cParam3='%s',cParam4='%s',cComment='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uResource
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

}//void Insert_tResource(void)


void Update_tResource(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tResource SET uResource=%u,uZone=%u,cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',cParam2='%s',cParam3='%s',cParam4='%s',cComment='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uResource
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

}//void Update_tResource(void)


void ModtResource(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uResource,uModDate FROM tResource WHERE uResource=%u",uResource);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tResource("<blink>Record does not exist");
	if(i<1) tResource(LANG_NBR_RECNOTEXIST);
	//if(i>1) tResource("<blink>Multiple rows!");
	if(i>1) tResource(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tResource(LANG_NBR_EXTMOD);
	Update_tResource(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
	uModDate=luGetModDate("tResource",uResource);
	iDNSLog(uResource,"tResource","Mod");
	tResource(gcQuery);

}//ModtResource(void)


void tResourceList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttResourceListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tResourceList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttResourceListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uResource"
		"<td><font face=arial,helvetica color=white>uZone"
		"<td><font face=arial,helvetica color=white>cName"
		"<td><font face=arial,helvetica color=white>uTTL"
		"<td><font face=arial,helvetica color=white>uRRType"
		"<td><font face=arial,helvetica color=white>cParam1"
		"<td><font face=arial,helvetica color=white>cParam2"
		"<td><font face=arial,helvetica color=white>cParam3"
		"<td><font face=arial,helvetica color=white>cParam4"
		"<td><font face=arial,helvetica color=white>cComment"
		"<td><font face=arial,helvetica color=white>uOwner"
		"<td><font face=arial,helvetica color=white>uCreatedBy"
		"<td><font face=arial,helvetica color=white>uCreatedDate"
		"<td><font face=arial,helvetica color=white>uModBy"
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
		long luTime12=strtoul(field[12],NULL,10);
		char cBuf12[32];
		if(luTime12)
			ctime_r(&luTime12,cBuf12);
		else
			sprintf(cBuf12,"---");
		long luTime14=strtoul(field[14],NULL,10);
		char cBuf14[32];
		if(luTime14)
			ctime_r(&luTime14,cBuf14);
		else
			sprintf(cBuf14,"---");
		printf("<td><a class=darkLink href=?gcFunction=tResource&uResource=%s>%s</a>"
			"<td>%s"
			"<td>%s"
			"<td>%s"
			"<td>%s"
			"<td>%s"
			"<td>%s"
			"<td>%s"
			"<td>%s"
			"<td><textarea disabled>%s</textarea>"
			"<td>%s"
			"<td>%s"
			"<td>%s"
			"<td>%s"
			"<td>%s"
			"</tr>"
				,field[0],field[0]
				,ForeignKey("tZone","cZone",strtoul(field[1],NULL,10))
				,field[2]
				,field[3]
				,ForeignKey("tRRType","cLabel",strtoul(field[4],NULL,10))
				,field[5]
				,field[6]
				,field[7]
				,field[8]
				,field[9]
				,ForeignKey(TCLIENT,"cLabel",strtoul(field[10],NULL,10))
				,ForeignKey(TCLIENT,"cLabel",strtoul(field[11],NULL,10))
				,cBuf12
				,ForeignKey(TCLIENT,"cLabel",strtoul(field[13],NULL,10))
				,cBuf14);
	}

	printf("</table></form>\n");
	Footer_ism3();

}//tResourceList()


void CreatetResource(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tResource ("
			" uResource INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
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
}//CreatetResource()

