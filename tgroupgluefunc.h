/*
FILE
	svn ID removed
	(Built initially by unixservice.com mysqlRAD2)
PURPOSE
	Non schema-dependent table and application table related functions.
AUTHOR/LEGAL
	(C) 2001-2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file included.
*/

//ModuleFunctionProtos()
void tTableMultiplePullDown(const char *cTableName,const char *cFieldName,const char *cOrderby);


void ExtProcesstGroupGlueVars(pentry entries[], int x)
{
	register int i;
	for(i=0;i<x;i++)
	{
/*
		if(!strcmp(entries[i].name,"cuMulContainerPullDown") && !strcmp(gcCommand,"Add Multiple Containers"))
		{
			sprintf(cuMulContainerPullDown,"%.255s",entries[i].val);
			uResource=ReadPullDown("tContainer","cLabel",cuMulContainerPullDown);
			uGroupGlue=0;
			uZone=0;
			if(uResource && uGroup) NewtGroupGlue(1);
		}
		else if(!strcmp(entries[i].name,"cuMulContainerPullDown") && !strcmp(gcCommand,"Del Multiple Containers"))
		{
			sprintf(cuMulContainerPullDown,"%.255s",entries[i].val);
			uResource=ReadPullDown("tContainer","cLabel",cuMulContainerPullDown);
			if(uResource && uGroup)
			{
				sprintf(gcQuery,"DELETE FROM tGroupGlue WHERE uResource=%u AND uGroup=%u",
						uResource,uGroup);
				MYSQL_RUN;
			}
		
		}
*/
	}

}//void ExtProcesstGroupGlueVars(pentry entries[], int x)


void ExttGroupGlueCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tGroupGlueTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=12)
			{
	                        ProcesstGroupGlueVars(entries,x);
                        	guMode=2000;
	                        tGroupGlue(LANG_NB_CONFIRMNEW);
			}
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=12)
			{
                        	ProcesstGroupGlueVars(entries,x);

                        	guMode=2000;
				if(!uGroup || (uZone==0 && uResource==0))
	                        	tGroupGlue("Must Supply a uGroup and a uZone or uResource");
                        	guMode=0;

				uGroupGlue=0;
				NewtGroupGlue(0);
			}
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
			if(guPermLevel>=12)
			{
                        	ProcesstGroupGlueVars(entries,x);
	                        guMode=2001;
				tGroupGlue(LANG_NB_CONFIRMDEL);
			}
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
			if(guPermLevel>=12)
			{
                        	ProcesstGroupGlueVars(entries,x);
				guMode=5;
				DeletetGroupGlue();
			}
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
			if(guPermLevel>=12)
			{
                        	ProcesstGroupGlueVars(entries,x);
				guMode=2002;
				tGroupGlue(LANG_NB_CONFIRMMOD);
			}
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
			if(guPermLevel>=12)
			{
                        	ProcesstGroupGlueVars(entries,x);
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				ModtGroupGlue();
			}
                }
	}

}//void ExttGroupGlueCommands(pentry entries[], int x)


void ExttGroupGlueButtons(void)
{
	OpenFieldSet("tGroupGlue Aux Panel",100);
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
			printf("This is just a glue table, see [New], [Delete] and <a href=?gcFunction=tGroup>tGroup</a>"
				" for more info.<br>");
	}
	CloseFieldSet();

}//void ExttGroupGlueButtons(void)


void ExttGroupGlueAuxTable(void)
{

}//void ExttGroupGlueAuxTable(void)


void ExttGroupGlueGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uGroupGlue"))
		{
			sscanf(gentries[i].val,"%u",&uGroupGlue);
			guMode=6;
		}
	}
	tGroupGlue("");

}//void ExttGroupGlueGetHook(entry gentries[], int x)


void ExttGroupGlueSelect(void)
{
	sprintf(gcQuery,"SELECT %s FROM tGroupGlue ORDER BY uGroupGlue",VAR_LIST_tGroupGlue);

}//void ExttGroupGlueSelect(void)


void ExttGroupGlueSelectRow(void)
{
	sprintf(gcQuery,"SELECT %s FROM tGroupGlue WHERE uGroupGlue=%u",VAR_LIST_tGroupGlue,uGroupGlue);

}//void ExttGroupGlueSelectRow(void)


void ExttGroupGlueListSelect(void)
{
	char cCat[512];

	sprintf(gcQuery,"SELECT %s FROM tGroupGlue",VAR_LIST_tGroupGlue);

	//Changes here must be reflected below in ExttGroupGlueListFilter()
        if(!strcmp(gcFilter,"uGroupGlue"))
        {
                sscanf(gcCommand,"%u",&uGroupGlue);
		sprintf(cCat," WHERE tGroupGlue.uGroupGlue=%u ORDER BY uGroupGlue",uGroupGlue);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uGroup"))
        {
                sscanf(gcCommand,"%u",&uGroup);
		sprintf(cCat," WHERE tGroupGlue.uGroup=%u ORDER BY uGroup",uGroup);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uZone"))
        {
                sscanf(gcCommand,"%u",&uZone);
		sprintf(cCat," WHERE tGroupGlue.uZone=%u ORDER BY uZone",uZone);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uResource"))
        {
                sscanf(gcCommand,"%u",&uResource);
		sprintf(cCat," WHERE tGroupGlue.uResource=%u ORDER BY uResource",uResource);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uGroupGlue");
        }

}//void ExttGroupGlueListSelect(void)


void ExttGroupGlueListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uGroupGlue"))
                printf("<option>uGroupGlue</option>");
        else
                printf("<option selected>uGroupGlue</option>");
        if(strcmp(gcFilter,"uGroup"))
                printf("<option>uGroup</option>");
        else
                printf("<option selected>uGroup</option>");
        if(strcmp(gcFilter,"uZone"))
                printf("<option>uZone</option>");
        else
                printf("<option selected>uZone</option>");
        if(strcmp(gcFilter,"uResource"))
                printf("<option>uResource</option>");
        else
                printf("<option selected>uResource</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttGroupGlueListFilter(void)


void ExttGroupGlueNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(guPermLevel>=12 && !guListMode)
		printf(LANG_NBB_NEW);

	if(guPermLevel>=12 && !guListMode)
		printf(LANG_NBB_MODIFY);

	if(guPermLevel>=12 && !guListMode)
		printf(LANG_NBB_DELETE);

	if(guPermLevel>=12)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttGroupGlueNavBar(void)


void tTableMultiplePullDown(const char *cTableName,const char *cFieldName,const char *cOrderby)
{
        register int i,n;
        char cLabel[256];
        MYSQL_RES *mysqlRes;         
        MYSQL_ROW mysqlField;

        char cSelectName[100]={""};
	char cHidden[100]={""};
        char cLocalTableName[256]={""};
        char *cp;

        if(!cTableName[0] || !cFieldName[0] || !cOrderby[0])
        {
                printf("Invalid input tTableMultiplePullDown()");
                return;
        }

        //Extended functionality
        strncpy(cLocalTableName,cTableName,255);
        if((cp=strchr(cLocalTableName,';')))
        {
                strncpy(cSelectName,cp+1,99);
                cSelectName[99]=0;
                *cp=0;
        }


        sprintf(gcQuery,"SELECT _rowid,%s FROM %s ORDER BY %s",
                                cFieldName,cLocalTableName,cOrderby);

	MYSQL_RUN_STORE_TEXT_RET_VOID(mysqlRes);
	
	i=mysql_num_rows(mysqlRes);

	if(cSelectName[0])
                sprintf(cLabel,"%s",cSelectName);
        else
                sprintf(cLabel,"%s_%sPullDown",cLocalTableName,cFieldName);

        if(i>0)
        {
                printf("<select multiple size=%u name=%s >\n",(i>16)?16:i,cLabel);

                for(n=0;n<i;n++)
                {
                        int unsigned field0=0;

                        mysqlField=mysql_fetch_row(mysqlRes);
                        sscanf(mysqlField[0],"%u",&field0);
			printf("<option>%s</option>\n",mysqlField[1]);
                }
        }
        else
        {
		printf("<select multiple size=1 name=%s><option title='No selection'>---</option></select>\n"
                        ,cLabel);
        }
        printf("</select>\n");
	if(cHidden[0])
		printf("%s",cHidden);

}//tTableMultiplePullDown()

