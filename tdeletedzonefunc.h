/*
FILE
	svn ID removed
	(Built initially by unixservice.com mysqlRAD2)
PURPOSE
	Non schema-dependent table and application table related functions.
AUTHOR/LEGAL
	(C) 2001-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file included.
*/

//ModuleFunctionProtos()
//from tdeletedresource.c
#define VAR_LIST_tDeletedResource "tDeletedResource.uDeletedResource,tDeletedResource.uZone,tDeletedResource.cName,tDeletedResource.uTTL,tDeletedResource.uRRType,tDeletedResource.cParam1,tDeletedResource.cParam2,tDeletedResource.cComment,tDeletedResource.uOwner"

static char cSearch[100]={""};
static unsigned uRestoreRR=0;

void tDeletedZoneNavList(void);


void ExtProcesstDeletedZoneVars(pentry entries[], int x)
{
	
	register int i;
	for(i=0;i<x;i++)
	{
                if(!strcmp(entries[i].name,"cSearch"))
                        sprintf(cSearch,"%.99s",entries[i].val);
		if(!strcmp(entries[i].name,"uRestoreRR"))
			sscanf(entries[i].val,"%u",&uRestoreRR);

	}
	
}//void ExtProcesstDeletedZoneVars(pentry entries[], int x)


void ExttDeletedZoneCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tDeletedZoneTools"))
	{
		//ModuleFunctionProcess()

		/*
		NOTE: we should not allow adding records at this table
		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=7)
			{
	                        ProcesstDeletedZoneVars(entries,x);
                        	guMode=2000;
	                        tDeletedZone(LANG_NB_CONFIRMNEW);
			}
			else
				tDeletedZone("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=7)
			{
                        	ProcesstDeletedZoneVars(entries,x);

                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uDeletedZone=0;
				uCreatedBy=guLoginClient;
				uOwner=guCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtDeletedZone(0);
			}
			else
				tDeletedZone("<blink>Error</blink>: Denied by permissions settings");
		}
		*/
		if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstDeletedZoneVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tDeletedZone(LANG_NB_CONFIRMDEL);
			}
			else
				tDeletedZone("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstDeletedZoneVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetDeletedZone();
			}
			else
				tDeletedZone("<blink>Error</blink>: Denied by permissions settings");
                }
		/*
		NOTE: we should not allow modyfing records
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstDeletedZoneVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tDeletedZone(LANG_NB_CONFIRMMOD);
			}
			else
				tDeletedZone("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstDeletedZoneVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtDeletedZone();
			}
			else
				tDeletedZone("<blink>Error</blink>: Denied by permissions settings");
                }
		*/
		else if(!strcmp(gcCommand,"Restore Zone"))
		{
			ProcesstDeletedZoneVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=4000;
				tDeletedZone("Double check that you want to restore the selected zone and confirm.");
			}
			else
				tDeletedZone("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,"Confirm Zone Restore"))
		{
			ProcesstDeletedZoneVars(entries,x);

			if(!uAllowMod(uOwner,uCreatedBy))
				tDeletedZone("<blink>Error</blink>: Denied by permissions settings");
			//
			//Restore tZone record
			sprintf(gcQuery,"INSERT INTO tZone SET uZone=%u,cZone='%s',uNSSet=%u,cHostmaster='%s',"
					"uSerial=%u,uExpire=%u,uRefresh=%u,uTTL=%u,uRetry=%u,uZoneTTL=%u,uMailServers=%u,"
					"uView=%u,cMainAddress='%s',uRegistrar=%u,uSecondaryOnly=%u,cOptions='%s',uOwner=%u,"
					"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					uDeletedZone
					,cZone
					,uNSSet
					,cHostmaster
					,uSerial
					,uExpire
					,uRefresh
					,uTTL
					,uRetry
					,uZoneTTL
					,uMailServers
					,uView
					,cMainAddress
					,uRegistrar
					,uSecondaryOnly
					,cOptions
					,uOwner
					);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));

			iDNSLog(uDeletedZone,"tZone","New (Restore Zone)");

			//
			//Restore tResource record(s) if available
			sprintf(gcQuery,"INSERT INTO tResource (uResource,uZone,cName,uTTL,uRRType,cParam1,"
					"cParam2,cParam3,cParam4,cComment,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate) "
					"SELECT uDeletedResource,uZone,cName,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4,"
					"cComment,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate FROM tDeletedResource "
					"WHERE uZone=%u",uDeletedZone);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
				
			iDNSLog(uDeletedZone,"tResource","Restored zone RR");	
			//
			//Now remove tDeletedZone and tDeletedResource records
			sprintf(gcQuery,"DELETE FROM tDeletedZone WHERE uDeletedZone=%u",uDeletedZone);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
			
			sprintf(gcQuery,"DELETE FROM tDeletedResource WHERE uZone=%u",uDeletedZone);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
			//
			//Submit job to restore zone all across the cluster
			if(SubmitJob("New",uNSSet,cZone,0,0))
				htmlPlainTextError(mysql_error(&gMysql));

			tDeletedZone("Zone restored");
		}
	}

}//void ExttDeletedZoneCommands(pentry entries[], int x)


void ExttDeletedZoneButtons(void)
{
	unsigned uDefault=0;
	OpenFieldSet("tDeletedZone Aux Panel",100);
	switch(guMode)
        {
                case 2000:
			printf("<p><u>Enter/mod data</u><br>");
                        printf(LANG_NBB_CONFIRMNEW);
                break;

                case 2001:
                        printf("<p><u>Think twice</u><br>");
                        printf(LANG_NBB_CONFIRMDEL);
                break;

                case 2002:
			printf("<p><u>Review changes</u><br>");
                        printf(LANG_NBB_CONFIRMMOD);
                break;
		case 4000:
			printf("<p><u>Restore Zone Confirm</u><br><br>");
			printf("<input class=largeButton title='' type=submit name=gcCommand value='Confirm Zone Restore'>\n");
		break;
		default:
			uDefault=1;
			printf("<u>Search Tools</u><br>");
			printf("<input type=text title='cZone, cMainAddr and uOwner=2000 search. Use %% . "
				"and _ for pattern matching when applicable.' name=cSearch value=\"%s\" maxlength=99 size=20>",cSearch);
			tDeletedZoneNavList();

			if(guLoginClient==uOwner || guReseller==guLoginClient 
						|| guPermLevel>9)
			{
				printf("<p><u>Deleted Zone Management Tools</u><br><br>");
				if(uDeletedZone)
				printf("<input class=largeButton title='' type=submit name=gcCommand value='Restore Zone'>\n");
			}

	}

	if(!uDefault && cSearch[0])
		printf("<input type=hidden name=cSearch value=\"%s\">",cSearch);
	
	CloseFieldSet();
	
}//void ExttDeletedZoneButtons(void)


void ExttDeletedZoneAuxTable(void)
{

}//void ExttDeletedZoneAuxTable(void)


void ExttDeletedZoneGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uDeletedZone"))
		{
			sscanf(gentries[i].val,"%u",&uDeletedZone);
			guMode=6;
		}
	}
	tDeletedZone("");

}//void ExttDeletedZoneGetHook(entry gentries[], int x)


void ExttDeletedZoneSelect(void)
{
	if(cSearch[0])
		ExtSelectSearch("tDeletedZone",VAR_LIST_tDeletedZone,"cZone",cSearch,NULL,20);
	else
		ExtSelect("tDeletedZone",VAR_LIST_tDeletedZone,0);

}//void ExttDeletedZoneSelect(void)


void ExttDeletedZoneSelectRow(void)
{
	ExtSelectRow("tDeletedZone",VAR_LIST_tDeletedZone,uDeletedZone);

}//void ExttDeletedZoneSelectRow(void)


void ExttDeletedZoneListSelect(void)
{
	char cCat[512];

	ExtListSelect("tDeletedZone",VAR_LIST_tDeletedZone);

	//Changes here must be reflected below in ExttDeletedZoneListFilter()
        if(!strcmp(gcFilter,"uDeletedZone"))
        {
                sscanf(gcCommand,"%u",&uDeletedZone);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tDeletedZone.uDeletedZone=%u \
						ORDER BY uDeletedZone",
						uDeletedZone);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uDeletedZone");
        }

}//void ExttDeletedZoneListSelect(void)


void ExttDeletedZoneListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uDeletedZone"))
                printf("<option>uDeletedZone</option>");
        else
                printf("<option selected>uDeletedZone</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttDeletedZoneListFilter(void)


void ExttDeletedZoneNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

//	if(guPermLevel>=10 && !guListMode)
//		printf(LANG_NBB_NEW);
	
//	if(uAllowMod(uOwner,uCreatedBy))
//		printf(LANG_NBB_MODIFY);
	
	if(uAllowDel(uOwner,uCreatedBy))
		printf(LANG_NBB_DELETE);

	if(uOwner)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttDeletedZoneNavBar(void)


void tDeletedZoneNavList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	printf("<br><u>tDeletedZoneNavList</u><br>\n");

	if(!cSearch[0])
	{
		printf("Must restrict via cSearch\n");
		return;
	}
	ExtSelectSearch("tDeletedZone",
			"tDeletedZone.uDeletedZone,tDeletedZone.cZone,(SELECT tView.cLabel FROM tView"
			" WHERE tView.uView=tDeletedZone.uView)",
			"cZone",
			cSearch,
			NULL,
			20);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);
	
	if(!mysql_num_rows(res))
		printf("No records found<br>\n");

	while((field=mysql_fetch_row(res)))
		printf("<a class=darkLink href=?gcFunction=tDeletedZone&uDeletedZone=%s>%s [%s]</a><br>",
			field[0],field[1],field[2]);
	
	mysql_free_result(res);

}//void tDeletedZoneNavList(void)

