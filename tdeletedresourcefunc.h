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


static char cSearch[100]={""};
void tDeletedResourceNavList(void);
void DeletedResourceLinks(unsigned uZone);
unsigned ZoneExists(unsigned uZone);

void ExtProcesstDeletedResourceVars(pentry entries[], int x)
{
	register int i;
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cSearch"))
			sprintf(cSearch,"%.99s",entries[i].val);
	
	}
	
}//void ExtProcesstDeletedResourceVars(pentry entries[], int x)


void ExttDeletedResourceCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tDeletedResourceTools"))
	{
		//ModuleFunctionProcess()
		/*
		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=7)
			{
	                        ProcesstDeletedResourceVars(entries,x);
                        	guMode=2000;
	                        tDeletedResource(LANG_NB_CONFIRMNEW);
			}
			else
				tDeletedResource("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=7)
			{
                        	ProcesstDeletedResourceVars(entries,x);

                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uDeletedResource=0;
				uCreatedBy=guLoginClient;
				uOwner=guCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtDeletedResource(0);
			}
			else
				tDeletedResource("<blink>Error</blink>: Denied by permissions settings");
		}
		*/
		if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstDeletedResourceVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tDeletedResource(LANG_NB_CONFIRMDEL);
			}
			else
				tDeletedResource("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstDeletedResourceVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetDeletedResource();
			}
			else
				tDeletedResource("<blink>Error</blink>: Denied by permissions settings");
                }
		/*
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstDeletedResourceVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tDeletedResource(LANG_NB_CONFIRMMOD);
			}
			else
				tDeletedResource("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtDeletedResource();
			}
			else
				tDeletedResource("<blink>Error</blink>: Denied by permissions settings");
		}
		*/
		else if(!strcmp(gcCommand,"Restore RR"))
		{
			ProcesstDeletedResourceVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=4000;
				tDeletedResource("Double check that you want to restore the selected RR and confirm.");
			}
			else
				tDeletedResource("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,"Confirm RR Restore"))
		{
			unsigned uNSSet=0;
			char cZone[200]={""};

			//
			//Check if the zone exists
			//Insert the tResource record
			//Delete the tDeletedResource record
			//Submit a zone modify job
			ProcesstDeletedResourceVars(entries,x);
			if(!uAllowMod(uOwner,uCreatedBy))
				tDeletedResource("<blink>Error</blink>: Denied by permissions settings");

			if(!ZoneExists(uZone))
				tDeletedResource("<blink>The zone associated with the RR does not exist at tZone."
						" Can't restore RR</blink>");
			else
			{
				//get zone data for job submission
				MYSQL_RES *res;
				MYSQL_ROW field;

				sprintf(gcQuery,"SELECT uNSSet,cZone FROM tZone WHERE uZone=%u",uZone);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
				res=mysql_store_result(&gMysql);
				field=mysql_fetch_row(res);
				sscanf(field[0],"%u",&uNSSet);
				sprintf(cZone,"%s",field[1]);
				mysql_free_result(res);
			}
				
			sprintf(gcQuery,"INSERT INTO tResource SET uResource=%u,uZone=%u,cName='%s',uTTL=%u,"
					"uRRType=%u,cParam1='%s',cParam2='%s',cComment='%s',uOwner=%u,uCreatedBy=1,"
					"uCreatedDate=UNIX_TIMESTAMP(NOW())",
						uDeletedResource
						,uZone
						,cName
						,uTTL
						,uRRType
						,cParam1
						,cParam2
						,cComment
						,uOwner);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));

			iDNSLog(uDeletedResource,"tResource","New (Restore RR)");
			
			if(SubmitJob("New",uNSSet,cZone,0,0))
				htmlPlainTextError(mysql_error(&gMysql));

			sprintf(gcQuery,"DELETE FROM tDeletedResource WHERE uDeletedResource=%u",uDeletedResource);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
				
			
			tDeletedResource("RR restored correctly");	
                }
	}

}//void ExttDeletedResourceCommands(pentry entries[], int x)


void ExttDeletedResourceButtons(void)
{
	unsigned uDefault=0;
	
	OpenFieldSet("tDeletedResource Aux Panel",100);
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
			printf("<p><u>Restore RR Confirm</u><br><br>");
			printf("<input class=largeButton title='' type=submit name=gcCommand value='Confirm RR Restore'>\n");
		break;
		default:

			uDefault=1;
			if(uDeletedResource)
				printf("<input type=submit name=gcCommand value='Restore RR'><br>\n");
			if(uZone)
			{
				printf("<p><u>RRNavList</u><br>");
				DeletedResourceLinks(uZone);
			}


	}

	if(!uDefault && cSearch[0])
		printf("<input type=hidden name=cSearch value=\"%s\">",cSearch);
	
	CloseFieldSet();

}//void ExttDeletedResourceButtons(void)


void ExttDeletedResourceAuxTable(void)
{

}//void ExttDeletedResourceAuxTable(void)


void ExttDeletedResourceGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uDeletedResource"))
		{
			sscanf(gentries[i].val,"%u",&uDeletedResource);
			guMode=6;
		}
	}
	tDeletedResource("");

}//void ExttDeletedResourceGetHook(entry gentries[], int x)


void ExttDeletedResourceSelect(void)
{
        //Set non search query here for tTableName()
	if(cSearch[0])
		ExtSelectSearch("tDeletedResource",VAR_LIST_tDeletedResource,"cName",cSearch,NULL,20);
	else
		ExtSelect("tDeletedResource",VAR_LIST_tDeletedResource,0);

}//void ExttDeletedResourceSelect(void)


void ExttDeletedResourceSelectRow(void)
{
	ExtSelectRow("tDeletedResource",VAR_LIST_tDeletedResource,uDeletedResource);

}//void ExttDeletedResourceSelectRow(void)


void ExttDeletedResourceListSelect(void)
{
	char cCat[512];

	ExtListSelect("tDeletedResource",VAR_LIST_tDeletedResource);

	//Changes here must be reflected below in ExttDeletedResourceListFilter()
        if(!strcmp(gcFilter,"uDeletedResource"))
        {
                sscanf(gcCommand,"%u",&uDeletedResource);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tDeletedResource.uDeletedResource=%u ORDER BY uDeletedResource",uDeletedResource);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uDeletedResource");
        }

}//void ExttDeletedResourceListSelect(void)


void ExttDeletedResourceListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uDeletedResource"))
                printf("<option>uDeletedResource</option>");
        else
                printf("<option selected>uDeletedResource</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttDeletedResourceListFilter(void)


void ExttDeletedResourceNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(guPermLevel>=10 && !guListMode)
		printf(LANG_NBB_NEW);
	
	if(uAllowMod(uOwner,uCreatedBy))
		printf(LANG_NBB_MODIFY);
	
	if(uAllowDel(uOwner,uCreatedBy))
		printf(LANG_NBB_DELETE);

	if(uOwner)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttDeletedResourceNavBar(void)


void DeletedResourceLinks(unsigned uZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cExtra[64]={""};

	//if(!uZone) return;}

	sprintf(cExtra,"tDeletedResource.uZone=%u",uZone);

	ExtSelectSearch("tDeletedResource",
			"tDeletedResource.cName,tDeletedResource.uDeletedResource,"
			" (SELECT tRRType.cLabel FROM tRRType WHERE"
			" tRRType.uRRType=tDeletedResource.uRRType)",
			NULL,
			NULL,
			cExtra,0);
	
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);

	while((field=mysql_fetch_row(res)))
		printf("<a class=darkLink href=?gcFunction=tDeletedResource&uDeletedResource=%s>%s %s</a><br>\n",
			field[1],field[0],field[2]);

}//void ResourceLinks(unsigned uZone)


unsigned ZoneExists(unsigned uZone)
{
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uNSSet,cZone FROM tZone WHERE uZone=%u",uZone);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        	htmlPlainTextError(mysql_error(&gMysql));
        res=mysql_store_result(&gMysql);
	
	return((unsigned)mysql_num_rows(res));

}//unsigned ZoneExists(unsigned uZone)


