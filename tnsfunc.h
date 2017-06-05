/*
FILE
	svn ID removed
	(Built initially by unixservice.com mysqlRAD2)
PURPOSE
	Non schema-dependent table and application table related functions.
AUTHOR
	(C) 2001-2009 Gary Wallis and Hugo Urquiza for Unixservice.
 
*/

//ModuleFunctionProtos()


void tNSNavList(void);
void tNSContextInfo(void);

void ExtProcesstNSVars(pentry entries[], int x)
{
	register int i;
	for(i=0;i<x;i++)
	{
		if(!strncmp(entries[i].name,"uNS",3))
		{
			unsigned uCheckedNS=0;

			sscanf(entries[i].name,"uNS%u",&uCheckedNS);
			if(uCheckedNS)
			{
				sprintf(gcQuery,"DELETE FROM tNS WHERE uNS=%u AND ( uOwner=%u OR %u>9 )"
					,uCheckedNS,guLoginClient,guPermLevel);
				macro_mySQLQueryHTMLError;
			}
		}
	}

}//void ExtProcesstNSVars(pentry entries[], int x)


void ExttNSCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tNSTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstNSVars(entries,x);
                        	guMode=2000;
	                        tNS(LANG_NB_CONFIRMNEW);
			}
			else
				tNS("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=9)
			{
				unsigned uContactParentCompany=0;
                        	ProcesstNSVars(entries,x);
				GetClientOwner(guLoginClient,&uContactParentCompany);
				
                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uNS=0;
				uCreatedBy=guLoginClient;
				uOwner=uContactParentCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtNS(0);
			}
			else
				tNS("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstNSVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tNS(LANG_NB_CONFIRMDEL);
			}
			else
				tNS("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstNSVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetNS();
			}
			else
				tNS("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstNSVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tNS(LANG_NB_CONFIRMMOD);
			}
			else
				tNS("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstNSVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtNS();
			}
			else
				tNS("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,"Create tServer"))
                {
			if(guPermLevel>10)
			{
        			MYSQL_RES *res;
				MYSQL_ROW field;
        			MYSQL_RES *res2;
				MYSQL_ROW field2;

                        	ProcesstNSVars(entries,x);
                        	guMode=0;

				sprintf(gcQuery,"SELECT cFQDN,uNS FROM tNS WHERE cFQDN!='' AND uServer=0");
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					tNS(mysql_error(&gMysql));
			        res=mysql_store_result(&gMysql);
	        		while((field=mysql_fetch_row(res)))
				{

					uServer=0;
					sprintf(gcQuery,"SELECT uServer FROM tServer WHERE cLabel='%s'",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						tNS(mysql_error(&gMysql));
				        res2=mysql_store_result(&gMysql);
		        		if((field2=mysql_fetch_row(res2)))
						sscanf(field2[0],"%u",&uServer);
					mysql_free_result(res2);
	
					if(!uServer)
					{
						sprintf(gcQuery,"INSERT INTO tServer SET cLabel='%s'",field[0]);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
							tNS(mysql_error(&gMysql));
						uServer=mysql_insert_id(&gMysql);
					}

					if(uServer)
					{
						sprintf(gcQuery,"UPDATE tNS SET uServer=%u WHERE uNS=%s",uServer,field[1]);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
							tNS(mysql_error(&gMysql));
					}
				}
				mysql_free_result(res);

				if(mysql_affected_rows(&gMysql))
					tNS("tServer was changed");
				else
					tNS("tServer was not changed");
			}
			else
				tNS("<blink>Error</blink>: Denied by permissions settings");
		}
	}

}//void ExttNSCommands(pentry entries[], int x)


void ExttNSButtons(void)
{
	OpenFieldSet("tNS Aux Panel",100);
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
			printf("These NSs are what are used to create NS sets. Also see tServer,tNSSet and tNSType.");
			if(guPermLevel>10)
			printf("<p><input class=largeButton title='Add tServer records based on cFQDN and associate' " 
				"type=submit name=gcCommand value='Create tServer'><br>\n");
			printf("<p><u>Record Context Info</u><br>");
			tNSContextInfo();
			tNSNavList();
	}
	CloseFieldSet();

}//void ExttNSButtons(void)


void ExttNSAuxTable(void)
{

}//void ExttNSAuxTable(void)


void ExttNSGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uNS"))
		{
			sscanf(gentries[i].val,"%u",&uNS);
			guMode=6;
		}
	}
	tNS("");

}//void ExttNSGetHook(entry gentries[], int x)


void ExttNSSelect(void)
{
	ExtSelect("tNS",VAR_LIST_tNS,0);

}//void ExttNSSelect(void)


void ExttNSSelectRow(void)
{
	ExtSelectRow("tNS",VAR_LIST_tNS,uNS);

}//void ExttNSSelectRow(void)


void ExttNSListSelect(void)
{
	char cCat[512];
	
	ExtListSelect("tNS",VAR_LIST_tNS);

	//Changes here must be reflected below in ExttNSListFilter()
        if(!strcmp(gcFilter,"uNS"))
        {
                sscanf(gcCommand,"%u",&uNS);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tNS.uNS=%u"
						" ORDER BY uNS",
						uNS);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uNS");
        }

}//void ExttNSListSelect(void)


void ExttNSListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uNS"))
                printf("<option>uNS</option>");
        else
                printf("<option selected>uNS</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttNSListFilter(void)


void ExttNSNavBar(void)
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

}//void ExttNSNavBar(void)


void tNSNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	ExtSelect("tNS","tNS.uNS,tNS.cFQDN",0);
        
	mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tNSNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res))
	{	
        	printf("<p><u>tNSNavList</u><br>\n");

	        while((field=mysql_fetch_row(res)))
		{
			if(guLoginClient==1)
				printf("<input type=checkbox name=uNS%s><a class=darkLink href=?gcFunction=tNS"
					"&uNS=%s>%s</a><br>\n",field[0],field[0],field[1]);
			else
				printf("<a class=darkLink href=?gcFunction=tNS"
					"&uNS=%s>%s</a><br>\n",field[0],field[1]);
		}
		if(guLoginClient==1)
			printf("<input title='Deletes all the tNS entries marked in checkboxes above'"
				" class=lwarnButton type=submit name=gcCommand value='Delete Checked'>");
	}
        mysql_free_result(res);

}//void tNSNavList(void)


void tNSContextInfo(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	if(!uNS) return;

	sprintf(gcQuery,"SELECT COUNT(tZone.uZone) FROM tNSSet,tNS,tZone WHERE"
			" tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND"
			" tNS.uNS=%u",uNS);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("tNSContextInfo:<br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
        	printf("tNSContextInfo:<br>\n");
		printf("%s zones are assigned to NS sets that contain this NS.",field[0]);
	}
        mysql_free_result(res);

}//void tNSContextInfo(void)


