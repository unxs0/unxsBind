/*
FILE
	tHit source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Zone DNS query stats. Does not work with modern BIND named.
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



#define VAR_LIST_tHit "tHit.uHit,tHit.cZone,tHit.cHost,tHit.uHitCount,tHit.uSuccess,tHit.uReferral,tHit.uNxrrset,tHit.uNxdomain,tHit.uRecursion,tHit.uFailure,tHit.uOwner,tHit.uCreatedBy,tHit.uCreatedDate,tHit.uModBy,tHit.uModDate"

 //Local only
void Insert_tHit(void);
void Update_tHit(char *cRowid);
void ProcesstHitListVars(pentry entries[], int x);

 //In tHitfunc.h file included below
void ExtProcesstHitVars(pentry entries[], int x);
void ExttHitCommands(pentry entries[], int x);
void ExttHitButtons(void);
void ExttHitNavBar(void);
void ExttHitGetHook(entry gentries[], int x);
void ExttHitSelect(void);
void ExttHitSelectRow(void);
void ExttHitListSelect(void);
void ExttHitListFilter(void);
void ExttHitAuxTable(void);

#include "thitfunc.h"

 //Table Variables Assignment Function
void ProcesstHitVars(pentry entries[], int x)
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
	ExtProcesstHitVars(entries,x);

}//ProcesstHitVars()


void ProcesstHitListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uHit);
                        guMode=2002;
                        tHit("");
                }
        }
}//void ProcesstHitListVars(pentry entries[], int x)


int tHitCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttHitCommands(entries,x);

	if(!strcmp(gcFunction,"tHitTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tHitList();
		}

		//Default
		ProcesstHitVars(entries,x);
		tHit("");
	}
	else if(!strcmp(gcFunction,"tHitList"))
	{
		ProcessControlVars(entries,x);
		ProcesstHitListVars(entries,x);
		tHitList();
	}

	return(0);

}//tHitCommands()


void tHit(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttHitSelectRow();
		else
			ExttHitSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetHit();
				iDNS("New tHit table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tHit WHERE uHit=%u"
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

	Header_ism3(":: tHit",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tHitTools>");
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

        ExttHitButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tHit Record Data",100);

	if(guMode==2000 || guMode==2002)
		tHitInput(1);
	else
		tHitInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttHitAuxTable();

	Footer_ism3();

}//end of tHit();


void tHitInput(unsigned uMode)
{

//uHit
	OpenRow(LANG_FL_tHit_uHit,"black");
	printf("<input title='%s' type=text name=uHit value=%u size=16 maxlength=10 "
,LANG_FT_tHit_uHit,uHit);
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
	OpenRow(LANG_FL_tHit_cZone,"black");
	printf("<input title='%s' type=text name=cZone value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tHit_cZone,EncodeDoubleQuotes(cZone));
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
	OpenRow(LANG_FL_tHit_uHitCount,"black");
	printf("<input title='%s' type=text name=uHitCount value=%lu size=16 maxlength=10 "
,LANG_FT_tHit_uHitCount,uHitCount);
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
	OpenRow(LANG_FL_tHit_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey("tClient","cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey("tClient","cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tHit_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey("tClient","cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey("tClient","cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tHit_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tHit_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey("tClient","cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey("tClient","cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tHit_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tHitInput(unsigned uMode)


void NewtHit(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uHit FROM tHit\
				WHERE uHit=%u"
							,uHit);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tHit("<blink>Record already exists");
		tHit(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tHit();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uHit=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tHit",uHit);
	iDNSLog(uHit,"tHit","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uHit);
	tHit(gcQuery);
	}

}//NewtHit(unsigned uMode)


void DeletetHit(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tHit WHERE uHit=%u AND ( uOwner=%u OR %u>9 )"
					,uHit,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tHit WHERE uHit=%u"
					,uHit);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tHit("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uHit,"tHit","Del");
#endif
		tHit(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uHit,"tHit","DelError");
#endif
		tHit(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetHit(void)


void Insert_tHit(void)
{

	sprintf(gcQuery,"INSERT INTO tHit SET uHit=%u,cZone='%s',cHost='%s',uHitCount=%lu,uSuccess=%lu,uReferral=%lu,uNxrrset=%lu,uNxdomain=%lu,uRecursion=%lu,uFailure=%lu,uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uHit
			,TextAreaSave(cZone)
			,TextAreaSave(cHost)
			,uHitCount
			,uSuccess,uReferral,uNxrrset,uNxdomain,uRecursion,uFailure
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tHit(void)


void Update_tHit(char *cRowid)
{

	sprintf(gcQuery,"UPDATE tHit SET uHit=%u,cZone='%s',cHost='%s',uHitCount=%lu,uSuccess=%lu,uReferral=%lu,uNxrrset=%lu,uNxdomain=%lu,uRecursion=%lu,uFailure=%lu,uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uHit
			,TextAreaSave(cZone)
			,TextAreaSave(cHost)
			,uHitCount
			,uSuccess,uReferral,uNxrrset,uNxdomain,uRecursion,uFailure
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tHit(void)


void ModtHit(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	//Mod select gcQuery
	if(guPermLevel<10)
	sprintf(gcQuery,"SELECT tHit.uHit,\
				tHit.uModDate\
				FROM tHit,tClient\
				WHERE tHit.uHit=%u\
				AND tHit.uOwner=tClient.uClient\
				AND (tClient.uOwner=%u OR tClient.uClient=%u)"
			,uHit,guLoginClient,guLoginClient);
	else
	sprintf(gcQuery,"SELECT uHit,uModDate FROM tHit\
				WHERE uHit=%u"
						,uHit);
#else
	sprintf(gcQuery,"SELECT uHit FROM tHit\
				WHERE uHit=%u"
						,uHit);
#endif

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tHit("<blink>Record does not exist");
	if(i<1) tHit(LANG_NBR_RECNOTEXIST);
	//if(i>1) tHit("<blink>Multiple rows!");
	if(i>1) tHit(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tHit(LANG_NBR_EXTMOD);
#endif

	Update_tHit(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tHit",uHit);
	iDNSLog(uHit,"tHit","Mod");
#endif
	tHit(gcQuery);

}//ModtHit(void)


void tHitList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttHitListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tHitList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttHitListFilter();

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

}//tHitList()


void CreatetHit(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tHit ( uHit INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cZone VARCHAR(255) NOT NULL DEFAULT '',INDEX (cZone), uOwner INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, uHitCount BIGINT UNSIGNED NOT NULL DEFAULT 0, uSuccess BIGINT UNSIGNED NOT NULL DEFAULT 0, uReferral BIGINT UNSIGNED NOT NULL DEFAULT 0, uNxrrset BIGINT UNSIGNED NOT NULL DEFAULT 0, uNxdomain BIGINT UNSIGNED NOT NULL DEFAULT 0, uRecursion BIGINT UNSIGNED NOT NULL DEFAULT 0, uFailure BIGINT UNSIGNED NOT NULL DEFAULT 0, cHost VARCHAR(255) NOT NULL DEFAULT '',INDEX (cHost) )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetHit()

