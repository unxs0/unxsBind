/*
FILE
	tRegistrar source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	Zone registrar table.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uRegistrar: Primary Key
static unsigned uRegistrar=0;
//cLabel: Registrar of Record
static char cLabel[101]={""};
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



#define VAR_LIST_tRegistrar "tRegistrar.uRegistrar,tRegistrar.cLabel,tRegistrar.uOwner,tRegistrar.uCreatedBy,tRegistrar.uCreatedDate,tRegistrar.uModBy,tRegistrar.uModDate"

 //Local only
void Insert_tRegistrar(void);
void Update_tRegistrar(char *cRowid);
void ProcesstRegistrarListVars(pentry entries[], int x);

 //In tRegistrarfunc.h file included below
void ExtProcesstRegistrarVars(pentry entries[], int x);
void ExttRegistrarCommands(pentry entries[], int x);
void ExttRegistrarButtons(void);
void ExttRegistrarNavBar(void);
void ExttRegistrarGetHook(entry gentries[], int x);
void ExttRegistrarSelect(void);
void ExttRegistrarSelectRow(void);
void ExttRegistrarListSelect(void);
void ExttRegistrarListFilter(void);
void ExttRegistrarAuxTable(void);

#include "tregistrarfunc.h"

 //Table Variables Assignment Function
void ProcesstRegistrarVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uRegistrar"))
			sscanf(entries[i].val,"%u",&uRegistrar);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.100s",entries[i].val);
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
	ExtProcesstRegistrarVars(entries,x);

}//ProcesstRegistrarVars()


void ProcesstRegistrarListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uRegistrar);
                        guMode=2002;
                        tRegistrar("");
                }
        }
}//void ProcesstRegistrarListVars(pentry entries[], int x)


int tRegistrarCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttRegistrarCommands(entries,x);

	if(!strcmp(gcFunction,"tRegistrarTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tRegistrarList();
		}

		//Default
		ProcesstRegistrarVars(entries,x);
		tRegistrar("");
	}
	else if(!strcmp(gcFunction,"tRegistrarList"))
	{
		ProcessControlVars(entries,x);
		ProcesstRegistrarListVars(entries,x);
		tRegistrarList();
	}

	return(0);

}//tRegistrarCommands()


void tRegistrar(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttRegistrarSelectRow();
		else
			ExttRegistrarSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetRegistrar();
				iDNS("New tRegistrar table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tRegistrar WHERE uRegistrar=%u"
						,uRegistrar);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uRegistrar);
		sprintf(cLabel,"%.100s",field[1]);
		sscanf(field[2],"%u",&uOwner);
		sscanf(field[3],"%u",&uCreatedBy);
		sscanf(field[4],"%lu",&uCreatedDate);
		sscanf(field[5],"%u",&uModBy);
		sscanf(field[6],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tRegistrar",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tRegistrarTools>");
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

        ExttRegistrarButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tRegistrar Record Data",100);

	if(guMode==2000 || guMode==2002)
		tRegistrarInput(1);
	else
		tRegistrarInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttRegistrarAuxTable();

	Footer_ism3();

}//end of tRegistrar();


void tRegistrarInput(unsigned uMode)
{

//uRegistrar
	OpenRow(LANG_FL_tRegistrar_uRegistrar,"black");
	printf("<input title='%s' type=text name=uRegistrar value=%u size=16 maxlength=10 "
,LANG_FT_tRegistrar_uRegistrar,uRegistrar);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uRegistrar value=%u >\n",uRegistrar);
	}
//cLabel
	OpenRow(LANG_FL_tRegistrar_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tRegistrar_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//uOwner
	OpenRow(LANG_FL_tRegistrar_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tRegistrar_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tRegistrar_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tRegistrar_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tRegistrar_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tRegistrarInput(unsigned uMode)


void NewtRegistrar(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uRegistrar FROM tRegistrar\
				WHERE uRegistrar=%u"
							,uRegistrar);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tRegistrar("<blink>Record already exists");
		tRegistrar(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tRegistrar();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uRegistrar=mysql_insert_id(&gMysql);
#ifdef ISM3FIELDS
	uCreatedDate=luGetCreatedDate("tRegistrar",uRegistrar);
	iDNSLog(uRegistrar,"tRegistrar","New");
#endif

	if(!uMode)
	{
	sprintf(gcQuery,LANG_NBR_NEWRECADDED,uRegistrar);
	tRegistrar(gcQuery);
	}

}//NewtRegistrar(unsigned uMode)


void DeletetRegistrar(void)
{
#ifdef ISM3FIELDS
	sprintf(gcQuery,"DELETE FROM tRegistrar WHERE uRegistrar=%u AND ( uOwner=%u OR %u>9 )"
					,uRegistrar,guLoginClient,guPermLevel);
#else
	sprintf(gcQuery,"DELETE FROM tRegistrar WHERE uRegistrar=%u"
					,uRegistrar);
#endif
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tRegistrar("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
#ifdef ISM3FIELDS
		iDNSLog(uRegistrar,"tRegistrar","Del");
#endif
		tRegistrar(LANG_NBR_RECDELETED);
	}
	else
	{
#ifdef ISM3FIELDS
		iDNSLog(uRegistrar,"tRegistrar","DelError");
#endif
		tRegistrar(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetRegistrar(void)


void Insert_tRegistrar(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tRegistrar SET uRegistrar=%u,cLabel='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uRegistrar
			,TextAreaSave(cLabel)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tRegistrar(void)


void Update_tRegistrar(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tRegistrar SET uRegistrar=%u,cLabel='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uRegistrar
			,TextAreaSave(cLabel)
			,uModBy
			,cRowid);

	mysql_query(&gMysql,gcQuery);

}//void Update_tRegistrar(void)


void ModtRegistrar(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
#ifdef ISM3FIELDS
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uRegistrar,uModDate FROM tRegistrar WHERE uRegistrar=%u"
			,uRegistrar);
#else
	sprintf(gcQuery,"SELECT uRegistrar FROM tRegistrar WHERE uRegistrar=%u"
			,uRegistrar);
#endif

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tRegistrar("<blink>Record does not exist");
	if(i<1) tRegistrar(LANG_NBR_RECNOTEXIST);
	//if(i>1) tRegistrar("<blink>Multiple rows!");
	if(i>1) tRegistrar(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
#ifdef ISM3FIELDS
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tRegistrar(LANG_NBR_EXTMOD);
#endif

	Update_tRegistrar(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
#ifdef ISM3FIELDS
	uModDate=luGetModDate("tRegistrar",uRegistrar);
	iDNSLog(uRegistrar,"tRegistrar","Mod");
#endif
	tRegistrar(gcQuery);

}//ModtRegistrar(void)


void tRegistrarList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttRegistrarListSelect();

	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tRegistrarList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttRegistrarListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uRegistrar<td><font face=arial,helvetica color=white>cLabel<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy<td><font face=arial,helvetica color=white>uModDate</tr>");



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
		long luTime4=strtoul(field[4],NULL,10);
		char cBuf4[32];
		if(luTime4)
			ctime_r(&luTime4,cBuf4);
		else
			sprintf(cBuf4,"---");
		long luTime6=strtoul(field[6],NULL,10);
		char cBuf6[32];
		if(luTime6)
			ctime_r(&luTime6,cBuf6);
		else
			sprintf(cBuf6,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[2],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[3],NULL,10))
			,cBuf4
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[5],NULL,10))
			,cBuf6
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tRegistrarList()


void CreatetRegistrar(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tRegistrar ( uRegistrar INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(100) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0 )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetRegistrar()

