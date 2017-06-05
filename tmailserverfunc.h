/*
FILE
	svn ID removed
PURPOSE
	Non-schema dependent tmailserver.c expansion.
AUTHOR
	GPL License applies, see www.fsf.org for details
	See LICENSE file in this distribution
	(C) 2001-2009 Gary Wallis and Hugo Urquiza.
*/

void tMailServerNavList(void);

void ExtProcesstMailServerVars(pentry entries[], int x)
{

	/*
	register int i;
	
	for(i=0;i<x;i++)
	{
	
	}
	*/

}//void ExtProcesstMailServerVars(pentry entries[], int x)


void ExttMailServerCommands(pentry entries[], int x)
{
	if(!strcmp(gcFunction,"tMailServerTools"))
	{
		//ModuleFunctionProcess()

		//Default wizard like two step creation and deletion
		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=10)
			{
				ProcesstMailServerVars(entries,x);
				//Check global conditions for new record here
				guMode=2000;
				tMailServer(LANG_NB_CONFIRMNEW);
			}
			else
				tMailServer("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=10)
			{
				ProcesstMailServerVars(entries,x);
				//Check entries here
				uMailServer=0;
				uCreatedBy=guLoginClient;
				uOwner=guCompany;
				uModBy=0;//Never modified
				NewtMailServer(0);
			}
			else
				tMailServer("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstMailServerVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=2001;
				tMailServer(LANG_NB_CONFIRMDEL);
			}
			else
				tMailServer("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstMailServerVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetMailServer();
			}
			else
				tMailServer("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstMailServerVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tMailServer(LANG_NB_CONFIRMMOD);
			}
			else
				tMailServer("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstMailServerVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				uModBy=guLoginClient;
				ModtMailServer();
			}
			else
				tMailServer("<blink>Error</blink>: Denied by permissions settings");
                }
	}

}//void ExttMailServerCommands(pentry entries[], int x)


void ExttMailServerButtons(void)
{
	OpenFieldSet("Aux Panel",100);

	switch(guMode)
        {
                case 2000:
			printf("Enter required data<br>");
                        printf(LANG_NBB_CONFIRMNEW);
			printf("<br>\n");
                break;

                case 2001:
                        printf(LANG_NBB_CONFIRMDEL);
			printf("<br>\n");
                break;

                case 2002:
			printf("Review record data<br>");
                        printf(LANG_NBB_CONFIRMMOD);
			printf("<br>\n");
                break;

		default:
			tMailServerNavList();

	}

	CloseFieldSet();
	
}//void ExttMailServerButtons(void)


void ExttMailServerAuxTable(void)
{

}//void ExttMailServerAuxTable(void)


void ExttMailServerGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uMailServer"))
		{
			sscanf(gentries[i].val,"%u",&uMailServer);
			guMode=6;
		}
	}
	tMailServer("");

}//void ExttMailServerGetHook(entry gentries[], int x)


void ExttMailServerSelect(void)
{
	ExtSelect("tMailServer",VAR_LIST_tMailServer,0);

}//void ExttMailServerSelect(void)


void ExttMailServerSelectRow(void)
{
	ExtSelectRow("tMailServer",VAR_LIST_tMailServer,uMailServer);

}//void ExttMailServerSelectRow(void)


void ExttMailServerListSelect(void)
{
	char cCat[512];

	ExtListSelect("tMailServer",VAR_LIST_tMailServer);

	//Changes here must be reflected below in ExttMailServerListFilter()
        if(!strcmp(gcFilter,"uMailServer"))
        {
                sscanf(gcCommand,"%u",&uMailServer);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tMailServer.uMailServer=%u \
						ORDER BY uMailServer",
						uMailServer);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uMailServer");
        }

}//void ExttMailServerListSelect(void)


void ExttMailServerListFilter(void)
{
        //Filter
        printf("<td align=right >Select ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uMailServer"))
                printf("<option>uMailServer</option>");
        else
                printf("<option selected>uMailServer</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttMailServerListFilter(void)


void ExttMailServerNavBar(void)
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

}//void ExttMailServerNavBar(void)


void tMailServerNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	register unsigned uCount=0;

	ExtSelect("tMailServer","tMailServer.uMailServer,tMailServer.cLabel",20);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
	        printf("<p><u>tMailServerNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{
	        printf("<p><u>tMailServerNavList</u><br>\n");
	        while((field=mysql_fetch_row(res)))
	        {
	                printf("<a class=darkLink href=?gcFunction=tMailServer&uMailServer=%s>%s</a><br>\n",field[0],field[1]);
			uCount++;
	        }
		if(uCount>100)
			printf("More than 100 found. Showing only 101<br>\n");
	}
        mysql_free_result(res);

}//void tMailServerNavList(void)


// vim:tw=78
//perlSAR patch1
