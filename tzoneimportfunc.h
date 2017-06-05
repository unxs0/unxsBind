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

static char cExtZone[256]={""};
void SerialNum(char *cSerialNum);//bind.c

void ExtProcesstZoneImportVars(pentry entries[], int x)
{
}//void ExtProcesstZoneImportVars(pentry entries[], int x)


void ExttZoneImportCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tZoneImportTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=11)
			{
	                        ProcesstZoneImportVars(entries,x);
                        	guMode=2000;
	                        tZoneImport(LANG_NB_CONFIRMNEW);
			}
			else
				tZoneImport("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=11)
			{
                        	ProcesstZoneImportVars(entries,x);

                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uZone=0;
				uCreatedBy=guLoginClient;
				uOwner=guCompany;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtZoneImport(0);
			}
			else
				tZoneImport("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstZoneImportVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tZoneImport(LANG_NB_CONFIRMDEL);
			}
			else
				tZoneImport("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstZoneImportVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetZoneImport();
			}
			else
				tZoneImport("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstZoneImportVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tZoneImport(LANG_NB_CONFIRMMOD);
			}
			else
				tZoneImport("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstZoneImportVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				uModBy=guLoginClient;
				ModtZoneImport();
			}
			else
				tZoneImport("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,"Remove Zone NSs"))
                {
                        ProcesstZoneImportVars(entries,x);
			if( guPermLevel>=12 && uZone )
			{
				sprintf(gcQuery,"DELETE FROM tResourceImport WHERE (cName='' OR cName='\t') AND uRRType=2");
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
				tZoneImport("Imported zone NSs RRs removed");
			}
			else
				tZoneImport("<blink>Error</blink>: Denied by permissions settings");
		}
                else if(!strcmp(gcCommand,"Import Zone"))
                {
                        ProcesstZoneImportVars(entries,x);
			if( guPermLevel>=12 && uZone && uView)
			{
				unsigned uNewZone=0;
				MYSQL_RES *res;
				char cSerialNum[16];

				SerialNum(cSerialNum);

				sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=%u",cZone,uView);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
				res=mysql_store_result(&gMysql);

				if(mysql_num_rows(res)==0)
				{
					mysql_free_result(res);
					sprintf(gcQuery,"INSERT INTO tZone (cZone,uNSSet,cHostmaster,"
							"uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
							"uMailServers,uView,cMainAddress,uRegistrar,"
							"uSecondaryOnly,cOptions,uOwner,uCreatedBy,"
							"uCreatedDate,uModBy,uModDate) SELECT cZone,"
							"uNSSet,cHostmaster,%s,uExpire,uRefresh,"
							"uTTL,uRetry,uZoneTTL,uMailServers,uView,"
							"cMainAddress,uRegistrar,uSecondaryOnly,cOptions,"
							"uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate "
							"FROM tZoneImport WHERE tZoneImport.uZone=%u",cSerialNum,uZone);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));

					uNewZone=mysql_insert_id(&gMysql);
					sprintf(gcQuery,"INSERT INTO tResource (uZone,cName,uTTL,uRRType,cParam1,"
							"cParam2,cComment,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate)"
							" SELECT %u,cName,uTTL,uRRType,cParam1,cParam2,cComment,uOwner,"
							"uCreatedBy,uCreatedDate,uModBy,uModDate FROM tResourceImport "
							"WHERE tResourceImport.uZone=%u",uNewZone,uZone);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
					tZoneImport("Single zone imported into tZone/tResource");
				}
				else
				{
					mysql_free_result(res);
					tZoneImport("Zone already exists. You can't use this function");
				}
			}
			else
				tZoneImport("<blink>Error</blink>: Denied by permissions settings");

		}
                else if(!strcmp(gcCommand,"Import All Zones"))
                {
                        ProcesstZoneImportVars(entries,x);
			if( guPermLevel>=12 && uZone )
			{
				unsigned uNewZone=0;
				MYSQL_RES *res;
				MYSQL_ROW field;
				MYSQL_RES *res2;
				char cSerialNum[16];

				SerialNum(cSerialNum);

				printf("Content-type: text/plain\n\n");
				printf("ImportAllZones() Start\n");
				sprintf(gcQuery,"SELECT uZone,cZone FROM tZoneImport ORDER BY cZone");
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
				{
					printf("%s\n",mysql_error(&gMysql));
					exit(0);
				}
				res=mysql_store_result(&gMysql);
				while((field=mysql_fetch_row(res)))
				{
					sscanf(field[0],"%u",&uZone);
					sprintf(cZone,"%.255s",field[1]);
					sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=2",cZone);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						printf("%s\n",mysql_error(&gMysql));
						exit(0);
					}
					res2=mysql_store_result(&gMysql);

					if(mysql_num_rows(res2)==0)
					{
						mysql_free_result(res2);
						printf("%s importing\n",cZone);
						sprintf(gcQuery,"INSERT INTO tZone (cZone,uNSSet,cHostmaster,uSerial,"
								"uExpire,uRefresh,uTTL,uRetry,uZoneTTL,uMailServers,uView,"
								"cMainAddress,uRegistrar,uSecondaryOnly,cOptions,uOwner,"
								"uCreatedBy,uCreatedDate,uModBy,uModDate) SELECT cZone,"
								"uNSSet,cHostmaster,%s,uExpire,uRefresh,uTTL,uRetry,"
								"uZoneTTL,uMailServers,uView,cMainAddress,uRegistrar,"
								"uSecondaryOnly,cOptions,uOwner,uCreatedBy,uCreatedDate,"
								"uModBy,uModDate FROM tZoneImport WHERE tZoneImport.uZone=%u"
									,cSerialNum,uZone);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
						{
							printf("%s\n",mysql_error(&gMysql));
							exit(0);
						}
	
						uNewZone=mysql_insert_id(&gMysql);
						sprintf(gcQuery,"INSERT INTO tResource (uZone,cName,uTTL,uRRType,cParam1,"
								"cParam2,cComment,uOwner,uCreatedBy,uCreatedDate,uModBy,"
								"uModDate) SELECT %u,cName,uTTL,uRRType,cParam1,cParam2,"
								"cComment,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate"
								" FROM tResourceImport "
								"WHERE tResourceImport.uZone=%u",uNewZone,uZone);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
						{
							printf("%s\n",mysql_error(&gMysql));
							exit(0);
						}
					}
					else
					{
						mysql_free_result(res2);
						printf("%s already in tZone\n",cZone);
					}
				}
				mysql_free_result(res);
				printf("ImportAllZones() End\n");
				exit(0);
			}
			else
				tZoneImport("<blink>Error</blink>: Denied by permissions settings");
		}
                else if(!strcmp(gcCommand,"Diff Report"))
                {
                      ProcesstZoneImportVars(entries,x);
		      if( guPermLevel>=12 && uZone )
		      {
				MYSQL_RES *res;
				MYSQL_ROW field;
				MYSQL_RES *res2;
				MYSQL_ROW field2;
				MYSQL_RES *res3;
				MYSQL_ROW field3;
				MYSQL_RES *res4;
				MYSQL_ROW field4;
				printf("Content-type: text/plain\n\n");
				printf("DiffReport() Start\ntResource tResourceImport\n\n");
				sprintf(gcQuery,"SELECT cZone,uZone FROM tZoneImport ORDER BY cZone");
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
				{
					printf("%s\n",mysql_error(&gMysql));
					exit(0);
				}
				res=mysql_store_result(&gMysql);
				while((field=mysql_fetch_row(res)))
				{
					sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=2",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						printf("%s\n",mysql_error(&gMysql));
						exit(0);
					}
					res2=mysql_store_result(&gMysql);
					if(mysql_num_rows(res2)>0)
					{
						field2=mysql_fetch_row(res2);
						sprintf(gcQuery,"SELECT tResource.cName,tResource.uTTL,tRRType.cLabel,"
								"tResource.cParam1,tResource.cParam2,tResource.cComment "
								"FROM tResource,tZone,tRRType WHERE"
								" tResource.uRRType=tRRType.uRRType "
								"AND tResource.uZone=tZone.uZone AND tZone.uZone=%s",
									field2[0]);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
						{
							printf("%s\n",mysql_error(&gMysql));
							exit(0);
						}
						res3=mysql_store_result(&gMysql);
						while((field3=mysql_fetch_row(res3)))
						{
							sprintf(gcQuery,"SELECT tResourceImport.cName,tResourceImport.uTTL,"
									"tRRType.cLabel,tResourceImport.cParam1,"
									"tResourceImport.cParam2,"
									"tResourceImport.cComment FROM tResourceImport,"
									"tZoneImport,tRRType "
									"WHERE tResourceImport.uRRType=tRRType.uRRType AND "
									"tResourceImport.uZone=tZoneImport.uZone AND"
									" tZoneImport.uZone=%s "
									"AND tResourceImport.cName='%s' AND"
									" tResourceImport.uTTL=%s AND "
									"tRRType.cLabel='%s' AND"
									" tResourceImport.cParam1='%s' AND "
									"tResourceImport.cParam2='%s'",
									field[1],
									field3[0],field3[1],
									field3[2],field3[3],
									field3[4]);
							mysql_query(&gMysql,gcQuery);
							if(mysql_errno(&gMysql))
							{
								printf("%s\n",mysql_error(&gMysql));
								exit(0);
							}
							res4=mysql_store_result(&gMysql);
							if((field4=mysql_fetch_row(res4)))
							{
								//printf("%s\t%s\t%s\t%s\t%s\t%s\n",
								//		field4[0],field4[1],
								//		field4[2],field4[3],
								//		field4[4],field4[5]);
								;
							}
							else
							{
								printf("%s\n",field[0]);
								printf("<<<%s\t%s\t%s\t%s\t%s\n",
										field3[0],field3[1],
										field3[2],field3[3],
										field3[4]);
							}
							mysql_free_result(res4);
						}
						mysql_free_result(res3);

						sprintf(gcQuery,"SELECT tResourceImport.cName,tResourceImport.uTTL,"
								"tRRType.cLabel,"
								"tResourceImport.cParam1,tResourceImport.cParam2,"
								"tResourceImport.cComment"
								" FROM tResourceImport,tZoneImport,tRRType WHERE "
								"tResourceImport.uRRType=tRRType.uRRType AND"
								" tResourceImport.uZone=tZoneImport.uZone "
								"AND tZoneImport.uZone=%s",field[1]);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
						{
							printf("%s\n",mysql_error(&gMysql));
							exit(0);
						}
						res3=mysql_store_result(&gMysql);
						while((field3=mysql_fetch_row(res3)))
						{
							sprintf(gcQuery,"SELECT tResource.cName,tResource.uTTL,"
									"tRRType.cLabel,"
									"tResource.cParam1,tResource.cParam2,"
									"tResource.cComment "
									"FROM tResource,tZone,tRRType WHERE"
									" tResource.uRRType=tRRType.uRRType "
									"AND tResource.uZone=tZone.uZone AND tZone.uZone=%s"
									" AND tResource.cName='%s' AND tResource.uTTL=%s AND"
									" tRRType.cLabel='%s' AND tResource.cParam1='%s' AND"
									" tResource.cParam2='%s'",
									field2[0],
									field3[0],field3[1],
									field3[2],field3[3],
									field3[4]);
							mysql_query(&gMysql,gcQuery);
							if(mysql_errno(&gMysql))
							{
								printf("%s\n",mysql_error(&gMysql));
								exit(0);
							}
							res4=mysql_store_result(&gMysql);
							if((field4=mysql_fetch_row(res4)))
							{
								//printf("%s\t%s\t%s\t%s\t%s\t%s\n",
								//		field4[0],field4[1],
								//		field4[2],field4[3],
								//		field4[4],field4[5]);
								;
							}
							else
							{
								printf(">>>%s\t%s\t%s\t%s\t%s\n\n",
										field3[0],field3[1],
										field3[2],field3[3],
										field3[4]);
							}
							mysql_free_result(res4);
					}
					mysql_free_result(res3);
				}
				else
				{
					printf("%s Only in tZoneImport\n",field[0]);
				}
				mysql_free_result(res2);
			}
			mysql_free_result(res);
			printf("DiffReport() End\n");
			exit(0);
		      }
		      else
		      	tZoneImport("<blink>Error</blink>: Denied by permissions settings");
		}
                else if(!strcmp(gcCommand,"Add Selected RRs"))
                {
			ProcesstZoneImportVars(entries,x);
			if( guPermLevel>=12 && uZone )
			{
				register int i;
				unsigned uResource;
				unsigned uTargetZone=0;
				MYSQL_RES *res;
				MYSQL_ROW field;

				printf("Content-type: text/plain\n\n");
				printf("AddSelectedRRs() Start\n");

				//Get required tZone data
				sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=2 AND uSecondaryOnly=0",
						cZone);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
				{
					printf("%s\n",mysql_error(&gMysql));
					exit(0);
				}
				res=mysql_store_result(&gMysql);
				if((field=mysql_fetch_row(res)))
					sscanf(field[0],"%u",&uTargetZone);
				mysql_free_result(res);

				if(!uTargetZone)
				{
					printf("AddSelectedRRs() Stopped: No appropiate uZone\n");
					exit(0);
				}

				for(i=0;i<x;i++)
				{
					if(!strncmp(entries[i].name,"ImportRR",8))
					{
						sscanf(entries[i].name,"ImportRR%u",&uResource);
						printf("Attempting to add %u to %s(%u)\n",
							uResource,cZone,uTargetZone);
						//Insert into tResource
						sprintf(gcQuery,"INSERT INTO tResource (uZone,cName,uTTL,uRRType,"
								"cParam1,cParam2,cComment,uOwner,uCreatedBy,uCreatedDate,"
								"uModBy,uModDate) SELECT %u,cName,uTTL,uRRType,cParam1,"
								"cParam2,'AddSelectedRRs()',uOwner,uCreatedBy,uCreatedDate,"
								"uModBy,uModDate FROM tResourceImport WHERE "
								"tResourceImport.uResource=%u",uTargetZone,uResource);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
						{
							printf("%s\n",mysql_error(&gMysql));
							break;
						}
						//Only once create a mod job
					}
				}
				printf("AddSelectedRRs() End\n");
				exit(0);
			}
			else
				tZoneImport("<blink>Error</blink>: Denied by permissions settings");
		}
	}

}//void ExttZoneImportCommands(pentry entries[], int x)


void ExttZoneImportButtons(void)
{
	OpenFieldSet("tZoneImport Aux Panel",100);
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
			if(uZone && guPermLevel>11)
			{
				printf("<u>Import Tools</u><p>\n");

				printf("<input title='Import the current zone with all its RRs as is' "
					"class=lwarnButton type=submit name=gcCommand value='Import Zone'><p>\n");
				printf("<input title='Import all zones with all their RRs as is' class=lwarnButton "
					"type=submit name=gcCommand value='Import All Zones'><p>\n");
				printf("<input title='Generate a list of all differences bewtween tZoneImport and tZone' "
					"class=largeButton type=submit name=gcCommand value='Diff Report'><p>\n");
				printf("<input title='Remove all tResourceImport zone NS RRs' class=lwarnButton "
					"type=submit name=gcCommand value='Remove Zone NSs'><p>\n");
				printf("<input title='Add checked RRs to production tables' class=lwarnButton "
					"type=submit name=gcCommand value='Add Selected RRs'><p>\n");
			}
			else
			{
				printf("No zones or no permission. Use command line to import.\n");
			}

	}
	CloseFieldSet();

}//void ExttZoneImportButtons(void)


void ExttZoneImportGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uZone"))
		{
			sscanf(gentries[i].val,"%u",&uZone);
			guMode=6;
		}
		else if(!strcmp(gentries[i].name,"cZone"))
		{
			sprintf(cExtZone,"%255s",gentries[i].val);
		}
	}
	tZoneImport("");

}//void ExttZoneImportGetHook(entry gentries[], int x)


void ExttZoneImportSelect(void)
{
	ExtSelect("tZoneImport",VAR_LIST_tZoneImport,0);

}//void ExttZoneImportSelect(void)


void ExttZoneImportSelectRow(void)
{
	ExtSelectRow("tZoneImport",VAR_LIST_tZoneImport,uZone);

}//void ExttZoneImportSelectRow(void)


void ExttZoneImportListSelect(void)
{
	char cCat[512];

	ExtListSelect("tZoneImport",VAR_LIST_tZoneImport);

	//Changes here must be reflected below in ExttZoneImportListFilter()
        if(!strcmp(gcFilter,"uZone"))
        {
                sscanf(gcCommand,"%u",&uZone);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tZoneImport.uZone=%u \
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

}//void ExttZoneImportListSelect(void)


void ExttZoneImportListFilter(void)
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

}//void ExttZoneImportListFilter(void)


void ExttZoneImportNavBar(void)
{
	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(guPermLevel>=11 && !guListMode)
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

}//void ExttZoneImportNavBar(void)


void ResourceImportRecordList(unsigned uZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	MYSQL_RES *res2;
	MYSQL_ROW field2;
	register int i=0;
	
	char cTTL[16]={"&nbsp;"};
	char cName[102];
	char cRed[4]={"Red"};
	char cBlack[6]={"Black"};

//	return;	
	sprintf(gcQuery,"SELECT tResourceImport.cName,tResourceImport.uTTL,tRRType.cLabel,"
			"tResourceImport.cParam1,tResourceImport.cParam2,tResourceImport.cComment,"
			"tResourceImport.uResource FROM tResourceImport,tRRType WHERE "
			"tResourceImport.uRRType=tRRType.uRRType AND tResourceImport.uZone=%u "
			"ORDER BY tResourceImport.uResource",uZone);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);


	OpenFieldSet("tZoneImport Resource Records",100);
	printf("<tr bgcolor=black><td ><font color=white>Name</td><td ><font color=white>TTL</td>"
		"<td ><font color=white>Type</td><td ><font color=white>Param 1</td>"
		"<td ><font color=white>Param 2</td ><td><font color=white>Comment</td></tr>");

	while((field=mysql_fetch_row(res)))
	{
		char *cColorName=cBlack;
		char *cColorTTL=cBlack;
		char *cColorType=cBlack;
		char *cColorParam1=cBlack;
		char *cColorParam2=cBlack;
		char *cColorComment=cBlack;

		//Compare against possible existing records
		sprintf(gcQuery,"SELECT tResource.cName,tResource.uTTL,tRRType.cLabel,tResource.cParam1,"
				"tResource.cParam2,tResource.cComment,tResource.uResource FROM "
				"tZone,tResource,tRRType WHERE tResource.uZone=tZone.uZone AND "
				"tResource.uRRType=tRRType.uRRType AND tZone.cZone='%s' AND "
				"tZone.uView=2 AND tResource.cName='%s' AND tRRType.cLabel='%s' AND tResource.cParam1='%s'",
				cZone,field[0],field[2],TextAreaSave(field[3]));
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res2=mysql_store_result(&gMysql);
		field2=mysql_fetch_row(res2);

		if(field2==NULL)
		{
			cColorName=cRed;
			cColorTTL=cRed;
			cColorType=cRed;
			cColorParam1=cRed;
			cColorParam2=cRed;
			cColorComment=cRed;
		}

		if(strcmp(field[1],"0"))
			sprintf(cTTL,"%.15s",field[1]);
		else
			sprintf(cTTL,"&nbsp;");

		if(!field[0][0] || field[0][0]=='\t')
			strcpy(cName,"@");
		else
			strcpy(cName,field[0]);

		//Compare and set color to red if it does not match current tResource
		if(field2!=NULL)
		{
			if(strcmp(field[1],field2[1]))
				cColorTTL=cRed;
			else
				cColorTTL=cBlack;
			if(strcmp(field[4],field2[4]))
				cColorParam2=cRed;
			else
				cColorParam2=cBlack;
			if(strcmp(field[5],field2[5]))
				cColorComment=cRed;
			else
				cColorComment=cBlack;
		}

		if(strcmp(cColorName,cRed))
		{
			printf("<tr><td valign=top><a class=darkLink href=?gcFunction="
			"tResourceImport&uResource=%s&cZone=%s>"
			"<font color=%s>%s</font></a></td><td valign=top><font color=%s>%s</font></td><td valign=top>"
			"<font color=%s>%s</font></td><td valign=top><font color=%s>%.64s</font></td><td valign=top>"
			"<font color=%s>%s</font></td><td valign=top><font color=%s>%s</font></td></tr>\n",
				field[6],cZone,cColorName,cName,
				cColorTTL,cTTL,cColorType,field[2],
				cColorParam1,field[3],cColorParam2,field[4],cColorComment,field[5]);
		}
		else
		{
			printf("<tr><td valign=top><input type=checkbox name=ImportRR%s checked>"
			"<a class=darkLink href=?gcFunction="
			"tResourceImport&uResource=%s&cZone=%s>"
			"<font color=%s>%s</font></a>"
			"</td><td valign=top>"
			"<font color=%s>%s</font></td><td valign=top><font color=%s>%s</font></td><td valign=top>"
			"<font color=%s>%.64s</font></td><td valign=top><font color=%s>%s</font></td>"
			"<td valign=top><font color=%s>%s</font></td></tr>\n",
				field[6],field[6],cZone,cColorName,cName,
				cColorTTL,cTTL,cColorType,field[2],
				cColorParam1,field[3],cColorParam2,field[4],cColorComment,field[5]);
		}
		i++;
		mysql_free_result(res2);
	}
	mysql_free_result(res);

	if(!i)
		printf("<tr><td colspan=6>No resource records</td></tr>\n");

	CloseFieldSet();


}//void ResourceImportRecordList(void)


void ExttZoneImportAuxTable(void)
{
	ResourceImportRecordList(uZone);

}//void ExttZoneImportAuxTable(void)
