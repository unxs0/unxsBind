/*
FILE 
	report.c
	svn ID removed
AUTHOR/LEGAL
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file in main source dir.
PURPOSE
	iDNSAdmin.cgi interface program file.
*/

#include "interface.h"

static char cSearch[100]={""};
static char *cSearchStyle="type_fields";

static char cuHit[16]={"0"};
static char cuClient[16]={"0"};
static char cMonthHit[16]={""};
static unsigned uCSVReport=1;

static char cNavList[8192]={"No results."};

void ReportSearchZone(char *cLabel);
char *cGetcZone(char *cuHit);
char *cGetClientName(char *cuClient);

void ProcessReportVars(pentry entries[], int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"cSearch"))
			sprintf(cSearch,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cMonthHit"))
			sprintf(cMonthHit,"%.15s",entries[i].val);
	}
	
}//void ProcessReportVars(pentry entries[], int x)

	
void ReportGetHook(entry gentries[],int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uHit"))
			sprintf(cuHit,"%.99s",gentries[i].val);
		if(!strcmp(gentries[i].name,"uClient"))
		{
			sprintf(cuClient,"%.99s",gentries[i].val);
			sprintf(cNavList,"<a href=idnsAdmin.cgi?gcPage=Report&gcFunction=CompanyFocus&uClient=%s>%s Statistics Report</a><br>\n",
					cuClient
					,cGetClientName(cuClient)
					);
		}
	}

	if(!strcmp(gcFunction,"ZoneFocus"))
	{
		htmlReportFocus();
	}
	else if(1)
		htmlReport();
	

}//void ReportGetHook(entry gentries[],int x)


void htmlReport(void)
{
	htmlHeader("idnsAdmin","Header");
	htmlReportPage("idnsAdmin","Report.Body");
	htmlFooter("Footer");

}//void htmlReport(void)


void htmlReportFocus(void)
{
	htmlHeader("idnsAdmin","Header");
	htmlReportPage("idnsAdmin","ReportFocus.Body");
	htmlFooter("Footer");
}//void htmlReportFocus(void)


void htmlReportPage(char *cTitle, char *cTemplateName)
{
	if(cTemplateName[0])
	{
        	MYSQL_RES *res;
	        MYSQL_ROW field;

		TemplateSelectInterface(cTemplateName,uPLAINSET,uIDNSADMINTYPE);
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			struct t_template template;
			char cuResource[16]={""};

			sprintf(cuResource,"%u",uResource);

			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
			
			template.cpName[1]="cCGI";
			template.cpValue[1]="idnsAdmin.cgi";
			
			template.cpName[2]="gcLogin";
			template.cpValue[2]=gcLogin;

			template.cpName[3]="gcName";
			template.cpValue[3]=gcName;

			template.cpName[4]="gcOrgName";
			template.cpValue[4]=gcOrgName;

			template.cpName[5]="cUserLevel";
			template.cpValue[5]=(char *)cUserLevel(guPermLevel);

			template.cpName[6]="gcHost";
			template.cpValue[6]=gcHost;

			template.cpName[7]="gcMessage";
			template.cpValue[7]=gcMessage;

			template.cpName[8]="cSearchStyle";
			template.cpValue[8]=cSearchStyle;

			template.cpName[9]="cNavList";
			template.cpValue[9]=cNavList;

			template.cpName[10]="cZone";
			template.cpValue[10]=gcZone;

			template.cpName[11]="cCustomer";
			template.cpValue[11]=gcCustomer;

			template.cpName[12]="uView";
			template.cpValue[12]=cuView;

			template.cpName[13]="uResource";
			template.cpValue[13]=cuResource;

			template.cpName[14]="cReportZone";
			if(!strcmp(gcFunction,"ZoneFocus"))
				template.cpValue[14]=cGetcZone(cuHit);
			else
				template.cpValue[14]="";

			template.cpName[15]="";

			printf("\n<!-- Start htmlReportPage(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlReportPage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlReportPage()


void ReportCommands(pentry entries[], int x)
{
	if(!strcmp(gcPage,"Report"))
	{
		ProcessReportVars(entries,x);
		if(!strcmp(gcFunction,"Search"))
		{
			MYSQL_RES *res;
			MYSQL_ROW field;
			char cTmp[512]={""};
			unsigned uRows=0;
			
			if(!cSearch[0])
			{
				cSearchStyle="type_fields_req";
				gcMessage="Must provide a valid search term";
				htmlZone();
			}
			
			sprintf(cTmp,"%s%%",cSearch);

			ReportSearchZone(cTmp);
			res=mysql_store_result(&gMysql);

			uRows=mysql_num_rows(res);
			
			if(uRows)
			{
				sprintf(cNavList,"<!-- Search NavList Start -->\n");

				/*if(uRows==1)
				{
					field=mysql_fetch_row(res);					
					cNavList[0]=0;
					sprintf(cuClient,"%s",field[0]);

					htmlReportFocus();
				}
				*/
				while((field=mysql_fetch_row(res)))
				{
					if(strlen(cNavList) > 8000) break; //avoid buffer overflow
					sprintf(cTmp,"<a href=\"idnsAdmin.cgi?gcPage=Report&gcFunction=CompanyFocus&uClient=%s&cCustomer=%s"
							"&cZone=%s&uView=%s&uResource=%u\">%s</a><br>\n",
							field[0]
							,gcCustomer
							,gcZone
							,cuView
							,uResource
							,field[1]);
					strcat(cNavList,cTmp);
				}
				strcat(cNavList,"<!-- Search NavList End -->\n");
				sprintf(cTmp,"%u record(s) found.",uRows);
				gcMessage=cTmp;
				htmlReport();
			}
			else
			{
				gcMessage="No records found";
				htmlReport();
			}
		}	
		else if(!strcmp(gcFunction,"Email Report"))
		{
			MYSQL_RES *res;
			MYSQL_ROW field;

			MYSQL_RES *res2;
			MYSQL_ROW field2;
			
			FILE *fpReport;
		
			char cReportEmail[100]={""};
			
			unsigned uTotalHits=0;
			unsigned uHitCount=0;

			GetConfiguration("cReportEmail",cReportEmail,1);

			if(!cReportEmail[0])
			{
				gcMessage="<blink>Error: </blink>You don't have a report email address configured. "
					"Please create a tConfiguration entry for cReportEmail";
				htmlReport();
			}
			if(!cMonthHit[0])
			{
				gcMessage="<blink>Error: </blink>You have to select the month for the hits report";
				htmlReport();
			}
			//load the archived data
			sprintf(gcQuery,"DELETE FROM tHitMonth");
		        mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));

			sprintf(gcQuery,"INSERT tHitMonth (uHit,cZone,uHitCount,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate) "
					"SELECT uHit,cZone,uHitCount,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate FROM %s",
					cMonthHit);
		        mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
			
			sprintf(gcQuery,"SELECT DISTINCT tClient.uClient,tClient.cLabel,tClient.cCode FROM tHitMonth,tZone,tClient "
					"WHERE tClient.uClient=tZone.uOwner AND tHitMonth.cZone=tZone.cZone ORDER BY tClient.cLabel");
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
			res=mysql_store_result(&gMysql);
			fpReport=popen("/usr/lib/sendmail -t","w");
			if(fpReport==NULL)
				htmlPlainTextError(strerror(errno));
			
			fprintf(fpReport,"To: %s\n",cReportEmail);
			fprintf(fpReport,"From: histatistics@unixservice.com\n");
			fprintf(fpReport,"Reply-to: noreply@unixservice.com\n");
			fprintf(fpReport,"Subject: Monthly Hit Report %s\n",cMonthHit);
			fprintf(fpReport,"Content-type: text/plain\n\n");
			//field[0] tClient.uClient
			//1 tClient.cLabel
			//field2[0] tHitMonth.cZone
			//1 tHitMonth.uHitCount
			//2 tZone.uZone
			while((field=mysql_fetch_row(res)))
			{
				if(!uCSVReport)
					fprintf(fpReport,"Company: %s\n",field[1]);
				sprintf(gcQuery,"SELECT tHitMonth.cZone,tHitMonth.uHitCount,tZone.uZone FROM tZone,tHitMonth,tClient "
						"WHERE tZone.cZone=tHitMonth.cZone AND tZone.uOwner=tClient.uClient AND tClient.uClient=%s",
						field[0]);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
				res2=mysql_store_result(&gMysql);
				while((field2=mysql_fetch_row(res2)))
				{
					if(uCSVReport)
						//cLabel,uClient,cZone,uZone,uHitCount			
						fprintf(fpReport,"\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
							field[1]
							,field[0]
							,field2[0]
							,field2[2]
							,field2[1]
							);
					else
						fprintf(fpReport,"\t%s\t\t%s\n",field2[0],field2[1]);
					sscanf(field2[1],"%u",&uHitCount);
					uTotalHits+=uHitCount;
				}

				if(uCSVReport)
					fprintf(fpReport,"\"%s\",\"%s\",\"%s\",\"%s\",\"%u\"\n",
						field[1]
						,field[0]
						,"TOTAL"
						,""
						,uTotalHits
						);
				else
					fprintf(fpReport,"Total hits\t%u\n\n",uTotalHits);
				uTotalHits=0;
				mysql_free_result(res2);
			}
			fprintf(fpReport,".\n");
			pclose(fpReport);
			gcMessage="Report emailed ok";
			htmlReport();
		}
		else if(1)
		{
			htmlReport();
		}
	}

}//void ReportCommands(pentry entries[], int x);


void ReportSearchZone(char *cLabel)
{
	sprintf(gcQuery,"SELECT DISTINCT tClient.uClient,tClient.cLabel FROM tHit,tZone,tClient WHERE "
			"tClient.uClient=tZone.uOwner AND tHit.cZone=tZone.cZone AND tClient.cLabel "
			"LIKE '%s' ORDER BY tClient.cLabel LIMIT 20",cLabel);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

}//void ReportSearchZone(char *cLabel)


void funcReportHitsTop20(FILE *fp)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	unsigned uCount=0;
	char cuCount[16]={""};
	struct t_template template;
	
	fprintf(fp,"<!-- funcReportHitsTop20(fp) Start -->\n");
	
        sprintf(gcQuery,"SELECT uHit,cZone,SUM(uHitCount) AS uTotalHits FROM tHit WHERE cZone!='allzone.stats' "
			"GROUP BY tHit.cZone ORDER BY uTotalHits DESC LIMIT 20");

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
                fprintf(fp,"%s",mysql_error(&gMysql));
                return;
        }
	
	res=mysql_store_result(&gMysql);
        if(mysql_num_rows(res))
        {
		char cuResource[16]={""};

		sprintf(cuResource,"%u",uResource);

		template.cpName[0]="";
		fpTemplate(fp,"ReportHitsTop20.Header",&template);

                while((field=mysql_fetch_row(res)))
		{
			uCount++;
			sprintf(cuCount,"%u",uCount);
			template.cpName[0]="uCount";
			template.cpValue[0]=cuCount;

			template.cpName[1]="uHit";
			template.cpValue[1]=field[0];
			
			template.cpName[2]="cZoneList";
			template.cpValue[2]=field[1];

			template.cpName[3]="uHitCount";
			template.cpValue[3]=field[2];

			template.cpName[4]="cZone";
			template.cpValue[4]=gcZone;
			
			template.cpName[5]="cCustomer";
			template.cpValue[5]=gcCustomer;

			template.cpName[6]="uView";
			template.cpValue[6]=cuView;

			template.cpName[7]="uResource";
			template.cpValue[7]=cuResource;

			template.cpName[8]="";

			fpTemplate(fp,"ReportHitsTop20.Row",&template);
		}

		template.cpName[0]="";
		fpTemplate(fp,"ReportHitsTop20.Footer",&template);
        }
	else
		fprintf(fp,"No tHit data available.");

        mysql_free_result(res);
	
	fprintf(fp,"<!-- funcReportHitsTop20(fp) End -->\n");
	
}//void funcReportHitsTop20(FILE *fp)


void funcReportOverallChanges(FILE *fp)
{
	MYSQL_RES *res;
	
	struct t_template template;
	char cuRRNewCount[16]={"0"};
	char cuRRModCount[16]={"0"};
	char cuRRDelCount[16]={"0"};
	char cuRRNewCountAdmin[16]={"0"};
	char cuRRModCountAdmin[16]={"0"};
	char cuRRDelCountAdmin[16]={"0"};
	char cuRRNewCountBO[16]={"0"};
	char cuRRModCountBO[16]={"0"};
	char cuRRDelCountBO[16]={"0"};
	char cuZoneModCount[16]={"0"};
	char cuZoneModCountBO[16]={"0"};
	char cuZoneModCountAdmin[16]={"0"};
		
	//
	//tLog Mod entries for Zone (uLogType=3: admin interface)
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tZone' AND cLabel='Mod' AND uLogType=3");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	
	res=mysql_store_result(&gMysql);
	
	sprintf(cuZoneModCountAdmin,"%u",(unsigned) mysql_num_rows(res));

	//
	//tLog Mod entries for Zone (uLogType=2: org interface)
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tZone' AND cLabel='Mod' AND uLogType=2");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	
	res=mysql_store_result(&gMysql);
	
	sprintf(cuZoneModCount,"%u",(unsigned) mysql_num_rows(res));
	//
	//tLog Mod entries for Zone (uLogType=1: back-office interface)
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tZone' AND cLabel='Mod' AND uLogType=1");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	
	res=mysql_store_result(&gMysql);
	
	sprintf(cuZoneModCountBO,"%u",(unsigned) mysql_num_rows(res));
//
//idnsOrg statistics
	mysql_free_result(res);
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='New' AND uLogType=2");

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	res=mysql_store_result(&gMysql);
	sprintf(cuRRNewCount,"%u",(unsigned) mysql_num_rows(res));
		
	mysql_free_result(res);
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Mod' AND uLogType=2");

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	res=mysql_store_result(&gMysql);
	sprintf(cuRRModCount,"%u",(unsigned) mysql_num_rows(res));
		
	mysql_free_result(res);
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Del' AND uLogType=2");

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	res=mysql_store_result(&gMysql);
	sprintf(cuRRDelCount,"%u",(unsigned) mysql_num_rows(res));

//
//idnsAdmin statistics
	mysql_free_result(res);
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='New' AND uLogType=3");

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	res=mysql_store_result(&gMysql);
	sprintf(cuRRNewCountAdmin,"%u",(unsigned) mysql_num_rows(res));
			
	mysql_free_result(res);
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Mod' AND uLogType=3");

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	res=mysql_store_result(&gMysql);
	sprintf(cuRRModCountAdmin,"%u",(unsigned) mysql_num_rows(res));
		
	mysql_free_result(res);
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Del' AND uLogType=3");

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	res=mysql_store_result(&gMysql);
	sprintf(cuRRDelCountAdmin,"%u",(unsigned) mysql_num_rows(res));
//
//iDNS back-office statistics
	mysql_free_result(res);
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='New' AND uLogType=1");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	res=mysql_store_result(&gMysql);
	sprintf(cuRRNewCountBO,"%u",(unsigned) mysql_num_rows(res));
			
	mysql_free_result(res);
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Mod' AND uLogType=1");

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	res=mysql_store_result(&gMysql);
	sprintf(cuRRModCountBO,"%u",(unsigned) mysql_num_rows(res));
		
	mysql_free_result(res);
	sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Del' AND uLogType=1");

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	res=mysql_store_result(&gMysql);
	sprintf(cuRRDelCountBO,"%u",(unsigned) mysql_num_rows(res));

	template.cpName[0]="uZoneModCount";
	template.cpValue[0]=cuZoneModCount;
		
	template.cpName[1]="uRRNewCount";
	template.cpValue[1]=cuRRNewCount;
		
	template.cpName[2]="uRRModCount";
	template.cpValue[2]=cuRRModCount;

	template.cpName[3]="uRRDelCount";
	template.cpValue[3]=cuRRDelCount;

	template.cpName[4]="uZoneModCountAdmin";
	template.cpValue[4]=cuZoneModCountAdmin;
		
	template.cpName[5]="uRRNewCountAdmin";
	template.cpValue[5]=cuRRNewCountAdmin;
		
	template.cpName[6]="uRRModCountAdmin";
	template.cpValue[6]=cuRRModCountAdmin;

	template.cpName[7]="uRRDelCountAdmin";
	template.cpValue[7]=cuRRDelCountAdmin;

	template.cpName[8]="uZoneModCountBO";
	template.cpValue[8]=cuZoneModCountBO;
		
	template.cpName[9]="uRRNewCountBO";
	template.cpValue[9]=cuRRNewCountBO;
		
	template.cpName[10]="uRRModCountBO";
	template.cpValue[10]=cuRRModCountBO;

	template.cpName[11]="uRRDelCountBO";
	template.cpValue[11]=cuRRDelCountBO;

	template.cpName[12]="";
	
	fpTemplate(fp,"ReportOverallChanges",&template);
		
}//
	
void funcReportResults(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;
	MYSQL_ROW field2;
	
	struct t_template template;
	char cuRRNewCount[16]={"0"};
	char cuRRModCount[16]={"0"};
	char cuRRDelCount[16]={"0"};
	char cuRRNewCountAdmin[16]={"0"};
	char cuRRModCountAdmin[16]={"0"};
	char cuRRDelCountAdmin[16]={"0"};
	char cuRRNewCountBO[16]={"0"};
	char cuRRModCountBO[16]={"0"};
	char cuRRDelCountBO[16]={"0"};
	char cuZoneModCount[16]={"0"};
	char cuZoneModCountBO[16]={"0"};
	char cuZoneModCountAdmin[16]={"0"};
	char cuZone[16]={""};
	char cuHits[16]={"0"};
	char cZone[100]={""};

	char cLastHitDate[32]={""};
	time_t luModDate=0;
	
	if(!cuHit[0])
		return;

	sprintf(gcQuery,"SELECT cZone FROM tHit WHERE uHit='%s'",cuHit);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sprintf(cZone,"%.99s",field[0]);
	mysql_free_result(res);

	sprintf(gcQuery,"SELECT SUM(uHitCount),GREATEST(uModDate,uCreatedDate) "
			"FROM tHit WHERE cZone='%s' GROUP BY cZone",cZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(fp,"%s",mysql_error(&gMysql));
		return;
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		
		sprintf(cuHits,"%s",field[0]);
		sscanf(field[1],"%lu",&luModDate);
		sprintf(cLastHitDate,"%s",ctime(&luModDate));

		sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",cZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		if(!(field2=mysql_fetch_row(res2)))
		{
			fprintf(fp,"Zone not in tZone, unable to get uZone; can't continue<br>");
			return;
		}
		sprintf(cuZone,"%s",field2[0]);
		mysql_free_result(res2);
		
		//
		//tLog Mod entries for Zone (uLogType=2: organization interface)
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tZone' AND cLabel='Mod' AND uLogType=2 AND uTablePK=%s",cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuZoneModCount,"%u",(unsigned) mysql_num_rows(res2));
		
		//
		//tLog Mod entries for Zone (uLogType=3: admin interface)
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tZone' AND cLabel='Mod' AND uLogType=3 AND uTablePK=%s",cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuZoneModCountAdmin,"%u",(unsigned) mysql_num_rows(res2));

		//
		//tLog Mod entries for Zone (uLogType=1: back-office interface)
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tZone' AND cLabel='Mod' AND uLogType=1 AND uTablePK=%s",cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuZoneModCount,"%u",(unsigned) mysql_num_rows(res2));

//
//idnsOrg statistics
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='New' "
				"AND uLogType=2 AND uTablePK IN (SELECT tResource.uResource FROM tResource WHERE uZone=%s)",
				cuZone);

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuRRNewCount,"%u",(unsigned) mysql_num_rows(res2));
			
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Mod' "
				"AND uLogType=2 AND uTablePK IN (SELECT tResource.uResource FROM tResource WHERE uZone=%s)",
				cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuRRModCount,"%u",(unsigned) mysql_num_rows(res2));
		
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Del' "
				"AND uLogType=2 AND uTablePK IN (SELECT tResource.uResource FROM tResource WHERE uZone=%s)",
				cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuRRDelCount,"%u",(unsigned) mysql_num_rows(res2));

//
//idnsAdmin statistics
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='New' "
				"AND uLogType=3 AND uTablePK IN (SELECT tResource.uResource FROM tResource WHERE uZone=%s)",
				cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuRRNewCountAdmin,"%u",(unsigned) mysql_num_rows(res2));
			
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Mod' "
				"AND uLogType=3 AND uTablePK IN (SELECT tResource.uResource FROM tResource WHERE uZone=%s)",
				cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuRRModCountAdmin,"%u",(unsigned) mysql_num_rows(res2));
		
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Del' "
				"AND uLogType=3 AND uTablePK IN (SELECT tResource.uResource FROM tResource WHERE uZone=%s)",
				cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuRRDelCountAdmin,"%u",(unsigned) mysql_num_rows(res2));
//
//iDNS back-office statistics
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='New' "
				"AND uLogType=1 AND uTablePK IN (SELECT tResource.uResource FROM tResource WHERE uZone=%s)",
				cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuRRNewCountBO,"%u",(unsigned) mysql_num_rows(res2));
			
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Mod' "
				"AND uLogType=1 AND uTablePK IN (SELECT tResource.uResource FROM tResource WHERE uZone=%s)",
				cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuRRModCountBO,"%u",(unsigned) mysql_num_rows(res2));
		
		sprintf(gcQuery,"SELECT uLog FROM tLog WHERE cTableName='tResource' AND cLabel='Del' "
				"AND uLogType=1 AND uTablePK IN (SELECT tResource.uResource FROM tResource WHERE uZone=%s)",
				cuZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(fp,"%s",mysql_error(&gMysql));
			return;
		}
		res2=mysql_store_result(&gMysql);
		sprintf(cuRRDelCountBO,"%u",(unsigned) mysql_num_rows(res2));
		mysql_free_result(res2);//Only free last result others above use same heap space

		//Prep template contents
		template.cpName[0]="uZoneModCount";
		template.cpValue[0]=cuZoneModCount;
		
		template.cpName[1]="uRRNewCount";
		template.cpValue[1]=cuRRNewCount;
		
		template.cpName[2]="uRRModCount";
		template.cpValue[2]=cuRRModCount;

		template.cpName[3]="uRRDelCount";
		template.cpValue[3]=cuRRDelCount;

		template.cpName[4]="uZoneModCountAdmin";
		template.cpValue[4]=cuZoneModCountAdmin;
		
		template.cpName[5]="uRRNewCountAdmin";
		template.cpValue[5]=cuRRNewCountAdmin;
		
		template.cpName[6]="uRRModCountAdmin";
		template.cpValue[6]=cuRRModCountAdmin;

		template.cpName[7]="uRRDelCountAdmin";
		template.cpValue[7]=cuRRDelCountAdmin;

		template.cpName[8]="uZoneModCountBO";
		template.cpValue[8]=cuZoneModCountBO;
		
		template.cpName[9]="uRRNewCountBO";
		template.cpValue[9]=cuRRNewCountBO;
		
		template.cpName[10]="uRRModCountBO";
		template.cpValue[10]=cuRRModCountBO;

		template.cpName[11]="uRRDelCountBO";
		template.cpValue[11]=cuRRDelCountBO;

		template.cpName[12]="uHits";
		template.cpValue[12]=cuHits;

		template.cpName[13]="cLastHitDate";
		template.cpValue[13]=cLastHitDate;

		template.cpName[14]="";
	
		fpTemplate(fp,"ReportRightPanel",&template);
	
		//Do not need results anymore free it	
		mysql_free_result(res);

	}//if((field=mysql_fetch_row(res)))

			
}//void funcReportResults(FILE *fp)


char *cGetcZone(char *cuHit)
{
	static char cZone[64];
	MYSQL_RES *res;
	MYSQL_ROW field;
	
	sprintf(gcQuery,"SELECT cZone FROM tHit WHERE uHit=%s",cuHit);
	
	mysql_query(&gMysql,gcQuery);
	
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);
	
	if((field=mysql_fetch_row(res)))
		sprintf(cZone,"%s",field[0]);

	return(cZone);

}//char *cGetcZone(char *cuHit)


char *cGetClientName(char *cuClient)
{
	static char cLabel[100];
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT cLabel FROM tClient WHERE uClient=%s",cuClient);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);

	if((field=mysql_fetch_row(res)))
		sprintf(cLabel,"%s",field[0]);

	return(cLabel);
	
}//char *cGetClientName(char *cuClient)

				
void funcAvailableZones(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	if(!strcmp(cuClient,"0"))
		return;

	fprintf(fp,"<!-- funcAvailableZones() start -->\n");

	sprintf(gcQuery,"SELECT tHit.uHit,tHit.cZone,SUM(tHit.uHitCount) AS uTotalHits FROM "
			"tZone,tHit,tClient WHERE tZone.cZone=tHit.cZone AND "
			"tZone.uOwner=tClient.uClient AND tClient.uClient=%s GROUP BY cZone "
			"ORDER BY uTotalHits DESC",cuClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		 htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if(!mysql_num_rows(res))
	{
		fprintf(fp,"No tHit data available for customer\n");
		return;
	}

	fprintf(fp,"<table>\n");
	while((field=mysql_fetch_row(res)))
		fprintf(fp,"<tr><td><a href=\"idnsAdmin.cgi?gcPage=Report&gcFunction=ZoneFocus"
			"&uHit=%s&cCustomer=%s&cZone=%s&uView=%s&uResource=%u\">%s</a></td><td align=right>%s</td></tr>\n",
				field[0]
				,gcCustomer
				,gcZone
				,cuView
				,uResource
				,field[1]
				,field[2]);
	mysql_free_result(res);

	sprintf(gcQuery,"SELECT SUM(tHit.uHitCount) FROM tZone,tHit,tClient WHERE "
			"tZone.cZone=tHit.cZone AND tZone.uOwner=tClient.uClient AND tClient.uClient=%s",cuClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		fprintf(fp,"<tr><td>Total hits for company:</td><td align=right>%s</td></tr>",field[0]);
	
	fprintf(fp,"</table>\n");

	fprintf(fp,"<!-- funcAvailableZones() end -->\n");
	
}//void funcAvailableZones(FILE *fp)


void funcSelectHitMonth(FILE *fp)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	fprintf(fp,"<!-- funcSelectHitMonth() start -->\n");

	sprintf(gcQuery,"SELECT cLabel FROM tMonthHit ORDER BY cLabel");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		 htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	fprintf(fp,"<select name=cMonthHit class=type_textarea>\n");
	
	if(!mysql_num_rows(res))
	{
		 fprintf(fp,"<option value=0>No archived hit data</option>\n");
		 fprintf(fp,"</select>\n");
		 fprintf(fp,"<!-- funcSelectHitMonth() end  -->\n");
		 return;
	}
	fprintf(fp,"<option>---</option>\n");
	
	while((field=mysql_fetch_row(res)))
		fprintf(fp,"<option>%s</option>\n",field[0]);
	fprintf(fp,"</select>\n");

	fprintf(fp,"<!-- funcSelectHitMonth() end  -->\n");
	
}//void funcSelectHitMonth(FILE *fp)

