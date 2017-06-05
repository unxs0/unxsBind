/*
FILE
	tGroupGlue source code of unxsVZ.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis 2001-2007
PURPOSE
	Group properties glue table.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uGroupGlue: Primary Key
static unsigned uGroupGlue=0;
//uGroup: Glue into tGroup
static unsigned uGroup=0;
static char cuGroupPullDown[256]={""};
//uZone: Glue into tZone
static unsigned uZone=0;
static char cuZonePullDown[256]={""};
//uResource: Glue into tResource
static unsigned uResource=0;
static char cuResourcePullDown[256]={""};



#define VAR_LIST_tGroupGlue "tGroupGlue.uGroupGlue,tGroupGlue.uGroup,tGroupGlue.uZone,tGroupGlue.uResource"

 //Local only
void Insert_tGroupGlue(void);
void Update_tGroupGlue(char *cRowid);
void ProcesstGroupGlueListVars(pentry entries[], int x);

 //In tGroupGluefunc.h file included below
void ExtProcesstGroupGlueVars(pentry entries[], int x);
void ExttGroupGlueCommands(pentry entries[], int x);
void ExttGroupGlueButtons(void);
void ExttGroupGlueNavBar(void);
void ExttGroupGlueGetHook(entry gentries[], int x);
void ExttGroupGlueSelect(void);
void ExttGroupGlueSelectRow(void);
void ExttGroupGlueListSelect(void);
void ExttGroupGlueListFilter(void);
void ExttGroupGlueAuxTable(void);

#include "tgroupgluefunc.h"

 //Table Variables Assignment Function
void ProcesstGroupGlueVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uGroupGlue"))
			sscanf(entries[i].val,"%u",&uGroupGlue);
		else if(!strcmp(entries[i].name,"uGroup"))
			sscanf(entries[i].val,"%u",&uGroup);
		else if(!strcmp(entries[i].name,"cuGroupPullDown"))
		{
			sprintf(cuGroupPullDown,"%.255s",entries[i].val);
			uGroup=ReadPullDown("tGroup","cLabel",cuGroupPullDown);
		}
		else if(!strcmp(entries[i].name,"uZone"))
			sscanf(entries[i].val,"%u",&uZone);
		else if(!strcmp(entries[i].name,"cuZonePullDown"))
		{
			sprintf(cuZonePullDown,"%.255s",entries[i].val);
			uZone=ReadPullDown("tZone","cLabel",cuZonePullDown);
		}
		else if(!strcmp(entries[i].name,"uResource"))
			sscanf(entries[i].val,"%u",&uResource);
		else if(!strcmp(entries[i].name,"cuResourcePullDown"))
		{
			sprintf(cuResourcePullDown,"%.255s",entries[i].val);
			uResource=ReadPullDown("tResource","cLabel",cuResourcePullDown);
		}

	}

	//After so we can overwrite form data if needed.
	ExtProcesstGroupGlueVars(entries,x);

}//ProcesstGroupGlueVars()


void ProcesstGroupGlueListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uGroupGlue);
                        guMode=2002;
                        tGroupGlue("");
                }
        }
}//void ProcesstGroupGlueListVars(pentry entries[], int x)


int tGroupGlueCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttGroupGlueCommands(entries,x);

	if(!strcmp(gcFunction,"tGroupGlueTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tGroupGlueList();
		}

		//Default
		ProcesstGroupGlueVars(entries,x);
		tGroupGlue("");
	}
	else if(!strcmp(gcFunction,"tGroupGlueList"))
	{
		ProcessControlVars(entries,x);
		ProcesstGroupGlueListVars(entries,x);
		tGroupGlueList();
	}

	return(0);

}//tGroupGlueCommands()


void tGroupGlue(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttGroupGlueSelectRow();
		else
			ExttGroupGlueSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetGroupGlue();
				iDNS("New tGroupGlue table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tGroupGlue WHERE uGroupGlue=%u"
						,uGroupGlue);
				MYSQL_RUN_STORE(res2);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uGroupGlue);
		sscanf(field[1],"%u",&uGroup);
		sscanf(field[2],"%u",&uZone);
		sscanf(field[3],"%u",&uResource);

		}

	}//Internal Skip

	Header_ism3(":: Glues uResources or uZones to uGroups",0);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tGroupGlueTools>");
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

        ExttGroupGlueButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tGroupGlue Record Data",100);

	if(guMode==2000 || guMode==2002)
		tGroupGlueInput(1);
	else
		tGroupGlueInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttGroupGlueAuxTable();

	Footer_ism3();

}//end of tGroupGlue();


void tGroupGlueInput(unsigned uMode)
{

//uGroupGlue
	OpenRow(LANG_FL_tGroupGlue_uGroupGlue,"black");
	printf("<input title='%s' type=text name=uGroupGlue value=%u size=16 maxlength=10 ",LANG_FT_tGroupGlue_uGroupGlue,uGroupGlue);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uGroupGlue value=%u >\n",uGroupGlue);
	}
//uGroup
	OpenRow(LANG_FL_tGroupGlue_uGroup,"black");
	if(guPermLevel>=10 && uMode)
		tTablePullDownOwner("tGroup;cuGroupPullDown","cLabel","cLabel",uGroup,1);
	else
		tTablePullDownOwner("tGroup;cuGroupPullDown","cLabel","cLabel",uGroup,0);
//uZone
	OpenRow("uZone","black");
	if(guPermLevel>=10 && uMode)
	{
		printf("<input title='uZone' type=text name=uZone value=%u size=16 maxlength=10 ",uZone);
		printf("></td></tr>\n");
	}
	else
	{
		printf("%s</td></tr>\n",ForeignKey("tZone","cZone",uZone));
		printf("<input type=hidden name=uGroupGlue value=%u >\n",uZone);
	}
//uResource
	OpenRow("uResource","black");
	if(guPermLevel>=10 && uMode)
	{
		printf("<input title='uResource' type=text name=uResource value=%u size=16 maxlength=10 ",uResource);
		printf("></td></tr>\n");
	}
	else
	{
		printf("%s</td></tr>\n",ForeignKey("tResource","cName",uResource));
		printf("<input type=hidden name=uResource value=%u >\n",uResource);
	}
	printf("</tr>\n");



}//void tGroupGlueInput(unsigned uMode)


void NewtGroupGlue(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uGroupGlue FROM tGroupGlue WHERE uGroupGlue=%u",uGroupGlue);
	MYSQL_RUN_STORE(res);
	i=mysql_num_rows(res);

	if(i) 
		//tGroupGlue("<blink>Record already exists");
		tGroupGlue(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tGroupGlue();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uGroupGlue=mysql_insert_id(&gMysql);

	if(!uMode)
	{
		sprintf(gcQuery,LANG_NBR_NEWRECADDED,uGroupGlue);
		tGroupGlue(gcQuery);
	}

}//NewtGroupGlue(unsigned uMode)


void DeletetGroupGlue(void)
{
	sprintf(gcQuery,"DELETE FROM tGroupGlue WHERE uGroupGlue=%u",uGroupGlue);
	MYSQL_RUN;
	//tGroupGlue("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
		tGroupGlue(LANG_NBR_RECDELETED);
	else
		tGroupGlue(LANG_NBR_RECNOTDELETED);
}//void DeletetGroupGlue(void)


void Insert_tGroupGlue(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tGroupGlue SET uGroupGlue=%u,uGroup=%u,uZone=%u,uResource=%u",
			uGroupGlue
			,uGroup
			,uZone
			,uResource
			);
	MYSQL_RUN;

}//void Insert_tGroupGlue(void)


void Update_tGroupGlue(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tGroupGlue SET uGroupGlue=%u,uGroup=%u,uZone=%u,uResource=%u WHERE _rowid=%s",
			uGroupGlue
			,uGroup
			,uZone
			,uResource
			,cRowid);
	MYSQL_RUN;

}//void Update_tGroupGlue(void)


void ModtGroupGlue(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
	sprintf(gcQuery,"SELECT uGroupGlue FROM tGroupGlue WHERE uGroupGlue=%u",uGroupGlue);

	MYSQL_RUN_STORE(res);
	i=mysql_num_rows(res);

	//if(i<1) tGroupGlue("<blink>Record does not exist");
	if(i<1) tGroupGlue(LANG_NBR_RECNOTEXIST);
	//if(i>1) tGroupGlue("<blink>Multiple rows!");
	if(i>1) tGroupGlue(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
	Update_tGroupGlue(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
	tGroupGlue(gcQuery);

}//ModtGroupGlue(void)


void tGroupGlueList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttGroupGlueListSelect();

	MYSQL_RUN_STORE(res);
	guI=mysql_num_rows(res);

	PageMachine("tGroupGlueList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttGroupGlueListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black>"
			"<td><font face=arial color=white>uGroupGlue"
			"<td><font face=arial color=white>uGroup"
			"<td><font face=arial color=white>uZone"
			"<td><font face=arial color=white>uResource</tr>");



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
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,ForeignKey("tGroup","cLabel",strtoul(field[1],NULL,10))
			,ForeignKey("tZone","cLabel",strtoul(field[2],NULL,10))
			,ForeignKey("tResource","cLabel",strtoul(field[3],NULL,10))
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tGroupGlueList()


void CreatetGroupGlue(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tGroupGlue ("
			" uGroup INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uGroup),"
			" uZone INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uZone),"
			" uResource INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uResource),"
			" uGroupGlue INT UNSIGNED PRIMARY KEY AUTO_INCREMENT )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetGroupGlue()

