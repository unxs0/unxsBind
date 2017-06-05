/*
FILE
	svn ID removed
	(Built initially by unixservice.com mysqlRAD2)
PURPOSE
	Non schema-dependent table and application table related functions.
AUTHOR
	(C) 2001-2009 Gary Wallis and Hugo Urquiza.
 
*/

//ModuleFunctionProtos()


void tRegistrarNavList(void);

void ExtProcesstRegistrarVars(pentry entries[], int x)
{
	/*
	register int i;
	for(i=0;i<x;i++)
	{
	}
	*/
}//void ExtProcesstRegistrarVars(pentry entries[], int x)


void ExttRegistrarCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tRegistrarTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=10)
			{
	                        ProcesstRegistrarVars(entries,x);
                        	guMode=2000;
	                        tRegistrar(LANG_NB_CONFIRMNEW);
			}
			else
				tRegistrar("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=10)
			{
                        	ProcesstRegistrarVars(entries,x);

                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uRegistrar=0;
				uCreatedBy=guLoginClient;
				uOwner=guCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtRegistrar(0);
			}
			else
				tRegistrar("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstRegistrarVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tRegistrar(LANG_NB_CONFIRMDEL);
			}
			else
				tRegistrar("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstRegistrarVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetRegistrar();
			}
			else
				tRegistrar("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstRegistrarVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tRegistrar(LANG_NB_CONFIRMMOD);
			}
			else
				tRegistrar("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstRegistrarVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtRegistrar();
			}
			else
				tRegistrar("<blink>Error</blink>: Denied by permissions settings");
                }
	}

}//void ExttRegistrarCommands(pentry entries[], int x)


void ExttRegistrarButtons(void)
{
	OpenFieldSet("tRegistrar Aux Panel",100);
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

			tRegistrarNavList();
	}
	CloseFieldSet();

}//void ExttRegistrarButtons(void)


void ExttRegistrarAuxTable(void)
{

}//void ExttRegistrarAuxTable(void)


void ExttRegistrarGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uRegistrar"))
		{
			sscanf(gentries[i].val,"%u",&uRegistrar);
			guMode=6;
		}
	}
	tRegistrar("");

}//void ExttRegistrarGetHook(entry gentries[], int x)


void ExttRegistrarSelect(void)
{
	ExtSelect("tRegistrar",VAR_LIST_tRegistrar,0);

}//void ExttRegistrarSelect(void)


void ExttRegistrarSelectRow(void)
{
	ExtSelectRow("tRegistrar",VAR_LIST_tRegistrar,uRegistrar);

}//void ExttRegistrarSelectRow(void)


void ExttRegistrarListSelect(void)
{
	char cCat[512];

	ExtListSelect("tRegistrar",VAR_LIST_tRegistrar);

	//Changes here must be reflected below in ExttRegistrarListFilter()
        if(!strcmp(gcFilter,"uRegistrar"))
        {
                sscanf(gcCommand,"%u",&uRegistrar);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tRegistrar.uRegistrar=%u \
						ORDER BY uRegistrar",
						uRegistrar);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uRegistrar");
        }

}//void ExttRegistrarListSelect(void)


void ExttRegistrarListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uRegistrar"))
                printf("<option>uRegistrar</option>");
        else
                printf("<option selected>uRegistrar</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttRegistrarListFilter(void)


void ExttRegistrarNavBar(void)
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

}//void ExttRegistrarNavBar(void)


void tRegistrarNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	ExtSelect("tRegistrar","tRegistrar.uRegistrar,tRegistrar.cLabel",20);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tRegistrarNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("<p><u>tRegistrarNavList</u><br>\n");

	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=?gcFunction=tRegistrar&uRegistrar=%s>%s</a><br>\n",field[0],field[1]);
	}
        mysql_free_result(res);

}//void tRegistrarNavList(void)


