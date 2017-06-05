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


void tNSSetNavList(void);
void tNSSetMembers(unsigned uNSSet);
void tNSSetZones(unsigned uNSSet);

void ExtProcesstNSSetVars(pentry entries[], int x)
{
	register int i;
	for(i=0;i<x;i++)
	{
		if(!strncmp(entries[i].name,"uNSSet",6))
		{
			unsigned uCheckedNSSet=0;

			sscanf(entries[i].name,"uNSSet%u",&uCheckedNSSet);
			if(uCheckedNSSet)
			{
				sprintf(gcQuery,"DELETE FROM tNSSet WHERE uNSSet=%u AND ( uOwner=%u OR %u>9 )"
					,uCheckedNSSet,guLoginClient,guPermLevel);
				macro_mySQLQueryHTMLError;
			}
		}
	}
}//void ExtProcesstNSSetVars(pentry entries[], int x)


void ExttNSSetCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tNSSetTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstNSSetVars(entries,x);
                        	guMode=2000;
	                        tNSSet(LANG_NB_CONFIRMNEW);
			}
			else
				tNSSet("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=9)
			{
				unsigned uContactParentCompany=0;
                        	ProcesstNSSetVars(entries,x);
				GetClientOwner(guLoginClient,&uContactParentCompany);
				
                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uNSSet=0;
				uCreatedBy=guLoginClient;
				uOwner=uContactParentCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtNSSet(0);
			}
			else
				tNSSet("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstNSSetVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tNSSet(LANG_NB_CONFIRMDEL);
			}
			else
				tNSSet("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstNSSetVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetNSSet();
			}
			else
				tNSSet("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstNSSetVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tNSSet(LANG_NB_CONFIRMMOD);
			}
			else
				tNSSet("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstNSSetVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtNSSet();
			}
			else
				tNSSet("<blink>Error</blink>: Denied by permissions settings");
                }
	}

}//void ExttNSSetCommands(pentry entries[], int x)


void ExttNSSetButtons(void)
{
	OpenFieldSet("tNSSet Aux Panel",100);
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
			tNSSetMembers(uNSSet);
			tNSSetNavList();
			if(uNSSet)
			{
				printf("<p><u>First 10000 zones with this uNSSet</u><br>");
				tNSSetZones(uNSSet);
			}
	}
	CloseFieldSet();

}//void ExttNSSetButtons(void)


void ExttNSSetAuxTable(void)
{

}//void ExttNSSetAuxTable(void)


void ExttNSSetGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uNSSet"))
		{
			sscanf(gentries[i].val,"%u",&uNSSet);
			guMode=6;
		}
	}
	tNSSet("");

}//void ExttNSSetGetHook(entry gentries[], int x)


void ExttNSSetSelect(void)
{

	unsigned uContactParentCompany=0;

	GetClientOwner(guLoginClient,&uContactParentCompany);

	if(guLoginClient==1 && guPermLevel>11)//Root can read access all
		sprintf(gcQuery,"SELECT %s FROM tNSSet ORDER BY"
				" uNSSet",
				VAR_LIST_tNSSet);
	else //If you own it, the company you work for owns the company that owns it,
		//you created it, or your company owns it you can at least read access it
		//select tTemplateSet.cLabel from tTemplateSet,tClient where tTemplateSet.uOwner=tClient.uClient and tClient.uOwner in (select uClient from tClient where uOwner=81 or uClient=51);
	sprintf(gcQuery,"SELECT %s FROM tNSSet,tClient WHERE tNSSet.uOwner=tClient.uClient"
				" AND tClient.uOwner IN (SELECT uClient FROM tClient WHERE uOwner=%u OR uClient=%u)"
				" ORDER BY uNSSet",
					VAR_LIST_tNSSet,uContactParentCompany,uContactParentCompany);
					

}//void ExttNSSetSelect(void)


void ExttNSSetSelectRow(void)
{
	unsigned uContactParentCompany=0;

	GetClientOwner(guLoginClient,&uContactParentCompany);

	if(guLoginClient==1 && guPermLevel>11)//Root can read access all
                sprintf(gcQuery,"SELECT %s FROM tNSSet WHERE uNSSet=%u",
			VAR_LIST_tNSSet,uNSSet);
	else
                sprintf(gcQuery,"SELECT %s FROM tNSSet,tClient"
                                " WHERE tNSSet.uOwner=tClient.uClient"
				" AND tClient.uOwner IN (SELECT uClient FROM tClient WHERE uOwner=%u OR uClient=%u)"
				" AND tNSSet.uNSSet=%u",
                        		VAR_LIST_tNSSet
					,uContactParentCompany,uContactParentCompany
					,uNSSet);

}//void ExttNSSetSelectRow(void)


void ExttNSSetListSelect(void)
{
	char cCat[512];
	unsigned uContactParentCompany=0;
	
	GetClientOwner(guLoginClient,&uContactParentCompany);

	if(guLoginClient==1 && guPermLevel>11)//Root can read access all
		sprintf(gcQuery,"SELECT %s FROM tNSSet",
				VAR_LIST_tNSSet);
	else
		sprintf(gcQuery,"SELECT %s FROM tNSSet,tClient"
				" WHERE tNSSet.uOwner=tClient.uClient"
				" AND tClient.uOwner IN (SELECT uClient FROM tClient WHERE uOwner=%u OR uClient=%u)",
				VAR_LIST_tNSSet
				,uContactParentCompany
				,uContactParentCompany);

	//Changes here must be reflected below in ExttNSSetListFilter()
        if(!strcmp(gcFilter,"uNSSet"))
        {
                sscanf(gcCommand,"%u",&uNSSet);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tNSSet.uNSSet=%u"
						" ORDER BY uNSSet",
						uNSSet);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uNSSet");
        }

}//void ExttNSSetListSelect(void)


void ExttNSSetListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uNSSet"))
                printf("<option>uNSSet</option>");
        else
                printf("<option selected>uNSSet</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttNSSetListFilter(void)


void ExttNSSetNavBar(void)
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

}//void ExttNSSetNavBar(void)


void tNSSetNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	ExtSelect("tNSSet","tNSSet.uNSSet,tNSSet.cLabel",0);
        
	mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tNSSetNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("<p><u>tNSSetNavList</u><br>\n");

	        while((field=mysql_fetch_row(res)))
		{
			if(guLoginClient==1)
				printf("<input type=checkbox name=uNSSet%s>"
					"<a class=darkLink href=?gcFunction=tNSSet"
					"&uNSSet=%s>%s</a><br>\n",field[0],field[0],field[1]);
			else
				printf("<a class=darkLink href=?gcFunction=tNSSet"
					"&uNSSet=%s>%s</a><br>\n",field[0],field[1]);
		}
		if(guLoginClient==1)
			printf("<input title='Deletes all the tNSSet entries marked in checkboxes above'"
				" class=lwarnButton type=submit name=gcCommand value='Delete Checked'>");
	}
        mysql_free_result(res);

}//void tNSSetNavList(void)


void tNSSetMembers(unsigned uNSSet)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	if(uNSSet==0) return;

	sprintf(gcQuery,"SELECT tNS.uNS,tNS.cFQDN,tNSType.cLabel FROM tNSSet,tNS,tNSType WHERE"
			" tNS.uNSSet=tNSSet.uNSSet AND tNS.uNSType=tNSType.uNSType AND"
			" tNSSet.uNSSet=%u ORDER BY tNS.cFQDN",uNSSet);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("tNSSetMembers w/tNSType:<br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("tNSSetMembers w/tNSType:<br>\n");

	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=?gcFunction=tNS"
				"&uNS=%s>%s %s</a><br>\n",
				field[0],field[1],field[2]);
	}
        mysql_free_result(res);

	sprintf(gcQuery,"SELECT tNS.uNS,tNS.cFQDN,tServer.cLabel FROM tNSSet,tNS,tServer WHERE"
			" tNS.uNSSet=tNSSet.uNSSet AND tNS.uServer=tServer.uServer AND"
			" tNSSet.uNSSet=%u ORDER BY tNS.cFQDN",uNSSet);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("tNSSetMembers w/tServer:<br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("tNSSetMembers w/tServer:<br>\n");

	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=?gcFunction=tNS"
				"&uNS=%s>%s %s</a><br>\n",
				field[0],field[1],field[2]);
	}
        mysql_free_result(res);

}//void tNSSetMembers(unsigned uNSSet)


void tNSSetZones(unsigned uNSSet)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	if(uNSSet==0) return;

	sprintf(gcQuery,"SELECT tZone.uZone,tZone.cZone,tZone.uView FROM tZone,tNSSet WHERE"
			" tZone.uNSSet=tNSSet.uNSSet"
			" AND tNSSet.uNSSet=%u AND tZone.uView=2 ORDER BY tZone.cZone",uNSSet);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	unsigned uCount=0;
	unsigned uNumRows=0;
	if((uNumRows=mysql_num_rows(res)))
	{	
        	printf("tZone.cZone (Count %u) for loaded uNSSet (external view only):<br>\n",uNumRows);

	        while((field=mysql_fetch_row(res)))
		{
			printf("<a class=darkLink href=?gcFunction=tZone"
				"&uZone=%s>%s</a><br>\n",
				field[0],field[1]);
			uCount++;
			if(uCount>10000) break;
		}
	}
        mysql_free_result(res);

}//void tNSSetZones(unsigned uNSSet)
