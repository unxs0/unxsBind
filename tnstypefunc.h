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


void tNSTypeNavList(void);
void tNSTypeContextInfo(void);

void ExtProcesstNSTypeVars(pentry entries[], int x)
{
	/*
	register int i;
	for(i=0;i<x;i++)
	{
	}
	*/
}//void ExtProcesstNSTypeVars(pentry entries[], int x)


void ExttNSTypeCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tNSTypeTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstNSTypeVars(entries,x);
                        	guMode=2000;
	                        tNSType(LANG_NB_CONFIRMNEW);
			}
			else
				tNSType("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=9)
			{
				unsigned uContactParentCompany=0;
                        	ProcesstNSTypeVars(entries,x);
				GetClientOwner(guLoginClient,&uContactParentCompany);
				
                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uNSType=0;
				uCreatedBy=guLoginClient;
				uOwner=uContactParentCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtNSType(0);
			}
			else
				tNSType("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstNSTypeVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tNSType(LANG_NB_CONFIRMDEL);
			}
			else
				tNSType("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstNSTypeVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetNSType();
			}
			else
				tNSType("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstNSTypeVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tNSType(LANG_NB_CONFIRMMOD);
			}
			else
				tNSType("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstNSTypeVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtNSType();
			}
			else
				tNSType("<blink>Error</blink>: Denied by permissions settings");
                }
	}

}//void ExttNSTypeCommands(pentry entries[], int x)


void ExttNSTypeButtons(void)
{
	OpenFieldSet("tNSType Aux Panel",100);
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
			tNSTypeContextInfo();
			tNSTypeNavList();
	}
	CloseFieldSet();

}//void ExttNSTypeButtons(void)


void ExttNSTypeAuxTable(void)
{

}//void ExttNSTypeAuxTable(void)


void ExttNSTypeGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uNSType"))
		{
			sscanf(gentries[i].val,"%u",&uNSType);
			guMode=6;
		}
	}
	tNSType("");

}//void ExttNSTypeGetHook(entry gentries[], int x)


void ExttNSTypeSelect(void)
{
	ExtSelect("tNSType",VAR_LIST_tNSType,0);

}//void ExttNSTypeSelect(void)


void ExttNSTypeSelectRow(void)
{
	ExtSelectRow("tNSType",VAR_LIST_tNSType,uNSType);
}//void ExttNSTypeSelectRow(void)


void ExttNSTypeListSelect(void)
{
	char cCat[512];
	
	ExtListSelect("tNSType",VAR_LIST_tNSType);

	//Changes here must be reflected below in ExttNSTypeListFilter()
        if(!strcmp(gcFilter,"uNSType"))
        {
                sscanf(gcCommand,"%u",&uNSType);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tNSType.uNSType=%u"
						" ORDER BY uNSType",
						uNSType);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uNSType");
        }

}//void ExttNSTypeListSelect(void)


void ExttNSTypeListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uNSType"))
                printf("<option>uNSType</option>");
        else
                printf("<option selected>uNSType</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttNSTypeListFilter(void)


void ExttNSTypeNavBar(void)
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

}//void ExttNSTypeNavBar(void)


void tNSTypeNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	ExtSelect("tNSType","tNSType.uNSType,tNSType.cLabel",0);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tNSTypeNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("<p><u>tNSTypeNavList</u><br>\n");

	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=?gcFunction=tNSType"
				"&uNSType=%s>%s</a><br>\n",
				field[0],field[1]);
	}
        mysql_free_result(res);

}//void tNSTypeNavList(void)


void tNSTypeContextInfo(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	if(!uNSType) return;

	sprintf(gcQuery,"SELECT COUNT(tZone.uZone) FROM tNSSet,tNS,tZone WHERE"
			" tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND"
			" tNS.uNSType=%u",uNSType);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("tNSTypeContextInfo:<br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
        	printf("tNSTypeContextInfo:<br>\n");
		printf("%s zones are assigned to NS sets that have NS associated with this type.",field[0]);
	}
        mysql_free_result(res);

}//void tNSTypeContextInfo(void)


