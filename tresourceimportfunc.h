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



void ExtProcesstResourceImportVars(pentry entries[], int x)
{
	/*
	register int i;
	for(i=0;i<x;i++)
	{
	}
	*/
}//void ExtProcesstResourceImportVars(pentry entries[], int x)


void ExttResourceImportCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tResourceImportTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>11)
			{
	                        ProcesstResourceImportVars(entries,x);
                        	guMode=2000;
	                        tResourceImport(LANG_NB_CONFIRMNEW);
			}
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>11)
			{
                        	ProcesstResourceImportVars(entries,x);

                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uZone=0;
				uCreatedBy=guLoginClient;
				uOwner=guCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtResourceImport(0);
			}
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstResourceImportVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tResourceImport(LANG_NB_CONFIRMDEL);
			}
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstResourceImportVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetResourceImport();
			}
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstResourceImportVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tResourceImport(LANG_NB_CONFIRMMOD);
			}
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstResourceImportVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtResourceImport();
			}
                }
	}

}//void ExttResourceImportCommands(pentry entries[], int x)


void ExttResourceImportButtons(void)
{
	OpenFieldSet("tResourceImport Aux Panel",100);
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
			if(uZone)
			{
				printf("<a class=darkLink href=?gcFunction=tZoneImport&uZone=%u>",uZone);
				printf("Back to Zone</a>");
			}
	}
	CloseFieldSet();

}//void ExttResourceImportButtons(void)


void ExttResourceImportAuxTable(void)
{

}//void ExttResourceImportAuxTable(void)


void ExttResourceImportGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uResource"))
		{
			sscanf(gentries[i].val,"%u",&uResource);
			guMode=6;
		}
	}
	tResourceImport("");

}//void ExttResourceImportGetHook(entry gentries[], int x)


void ExttResourceImportSelect(void)
{
        //Set non search gcQuery here for tTableName()
	if(guPermLevel>=9)
	sprintf(gcQuery,"SELECT %s FROM tResourceImport ORDER BY\
					uZone",
					VAR_LIST_tResourceImport);
	else
	sprintf(gcQuery,"SELECT %s FROM tResourceImport WHERE uOwner=%u ORDER BY\
					uZone",
					VAR_LIST_tResourceImport,guLoginClient);

}//void ExttResourceImportSelect(void)


void ExttResourceImportSelectRow(void)
{
	sprintf(gcQuery,"SELECT %s FROM tResourceImport WHERE uResource=%u",
			VAR_LIST_tResourceImport,uResource);

}//void ExttResourceImportSelectRow(void)


void ExttResourceImportListSelect(void)
{
	char cCat[512];

	if(guPermLevel<10)
		sprintf(gcQuery,"SELECT %s FROM tResourceImport,tClient \
		WHERE tResourceImport.uOwner=tClient.uClient \
		AND (tClient.uOwner=%u OR tClient.uClient=%u)",
				VAR_LIST_tResourceImport,
				guLoginClient,
				guLoginClient);
	else
                sprintf(gcQuery,"SELECT %s FROM tResourceImport",
				VAR_LIST_tResourceImport);

	//Changes here must be reflected below in ExttResourceImportListFilter()
        if(!strcmp(gcFilter,"uZone"))
        {
                sscanf(gcCommand,"%u",&uZone);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tResourceImport.uZone=%u \
						ORDER BY uZone",
						uZone);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uZone");
        }

}//void ExttResourceImportListSelect(void)


void ExttResourceImportListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uZone"))
                printf("<option>uZone</option>");
        else
                printf("<option selected>uZone</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttResourceImportListFilter(void)


void ExttResourceImportNavBar(void)
{
	if(uOwner) GetClientOwner(uOwner,&guReseller);

	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(guPermLevel>11 && !guListMode)
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

}//void ExttResourceImportNavBar(void)




