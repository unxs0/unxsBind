/*
FILE
	svn ID removed
	(Built initially by unixservice.com mysqlRAD2)
PURPOSE
	Non schema-dependent table and application table related functions.
AUTHOR/LEGAL
	(C) 2001-2010 Gary Wallis for Unixservice, LLC.
	GPL License applies, see www.fsf.org for details.
	See LICENSE file in this distribution.
*/

//ModuleFunctionProtos()
static char cSearch[64]={""};

void tViewNavList(void);
void tViewContextInfo(void);

void ExtProcesstViewVars(pentry entries[], int x)
{
	
	register int i;
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cSearch"))
			sprintf(cSearch,"%.63s",entries[i].val);
	}
	
}//void ExtProcesstViewVars(pentry entries[], int x)


void ExttViewCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tViewTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=10)
			{
	                        ProcesstViewVars(entries,x);
                        	guMode=2000;
	                        tView(LANG_NB_CONFIRMNEW);
			}
			else
				tView("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=10)
			{
                        	ProcesstViewVars(entries,x);

                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uView=0;
				uCreatedBy=guLoginClient;
				uOwner=guCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtView(0);
			}
			else
				tView("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstViewVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tView(LANG_NB_CONFIRMDEL);
			}
			else
				tView("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstViewVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetView();
			}
			else
				tView("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstViewVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tView(LANG_NB_CONFIRMMOD);
			}
			else
				tView("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstViewVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				unsigned uJobNSSet=1;

                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				if(!uNSSet)
				{
					char cuNSSet[256]={"1"};

					GetConfiguration("cuDefaultViewNS",cuNSSet,0);
					sscanf(cuNSSet,"%u",&uJobNSSet);
				}
				else
				{
					uJobNSSet=uNSSet;
				}

				if(SubmitJob("Delete",uJobNSSet,"ViewReload",0,0))
					htmlPlainTextError("tView.SubmitJob() CONFIRMMOD");;
				uModBy=guLoginClient;
				ModtView();
			}
			else
				tView("<blink>Error</blink>: Denied by permissions settings");
                }
	}

}//void ExttViewCommands(pentry entries[], int x)


void ExttViewButtons(void)
{
	OpenFieldSet("tView Aux Panel",100);
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

		default:
			printf("<u>Table Tips</u><br>");
			printf("BIND 9 introduced views. mysqlBind could not cope, thus iDNS (mysqlBind2.)"
				" Apart from the mundane internal/external view model for firewalled environments."
				" iDNS supports n views, for advanced geographically (approximated by IP ranges)"
				" differentiated DNS query result sets. Start out with internal/external views and"
				" then if you are a very large world-wide concern you can setup whatever you need,"
				" and the iDNS automation will reduce critical errors and keep you sane and away from"
				" your shell editor.\n");
			printf("<p><u>Search Tools</u><br>");
			printf("<input type=text title='cLabel search. Use %% . and _ for pattern matching when applicable.'"
				" name=cSearch value=\"%s\" maxlength=99 size=20> cSearch",cSearch);
			tViewNavList();
			printf("<br>\n");
			tViewContextInfo();
	}
	CloseFieldSet();

}//void ExttViewButtons(void)


void ExttViewAuxTable(void)
{

}//void ExttViewAuxTable(void)


void ExttViewGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uView"))
		{
			sscanf(gentries[i].val,"%u",&uView);
			guMode=6;
		}
		else if(!strcmp(gentries[i].name,"cSearch"))
			sprintf(cSearch,"%.63s",gentries[i].val);
	}
	tView("");

}//void ExttViewGetHook(entry gentries[], int x)


void ExttViewSelect(void)
{
	if(cSearch[0])
		ExtSelectSearch("tView",VAR_LIST_tView,"tView.cLabel",cSearch,NULL,20);
	else
		ExtSelect("tView",VAR_LIST_tView,0);

}//void ExttViewSelect(void)


void ExttViewSelectRow(void)
{
	ExtSelectRow("tView",VAR_LIST_tView,uView);

}//void ExttViewSelectRow(void)


void ExttViewListSelect(void)
{
	char cCat[512];

	ExtListSelect("tView",VAR_LIST_tView);

	//Changes here must be reflected below in ExttViewListFilter()
        if(!strcmp(gcFilter,"uView"))
        {
                sscanf(gcCommand,"%u",&uView);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tView.uView=%u ORDER BY uView",uView);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uView");
        }

}//void ExttViewListSelect(void)


void ExttViewListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uView"))
                printf("<option>uView</option>");
        else
                printf("<option selected>uView</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttViewListFilter(void)


void ExttViewNavBar(void)
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

}//void ExttViewNavBar(void)


void tViewNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	unsigned uCount=0;

	if(!cSearch[0])
	{
        	printf("<p><u>tViewNavList</u><br>\n");
        	printf("Must restrict via cSearch<br>\n");
		return;
	}


	ExtSelectSearch("tView","tView.uView,tView.cLabel","tView.cLabel",cSearch,NULL,0);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tViewNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	printf("<p><u>tViewNavList</u><br>\n");
	if(mysql_num_rows(res))
	{	
	        while((field=mysql_fetch_row(res)))
		{
			uCount++;
			printf("<a class=darkLink href=?gcFunction=tView&uView=%s&cSearch=%s>%s</a><br>\n",
				field[0],cURLEncode(cSearch),field[1]);
			if(uCount>=100)
			{
				printf("More than 100 records: You must refine your search further<br>\n");
				break;
			}
	        }
	}
	else
		printf("No records found<br>");
		
        mysql_free_result(res);

}//void tViewNavList(void)


void tViewContextInfo(void)
{
	printf("<u>Record Context Info</u><br>");
	printf("No context info available<br>");

}//void tViewContextInfo(void)

