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


void tHitNavList(void);
static void htmlRecordContext(void);
static char cSearch[100]={""};

void ExtProcesstHitVars(pentry entries[], int x)
{
	register int i;
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cSearch"))
			sprintf(cSearch,"%.99s",entries[i].val);
	}
}//void ExtProcesstHitVars(pentry entries[], int x)


void ExttHitCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tHitTools"))
	{
	}

}//void ExttHitCommands(pentry entries[], int x)


void ExttHitButtons(void)
{
	OpenFieldSet("tHit Aux Panel",100);
	printf("<u>Table Tips</u><br>");
	printf("This table holds the non-archived (usually the current month only) per tZone.cZone Dns/BIND named daemon "
		"query hits (cluster wide.) Not real time data: This data is collected from all cluster NSs and then "
		"aggregated here usually only once a day. When possible some context related info is provided below.<p>"
		"<a href=?gcFunction=tHitMonth>tHitMonth</a> allows access to all archived (read-only and compressed)"
		"monthly tHit data sets. These archives are created from the command line usually by crontab operation.");
	printf("<p><u>Search Tools</u><br>");
	printf("Enter the complete or the first part of a <i>cZone</i>. You can use %% and _ SQL LIKE matching chars.<br>");
	printf("<input type=text title='cZone search. Use %% . and _ for pattern matching.' name=cSearch value=\"%s\""
		" maxlength=99 size=20><br>",cSearch);

	if(cZone[0])
		htmlRecordContext();
	tHitNavList();

	CloseFieldSet();

}//void ExttHitButtons(void)


void ExttHitAuxTable(void)
{

}//void ExttHitAuxTable(void)


void ExttHitGetHook(entry gentries[], int x)
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
	tHit("");

}//void ExttHitGetHook(entry gentries[], int x)


void ExttHitSelect(void)
{
	if(cSearch[0])
		ExtSelectSearch("tHit",VAR_LIST_tHit,"cZone",cSearch,NULL,20);
	else
		ExtSelect("tHit",VAR_LIST_tHit,0);

}//void ExttHitSelect(void)


void ExttHitSelectRow(void)
{
	ExtSelectRow("tHit",VAR_LIST_tHit,uHit);

}//void ExttHitSelectRow(void)


void ExttHitListSelect(void)
{
	char cCat[512];

	ExtListSelect("tHit",VAR_LIST_tHit);

	//Changes here must be reflected below in ExttHitListFilter()
        if(!strcmp(gcFilter,"uHit"))
        {
                sscanf(gcCommand,"%u",&uHit);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tHit.uHit=%u ORDER BY uHit",uHit);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uHit");
        }

}//void ExttHitListSelect(void)


void ExttHitListFilter(void)
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

}//void ExttHitListFilter(void)


void ExttHitNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(uOwner)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttHitNavBar(void)


void tHitNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	sprintf(gcQuery,"SELECT uHit,cZone,uHitCount FROM tHit ORDER BY uHitCount DESC LIMIT 20");

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tHitNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("<p><u>tHitNavList Top 20 by uHitCount</u><br>\n");

	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=?gcFunction=tHit&uHit=%s>%s/%s</a><br>\n",
				field[0],field[1],field[2]);
	}
        mysql_free_result(res);

}//void tHitNavList(void)


void htmlRecordContext(void)
{
        MYSQL_RES *res;

	printf("<p><u>Record Context Info</u><br>");

	sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",cZone);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tHitNavList</u><br>\n");
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
