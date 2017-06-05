/*
FILE
	tAuthorize source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
PURPOSE
	User rols based authentication table with Google Authenticator OTP support.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;



#include "mysqlrad.h"

//Table Variables
//Table Variables
//uAuthorize: Primary Key
static unsigned uAuthorize=0;
//cLabel: Short label
static char cLabel[33]={""};
//cIpMask: Allow user from this IP
static char cIpMask[21]={"0.0.0.0/0"};
//uPerm: User permission level
static unsigned uPerm=0;
//uCertClient: User uClient
static unsigned uCertClient=0;
//cPasswd: Encrypted cgi login password
static char cPasswd[101]={""};
//cClrPasswd: Optionally used non encrypted login password
static char cClrPasswd[33]={""};
//uOwner: Record owner
static unsigned uOwner=0;
//uCreatedBy: uClient for last insert
static unsigned uCreatedBy=0;
//uCreatedDate: Unix seconds date last insert
static time_t uCreatedDate=0;
//uModBy: uClient for last update
static unsigned uModBy=0;
//uModDate: Unix seconds date last update
static time_t uModDate=0;
//cOTPSecret: OATH TOPT or HOPT key
static char cOTPSecret[65]={""};
//uOTPExpire: Unix seconds date last when cOTPSecret is no longer valid. If 0 do not
//require OTP
static time_t uOTPExpire=0;


#define VAR_LIST_tAuthorize "tAuthorize.uAuthorize,tAuthorize.cLabel,tAuthorize.cIpMask,tAuthorize.uPerm,tAuthorize.uCertClient,tAuthorize.cPasswd,tAuthorize.cClrPasswd,tAuthorize.uOwner,tAuthorize.uCreatedBy,tAuthorize.uCreatedDate,tAuthorize.uModBy,tAuthorize.uModDate,tAuthorize.cOTPSecret,tAuthorize.uOTPExpire"



 //Local only
void Insert_tAuthorize(void);
void Update_tAuthorize(char *cRowid);
void ProcesstAuthorizeListVars(pentry entries[], int x);

 //In tAuthorizefunc.h file included below
void ExtProcesstAuthorizeVars(pentry entries[], int x);
void ExttAuthorizeCommands(pentry entries[], int x);
void ExttAuthorizeButtons(void);
void ExttAuthorizeNavBar(void);
void ExttAuthorizeGetHook(entry gentries[], int x);
void ExttAuthorizeSelect(void);
void ExttAuthorizeSelectRow(void);
void ExttAuthorizeListSelect(void);
void ExttAuthorizeListFilter(void);
void ExttAuthorizeAuxTable(void);

#include "tauthorizefunc.h"

 //Table Variables Assignment Function
void ProcesstAuthorizeVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uAuthorize"))
			sscanf(entries[i].val,"%u",&uAuthorize);
		else if(!strcmp(entries[i].name,"cLabel"))
			sprintf(cLabel,"%.32s",entries[i].val);
		else if(!strcmp(entries[i].name,"cIpMask"))
			sprintf(cIpMask,"%.20s",entries[i].val);
		else if(!strcmp(entries[i].name,"uPerm"))
			sscanf(entries[i].val,"%u",&uPerm);
		else if(!strcmp(entries[i].name,"uCertClient"))
			sscanf(entries[i].val,"%u",&uCertClient);
		else if(!strcmp(entries[i].name,"cPasswd"))
			sprintf(cPasswd,"%.100s",entries[i].val);
		else if(!strcmp(entries[i].name,"cClrPasswd"))
			sprintf(cClrPasswd,"%.32s",entries[i].val);
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
		else if(!strcmp(entries[i].name,"cOTPSecret"))
			sprintf(cOTPSecret,"%.64s",entries[i].val);
		else if(!strcmp(entries[i].name,"uOTPExpire"))
			sscanf(entries[i].val,"%lu",&uOTPExpire);

	}

	//After so we can overwrite form data if needed.
	ExtProcesstAuthorizeVars(entries,x);

}//ProcesstAuthorizeVars()


void ProcesstAuthorizeListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uAuthorize);
                        guMode=2002;
                        tAuthorize("");
                }
        }
}//void ProcesstAuthorizeListVars(pentry entries[], int x)


int tAuthorizeCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttAuthorizeCommands(entries,x);

	if(!strcmp(gcFunction,"tAuthorizeTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tAuthorizeList();
		}

		//Default
		ProcesstAuthorizeVars(entries,x);
		tAuthorize("");
	}
	else if(!strcmp(gcFunction,"tAuthorizeList"))
	{
		ProcessControlVars(entries,x);
		ProcesstAuthorizeListVars(entries,x);
		tAuthorizeList();
	}

	return(0);

}//tAuthorizeCommands()


void tAuthorize(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttAuthorizeSelectRow();
		else
			ExttAuthorizeSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetAuthorize();
				iDNS("New tAuthorize table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tAuthorize WHERE uAuthorize=%u"
						,uAuthorize);
				macro_mySQLRunAndStore(res2);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uAuthorize);
		sprintf(cLabel,"%.32s",field[1]);
		sprintf(cIpMask,"%.20s",field[2]);
		sscanf(field[3],"%u",&uPerm);
		sscanf(field[4],"%u",&uCertClient);
		sprintf(cPasswd,"%.100s",field[5]);
		sprintf(cClrPasswd,"%.32s",field[6]);
		sscanf(field[7],"%u",&uOwner);
		sscanf(field[8],"%u",&uCreatedBy);
		sscanf(field[9],"%lu",&uCreatedDate);
		sscanf(field[10],"%u",&uModBy);
		sscanf(field[11],"%lu",&uModDate);
		sprintf(cOTPSecret,"%.64s",field[12]);
		sscanf(field[13],"%lu",&uOTPExpire);

		}

	}//Internal Skip

	Header_ism3(":: tAuthorize",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tAuthorizeTools>");
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

        ExttAuthorizeButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tAuthorize Record Data",100);

	if(guMode==2000 || guMode==2002)
		tAuthorizeInput(1);
	else
		tAuthorizeInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttAuthorizeAuxTable();

	Footer_ism3();

}//end of tAuthorize();


void tAuthorizeInput(unsigned uMode)
{

//uAuthorize
	OpenRow(LANG_FL_tAuthorize_uAuthorize,"black");
	printf("<input title='%s' type=text name=uAuthorize value=%u size=16 maxlength=10 "
,LANG_FT_tAuthorize_uAuthorize,uAuthorize);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uAuthorize value=%u >\n",uAuthorize);
	}
//cLabel
	OpenRow(LANG_FL_tAuthorize_cLabel,"black");
	printf("<input title='%s' type=text name=cLabel value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tAuthorize_cLabel,EncodeDoubleQuotes(cLabel));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cLabel value=\"%s\">\n",EncodeDoubleQuotes(cLabel));
	}
//cIpMask
	OpenRow(LANG_FL_tAuthorize_cIpMask,"black");
	printf("<input title='%s' type=text name=cIpMask value=\"%s\" size=40 maxlength=20 "
,LANG_FT_tAuthorize_cIpMask,EncodeDoubleQuotes(cIpMask));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cIpMask value=\"%s\">\n",EncodeDoubleQuotes(cIpMask));
	}
//uPerm
	OpenRow(LANG_FL_tAuthorize_uPerm,"black");
	printf("<input title='%s' type=text name=uPerm value=%u size=16 maxlength=10 "
,LANG_FT_tAuthorize_uPerm,uPerm);
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uPerm value=%u >\n",uPerm);
	}
//uCertClient
	OpenRow(LANG_FL_tAuthorize_uCertClient,"black");
	printf("<input title='%s' type=text name=uCertClient value=%u size=16 maxlength=10 "
,LANG_FT_tAuthorize_uCertClient,uCertClient);
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uCertClient value=%u >\n",uCertClient);
	}
//cPasswd
	OpenRow(LANG_FL_tAuthorize_cPasswd,"black");
	printf("<input title='%s' type=text name=cPasswd value=\"%s\" size=40 maxlength=100 "
,LANG_FT_tAuthorize_cPasswd,EncodeDoubleQuotes(cPasswd));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cPasswd value=\"%s\">\n",EncodeDoubleQuotes(cPasswd));
	}
//cClrPasswd
	OpenRow(LANG_FL_tAuthorize_cClrPasswd,"black");
	printf("<input title='%s' type=text name=cClrPasswd value=\"%s\" size=40 maxlength=32 "
,LANG_FT_tAuthorize_cClrPasswd,EncodeDoubleQuotes(cClrPasswd));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cClrPasswd value=\"%s\">\n",EncodeDoubleQuotes(cClrPasswd));
	}
//uOwner
	OpenRow(LANG_FL_tAuthorize_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey("tClient","cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey("tClient","cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tAuthorize_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey("tClient","cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey("tClient","cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tAuthorize_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tAuthorize_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey("tClient","cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey("tClient","cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tAuthorize_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
#ifdef LIBOATH
//cOTPSecret
	OpenRow("cOTPSecret","black");
	if(guPermLevel<12 && uPerm>10)
	{
		 printf("private");
	}
	else
	{
		printf("<input title='%s' type=text name=cOTPSecret value=\"%s\" size=40 maxlength=64 ",
						"liboath TOTP or HTOP base32 key",cOTPSecret);
		if(guPermLevel>=0 && uMode)
		{
			printf("></td></tr>\n");
		}
		else
		{
			printf("disabled>");
			if(cOTPSecret[0] && cLabel[0])
			{
	
				char *cp;
				if((cp=strchr(cLabel,' ')))
					*cp=0;
				printf(" <a href=\"https://www.google.com/chart?chs=200x200&chld=M|0&cht=qr&chl="
						"otpauth://totp/%.31s%%3Fsecret%%3D%.20s\">QRCode link (load twice)</a>",
								cLabel,cOTPSecret);
				char *secret;
				size_t secretlen=0;
				int rc;
				char otp[10];
				time_t now=time(NULL);
	
				rc=oath_init();
				if(rc==OATH_OK)
				{
					rc=oath_base32_decode(cOTPSecret,strlen(cOTPSecret),&secret,&secretlen);
					if(rc==OATH_OK)
					{
						rc=oath_totp_generate(secret,secretlen,now,30,0,6,otp);
						if(rc==OATH_OK)
							printf(" Validation code: %s",otp);
					}
				}
			}
			printf("</td></tr>\n");
			printf("<input type=hidden name=cOTPSecret value=\"%s\">\n",cOTPSecret);
		}
	}

//uOTPExpire
	OpenRow("uOTPExpire","black");
	if(uOTPExpire)
	{
		time_t timeNow=time(NULL);
		printf("%s %lus left\n\n",ctime(&uOTPExpire),(uOTPExpire-timeNow));
	}
	else
		printf("---\n\n");
#endif
	printf("</tr>\n");



}//void tAuthorizeInput(unsigned uMode)


void NewtAuthorize(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uAuthorize FROM tAuthorize WHERE uAuthorize=%u"
							,uAuthorize);
	macro_mySQLRunAndStore(res);
	i=mysql_num_rows(res);

	if(i) 
		//tAuthorize("<blink>Record already exists");
		tAuthorize(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tAuthorize();
	//sprintf(gcQuery,"New record %u added");
	uAuthorize=mysql_insert_id(&gMysql);
	uCreatedDate=luGetCreatedDate("tAuthorize",uAuthorize);
	iDNSLog(uAuthorize,"tAuthorize","New");

	if(!uMode)
	{
		sprintf(gcQuery,LANG_NBR_NEWRECADDED,uAuthorize);
		tAuthorize(gcQuery);
	}

}//NewtAuthorize(unsigned uMode)


void DeletetAuthorize(void)
{
	sprintf(gcQuery,"DELETE FROM tAuthorize WHERE uAuthorize=%u AND ( uOwner=%u OR %u>9 )"
					,uAuthorize,guLoginClient,guPermLevel);
	macro_mySQLQueryHTMLError;
	//tAuthorize("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
		iDNSLog(uAuthorize,"tAuthorize","Del");
		tAuthorize(LANG_NBR_RECDELETED);
	}
	else
	{
		iDNSLog(uAuthorize,"tAuthorize","DelError");
		tAuthorize(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetAuthorize(void)


void Insert_tAuthorize(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tAuthorize SET uAuthorize=%u,cLabel='%s',cIpMask='%s',uPerm=%u,uCertClient=%u,cPasswd='%s',cClrPasswd='%s',uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW()),cOTPSecret='%s',uOTPExpire=%lu",
			uAuthorize
			,TextAreaSave(cLabel)
			,TextAreaSave(cIpMask)
			,uPerm
			,uCertClient
			,TextAreaSave(cPasswd)
			,TextAreaSave(cClrPasswd)
			,uOwner
			,uCreatedBy
			,TextAreaSave(cOTPSecret)
			,uOTPExpire
			);

	macro_mySQLQueryHTMLError;
	
}//void Insert_tAuthorize(void)


void Update_tAuthorize(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tAuthorize SET uAuthorize=%u,cLabel='%s',cIpMask='%s',uPerm=%u,uCertClient=%u,cPasswd='%s',cClrPasswd='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()),cOTPSecret='%s',uOTPExpire=%lu WHERE _rowid=%s",
			uAuthorize
			,TextAreaSave(cLabel)
			,TextAreaSave(cIpMask)
			,uPerm
			,uCertClient
			,TextAreaSave(cPasswd)
			,TextAreaSave(cClrPasswd)
			,uModBy
			,TextAreaSave(cOTPSecret)
			,uOTPExpire
			,cRowid);
	macro_mySQLQueryHTMLError;

}//void Update_tAuthorize(void)


void ModtAuthorize(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uPreModDate=0;

	//Mod select gcQuery
	if(guPermLevel<10)
	sprintf(gcQuery,"SELECT tAuthorize.uAuthorize,\
				tAuthorize.uModDate\
				FROM tAuthorize,tClient\
				WHERE tAuthorize.uAuthorize=%u\
				AND tAuthorize.uOwner=tClient.uClient\
				AND (tClient.uOwner=%u OR tClient.uClient=%u)"
			,uAuthorize,guLoginClient,guLoginClient);
	else
	sprintf(gcQuery,"SELECT uAuthorize,uModDate FROM tAuthorize\
				WHERE uAuthorize=%u"
						,uAuthorize);
	macro_mySQLRunAndStore(res);
	i=mysql_num_rows(res);

	//if(i<1) tAuthorize("<blink>Record does not exist");
	if(i<1) tAuthorize(LANG_NBR_RECNOTEXIST);
	//if(i>1) tAuthorize("<blink>Multiple rows!");
	if(i>1) tAuthorize(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tAuthorize(LANG_NBR_EXTMOD);

	Update_tAuthorize(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
	uModDate=luGetModDate("tAuthorize",uAuthorize);
	iDNSLog(uAuthorize,"tAuthorize","Mod");
	tAuthorize(gcQuery);

}//ModtAuthorize(void)


void tAuthorizeList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttAuthorizeListSelect();
	macro_mySQLRunAndStore(res);
	guI=mysql_num_rows(res);

	PageMachine("tAuthorizeList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttAuthorizeListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black>"
		"<td><font face=arial,helvetica color=white>uAuthorize"
		"<td><font face=arial,helvetica color=white>cLabel"
		"<td><font face=arial,helvetica color=white>cIpMask"
		"<td><font face=arial,helvetica color=white>uPerm"
		"<td><font face=arial,helvetica color=white>uCertClient"
		"<td><font face=arial,helvetica color=white>cPasswd"
		"<td><font face=arial,helvetica color=white>cClrPasswd"
		"<td><font face=arial,helvetica color=white>uOwner"
		"<td><font face=arial,helvetica color=white>uCreatedBy"
		"<td><font face=arial,helvetica color=white>uCreatedDate"
		"<td><font face=arial,helvetica color=white>uModBy"
		"<td><font face=arial,helvetica color=white>uModDate"
		"<td><font face=arial,helvetica color=white>cOTPSecret"
		"<td><font face=arial,helvetica color=white>uOTPExpire"
						"</tr>");



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
		time_t luTime9=strtoul(field[9],NULL,10);
		char cBuf9[32];
		if(luTime9)
			ctime_r(&luTime9,cBuf9);
		else
			sprintf(cBuf9,"---");
		time_t luTime11=strtoul(field[11],NULL,10);
		char cBuf11[32];
		if(luTime11)
			ctime_r(&luTime11,cBuf11);
		else
			sprintf(cBuf11,"---");
		time_t luTime13=strtoul(field[13],NULL,10);
		char cBuf13[32];
		if(luTime13)
			ctime_r(&luTime13,cBuf13);
		else
			sprintf(cBuf13,"---");
		printf("<td><a class=darkLink href=?gcFunction=tAuthorize&uAuthorize=%s>"
                	" %s<td>%s<td><textarea disabled>%s</textarea><td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,field[2]
			,field[3]
			,field[4]
			,field[5]
			,field[6]
			,ForeignKey("tClient","cLabel",strtoul(field[7],NULL,10))
			,ForeignKey("tClient","cLabel",strtoul(field[8],NULL,10))
			,cBuf9
			,ForeignKey("tClient","cLabel",strtoul(field[10],NULL,10))
			,cBuf11
			,field[12]
			,cBuf13
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tAuthorizeList()


void CreatetAuthorize(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tAuthorize ( "
			"uAuthorize INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
			"cLabel VARCHAR(32) NOT NULL DEFAULT '',"
			"UNIQUE (cLabel,uOwner),"
			"uOwner INT UNSIGNED NOT NULL DEFAULT 0,"
			"INDEX (uOwner),"
			"uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0,"
			"uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0,"
			"uModBy INT UNSIGNED NOT NULL DEFAULT 0,"
			"uModDate INT UNSIGNED NOT NULL DEFAULT 0,"
			"cIpMask VARCHAR(20) NOT NULL DEFAULT '',"
			"uPerm INT UNSIGNED NOT NULL DEFAULT 0,"
			"uCertClient INT UNSIGNED NOT NULL DEFAULT 0,"
			"cPasswd VARCHAR(100) NOT NULL DEFAULT '',"
			"cClrPasswd VARCHAR(32) NOT NULL DEFAULT '',"
			"cOTPSecret VARCHAR(64) NOT NULL DEFAULT '',"
			"uOTPExpire INT UNSIGNED NOT NULL DEFAULT 0 )");
	macro_mySQLQueryHTMLError;

}//CreatetAuthorize()

