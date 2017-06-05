/*
FILE
	svn ID removed
	(Built initially by unixservice.com mysqlRAD2)
PURPOSE
	Non schema-dependent table and application table related functions.
AUTHOR
	(C) 2001-2009 Gary Wallis and Hugo Urquiza.
 
*/

static unsigned uMonth=0;
static char cuMonthPullDown[33]={""};

static void htmlRecordContext(void);
static void tHitMonthNavList(void);

void ExtProcesstHitMonthVars(pentry entries[], int x)
{
	register int i;
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cuMonthPullDown"))
		{
			//escape_shell_cmd used to stop cross site SQL injection
			//of a valid user :( yikes!
			escape_shell_cmd(entries[i].val);
			strcpy(cuMonthPullDown,entries[i].val);
			uMonth=ReadPullDown("tMonthHit","cLabel",cuMonthPullDown);
		}
	}
}//void ExtProcesstHitMonthVars(pentry entries[], int x)


void ExttHitMonthCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tHitMonthTools"))
	{
		if(!strcmp(gcCommand,"Load"))
		{
			ExtProcesstHitMonthVars(entries,x);
			if(!uMonth)
				tHitMonth("<blink>Error:</blink> Must specify valid month table!");

			sprintf(gcQuery,"DELETE FROM tHitMonth");
		        mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));

			sprintf(gcQuery,"INSERT tHitMonth (uHit,cZone,uHitCount,uOwner,"
					"uCreatedBy,uCreatedDate,uModBy,uModDate) "
					"SELECT uHit,cZone,uHitCount,uOwner,uCreatedBy,"
					"uCreatedDate,uModBy,uModDate FROM %s",cuMonthPullDown);
		        mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));

		}
		else if(!strcmp(gcCommand,"Top Zone Hits Report"))
		{
			MYSQL_RES *res;
			MYSQL_ROW field;
			char *cTabs;

			ExtProcesstHitMonthVars(entries,x);
			if(!uMonth)
				tHitMonth("<blink>Error:</blink> Must select archived table!");


			printf("Content-type: text/plain\n\n");
			printf("tHitMonth.Report(%s) start.\n",cuMonthPullDown);

			printf("\nTop 200\n\n");
			sprintf(gcQuery,"SELECT cZone,uHitCount FROM %s ORDER BY uHitCount DESC LIMIT 200",cuMonthPullDown);
		        mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				printf("mysql_error: %s\n",mysql_error(&gMysql));
				exit(0);
			}
			res=mysql_store_result(&gMysql);
			while((field=mysql_fetch_row(res)))
			{
				if(strlen(field[0])>7)
					cTabs="\t\t\t\t\t";
				else
					cTabs="\t\t\t\t\t\t";
				printf("%s%s%s\n",field[0],cTabs,field[1]);
			}
			mysql_free_result(res);

			printf("\ntHitMonth.Report() end.\n");
			exit(0);
		}
	}

}//void ExttHitMonthCommands(pentry entries[], int x)


void ExttHitMonthButtons(void)
{
	OpenFieldSet("tHitMonth Aux Panel",100);

	printf("<u>Table Tips</u><br>");
	printf("This table is used to load command line archived read only tHit data one month at a time. "
		"You can then search and/or run reports on this historical data. You can also run simple "
		"reports just by selecting the Archive Tools archived table name.<p>");

	printf("<u>Archive Tools</u><br>");
	tTablePullDown("tMonthHit;cuMonthPullDown","cLabel","cLabel",uMonth,1);
	printf("<p><input class=lalertButton title='Load selected month data. Removes currently loaded data. "
		"Not needed for some report types.' type=submit name=gcCommand value='Load'><p>");
	printf("<p><input class=largeButton title='Simple example must select archive above' type=submit "
		"name=gcCommand value='Top Zone Hits Report'><p>");

	if(guPermLevel>9)
	{
		if(cZone[0])
			htmlRecordContext();
		tHitMonthNavList();
	}

	CloseFieldSet();

}//void ExttHitMonthButtons(void)


void ExttHitMonthAuxTable(void)
{

}//void ExttHitMonthAuxTable(void)


void ExttHitMonthGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uHit"))
		{
			sscanf(gentries[i].val,"%u",&uHit);
			guMode=6;
		}
	}
	tHitMonth("");

}//void ExttHitMonthGetHook(entry gentries[], int x)


void ExttHitMonthSelect(void)
{
	ExtSelect("tHitMonth",VAR_LIST_tHitMonth,0);

}//void ExttHitMonthSelect(void)


void ExttHitMonthSelectRow(void)
{
	ExtSelectRow("tHitMonth",VAR_LIST_tHitMonth,uHit);

}//void ExttHitMonthSelectRow(void)


void ExttHitMonthListSelect(void)
{
	char cCat[512];

	ExtListSelect("tHitMonth",VAR_LIST_tHitMonth);

	//Changes here must be reflected below in ExttHitMonthListFilter()
        if(!strcmp(gcFilter,"uHit"))
        {
                sscanf(gcCommand,"%u",&uHit);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tHitMonth.uHit=%u ORDER BY uHit",uHit);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uHit");
        }

}//void ExttHitMonthListSelect(void)


void ExttHitMonthListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uHit"))
                printf("<option>uHit</option>");
        else
                printf("<option selected>uHit</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttHitMonthListFilter(void)


void ExttHitMonthNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(uOwner)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttHitMonthNavBar(void)


void tHitMonthNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	sprintf(gcQuery,"SELECT uHit,cZone,uHitCount FROM tHitMonth ORDER BY uHitCount DESC LIMIT 20");

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tHitMonthNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("<p><u>tHitMonthNavList Top 20 by uHitCount</u><br>\n");

	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=?gcFunction=tHitMonth&uHit=%s>%s/%s</a><br>\n",
				field[0],field[1],field[2]);
	}
        mysql_free_result(res);

}//void tHitMonthNavList(void)



void htmlRecordContext(void)
{
        MYSQL_RES *res;

	printf("<p><u>Record Context Info</u><br>");

	sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",cZone);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>htmlRecordContext</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }
        res=mysql_store_result(&gMysql);
	printf("This cZone is ");
	if(mysql_num_rows(res)==0)
		printf("not ");
	printf("a current tZone.cZone<br>\n");
        mysql_free_result(res);

}//void htmlRecordContext(void)

