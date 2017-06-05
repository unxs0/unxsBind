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


void tMonthNavList(void);

void ExtProcesstMonthVars(pentry entries[], int x)
{
	/*
	register int i;
	for(i=0;i<x;i++)
	{
	}
	*/
}//void ExtProcesstMonthVars(pentry entries[], int x)


void ExttMonthCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tMonthTools"))
	{
	}

}//void ExttMonthCommands(pentry entries[], int x)


void ExttMonthButtons(void)
{
	OpenFieldSet("tMonth Aux Panel",100);
	switch(guMode)
        {
		default:

			tMonthNavList();
	}
	CloseFieldSet();

}//void ExttMonthButtons(void)


void ExttMonthAuxTable(void)
{

}//void ExttMonthAuxTable(void)


void ExttMonthGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uMonth"))
		{
			sscanf(gentries[i].val,"%u",&uMonth);
			guMode=6;
		}
	}
	tMonth("");

}//void ExttMonthGetHook(entry gentries[], int x)


void ExttMonthSelect(void)
{
	ExtSelect("tMonth",VAR_LIST_tMonth,0);

}//void ExttMonthSelect(void)


void ExttMonthSelectRow(void)
{
	ExtSelectRow("tMonth",VAR_LIST_tMonth,uMonth);

}//void ExttMonthSelectRow(void)


void ExttMonthListSelect(void)
{
	char cCat[512];

	ExtListSelect("tMonth",VAR_LIST_tMonth);

	//Changes here must be reflected below in ExttMonthListFilter()
        if(!strcmp(gcFilter,"uMonth"))
        {
                sscanf(gcCommand,"%u",&uMonth);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tMonth.uMonth=%u ORDER BY uMonth",uMonth);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uMonth");
        }

}//void ExttMonthListSelect(void)


void ExttMonthListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uMonth"))
                printf("<option>uMonth</option>");
        else
                printf("<option selected>uMonth</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttMonthListFilter(void)


void ExttMonthNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(uOwner)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttMonthNavBar(void)


void tMonthNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	
	ExtSelect("tMonth","tMonth.uMonth,tMonth.cLabel",20);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tMonthNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("<p><u>tMonthNavList</u><br>\n");

	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=?gcFunction=tMonth&uMonth=%s>%s</a><br>\n",
				field[0],field[1]);
	}
        mysql_free_result(res);

}//void tMonthNavList(void)


