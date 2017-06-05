/*
FILE
	svn ID removed
PURPOSE
	Non-schema dependent trrtype.c expansion.
AUTHOR
	GPL License applies, see www.fsf.org for details
	See LICENSE file in this distribution
	(C) 2001-2009 Gary Wallis and Hugo Urquiza.
*/

void tRRTypeNavList(void);

void ExtProcesstRRTypeVars(pentry entries[], int x)
{

	/*
	register int i;
	
	for(i=0;i<x;i++)
	{
	
	}
	*/

}//void ExtProcesstRRTypeVars(pentry entries[], int x)


void ExttRRTypeCommands(pentry entries[], int x)
{
	if(!strcmp(gcFunction,"tRRTypeTools"))
	{
		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=12)
			{
				ProcesstRRTypeVars(entries,x);
				//Check global conditions for new record here
				guMode=2000;
				tRRType(LANG_NB_CONFIRMNEW);
			}
			else
				tRRType("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=12)
			{
				ProcesstRRTypeVars(entries,x);
				//Check entries here
				uRRType=0;
				uCreatedBy=guLoginClient;
				uOwner=guCompany;
				uModBy=0;//Never modified
				NewtRRType(0);
			}
			else
				tRRType("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstRRTypeVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=2001;
				tRRType(LANG_NB_CONFIRMDEL);
			}
			else
				tRRType("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstRRTypeVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetRRType();
			}
			else
				tRRType("<blink>Error</blink>: Denied by permissions settings");	
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstRRTypeVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tRRType(LANG_NB_CONFIRMMOD);
			}
			else
				tRRType("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstRRTypeVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				uModBy=guLoginClient;
				ModtRRType();
			}
			else
				tRRType("<blink>Error</blink>: Denied by permissions settings");
                }
	}

}//void ExttRRTypeCommands(pentry entries[], int x)


void ExttRRTypeButtons(void)
{
	OpenFieldSet("Aux Panel",100);

	switch(guMode)
        {
                case 2000:
			printf("Enter required data<br>");
                        printf(LANG_NBB_CONFIRMNEW);
			printf("<br>\n");
                break;

                case 2001:
                        printf(LANG_NBB_CONFIRMDEL);
			printf("<br>\n");
                break;

                case 2002:
			printf("Review record data<br>");
                        printf(LANG_NBB_CONFIRMMOD);
			printf("<br>\n");
                break;

		default:
			printf("<p><u>tRRTypeNavList</u><br>\n");
			tRRTypeNavList();

	}


	CloseFieldSet();
}//void ExttRRTypeButtons(void)


void ExttRRTypeAuxTable(void)
{

}//void ExttRRTypeAuxTable(void)


void ExttRRTypeGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uRRType"))
		{
			sscanf(gentries[i].val,"%u",&uRRType);
			guMode=6;
		}
	}
	tRRType("");

}//void ExttRRTypeGetHook(entry gentries[], int x)


void ExttRRTypeSelect(void)
{
	ExtSelect("tRRType",VAR_LIST_tRRType,0);

}//void ExttRRTypeSelect(void)


void ExttRRTypeSelectRow(void)
{
	ExtSelectRow("tRRType",VAR_LIST_tRRType,uRRType);

}//void ExttRRTypeSelectRow(void)


void ExttRRTypeListSelect(void)
{
	char cCat[512];

	ExtListSelect("tRRType",VAR_LIST_tRRType);

	//Changes here must be reflected below in ExttRRTypeListFilter()
        if(!strcmp(gcFilter,"uRRType"))
        {
                sscanf(gcCommand,"%u",&uRRType);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tRRType.uRRType=%u ORDER BY uRRType",uRRType);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uRRType");
        }

}//void ExttRRTypeListSelect(void)


void ExttRRTypeListFilter(void)
{
        //Filter
        printf("<td align=right >Select ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uRRType"))
                printf("<option>uRRType</option>");
        else
                printf("<option selected>uRRType</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttRRTypeListFilter(void)


void ExttRRTypeNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(guPermLevel>=10 && !guListMode)
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

}//void ExttRRTypeNavBar(void)


void tRRTypeNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	ExtSelect("tRRType","tRRType.uRRType,tRRType.cLabel",20);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
        while((field=mysql_fetch_row(res)))
        {
                printf("<a class=darkLink href=?gcFunction=tRRType&uRRType=%s>%s</a><br>\n",field[0],field[1]);
        }
        mysql_free_result(res);

}//void tRRTypeNavList(void)


// vim:tw=78
//perlSAR patch1
