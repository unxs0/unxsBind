/*
FILE
	svn ID removed
	(Built initially by unixservice.com mysqlRAD2)
PURPOSE
	Non schema-dependent table and application table related functions.
AUTHOR
	(C) 2001-2008 Gary Wallis and Hugo Urquiza.
 
*/

//ModuleFunctionProtos()


void tServerNavList(void);
void tServerContextInfo(void);

void ExtProcesstServerVars(pentry entries[], int x)
{
	/*
	register int i;
	for(i=0;i<x;i++)
	{
	}
	*/
}//void ExtProcesstServerVars(pentry entries[], int x)


void ExttServerCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tServerTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstServerVars(entries,x);
                        	guMode=2000;
	                        tServer(LANG_NB_CONFIRMNEW);
			}
			else
				tServer("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=9)
			{
				unsigned uContactParentCompany=0;
                        	ProcesstServerVars(entries,x);
				GetClientOwner(guLoginClient,&uContactParentCompany);
				
                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uServer=0;
				uCreatedBy=guLoginClient;
				uOwner=uContactParentCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtServer(0);
			}
			else
				tServer("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstServerVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tServer(LANG_NB_CONFIRMDEL);
			}
			else
				tServer("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstServerVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetServer();
			}
			else
				tServer("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstServerVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tServer(LANG_NB_CONFIRMMOD);
			}
			else
				tServer("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstServerVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtServer();
			}
			else
				tServer("<blink>Error</blink>: Denied by permissions settings");
                }
	}

}//void ExttServerCommands(pentry entries[], int x)


void ExttServerButtons(void)
{
	OpenFieldSet("tServer Aux Panel",100);
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
			printf("<p><u>Record Context Info</u><br>");
			tServerContextInfo();
			tServerNavList();
	}
	CloseFieldSet();

}//void ExttServerButtons(void)


void ExttServerAuxTable(void)
{

}//void ExttServerAuxTable(void)


void ExttServerGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uServer"))
		{
			sscanf(gentries[i].val,"%u",&uServer);
			guMode=6;
		}
	}
	tServer("");

}//void ExttServerGetHook(entry gentries[], int x)


void ExttServerSelect(void)
{
	ExtSelect("tServer",VAR_LIST_tServer,0);

}//void ExttServerSelect(void)


void ExttServerSelectRow(void)
{
	ExtSelectRow("tServer",VAR_LIST_tServer,uServer);
}//void ExttServerSelectRow(void)


void ExttServerListSelect(void)
{
	char cCat[512];
	ExtListSelect("tServer",VAR_LIST_tServer);

	//Changes here must be reflected below in ExttServerListFilter()
        if(!strcmp(gcFilter,"uServer"))
        {
                sscanf(gcCommand,"%u",&uServer);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tServer.uServer=%u"
						" ORDER BY uServer",
						uServer);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uServer");
        }

}//void ExttServerListSelect(void)


void ExttServerListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uServer"))
                printf("<option>uServer</option>");
        else
                printf("<option selected>uServer</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttServerListFilter(void)


void ExttServerNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(guPermLevel>=7 && !guListMode)
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

}//void ExttServerNavBar(void)


void tServerNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	
	ExtSelect("tServer","tServer.uServer,tServer.cLabel",0);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tServerNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("<p><u>tServerNavList</u><br>\n");

	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=?gcFunction=tServer"
				"&uServer=%s>%s</a><br>\n",
				field[0],field[1]);
	}
        mysql_free_result(res);

}//void tServerNavList(void)


void tServerContextInfo(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	if(!uServer) return;

	sprintf(gcQuery,"SELECT COUNT(tZone.uZone) FROM tNSSet,tNS,tZone WHERE"
			" tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND"
			" tNS.uServer=%u",uServer);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("tServerContextInfo:<br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
        	printf("tServerContextInfo:<br>\n");
		printf("%s zones are assigned to NS sets that have NS associated with this server.",field[0]);
	}
        mysql_free_result(res);

}//void tServerContextInfo(void)


