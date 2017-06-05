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

#include <math.h>
#include <openisp/ucidr.h>

static char cSearch[64]={""};
//Aux drop/pull downs
static char cForClientPullDown[256]={""};
static unsigned uForClient=0;
void tTablePullDownResellers(unsigned uSelector,unsigned uMode);
void tBlockNavList(void);
void tBlockContextInfo(void);

//Block assignment tool vars
static char cIPBlock[256]={""};
static char cZone[256]={""};
static unsigned uZone=0;
static unsigned uDryRun=0;

void UpdateSerialNum(unsigned uZone);
void CustomerDropDown(unsigned uClient);

void ExtProcesstBlockVars(pentry entries[], int x)
{
	register int i;
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cForClientPullDown"))
		{
			strcpy(cForClientPullDown,entries[i].val);
			uForClient=ReadPullDown(TCLIENT,"cLabel",cForClientPullDown);
		}
		else if(!strcmp(entries[i].name,"cSearch"))
			sprintf(cSearch,"%.63s",entries[i].val);
		else if(!strcmp(entries[i].name,"cIPBlock"))
			sprintf(cIPBlock,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"uDryRun"))
			uDryRun=1;
	}

}//void ExtProcesstBlockVars(pentry entries[], int x)


void ExttBlockCommands(pentry entries[], int x)
{

	if(!strcmp(gcFunction,"tBlockTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=10)
			{
	                        ProcesstBlockVars(entries,x);
                        	guMode=2000;
	                        tBlock(LANG_NB_CONFIRMNEW);
			}
			else
				tBlock("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=10)
			{
                        	ProcesstBlockVars(entries,x);

                        	guMode=2000;
				//Check entries here
                        	guMode=0;

				uBlock=0;
				uCreatedBy=guLoginClient;
				if(!uForClient)
					uOwner=guCompany;
				else
					uOwner=uForClient;
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				NewtBlock(0);
			}
			else
				tBlock("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstBlockVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
	                        guMode=2001;
				tBlock(LANG_NB_CONFIRMDEL);
			}
			else
				tBlock("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstBlockVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				guMode=5;
				DeletetBlock();
			}
			else
				tBlock("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstBlockVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=2002;
				tBlock(LANG_NB_CONFIRMMOD);
			}
			else
				tBlock("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstBlockVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
				//Check entries here
                        	guMode=0;

				if(uForClient)
				{
					char cZone[100]={""};
					sprintf(gcQuery,"UPDATE tBlock SET uOwner=%u WHERE uBlock=%u",uForClient,uBlock);
					mysql_query(&gMysql,gcQuery);
        				if(mysql_errno(&gMysql))
                				tBlock(mysql_error(&gMysql));
					uOwner=uForClient;

					//Ticket #81
					unsigned uA=0;
					unsigned uB=0;
					unsigned uC=0;
					unsigned uD=0;
					unsigned uE=0;
					unsigned uNumIPs=0;
					unsigned uBlockEnd=0;
					unsigned uI=0;
					MYSQL_RES *res;
					
					//remove extra spaces or any other junk in CIDR
					sscanf(cLabel,"%s",gcQuery);
					sprintf(cLabel,"%.99s",gcQuery);
					sscanf(cLabel,"%u.%u.%u.%u/%u",&uA,&uB,&uC,&uD,&uE);
			
					if(!uA)
					{
						guMode=2002;
						tBlock("<blink>IP Block incorrect format</blink>");
					}
					if((uA>255)||(uB>255)||(uC>255)||(uD>255))
					{
						guMode=2002;
						tBlock("<blink>IP Block incorrect format</blink>");
					}
					if(uE<21 || uE>29)
					{
						guMode=2002;
						tBlock("<blink>This tool is only for /21 to /29 blocks</blink>");
					}

					sprintf(cZone,"%u.%u.%u.in-addr.arpa",uC,uB,uA);
					sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",cZone);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
					res=mysql_store_result(&gMysql);
					if(mysql_num_rows(res))
					{
						MYSQL_ROW field;
						uNumIPs=uGetNumIPs(cLabel);
						uNumIPs+=2; //uCIDRLib TODO Add 2 since uNumIPs are the usable IPs.
						uBlockEnd=uD+uNumIPs;

						//Loop is for handling all zone views
						while((field=mysql_fetch_row(res)))
						{
							for(uI=uD;uI<uBlockEnd;uI++)
							{
								sprintf(gcQuery,"UPDATE tResource SET uOwner=%u "
										"WHERE cName='%i' AND uZone='%s' "
										"AND uRRType=7",uForClient,uI,field[0]);
								mysql_query(&gMysql,gcQuery);
								if(mysql_errno(&gMysql))
									htmlPlainTextError(mysql_error(&gMysql));
							}
						}
					}
				}//if(uForClient)	
				uModBy=guLoginClient;
				ModtBlock();
			}
			else
				tBlock("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,"Block Assignment Tools"))
		{
			ProcesstZoneVars(entries,x);
			guMode=6000;
			tBlock("");
		}
		else if(!strcmp(gcCommand,"Assign IP Block"))
		{
			unsigned uA=0;
			unsigned uB=0;
			unsigned uC=0;
			unsigned uD=0;
			unsigned uE=0;
			unsigned uNumIPs=0;
			unsigned uNumNets=0;
			unsigned uI=0;
			MYSQL_RES *res;
			
			ProcesstBlockVars(entries,x);

			if(!uForClient)
			{
				guMode=6000;
				tBlock("<blink>Must select a company/organization first</blink>");
			}

			if(!cIPBlock[0])
			{
				guMode=6000;
				tBlock("<blink>Must enter a block in CIDR format</blink>");
			}
					
			//remove extra spaces or any other junk in CIDR
			sscanf(cIPBlock,"%s",gcQuery);
			sprintf(cIPBlock,"%.99s",gcQuery);
			sscanf(cIPBlock,"%u.%u.%u.%u/%u",&uA,&uB,&uC,&uD,&uE);
			sprintf(cIPBlock,"%u.%u.%u.%u/%u",uA,uB,uC,uD,uE);
			
			if(!uA)
			{
				guMode=6000;
				tBlock("<blink>IP Block incorrect format</blink>");
			}
			if((uA>255)||(uB>255)||(uC>255)||(uD>255))
			{
				guMode=6000;
				tBlock("<blink>IP Block incorrect format</blink>");
			}
			if(uE<21 || uE>29)
			{
				guMode=6000;
				tBlock("<blink>This tool is only for /21 to /29 blocks</blink>");
			}
			
			uNumIPs=uGetNumIPs(cIPBlock);
			uNumNets=uGetNumNets(cIPBlock);
			
			//Debug Only
			if(uDryRun)
			{
				printf("Content-type: text/plain\n\n");
				printf("Assign IP Block debug/dry-run (use back to return)\n\n");
				printf("Number of networks:%u\n",uNumNets);
				printf("Number of IPs:%u\n",uNumIPs);
			}
			for(uI=uC;uI<((uC+uNumNets));uI++)
			{
				//
				//Create the tZone record if it not exists, otherwise update uOwner
				sprintf(cZone,"%u.%u.%u.in-addr.arpa",uI,uB,uA);
				if(uDryRun)
					printf("cZone:%s\n",cZone);
			
				//TODO External zone only	
				sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=2",cZone);
				//printf("%s\n",gcQuery);
				mysql_query(&gMysql,gcQuery);

				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));

				res=mysql_store_result(&gMysql);
				
				if(!mysql_num_rows(res))
				{
					sprintf(gcQuery,"INSERT INTO tZone SET cZone='%s',uNSSet=1,cHostmaster='%s %s"
							" Assigned',"
							"uExpire=604800,uRefresh=28800,uTTL=86400,uRetry=7200,uZoneTTL=86400,"
							"uView=2,uOwner='%u',uCreatedBy='%u',"
							"uCreatedDate=UNIX_TIMESTAMP(NOW())",
							cZone ,HOSTMASTER,cIPBlock,uForClient,guLoginClient);
					if(uDryRun)
					{
						printf("%s\n",gcQuery);
					}
					else
					{
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
							htmlPlainTextError(mysql_error(&gMysql));
						uZone=mysql_insert_id(&gMysql);
						UpdateSerialNum(uZone);
						SubmitJob("New",1,cZone,0,0);
					}
				}
				else
				{
					MYSQL_ROW field;
					
					sprintf(gcQuery,"UPDATE tZone SET uOwner='%u',cHostmaster='%s %s"
							" Assigned',uModBy='%u',"
							"uModDate=UNIX_TIMESTAMP(NOW()) WHERE cZone='%s' AND uView=2",
							uForClient,HOSTMASTER,cIPBlock,guLoginClient,cZone);
					if(uDryRun)
					{
						printf("%s\n",gcQuery);
					}
					else
					{
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
							htmlPlainTextError(mysql_error(&gMysql));
						field=mysql_fetch_row(res);
						sprintf(gcQuery,"UPDATE tResource SET uOwner='%u' WHERE uZone='%s'",
							uForClient,field[0]);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
							htmlPlainTextError(mysql_error(&gMysql));
					}
				}
				//Create the tBlock entry if it does not exists
				sprintf(cZone,"%u.%u.%u.0/24",uA,uB,uI);
				if(uDryRun)
					printf("cZone:%s\n",cZone);
				
				sprintf(gcQuery,"SELECT uBlock FROM tBlock WHERE cLabel='%s'",cZone);
				mysql_query(&gMysql,gcQuery);
				//printf("%s\n",gcQuery);
				
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
				res=mysql_store_result(&gMysql);

				if(!mysql_num_rows(res))
					sprintf(gcQuery,"INSERT INTO tBlock SET cLabel='%s',"
							"cComment='BlockAssign()',uCreatedBy='%u',"
							"uOwner='%u',uCreatedDate=UNIX_TIMESTAMP(NOW())",
								cZone,guLoginClient,uForClient);
				else
					sprintf(gcQuery,"UPDATE tBlock SET uOwner='%u',uModBy='%u',"
							"uModDate=UNIX_TIMESTAMP(NOW()) WHERE cLabel='%s'",
								uForClient,guLoginClient,cZone);
				if(uDryRun)
				{
					printf("%s\n",gcQuery);
				}
				else
				{
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));	
				}

			}//for
			if(uDryRun)
				exit(0);
			tBlock("CIDR IP Block Assigned OK");
		}
		else if(!strcmp(gcCommand,"Remove Assigned IP Block"))
		{	
			unsigned uA=0;
			unsigned uB=0;
			unsigned uC=0;
			unsigned uD=0;
			unsigned uE=0;
			unsigned uNumIPs=0;
			unsigned uNumNets=0;
			unsigned uI=0;
			MYSQL_RES *res;
			MYSQL_ROW field;
			
			ProcesstBlockVars(entries,x);

			if(!cIPBlock[0])
			{
				guMode=6000;
				tBlock("<blink>Must enter a block in CIDR format</blink>");
			}
					
			//remove extra spaces or any other junk in CIDR
			sscanf(cIPBlock,"%s",gcQuery);
			sprintf(cIPBlock,"%.99s",gcQuery);
			
			sscanf(cIPBlock,"%u.%u.%u.%u/%u",&uA,&uB,&uC,&uD,&uE);
			sprintf(cIPBlock,"%u.%u.%u.%u/%u",uA,uB,uC,uD,uE);

			if(!uA)
			{
				guMode=6000;
				tBlock("<blink>IP Block incorrect format</blink>");
			}
			if((uA>255)||(uB>255)||(uC>255)||(uD>255))
			{
				guMode=6000;
				tBlock("<blink>IP Block incorrect format</blink>");
			}
			if(uE<21 || uE>29)
			{
				guMode=6000;
				tBlock("<blink>This tool is only for /21 to /29 blocks</blink>");
			}
			
			uNumIPs=uGetNumIPs(cIPBlock);
			uNumNets=uGetNumNets(cIPBlock);

			if(uDryRun)
			{
				printf("Content-type: text/plain\n\n");
				printf("Number of networks:%u\n",uNumNets);
				printf("Number of IPs:%u\n",uNumIPs);
				exit(0);
			}

			for(uI=uC;uI<((uC+uNumNets));uI++)
			{
				//
				//Delete the tZone record
				sprintf(cZone,"%u.%u.%u.in-addr.arpa",uI,uB,uA);
				
				sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=2 AND"
						" cHostmaster LIKE '%%%s Assigned'",cZone,cIPBlock);
				//printf("%s\n",gcQuery);
				mysql_query(&gMysql,gcQuery);

				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));

				res=mysql_store_result(&gMysql);

				if((field=mysql_fetch_row(res)))
				{
					sprintf(gcQuery,"DELETE FROM tZone WHERE uZone='%s'",field[0]);
					mysql_query(&gMysql,gcQuery);
					printf("%s\n",gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));

					//Delete orphan tResource records
					sprintf(gcQuery,"DELETE FROM tResource WHERE uZone='%s'",field[0]);
					mysql_query(&gMysql,gcQuery);
					//printf("%s\n",gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));

					SubmitJob("Del",1,cZone,0,0);
					
				}//if((field=mysql_fetch_row(res)))
				
				sprintf(cZone,"%u.%u.%u.0/24",uA,uB,uI);
				
				sprintf(gcQuery,"DELETE FROM tBlock WHERE cLabel='%s'",cZone);
				mysql_query(&gMysql,gcQuery);

				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));	
			}
			//exit(0);

			tBlock("CIDR IP Block Assignment Deleted OK");
		}
	}

}//void ExttBlockCommands(pentry entries[], int x)


void ExttBlockButtons(void)
{
	OpenFieldSet("tBlock Aux Panel",100);
	switch(guMode)
        {
                case 2000:
			printf("<p><u>Enter/mod data</u><br>");
			if(guPermLevel>7)
			{
				printf("<p><u>Create for customer</u><br>");
				tTablePullDownResellers(uForClient,1);
			}
                        printf(LANG_NBB_CONFIRMNEW);
                break;

                case 2001:
                        printf("<p><u>Think twice</u><br>");
                        printf(LANG_NBB_CONFIRMDEL);
                break;

                case 2002:
			printf("<p><u>Review changes</u><br>");
			if(guPermLevel>7)
			{
				printf("<p><u>Change uOwner for customer</u><br>");
				tTablePullDownResellers(uForClient,1);
			}
                        printf(LANG_NBB_CONFIRMMOD);
		break;

		case 6000:
			printf("<p><u>Block Assignment Tools</u></p>\n");
			printf("<p>Here you can create and assign or delete (previously assigned) arpa zones and tBlock"
				" entries in one step. Existing arpa zone PTR resource records in the block also have"
				" their ownership updated.\n");

				printf("<p><u>Assign to customer</u><br>\n");
				tTablePullDownResellers(uForClient,1);
				printf("<p><u>CIDR IP Block</u>\n");
				printf("<br><input title='Ex. entry 10.0.0.0/24' type=text name=cIPBlock"
						" maxlength=18 size=30 value='%s'>\n",cIPBlock);
				printf("<p><input type=submit title='Assigns the CIDR block to "
					"the selected customer' name=gcCommand class=largeButton value='Assign IP Block'>"
					"\n");
				printf("<p><input type=submit title='Deletes the zones and blocks for the "
					"entered CIDR block' name=gcCommand class=lwarnButton value='Remove Assigned"
					" IP Block'>\n");
				printf("<p><input type=checkbox name=uDryRun CHECKED> uDryRun\n");
				
		break;
		
		default:
			printf("<u>Table Tips</u><br>");
			printf("Blocks of IPs are allocated to companies/organizations here. CIDR is spoken and certain "
				"in-addr.arpa zone operations require that the information here be correct.\n");
			printf("<p><u>Search Tools</u><br>");
			printf("<input type=text title='cLabel search. Use %% . and _ for pattern matching when applicable.' "
				"name=cSearch value=\"%s\" maxlength=99 size=20> cSearch",cSearch);
			tBlockNavList();
			printf("<br>\n");
			tBlockContextInfo();
			printf("<br><input class=largeButton title='IP Block Assignment Tools' type=submit name=gcCommand "
				"value='Block Assignment Tools'><br>\n");
			
                break;

	}
	CloseFieldSet();

}//void ExttBlockButtons(void)


void ExttBlockAuxTable(void)
{

}//void ExttBlockAuxTable(void)


void ExttBlockGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uBlock"))
		{
			sscanf(gentries[i].val,"%u",&uBlock);
			guMode=6;
		}
		else if(!strcmp(gentries[i].name,"cSearch"))
		{
			sprintf(cSearch,"%.63s",gentries[i].val);
		}
	}
	tBlock("");

}//void ExttBlockGetHook(entry gentries[], int x)


void ExttBlockSelect(void)
{
	if(cSearch[0])
		ExtSelectSearch("tBlock",VAR_LIST_tBlock,"tBlock.cLabel",cSearch,NULL,20);
	else
		ExtSelect("tBlock",VAR_LIST_tBlock,0);

}//void ExttBlockSelect(void)


void ExttBlockSelectRow(void)
{
	ExtSelectRow("tBlock",VAR_LIST_tBlock,uBlock);

}//void ExttBlockSelectRow(void)


void ExttBlockListSelect(void)
{
	char cCat[512];

	ExtListSelect("tBlock",VAR_LIST_tBlock);

	//Changes here must be reflected below in ExttBlockListFilter()
        if(!strcmp(gcFilter,"uBlock"))
        {
                sscanf(gcCommand,"%u",&uBlock);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tBlock.uBlock=%u ORDER BY uBlock",uBlock);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uOwner"))
        {
                sscanf(gcCommand,"%u",&uOwner);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tBlock.uOwner=%u ORDER BY cLabel",uOwner);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"cLabel"))
        {
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tBlock.cLabel LIKE '%s%%' ORDER BY tBlock.cLabel",
				TextAreaSave(gcCommand));
		strcat(gcQuery,cCat);
	}
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uBlock");
        }

}//void ExttBlockListSelect(void)


void ExttBlockListFilter(void)
{
        //Filter
        printf("&nbsp;&nbsp;&nbsp;Filter on ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uBlock"))
                printf("<option>uBlock</option>");
        else
                printf("<option selected>uBlock</option>");
        if(strcmp(gcFilter,"uOwner"))
                printf("<option title='Input tOwner.uOwner number'>uOwner</option>");
        else
                printf("<option selected>uOwner</option>");
        if(strcmp(gcFilter,"cLabel"))
                printf("<option>cLabel</option>");
        else
                printf("<option selected>cLabel</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttBlockListFilter(void)


void ExttBlockNavBar(void)
{
	if(guMode==6000)
		return;

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

}//void ExttBlockNavBar(void)


void tBlockNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

	unsigned uCount=0;

	if(!cSearch[0])
	{
        	printf("<p><u>tBlockNavList</u><br>\n");
        	printf("Must restrict via cSearch<br>\n");
		return;
	}


	ExtSelectSearch("tBlock","tBlock.uBlock,tBlock.cLabel","tBlock.cLabel",cSearch,NULL,0);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tBlockNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	printf("<p><u>tBlockNavList</u><br>\n");
	if(mysql_num_rows(res))
	{	
	        while((field=mysql_fetch_row(res)))
		{
			uCount++;
			printf("<a class=darkLink href=?gcFunction=tBlock&uBlock=%s&cSearch=%s>%s</a><br>\n",
				field[0],cURLEncode(cSearch),field[1]);
			if(uCount>=100)
			{
				printf("More than 100 records: You must refine your search further<br>\n");
				break;
			}
	        }
	}
	else
		printf("No records found<br>");
		
        mysql_free_result(res);

}//void tBlockNavList(void)


void tBlockContextInfo(void)
{
        printf("<u>Record Context Info</u><br>");
        printf("No context info available<br>");

}//void tBlockContextInfo(void)

