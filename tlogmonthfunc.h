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

void LogMonthSummary(void);

void ExtProcesstLogMonthVars(pentry entries[], int x)
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
			uMonth=ReadPullDown("tMonth","cLabel",cuMonthPullDown);
		}
	}
}//void ExtProcesstLogMonthVars(pentry entries[], int x)


void ExttLogMonthCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tLogMonthTools"))
	{
		if(!strcmp(gcCommand,"Load"))
		{
			ExtProcesstLogMonthVars(entries,x);
			if(!uMonth)
				tLogMonth("<blink>Error:</blink> Must specify valid month table");

			sprintf(gcQuery,"DELETE FROM tLogMonth");
		        mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));

			sprintf(gcQuery,"INSERT DELAYED tLogMonth (uLog,cLabel,uLogType,cHash,uPermLevel,"
					"uLoginClient,cLogin,cHost,uTablePK,cTableName,uOwner,uCreatedBy,"
					"uCreatedDate,uModBy,uModDate) SELECT uLog,cLabel,uLogType,cHash,"
					"uPermLevel,uLoginClient,cLogin,cHost,uTablePK,cTableName,uOwner,"
					"uCreatedBy,uCreatedDate,uModBy,uModDate FROM %s",cuMonthPullDown);
		        mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));

		}
		else if(!strcmp(gcCommand,"Top OP Stats Report"))
		{
			MYSQL_RES *res;
			MYSQL_ROW field;
			char *cTabs;

			ExtProcesstLogMonthVars(entries,x);
			if(!uMonth)
				tLogMonth("<blink>Error:</blink> Must select archived table!");


			printf("Content-type: text/plain\n\n");
			printf("tLogMonth.Report(%s) start.\n",cuMonthPullDown);

			printf("\nNew OPs: Top 20 by login\n\n");
			sprintf(gcQuery,"SELECT cLogin,COUNT(uLog) AS CountLog FROM %s WHERE cLabel='New' "
					"GROUP BY cLogin ORDER BY CountLog DESC LIMIT 20",cuMonthPullDown);
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

			printf("\nMod OPs: Top 20 by login\n\n");
			sprintf(gcQuery,"SELECT cLogin,COUNT(uLog) AS CountLog FROM %s WHERE cLabel='Mod'"
					" GROUP BY cLogin ORDER BY CountLog DESC LIMIT 20",cuMonthPullDown);
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

			printf("\nDel OPs: Top 20 by login\n\n");
			sprintf(gcQuery,"SELECT cLogin,COUNT(uLog) AS CountLog FROM %s WHERE cLabel='Del'"
					" GROUP BY cLogin ORDER BY CountLog DESC LIMIT 20",cuMonthPullDown);
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


			printf("\ntLogMonth.Report() end.\n");
			exit(0);
		}
	}

}//void ExttLogMonthCommands(pentry entries[], int x)


void ExttLogMonthButtons(void)
{
	OpenFieldSet("tLogMonth Aux Panel",100);

	printf("<u>Table Tips</u><br>");
	printf("This table is used to load command line archived read only tLog data one month at a time."
		" You can then search and/or run reports on this historical data. Some simple reports do "
		"not require loading, just selecting the archive table below.<p>");

	printf("<u>Archive Tools</u><br>");
	tTablePullDown("tMonth;cuMonthPullDown","cLabel","cLabel",uMonth,1);
	printf("<p><input class=lalertButton title='Load selected month data' type=submit name=gcCommand value='Load'><p>");
	printf("<p><input class=largeButton title='Simple example must select archive above' type=submit name=gcCommand value='Top OP Stats Report'><p>");

	if(uLog && cLabel[0] && strcmp(cLabel,"Del") && guPermLevel>10 
			&& uTablePK[0] && cTableName[0])
		LogMonthSummary();

	CloseFieldSet();

}//void ExttLogMonthButtons(void)


void ExttLogMonthAuxTable(void)
{

}//void ExttLogMonthAuxTable(void)


void ExttLogMonthGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uLog"))
		{
			sscanf(gentries[i].val,"%u",&uLog);
			guMode=6;
		}
	}
	tLogMonth("");

}//void ExttLogMonthGetHook(entry gentries[], int x)


void ExttLogMonthSelect(void)
{
	ExtSelect("tLogMonth",VAR_LIST_tLogMonth,0);
/*
	if(guLoginClient==1 && guPermLevel>11)//Root can read access all
		sprintf(gcQuery,"SELECT " VAR_LIST_tZoneImport " FROM tZoneImport ORDER BY uZone");
	else 
		sprintf(gcQuery,"SELECT "VAR_LIST_tZoneImport" FROM tZoneImport," TCLIENT
				" WHERE tZoneImport.uOwner=" TCLIENT ".uClient"
				" AND (" TCLIENT ".uClient=%1$u OR " TCLIENT ".uOwner"
				" IN (SELECT uClient FROM " TCLIENT " WHERE uOwner=%1$u OR uClient=%1$u))"
				" ORDER BY uZone",
					guCompany);
*/
}//void ExttLogMonthSelect(void)


void ExttLogMonthSelectRow(void)
{
	ExtSelectRow("tLogMonth",VAR_LIST_tLogMonth,uLog);

}//void ExttLogMonthSelectRow(void)


void ExttLogMonthListSelect(void)
{
	char cCat[512];

	ExtListSelect("tLogMonth",VAR_LIST_tLogMonth);

	//Changes here must be reflected below in ExttLogMonthListFilter()
        if(!strcmp(gcFilter,"uLog"))
        {
                sscanf(gcCommand,"%u",&uLog);
		strcat(gcQuery," WHERE ");
		sprintf(cCat,"tLogMonth.uLog=%u ORDER BY uLog",uLog);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"cLabel"))
        {
		strcat(gcQuery," WHERE ");
		sprintf(cCat,"tLogMonth.cLabel LIKE '%s%%' ORDER BY uLog",
				TextAreaSave(gcCommand));
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uLogType"))
        {
                sscanf(gcCommand,"%u",&uLogType);
		strcat(gcQuery," WHERE ");
		sprintf(cCat,"tLogMonth.uLogType=%u ORDER BY uLog",uLogType);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uLoginClient"))
        {
                sscanf(gcCommand,"%u",&uLoginClient);
		strcat(gcQuery," WHERE ");
		sprintf(cCat,"tLogMonth.uLoginClient=%u ORDER BY uLog",uLoginClient);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"cLogin"))
        {
		strcat(gcQuery," WHERE ");
		sprintf(cCat,"tLogMonth.cLogin LIKE '%s%%' ORDER BY cLogin,uLog",
				TextAreaSave(gcCommand));
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"cHost"))
        {
		strcat(gcQuery," WHERE ");
		sprintf(cCat,"tLogMonth.cHost LIKE '%s%%' ORDER BY cHost,uLog",
				TextAreaSave(gcCommand));
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uTablePK"))
        {
		strcat(gcQuery," WHERE ");
		sprintf(cCat,"tLogMonth.uTablePK=%s ORDER BY cTableName,uTablePK,uLog",gcCommand);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uZone"))
        {
		unsigned uZone=0;

                sscanf(gcCommand,"%u",&uZone);
		sprintf(cCat,",tResource WHERE tLogMonth.uTablePK=tResource.uResource AND cTableName='tResource' AND tResource.uZone=%u",uZone);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uLog");
        }

}//void ExttLogMonthListSelect(void)


void ExttLogMonthListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uLog"))
                printf("<option>uLog</option>");
        else
                printf("<option selected>uLog</option>");
        if(strcmp(gcFilter,"cLabel"))
                printf("<option>cLabel</option>");
        else
                printf("<option selected>cLabel</option>");
        if(strcmp(gcFilter,"uLogType"))
                printf("<option>uLogType</option>");
        else
                printf("<option selected>uLogType</option>");
        if(strcmp(gcFilter,"uLoginClient"))
                printf("<option>uLoginClient</option>");
        else
                printf("<option selected>uLoginClient</option>");
        if(strcmp(gcFilter,"cLogin"))
                printf("<option>cLogin</option>");
        else
                printf("<option selected>cLogin</option>");
        if(strcmp(gcFilter,"cHost"))
                printf("<option>cHost</option>");
        else
                printf("<option selected>cHost</option>");
        if(strcmp(gcFilter,"uTablePK"))
                printf("<option>uTablePK</option>");
        else
                printf("<option selected>uTablePK</option>");
        if(strcmp(gcFilter,"uZone"))
                printf("<option>uZone</option>");
        else
                printf("<option selected>uZone</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttLogMonthListFilter(void)


void ExttLogMonthNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(guPermLevel>9)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttLogMonthNavBar(void)



void LogMonthSummary(void)
{
	unsigned uTPK=0;

	sscanf(uTablePK,"%u",&uTPK);

	printf("<u>LogMonthSummary</u><br>\n");
	if(!strcmp(cTableName,"tZone"))
	{
		printf("tZone<blockquote>\n");
		printf("%s<br>\n",ForeignKey("tZone","cZone",uTPK));
		printf("</blockquote>\n");
	}
	else if(!strcmp(cTableName,"tResource"))
	{
		unsigned uZone=0;
		unsigned uRRType=0;

		sscanf(ForeignKey("tResource","uZone",uTPK),"%u",&uZone);
		sscanf(ForeignKey("tResource","uRRType",uTPK),"%u",&uRRType);

		printf("<a class=darkLink title='Jump to tResource entry' href=?gcFunction=tResource&uResource=%u>tResource</a><blockquote>\n",uTPK);
		printf("cZone=%s<br>\n",ForeignKey("tZone","cZone",uZone));
		printf("cName=%s<br>\n",ForeignKey("tResource","cName",uTPK));
		printf("RRType=%s<br>\n",ForeignKey("tRRType","cLabel",uRRType));
		printf("cParam1=%.32s<br>\n",ForeignKey("tResource","cParam1",uTPK));
		printf("cParam2=%s<br>\n",ForeignKey("tResource","cParam2",uTPK));
		printf("cComment=%s<br>\n",ForeignKey("tResource","cComment",uTPK));
		printf("Contact=%s<br></blockquote>\n",
			ForeignKey(TCLIENT,"cLabel",uLoginClient));
	}
	else if(1)
	{
		printf("No summary available for %s\n",cTableName);
	}
}//void LogMonthSummary(void);

