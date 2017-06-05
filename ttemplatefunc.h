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

static unsigned uTemplateSetFilter=1;
static char cuTemplateSetPullDownFilter[256]={""};
static unsigned uTemplateTypeFilter=1;
static char cuTemplateTypePullDownFilter[256]={""};

void tTemplateNavList(void);

void ExtProcesstTemplateVars(pentry entries[], int x)
{
	register int i;
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cuTemplateSetPullDownFilter"))
		{
			sprintf(cuTemplateSetPullDownFilter,"%.255s",entries[i].val);
			uTemplateSetFilter=ReadPullDown("tTemplateSet","cLabel",cuTemplateSetPullDownFilter);
		}
		else if(!strcmp(entries[i].name,"cuTemplateTypePullDownFilter"))
		{
			sprintf(cuTemplateTypePullDownFilter,"%.255s",entries[i].val);
			uTemplateTypeFilter=ReadPullDown("tTemplateType","cLabel",cuTemplateTypePullDownFilter);
		}
	}
}//void ExtProcesstTemplateVars(pentry entries[], int x)


void ExttTemplateCommands(pentry entries[], int x)
{
	if(!strcmp(gcFunction,"tTemplateTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=10)
			{
	                        ProcesstTemplateVars(entries,x);
                        	guMode=2000;
	                        tTemplate(LANG_NB_CONFIRMNEW);
			}
			else
				tTemplate("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=10)
			{
                        	ProcesstTemplateVars(entries,x);

                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uTemplate=0;
				uCreatedBy=guLoginClient;
				uOwner=guCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtTemplate(0);
			}
			else
				tTemplate("<blink>Error</blink>: Denied by permissions settings");

		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstTemplateVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tTemplate(LANG_NB_CONFIRMDEL);
			}
			else
				tTemplate("<blink>Error</blink>: Denied by permissions settings");

                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstTemplateVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetTemplate();
			}
			else
				tTemplate("<blink>Error</blink>: Denied by permissions settings");

                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstTemplateVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tTemplate(LANG_NB_CONFIRMMOD);
			}
			else
				tTemplate("<blink>Error</blink>: Denied by permissions settings");

                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstTemplateVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtTemplate();
			}
			else
				tTemplate("<blink>Error</blink>: Denied by permissions settings");

                }
	}

}//void ExttTemplateCommands(pentry entries[], int x)


void ExttTemplateButtons(void)
{
	OpenFieldSet("Aux Panel",100);
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
			tTemplateNavList();
	}
	CloseFieldSet();

}//void ExttTemplateButtons(void)


void ExttTemplateAuxTable(void)
{

}//void ExttTemplateAuxTable(void)


void ExttTemplateGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uTemplate"))
		{
			sscanf(gentries[i].val,"%u",&uTemplate);
			guMode=6;
		}
		else if(!strcmp(gentries[i].name,"uTemplateSetFilter"))
		{
			sscanf(gentries[i].val,"%u",&uTemplateSetFilter);
		}
		else if(!strcmp(gentries[i].name,"uTemplateTypeFilter"))
		{
			sscanf(gentries[i].val,"%u",&uTemplateTypeFilter);
		}
	}
	tTemplate("");

}//void ExttTemplateGetHook(entry gentries[], int x)


void ExttTemplateSelect(void)
{
	ExtSelect("tTemplate",VAR_LIST_tTemplate,0);

}//void ExttTemplateSelect(void)


void ExttTemplateSelectRow(void)
{
	ExtSelectRow("tTemplate",VAR_LIST_tTemplate,uTemplate);

}//void ExttTemplateSelectRow(void)


void ExttTemplateListSelect(void)
{
	char cCat[512];

	ExtListSelect("tTemplate",VAR_LIST_tTemplate);

	//Changes here must be reflected below in ExttTemplateListFilter()
        if(!strcmp(gcFilter,"uTemplate"))
        {
                sscanf(gcCommand,"%u",&uTemplate);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tTemplate.uTemplate=%u ORDER BY uTemplate",uTemplate);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uTemplate");
        }

}//void ExttTemplateListSelect(void)


void ExttTemplateListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uTemplate"))
                printf("<option>uTemplate</option>");
        else
                printf("<option selected>uTemplate</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttTemplateListFilter(void)


void ExttTemplateNavBar(void)
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

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttTemplateNavBar(void)


void tTemplateNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	unsigned uCount=0;

	sprintf(gcQuery,"SELECT uTemplate,cLabel FROM tTemplate WHERE uTemplateType=%u AND uTemplateSet=%u"
				" ORDER BY uTemplateType,uTemplateSet,cLabel",
						uTemplateTypeFilter,uTemplateSetFilter);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tTemplateNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if((uCount=mysql_num_rows(res)))
	{	
        	printf("<p><u>tTemplateNavList</u><br>\n");
        	printf("<u>Filters</u><br>\n");
		tTablePullDown("tTemplateSet;cuTemplateSetPullDownFilter","cLabel","cLabel",uTemplateSetFilter,1);
		tTablePullDown("tTemplateType;cuTemplateTypePullDownFilter","cLabel","cLabel",uTemplateTypeFilter,1);
        	printf("<p>\n");

	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=?gcFunction=tTemplate&uTemplate=%s&"
				"uTemplateTypeFilter=%u&uTemplateSetFilter=%u>%s</a><br>\n",
				field[0],uTemplateTypeFilter,uTemplateSetFilter,field[1]);
	}
	else
	{
        	printf("<p><u>tTemplateNavList</u><br>\n");
        	printf("<u>Filters</u><br>\n");
		tTablePullDown("tTemplateSet;cuTemplateSetPullDownFilter","cLabel","cLabel",uTemplateSetFilter,1);
		tTablePullDown("tTemplateType;cuTemplateTypePullDownFilter","cLabel","cLabel",uTemplateTypeFilter,1);
        	printf("<p>\n");
	}
        mysql_free_result(res);
	printf("(uCount=%u)",uCount);

}//void tTemplateNavList(void)

