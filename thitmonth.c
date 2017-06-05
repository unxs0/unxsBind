/*
FILE
	tHitMonth source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Month table to tHit archives.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uHit: Primary Key
static unsigned uHit=0;
//cZone: Zone name
static char cZone[101]={""};
static char cHost[101]={""};
//uHitCount: Hit Counter
static long unsigned uHitCount=0;
//New fields via direct edit not RAD tool
static long unsigned uSuccess=0;
static long unsigned uReferral=0;
static long unsigned uNxrrset=0;
static long unsigned uNxdomain=0;
static long unsigned uRecursion=0;
static long unsigned uFailure=0;
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



#define VAR_LIST_tHitMonth "tHitMonth.uHit,tHitMonth.cZone,tHitMonth.cHost,tHitMonth.uHitCount,tHitMonth.uSuccess,tHitMonth.uReferral,tHitMonth.uNxrrset,tHitMonth.uNxdomain,tHitMonth.uRecursion,tHitMonth.uFailure,tHitMonth.uOwner,tHitMonth.uCreatedBy,tHitMonth.uCreatedDate,tHitMonth.uModBy,tHitMonth.uModDate"

 //Local only
void Insert_tHitMonth(void);
void Update_tHitMonth(char *cRowid);
void ProcesstHitMonthListVars(pentry entries[], int x);

 //In tHitMonthfunc.h file included below
void ExtProcesstHitMonthVars(pentry entries[], int x);
void ExttHitMonthCommands(pentry entries[], int x);
void ExttHitMonthButtons(void);
void ExttHitMonthNavBar(void);
void ExttHitMonthGetHook(entry gentries[], int x);
void ExttHitMonthSelect(void);
void ExttHitMonthSelectRow(void);
void ExttHitMonthListSelect(void);
void ExttHitMonthListFilter(void);
void ExttHitMonthAuxTable(void);

#include "thitmonthfunc.h"

 //Table Variables Assignment Function
void ProcesstHitMonthVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uHit"))
			sscanf(entries[i].val,"%u",&uHit);
		else if(!strcmp(entries[i].name,"cZone"))
			sprintf(cZone,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"cHost"))
			sprintf(cHost,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"uHitCount"))
			sscanf(entries[i].val,"%lu",&uHitCount);
		else if(!strcmp(entries[i].name,"uSuccess"))
			sscanf(entries[i].val,"%lu",&uSuccess);
		else if(!strcmp(entries[i].name,"uReferral"))
			sscanf(entries[i].val,"%lu",&uReferral);
		else if(!strcmp(entries[i].name,"uNxrrset"))
			sscanf(entries[i].val,"%lu",&uNxrrset);
		else if(!strcmp(entries[i].name,"uNxdomain"))
			sscanf(entries[i].val,"%lu",&uNxdomain);
		else if(!strcmp(entries[i].name,"uRecursion"))
			sscanf(entries[i].val,"%lu",&uRecursion);
		else if(!strcmp(entries[i].name,"uFailure"))
			sscanf(entries[i].val,"%lu",&uFailure);
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
	ExtProcesstHitMonthVars(entries,x);

}//ProcesstHitMonthVars()


void ProcesstHitMonthListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uHit);
                        guMode=2002;
                        tHitMonth("");
                }
        }
}//void ProcesstHitMonthListVars(pentry entries[], int x)


int tHitMonthCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttHitMonthCommands(entries,x);

	if(!strcmp(gcFunction,"tHitMonthTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tHitMonthList();
		}

		//Default
		ProcesstHitMonthVars(entries,x);
		tHitMonth("");
	}
	else if(!strcmp(gcFunction,"tHitMonthList"))
	{
		ProcessControlVars(entries,x);
		ProcesstHitMonthListVars(entries,x);
		tHitMonthList();
	}

	return(0);

}//tHitMonthCommands()


void tHitMonth(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttHitMonthSelectRow();
		else
			ExttHitMonthSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetHitMonth();
				iDNS("New tHitMonth table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tHitMonth WHERE uHit=%u"
						,uHit);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uHit);
		sprintf(cZone,"%.100s",field[1]);
		sprintf(cHost,"%.100s",field[2]);
		sscanf(field[3],"%lu",&uHitCount);
		sscanf(field[4],"%lu",&uSuccess);
		sscanf(field[5],"%lu",&uReferral);
		sscanf(field[6],"%lu",&uNxrrset);
		sscanf(field[7],"%lu",&uNxdomain);
		sscanf(field[8],"%lu",&uRecursion);
		sscanf(field[9],"%lu",&uFailure);
		sscanf(field[10],"%u",&uOwner);
		sscanf(field[11],"%u",&uCreatedBy);
		sscanf(field[12],"%lu",&uCreatedDate);
		sscanf(field[13],"%u",&uModBy);
		sscanf(field[14],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tHitMonth",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tHitMonthTools>");
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

        ExttHitMonthButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tHitMonth Record Data",100);

	if(guMode==2000 || guMode==2002)
		tHitMonthInput(1);
	else
		tHitMonthInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttHitMonthAuxTable();

	Footer_ism3();

}//end of tHitMonth();


void tHitMonthInput(unsigned uMode)
{

//uHit
	OpenRow(LANG_FL_tHitMonth_uHit,"black");
	printf("<input title='%s' type=text name=uHit value=%u size=16 maxlength=10 "
,LANG_FT_tHitMonth_uHit,uHit);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uHit value=%u >\n",uHit);
	}
//cZone
	OpenRow(LANG_FL_tHitMonth_cZone,"black");
	printf("<input title='%s' type=text name=cZone value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tHitMonth_cZone,EncodeDoubleQuotes(cZone));
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cZone value=\"%s\">\n",EncodeDoubleQuotes(cZone));
	}
//cHost
	OpenRow("cHost","black");
	printf("<input title='cHost' type=text name=cHost value=\"%s\" size=40 maxlength=100 ",EncodeDoubleQuotes(cHost));
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cHost value=\"%s\">\n",EncodeDoubleQuotes(cHost));
	}


//uHitCount
	OpenRow(LANG_FL_tHitMonth_uHitCount,"black");
	printf("<input title='%s' type=text name=uHitCount value=%lu size=16 maxlength=10 "
,LANG_FT_tHitMonth_uHitCount,uHitCount);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uHitCount value=%lu >\n",uHitCount);
	}
//uSuccess
	OpenRow("uSuccess","black");
	printf("<input title='BIND9 named.stats success stat' type=text name=uSuccess value=%lu size=16 maxlength=10 "
		,uSuccess);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uSuccess value=%lu >\n",uSuccess);
	}
//uReferral
	OpenRow("uReferral","black");
	printf("<input title='uReferral' type=text name=uReferral value=%lu size=16 maxlength=10 ",uReferral);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uReferral value=%lu >\n",uReferral);
	}
//uNxrrset
	OpenRow("uNxrrset","black");
	printf("<input title='uNxrrset' type=text name=uNxrrset value=%lu size=16 maxlength=10 ",uNxrrset);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uNxrrset value=%lu >\n",uNxrrset);
	}
//uNxdomain
	OpenRow("uNxdomain","black");
	printf("<input title='uNxdomain' type=text name=uNxdomain value=%lu size=16 maxlength=10 ",uNxdomain);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uNxdomain value=%lu >\n",uNxdomain);
	}
//uRecursion
	OpenRow("uRecursion","black");
	printf("<input title='uRecursion' type=text name=uRecursion value=%lu size=16 maxlength=10 ",uRecursion);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uRecursion value=%lu >\n",uRecursion);
	}
//uFailure
	OpenRow("uFailure","black");
	printf("<input title='uFailure' type=text name=uFailure value=%lu size=16 maxlength=10 ",uFailure);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uFailure value=%lu >\n",uFailure);
	}

//uOwner
	OpenRow(LANG_FL_tHitMonth_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey("tClient","cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey("tClient","cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tHitMonth_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey("tClient","cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey("tClient","cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tHitMonth_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tHitMonth_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey("tClient","cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey("tClient","cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tHitMonth_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tHitMonthInput(unsigned uMode)


void NewtHitMonth(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uHit FROM tHitMonth\
				WHERE uHit=%u"
							,uHit);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tHitMonth("<blink>Record already exists");
		tHitMonth(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tHitMonth();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uHit=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tHitMonth",uHit);
	iDNSLog(uHit,"tHitMonth","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uHit);
	tHitMonth(gcQuery);
	}

}//NewtHitMonth(unsigned uMode)


void DeletetHitMonth(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tHitMonth WHERE uHit=%u AND ( uOwner=%u OR %u>9 )"
					,uHit,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tHitMonth WHERE uHit=%u"
					,uHit);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tHitMonth("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uHit,"tHitMonth","Del");
#endif
		tHitMonth(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uHit,"tHitMonth","DelError");
#endif
		tHitMonth(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetHitMonth(void)


void Insert_tHitMonth(void)
{

	sprintf(gcQuery,"INSERT INTO tHitMonth SET uHit=%u,cZone='%s',cHost='%s',uHitCount=%lu,uSuccess=%lu,uReferral=%lu,uNxrrset=%lu,uNxdomain=%lu,uRecursion=%lu,uFailure=%lu,uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uHit
			,TextAreaSave(cZone)
			,TextAreaSave(cHost)
			,uHitCount
			,uSuccess,uReferral,uNxrrset,uNxdomain,uRecursion,uFailure
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tHitMonth(void)


void Update_tHitMonth(char *cRowid)
{

	sprintf(gcQuery,"UPDATE tHitMonth SET uHit=%u,cZone='%s',cHost='%s',uHitCount=%lu,uSuccess=%lu,uReferral=%lu,uNxrrset=%lu,uNxdomain=%lu,uRecursion=%lu,uFailure=%lu,uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uHit
			,TextAreaSave(cZone)
			,TextAreaSave(cHost)
			,uHitCount
			,uSuccess,uReferral,uNxrrset,uNxdomain,uRecursion,uFailure
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tHitMonth(void)


void ModtHitMonth(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uPreModDate=0;

	//Mod select gcQuery
	sprintf(gcQuery,"SELECT uHit FROM tHitMonth WHERE uHit=%u",uHit);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tHitMonth("<blink>Record does not exist");
	if(i<1) tHitMonth(LANG_NBR_RECNOTEXIST);
	//if(i>1) tHitMonth("<blink>Multiple rows!");
	if(i>1) tHitMonth(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tHitMonth(LANG_NBR_EXTMOD);
#endif

	Update_tHitMonth(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tHitMonth",uHit);
	iDNSLog(uHit,"tHitMonth","Mod");
#endif
	tHitMonth(gcQuery);

}//ModtHitMonth(void)


void tHitMonthList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttHitMonthListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tHitMonthList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttHitMonthListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uHit<td><font face=arial,helvetica color=white>cZone<td><font face=arial,helvetica color=white>cHost<td><font face=arial,helvetica color=white>uHitCount<td><font face=arial,helvetica color=white>uSuccess<td><font face=arial,helvetica color=white>uReferral<td><font face=arial,helvetica color=white>uNxrrset<td><font face=arial,helvetica color=white>uNxdomain<td><font face=arial,helvetica color=white>uRecursion<td><font face=arial,helvetica color=white>uFailure<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		long luTime5=strtoul(field[12],NULL,10);
		char cBuf5[32];
		if(luTime5)
			ctime_r(&luTime5,cBuf5);
		else
			sprintf(cBuf5,"---");
		long luTime7=strtoul(field[14],NULL,10);
		char cBuf7[32];
		if(luTime7)
			ctime_r(&luTime7,cBuf7);
		else
			sprintf(cBuf7,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,field[2]
			,field[3]
			,field[4]
			,field[5]
			,field[6]
			,field[7]
			,field[8]
			,field[9]
			,ForeignKey("tClient","cLabel",strtoul(field[10],NULL,10))
			,ForeignKey("tClient","cLabel",strtoul(field[11],NULL,10))
			,cBuf5
			,ForeignKey("tClient","cLabel",strtoul(field[13],NULL,10))
			,cBuf7
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tHitMonthList()


void CreatetHitMonth(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tHitMonth ( uHit INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cZone VARCHAR(255) NOT NULL DEFAULT '',INDEX (cZone), uOwner INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, uHitCount BIGINT UNSIGNED NOT NULL DEFAULT 0, uSuccess BIGINT UNSIGNED NOT NULL DEFAULT 0, uReferral BIGINT UNSIGNED NOT NULL DEFAULT 0, uNxrrset BIGINT UNSIGNED NOT NULL DEFAULT 0, uNxdomain BIGINT UNSIGNED NOT NULL DEFAULT 0, uRecursion BIGINT UNSIGNED NOT NULL DEFAULT 0, uFailure BIGINT UNSIGNED NOT NULL DEFAULT 0, cHost VARCHAR(255) NOT NULL DEFAULT '',INDEX (cHost) )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetHitMonth()

