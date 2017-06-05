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


void tMonthHitNavList(void);

void ExtProcesstMonthHitVars(pentry entries[], int x)
{
	/*
	register int i;
	for(i=0;i<x;i++)
	{
	}
	*/
}//void ExtProcesstMonthHitVars(pentry entries[], int x)


void ExttMonthHitCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tMonthHitTools"))
	{
	}

}//void ExttMonthHitCommands(pentry entries[], int x)


void ExttMonthHitButtons(void)
{
	OpenFieldSet("tMonthHit Aux Panel",100);
	switch(guMode)
        {
		default:
			tMonthHitNavList();
	}
	CloseFieldSet();

}//void ExttMonthHitButtons(void)


void ExttMonthHitAuxTable(void)
{

}//void ExttMonthHitAuxTable(void)


void ExttMonthHitGetHook(entry gentries[], int x)
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
	tMonthHit("");

}//void ExttMonthHitGetHook(entry gentries[], int x)


void ExttMonthHitSelect(void)
{
	ExtSelect("tMonthHit",VAR_LIST_tMonthHit,0);

}//void ExttMonthHitSelect(void)


void ExttMonthHitSelectRow(void)
{
	ExtSelectRow("tMonthHit",VAR_LIST_tMonthHit,uMonth);

}//void ExttMonthHitSelectRow(void)


void ExttMonthHitListSelect(void)
{
	char cCat[512];

	ExtListSelect("tMonthHit",VAR_LIST_tMonthHit);

	//Changes here must be reflected below in ExttMonthHitListFilter()
        if(!strcmp(gcFilter,"uMonth"))
        {
                sscanf(gcCommand,"%u",&uMonth);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tMonthHit.uMonth=%u ORDER BY uMonth",uMonth);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uMonth");
        }

}//void ExttMonthHitListSelect(void)


void ExttMonthHitListFilter(void)
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

}//void ExttMonthHitListFilter(void)


void ExttMonthHitNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(uOwner)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttMonthHitNavBar(void)


void tMonthHitNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	ExtSelect("tMonthHit","tMonthHit.uMonth,tMonthHit.cLabel",20);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tMonthHitNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("<p><u>tMonthHitNavList</u><br>\n");

	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=?gcFunction=tMonthHit&uMonth=%s>%s</a><br>\n",
				field[0],field[1]);
	}
        mysql_free_result(res);

}//void tMonthHitNavList(void)


