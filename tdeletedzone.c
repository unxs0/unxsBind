/*
FILE
	tDeletedZone source code of iDNS.cgi
	Built by mysqlRAD2.cgi (C) Gary Wallis and Hugo Urquiza 2001-2009
	svn ID removed
PURPOSE
	tZone undo table.
AUTHOR/LEGAL
        (C) 2001-2016 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/
//git describe version info
static char *cGitVersion="GitVersion:"GitVersion;


#include "mysqlrad.h"

//Table Variables
//Table Variables
//uDeletedZone: Primary Key
static unsigned uDeletedZone=0;
//cZone: Zone name
static char cZone[101]={"domain.tld"};
//uNSSet: Pulldown of configured name server groups
static unsigned uNSSet=0;
static char cuNSSetPullDown[256]={""};
//cHostmaster: eMail address of responsible person
static char cHostmaster[101]={""};
//uSerial: Zone serial number
static unsigned uSerial=0;
//uExpire: Seconds to expire for slave name servers
static unsigned uExpire=604800;
//uRefresh: How often slaves should contact master
static unsigned uRefresh=28800;
//uTTL: Default minimum TTL for RRs
static unsigned uTTL=86400;
//uRetry: Slave server retry interval after uRefresh
static unsigned uRetry=7200;
//uZoneTTL: Zone TTL
static unsigned uZoneTTL=86400;
//uMailServers: Pulldown of configured mail server groups
static unsigned uMailServers=0;
static char cuMailServersPullDown[256]={""};
//uView: Optional tView Group
static unsigned uView=1;
static char cuViewPullDown[256]={""};
//cMainAddress: Zones main A record
static char cMainAddress[17]={""};
//uRegistrar: Registrar of Record for this Zone if Applies
static unsigned uRegistrar=0;
static char cuRegistrarPullDown[256]={""};
//uSecondaryOnly: Secondary DNS Service Only. cMasterIPs Would be External Customer Provided Master
static unsigned uSecondaryOnly=0;
static char cYesNouSecondaryOnly[32]={""};
//cMasterIPs: Slave zones master IP source(s)
static char cMasterIPs[256]={""};
//cAllowTransfer: IP address(es) that are allowed to copy the zone info from the server (master or slave)
static char cAllowTransfer[256]={""};
//cOptions: slave.zones and master.zones zone options
static char *cOptions={""};
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



#define VAR_LIST_tDeletedZone "tDeletedZone.uDeletedZone,tDeletedZone.cZone,tDeletedZone.uNSSet,tDeletedZone.cHostmaster,tDeletedZone.uSerial,tDeletedZone.uExpire,tDeletedZone.uRefresh,tDeletedZone.uTTL,tDeletedZone.uRetry,tDeletedZone.uZoneTTL,tDeletedZone.uMailServers,tDeletedZone.uView,tDeletedZone.cMainAddress,tDeletedZone.uRegistrar,tDeletedZone.uSecondaryOnly,tDeletedZone.cAllowTransfer,tDeletedZone.cOptions,tDeletedZone.uOwner,tDeletedZone.uCreatedBy,tDeletedZone.uCreatedDate,tDeletedZone.uModBy,tDeletedZone.uModDate"

 //Local only
void Insert_tDeletedZone(void);
void Update_tDeletedZone(char *cRowid);
void ProcesstDeletedZoneListVars(pentry entries[], int x);

 //In tDeletedZonefunc.h file included below
void ExtProcesstDeletedZoneVars(pentry entries[], int x);
void ExttDeletedZoneCommands(pentry entries[], int x);
void ExttDeletedZoneButtons(void);
void ExttDeletedZoneNavBar(void);
void ExttDeletedZoneGetHook(entry gentries[], int x);
void ExttDeletedZoneSelect(void);
void ExttDeletedZoneSelectRow(void);
void ExttDeletedZoneListSelect(void);
void ExttDeletedZoneListFilter(void);
void ExttDeletedZoneAuxTable(void);

#include "tdeletedzonefunc.h"

 //Table Variables Assignment Function
void ProcesstDeletedZoneVars(pentry entries[], int x)
{
	register int i;


	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uDeletedZone"))
			sscanf(entries[i].val,"%u",&uDeletedZone);
		else if(!strcmp(entries[i].name,"cZone"))
			sprintf(cZone,"%.100s",FQDomainName(entries[i].val));
		else if(!strcmp(entries[i].name,"uNSSet"))
			sscanf(entries[i].val,"%u",&uNSSet);
		else if(!strcmp(entries[i].name,"cuNSSetPullDown"))
		{
			sprintf(cuNSSetPullDown,"%.255s",entries[i].val);
			uNSSet=ReadPullDown("tNSSet","cLabel",cuNSSetPullDown);
		}
		else if(!strcmp(entries[i].name,"cHostmaster"))
			sprintf(cHostmaster,"%.100s",FQDomainName(entries[i].val));
		else if(!strcmp(entries[i].name,"uSerial"))
			sscanf(entries[i].val,"%u",&uSerial);
		else if(!strcmp(entries[i].name,"uExpire"))
			sscanf(entries[i].val,"%u",&uExpire);
		else if(!strcmp(entries[i].name,"uRefresh"))
			sscanf(entries[i].val,"%u",&uRefresh);
		else if(!strcmp(entries[i].name,"uTTL"))
			sscanf(entries[i].val,"%u",&uTTL);
		else if(!strcmp(entries[i].name,"uRetry"))
			sscanf(entries[i].val,"%u",&uRetry);
		else if(!strcmp(entries[i].name,"uZoneTTL"))
			sscanf(entries[i].val,"%u",&uZoneTTL);
		else if(!strcmp(entries[i].name,"uMailServers"))
			sscanf(entries[i].val,"%u",&uMailServers);
		else if(!strcmp(entries[i].name,"cuMailServersPullDown"))
		{
			sprintf(cuMailServersPullDown,"%.255s",entries[i].val);
			uMailServers=ReadPullDown("tMailServer","cLabel",cuMailServersPullDown);
		}
		else if(!strcmp(entries[i].name,"uView"))
			sscanf(entries[i].val,"%u",&uView);
		else if(!strcmp(entries[i].name,"cuViewPullDown"))
		{
			sprintf(cuViewPullDown,"%.255s",entries[i].val);
			uView=ReadPullDown("tView","cLabel",cuViewPullDown);
		}
		else if(!strcmp(entries[i].name,"cMainAddress"))
			sprintf(cMainAddress,"%.16s",IPNumber(entries[i].val));
		else if(!strcmp(entries[i].name,"uRegistrar"))
			sscanf(entries[i].val,"%u",&uRegistrar);
		else if(!strcmp(entries[i].name,"cuRegistrarPullDown"))
		{
			sprintf(cuRegistrarPullDown,"%.255s",entries[i].val);
			uRegistrar=ReadPullDown("tRegistrar","cLabel",cuRegistrarPullDown);
		}
		else if(!strcmp(entries[i].name,"uSecondaryOnly"))
			sscanf(entries[i].val,"%u",&uSecondaryOnly);
		else if(!strcmp(entries[i].name,"cYesNouSecondaryOnly"))
		{
			sprintf(cYesNouSecondaryOnly,"%.31s",entries[i].val);
			uSecondaryOnly=ReadYesNoPullDown(cYesNouSecondaryOnly);
		}
		else if(!strcmp(entries[i].name,"cMasterIPs"))
			sprintf(cMasterIPs,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"cAllowTransfer"))
			sprintf(cAllowTransfer,"%.255s",entries[i].val);
		else if(!strcmp(entries[i].name,"cOptions"))
			cOptions=entries[i].val;
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
	ExtProcesstDeletedZoneVars(entries,x);

}//ProcesstDeletedZoneVars()


void ProcesstDeletedZoneListVars(pentry entries[], int x)
{
        register int i;

        for(i=0;i<x;i++)
        {
                if(!strncmp(entries[i].name,"ED",2))
                {
                        sscanf(entries[i].name+2,"%u",&uDeletedZone);
                        guMode=2002;
                        tDeletedZone("");
                }
        }
}//void ProcesstDeletedZoneListVars(pentry entries[], int x)


int tDeletedZoneCommands(pentry entries[], int x)
{
	ProcessControlVars(entries,x);

	ExttDeletedZoneCommands(entries,x);

	if(!strcmp(gcFunction,"tDeletedZoneTools"))
	{
		if(!strcmp(gcFind,LANG_NB_LIST))
		{
			tDeletedZoneList();
		}

		//Default
		ProcesstDeletedZoneVars(entries,x);
		tDeletedZone("");
	}
	else if(!strcmp(gcFunction,"tDeletedZoneList"))
	{
		ProcessControlVars(entries,x);
		ProcesstDeletedZoneListVars(entries,x);
		tDeletedZoneList();
	}

	return(0);

}//tDeletedZoneCommands()


void tDeletedZone(const char *cResult)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;

	//Internal skip reloading
	if(!cResult[0])
	{
		if(guMode)
			ExttDeletedZoneSelectRow();
		else
			ExttDeletedZoneSelect();

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
        	{
			if(strstr(mysql_error(&gMysql)," doesn't exist"))
                	{
				CreatetDeletedZone();
				iDNS("New tDeletedZone table created");
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
			sprintf(gcQuery,"SELECT _rowid FROM tDeletedZone WHERE uDeletedZone=%u"
						,uDeletedZone);
				mysql_query(&gMysql,gcQuery);
				res2=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res2);
				sscanf(field[0],"%lu",&gluRowid);
				gluRowid++;
			}
			PageMachine("",0,"");
			if(!guMode) mysql_data_seek(res,gluRowid-1);
			field=mysql_fetch_row(res);
		sscanf(field[0],"%u",&uDeletedZone);
		sprintf(cZone,"%.100s",field[1]);
		sscanf(field[2],"%u",&uNSSet);
		sprintf(cHostmaster,"%.100s",field[3]);
		sscanf(field[4],"%u",&uSerial);
		sscanf(field[5],"%u",&uExpire);
		sscanf(field[6],"%u",&uRefresh);
		sscanf(field[7],"%u",&uTTL);
		sscanf(field[8],"%u",&uRetry);
		sscanf(field[9],"%u",&uZoneTTL);
		sscanf(field[10],"%u",&uMailServers);
		sscanf(field[11],"%u",&uView);
		sprintf(cMainAddress,"%.16s",field[12]);
		sscanf(field[13],"%u",&uRegistrar);
		sscanf(field[14],"%u",&uSecondaryOnly);
		sprintf(cMasterIPs,"%.255s",ForeignKey("tNSSet","cMasterIPs",uNSSet));
		sprintf(cAllowTransfer,"%.255s",field[15]);
		cOptions=field[16];
		sscanf(field[17],"%u",&uOwner);
		sscanf(field[18],"%u",&uCreatedBy);
		sscanf(field[19],"%lu",&uCreatedDate);
		sscanf(field[20],"%u",&uModBy);
		sscanf(field[21],"%lu",&uModDate);

		}

	}//Internal Skip

	Header_ism3(":: tDeletedZone",1);
	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");


	printf("<input type=hidden name=gcFunction value=tDeletedZoneTools>");
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

        ExttDeletedZoneButtons();

        printf("</td><td valign=top>");
	//
	OpenFieldSet("tDeletedZone Record Data",100);

	if(guMode==2000 || guMode==2002)
		tDeletedZoneInput(1);
	else
		tDeletedZoneInput(0);

	//
	CloseFieldSet();

	//Bottom table
	printf("<tr><td colspan=2>");
        ExttDeletedZoneAuxTable();

	Footer_ism3();

}//end of tDeletedZone();


void tDeletedZoneInput(unsigned uMode)
{

//uDeletedZone
	OpenRow(LANG_FL_tDeletedZone_uDeletedZone,"black");
	printf("<input title='%s' type=text name=uDeletedZone value=%u size=16 maxlength=10 "
			,LANG_FT_tDeletedZone_uDeletedZone,uDeletedZone);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uDeletedZone value=%u >\n",uDeletedZone);
	}
//cZone
	OpenRow(LANG_FL_tDeletedZone_cZone,EmptyString(cZone));
	printf("<input title='%s' type=text name=cZone value=\"%s\" size=40 maxlength=99 "
			,LANG_FT_tDeletedZone_cZone,EncodeDoubleQuotes(cZone));
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
	OpenRow(LANG_FL_tDeletedZone_uNSSet,IsZero(uNSSet));
	if(guPermLevel>=0 && uMode)
		tTablePullDown("tNSSet;cuNSSetPullDown","cLabel","cLabel",uNSSet,1);
	else
		tTablePullDown("tNSSet;cuNSSetPullDown","cLabel","cLabel",uNSSet,0);
//cHostmaster
	OpenRow(LANG_FL_tDeletedZone_cHostmaster,EmptyString(cHostmaster));
	printf("<input title='%s' type=text name=cHostmaster value=\"%s\" size=40 maxlength=100 "
			,LANG_FT_tDeletedZone_cHostmaster,EncodeDoubleQuotes(cHostmaster));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cHostmaster value=\"%s\">\n",EncodeDoubleQuotes(cHostmaster));
	}
//uSerial
	OpenRow(LANG_FL_tDeletedZone_uSerial,IsZero(uSerial));
	printf("<input title='%s' type=text name=uSerial value=%u size=16 maxlength=10 "
			,LANG_FT_tDeletedZone_uSerial,uSerial);
	if(guPermLevel>=20 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uSerial value=%u >\n",uSerial);
	}
//uExpire
	OpenRow(LANG_FL_tDeletedZone_uExpire,IsZero(uExpire));
	printf("<input title='%s' type=text name=uExpire value=%u size=16 maxlength=10 "
			,LANG_FT_tDeletedZone_uExpire,uExpire);
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uExpire value=%u >\n",uExpire);
	}
//uRefresh
	OpenRow(LANG_FL_tDeletedZone_uRefresh,IsZero(uRefresh));
	printf("<input title='%s' type=text name=uRefresh value=%u size=16 maxlength=10 "
			,LANG_FT_tDeletedZone_uRefresh,uRefresh);
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uRefresh value=%u >\n",uRefresh);
	}
//uTTL
	OpenRow(LANG_FL_tDeletedZone_uTTL,IsZero(uTTL));
	printf("<input title='%s' type=text name=uTTL value=%u size=16 maxlength=10 "
			,LANG_FT_tDeletedZone_uTTL,uTTL);
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uTTL value=%u >\n",uTTL);
	}
//uRetry
	OpenRow(LANG_FL_tDeletedZone_uRetry,IsZero(uRetry));
	printf("<input title='%s' type=text name=uRetry value=%u size=16 maxlength=10 "
			,LANG_FT_tDeletedZone_uRetry,uRetry);
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uRetry value=%u >\n",uRetry);
	}
//uZoneTTL
	OpenRow(LANG_FL_tDeletedZone_uZoneTTL,IsZero(uZoneTTL));
	printf("<input title='%s' type=text name=uZoneTTL value=%u size=16 maxlength=10 "
			,LANG_FT_tDeletedZone_uZoneTTL,uZoneTTL);
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=uZoneTTL value=%u >\n",uZoneTTL);
	}
//uMailServers
	OpenRow(LANG_FL_tDeletedZone_uMailServers,"black");
	if(guPermLevel>=0 && uMode)
		tTablePullDown("tMailServer;cuMailServersPullDown","cLabel","cLabel",uMailServers,1);
	else
		tTablePullDown("tMailServer;cuMailServersPullDown","cLabel","cLabel",uMailServers,0);
//uView
	OpenRow(LANG_FL_tDeletedZone_uView,"black");
	if(guPermLevel>=0 && uMode)
		tTablePullDown("tView;cuViewPullDown","cLabel","cLabel",uView,1);
	else
		tTablePullDown("tView;cuViewPullDown","cLabel","cLabel",uView,0);
//cMainAddress
	OpenRow(LANG_FL_tDeletedZone_cMainAddress,"black");
	printf("<input title='%s' type=text name=cMainAddress value=\"%s\" size=40 maxlength=16 "
		,LANG_FT_tDeletedZone_cMainAddress,EncodeDoubleQuotes(cMainAddress));
	if(guPermLevel>=0 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cMainAddress value=\"%s\">\n",EncodeDoubleQuotes(cMainAddress));
	}
//uRegistrar
	OpenRow(LANG_FL_tDeletedZone_uRegistrar,"black");
	if(guPermLevel>=0 && uMode)
		tTablePullDown("tRegistrar;cuRegistrarPullDown","cLabel","cLabel",uRegistrar,1);
	else
		tTablePullDown("tRegistrar;cuRegistrarPullDown","cLabel","cLabel",uRegistrar,0);
//uSecondaryOnly
	OpenRow(LANG_FL_tDeletedZone_uSecondaryOnly,"black");
	if(guPermLevel>=0 && uMode)
		YesNoPullDown("uSecondaryOnly",uSecondaryOnly,1);
	else
		YesNoPullDown("uSecondaryOnly",uSecondaryOnly,0);
//cMasterIPs
	OpenRow(LANG_FL_tDeletedZone_cMasterIPs,"black");
	printf("<input title='%s' type=text name=cMasterIPs value=\"%s\" size=40 maxlength=255 "
			,LANG_FT_tDeletedZone_cMasterIPs,EncodeDoubleQuotes(cMasterIPs));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cMasterIPs value=\"%s\">\n",EncodeDoubleQuotes(cMasterIPs));
	}
//cAllowTransfer
	OpenRow(LANG_FL_tDeletedZone_cAllowTransfer,"black");
	printf("<input title='%s' type=text name=cAllowTransfer value=\"%s\" size=40 maxlength=255 "
			,LANG_FT_tDeletedZone_cAllowTransfer,EncodeDoubleQuotes(cAllowTransfer));
	if(guPermLevel>=7 && uMode)
	{
		printf("></td></tr>\n");
	}
	else
	{
		printf("disabled></td></tr>\n");
		printf("<input type=hidden name=cAllowTransfer value=\"%s\">\n",EncodeDoubleQuotes(cAllowTransfer));
	}
//cOptions
	OpenRow(LANG_FL_tDeletedZone_cOptions,"black");
	printf("<textarea title='%s' cols=40 wrap=hard rows=3 name=cOptions "
			,LANG_FT_tDeletedZone_cOptions);
	if(guPermLevel>=7 && uMode)
	{
		printf(">%s</textarea></td></tr>\n",cOptions);
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",cOptions);
		printf("<input type=hidden name=cOptions value=\"%s\" >\n",EncodeDoubleQuotes(cOptions));
	}
//uOwner
	OpenRow(LANG_FL_tDeletedZone_uOwner,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
	else
	{
	printf("%s<input type=hidden name=uOwner value=%u >\n",ForeignKey(TCLIENT,"cLabel",uOwner),uOwner);
	}
//uCreatedBy
	OpenRow(LANG_FL_tDeletedZone_uCreatedBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
	else
	{
	printf("%s<input type=hidden name=uCreatedBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uCreatedBy),uCreatedBy);
	}
//uCreatedDate
	OpenRow(LANG_FL_tDeletedZone_uCreatedDate,"black");
	if(uCreatedDate)
		printf("%s\n\n",ctime(&uCreatedDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uCreatedDate value=%lu >\n",uCreatedDate);
//uModBy
	OpenRow(LANG_FL_tDeletedZone_uModBy,"black");
	if(guPermLevel>=20 && uMode)
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
	else
	{
	printf("%s<input type=hidden name=uModBy value=%u >\n",ForeignKey(TCLIENT,"cLabel",uModBy),uModBy);
	}
//uModDate
	OpenRow(LANG_FL_tDeletedZone_uModDate,"black");
	if(uModDate)
		printf("%s\n\n",ctime(&uModDate));
	else
		printf("---\n\n");
	printf("<input type=hidden name=uModDate value=%lu >\n",uModDate);
	printf("</tr>\n");



}//void tDeletedZoneInput(unsigned uMode)


void NewtDeletedZone(unsigned uMode)
{
	register int i=0;
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uDeletedZone FROM tDeletedZone\
				WHERE uDeletedZone=%u"
							,uDeletedZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	if(i) 
		//tDeletedZone("<blink>Record already exists");
		tDeletedZone(LANG_NBR_RECEXISTS);

	//insert query
	Insert_tDeletedZone();
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(gcQuery,"New record %u added");
	uDeletedZone=mysql_insert_id(&gMysql);
	uCreatedDate=luGetCreatedDate("tDeletedZone",uDeletedZone);
	iDNSLog(uDeletedZone,"tDeletedZone","New");

	if(!uMode)
	{
		sprintf(gcQuery,LANG_NBR_NEWRECADDED,uDeletedZone);
		tDeletedZone(gcQuery);
	}

}//NewtDeletedZone(unsigned uMode)


void DeletetDeletedZone(void)
{
	sprintf(gcQuery,"DELETE FROM tDeletedZone WHERE uDeletedZone=%u AND ( uOwner=%u OR %u>9 )"
					,uDeletedZone,guLoginClient,guPermLevel);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	//tDeletedZone("Record Deleted");
	if(mysql_affected_rows(&gMysql)>0)
	{
		iDNSLog(uDeletedZone,"tDeletedZone","Del");
		tDeletedZone(LANG_NBR_RECDELETED);
	}
	else
	{
		iDNSLog(uDeletedZone,"tDeletedZone","DelError");
		tDeletedZone(LANG_NBR_RECNOTDELETED);
	}

}//void DeletetDeletedZone(void)


void Insert_tDeletedZone(void)
{

	//insert query
	sprintf(gcQuery,"INSERT INTO tDeletedZone SET uDeletedZone=%u,cZone='%s',uNSSet=%u,cHostmaster='%s',uSerial=%u,"
			"uExpire=%u,uRefresh=%u,uTTL=%u,uRetry=%u,uZoneTTL=%u,uMailServers=%u,uView=%u,cMainAddress='%s',"
			"uRegistrar=%u,uSecondaryOnly=%u,cAllowTransfer='%s',cOptions='%s',uOwner=%u,uCreatedBy=%u,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uDeletedZone
			,TextAreaSave(cZone)
			,uNSSet
			,TextAreaSave(cHostmaster)
			,uSerial
			,uExpire
			,uRefresh
			,uTTL
			,uRetry
			,uZoneTTL
			,uMailServers
			,uView
			,TextAreaSave(cMainAddress)
			,uRegistrar
			,uSecondaryOnly
			,TextAreaSave(cAllowTransfer)
			,TextAreaSave(cOptions)
			,uOwner
			,uCreatedBy
			);

	mysql_query(&gMysql,gcQuery);

}//void Insert_tDeletedZone(void)


void Update_tDeletedZone(char *cRowid)
{

	//update query
	sprintf(gcQuery,"UPDATE tDeletedZone SET uDeletedZone=%u,cZone='%s',uNSSet=%u,cHostmaster='%s',uSerial=%u,"
			"uExpire=%u,uRefresh=%u,uTTL=%u,uRetry=%u,uZoneTTL=%u,uMailServers=%u,uView=%u,cMainAddress='%s',"
			"uRegistrar=%u,uSecondaryOnly=%u,cAllowTransfer='%s',cOptions='%s',uModBy=%u,"
			"uModDate=UNIX_TIMESTAMP(NOW()) WHERE _rowid=%s",
			uDeletedZone
			,TextAreaSave(cZone)
			,uNSSet
			,TextAreaSave(cHostmaster)
			,uSerial
			,uExpire
			,uRefresh
			,uTTL
			,uRetry
			,uZoneTTL
			,uMailServers
			,uView
			,TextAreaSave(cMainAddress)
			,uRegistrar
			,uSecondaryOnly
			,TextAreaSave(cAllowTransfer)
			,TextAreaSave(cOptions)
			,uModBy
			,cRowid);
	mysql_query(&gMysql,gcQuery);

}//void Update_tDeletedZone(void)


void ModtDeletedZone(void)
{
	register int i=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uPreModDate=0;

	sprintf(gcQuery,"SELECT uDeletedZone,uModDate FROM tDeletedZone WHERE uDeletedZone=%u"
			,uDeletedZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	i=mysql_num_rows(res);

	//if(i<1) tDeletedZone("<blink>Record does not exist");
	if(i<1) tDeletedZone(LANG_NBR_RECNOTEXIST);
	//if(i>1) tDeletedZone("<blink>Multiple rows!");
	if(i>1) tDeletedZone(LANG_NBR_MULTRECS);

	field=mysql_fetch_row(res);
	sscanf(field[1],"%u",&uPreModDate);
	if(uPreModDate!=uModDate) tDeletedZone(LANG_NBR_EXTMOD);

	Update_tDeletedZone(field[0]);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));
	//sprintf(query,"record %s modified",field[0]);
	sprintf(gcQuery,LANG_NBRF_REC_MODIFIED,field[0]);
	uModDate=luGetModDate("tDeletedZone",uDeletedZone);
	iDNSLog(uDeletedZone,"tDeletedZone","Mod");
	tDeletedZone(gcQuery);

}//ModtDeletedZone(void)


void tDeletedZoneList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	ExttDeletedZoneListSelect();
	mysql_query(&gMysql,gcQuery);
	if(mysql_error(&gMysql)[0]) htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	guI=mysql_num_rows(res);

	PageMachine("tDeletedZoneList",1,"");//1 is auto header list guMode. Opens table!

	//Filter select drop down
	ExttDeletedZoneListFilter();

	printf("<input type=text size=16 name=gcCommand maxlength=98 value=\"%s\" >",gcCommand);

	printf("</table>\n");

	printf("<table bgcolor=#9BC1B3 border=0 width=100%%>\n");
	printf("<tr bgcolor=black><td><font face=arial,helvetica color=white>uZone<td>"
		"<font face=arial,helvetica color=white>cZone<td><font face=arial,helvetica color=white>uNSSet"
		"<td><font face=arial,helvetica color=white>cHostmaster<td><font face=arial,helvetica color=white>uSerial"
		"<td><font face=arial,helvetica color=white>uExpire<td><font face=arial,helvetica color=white>uRefresh"
		"<td><font face=arial,helvetica color=white>uTTL<td><font face=arial,helvetica color=white>uRetry"
		"<td><font face=arial,helvetica color=white>uZoneTTL<td><font face=arial,helvetica color=white>uMailServers"
		"<td><font face=arial,helvetica color=white>uView<td><font face=arial,helvetica color=white>cMainAddress"
		"<td><font face=arial,helvetica color=white>uRegistrar"
		"<td><font face=arial,helvetica color=white>uSecondaryOnly"
		"<td><font face=arial,helvetica color=white>cMasterIPs<td><font face=arial,helvetica color=white>cOptions"
		"<td><font face=arial,helvetica color=white>uOwner<td><font face=arial,helvetica color=white>uCreatedBy"
		"<td><font face=arial,helvetica color=white>uCreatedDate<td><font face=arial,helvetica color=white>uModBy"
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
		long luYesNo14=strtoul(field[14],NULL,10);
		char cBuf14[4];
		if(luYesNo14)
			sprintf(cBuf14,"Yes");
		else
			sprintf(cBuf14,"No");
		long luTime19=strtoul(field[19],NULL,10);
		char cBuf19[32];
		if(luTime19)
			ctime_r(&luTime19,cBuf19);
		else
			sprintf(cBuf19,"---");
		long luTime21=strtoul(field[21],NULL,10);
		char cBuf21[32];
		if(luTime21)
			ctime_r(&luTime21,cBuf21);
		else
			sprintf(cBuf21,"---");
		printf("<td><input type=submit name=ED%s value=Edit> %s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s"
			"<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td>%s<td><textarea disabled>%s</textarea><td>%s<td>%s<td>%s"
			"<td>%s<td>%s</tr>"
			,field[0]
			,field[0]
			,field[1]
			,ForeignKey("tNSSet","cLabel",strtoul(field[2],NULL,10))
			,field[3]
			,field[4]
			,field[5]
			,field[6]
			,field[7]
			,field[8]
			,field[9]
			,ForeignKey("tMailServer","cLabel",strtoul(field[10],NULL,10))
			,ForeignKey("tView","cLabel",strtoul(field[11],NULL,10))
			,field[12]
			,ForeignKey("tRegistrar","cLabel",strtoul(field[13],NULL,10))
			,cBuf14
			,ForeignKey("tNSSet","cMasterIPs",strtoul(field[2],NULL,10))
			,field[16]
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[17],NULL,10))
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[18],NULL,10))
			,cBuf19
			,ForeignKey(TCLIENT,"cLabel",strtoul(field[20],NULL,10))
			,cBuf21
				);

	}

	printf("</table></form>\n");
	Footer_ism3();

}//tDeletedZoneList()


void CreatetDeletedZone(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tDeletedZone ("
			" uDeletedZone INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
			" cZone VARCHAR(100) NOT NULL DEFAULT '',"
			" uOwner INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uOwner),"
			" uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0,"
			" uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0,"
			" uModBy INT UNSIGNED NOT NULL DEFAULT 0,"
			" uModDate INT UNSIGNED NOT NULL DEFAULT 0,"
			" uNSSet INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uNSSet),"
			" cHostmaster VARCHAR(100) NOT NULL DEFAULT '',"
			" uSerial INT UNSIGNED NOT NULL DEFAULT 0,"
			" uExpire INT UNSIGNED NOT NULL DEFAULT 0,"
			" uRefresh INT UNSIGNED NOT NULL DEFAULT 0,"
			" uTTL INT UNSIGNED NOT NULL DEFAULT 0,"
			" uRetry INT UNSIGNED NOT NULL DEFAULT 0,"
			" uZoneTTL INT UNSIGNED NOT NULL DEFAULT 0,"
			" uMailServers INT UNSIGNED NOT NULL DEFAULT 0,"
			" cMainAddress VARCHAR(16) NOT NULL DEFAULT '',"
			" uView INT UNSIGNED NOT NULL DEFAULT 0,"
			" cOptions TEXT NOT NULL DEFAULT '',"
			" uSecondaryOnly INT UNSIGNED NOT NULL DEFAULT 0,"
			" uRegistrar INT UNSIGNED NOT NULL DEFAULT 0,"
			" cAllowTransfer VARCHAR(255) NOT NULL DEFAULT '',"
			" uClient INT UNSIGNED NOT NULL DEFAULT 0 )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//CreatetDeletedZone()

