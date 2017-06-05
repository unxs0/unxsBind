/*
FILE
	tView source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Zone view table. E.g. internal/external etc.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uView: Primary Key
static unsigned uView=0;
//cLabel: Short label
static char cLabel[33]={""};
//uOrder: Order in zone file
static unsigned uOrder=0;
//uNSSet: Name Server Set
static unsigned uNSSet=0;
static char cuNSSetPullDown[256]={""};
//cMaster: BIND configuration master.zones view section header
static char *cMaster={""};
//cSlave: Bind configuration slave.zones view section header
static char *cSlave={""};
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



#define VAR_LIST_tView "tView.uView,tView.cLabel,tView.uOrder,tView.uNSSet,tView.cMaster,tView.cSlave,tView.uOwner,tView.uCreatedBy,tView.uCreatedDate,tView.uModBy,tView.uModDate"

 //Local only
void Insert_tView(void);
void Update_tView(char *cRowid);
void ProcesstViewListVars(pentry entries[], int x);

 //In tViewfunc.h file included below
void ExtProcesstViewVars(pentry entries[], int x);
void ExttViewCommands(pentry entries[], int x);
void ExttViewButtons(void);
void ExttViewNavBar(void);
void ExttViewGetHook(entry gentries[], int x);
void ExttViewSelect(void);
void ExttViewSelectRow(void);
void ExttViewListSelect(void);
void ExttViewListFilter(void);
void ExttViewAuxTable(void);

#include "tviewfunc.h"

 //Table Variables Assignment Function
void ProcesstViewVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uView"))
			sscanf(entries[i].val,"%u",&uView);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"uOrder"))
			sscanf(entries[i].val,"%u",&uOrder);
		else if(!strcmp(entries[i].name,"uNSSet"))
			sscanf(entries[i].val,"%u",&uNSSet);
		else if(!strcmp(entries[i].name,"cuNSSetPullDown"))
		{
			sprintf(cuNSSetPullDown,"%.255s",entries[i].val);
			uNSSet=ReadPullDown("tNSSet","cLabel",cuNSSetPullDown);
		}
		else if(!strcmp(entries[i].name,"cMaster"))
			cMaster=entries[i].val;
		else if(!strcmp(entries[i].name,"cSlave"))
			cSlave=entries[i].val;
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
	ExtProcesstViewVars(entries,x);

}//ProcesstViewVars()


void ProcesstViewListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uView);
                        guMode=2002;
                        tView("");
                }
        }
}//void ProcesstViewListVars(pentry entries[], int x)


int tViewCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttViewCommands(entries,x);

	if(!strcmp(gcFunction,"tViewTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tViewList();
		}

		//Default
		ProcesstViewVars(entries,x);
		tView("");
	}
	else if(!strcmp(gcFunction,"tViewList"))
	{
		ProcessControlVars(entries,x);
		ProcesstViewListVars(entries,x);
		tViewList();
	}

	return(0);

}//tViewCommands()


void tView(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttViewSelectRow();
		else
			ExttViewSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetView();
				iDNS("New tView table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tView WHERE uView=%u"
						,uView);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uView);
		sprintf(cLabel,"%.32s",field[1]);
		sscanf(field[2],"%u",&uOrder);
		sscanf(field[3],"%u",&uNSSet);
		cMaster=field[4];
		cSlave=field[5];
		sscanf(field[6],"%u",&uOwner);
		sscanf(field[7],"%u",&uCreatedBy);
		sscanf(field[8],"%lu",&uCreatedDate);
		sscanf(field[9],"%u",&uModBy);
		sscanf(field[10],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tView",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tViewTools>");
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

        ExttViewButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tView Record Data",100);

	if(guMode==2000 || guMode==2002)
		tViewInput(1);
	else
		tViewInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttViewAuxTable();

	Footer_ism3();

}//end of tView();


void tViewInput(unsigned uMode)
{

//uView
	OpenRow(LANG_FL_tView_uView,"black");
	printf("<input title='%s' type=text name=uView value=%u size=16 maxlength=10 "
,LANG_FT_tView_uView,uView);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uView value=%u >\n",uView);
	}
//cLabel
	OpenRow(LANG_FL_tView_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tView_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//uOrder
	OpenRow(LANG_FL_tView_uOrder,"black");
	printf("<input title='%s' type=text name=uOrder value=%u size=16 maxlength=10 "
,LANG_FT_tView_uOrder,uOrder);
	if(guPermLevel>=10 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uOrder value=%u >\n",uOrder);
	}
//uNSSet
	OpenRow(LANG_FL_tView_uNSSet,"black");
	if(guPermLevel>=10 && uMode)
		tTablePullDown("tNSSet;cuNSSetPullDown","cLabel","cLabel",uNSSet,1);
	else
		tTablePullDown("tNSSet;cuNSSetPullDown","cLabel","cLabel",uNSSet,0);
//cMaster
	OpenRow(LANG_FL_tView_cMaster,"black");
	printf("<textarea title='%s' cols=80 wrap=hard rows=8 name=cMaster "
,LANG_FT_tView_cMaster);
	if(guPermLevel>=0 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",cMaster);
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",cMaster);
		printf("<input type=hidden name=cMaster value=\"%s\" >\n",EncodeDoubleQuotes(cMaster));
	}
//cSlave
	OpenRow(LANG_FL_tView_cSlave,"black");
	printf("<textarea title='%s' cols=80 wrap=hard rows=8 name=cSlave "
,LANG_FT_tView_cSlave);
	if(guPermLevel>=0 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",cSlave);
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",cSlave);
		printf("<input type=hidden name=cSlave value=\"%s\" >\n",EncodeDoubleQuotes(cSlave));
	}
//uOwner
	OpenRow(LANG_FL_tView_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tView_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tView_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tView_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tView_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tViewInput(unsigned uMode)


void NewtView(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uView FROM tView\
				WHERE uView=%u"
							,uView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tView("<blink>Record already exists");
		tView(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tView();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uView=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tView",uView);
	iDNSLog(uView,"tView","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uView);
	tView(gcQuery);
	}

}//NewtView(unsigned uMode)


void DeletetView(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tView WHERE uView=%u AND ( uOwner=%u OR %u>9 )"
					,uView,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tView WHERE uView=%u"
					,uView);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tView("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uView,"tView","Del");
#endif
		tView(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uView,"tView","DelError");
#endif
		tView(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetView(void)


void Insert_tView(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tView SET uView=%u,cLabel='%s',uOrder=%u,uNSSet=%u,cMaster='%s',cSlave='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uView
			,TextAreaSave(cLabel)
			,uOrder
			,uNSSet
			,TextAreaSave(cMaster)
			,TextAreaSave(cSlave)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tView(void)


void Update_tView(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tView SET uView=%u,cLabel='%s',uOrder=%u,uNSSet=%u,cMaster='%s',cSlave='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uView
			,TextAreaSave(cLabel)
			,uOrder
			,uNSSet
			,TextAreaSave(cMaster)
			,TextAreaSave(cSlave)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tView(void)


void ModtView(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uView,uModDate FROM tView WHERE uView=%u"
			,uView);
#else
	sprintf(gcQuery,"SELECT uView FROM tView WHERE uView=%u"
			,uView);
#endif

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tView("<blink>Record does not exist");
	if(i<1) tView(LANG_NBR_RECNOTEXIST);
	//if(i>1) tView("<blink>Multiple rows!");
	if(i>1) tView(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tView(LANG_NBR_EXTMOD);
#endif

	Update_tView(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tView",uView);
	iDNSLog(uView,"tView","Mod");
#endif
	tView(gcQuery);

}//ModtView(void)


void tViewList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttViewListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tViewList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttViewListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uView<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>uOrder<td><font face=arial,helvetica color=white>uNSSet<td><font face=arial,helvetica color=white>cMaster<td><font face=arial,helvetica color=white>cSlave<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td><textarea disabled>%s</textarea><td><textarea disabled>%s</textarea><td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,field[2]
			,ForeignKey("tNSSet","cLabel",strtoul(field[3],NULL,10))
			,field[4]
			,field[5]
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[6],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[7],NULL,10))
			,cBuf8
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[9],NULL,10))
			,cBuf10
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tViewList()


void CreatetView(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tView ( uView INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(32) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, cMaster TEXT NOT NULL DEFAULT '', cSlave TEXT NOT NULL DEFAULT '', uOrder INT UNSIGNED NOT NULL DEFAULT 0, uNSSet INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uNSSet) )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetView()

