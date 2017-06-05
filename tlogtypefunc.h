/*
FILE
	svn ID removed
	(Built initially by unixservice.com mysqlRAD2)
PURPOSE
	Non schema-dependent table and application table related functions.
AUTHOR/LEGAL
	(C) 2001-2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file.
*/

//ModuleFunctionProtos()


void tLogTypeNavList(void);

void ExtProcesstLogTypeVars(pentry entries[], int x)
{
	/*
	register int i;
	for(i=0;i<x;i++)
	{
	}
	*/
}//void ExtProcesstLogTypeVars(pentry entries[], int x)


void ExttLogTypeCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tLogTypeTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=12)
			{
	                        ProcesstLogTypeVars(entries,x);
                        	guMode=2000;
	                        tLogType(LANG_NB_CONFIRMNEW);
			}
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=12)
			{
                        	ProcesstLogTypeVars(entries,x);

                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uLogType=0;
				uCreatedBy=guLoginClient;
				uOwner=guCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtLogType(0);
			}
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstLogTypeVars(entries,x);
			if(guPermLevel>=12 && guLoginClient==1)
			{
	                        guMode=2001;
				tLogType(LANG_NB_CONFIRMDEL);
			}
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstLogTypeVars(entries,x);
			if(guPermLevel>=12 && guLoginClient==1)
			{
				guMode=5;
				DeletetLogType();
			}
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstLogTypeVars(entries,x);
			if(guPermLevel>=12)
			{
				guMode=2002;
				tLogType(LANG_NB_CONFIRMMOD);
			}
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstLogTypeVars(entries,x);
			if(guPermLevel>=12)
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtLogType();
			}
                }
	}

}//void ExttLogTypeCommands(pentry entries[], int x)


void ExttLogTypeButtons(void)
{
	OpenFieldSet("tLogType Aux Panel",100);
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

			tLogTypeNavList();
	}
	CloseFieldSet();

}//void ExttLogTypeButtons(void)


void ExttLogTypeAuxTable(void)
{

}//void ExttLogTypeAuxTable(void)


void ExttLogTypeGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uLogType"))
		{
			sscanf(gentries[i].val,"%u",&uLogType);
			guMode=6;
		}
	}
	tLogType("");

}//void ExttLogTypeGetHook(entry gentries[], int x)


void ExttLogTypeSelect(void)
{
	ExtSelectPublic("tLogType",VAR_LIST_tLogType);

}//void ExttLogTypeSelect(void)


void ExttLogTypeSelectRow(void)
{
	ExtSelectRowPublic("tLogType",VAR_LIST_tLogType,uLogType);

}//void ExttLogTypeSelectRow(void)


void ExttLogTypeListSelect(void)
{
	char cCat[512];

	ExtListSelectPublic("tLogType",VAR_LIST_tLogType);
	
	//Changes here must be reflected below in ExttLogTypeListFilter()
        if(!strcmp(gcFilter,"uLogType"))
        {
                sscanf(gcCommand,"%u",&uLogType);
		sprintf(cCat," WHERE tLogType.uLogType=%u ORDER BY uLogType",
						uLogType);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uLogType");
        }

}//void ExttLogTypeListSelect(void)


void ExttLogTypeListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uLogType"))
                printf("<option>uLogType</option>");
        else
                printf("<option selected>uLogType</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttLogTypeListFilter(void)


void ExttLogTypeNavBar(void)
{
	if(uOwner) GetClientOwner(uOwner,&guReseller);

	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(guPermLevel>=12 && !guListMode)
		printf(LANG_NBB_NEW);

	if(guPermLevel>=12)
		printf(LANG_NBB_MODIFY);

	if(guPermLevel>=12 && guLoginClient==1)
		printf(LANG_NBB_DELETE);

	if(uOwner)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttLogTypeNavBar(void)


void tLogTypeNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	ExtSelectPublic("tLogType","tLogType.uLogType,tLogType.cLabel");
	
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tLogTypeNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("<p><u>tLogTypeNavList</u><br>\n");

	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=unxsVZ.cgi?gcFunction=tLogType&uLogType=%s>%s</a><br>\n",
				field[0],field[1]);
	}
        mysql_free_result(res);

}//void tLogTypeNavList(void)

