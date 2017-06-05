/*
FILE
	svn ID removed
	(Built initially by unixservice.com mysqlRAD2)
PURPOSE
	Non schema-dependent table and application table related functions.
AUTHOR
	(C) 2001-2007 Gary Wallis.
 
*/

//ModuleFunctionProtos()


void tGroupNavList(void);
void tGroupMemberNavList(void);

void ExtProcesstGroupVars(pentry entries[], int x)
{
	/*
	register int i;
	for(i=0;i<x;i++)
	{
	}
	*/
}//void ExtProcesstGroupVars(pentry entries[], int x)


void ExttGroupCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tGroupTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=10)
			{
	                        ProcesstGroupVars(entries,x);
                        	guMode=2000;
	                        tGroup(LANG_NB_CONFIRMNEW);
			}
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=10)
			{
                        	ProcesstGroupVars(entries,x);

                        	guMode=2000;
				//Check entries here
				if(strlen(cLabel)<4)
					tGroup("<blink>Error</blink>: cLabel too short");
				if(!uGroupType)
					tGroup("<blink>Error</blink>: uGroupType must be selected");
                        	guMode=0;

				uGroup=0;
				uCreatedBy=guLoginClient;
				GetClientOwner(guLoginClient,&guReseller);
				uOwner=guReseller;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
			
				NewtGroup(1);
				mysql_insert_id(&gMysql);
				tGroup("New group created properties copied from group type");
			}
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstGroupVars(entries,x);
			if(uOwner) GetClientOwner(uOwner,&guReseller);
			if( (guPermLevel>=12 && uOwner==guLoginClient)
				|| (guPermLevel>9 && uOwner!=1 && uOwner!=0)
				|| (guPermLevel>7 && guReseller==guLoginClient) )
			{
	                        guMode=2001;
				tGroup(LANG_NB_CONFIRMDEL);
			}
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstGroupVars(entries,x);
			if(uOwner) GetClientOwner(uOwner,&guReseller);
			if( (guPermLevel>=12 && uOwner==guLoginClient)
				|| (guPermLevel>9 && uOwner!=1 && uOwner!=0)
				|| (guPermLevel>7 && guReseller==guLoginClient) )
			{
				guMode=5;
				DeletetGroup();
			}
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstGroupVars(entries,x);
			if(uOwner) GetClientOwner(uOwner,&guReseller);
			if( (guPermLevel>=10 && uOwner==guLoginClient)
				|| (guPermLevel>9 && uOwner!=1 && uOwner!=0)
				|| (guPermLevel>7 && guReseller==guLoginClient) )
			{
				guMode=2002;
				tGroup(LANG_NB_CONFIRMMOD);
			}
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstGroupVars(entries,x);
			if(uOwner) GetClientOwner(uOwner,&guReseller);
			if( (guPermLevel>=10 && uOwner==guLoginClient)
				|| (guPermLevel>9 && uOwner!=1 && uOwner!=0)
				|| (guPermLevel>7 && guReseller==guLoginClient) )
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtGroup();
			}
                }
	}

}//void ExttGroupCommands(pentry entries[], int x)


void ExttGroupButtons(void)
{
	OpenFieldSet("tGroup Aux Panel",100);
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
			printf("tGroup (and it's related tables <a href=?gcFunction=tGroupGlue>"
				"tGroupGlue</a> and <a href=?gcFunction=tGroupType>tGroupType</a>)"
				" provide a way to create"
				" container and node groups of different types."
				" These groups can then be used for organizational and/or autonomic purposes.");
			if(uGroup) tGroupMemberNavList();
			tGroupNavList();
	}
	CloseFieldSet();

}//void ExttGroupButtons(void)


void ExttGroupAuxTable(void)
{

}//void ExttGroupAuxTable(void)


void ExttGroupGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uGroup"))
		{
			sscanf(gentries[i].val,"%u",&uGroup);
			guMode=6;
		}
	}
	tGroup("");

}//void ExttGroupGetHook(entry gentries[], int x)


void ExttGroupSelect(void)
{
	ExtSelect("tGroup",VAR_LIST_tGroup,0);

}//void ExttGroupSelect(void)


void ExttGroupSelectRow(void)
{
	ExtSelectRow("tGroup",VAR_LIST_tGroup,uGroup);

}//void ExttGroupSelectRow(void)


void ExttGroupListSelect(void)
{
	char cCat[512];

	ExtListSelect("tGroup",VAR_LIST_tGroup);

	//Changes here must be reflected below in ExttGroupListFilter()
        if(!strcmp(gcFilter,"uGroup"))
        {
                sscanf(gcCommand,"%u",&uGroup);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tGroup.uGroup=%u \
						ORDER BY uGroup",
						uGroup);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uGroup");
        }

}//void ExttGroupListSelect(void)


void ExttGroupListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uGroup"))
                printf("<option>uGroup</option>");
        else
                printf("<option selected>uGroup</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttGroupListFilter(void)


void ExttGroupNavBar(void)
{
	if(uOwner) GetClientOwner(uOwner,&guReseller);

	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(guPermLevel>=10 && !guListMode)
		printf(LANG_NBB_NEW);

			if( (guPermLevel>=10 && uOwner==guLoginClient)
				|| (guPermLevel>9 && uOwner!=1 && uOwner!=0)
				|| (guPermLevel>7 && guReseller==guLoginClient) )
		printf(LANG_NBB_MODIFY);

			if( (guPermLevel>=12 && uOwner==guLoginClient)
				|| (guPermLevel>9 && uOwner!=1 && uOwner!=0)
				|| (guPermLevel>7 && guReseller==guLoginClient) )
		printf(LANG_NBB_DELETE);

	if(uOwner)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttGroupNavBar(void)


void tGroupNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

        ExtSelect("tGroup","tGroup.uGroup,tGroup.cLabel",0);

	mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tGroupNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }
        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("<p><u>tGroupNavList</u><br>\n");
	        while((field=mysql_fetch_row(res)))
			printf("<a class=darkLink href=?gcFunction=tGroup&uGroup=%s>%s</a><br>\n",field[0],field[1]);
	}
        mysql_free_result(res);

}//void tGroupNavList(void)


void tGroupMemberNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	sprintf(gcQuery,"SELECT uResource,uZone FROM tGroupGlue WHERE uGroup=%u LIMIT 33",uGroup);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tGroupMemberNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }
        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{
		unsigned uResource,uZone,uLimit=0;
        	MYSQL_RES *res2;
        	MYSQL_ROW field2;

        	printf("<p><u>tGroupMemberNavList</u><br>\n");
	        while((field=mysql_fetch_row(res)))
		{	
			if(++uLimit>32)
			{
				printf("(Only first 32 shown)<br>");
				break;
			}

			sscanf(field[0],"%u",&uResource);
			sscanf(field[1],"%u",&uZone);
			if(uResource)
			{
				sprintf(gcQuery,"SELECT cName FROM tResource WHERE uResource=%u",uResource);
        			mysql_query(&gMysql,gcQuery);
			        if(mysql_errno(&gMysql))
			        {
			        	printf("<p><u>tGroupMemberNavList</u><br>\n");
			                printf("%s",mysql_error(&gMysql));
			                return;
			        }
			        res2=mysql_store_result(&gMysql);
	        		if((field2=mysql_fetch_row(res2)))
					printf("<a class=darkLink href=?gcFunction=tResource&uResource=%u>%s</a><br>\n",
							uResource,field2[0]);
				mysql_free_result(res2);
			}
			else if(uZone)
			{
				sprintf(gcQuery,"SELECT cZone FROM tZone WHERE uZone=%u",uZone);
        			mysql_query(&gMysql,gcQuery);
			        if(mysql_errno(&gMysql))
			        {
			        	printf("<p><u>tGroupMemberNavList</u><br>\n");
			                printf("%s",mysql_error(&gMysql));
			                return;
			        }
			        res2=mysql_store_result(&gMysql);
	        		if((field2=mysql_fetch_row(res2)))
					printf("<a class=darkLink href=?gcFunction=tZone&uZone=%u>%s</a><br>\n",uZone,field2[0]);
				mysql_free_result(res2);
			}
		}
	}
        mysql_free_result(res);

}//void tGroupMemberNavList(void)

