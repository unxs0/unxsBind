/*
FILE
	tJob source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Job queue table.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uJob: Unique job ID
static unsigned uJob=0;
//uMasterJob: Unique group job ID
static unsigned uMasterJob=0;
//cJob: Job description
static char cJob[101]={""};
//cZone: Zone to be operated on
static char cZone[101]={""};
//uNSSet: Key into tNSSet
static unsigned uNSSet=0;
static char cuNSSetPullDown[256]={""};
//cTargetServer: Name of server that should process this job
static char cTargetServer[101]={""};
//uPriority: Job priority
static unsigned uPriority=0;
//uTime: Unix seconds threshold
static long uTime=0;
//cJobData: Aux job data
static char cJobData[101]={""};
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



#define VAR_LIST_tJob "tJob.uJob,tJob.uMasterJob,tJob.cJob,tJob.cZone,tJob.uNSSet,tJob.cTargetServer,tJob.uPriority,tJob.uTime,tJob.cJobData,tJob.uOwner,tJob.uCreatedBy,tJob.uCreatedDate,tJob.uModBy,tJob.uModDate"

 //Local only
void Insert_tJob(void);
void Update_tJob(char *cRowid);
void ProcesstJobListVars(pentry entries[], int x);

 //In tJobfunc.h file included below
void ExtProcesstJobVars(pentry entries[], int x);
void ExttJobCommands(pentry entries[], int x);
void ExttJobButtons(void);
void ExttJobNavBar(void);
void ExttJobGetHook(entry gentries[], int x);
void ExttJobSelect(void);
void ExttJobSelectRow(void);
void ExttJobListSelect(void);
void ExttJobListFilter(void);
void ExttJobAuxTable(void);

#include "tjobfunc.h"

 //Table Variables Assignment Function
void ProcesstJobVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uJob"))
			sscanf(entries[i].val,"%u",&uJob);
		else if(!strcmp(entries[i].name,"uMasterJob"))
			sscanf(entries[i].val,"%u",&uMasterJob);
		else if(!strcmp(entries[i].name,"cJob"))
			sprintf(cJob,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"cZone"))
			sprintf(cZone,"%.100s",FQDomainName(entries[i].val));
		else if(!strcmp(entries[i].name,"uNSSet"))
			sscanf(entries[i].val,"%u",&uNSSet);
		else if(!strcmp(entries[i].name,"cuNSSetPullDown"))
		{
			sprintf(cuNSSetPullDown,"%.255s",entries[i].val);
			uNSSet=ReadPullDown("tNSSet","cLabel",cuNSSetPullDown);
		}
		else if(!strcmp(entries[i].name,"cTargetServer"))
			sprintf(cTargetServer,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"uPriority"))
			sscanf(entries[i].val,"%u",&uPriority);
		else if(!strcmp(entries[i].name,"uTime"))
			sscanf(entries[i].val,"%lu",&uTime);
		else if(!strcmp(entries[i].name,"cJobData"))
			sprintf(cJobData,"%.100s",entries[i].val);
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
	ExtProcesstJobVars(entries,x);

}//ProcesstJobVars()


void ProcesstJobListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uJob);
                        guMode=2002;
                        tJob("");
                }
        }
}//void ProcesstJobListVars(pentry entries[], int x)


int tJobCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttJobCommands(entries,x);

	if(!strcmp(gcFunction,"tJobTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tJobList();
		}

		//Default
		ProcesstJobVars(entries,x);
		tJob("");
	}
	else if(!strcmp(gcFunction,"tJobList"))
	{
		ProcessControlVars(entries,x);
		ProcesstJobListVars(entries,x);
		tJobList();
	}

	return(0);

}//tJobCommands()


void tJob(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttJobSelectRow();
		else
			ExttJobSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetJob();
				iDNS("New tJob table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tJob WHERE uJob=%u"
						,uJob);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uJob);
		sscanf(field[1],"%u",&uMasterJob);
		sprintf(cJob,"%.100s",field[2]);
		sprintf(cZone,"%.100s",field[3]);
		sscanf(field[4],"%u",&uNSSet);
		sprintf(cTargetServer,"%.100s",field[5]);
		sscanf(field[6],"%u",&uPriority);
		sscanf(field[7],"%lu",&uTime);
		sprintf(cJobData,"%.100s",field[8]);
		sscanf(field[9],"%u",&uOwner);
		sscanf(field[10],"%u",&uCreatedBy);
		sscanf(field[11],"%lu",&uCreatedDate);
		sscanf(field[12],"%u",&uModBy);
		sscanf(field[13],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tJob",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tJobTools>");
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

        ExttJobButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tJob Record Data",100);

	if(guMode==2000 || guMode==2002)
		tJobInput(1);
	else
		tJobInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttJobAuxTable();

	Footer_ism3();

}//end of tJob();


void tJobInput(unsigned uMode)
{

//uJob
	OpenRow(LANG_FL_tJob_uJob,"black");
	printf("<input title='%s' type=text name=uJob value=%u size=16 maxlength=10 "
,LANG_FT_tJob_uJob,uJob);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uJob value=%u >\n",uJob);
	}
//uMasterJob
	OpenRow(LANG_FL_tJob_uMasterJob,"black");
	printf("<input title='%s' type=text name=uMasterJob value=%u size=16 maxlength=10 "
,LANG_FT_tJob_uMasterJob,uMasterJob);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uMasterJob value=%u >\n",uMasterJob);
	}
//cJob
	OpenRow(LANG_FL_tJob_cJob,"black");
	printf("<input title='%s' type=text name=cJob value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tJob_cJob,EncodeDoubleQuotes(cJob));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cJob value=\"%s\">\n",EncodeDoubleQuotes(cJob));
	}
//cZone
	OpenRow(LANG_FL_tJob_cZone,"black");
	printf("<input title='%s' type=text name=cZone value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tJob_cZone,EncodeDoubleQuotes(cZone));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cZone value=\"%s\">\n",EncodeDoubleQuotes(cZone));
	}
//uNSSet
	OpenRow(LANG_FL_tJob_uNSSet,"black");
	if(guPermLevel>=0 && uMode)
		tTablePullDown("tNSSet;cuNSSetPullDown","cLabel","cLabel",uNSSet,1);
	else
		tTablePullDown("tNSSet;cuNSSetPullDown","cLabel","cLabel",uNSSet,0);
//cTargetServer
	OpenRow(LANG_FL_tJob_cTargetServer,"black");
	printf("<input title='%s' type=text name=cTargetServer value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tJob_cTargetServer,EncodeDoubleQuotes(cTargetServer));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cTargetServer value=\"%s\">\n",EncodeDoubleQuotes(cTargetServer));
	}
//uPriority
	OpenRow(LANG_FL_tJob_uPriority,"black");
	printf("<input title='%s' type=text name=uPriority value=%u size=16 maxlength=10 "
,LANG_FT_tJob_uPriority,uPriority);
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uPriority value=%u >\n",uPriority);
	}
//uTime
	OpenRow(LANG_FL_tJob_uTime,"black");
	if(uTime)
		printf("<input type=text name=cuTime value='%s' disabled>\n",ctime(&uTime));
	else
		printf("<input type=text name=cuTime value='---' disabled>\n");
	printf("<input type=hidden name=uTime value=%lu>\n",uTime);
//cJobData
	OpenRow(LANG_FL_tJob_cJobData,"black");
	printf("<input title='%s' type=text name=cJobData value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tJob_cJobData,EncodeDoubleQuotes(cJobData));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cJobData value=\"%s\">\n",EncodeDoubleQuotes(cJobData));
	}
//uOwner
	OpenRow(LANG_FL_tJob_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tJob_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tJob_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tJob_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tJob_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tJobInput(unsigned uMode)


void NewtJob(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uJob FROM tJob\
				WHERE uJob=%u"
							,uJob);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tJob("<blink>Record already exists");
		tJob(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tJob();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uJob=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tJob",uJob);
	iDNSLog(uJob,"tJob","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uJob);
	tJob(gcQuery);
	}

}//NewtJob(unsigned uMode)


void DeletetJob(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tJob WHERE uJob=%u AND ( uOwner=%u OR %u>9 )"
					,uJob,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tJob WHERE uJob=%u"
					,uJob);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tJob("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uJob,"tJob","Del");
#endif
		tJob(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uJob,"tJob","DelError");
#endif
		tJob(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetJob(void)


void Insert_tJob(void)
{

	if(uTime)
		sprintf(gcQuery,"INSERT INTO tJob SET uJob=%u,uMasterJob=%u,cJob='%s',cZone='%s',uNSSet=%u,"
				"cTargetServer='%s',uPriority=%u,uTime=%lu,cJobData='%s',uOwner=%u,uCreatedBy=%u,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uJob
			,uMasterJob
			,TextAreaSave(cJob)
			,TextAreaSave(cZone)
			,uNSSet
			,TextAreaSave(cTargetServer)
			,uPriority
			,uTime
			,TextAreaSave(cJobData)
			,uOwner
			,uCreatedBy
			);
	else
		sprintf(gcQuery,"INSERT INTO tJob SET uJob=%u,uMasterJob=%u,cJob='%s',cZone='%s',uNSSet=%u,"
				"cTargetServer='%s',uPriority=%u,uTime=UNIX_TIMESTAMP(NOW()),cJobData='%s',"
				"uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uJob
			,uMasterJob
			,TextAreaSave(cJob)
			,TextAreaSave(cZone)
			,uNSSet
			,TextAreaSave(cTargetServer)
			,uPriority
			,TextAreaSave(cJobData)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tJob(void)


void Update_tJob(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tJob SET uJob=%u,uMasterJob=%u,cJob='%s',cZone='%s',uNSSet=%u,cTargetServer='%s',uPriority=%u,uTime=%lu,cJobData='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uJob
			,uMasterJob
			,TextAreaSave(cJob)
			,TextAreaSave(cZone)
			,uNSSet
			,TextAreaSave(cTargetServer)
			,uPriority
			,uTime
			,TextAreaSave(cJobData)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tJob(void)


void ModtJob(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uJob,uModDate FROM tJob WHERE uJob=%u"
			,uJob);
#else
	sprintf(gcQuery,"SELECT uJob FROM tJob WHERE uJob=%u"
			,uJob);
#endif

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tJob("<blink>Record does not exist");
	if(i<1) tJob(LANG_NBR_RECNOTEXIST);
	//if(i>1) tJob("<blink>Multiple rows!");
	if(i>1) tJob(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tJob(LANG_NBR_EXTMOD);
#endif

	Update_tJob(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tJob",uJob);
	iDNSLog(uJob,"tJob","Mod");
#endif
	tJob(gcQuery);

}//ModtJob(void)


void tJobList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttJobListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tJobList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttJobListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uJob<td><font face=arial,helvetica color=white>uMasterJob<td><font face=arial,helvetica color=white>cJob<td><font face=arial,helvetica color=white>cZone<td><font face=arial,helvetica color=white>uNSSet<td><font face=arial,helvetica color=white>cTargetServer<td><font face=arial,helvetica color=white>uPriority<td><font face=arial,helvetica color=white>uTime<td><font face=arial,helvetica color=white>cJobData<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		long luTime7=strtoul(field[7],NULL,10);
		char cBuf7[32];
		if(luTime7)
			ctime_r(&luTime7,cBuf7);
		else
			sprintf(cBuf7,"---");
		long luTime11=strtoul(field[11],NULL,10);
		char cBuf11[32];
		if(luTime11)
			ctime_r(&luTime11,cBuf11);
		else
			sprintf(cBuf11,"---");
		long luTime13=strtoul(field[13],NULL,10);
		char cBuf13[32];
		if(luTime13)
			ctime_r(&luTime13,cBuf13);
		else
			sprintf(cBuf13,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,field[2]
			,field[3]
			,ForeignKey("tNSSet","cLabel",strtoul(field[4],NULL,10))
			,field[5]
			,field[6]
			,cBuf7
			,field[8]
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[9],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[10],NULL,10))
			,cBuf11
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[12],NULL,10))
			,cBuf13
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tJobList()


void CreatetJob(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tJob ( uPriority INT UNSIGNED NOT NULL DEFAULT 0, uTime INT UNSIGNED NOT NULL DEFAULT 0, cZone VARCHAR(100) NOT NULL DEFAULT '', uNSSet INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uNSSet), uModDate INT UNSIGNED NOT NULL DEFAULT 0, uJob INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cJob VARCHAR(100) NOT NULL DEFAULT '', uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uOwner INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uOwner), cTargetServer VARCHAR(100) NOT NULL DEFAULT '', uMasterJob INT UNSIGNED NOT NULL DEFAULT 0, cJobData VARCHAR(100) NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetJob()

