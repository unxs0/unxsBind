/*
FILE
	tNSSet source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis 2001-2009 for Unixservice
PURPOSE
	Name server set grouping.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uNSSet: Primary Key
static unsigned uNSSet=0;
//cLabel: Short label
static char cLabel[33]={""};
//cMasterIPs: Master IPs of a given set
static char cMasterIPs[256]={""};
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



#define VAR_LIST_tNSSet "tNSSet.uNSSet,tNSSet.cLabel,tNSSet.cMasterIPs,tNSSet.uOwner,tNSSet.uCreatedBy,tNSSet.uCreatedDate,tNSSet.uModBy,tNSSet.uModDate"

 //Local only
void Insert_tNSSet(void);
void Update_tNSSet(char *cRowid);
void ProcesstNSSetListVars(pentry entries[], int x);

 //In tNSSetfunc.h file included below
void ExtProcesstNSSetVars(pentry entries[], int x);
void ExttNSSetCommands(pentry entries[], int x);
void ExttNSSetButtons(void);
void ExttNSSetNavBar(void);
void ExttNSSetGetHook(entry gentries[], int x);
void ExttNSSetSelect(void);
void ExttNSSetSelectRow(void);
void ExttNSSetListSelect(void);
void ExttNSSetListFilter(void);
void ExttNSSetAuxTable(void);

#include "tnssetfunc.h"

 //Table Variables Assignment Function
void ProcesstNSSetVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uNSSet"))
			sscanf(entries[i].val,"%u",&uNSSet);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cMasterIPs"))
			sprintf(cMasterIPs,"%.255s",entries[i].val);
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
	ExtProcesstNSSetVars(entries,x);

}//ProcesstNSSetVars()


void ProcesstNSSetListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uNSSet);
                        guMode=2002;
                        tNSSet("");
                }
        }
}//void ProcesstNSSetListVars(pentry entries[], int x)


int tNSSetCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttNSSetCommands(entries,x);

	if(!strcmp(gcFunction,"tNSSetTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tNSSetList();
		}

		//Default
		ProcesstNSSetVars(entries,x);
		tNSSet("");
	}
	else if(!strcmp(gcFunction,"tNSSetList"))
	{
		ProcessControlVars(entries,x);
		ProcesstNSSetListVars(entries,x);
		tNSSetList();
	}

	return(0);

}//tNSSetCommands()


void tNSSet(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttNSSetSelectRow();
		else
			ExttNSSetSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetNSSet();
				iDNS("New tNSSet table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tNSSet WHERE uNSSet=%u"
						,uNSSet);
				macro_mySQLRunAndStore(res2);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uNSSet);
		sprintf(cLabel,"%.32s",field[1]);
		sprintf(cMasterIPs,"%.255s",field[2]);
		sscanf(field[3],"%u",&uOwner);
		sscanf(field[4],"%u",&uCreatedBy);
		sscanf(field[5],"%lu",&uCreatedDate);
		sscanf(field[6],"%u",&uModBy);
		sscanf(field[7],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tNSSet",0);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tNSSetTools>");
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

        ExttNSSetButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tNSSet Record Data",100);

	if(guMode==2000 || guMode==2002)
		tNSSetInput(1);
	else
		tNSSetInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttNSSetAuxTable();

	Footer_ism3();

}//end of tNSSet();


void tNSSetInput(unsigned uMode)
{

//uNSSet
	OpenRow(LANG_FL_tNSSet_uNSSet,"black");
	printf("<input title='%s' type=text name=uNSSet value=%u size=16 maxlength=10 "
,LANG_FT_tNSSet_uNSSet,uNSSet);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uNSSet value=%u >\n",uNSSet);
	}
//cLabel
	OpenRow(LANG_FL_tNSSet_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tNSSet_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//cMasterIPs
	OpenRow(LANG_FL_tNSSet_cMasterIPs,"black");
	printf("<input title='%s' type=text name=cMasterIPs value=\"%s\" size=40 maxlength=255 "
,LANG_FT_tNSSet_cMasterIPs,EncodeDoubleQuotes(cMasterIPs));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cMasterIPs value=\"%s\">\n",EncodeDoubleQuotes(cMasterIPs));
	}
//uOwner
	OpenRow(LANG_FL_tNSSet_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tNSSet_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tNSSet_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tNSSet_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tNSSet_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tNSSetInput(unsigned uMode)


void NewtNSSet(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uNSSet FROM tNSSet\
				WHERE uNSSet=%u"
							,uNSSet);
	macro_mySQLRunAndStore(res);
	i=mysql_num_rows(res);

	if(i) 
		//tNSSet("<blink>Record already exists");
		tNSSet(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tNSSet();
	//sprintf(gcQuery,"New record %u added");
	uNSSet=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tNSSet",uNSSet);
	iDNSLog(uNSSet,"tNSSet","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uNSSet);
	tNSSet(gcQuery);
	}

}//NewtNSSet(unsigned uMode)


void DeletetNSSet(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tNSSet WHERE uNSSet=%u AND ( uOwner=%u OR %u>9 )"
					,uNSSet,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tNSSet WHERE uNSSet=%u"
					,uNSSet);
#endif
	macro_mySQLQueryHTMLError;
	//tNSSet("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uNSSet,"tNSSet","Del");
#endif
		tNSSet(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uNSSet,"tNSSet","DelError");
#endif
		tNSSet(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetNSSet(void)


void Insert_tNSSet(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tNSSet SET uNSSet=%u,cLabel='%s',cMasterIPs='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uNSSet
			,TextAreaSave(cLabel)
			,TextAreaSave(cMasterIPs)
			,uOwner
			,uCreatedBy
			);

	macro_mySQLQueryHTMLError;

}//void Insert_tNSSet(void)


void Update_tNSSet(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tNSSet SET uNSSet=%u,cLabel='%s',cMasterIPs='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uNSSet
			,TextAreaSave(cLabel)
			,TextAreaSave(cMasterIPs)
			,uModBy
			,cRowid);

	macro_mySQLQueryHTMLError;

}//void Update_tNSSet(void)


void ModtNSSet(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	//Mod select gcQuery
	if(guPermLevel<10)
	sprintf(gcQuery,"SELECT tNSSet.uNSSet,\
				tNSSet.uModDate\
				FROM tNSSet,tClient\
				WHERE tNSSet.uNSSet=%u\
				AND tNSSet.uOwner=tClient.uClient\
				AND (tClient.uOwner=%u OR tClient.uClient=%u)"
			,uNSSet,guLoginClient,guLoginClient);
	else
	sprintf(gcQuery,"SELECT uNSSet,uModDate FROM tNSSet\
				WHERE uNSSet=%u"
						,uNSSet);
#else
	sprintf(gcQuery,"SELECT uNSSet FROM tNSSet\
				WHERE uNSSet=%u"
						,uNSSet);
#endif

	macro_mySQLRunAndStore(res);
	i=mysql_num_rows(res);

	//if(i<1) tNSSet("<blink>Record does not exist");
	if(i<1) tNSSet(LANG_NBR_RECNOTEXIST);
	//if(i>1) tNSSet("<blink>Multiple rows!");
	if(i>1) tNSSet(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tNSSet(LANG_NBR_EXTMOD);
#endif

	Update_tNSSet(field[0]);
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tNSSet",uNSSet);
	iDNSLog(uNSSet,"tNSSet","Mod");
#endif
	tNSSet(gcQuery);

}//ModtNSSet(void)


void tNSSetList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttNSSetListSelect();

	macro_mySQLRunAndStore(res);
	guI=mysql_num_rows(res);

	PageMachine("tNSSetList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttNSSetListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uNSSet<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>cMasterIPs<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		time_t luTime5=strtoul(field[5],NULL,10);
		char cBuf5[32];
		if(luTime5)
			ctime_r(&luTime5,cBuf5);
		else
			sprintf(cBuf5,"---");
		time_t luTime7=strtoul(field[7],NULL,10);
		char cBuf7[32];
		if(luTime7)
			ctime_r(&luTime7,cBuf7);
		else
			sprintf(cBuf7,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
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

}//tNSSetList()


void CreatetNSSet(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tNSSet ( uNSSet INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(32) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, cMasterIPs VARCHAR(255) NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetNSSet()

