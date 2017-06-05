/*
FILE
	tConfiguration source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Global configuration name/value pair table.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uConfiguration: Primary Key
static unsigned uConfiguration=0;
//cLabel: Short label
static char cLabel[101]={""};
//cValue: Value of name(cLabel)/value pair
static char cValue[256]={""};
//cComment: Comment about this configuration name/value pair
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



#define VAR_LIST_tConfiguration "tConfiguration.uConfiguration,tConfiguration.cLabel,tConfiguration.cValue,tConfiguration.cComment,tConfiguration.uOwner,tConfiguration.uCreatedBy,tConfiguration.uCreatedDate,tConfiguration.uModBy,tConfiguration.uModDate"

 //Local only
void Insert_tConfiguration(void);
void Update_tConfiguration(char *cRowid);
void ProcesstConfigurationListVars(pentry entries[], int x);

 //In tConfigurationfunc.h file included below
void ExtProcesstConfigurationVars(pentry entries[], int x);
void ExttConfigurationCommands(pentry entries[], int x);
void ExttConfigurationButtons(void);
void ExttConfigurationNavBar(void);
void ExttConfigurationGetHook(entry gentries[], int x);
void ExttConfigurationSelect(void);
void ExttConfigurationSelectRow(void);
void ExttConfigurationListSelect(void);
void ExttConfigurationListFilter(void);
void ExttConfigurationAuxTable(void);

#include "tconfigurationfunc.h"

 //Table Variables Assignment Function
void ProcesstConfigurationVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uConfiguration"))
			sscanf(entries[i].val,"%u",&uConfiguration);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"cValue"))
			sprintf(cValue,"%.255s",entries[i].val);
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
	ExtProcesstConfigurationVars(entries,x);

}//ProcesstConfigurationVars()


void ProcesstConfigurationListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uConfiguration);
                        guMode=2002;
                        tConfiguration("");
                }
        }
}//void ProcesstConfigurationListVars(pentry entries[], int x)


int tConfigurationCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttConfigurationCommands(entries,x);

	if(!strcmp(gcFunction,"tConfigurationTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tConfigurationList();
		}

		//Default
		ProcesstConfigurationVars(entries,x);
		tConfiguration("");
	}
	else if(!strcmp(gcFunction,"tConfigurationList"))
	{
		ProcessControlVars(entries,x);
		ProcesstConfigurationListVars(entries,x);
		tConfigurationList();
	}

	return(0);

}//tConfigurationCommands()


void tConfiguration(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttConfigurationSelectRow();
		else
			ExttConfigurationSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetConfiguration();
				iDNS("New tConfiguration table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tConfiguration WHERE uConfiguration=%u"
						,uConfiguration);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uConfiguration);
		sprintf(cLabel,"%.100s",field[1]);
		sprintf(cValue,"%.255s",field[2]);
		cComment=field[3];
		sscanf(field[4],"%u",&uOwner);
		sscanf(field[5],"%u",&uCreatedBy);
		sscanf(field[6],"%lu",&uCreatedDate);
		sscanf(field[7],"%u",&uModBy);
		sscanf(field[8],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tConfiguration",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tConfigurationTools>");
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

        ExttConfigurationButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tConfiguration Record Data",100);

	if(guMode==2000 || guMode==2002)
		tConfigurationInput(1);
	else
		tConfigurationInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttConfigurationAuxTable();

	Footer_ism3();

}//end of tConfiguration();


void tConfigurationInput(unsigned uMode)
{

//uConfiguration
	OpenRow(LANG_FL_tConfiguration_uConfiguration,"black");
	printf("<input title='%s' type=text name=uConfiguration value=%u size=16 maxlength=10 "
,LANG_FT_tConfiguration_uConfiguration,uConfiguration);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uConfiguration value=%u >\n",uConfiguration);
	}
//cLabel
	OpenRow(LANG_FL_tConfiguration_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tConfiguration_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=8 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//cValue
	OpenRow(LANG_FL_tConfiguration_cValue,"black");
	printf("<input title='%s' type=text name=cValue value=\"%s\" size=40 maxlength=255 "
,LANG_FT_tConfiguration_cValue,EncodeDoubleQuotes(cValue));
	if(guPermLevel>=8 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cValue value=\"%s\">\n",EncodeDoubleQuotes(cValue));
	}
//cComment
	OpenRow(LANG_FL_tConfiguration_cComment,"black");
	printf("<textarea title='%s' cols=80 wrap=hard rows=16 name=cComment "
,LANG_FT_tConfiguration_cComment);
	if(guPermLevel>=8 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",cComment);
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",cComment);
		printf("<input type=hidden name=cComment value=\"%s\" >\n",EncodeDoubleQuotes(cComment));
	}
//uOwner
	OpenRow(LANG_FL_tConfiguration_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tConfiguration_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tConfiguration_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tConfiguration_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tConfiguration_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tConfigurationInput(unsigned uMode)


void NewtConfiguration(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uConfiguration FROM tConfiguration\
				WHERE uConfiguration=%u"
							,uConfiguration);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tConfiguration("<blink>Record already exists");
		tConfiguration(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tConfiguration();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uConfiguration=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tConfiguration",uConfiguration);
	iDNSLog(uConfiguration,"tConfiguration","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uConfiguration);
	tConfiguration(gcQuery);
	}

}//NewtConfiguration(unsigned uMode)


void DeletetConfiguration(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tConfiguration WHERE uConfiguration=%u AND ( uOwner=%u OR %u>9 )"
					,uConfiguration,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tConfiguration WHERE uConfiguration=%u"
					,uConfiguration);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tConfiguration("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uConfiguration,"tConfiguration","Del");
#endif
		tConfiguration(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uConfiguration,"tConfiguration","DelError");
#endif
		tConfiguration(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetConfiguration(void)


void Insert_tConfiguration(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tConfiguration SET uConfiguration=%u,cLabel='%s',cValue='%s',cComment='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uConfiguration
			,TextAreaSave(cLabel)
			,TextAreaSave(cValue)
			,TextAreaSave(cComment)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tConfiguration(void)


void Update_tConfiguration(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tConfiguration SET uConfiguration=%u,cLabel='%s',cValue='%s',cComment='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uConfiguration
			,TextAreaSave(cLabel)
			,TextAreaSave(cValue)
			,TextAreaSave(cComment)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tConfiguration(void)


void ModtConfiguration(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;
	
	sprintf(gcQuery,"SELECT uConfiguration,uModDate FROM tConfiguration WHERE uConfiguration=%u"
			,uConfiguration);
#else
	sprintf(gcQuery,"SELECT uConfiguration FROM tConfiguration WHERE uConfiguration=%u"
			,uConfiguration);
#endif

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tConfiguration("<blink>Record does not exist");
	if(i<1) tConfiguration(LANG_NBR_RECNOTEXIST);
	//if(i>1) tConfiguration("<blink>Multiple rows!");
	if(i>1) tConfiguration(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tConfiguration(LANG_NBR_EXTMOD);
#endif

	Update_tConfiguration(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tConfiguration",uConfiguration);
	iDNSLog(uConfiguration,"tConfiguration","Mod");
#endif
	tConfiguration(gcQuery);

}//ModtConfiguration(void)


void tConfigurationList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttConfigurationListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tConfigurationList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttConfigurationListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uConfiguration<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>cValue<td><font face=arial,helvetica color=white>cComment<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		long luTime6=strtoul(field[6],NULL,10);
		char cBuf6[32];
		if(luTime6)
			ctime_r(&luTime6,cBuf6);
		else
			sprintf(cBuf6,"---");
		long luTime8=strtoul(field[8],NULL,10);
		char cBuf8[32];
		if(luTime8)
			ctime_r(&luTime8,cBuf8);
		else
			sprintf(cBuf8,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td><textarea disabled>%s</textarea><td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,field[2]
			,field[3]
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[4],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[5],NULL,10))
			,cBuf6
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[7],NULL,10))
			,cBuf8
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tConfigurationList()


void CreatetConfiguration(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tConfiguration ( uModBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, cLabel VARCHAR(100) NOT NULL DEFAULT '', UNIQUE (cLabel), uConfiguration INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cComment TEXT NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uModDate INT UNSIGNED NOT NULL DEFAULT 0, cValue VARCHAR(255) NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetConfiguration()

