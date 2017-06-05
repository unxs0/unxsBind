/*
FILE
	svn ID removed
PURPOSE
	Non-schema dependent tzone.c expansion.
AUTHOR/LEGAL
	(C) 2001-2010 Gary Wallis for Unixservice, LLC.
	GPLv2 License applies. See included LICENSE file.
TODO
	See TODOs in this file.
	Move them and all this to CHANGES.

	Problems:
		1-. Can't allow modifying an existing zone's cZone. Must delete it first
		to create a new one.

		2-. RevDNS checkbox does not work for "first time" add of first ns. So I turned
		it off for ALL New ops. This is dumb but better than broken zones.
		(support @ openisp . net)
	Solution:
		Must check cZone against currently saved cZone. If changed
		must notify user and not allow mod.
	
*/

#include <openisp/ucidr.h>

//from tresource.c
#define VAR_LIST_tResource "tResource.uResource,tResource.uZone,tResource.cName,tResource.uTTL,tResource.uRRType,tResource.cParam1,tResource.cParam2,tResource.cComment,tResource.uOwner,tResource.uCreatedBy,tResource.uCreatedDate,tResource.uModBy,tResource.uModDate"

static char *cMassList={""};
static char *cMessage={"Cut and paste your list into cMassList. Select an operation to perform from \"Mass/Bulk OPs Panel.\""};
static char cSearch[64]={""};
static char cExtZone[256]={""};
static char cIPBlock[100]={""};
static unsigned uDelegationTTL=0;
static char *cNSList={""};
static unsigned uSubmitJob=0;
static unsigned uOverwriteRRs=0;
static char cExcludePattern[64]={""};

static char cTargetZone[100]={""};

//static char cuNSSetPullDown[256]={""};
static unsigned uSelectNSSet=0;

//Aux checkboxes
static unsigned uSearchSecOnly=0;

//Aux drop/pull downs
static char cCustomerDropDown[256]={""};
static unsigned uDDClient=0;

//Local
void ResourceRecordList(unsigned uZone);
void TableAddRR(void);
void UpdateSerialNum(unsigned uZone);
void tResourceTableAddRR(unsigned uZone);
int AddNewArpaZone(const char *cArpaZone, unsigned uExtNSSet, char *cExtHostmaster, unsigned uExtOwner);
int IllegalZoneDataChange(void);
#ifndef DEBUG_REPORT_STATS_OFF
	int UpdateInfo();
#endif
void MassOperations(void);
char *ParseTextAreaLines(char *cTextArea);
void htmlInZone(void);
void htmlSecondaryServiceOnly(void);
void htmlSecondaryServiceCleanup(void);
void htmlMassUpdate(void);
void htmlMassDelete(void);
void htmlMassResourceFix(void);
void htmlCustomerZones(void);
void htmlZoneList(void);
void CustomerDropDown(unsigned uSelector);
void htmlMassResourceImport(void);
void htmlZonesFromBlocks(void);
void htmlMassCopyToOtherViews(void);
void htmlMassUpdate(void);
void htmlMassPTRCheck(void);
void htmlMassCheckZone(void);
void tZoneNavList(void);	
void tZoneContextInfo(void);
void DelegationTools(void);
unsigned uPTRInCIDR(unsigned uZone,char *cIPBlock);
unsigned uPTRInBlock(unsigned uZone,unsigned uStart,unsigned uEnd);
void htmlCheckSOA(void);
void htmlMasterZoneFile(void);
void htmlExternalNSRecords(void);
void htmlMasterZonesCheck(void);
void htmlMasterNamedCheckZone(void);
void tNSSetMembers(unsigned uNSSet);//tnssetfunc.h
void CloneZone(char *cSourceZone,char *cTargetZone,unsigned uView);
unsigned uGetZoneSearchGroup(const char *gcUser);
char *cNSFromWhois(char const *cZone,char const *cuNSSet);

//bind.c
void ProcessRRLine(const char *cLine,char *cZoneName,const unsigned uZone,const unsigned uCustId,
			const unsigned uNSSet,const unsigned uCreatedBy,const char *cComment);
#include "local.h"
void SerialNum(char *serial);
void GenerateArpaZones(void);
void GetGlossary(const char *cName, char *cValue);
int FetchNSSet(char *cList, char *cNSSet);

//bind.c
void CreateMasterFiles(char *cMasterNS, char *cZone, unsigned uModDBFiles,
		                unsigned uModStubs, unsigned uDebug);
void PassDirectHtml(char *file);//bind.c aux section
void PassDirectHtmlLineNum(char *file);

void PrepareTestData(unsigned uResource,char *cName,char *cParam1,char *cParam2,char *cParam3,
			char *cParam4,char *cRRType,char *cComment,unsigned uRRTTL,unsigned uCalledFrom);
void PrepDelToolsTestData(unsigned uNumIPs);

void ExtProcesstZoneVars(pentry entries[], int x)
{
	register int i;
	for(i=0;i<x;i++)
	{
		//printf("%s\n",entries[i].name);
		
		if(!strcmp(entries[i].name,"cSearch"))
			sprintf(cSearch,"%.63s",entries[i].val);
		else if(!strcmp(entries[i].name,"uSearchSecOnly"))
			uSearchSecOnly=1;
		else if(!strcmp(entries[i].name,"cCustomerDropDown"))
		{
			strcpy(cCustomerDropDown,entries[i].val);
			uDDClient=ReadPullDown(TCLIENT,"cLabel",
					cCustomerDropDown);
		}
		else if(!strcmp(entries[i].name,"cMassList"))
			cMassList=entries[i].val;
		else if(!strcmp(entries[i].name,"cIPBlock"))
			sprintf(cIPBlock,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"uDelegationTTL"))
			sscanf(entries[i].val,"%u",&uDelegationTTL);
		else if(!strcmp(entries[i].name,"cNSList"))
			cNSList=entries[i].val;
		else if(!strcmp(entries[i].name,"uSubmitJobNoCA"))
			uSubmitJob=1;			
		else if(!strcmp(entries[i].name,"uOverwriteRRsNoCA"))
			uOverwriteRRs=1;			
		if(!strcmp(entries[i].name,"cExcludePattern"))
			sprintf(cExcludePattern,"%.63s",entries[i].val);
		else if(!strcmp(entries[i].name,"cTargetZone"))
			sprintf(cTargetZone,"%.99s",FQDomainName(entries[i].val));
		else if(!strcmp(entries[i].name,"cuNSSetPullDown"))
		{
			sprintf(cuNSSetPullDown,"%.255s",entries[i].val);
			uSelectNSSet=ReadPullDown("tNSSet","cLabel",cuNSSetPullDown);
		}
	}

}//void ExtProcesstZoneVars(pentry entries[], int x)


void ZoneCheck(unsigned uMode)
{
	
	//cZone
	//a.tv	
	if(strlen(cZone)<4)
	{
		guMode=uMode;
		tZone("FQDN cZone required. Must specify TLD.");
	}
	if(!strchr(cZone,'.'))
	{
		guMode=uMode;
		tZone("Must use a FQDN for cZone. Must specify TLD.");
	}
	if(cZone[strlen(cZone)-1]=='.')
	{
		guMode=uMode;
		tZone("cZone should not end with a period. Must specify TLD.");
	}
	
	//uNSSet
	if(!uNSSet)
	{
		guMode=uMode;
		tZone("Must select a name server set.");
	}
	
	//cHostmaster
	//a.a.tv
	if(strlen(cHostmaster)<6)
	{
		guMode=uMode;
		tZone("FQDN cHostmaster required. Must specify 'email.' in front.");
	}
	if(!strchr(cHostmaster,'.'))
	{
		guMode=uMode;
		tZone("Must use a FQDN for cHostmaster with 'email.' in front");
	}
	if(strchr(cHostmaster,'@'))
	{
		guMode=uMode;
		tZone("Must use . for @ first symbol in hostmaster email address.");
	}
	if(cHostmaster[strlen(cHostmaster)-1]=='.')
	{
		guMode=uMode;
		tZone("email.FQDN cHostmaster should not end with a period.");
	}
	
	if(uTTL>1000000)
	{
		guMode=uMode;
		tZone("uTTL out of range.");
	}
	//Up to 4 weeks, RFC1912
	if(!uExpire || uExpire>2419200)
	{
		guMode=uMode;
		tZone("uExpire out of range.");
	}
	if(!uRefresh || uRefresh>1000000)
	{
		guMode=uMode;
		tZone("uRefresh out of range.");
	}
	if(!uRetry || uRetry>1000000)
	{
		guMode=uMode;
		tZone("uRetry out of range.");
	}
	if(!uZoneTTL || uZoneTTL >1000000)
	{
		guMode=uMode;
		tZone("uZoneTTL out of range.");
	}

	//Sanity checks from BIND source db_load.c
	//WARNING SOA expire value is less than SOA refresh+retry
	if(  uExpire < (uRefresh + uRetry) )
	{
		guMode=uMode;
		tZone("SOA expire value is less than SOA refresh+retry.");
	}
	//WARNING SOA expire value is less than refresh + 10 * retry 
	if(  uExpire < (uRefresh + (10 * uRetry)) )
	{
		guMode=uMode;
		tZone("SOA expire value is less than refresh + 10 * retry.");
	}
	//WARNING SOA expire value is less than 7 days
	if(  uExpire < (7 * 24 * 3600) )
	{
		guMode=uMode;
		tZone("SOA expire value is less than 7 days");
	}
	//WARNING SOA expire value is greater than 6 months
	if(  uExpire > ( 183 * 24 * 3600) )
	{
		guMode=uMode;
		tZone("SOA expire value is greater than 6 months");
	}
	//WARNING SOA refresh value is less than 2 * retry
	if(  uRefresh < (uRetry * 2))
	{
		guMode=uMode;
		tZone("SOA refresh value is less than 2 * retry");
	}
	
}//void ZoneCheck(unsigned uMode)


void ExttZoneCommands(pentry entries[], int x)
{
	char cSerial[16]={""};

	if(!strcmp(gcFunction,"tZoneTools"))
	{
		if(!strcmp(gcFind,LANG_BL_TableAddRR))
		{
                        ProcesstZoneVars(entries,x);
			if(uOwner) GetClientOwner(uOwner,&guReseller);
			if( (guPermLevel>=7 && uOwner==guLoginClient)
				|| (guPermLevel>9 && uOwner!=1)
				|| (guPermLevel>7 && guReseller==guLoginClient) )
			{
				TableAddRR();
			}
		}
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstZoneVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
	                        tZone(LANG_NB_CONFIRMMOD);
			}
			else
				tZone("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstZoneVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				long unsigned luYearMonDay=0;
				char cSerial[12]={""};
				char *cJobType="Modify";


				if(IllegalZoneDataChange())
				{
					guMode=2000;
					tZone("Can't change cZone. Delete and add corrected name if needed.");
				}
				ZoneCheck(2002);
				uModBy=guLoginClient;
				if(!uView) uView=1;//This should be the internal 
						//or one of mutually exclusive restrictive views
						//The uView with the largest uOrder should be the 
						//external or least restrictive view.

				SerialNum(cSerial);
				sscanf(cSerial,"%lu",&luYearMonDay);

				//debug only
				//sprintf(gcQuery,"luYearMonDay=%lu",luYearMonDay);
				//tZone(gcQuery);

				//Typical year month day and 99 changes per day max
				//to stay in correct date format. Will still increment.
				if(uSerial<luYearMonDay)
				{
					sscanf(cSerial,"%u",&uSerial);
				}
				else
				{
					uSerial++;
				}

				//If uNSSet was changed the job we have to submit is different
				if(uZone && uNSSet)
				{
					unsigned uPrevNSSet=0;

					sscanf(ForeignKey("tZone","uNSSet",uZone),"%u",
						&uPrevNSSet);	
					if(uPrevNSSet && uNSSet!=uPrevNSSet)
						cJobType="Modify New";
				}

				//Optionally allow owner modification.
				if(uDDClient)
					uOwner=uDDClient;

				if(SubmitJob(cJobType,uNSSet,cZone,0,0))
					htmlPlainTextError(mysql_error(&gMysql));
	                        ModtZone();
			}
			else
				tZone("<blink>Error</blink>: Denied by permissions settings");
                }
		//Default wizard like two step creation and deletion
		else if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			if(guPermLevel>=7)
			{
				ProcesstZoneVars(entries,x);
				strcpy(cMainAddress,"0.0.0.0");
				//Check global conditions for new record here
				guMode=2000;
				tZone(LANG_NB_CONFIRMNEW);
			}
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			if(guPermLevel>=7)
			{
				MYSQL_RES *res;

	                        ProcesstZoneVars(entries,x);
			
				if(guPermLevel<10 && 
						!strcmp(cZone+strlen(cZone)-5,".arpa"))
				tZone("Only admin level users can add arpa zones directly.");

				if(!uView) uView=1;

				//Make sure cZone not already taken
				sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=%u",cZone,uView);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
				res=mysql_store_result(&gMysql);
				if(mysql_num_rows(res))
				{
					guMode=2000;
					mysql_free_result(res);
					tZone("<blink>Error: </blink>cZone/uView already used!");
				}
                                mysql_free_result(res);



				ZoneCheck(2000);
				uZone=0;
				uCreatedBy=guLoginClient;
				if(strcmp(cZone+strlen(cZone)-5,".arpa"))
					uOwner=guCompany;
				else
					uOwner=0;//Public zone
				uModBy=0;//Never modified
				uModDate=0;//Never modified
				SerialNum(cSerial);
				sscanf(cSerial,"%u",&uSerial);

				//TODO figure out...
				//This should be called from inside NewtZone()
				//FIX with ExtNewtZone() success hook or
				//not using NewtZone at all ?

				//Read only cMasterIPs must come from selected
				//uNSSet FK NS set
				sprintf(cMasterIPs,"%.255s",
					ForeignKey("tNSSet","cMasterIPs",uNSSet));

				//For select drop down
				if(!uDDClient)
					uOwner=guCompany;
				else
					uOwner=uDDClient;

				if(uSubmitJob)
				{
					if(SubmitJob("New",uNSSet,cZone,0,0))
						htmlPlainTextError(mysql_error(&gMysql));
				}
#ifndef DEBUG_REPORT_STATS_OFF
				UpdateInfo();
#endif
				//debug only
				//sprintf(gcQuery,"%u",uDDClient);
				//tZone(gcQuery);

				NewtZone(0);
			}
			else
				tZone("<blink>Error</blink>: Denied by permissions settings");
				
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstZoneVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
                        	guMode=2001;
                        	tZone(LANG_NB_CONFIRMDEL);
			}
			else
				tZone("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstZoneVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
//This neeeds to be reviewed and moved TODO
#define PRIORITY_LOGERROR 2000
				guMode=5;
				//
				//Copy the zone to tDeletedZone before deleting it.
				MYSQL_RES *res;
				MYSQL_ROW field;
				
				sprintf(gcQuery,"INSERT INTO tDeletedZone SET uDeletedZone=%u,cZone='%s',"
						"uNSSet=%u,cHostmaster='%s',uSerial=%u,uExpire=%u,"
						"uRefresh=%u,uTTL=%u,uRetry=%u,uZoneTTL=%u,uMailServers=%u,"
						"uView=%u,cMainAddress='%s',uRegistrar=%u,uSecondaryOnly=%u,"
						"cOptions='%s',uOwner=%u,uClient=%u,"
						"uCreatedDate=UNIX_TIMESTAMP(NOW()),uCreatedBy=1",
						uZone,
						cZone,
						uNSSet,
						cHostmaster,
						uSerial,
						uExpire,
						uRefresh,
						uTTL,
						uRetry,
						uZoneTTL,
						uMailServers,
						uView,
						cMainAddress,
						uRegistrar,
						uSecondaryOnly,
						cOptions,
						uOwner,
						uClient);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					printf("Content-type: text/html\n\n%s\n",mysql_error(&gMysql));
				//
				//Copy the tResource records to tDeletedResource
				
				sprintf(gcQuery,"SELECT uResource,uZone,cName,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4,"
						"cComment,uOwner FROM tResource WHERE uZone=%u",uZone);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					printf("Content-type: text/html\n\n%s\n",mysql_error(&gMysql));
				
					//tZone(mysql_error(&gMysql));
				
				res=mysql_store_result(&gMysql);
				while((field=mysql_fetch_row(res)))
				{
					sprintf(gcQuery,"INSERT INTO tDeletedResource SET uDeletedResource='%s',uZone='%s',"
							"cName='%s',uTTL='%s',uRRType='%s',cParam1='%s',cParam2='%s',"
							"cParam3='%s',cParam4='%s',cComment='%s',uOwner='%s',"
							"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							field[0],field[1],field[2],field[3],field[4],field[5],
							field[6],field[7],field[8],field[9],field[10]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
				}
				mysql_free_result(res);
				//Try to delete zone
				sprintf(gcQuery,"DELETE FROM tZone WHERE uZone=%u",uZone);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
				if(mysql_affected_rows(&gMysql)>0)
				{
					//If zone deleted delete RR's. Log error only
					sprintf(gcQuery,
					"DELETE FROM tResource WHERE uZone=%u",uZone);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql)) 
						SubmitJob("DeleteRRError",
							uNSSet,
							mysql_error(&gMysql),
							PRIORITY_LOGERROR,0);


					//Schedule job: Queue handler should 
					//replace any mods
					if(SubmitJob("Delete",uNSSet,cZone,0,0))
						htmlPlainTextError(mysql_error(&gMysql));
					tZone(LANG_NBR_RECDELETED);
				}
				else
				{
					tZone(LANG_NBR_RECNOTDELETED);
				}

			}//guPermLevel block
			else
				tZone("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcFind,"Remove Generated PTRs")
				&& guPermLevel>9)
		{
			guMode=2002;
			sprintf(gcQuery,
				"DELETE FROM tResource WHERE cComment='GenerateArpaZones()'");
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
			tZone("GenerateArpaZones() PTR records removed");

		}
		else if(!strcmp(gcFind,"Generate Arpa Zones")
				&& guPermLevel>9)
		{
			guMode=2002;
			ProcesstZoneVars(entries,x);
			ZoneCheck(2002);
			GenerateArpaZones();
		}
		else if(!strcmp(gcCommand,"RR Search Set OPs"))
                {
			if(guPermLevel>=9)
			{
                        	guMode=12001;
	                        tResource("Search Set Operations");
			}
			else
			{
				tZone("<blink>Error:</blink> Denied by permissions settings");
			}
		}
		else if(!strcmp(gcCommand,"Zone Search Set OPs"))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstZoneVars(entries,x);
                        	guMode=12001;
	                        tZone("Search Set Operations");
			}
			else
			{
				tZone("<blink>Error:</blink> Denied by permissions settings");
			}
                }
		else if(!strcmp(gcCommand,"Reload Search Set"))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstZoneVars(entries,x);
                        	guMode=12002;
	                        tZone("Search set reloaded");
			}
			else
			{
				tZone("<blink>Error:</blink> Denied by permissions settings");
			}
		}
                else if(!strcmp(gcCommand,"Fold A Records") || !strcmp(gcCommand,"Delete Checked")
                	|| (!strcmp(gcCommand,"Backup")) || (!strcmp(gcCommand,"Restore")) || (!strcmp(gcCommand,"Whois")))
                {
			ProcesstZoneVars(entries,x);
                        guMode=12002;
			tZone(gcCommand);
		}
		else if(!strcmp(gcCommand,"Remove from Search Set"))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstZoneVars(entries,x);
                        	guMode=12002;
				char cQuerySection[256];
				unsigned uLink=0;
				unsigned uGroup=0;

				if(cZoneSearch[0]==0 &&uSelectNSSet==0 &&uView==0)
	                        	tZone("You must specify at least one search parameter");

				if((uGroup=uGetZoneSearchGroup(gcUser))==0)
		                        tZone("No search set exists. Please create one first.");

				//Initial query section
				sprintf(gcQuery,"DELETE FROM tGroupGlue WHERE uGroup=%u AND uZone IN"
						" (SELECT uZone FROM tZone WHERE",uGroup);

				//Build AND query section

				if(cZoneSearch[0])
				{
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE cZone LIKE '%s%%')",cZoneSearch);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}
				else
				{
					uLink=0;
				}

				if(uSelectNSSet)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE uNSSet=%u)",uSelectNSSet);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(uView)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE uView=%u)",uView);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				strcat(gcQuery,")");
				//debug only
	                        //tZone(gcQuery);

				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
				unsigned uNumber=0;
				if((uNumber=mysql_affected_rows(&gMysql))>0)
				{
	                        	sprintf(gcQuery,"%u zone records were removed from your search set",uNumber);
	                        	tZone(gcQuery);
				}
				else
				{
	                        	tZone("No records were removed from your search set");
				}
			}
			else
			{
				tZone("<blink>Error:</blink> Denied by permissions settings");
			}
		}
		else if(!strcmp(gcCommand,"Add to Search Set") || !strcmp(gcCommand,"Create Search Set"))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstZoneVars(entries,x);
                        	guMode=12002;
				char cQuerySection[256];
				unsigned uLink=0;
				unsigned uGroup=0;

				if(cZoneSearch[0]==0 &&uSelectNSSet==0 &&uView==0)
	                        	tZone("You must specify at least one search parameter");

				if((uGroup=uGetZoneSearchGroup(gcUser))==0)
				{
					sprintf(gcQuery,"INSERT INTO tGroup SET cLabel='%sZone',uGroupType=2"
						",uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							gcUser,guCompany,guLoginClient);//2=search set type TODO
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
							htmlPlainTextError(mysql_error(&gMysql));
					if((uGroup=mysql_insert_id(&gMysql))==0)
		                        	tZone("An error ocurred when attempting to create your search set");
				}
				else
				{
					if(!strcmp(gcCommand,"Create Search Set"))
					{
						sprintf(gcQuery,"DELETE FROM tGroupGlue WHERE uGroup=%u",uGroup);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
							htmlPlainTextError(mysql_error(&gMysql));
					}
                		}

				//Initial query section
				sprintf(gcQuery,"INSERT INTO tGroupGlue (uGroupGlue,uGroup,uZone,uResource)"
						" SELECT 0,%u,uZone,0 FROM tZone WHERE",uGroup);

				//Build AND query section

				if(cZoneSearch[0])
				{
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE cZone LIKE '%s%%')",cZoneSearch);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}
				else
				{
					uLink=0;
				}

				if(uSelectNSSet)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE uNSSet=%u)",uSelectNSSet);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(uView)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE uView=%u)",uView);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}


				//debug only
	                        //tZone(gcQuery);

				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
				unsigned uNumber=0;
				if((uNumber=mysql_affected_rows(&gMysql))>0)
				{
	                        	sprintf(gcQuery,"%u zone records were added to your search set",uNumber);
	                        	tZone(gcQuery);
				}
				else
				{
	                        	tZone("No records were added to your search set. Filter returned 0 records");
				}
			}
			else
			{
				tZone("<blink>Error:</blink> Denied by permissions settings");
			}
		}
		else if(!strcmp(gcCommand,"Mass Operations")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			MassOperations();
		}
		else if(!strcmp(gcCommand,"Zones From Blocks")
				&& guPermLevel>9)
		{

			ProcesstZoneVars(entries,x);
			htmlZonesFromBlocks();
		}
		else if(!strcmp(gcCommand,"Customer Zones")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!uDDClient)
			{
				cMessage="<blink>Error.</blink> You must select a customer!.";
				MassOperations();
			}
			htmlCustomerZones();
		}
		else if(!strcmp(gcFind,"Zone List")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!cMassList[0])
			{
				cMessage="<blink>cMassList empty.</blink> Help below.";
				MassOperations();
			}
			htmlZoneList();
		}
		else if(!strcmp(gcFind,"Copy to Views")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!cMassList[0])
			{
				cMessage="<blink>cMassList empty.</blink> Help below.";
				MassOperations();
			}
			htmlMassCopyToOtherViews();
		}
		else if(!strcmp(gcFind,"In tZone?")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!cMassList[0])
			{
				cMessage="<blink>cMassList empty.</blink> Help below.";
				MassOperations();
			}
			htmlInZone();
		}
		else if(!strcmp(gcFind,"Secondary Service Cleanup")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!cMassList[0])
			{
				cMessage="<blink>cMassList empty.</blink> Help below.";
				MassOperations();
			}
			htmlSecondaryServiceCleanup();
		}
		else if(!strcmp(gcFind,"Secondary Service Only")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!cMassList[0])
			{
				cMessage="<blink>cMassList empty.</blink> Help below.";
				MassOperations();
			}
			htmlSecondaryServiceOnly();
		}
		else if(!strcmp(gcFind,"Mass Update")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!cMassList[0])
			{
				cMessage="<blink>cMassList empty.</blink> Add one tZone.cZone per line. Help below:";
				MassOperations();
			}
			htmlMassUpdate();
		}
		else if(!strcmp(gcFind,"Mass Delete")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!cMassList[0])
			{
				cMessage="<blink>cMassList empty.</blink> Add one tZone.cZone per line. Help below.";
				MassOperations();
			}
			htmlMassDelete();
		}
		else if(!strcmp(gcFind,"Mass PTR Check")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!cMassList[0])
			{
				cMessage="<blink>cMassList empty.</blink> Help below.";
				MassOperations();
			}
			htmlMassPTRCheck();
		}
		else if(!strcmp(gcFind,"Mass named-checkzone")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!cMassList[0])
			{
				cMessage="<blink>cMassList empty.</blink> Help below.";
				MassOperations();
			}
			htmlMassCheckZone();
		}
		else if(!strcmp(gcFind,"Mass Resource Import")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!cMassList[0])
			{
				cMessage="<blink>cMassList empty.</blink> Help below.";
				MassOperations();
			}
			htmlMassResourceImport();
		}
		else if(!strcmp(gcFind,"Mass Resource Fix")
				&& guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!cMassList[0])
			{
				cMessage="<blink>cMassList empty.</blink> Help below.";
				MassOperations();
			}
			MassOperations();
			//htmlMassResourceFix();
		}
		else if(!strcmp(gcCommand,"Delegation Tools") && guPermLevel>=10)
		{
			ProcesstZoneVars(entries,x);
			guMode=4000;
		}
		else if(!strcmp(gcCommand,"Delegate IP Block"))
		{
			ProcesstZoneVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
				guMode=4001;
			else
				tZone("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,"Confirm Delegation") && guPermLevel>=10)
		{
			char cNS[100]={""};
			char cName[100]={""};
			char cParam1[100]={""};
			char cLogEntry[100]={""};

			ProcesstZoneVars(entries,x);
			if(!uAllowMod(uOwner,uCreatedBy)) 
				tZone("<blink>Error</blink>: Denied by permissions settings");

			if(!cIPBlock[0])
			{
				guMode=4001;
				tZone("<blink>cIPBlock is a required value</blink>");
			}

			if(!cNSList[0])
			{
				guMode=4001;
				tZone("<blink>cNSList is a required value</blink>");
			}
				
			//remove extra spaces or any other junk in CIDR
			sscanf(cIPBlock,"%s",gcQuery);
			sprintf(cIPBlock,"%.99s",gcQuery);
			
			char *cpZone;
			char cIPv6ZonePart[100]={""};
			sprintf(cIPv6ZonePart,"%.99s",cZone);
			if((cpZone=strstr(cIPv6ZonePart,".ip6.arpa")))
			{
				*cpZone=0;
				char *cp;
				if((cp=strchr(cIPBlock,'/')))
				{
					char cIPv6FromBlock[64]={""};
					unsigned h1=0,h2=0,h3=0,h4=0,h5=0,h6=0,h7=0,h8=0,h9=0,h10=0,h11=0,h12=0,h13=0;
					//unsigned h14=0,h15=0,h16=0,uCIDR=0;
					unsigned uCIDR=0;
					if(!uInCIDR6Format32(cIPBlock,&h1,&h2,&h3,&h4,&h5,&h6,&h7,&h8,&uCIDR))
					{
						guMode=4001;
						tZone("<blink>Error:</blink> The entered ip6 block is not in a valid ip6 format.");
					}
					sprintf(cIPv6FromBlock,"%x:%x:%x:%x:%x:%x:%x:%x",h1,h2,h3,h4,h5,h6,h7,h8);
					//guMode=4001;
					//tZone(cIPv6FromBlock);

					//CIDR only checks and calculations
					///48, /52, /56, /60 or /64
					if(uCIDR!=48 && uCIDR!=52 && uCIDR!=56 && uCIDR!=60 && uCIDR!=64)
					{
						guMode=4001;
						tZone("<blink>ip6 CIDR nibble blocks /48 - /64 are supported only</blink>");
					}
				
					unsigned uDigitCount=sscanf(cIPv6ZonePart,"%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x",
									&h1,&h2,&h3,&h4,&h5,&h6,&h7,&h8,&h9,&h10,&h11,&h12,&h13);
					if(uDigitCount!=13)
					{
						guMode=4001;
						sprintf(cIPv6ZonePart,"%u",uDigitCount);
						tZone(cIPv6ZonePart);
						tZone("<blink>Error:</blink> This zone is not suited for automated delegation at this time.");
					}
					char cIPv6BlockFromZone[64]={""};
					//sprintf(cIPv6BlockFromZone,"%x%x%x%x:%x%x%x%x:%x%x%x%x:%x::/48",h13,h12,h11,h10,h9,h8,h7,h6,h5,h4,h3,h2,h1);
					sprintf(cIPv6BlockFromZone,"%x%x%x%x:%x%x%x%x:%x%x%x%x::/48",h13,h12,h11,h10,h9,h8,h7,h6,h5,h4,h3,h2);
					//guMode=4001;
					//tZone(cIPv6BlockFromZone);
					if(!uIpv6InCIDR632(cIPv6FromBlock,cIPv6BlockFromZone))
					{
						guMode=4001;
						tZone("<blink>Error:</blink> The entered ip6 block is not inside the loaded zone.");
					}

					guMode=4001;
					tZone("IPv6 block delegation being installed please wait...");
				}
				else
				{
					guMode=4001;
					tZone("<blink>Error:</blink> The entered block has no /.");
				}
			}
			else
			{
#define IP_BLOCK_CIDR 1
#define IP_BLOCK_DASH 2
			unsigned uA,uB,uC,uD,uE,uNumIPs;
			unsigned uMa,uMb,uMc;
			unsigned uIPBlockFormat;

			sscanf(cZone,"%u.%u.%u.in-addr.arpa",&uMc,&uMb,&uMa);
			
			if(strchr(cIPBlock,'/'))
			{
				//cIPBlock is in CIDR format
				sscanf(cIPBlock,"%u.%u.%u.%u/%u",&uA,&uB,&uC,&uD,&uE);

				if((uA!=uMa) || (uB!=uMb) || (uC != uMc))
				{
					guMode=4001;
					tZone("<blink>Error:</blink> The entered block is not inside the loaded zone.");
				}

				//CIDR only checks and calculations
				if(uE<24)
				{
					guMode=4001;
					tZone("<blink>CIDR not supported</blink>");
				}
				
				uNumIPs=2<<(32-uE-1);
				uNumIPs--;

				if((uD+uNumIPs)>255)
				{
					guMode=4001;
					sprintf(gcQuery,"IP Block range error %u (%u)",uD,(2<<(32-uE-1)));
					tZone(gcQuery);
//					tZone("<blink>CIDR range error</blink>");
				}
				/*
				if(uPTRInCIDR(uZone,cIPBlock))
				{
					guMode=4001;
					tZone("<blink>Delegation overlaps existing PTR records. Can't continue</blink>");
				}
				*/
				uIPBlockFormat=IP_BLOCK_CIDR;
			}
			else if(strchr(cIPBlock,'-'))
			{
				//cIPBlock is in dash format
				sscanf(cIPBlock,"%u.%u.%u.%u-%u",&uA,&uB,&uC,&uD,&uE);

				if((uA!=uMa) || (uB!=uMb) || (uC != uMc))
				{
					guMode=4001;
					tZone("<blink>Error:</blink> The entered range is not inside the loaded zone.");
				}

				if(uE>255)
				{
					guMode=4001;
					tZone("<blink>IP Block incorrect format</blink>");
				}
				
				if(uE<uD)
				{
					guMode=4001;
					tZone("<blink>IP block range error</blink>");
				}
				/*
				if(uPTRInBlock(uZone,uD,uE))
				{
					guMode=4001;
					tZone("<blink>Delegation overlaps existing PTR records. Can't continue</blink>");
				}
				*/
				uNumIPs=uE-uD;
				uIPBlockFormat=IP_BLOCK_DASH;
			}
			
			//basic sanity check (common)
			
			//add check
			if(!uA)
			{
				guMode=4001;
				tZone("<blink>IP Block incorrect format</blink>");
			}
			if((uA>255)||(uB>255)||(uC>255)||(uD>255))
			{
				guMode=4001;
				tZone("<blink>IP Block incorrect format</blink>");
			}
			if(uDelegationTTL>uTTL)
			{
				guMode=4001;
				tZone("<blink>uDelegationTTL out of range</blink>");
			}
			if(!uDelegationTTL)
				uDelegationTTL=uTTL;
			
			//named-checkzone online check
			PrepDelToolsTestData(uNumIPs);
			OnLineZoneCheck(uZone,4001,1);
			fprintf(stderr,"cNSList=%s\n",cNSList);
			while(1)
			{
				sprintf(cNS,"%.99s",ParseTextAreaLines(cNSList));
				if(!cNS[0]) break;
				fprintf(stderr,"cNS=%s",cNS);
				if(uIPBlockFormat==IP_BLOCK_CIDR)
					sprintf(cName,"%u/%u",uD,uE);
				else if(uIPBlockFormat==IP_BLOCK_DASH)
					sprintf(cName,"%u-%u",uD,uE);

				sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%s',uTTL=%u,"
						"uRRType=2,cParam1='%s',cComment='Delegation (%s)',"
						"uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
						uZone
						,cName
						,uDelegationTTL
						,cNS
						,cIPBlock
						,uOwner
						,guLoginClient);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
			}

			//$GENERATE 0-255 $ CNAME $.0/24.21.68.217.in-addr.arpa.
			if(uIPBlockFormat==IP_BLOCK_CIDR)
				sprintf(cParam1,"$.%u/%u.%u.%u.%u.in-addr.arpa.",
						uD
						,uE
						,uC
						,uB
						,uA
				       );
			else if(uIPBlockFormat==IP_BLOCK_DASH)
			{
				sprintf(cParam1,"$.%u-%u.%u.%u.%u.in-addr.arpa.",
						uD
						,uE
						,uC
						,uB
						,uA
				       );
			}
			sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='$GENERATE %u-%u $',"
					"uRRType=5,cParam1='%s',cComment='Delegation (%s)',uOwner=%u,"
					"uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					uZone
					,uD
					,(uD+uNumIPs)
					,cParam1
					,cIPBlock
					,uOwner
					,guLoginClient);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));

			UpdateSerialNum(uZone);	
			if(uSubmitJob)
			{
				if(SubmitJob("Modify",uNSSet,cZone,0,0))
					htmlPlainTextError(mysql_error(&gMysql));
			}
			sprintf(cLogEntry,"%s Delegation",cIPBlock);
			iDNSLog(uZone,"tZone",cLogEntry);

			tZone("IPv4 block delegation done");

			}
			
		}
		else if(!strcmp(gcCommand,"Remove Delegation") && guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			if(!uAllowDel(uOwner,uCreatedBy))
				tZone("<blink>Error</blink>: Denied by permissions settings");
			guMode=5000;
			tZone("");
		}
		else if(!strcmp(gcCommand,"Confirm Del. Removal") && guPermLevel>9)
		{
			char cLogEntry[100]={""};

			ProcesstZoneVars(entries,x);
			if(!uAllowDel(uOwner,uCreatedBy))
				tZone("<blink>Error</blink>: Denied by permissions settings");

			if(!cIPBlock[0])
			{
				guMode=5000;
				tZone("<blink>cIPBlock is a required value</blink>");
			}
			sprintf(gcQuery,"DELETE FROM tResource WHERE uZone=%u AND cComment='Delegation (%s)'",uZone,cIPBlock);
			mysql_query(&gMysql,gcQuery);
			if(!mysql_affected_rows(&gMysql))
				 tZone("<blink>No delegation removed</blink>");
			
			if(mysql_errno(&gMysql))
				 htmlPlainTextError(mysql_error(&gMysql));
			UpdateSerialNum(uZone);
			if(uSubmitJob)
			{
				if(SubmitJob("Modify",uNSSet,cZone,0,0))
					htmlPlainTextError(mysql_error(&gMysql));
			}
			sprintf(cLogEntry,"%s Delegation Removal",cIPBlock);
			iDNSLog(uZone,"tZone",cLogEntry);
			tZone("IP block delegation removed");
		}
		else if(!strcmp(gcCommand,"Clone Zone"))
		{
			ProcesstZoneVars(entries,x);
			if(!cZone[0])
				tZone("<blink>Error:</blink> You must select a source zone first");
			if(!cTargetZone[0])
				tZone("<blink>Error:</blink> You must enter a target zone name for clonning a zone");

			CloneZone(cZone,cTargetZone,uView);
			//Submit new job for cTargetZone
		}
		else if(!strcmp(gcFind,"Check SOA") && guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			htmlCheckSOA();
		}
		else if(!strcmp(gcFind,"Master Zone File") && guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			htmlMasterZoneFile();
		}
		else if(!strcmp(gcFind,"External NS Records") && guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			htmlExternalNSRecords();
		}
		else if(!strcmp(gcFind,"Check master.zones") && guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			htmlMasterZonesCheck();
		}
		else if(!strcmp(gcFind,"Run named-checkzone") && guPermLevel>9)
		{
			ProcesstZoneVars(entries,x);
			htmlMasterNamedCheckZone();
		}
			
	}

}//void ExttZoneCommands(pentry entries[], int x)


void ExttZoneButtons(void)
{
	unsigned uDefault=0;

	OpenFieldSet("tZone Aux Panel",100);
	
	switch(guMode)
        {
                case 12001:
                case 12002:
			printf("<u>Create or refine your user search set</u><br>");
			printf("In the right panel you can select your search criteria. When refining you do not need"
				" to reuse your initial search critieria. Your search set is persistent even across unxsVZ sessions.<p>");
			printf("<input type=submit class=largeButton title='Create an initial or replace an existing search set'"
				" name=gcCommand value='Create Search Set'>\n");
			printf("<input type=submit class=largeButton title='Add the results to your current search set. Do not add the same search"
				" over and over again it will not result in any change but may slow down processing.'"
				" name=gcCommand value='Add to Search Set'>\n");
			printf("<p><input type=submit class=largeButton title='Apply the right panel filter to refine your existing search set"
				" by removing set elements that match the filter settings.'"
				" name=gcCommand value='Remove from Search Set'>\n");
			printf("<p><input type=submit class=largeButton title='Reload current search set. Good for checking for any new status updates'"
				" name=gcCommand value='Reload Search Set'>\n");
			printf("<input type=submit class=largeButton title='Return to main tZone tab page'"
				" name=gcCommand value='Cancel'>\n");
                break;

                case 2000:
			printf("<p><u>Enter required data into form</u><br>");
			if(guPermLevel>9)
			{
				printf("<p><u>Create for customer</u><br>");
				if(uOwner) uDDClient=uOwner;
				CustomerDropDown(uDDClient);
			}
                        printf("<br>");
                        printf(LANG_NBB_CONFIRMNEW);
			printf("<br>uSubmitJob <input title='If not checked, no job will "
				"be created for the new zone ' type=checkbox name=uSubmitJobNoCA checked value=1>");
                break;

                case 2001:
			printf("<p><u>Think twice</u><br>");
                        printf(LANG_NBB_CONFIRMDEL);
                break;

                case 2002:
			printf("<p><u>Review record data</u><br>");
			printf("<p><u>Optionally change owner</u><br>");
			CustomerDropDown(uDDClient);
			printf("<p>");
                        printf(LANG_NBB_CONFIRMMOD);
                break;
		
		case 4000:
			printf("<p><u>Delegation Tools</u></p>\n");
			printf("<p>Enter below the IP block in CIDR format (e.g. 217.23.24.0/24 or for ip6 2001:4a78:500:200::/56) "
				"or in dash format -only for ipv4- (e.g. 217.125.32.17-25) that you wish to delegate. "
				"In the textarea you must place a list (one per line) with the fully qualified domain "
				"name(s) of the nameserver(s) for the delegation. An optional parameter "
				"is uTTL, if not set the default zone TTL will be used. The uSubmitJob "
				"checkbox determines if a modify job is submitted for the zone upon "
				"delegation. If you want to remove a block delegation enter the IP block "
				"in CIDR format or in dash format in the cIPBlock text box and press the "
				"[Remove Delegation] button below. Make sure no existing zone records will"
				"conflict with the delelgation, if any do remove them.</p>\n");
			DelegationTools();
			printf("<br><input class=largeButton title='Delegate the entered IP block in a two step procedure'"
				" type=submit name=gcCommand value='Delegate IP Block'><br>\n");
			printf("<br><input class=largeButton title='Remove the entered IP block delegation in a two step procedure' " 
				"type=submit name=gcCommand value='Remove Delegation'><br>\n");
		break;
			
		case 4001:
			printf("<p><u>Delegation Tools. Confirm delegation</u></p>\n");
			printf("<p>By pressing the button below you confirm the delegation. All the required "
			"NS and CNAME records will be created. You can avoid the modify job submission for "
			"this zone upon delegation by un-checking the uSubmitJob checkbox below.</p>\n");
			if(!uSubmitJob)
				uSubmitJob=1;
			DelegationTools();
			printf("<br><input class=largeButton title='Confirms the delegation. Will create all "
			"the required NS and CNAME records' type=submit name=gcCommand value='Confirm Delegation'><br>\n");
		break;

		case 5000:
			printf("<p><u>Delegation Tools. Confirm delegation removal</u></p>\n");
			printf("<p>By pressing the button below you confirm the removal of the  delegation. "
			"You can avoid the modify job submission for this zone upon delegation by un-checking "
			" the uSubmitJob checkbox below.</p>\n");
			if(!uSubmitJob)
				uSubmitJob=1;
			DelegationTools();
			 printf("<br><input class=largeButton title='Confirms the delegation removal. Will "
			 "remove all the NS and CNAME records for the specified IP block' type=submit "
			 "name=gcCommand value='Confirm Del. Removal'><br>\n");
		break;			

		default:
			uDefault=1;
			printf("<u>Table Tips</u><br>");
			printf("The zone table provides the SOA header and some other iDNS only fields. "
				"And is of course the anchor point for all a given zone's resource records. "
				"A zone may have n instances via uView. Important non BIND fields that must be "
				"understood are the uNSSet (required), uMailServer (optional) server groups "
				"and the specialized uSecondaryOnly, cMasterIPs and cOptions. cMainAddress is a "
				"backwords compatible optional field (mysqlBind) for simple A record only zones.\n");
			printf("<p><u>Search Tools</u><br>");
			printf("<input type=text title='cZone search. Use %% . and _"
				" for pattern matching when applicable.' name=cSearch value=\"%s\" maxlength=63"
				" size=20> cSearch ",cSearch);
			printf("<input type=checkbox title='Limit to uSecondaryOnly=Yes if checked' name=uSearchSecOnly");
			if(uSearchSecOnly)  printf(" checked");
			printf(" > uSecondaryOnly");
			tZoneNavList();
			if(guLoginClient==uOwner || guReseller==guLoginClient 
						|| guPermLevel>9)
			{
				printf("<p><u>Zone Management Tools</u><br>");
				if(uZone)
				printf("<input class=largeButton title=\"%s\" type=submit name=gcFind value=\"%s\"><br><br>",
					LANG_BT_TableAddRR,LANG_BL_TableAddRR);
			}
			tZoneContextInfo();
			printf("<p><u>Other Zone Tools</u><br>");
			if(uZone && guPermLevel>9)
			{
				printf("<input class=largeButton title='Check SOA via dig text output' "
					"type=submit name=gcFind value='Check SOA'>"); 
				printf("<br><input class=largeButton title='Show master zone file contents' "
					"type=submit name=gcFind value='Master Zone File'>"); 
				printf("<br><input class=largeButton title='Verify that cZone is in master.zones' "
					"type=submit name=gcFind value='Check master.zones'>"); 
				printf("<br><input class=largeButton title='Verify that cZone has no errors that "
					"may cause it not to load or propagate' type=submit name=gcFind"
					" value='Run named-checkzone'>"); 
				if(guPermLevel>9&&(strstr(cZone,".in-addr.arpa")||strstr(cZone,".ip6.arpa")))
					printf("<br><input class=largeButton title='IP Block Delegation Tools' "
						"type=submit name=gcCommand value='Delegation Tools'>\n");
				printf("<br><input class=largeButton title='Check external via resolv.conf NS records' "
					"type=submit name=gcFind value='External NS Records'>"); 
			}
			if(guPermLevel>9)
			{
				printf("<p><u>Other Tools</u><br>");
				printf("<input type=submit class=largeButton title='Open RR search set page."
					" There you can create search sets and operate"
					" on selected resources of the loaded resource set.'"
					" name=gcCommand value='RR Search Set OPs'>\n");
				printf("<br><input type=submit class=largeButton title='Open zone search set page."
					" There you can create search sets and operate"
					" on selected zones of the loaded zone set.'"
					" name=gcCommand value='Zone Search Set OPs'>\n");
				printf("<br><input class=largeButton title='Mass/Bulk operations. !Being deprecated!' type=submit"
					" name=gcCommand value='Mass Operations'><br>\n");
				printf("<p><u>Clone Zone</u><br>");
				printf("<input type=text name=cTargetZone size=30 title='Enter the name of the zone you "
					"wish to clone the loaded zone to'<br>\n");
				printf("<input class=lwarnButton title='Clone the loaded zone into a new zone entered above'"
					" type=submit name=gcCommand value='Clone Zone'><br>\n");
			}

	}

	if(!uDefault && cSearch[0])
	{
		printf("<input type=hidden name=cSearch value=\"%s\">",cSearch);
		if(uSearchSecOnly) printf("<input type=hidden name=uSearchSecOnly >");
	}
	
	CloseFieldSet();
	
}//void ExttZoneButtons(void)


void ExttZoneAuxTable(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uGroup=0;
	unsigned uRow=0;
	unsigned uNumRows=0;
	//unsigned uOnlyOncePerSet=1;

	switch(guMode)
	{
		case 12001:
		case 12002:
			//Set operation buttons
			OpenFieldSet("Set Operations",100);
			printf("<input title='For supported set operations (like [Fold A Records])"
				" create the appropiate jobs for changes to go into effect.'"
				" type=checkbox name=uSubmitJobNoCA");
			if(uSubmitJob)
				printf(" checked");
			printf("> uSubmitJob");
			printf(" &nbsp;<input title='For supported set operations (like [Fold A Records])"
				" overwrite existing resource records.'"
				" type=checkbox name=uOverwriteRRsNoCA");
			if(uOverwriteRRs)
				printf(" checked");
			printf("> uOverwriteRRs");
			printf(" &nbsp;<input type=text size=30 title='For supported set operations (like [Fold A Records)]"
					" you can specify a MySQL exclude pattern (E.g. %%.backup%%) for A records names'"
					" name=cExcludePattern value='%s'> cExcludePattern\n",cExcludePattern);
			printf("<p><input title='Delete checked resource records from your search set. They will still be visible but will"
				" marked deleted and will not be used in any subsequent set operation'"
				" type=submit class=largeButton name=gcCommand value='Delete Checked'>\n");
			printf("&nbsp;<input title='Copy A records from this zone into the parent zone of same view if it exists'"
				" type=submit class=lwarnButton name=gcCommand value='Fold A Records'>\n");
			printf("&nbsp;<input title='Backup zone and all resource records into tZoneBackup and tResourceBackup tables."
				" Any existing backup records will be deleted or replaced!'"
				" type=submit class=lwarnButton name=gcCommand value='Backup'>\n");
			printf("&nbsp;<input title='Restore zone from tZoneBackup and tResourceBackup tables."
				" Any existing tZone and tResource records will be deleted or replaced!'"
				" type=submit class=lwarnButton name=gcCommand value='Restore'>\n");
			printf("&nbsp;<input title='Whois report of name servers'"
				" type=submit class=largeButton name=gcCommand value='Whois'>\n");
			CloseFieldSet();

			sprintf(gcQuery,"Search Set Contents");
			OpenFieldSet(gcQuery,100);
			uGroup=uGetZoneSearchGroup(gcUser);
			//we need to use left joins here to allow for null uClient
			sprintf(gcQuery,"SELECT"
					" tZone.uZone,"
					" tZone.cZone,"
					" tView.cLabel,"
					" tNSSet.cLabel,"
					//not for display
					" tView.uView,"
					" tNSSet.uNSSet"
					" FROM tZone,tView,tNSSet"
					" WHERE tZone.uView=tView.uView"
					" AND tZone.uNSSet=tNSSet.uNSSet"
					" AND uZone IN (SELECT uZone FROM tGroupGlue WHERE uGroup=%u) ORDER BY tZone.cZone",uGroup);
		        mysql_query(&gMysql,gcQuery);
		        if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
		        res=mysql_store_result(&gMysql);
			if((uNumRows=mysql_num_rows(res)))
			{
				char cResult[256]={""};
				char cCtLabel[100]={""};

				printf("<table>");
				printf("<tr>");
				printf("<td valign=top>"
					"<input title='Check all records' type=checkbox name=all onClick='checkAll(document.formMain,this)'></td>"
					"<td><u>cZone</u></td>"
					"<td><u>View</u></td>"
					"<td><u>NSSet</u></td>"
					"<td><u>Set operation result</u></td></tr>");
//Reset margin start
while((field=mysql_fetch_row(res)))
{
	if(guMode==12002)
	{
		register int i;
		unsigned uCtZone=0;

		cResult[0]=0;
		sscanf(field[0],"%u",&uCtZone);
		sprintf(cCtLabel,"Ct%u",uCtZone);
		for(i=0;i<x;i++)
		{
			//insider xss protection
			if(guPermLevel<10)
				continue;

			if(!strcmp(entries[i].name,cCtLabel))
			{
				if(!strcmp(gcCommand,"Delete Checked"))
				{
					sprintf(gcQuery,"DELETE FROM tGroupGlue WHERE uGroup=%u AND uZone=%u",uGroup,uCtZone);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}

					if(mysql_affected_rows(&gMysql)>0)
						sprintf(cResult,"Deleted from set");
					else
						sprintf(cResult,"Unexpected non deletion");
					break;
				}//Delete Checked

				else if(!strcmp(gcCommand,"Whois"))
				{
					sprintf(cResult,cNSFromWhois(field[1],field[5]));
					break;
				}//Delete Checked

				else if(!strcmp(gcCommand,"Restore"))
				{
					MYSQL_RES *res2;

					sprintf(gcQuery,"SELECT uZone FROM tZoneBackup WHERE uZone=%s",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					res2=mysql_store_result(&gMysql);
					if(mysql_num_rows(res2)<1)
					{
						sprintf(cResult,"No tZoneBackup records!");
						break;
					}
					sprintf(gcQuery,"SELECT uZone FROM tResourceBackup WHERE uZone=%s",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					res2=mysql_store_result(&gMysql);
					if(mysql_num_rows(res2)<1)
					{
						sprintf(cResult,"No tResourceBackup records!");
						break;
					}
					//sprintf(cResult,"d1");
					//break;

					sprintf(gcQuery,"DELETE FROM tZone WHERE uZone=%s",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					sprintf(gcQuery,"DELETE FROM tResource WHERE uZone=%s",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					sprintf(gcQuery,"INSERT tZone SELECT * FROM tZoneBackup WHERE uZone=%s",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					//Mark restore modby
					sprintf(gcQuery,"UPDATE tZone SET uModDate=UNIX_TIMESTAMP(NOW()),uModBy=%u WHERE uZone=%s",
							guLoginClient,field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					sprintf(gcQuery,"INSERT tResource SELECT * FROM tResourceBackup WHERE uZone=%s",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					if(mysql_affected_rows(&gMysql)>0)
					{
						sprintf(cResult,"Restore done");
					}
					else
					{
						sprintf(cResult,"No resource records?");
						break;
					}
					if(uSubmitJob)
					{
						unsigned uZone=0;
						sscanf(field[0],"%u",&uZone);
						unsigned uNSSet=0;
						sscanf(ForeignKey("tZone","uNSSet",uZone),"%u",&uNSSet);
						if(uNSSet && uZone)
						{
							time_t clock;
							time(&clock);
							UpdateSerialNum(uZone);                      	
							if(SubmitJob("Modify",uNSSet,field[1],0,clock+60))
								strncat(cResult,mysql_error(&gMysql),31);
							else
								strncat(cResult," +uSubmitJob",31);
						}
					}
					break;
				}//Restore

				else if(!strcmp(gcCommand,"Backup"))
				{
					//CREATE TABLE newtable LIKE oldtable; 
					//INSERT newtable SELECT * FROM oldtable;

					sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tZoneBackup LIKE tZone");
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tResourceBackup LIKE tResource");
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}

					sprintf(gcQuery,"DELETE FROM tZoneBackup WHERE uZone=%s",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					sprintf(gcQuery,"DELETE FROM tResourceBackup WHERE uZone=%s",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					sprintf(gcQuery,"INSERT tZoneBackup SELECT * FROM tZone WHERE uZone=%s",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					//Mark restore modby
					sprintf(gcQuery,"UPDATE tZoneBackup SET uModDate=UNIX_TIMESTAMP(NOW()),uModBy=%u WHERE uZone=%s",
							guLoginClient,field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					sprintf(gcQuery,"INSERT tResourceBackup SELECT * FROM tResource WHERE uZone=%s",field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					if(mysql_affected_rows(&gMysql)>0)
						sprintf(cResult,"Backup done");
					else
						sprintf(cResult,"No resource records?");
					break;
				}//Backup

				else if(!strcmp(gcCommand,"Fold A Records"))
				{
					MYSQL_RES *res2;
					MYSQL_ROW field2;

					unsigned uSourceZone=0;
					unsigned uDestinationZone=0;
					unsigned uNSSet=0;
					sscanf(field[0],"%u",&uSourceZone);

					sprintf(gcQuery,"SELECT uZone,cZone,uNSSet FROM tZone"
							" WHERE cZone=SUBSTR('%s',INSTR('%s','.')+1)"
							" AND uView=%s",
								field[1],field[1],
								field[4]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,"%.255s",mysql_error(&gMysql));
						break;
					}
					res2=mysql_store_result(&gMysql);
					if(mysql_num_rows(res2)>1)
					{
						sprintf(cResult,"More than one candidate zone.");
						break;
					}
					if((field2=mysql_fetch_row(res2)))
					{
						MYSQL_RES *res3;
						MYSQL_ROW field3;

						if(!uSourceZone)
						{
							sprintf(cResult,"uSourceZone==0");
							break;
						}
						sscanf(field2[0],"%u",&uDestinationZone);
						sscanf(field2[2],"%u",&uNSSet);
						if(!uDestinationZone)
						{
							sprintf(cResult,"uDestinationZone==0");
							break;
						}
						//cp A RRs from field[0] (uZone) do not overwrite if exist
						//if fully qualified remove parent zone part
						if(cExcludePattern[0])
							sprintf(gcQuery,"SELECT CONCAT("
										"IF(SUBSTR(cName,-1)='.',"
											"SUBSTR(cName,1,INSTR(cName,'%s')-2),"
											"cName)"
										",'.','%s','.'),"
									"cParam1,cComment,uOwner,uCreatedBy,uCreatedDate"
								" FROM tResource WHERE uZone=%u AND uRRType=1 AND cName NOT LIKE '%s'",
										field2[1],field2[1],uSourceZone,cExcludePattern);
						else
							sprintf(gcQuery,"SELECT CONCAT("
										"IF(SUBSTR(cName,-1)='.',"
											"SUBSTR(cName,1,INSTR(cName,'%s')-2),"
											"cName)"
										",'.','%s','.'),"
									"cParam1,cComment,uOwner,uCreatedBy,uCreatedDate"
								" FROM tResource WHERE uZone=%u AND uRRType=1",field2[1],field2[1],uSourceZone);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
						{
							sprintf(cResult,"%.255s",mysql_error(&gMysql));
							break;
						}
						//debug
						//sprintf(cResult,"%.255s",gcQuery);
						//break;
						res3=mysql_store_result(&gMysql);
						unsigned uNumRows=0;
						cResult[0]=0;
						while((field3=mysql_fetch_row(res3)))
						{
							MYSQL_RES *res4;
							MYSQL_ROW field4;
							sprintf(gcQuery,"SELECT uResource FROM tResource WHERE uZone=%u"
									" AND cName='%s'"
									" AND uRRType=1"
										,uDestinationZone,field3[0]);
							mysql_query(&gMysql,gcQuery);
							if(mysql_errno(&gMysql))
							{
								sprintf(cResult,"%.255s",mysql_error(&gMysql));
								break;
							}
							res4=mysql_store_result(&gMysql);
							//only add if cName does not exist in parent zone (uDestinationZone)
							unsigned uRows=0;
							if(((uRows=mysql_num_rows(res4))==0) || uOverwriteRRs)
							{
								if(!uRows)
								{
							
									sprintf(gcQuery,"INSERT INTO tResource"
									" SET uZone=%u,"
									"cName='%s',"
									"uRRType=1,"
									"cParam1='%s',"
									"cComment='FoldARecords %s',"
									"uOwner=%s,"
									"uCreatedBy=%s,"
									"uCreatedDate=%s,"
									"uModDate=UNIX_TIMESTAMP(NOW()),"
									"uModBy=%u"
										,uDestinationZone,field3[0],field3[1],field3[2],
										field3[3],field3[4],field3[5],guLoginClient);
									mysql_query(&gMysql,gcQuery);
									if(mysql_errno(&gMysql))
									{
										sprintf(cResult,"%.255s",mysql_error(&gMysql));
										break;
									}
									if(mysql_affected_rows(&gMysql)>0)
										uNumRows++;
								}
								else
								{
									if(!(field4=mysql_fetch_row(res4)))
									{
										sprintf(cResult,"field4 error");
										break;
									}
									
									sprintf(gcQuery,"UPDATE tResource SET"
									" cParam1='%s',"
									" cComment='FoldARecords update %s',"
									" uModDate=UNIX_TIMESTAMP(NOW()),"
									" uModBy=%u WHERE uResource=%s",
										field3[1],
										field3[2],
										guLoginClient,field4[0]);
									mysql_query(&gMysql,gcQuery);
									if(mysql_errno(&gMysql))
									{
										sprintf(cResult,"%.255s",mysql_error(&gMysql));
										break;
									}
									if(mysql_affected_rows(&gMysql)>0)
										uNumRows++;
								}
								if(uSubmitJob && uNSSet)
								{
									time_t clock;
									time(&clock);
									UpdateSerialNum(uDestinationZone);                      	
									if(SubmitJob("Modify",uNSSet,field2[1],0,clock+60))
									{
										strncat(cResult,mysql_error(&gMysql),31);
										break;
									}
								}
							}
							else
							{
								sprintf(cResult,"RR exists.");
								break;
							}
						}

						//
						//debug
						//sprintf(cResult,"%s %s %s",field3[0],field3[1],field3[2]);
						//sprintf(cResult,"%s",gcQuery);
						//break;

						if(cResult[0]==0)
							sprintf(cResult,"Folded %u A records into %s",uNumRows,field2[1]);
					}
					else
					{
						sprintf(cResult,"No parent zone for view %s/%s found",field[2],field[4]);
					}
					mysql_free_result(res2);
					break;
				}

				else if(1)
				{
					sprintf(cResult,"Unexpected gcCommand=%.64s",gcCommand);
					break;
				}
			}//end if Ct block
		}//end for()
	}

	printf("<tr");
	if((uRow++) % 2)
		printf(" bgcolor=#E7F3F1 ");
	else
		printf(" bgcolor=#EFE7CF ");
	printf(">");

		printf("<td valign=top><input type=checkbox name=Ct%s ></td>"
		"<td valign=top><a class=darkLink href=?gcFunction=tZone&uZone=%s>%s</a></td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.255s</td>\n",
			field[0],
			field[0],field[1],
			field[2],field[3],cResult);
	printf("</tr>");

}//while()
//Reset margin end

			printf("<tr><td valign=top>"
				"<input title='Check all records' type=checkbox name=all onClick='checkAll(document.formMain,this)'></td>"
				"<td valign=top colspan=3>Check all %u zone records</td></tr>\n",uNumRows);
			printf("</table></form>");

			}//If results
			mysql_free_result(res);
			CloseFieldSet();
		break;

		default:
			ResourceRecordList(uZone);
		break;

	}//switch(guMode)

}//void ExttZoneAuxTable(void)


unsigned uPTRInBlock(unsigned uZone,unsigned uStart,unsigned uEnd)
{
	MYSQL_RES *res;
	register unsigned uI;

	for(uI=uStart;uI<uEnd;uI++)
	{
		sprintf(gcQuery,"SELECT cName FROM tResource WHERE uRRType=7 AND uZone=%u AND cName='%u'",uZone,uI);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)) return(1);
	}
	
	return(0);
	
}//unsigned uPTRInBlock(unsigned uZone,unsigned uStart,unsigned uEnd)
	
			
unsigned uPTRInCIDR(unsigned uZone,char *cIPBlock)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uA,uB,uC;
	char cIP[100]={""};

	sscanf(cZone,"%u.%u.%u.",&uC,&uB,&uA);
	
	//uRRType=7: PTR
	sprintf(gcQuery,"SELECT cName FROM tResource WHERE uRRType=7 AND uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		sprintf(cIP,"%u.%u.%u.%s",uA,uB,uC,field[0]);
		if(uIpv4InCIDR4(cIP,cIPBlock)) return(1);
	}
	return(0);
	
}//unsigned uPTRInCIDR(unsigned uZone,char *cIPBlock)


void DelegationTools(void)
{
	printf("<u>cIPBlock</u><br><input type=text name=cIPBlock size=30 value='%s' "
		"title='IP block in CIDR (e.g. 217.68.21.0/24) or in dash "
		"(e.g. 217.68.21.0-255) format'><br>\n",cIPBlock);
	printf("<u>uTTL</u><br><input type=text name=uDelegationTTL size=30 value='%u' "
		"title='NS RR TTL. If 0, will be set to the default zone TTL'><br>\n",uDelegationTTL);
	printf("<u>cNSList</u><br><textarea  cols=40 wrap=hard rows=3 name=cNSList "
		"title='List of NSs to which you are going to delegate the IP block'>%s</textarea><br>\n",cNSList);
	printf("<u>uSubmitJob</u><br><input title='If not checked, no modify job will "
		"be created for the zone upon delegation' type=checkbox name=uSubmitJobCA value=1 ");
	if(uSubmitJob)
		printf("checked");
	printf(">\n");

}//void DelegationTools(void)


void ResourceRecordList(unsigned uZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	register int i=0;
	register int uArpa=0;
	
	char cTTL[16]={"&nbsp;"};
	char cName[102];
	
	if(!strcmp(cZone+strlen(cZone)-5,".arpa"))
	{
		sprintf(gcQuery,"SELECT tResource.cName,tResource.uTTL,tRRType.cLabel,"
				"tResource.cParam1,tResource.cParam2,tResource.cComment,"
				"tResource.uResource FROM tResource,tRRType WHERE "
				"tResource.uRRType=tRRType.uRRType AND tResource.uZone=%u ORDER BY ABS(tResource.cName)",uZone);
		uArpa=1;
	}
	else
	{
		sprintf(gcQuery,"SELECT tResource.cName,tResource.uTTL,tRRType.cLabel,"
				"tResource.cParam1,tResource.cParam2,tResource.cParam3,"
				"tResource.cParam4,tResource.cComment,tResource.uResource "
				"FROM tResource,tRRType WHERE tResource.uRRType=tRRType.uRRType "
				"AND tResource.uZone=%u ORDER BY tResource.cName",uZone);
	}
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	OpenFieldSet("Resource Records",100);
	if(uArpa)
		printf("<tr bgcolor=black>"
			"<td ><font color=white>Name</td><td ><font color=white>TTL</td>"
			"<td ><font color=white>Type</td><td ><font color=white>Param 1</td>"
			"<td ><font color=white>Param 2</td ><td><font color=white>Comment</td></tr>");
	else
			printf("<tr bgcolor=black>"
				"<td ><font color=white>Name</td><td ><font color=white>TTL</td>"
				"<td ><font color=white>Type</td><td ><font color=white>Param 1</td>"
				"<td ><font color=white>Param 2</td ><td><font color=white>Param 3</td>"
				"<td><font color=white>Param 4</td><td><font color=white>Comment</td></tr>");
	while((field=mysql_fetch_row(res)))
	{
		if(strcmp(field[1],"0"))
			sprintf(cTTL,"%.15s",field[1]);
		else
			sprintf(cTTL,"&nbsp;");

		if(!field[0][0] || field[0][0]=='\t')
			strcpy(cName,"@");
		else
			strcpy(cName,field[0]);

		if(uArpa)
			printf("<tr>"
			"<td valign=top><a class=darkLink href=?gcFunction=tResource&uResource=%s&cZone=%s>%s</a>"
			"</td><td valign=top>%s</td><td valign=top>%s</td><td valign=top>%s</td><td valign=top>%s</td>"
			"<td valign=top>%s</td>"
			"</tr>\n",
					field[6],cZone,cName,cTTL,field[2],
					field[3],field[4],field[5]);
		else
/*
tResource.cName 0 
,tResource.uTTL 1
,tRRType.cLabel 2
,tResource.cParam1 3
,tResource.cParam2 4
,tResource.cParam3 5 
,tResource.cParam4 6
,tResource.cComment, 7
tResource.uResource 8
*/
			printf("<tr>"
				"<td valign=top><a class=darkLink href=?gcFunction=tResource&uResource=%s>%s</a></td>"
				"<td valign=top>%s</td><td valign=top>%s</td><td valign=top>%.64s</td><td valign=top>%s</td>"
				"<td valign=top>%s</td><td valign=top>%s</td><td valign=top>%s</td>"
				"</tr>\n",
					field[8],cName,cTTL,field[2],
					field[3],field[4],field[5],
					field[6],field[7]);
		i++;

	}
	if(!i)
		printf("<tr><td colspan=6>No resource records</td></tr>\n");
	CloseFieldSet();






}//void ResourceRecordList(void)


void ExttZoneGetHook(entry gentries[], int x)
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
		else if(!strcmp(gentries[i].name,"cSearch"))
			sprintf(cSearch,"%.63s",gentries[i].val);
		else if(!strcmp(gentries[i].name,"uSearchSecOnly"))
			uSearchSecOnly=1;

	}
//Debug only
//printf("Content-type: text/plain\n\n");
//printf("uZone=%u,guMode=%u\n",uZone,guMode);
//exit(0);
	if(uZone) ExttZoneSelectRow();

	tZone("");

}//void ExttZoneGetHook(entry gentries[], int x)


void ExttZoneSelect(void)
{
	//Owner 0 is public. Example arpa zones
        //Set non search query here for tTableName()
	if(cSearch[0] && !uSearchSecOnly)
		//ExtSelectSearch("tZone",VAR_LIST_tZone,cSearch,"cZone",NULL,20);
		ExtSelectSearch("tZone",VAR_LIST_tZone,"cZone",cSearch,NULL,20);
	else if(cSearch[0] && uSearchSecOnly)
		ExtSelectSearch("tZone",VAR_LIST_tZone,"cZone",cSearch,"uSecondaryOnly=1",20);
	else if(1)
		ExtSelect("tZone",VAR_LIST_tZone,0);

}//void ExttZoneSelect(void)


void ExttZoneSelectRow(void)
{
	ExtSelectRow("tZone",VAR_LIST_tZone,uZone);

}//void ExttZoneSelectRow(void)


void ExttZoneListSelect(void)
{
	char cCat[512];

	ExtListSelect("tZone",VAR_LIST_tZone);

	//Changes here must be reflected below in ExttZoneListFilter()
        if(!strcmp(gcFilter,"cZone"))
        {
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tZone.cZone LIKE '%s%%' ORDER BY tZone.cZone",TextAreaSave(gcCommand));
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"cMainAddress"))
        {
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tZone.cMainAddress LIKE '%s%%' ORDER BY tZone.cZone",TextAreaSave(gcCommand));
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uZone"))
        {
                sscanf(gcCommand,"%u",&uZone);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tZone.uZone=%u",uZone);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uOwner"))
        {
                sscanf(gcCommand,"%u",&uZone);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tZone.uOwner=%u ORDER BY tZone.cZone",uZone);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uNSSet"))
        {
                sscanf(gcCommand,"%u",&uZone);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tZone.uNSSet=%u ORDER BY tZone.cZone",uZone);
		strcat(gcQuery,cCat);
	}
        else if(!strcmp(gcFilter,"uMailServer"))
        {
                sscanf(gcCommand,"%u",&uZone);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tZone.uMailServer=%u ORDER BY tZone.cZone",uZone);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"uSecondaryOnly"))
        {
                sscanf(gcCommand,"%u",&uZone);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tZone.uSecondaryOnly=%u ORDER BY tZone.cZone",uZone);
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uZone");
        }

}//void ExttZoneListSelect(void)


void ExttZoneListFilter(void)
{
        //Filter
        printf("<td align=right >Select ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uZone"))
                printf("<option>uZone</option>");
        else
                printf("<option selected>uZone</option>");
        if(strcmp(gcFilter,"cZone"))
                printf("<option>cZone</option>");
        else
                printf("<option selected>cZone</option>");
        if(strcmp(gcFilter,"cMainAddress"))
                printf("<option>cMainAddress</option>");
        else
                printf("<option selected>cMainAddress</option>");
        if(strcmp(gcFilter,"uOwner"))
                printf("<option>uOwner</option>");
        else
                printf("<option selected>uOwner</option>");
        if(strcmp(gcFilter,"uNSSet"))
                printf("<option>uNSSet</option>");
        else
                printf("<option selected>uNSSet</option>");
        if(strcmp(gcFilter,"uMailServer"))
                printf("<option>uMailServer</option>");
        else
                printf("<option selected>uMailServer</option>");
        if(strcmp(gcFilter,"uSecondaryOnly"))
                printf("<option>uSecondaryOnly</option>");
        else
                printf("<option selected>uSecondaryOnly</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttZoneListFilter(void)


void ExttZoneNavBar(void)
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

	if(uOwner || guLoginClient==1)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttZoneNavBar(void)


void TableAddRR(void)
{
	tResourceTableAddRR(uZone);

}//TableAddRR


int AddNewArpaZone(const char *cArpaZone, unsigned uExtNSSet, char *cExtHostmaster, unsigned uExtOwner)
{
	int retval=0;
	char cSerial[32];
	
	unsigned uSaveuZone=uZone;
	unsigned uSaveuCreatedBy=uCreatedBy;
	unsigned uSaveuOwner=uOwner;
	unsigned uSaveuModBy=uModBy;
	unsigned uSaveuSerial=uSerial;
	char cSavecMainAddress[100];
	char cSavecZone[100];
	
	strcpy(cSavecMainAddress,cMainAddress);
	strcpy(cSavecZone,cZone);

	uZone=0;
	strcpy(cMainAddress,"0.0.0.0");
	strcpy(cZone,cArpaZone);
	uCreatedBy=0;//Do not provide info for other resellers etc...
	if(strstr(cZone,"ip6.arpa") && uExtOwner)
		uOwner=uExtOwner;
	else
		uOwner=0;//Public zone unless ip6.arpa
	uModBy=0;//Never modified
	uView=1;
	SerialNum(cSerial);
	sscanf(cSerial,"%u",&uSerial);
	uNSSet=uExtNSSet;
	strcpy(cHostmaster,cExtHostmaster);
	
	Insert_tZone();
	if(mysql_errno(&gMysql)) 
		retval=1;
	//Add to both internal and external views. If system
	//is hosting more than 2 views. Must be edited via tZone
	//by operator.
	uView=2;
	Insert_tZone();
	if(mysql_errno(&gMysql)) 
		retval=1;

	uZone=uSaveuZone;
	uCreatedBy=uSaveuCreatedBy;
	uOwner=uSaveuOwner;
	uModBy=uSaveuModBy;
	uSerial=uSaveuSerial;
	strcpy(cMainAddress,cSavecMainAddress);
	strcpy(cZone,cSavecZone);

	return(retval);

}//int AddNewArpaZone()


void UpdateSerialNum(unsigned uZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	long unsigned luYearMonDay=0;
	unsigned uSerial=0;
	char cSerial[16]={""};


	sprintf(gcQuery,"SELECT uSerial FROM tZone WHERE uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uSerial);
	mysql_free_result(res);
	
	SerialNum(cSerial);
	sscanf(cSerial,"%lu",&luYearMonDay);


	//Typical year month day and 99 changes per day max
	//to stay in correct date format. Will still increment even if>99 changes in one day
	//but will be stuck until 1 day goes by with no changes.
	if(uSerial<luYearMonDay)
		sprintf(gcQuery,"UPDATE tZone SET uSerial=%s WHERE uZone=%u",cSerial,uZone);
	else
		sprintf(gcQuery,"UPDATE tZone SET uSerial=uSerial+1 WHERE uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

}//void UpdateSerialNum(unsigned uZone)


int IllegalZoneDataChange(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uRetVal=0;
	
	sprintf(gcQuery,"SELECT cZone FROM tZone WHERE uZone=%u",uZone);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	if((field=mysql_fetch_row(res)))
	{
		if(strcmp(field[0],cZone))
			uRetVal=1;
	}
	mysql_free_result(res);
	
	return(uRetVal);

}//int IllegalZoneDataChange(void)


#ifndef DEBUG_REPORT_STATS_OFF

int UpdateInfo(void)
{
	register int sd, rc;
	struct sockaddr_in cliAddr, remoteServAddr;
	struct hostent *h;
	time_t luClock=0,luModDate=0;

	char cQuery[256];

	MYSQL_RES *res;
	MYSQL_ROW field;

	char cInfo[128]={"Emtpy"};
	unsigned uMaxuZone=0;

	time(&luClock);

	sprintf(cQuery,"SELECT uModDate FROM tConfiguration WHERE cLabel='UpdateInfo'");
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
		return(1);
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%lu",&luModDate);
	mysql_free_result(res);

	if(luModDate && (luClock < (luModDate + 86400)) )
		return(2);

	sprintf(cQuery,"SELECT MAX(uZone) FROM tZone");
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
		return(3);
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		if(field[0]!=NULL)
			sscanf(field[0],"%u",&uMaxuZone);
	}
	mysql_free_result(res);

	sprintf(cInfo,"iDNS%s %u %u %u %.70s",REV,guLoginClient,guPermLevel,uMaxuZone,cZone);

	if(!cInfo[0])
		return(4);

	h=gethostbyname("saturn.openisp.net");
	if(h==NULL)
		return(5);


	remoteServAddr.sin_family = h->h_addrtype;
	memcpy((char *) &remoteServAddr.sin_addr.s_addr,h->h_addr_list[0], h->h_length);
	remoteServAddr.sin_port=htons(53);

	sd=socket(AF_INET,SOCK_DGRAM,0);
	if(sd<0)
		return(6);
  
	cliAddr.sin_family = AF_INET;
	cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	cliAddr.sin_port = htons(0);
  
	rc=bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
	if(rc<0)
		return(7);


	rc=sendto(sd,cInfo,strlen(cInfo)+1,0,(struct sockaddr *)&remoteServAddr,
				sizeof(remoteServAddr));

	if(rc<0)
		return(8);

	if(luModDate)
		sprintf(cQuery,"UPDATE tConfiguration SET uModBy=1,uModDate=%lu,cComment='%s' WHERE cLabel='UpdateInfo'",
			luClock,cInfo);
	else
		sprintf(cQuery,"INSERT INTO tConfiguration SET cLabel='UpdateInfo',cValue='uModDate',cComment='%s',"
				"uCreatedBy=1,uCreatedDate=%lu,uModDate=%lu,uModBy=1",cInfo,luClock,luClock);
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
	{
		char cDebug[512]={""};
		sprintf (cDebug,"query=%s error=%s",cQuery,mysql_error(&gMysql));
		tZone(cDebug);
		return(9);
	}
  
	return(0);

}//int UpdateInfo(void)

#endif


void MassOperations(void)
{
	Header_ism3(":: tZone:Mass Operations",0);

	printf("<table width=100%% cellspacing=0 cellpadding=0>\n");
	printf("<tr><td colspan=2 align=right valign=center>");
	printf("<input type=hidden name=gcFunction value=tZoneTools>");
	if(cMessage[0]) 
	{
		printf("%s",cMessage);

		//Extended help via tGlossary
		char cText[512]={"#No tGlossary help text available!"};
		if(gcFind[0])
		{
			GetGlossary(gcFind,cText);
			if(cText[0])
				cMassList=cText;
		}
	}
	printf("</td></tr>");
	printf("<tr><td valign=top width=25%%>");
	OpenFieldSet("Mass/Bulk OPs Panel",100);

	//Panel help
	printf("Warning this panel is not a two step system. No undo available. Operations take place immediately. "
		"Output is text/plain suitable for cutting and pasting into reports and other mass/bulk operations. "
		"<p><font color=red>Red</font> buttons require advanced knowledge and may change your data. Make sure "
		" you have a fresh backup before making mass changes "
		"to your DNS data. "
		"<p>Most lists with more than one column per line can be csv, tab-sv or single space "
		"separated values. More per OP help available by clicking on yellow or red mass OP buttons with "
		"<i>cMassList</i> <b>empty.</b><p>");

	//May apply to buttons below
	printf("<p><u>Use this customer for OP (if applies)</u><br>");
	CustomerDropDown(uDDClient);

	//Report Zones From Blocks
	printf("<p><input class=largeButton title='Get sorted list of zones back based on tBlock entries. Optionally "
		"limit to uOwner. No cMassList entries used.' type=submit name=gcCommand value='Zones From Blocks'\n");

	//Report Customer Zones
	printf("<p><input class=largeButton title='Select customer and get sorted list of zones back. No cMassList "
		"entries used.' type=submit name=gcCommand value='Customer Zones'>\n");

	//Report Zone List
	printf("<p><input class=lalertButton title='Create a custom list of zones based on LIKE syntax. "
		"Customer drop down maybe used.' type=submit name=gcFind value='Zone List'>\n");

	//In tZone?
	printf("<p><input class=lalertButton title='Are these zones in tZone? Only looks at first column. "
		"CSV or TabSV ok. No double quotes.' type=submit name=gcFind value='In tZone?'>\n");

	//named-checkzone
	printf("<p><input class=lalertButton title='Create named-checkzone commands for all zones in list' "
		"type=submit name=gcFind value='Mass named-checkzone'>");

	//Mass PTR Check
	printf("<p><input class=lalertButton title='Check list of in-addr.arpa zones:N PTR cFQDN lines for "
		"existence' type=submit name=gcFind value='Mass PTR Check'>");

	//Mass Resource Import
	printf("<p><input class=lwarnButton title='Import RRs for existing zones. Allows import of multiple "
		"zones with multiple RRs' type=submit name=gcFind value='Mass Resource Import'>");

	//Secondary Service Only
	printf("<p><input class=lwarnButton title='Add zones in two col list: cZone,tNSSet.cMasterIPs(with "
		"the ;)|cLabel|uNSSet. CSV or Tab/SpaceSV. No quotes.' type=submit name=gcFind value='Secondary Service Only'>\n");

	//Secondary Service Cleanup
	printf("<p><input class=lwarnButton title='Remove RRs from all zones with uSecondaryOnly set to yes '"
		" type=submit name=gcFind value='Secondary Service Cleanup'>\n");

	//Mass Update
	printf("<p><input class=lwarnButton title='Create modify jobs for zones in list. Get extended help via "
		"empty cMassList!' type=submit name=gcFind value='Mass Update'>");

	//Mass Delete
	printf("<p><input class=lwarnButton title='delete zones in list from db no jobs created' "
		"type=submit name=gcFind value='Mass Delete'>");

	
	//Mass Resource Fix
	//printf("<p><input class=lwarnButton title='Custom bind.c Import() $ORIGIN bug fix' "
	//	"type=submit name=gcFind value='Mass Resource Fix' disabled>");

	//Copy to Views
	printf("<p><input class=lwarnButton title='Copy existing zone data (and RRs) to all other views' "
		" type=submit name=gcFind value='Copy to Views'>\n");

	CloseFieldSet();

        printf("</td><td valign=top>");
	OpenFieldSet("Mass Data Entry Panel",100);
	OpenRow("cMassList","black");
	printf("<textarea title='Enter your list. Cut and paste from other sources.' cols=80 rows=20 name=cMassList");
	if(guPermLevel>=7)
	{
		printf(">%s</textarea></td></tr>\n",cMassList);
	}
	else
	{
		printf("disabled>%s</textarea></td></tr>\n",cMassList);
		printf("<input type=hidden name=cMassList value=\"%s\" >\n",EncodeDoubleQuotes(cMassList));
	}
	CloseFieldSet();


	Footer_ism3();

}//void MassOperations(void)


//Does not allow empty lines...this may need reviewing ;) to say the least.
char *ParseTextAreaLines(char *cTextArea)
{
	static unsigned uEnd=0;
	static unsigned uStart=0;
	static char cRetVal[512];
	
	uStart=uEnd;
	while(cTextArea[uEnd++])
	{
		if(cTextArea[uEnd]=='\n' || cTextArea[uEnd]=='\r' || cTextArea[uEnd]==0
				|| cTextArea[uEnd]==10 || cTextArea[uEnd]==13 )
		{
			if(cTextArea[uEnd]==0)
				break;

			cTextArea[uEnd]=0;
			sprintf(cRetVal,"%.511s",cTextArea+uStart);

			if(cRetVal[0]=='\n' || cRetVal[0]==13)
			{
				uStart=uEnd=0;
				return("");
			}

			if(cTextArea[uEnd+1]==10)
				uEnd+=2;
			else
				uEnd++;

			return(cRetVal);
		}
	}

	if(uStart!=uEnd)
	{
		sprintf(cRetVal,"%.511s",cTextArea+uStart);
		return(cRetVal);
	}

	uStart=uEnd=0;
	return("");

}//char *ParseTextAreaLines(char *cTextArea)


char *ParseTextAreaLines2(char *cTextArea)
{
	static unsigned uEnd=0;
	static unsigned uStart=0;
	static char cRetVal[512];
	
	uStart=uEnd;
	while(cTextArea[uEnd++])
	{
		if(cTextArea[uEnd]=='\n' || cTextArea[uEnd]=='\r' || cTextArea[uEnd]==0
				|| cTextArea[uEnd]==10 || cTextArea[uEnd]==13 )
		{
			if(cTextArea[uEnd]==0)
				break;

			cTextArea[uEnd]=0;
			sprintf(cRetVal,"%.511s",cTextArea+uStart);

			if(cRetVal[0]=='\n' || cRetVal[0]==13)
			{
				uStart=uEnd=0;
				return("");
			}

			if(cTextArea[uEnd+1]==10)
				uEnd+=2;
			else
				uEnd++;

			return(cRetVal);
		}
	}

	if(uStart!=uEnd)
	{
		sprintf(cRetVal,"%.511s",cTextArea+uStart);
		return(cRetVal);
	}

	uStart=uEnd=0;
	return("");
}


void htmlInZone(void)
{
	char cZone[256]={"ERROR"};
	char cImportSource[256]={""};
	char *cp;
	MYSQL_RES *res;
	unsigned uCount=0;
	unsigned uFoundCount=0;
	unsigned uOnlyCheck=0;

	printf("Content-type: text/plain\n\n");
	printf("htmlInZone() start\n");
	while(cZone[0])
	{

		sprintf(cZone,"%.250s",ParseTextAreaLines(cMassList));
		if(cZone[0]==0) break;

		if(!strncmp(cZone,"#ImportSource=",strlen("#ImportSource=")))
		{
			char *cp;

			sprintf(cImportSource,"%.255s",cZone+strlen("#ImportSource="));
			if((cp=strchr(cImportSource,';'))) *cp=0;
			printf("ImportSource=%s\n",cImportSource);
		}
		if(!strncmp(cZone,"#OnlyCheck",strlen("#OnlyCheck")))
		{
			uOnlyCheck=1;
			printf("OnlyCheck is on\n");
		}


		if(cZone[0]=='#') continue;

		uCount++;
		//Allow tabs, commas or spaces after zone
		if((cp=strchr(cZone,'\t'))) *cp=0;
		if((cp=strchr(cZone,' '))) *cp=0;
		if((cp=strchr(cZone,','))) *cp=0;

		if(cImportSource[0] && !uOnlyCheck)
		{
			if(uDDClient)
				sprintf(gcQuery,"SELECT tResource.cName,tResource.uTTL,tRRType.cLabel,"
						"tResource.cParam1,tResource.cParam2 FROM tResource,tZone,tRRType "
						"WHERE tResource.uZone=tZone.uZone AND tResource.uRRType=tRRType.uRRType "
						"AND tZone.cZone='%s' AND tZone.uOwner=%u",cZone,uDDClient);
			else
				sprintf(gcQuery,"SELECT tResource.cName,tResource.uTTL,tRRType.cLabel,tResource.cParam1,"
						"tResource.cParam2 FROM tResource,tZone,tRRType WHERE"
						" tResource.uZone=tZone.uZone "
						"AND tResource.uRRType=tRRType.uRRType AND tZone.cZone='%s'",cZone);
		}
		else
		{
			if(uDDClient)
				sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uOwner=%u ORDER BY cZone",
						cZone,uDDClient);
			else
				sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' ORDER BY cZone",cZone);
		}
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)==0)
		{
			if(uDDClient)
				printf("%s not found with owner \"%s\"\n",cZone,cCustomerDropDown);
			else
				printf("%s not found\n",cZone);
		}
		else
		{
			if(cImportSource[0])
			{
				char cPath[512]={""};
				FILE *fp;

				sprintf(cPath,"%s/%.254s",cImportSource,cZone);
				if((fp=fopen(cPath,"r"))!=NULL)
				{
					if(!uOnlyCheck)
					{
						MYSQL_ROW field;

						printf("%s\n",cPath);
						while(fgets(gcQuery,254,fp)!=NULL)
							printf("%s",gcQuery);
						printf("\nFrom db\n");

						while((field=mysql_fetch_row(res)))
						{
							printf("%s %s %s %s %s\n",
							field[0],
							field[1],
							field[2],
							field[3],
							field[4]
							);
						}
						printf("\n\n");
					}
					fclose(fp);
				}
				else
				{
					printf("%s not found\n",cPath);
				}
			}
			uFoundCount++;
		}
                mysql_free_result(res);
	}
	printf("htmlInZone() end\n");
	if(!uDDClient)
		printf("%u of %u found. %u not found.\n",uFoundCount,uCount,uCount-uFoundCount);
	else
		printf("%u of %u found. %u not found for \"%s\".\n",
				uFoundCount,uCount,uCount-uFoundCount,cCustomerDropDown);
	exit(0);

}//void htmlInZone(void)


void htmlSecondaryServiceOnly(void)
{
	char cZone[256]={"ERROR"};
	char *cp;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uNSSet=0;
	unsigned uNSSetFromTextArea=0;
	char cNSSet[33]={""};
	char cMasterIPs[100]={""};//10.0.0.1; 192.168.10.23; format
	unsigned uFoundCount=0,uCount=0,uAddCount=0;
	unsigned uOnExistUpdate=0;

	printf("Content-type: text/plain\n\n");
	printf("htmlSecondaryServiceOnly() start\n");
	while(cZone[0])
	{

		sprintf(cZone,"%.250s",ParseTextAreaLines(cMassList));
		if(cZone[0]==0) break;
		uCount++;
		//Allow tabs, commas or single spaces after zone and before
		//One of three optional uNSSet IDs
		if((cp=strchr(cZone,'\t')))
			*cp=0;
		else if((cp=strchr(cZone,' ')))
			*cp=0;
		else if((cp=strchr(cZone,',')))
			*cp=0;

		if(cp==NULL)
		{
			if(!strcmp(cZone,";update"))
			{
				printf("uOnExistUpdate=1\n");
				uOnExistUpdate=1;
				continue;
			}
			printf("Bad format: (%s)\n",cZone);
			continue;
		}

		//Allow multiple options
		sprintf(cNSSet,"%.32s",cp+1);
		sprintf(cMasterIPs,"%.99s",cp+1);
		sscanf(cp+1,"%u",&uNSSetFromTextArea);

		//Debug only
		//printf("cNSSet=(%s)\n",cNSSet);

		//First check tZone
		sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",cZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res))
		{
			printf("%s already exists\n",cZone);
			if(!uOnExistUpdate)
				continue;

		}
                mysql_free_result(res);
		uFoundCount++;

		//NSs selection
		sprintf(gcQuery,"SELECT uNSSet,cMasterIPs FROM tNSSet WHERE cLabel='%s'",
				cNSSet);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			sscanf(field[0],"%u",&uNSSet);
			sprintf(cMasterIPs,"%.99s",field[1]);
		}
                mysql_free_result(res);

		if(uNSSet) goto NSSetSelected;

		sprintf(gcQuery,"SELECT uNSSet FROM tNSSet WHERE cMasterIPs='%s'",cMasterIPs);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
			sscanf(field[0],"%u",&uNSSet);
                mysql_free_result(res);

		if(uNSSet) goto NSSetSelected;

		sprintf(gcQuery,"SELECT uNSSet,cMasterIPs FROM tNSSet WHERE uNSSet=%u",uNSSetFromTextArea);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			sscanf(field[0],"%u",&uNSSet);
			sprintf(cMasterIPs,"%.99s",field[1]);
		}
                mysql_free_result(res);

		if(uNSSet)
		{
			goto NSSetSelected;
		}
		else
		{
			printf("Could not determine a uNSSet for %s\n",cZone);
			continue;
		}

NSSetSelected:
		if(!uDDClient) uDDClient=guCompany;
		if(uOnExistUpdate)
			sprintf(gcQuery,"UPDATE tZone SET uNSSet=%u,uSecondaryOnly=1,uModBy=1,"
					"uModDate=UNIX_TIMESTAMP(NOW()) WHERE cZone='%s' AND uView=2 "
					"AND uSecondaryOnly=0 AND uOwner=%u",
					uNSSet,
					cZone,uDDClient);
		else
			sprintf(gcQuery,"INSERT INTO tZone SET cZone='%s',uNSSet=%u,uView=2,"
					"uSecondaryOnly=1,uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					cZone,
					uNSSet,
					uDDClient);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		if(uOnExistUpdate)
		{
			if(mysql_affected_rows(&gMysql))
			{
				SubmitJob("Modify",uNSSet,cZone,0,0);
				printf("Updated %s and created modify job\n",cZone);
			}
		}
		else
		{
			SubmitJob("New",uNSSet,cZone,0,0);
			printf("Added %s and created new job\n",cZone);
		}
		uAddCount++;

	}
	printf("htmlSecondaryServiceOnly() end\n");
	printf("%u of %u lines in tZone. %u added/modified.\n",uFoundCount,uCount,uAddCount);
	exit(0);

}//void htmlSecondaryServiceOnly(void)


void htmlMassUpdate(void)
{
	char cJobName[32]={"Modify"};
	char cZone[256]={"ERROR"};
	char cNSSet[100]={""};
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uNSSet=0;
	unsigned uAssignedNSSet=0;
	time_t luClock;
	unsigned uFoundCount=0,uCount=0,uUpdateCount=0;
	unsigned uZone=0;
	unsigned uView=1;//default view internal usually


	printf("Content-type: text/plain\n\n");
	printf("htmlMassUpdate() start\n");
	while(cZone[0])
	{

		sprintf(cZone,"%.250s",ParseTextAreaLines(cMassList));
		if(cZone[0]==0)
			break;
		//These ignore lines are compatible with ParseTextAreaLines()
		//To be able to ignore empty lines we need to change ParseTextAreaLines()
		if(cZone[0]=='#' || cZone[0]==' ' || cZone[0]==';')
			continue;

		char *cp;
		if((cp=strstr(cZone,"cNSSet=")))
		{
			char *cp2;
			if((cp2=strchr(cp+strlen("cNSSet="),';')))
			{
				*cp2=0;
				sprintf(cNSSet,"%.99s",cp+strlen("cNSSet="));
				//NSs selection
				sprintf(gcQuery,"SELECT uNSSet,cMasterIPs FROM tNSSet WHERE cLabel='%s'",
						cNSSet);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
				{
					printf("%s\n",mysql_error(&gMysql));
					exit(0);
				}
				res=mysql_store_result(&gMysql);
				if((field=mysql_fetch_row(res)))
					sscanf(field[0],"%u",&uAssignedNSSet);
       		         	mysql_free_result(res);

				if(uAssignedNSSet)
					printf("cNSSet assigned:%s\n",cNSSet);
				else
					printf("cNSSet not assigned:%s\n",cNSSet);
				continue;
			}
		}
		else if((cp=strstr(cZone,"uView=")))
		{
			char *cp2;
			if((cp2=strchr(cp+6,';')))
			{
				*cp2=0;
				sscanf(cp+6,"%u",&uView);
				//NSs selection
				sprintf(gcQuery,"SELECT uView FROM tView WHERE uView=%u",uView);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
				{
					printf("%s\n",mysql_error(&gMysql));
					exit(0);
				}
				res=mysql_store_result(&gMysql);
				uView=0;
				if((field=mysql_fetch_row(res)))
					sscanf(field[0],"%u",&uView);
       		         	mysql_free_result(res);

				if(uView)
					printf("uView assigned:%u\n",uView);
				continue;
			}
		}
		else if(1)
		{
			uCount++;
		}

		//First check tZone
		if(guLoginClient==1)
			sprintf(gcQuery,"SELECT uNSSet,uZone FROM tZone WHERE cZone='%s' AND uView=%u"
					,cZone,uView);
		else
			sprintf(gcQuery,"SELECT uNSSet,uZone FROM tZone WHERE cZone='%s'  AND uView=%u AND uOwner=%u"
					,cZone,uView,guCompany);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)==0)
		{
			printf("%s/uView=%u not found. Ignoring this zone for update\n",cZone,uView);
			continue;
		}
		else if((field=mysql_fetch_row(res)))
		{
			sscanf(field[0],"%u",&uNSSet);
			sscanf(field[1],"%u",&uZone);
		}
                mysql_free_result(res);
		uFoundCount++;


		//NSs selection
		sprintf(gcQuery,"SELECT uNSSet,cMasterIPs FROM tNSSet WHERE uNSSet=%u",uNSSet);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
			sscanf(field[0],"%u",&uNSSet);
                mysql_free_result(res);

		if(!uNSSet)
		{
			printf("Could not determine a uNSSet for %s. Ignoring this zone for update\n",cZone);
			continue;
		}


		if(cNSSet[0] && uNSSet!=uAssignedNSSet)
		{
			//All views!
			sprintf(gcQuery,"UPDATE tZone SET uNSSet=%u WHERE cZone='%s'",
				uAssignedNSSet,cZone);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				printf("%s\n",mysql_error(&gMysql));
				exit(0);
			}
			printf("NS set changed for this zone\n");
			uNSSet=uAssignedNSSet;
			sprintf(cJobName,"Modify New");
		}
		else if(cNSSet[0])
		{
			printf("NS set not changed ignoring this zone for update\n");
			continue;
		}

		UpdateSerialNum(uZone);
		if(uDDClient)
		{
			//All views!
			sprintf(gcQuery,"UPDATE tZone SET uOwner=%u WHERE cZone='%s'",
				uDDClient,cZone);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				printf("%s\n",mysql_error(&gMysql));
				exit(0);
			}

			//All views!
			//Updates RRs also
			sprintf(gcQuery,"UPDATE tResource,tZone SET tResource.uOwner=%u"
					" WHERE tResource.uZone=tZone.uZone AND tZone.cZone='%s'",
							uDDClient,cZone);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				printf("%s\n",mysql_error(&gMysql));
				exit(0);
			}
			printf("uOwner changed for this zone\n");
		}
		time(&luClock);
		SubmitJob(cJobName,uNSSet,cZone,0,luClock+300);//Run in 5 mins
		printf("Serial number updated %s and new job created\n",cZone);
		uUpdateCount++;

	}
	printf("htmlMassUpdate() end\n");
	printf("%u of %u found in tZone. %u updated.\n",uFoundCount,uCount,uUpdateCount);
	exit(0);


}//void htmlMassUpdate(void)


void htmlMassDelete(void)
{
	char cZone[256]={"ERROR"};
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uNSSet=0;
	unsigned uZone=0;
	unsigned uFoundCount=0,uCount=0,uUpdateCount=0;
	time_t luClock;

	time(&luClock);

	printf("Content-type: text/plain\n\n");
	printf("htmlMassDelete() start\n");
	while(cZone[0])
	{

		sprintf(cZone,"%.250s",ParseTextAreaLines(cMassList));
		if(cZone[0]==0) break;
		uCount++;

		//First check tZone
		if(uDDClient)
			sprintf(gcQuery,"SELECT uZone,uNSSet FROM tZone WHERE cZone='%s' AND uOwner=%u",
							cZone,uDDClient);
		else
			sprintf(gcQuery,"SELECT uZone,uNSSet FROM tZone WHERE cZone='%s' AND uOwner=%u",
							cZone,guCompany);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)==0)
		{
			printf("%s not found (%u)\n",cZone,uDDClient);
			continue;
		}
		else if((field=mysql_fetch_row(res)))
		{
			sscanf(field[0],"%u",&uZone);
			sscanf(field[1],"%u",&uNSSet);
		}
                mysql_free_result(res);
		uFoundCount++;

		//NSs selection
		sprintf(gcQuery,"SELECT uNSSet FROM tNSSet WHERE uNSSet=%u",uNSSet);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
			sscanf(field[0],"%u",&uNSSet);
                mysql_free_result(res);

		if(!uNSSet)
		{
			printf("Could not determine a uNSSet for %s\n",cZone);
			continue;
		}

		//Delete or Del?
		SubmitJob("Delete",uNSSet,cZone,0,luClock+600);
		printf("Delete job created for %s\n",cZone);

		sprintf(gcQuery,"DELETE FROM tZone WHERE uZone=%u",uZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		sprintf(gcQuery,"DELETE FROM tResource WHERE uZone=%u",uZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		printf("%s deleted (%u)\n",cZone,uDDClient);
		uUpdateCount++;

	}
	printf("htmlMassDelete() end\n");
	printf("%u of %u found in tZone. %u deleted.\n",uFoundCount,uCount,uUpdateCount);
	exit(0);


}//void htmlMassDelete(void)


void htmlCustomerZones(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uCount=0;
	FILE *fp;
	char cPath[512]={""};

	printf("Content-type: text/plain\n\n");
	printf("htmlCustomerZones() start\n");

	if(!uDDClient) uDDClient=guLoginClient;
	//Hardwired external SELECT
	sprintf(gcQuery,"SELECT cZone FROM tZone WHERE uOwner=%u AND uView=2 ORDER BY cZone",
		uDDClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(0);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		printf("%s\n",field[0]);
		uCount++;

		sprintf(cPath,"/usr/local/idns/named.d/master/external/%c/%s",
				field[0][0],field[0]);
		printf("%s:\n\n",cPath);
		if((fp=fopen(cPath,"r"))!=NULL)
		{
			while(fgets(gcQuery,254,fp)!=NULL)
				printf("%s",gcQuery);
			fclose(fp);
			printf("\n");
		}
	}
	mysql_free_result(res);
	printf("htmlCustomerZones() end\n");
	printf("%u found. External view only.\n",uCount);
	exit(0);

}//void htmlCustomerZones(void)


void htmlZoneList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uCount=0;
	char cZone[256]={"Error"};

	printf("Content-type: text/plain\n\n");
	printf("htmlZoneList() start\n");

	while(cZone[0])
	{

		sprintf(cZone,"%.250s",ParseTextAreaLines(cMassList));
		if(cZone[0]==0) break;
		uCount++;

		printf("\n%s\n",cZone);
		if(uDDClient)
		sprintf(gcQuery,"SELECT DISTINCT cZone FROM tZone WHERE cZone LIKE '%s'"
				" AND uOwner=%u ORDER BY cZone",cZone,uDDClient);
		else
		sprintf(gcQuery,"SELECT DISTINCT cZone FROM tZone WHERE cZone LIKE '%s' ORDER BY cZone",
			cZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		while((field=mysql_fetch_row(res)))
		{
			printf("%s\n",field[0]);
			uCount++;
		}
		mysql_free_result(res);
	}

	printf("uCount:%u\n",uCount);
	printf("htmlZoneList() end\n");
	exit(0);

}//void htmlZoneList(void)


void CustomerDropDown(unsigned uSelector)
{
        register int i,n;
        MYSQL_RES *mysqlRes;         
        MYSQL_ROW mysqlField;
	
	if(guLoginClient==1)
	        sprintf(gcQuery,"SELECT uClient,cLabel FROM "TCLIENT" WHERE cCode='Organization' OR SUBSTR(cCode,1,4)='COMP' ORDER BY cLabel");
	else
		sprintf(gcQuery,"SELECT uClient,cLabel FROM "TCLIENT" WHERE (uClient=%u OR uOwner=%u) AND"
				" (cCode='Organization' OR SUBSTR(cCode,1,4)='COMP') ",
				guCompany,guCompany);

        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
        {
                printf("%s",mysql_error(&gMysql));
                return;
        }
	mysqlRes=mysql_store_result(&gMysql);
	i=mysql_num_rows(mysqlRes);

        if(i>0)
        {
                printf("<select name=cCustomerDropDown>\n");

                //Default no selection
                printf("<option title='No selection'>---</option>\n");

                for(n=0;n<i;n++)
                {
                        int unsigned field0=0;

                        mysqlField=mysql_fetch_row(mysqlRes);
                        sscanf(mysqlField[0],"%u",&field0);

                        if(uSelector != field0)
                        {
                             printf("<option>%s</option>\n",mysqlField[1]);
                        }
                        else
                        {
                             printf("<option selected>%s</option>\n",mysqlField[1]);
                        }
                }
        }
        else
        {
		printf("<select name=cCustomerDropDown><option title='No selection'>---"
				"</option></select>\n");
        }
        printf("</select>\n");

}//CustomerDropDown()


//This is a custom one time fix
void htmlMassResourceFix(void)
{
	char cNamePart[256]={"ERROR"};
	char cZone[256]={"ERROR"};
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uNSSet=0;
	unsigned uZone=0;
	unsigned uResource=0;
	unsigned uFoundCount=0,uCount=0,uUpdateCount=0;

	printf("Content-type: text/plain\n\n");
	printf("htmlMassResourceFix() start\n");
	while(cZone[0])
	{

		sprintf(cNamePart,"%.250s",ParseTextAreaLines(cMassList));
		if(cNamePart[0]==0) break;
		uCount++;

		//Fix nasty import bug
		//example ns01lhr1.uk.prv.asc-services.net. should be
		//ns01lhr1.uk.prv.asc-services.net.
		//We have a list from import log of lines (thank goodness)
		//Like this (for this example):
		//lhr1.uk.prv.asc-services.net.
		//That string is cNamePart
		//First check tResource against the lines we need to fix
		sprintf(gcQuery,"SELECT tZone.uZone,tZone.uNSSet,tResource.uResource,"
				"tResource.cName,tZone.cZone FROM tResource,tZone WHERE "
				"tResource.uZone=tZone.uZone AND tResource.cName LIKE '%%%s' "
				"AND tResource.cName NOT LIKE '%%.%s' AND tZone.uView=2 AND "
				"tZone.uSecondaryOnly=0 AND tZone.uOwner=%u",
					cNamePart,cNamePart,guCompany);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)==0)
			printf("%s not found\n",cNamePart);
		while((field=mysql_fetch_row(res)))
		{
			sscanf(field[0],"%u",&uZone);
			sscanf(field[1],"%u",&uNSSet);
			sscanf(field[2],"%u",&uResource);
			sprintf(cZone,"%.255s",field[4]);
			//Debug only
			printf("Will fix this tResource.cName=(%s) using (%s). uResource=%u of cZone=(%s)\n",
					field[3],cNamePart,uResource,cZone);
			uFoundCount++;

			//If the original cName and the $ORIGIN change part are the same skip
			if(!strcmp(cNamePart,field[3]))
				continue;

			//2-. Fix section
			//Ex. SELECT CONCAT(SUBSTR(cName,1,INSTR(cName,'lhr1.uk.asc-services.net.')-1),
			//'.lhr1.uk.asc-services.net.') FROM tResource WHERE uResource=98526;
			sprintf(gcQuery,"UPDATE tResource SET cName=CONCAT(SUBSTR(cName,1,(INSTR(cName,'%s')-1)),'.%s')"
					"WHERE uResource=%u AND uOwner=%u",
							cNamePart,cNamePart,uResource,guCompany);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				printf("%s\n",mysql_error(&gMysql));
				exit(0);
			}
			if(mysql_affected_rows(&gMysql)==0)
			{
				printf("Did not fix %s\n",cNamePart);
				continue;
			}

			UpdateSerialNum(uZone);
			SubmitJob("Modify",1,cZone,0,0);

			printf("%s fixed and mod job created\n",cNamePart);
			uUpdateCount++;
		}//while uResources
                mysql_free_result(res);
	}
	printf("htmlMassResourceFix() end\n");
	printf("%u of %u found in tResource. %u fixed.\n",uFoundCount,uCount,uUpdateCount);
	exit(0);


}//void htmlMassResourceFix(void)


void htmlMassResourceImport(void)
{
	char cLine[512]={"ERROR"};
	char cZone[256]={"ERROR"};
	char *cp;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uWipeRRs=0;
	unsigned uAddNewZones=0;
	unsigned uNewZone=0;
	unsigned uCloneZone=0;
	unsigned uDebug=0;
	unsigned uZone;
	unsigned uView=2;//external
	unsigned uZoneOwner;
	unsigned uNSSet;
	unsigned uZoneCount=0,uZoneFoundCount=0,uResourceCount=0,uImportCount=0;
	unsigned uOnlyOncePerZone;

	printf("Content-type: text/plain\n\n");
	printf("htmlMassResourceImport() start (uDDClient=%u)\n",uDDClient);
	while(cZone[0])
	{

		sprintf(cLine,"%.255s",ParseTextAreaLines(cMassList));
		//ParseTextAreaLines() required break;
		if(cLine[0]==0) break;

		//Comments ignored, note that empty lines cause above break.
		if(cLine[0]=='#') continue;
		if(cLine[0]==';') continue;

		//Debug only
		printf("cLine=(%s)\n",cLine);

		//Debug only line everything after this will
		//have no jobs created. Later add uDebug to ProcessRRLine()
		if(!strncmp(cLine,"cMode=debug;",12))
		{
			uDebug=1;
			printf("uDebug=1\n");
			continue;
		}

		if(!strncmp(cLine,"uView=",6))
		{
			if((cp=strchr(cLine,';')))
				*cp=0;
			sscanf(cLine+6,"%u",&uView);
			if(uView>2 || uView<1)
				uView=2;
			printf("uView=%u\n",uView);
			continue;
		}

		if(!strncmp(cLine,"uWipeRRs",8))
		{
			uWipeRRs=1;
			printf("uWipeRRs on\n");
			continue;
		}

		if(!strncmp(cLine,"uAddNewZones",12))
		{
			uAddNewZones=1;
			printf("uAddNewZones on\n");
			continue;
		}

		if(!strncmp(cLine,"uCloneZone=",11))
		{
			if((cp=strchr(cLine,';')))
				*cp=0;
			sscanf(cLine+11,"%u",&uCloneZone);
			printf("uCloneZone=%u\n",uCloneZone);
			continue;
		}

		//New zone
		if(!strncmp(cLine,"cZone=",6))
		{
			uOnlyOncePerZone=0;
			uZone=0;
			uNewZone=0;
			uZoneOwner=0;
			uNSSet=0;
			uZoneCount++;

			if((cp=strchr(cLine,';')))
				*cp=0;
			sprintf(cZone,"%.255s",cLine+6);
			//Debug only
			printf("cZone=(%s)\n",cZone);

			//First check tZone
			sprintf(gcQuery,"SELECT uZone,uNSSet,uOwner FROM tZone WHERE cZone='%s' AND"
						" uSecondaryOnly=0 AND uView=%u",cZone,uView);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				printf("%s\n",mysql_error(&gMysql));
				exit(0);
			}
			res=mysql_store_result(&gMysql);
			if((field=mysql_fetch_row(res)))
			{
				sscanf(field[0],"%u",&uZone);
				sscanf(field[1],"%u",&uNSSet);
				sscanf(field[2],"%u",&uZoneOwner);
			}
			else if(uAddNewZones && uCloneZone)
			{
				printf("New zone created %s based on %u\n",cZone,uCloneZone);
				uNewZone=1;
			}
			mysql_free_result(res);
			if(!uZone || !uZoneOwner || !uNSSet)
			{
				printf("Valid %s not found. Skipping.\n",cZone);
				continue;

			}

			if(uWipeRRs && uZone && !uNewZone)
			{
				sprintf(gcQuery,"DELETE FROM tResource WHERE uZone=%u",uZone);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
				{
					printf("%s\n",mysql_error(&gMysql));
					exit(0);
				}
			}

			uOnlyOncePerZone=1;
			uZoneFoundCount++;
		}
		else
		{
			//A resource candidate line
			uResourceCount++;
			//If we have no defined zone keep on going.
			if(!uZone) continue;
			//If select is used then the RRs are owned by that customer.
			if(uDDClient) uZoneOwner=uDDClient;
			ProcessRRLine(cLine,cZone,uZone,uZoneOwner,uNSSet,guLoginClient,
				"htmlMassResourceImport()");
			if(mysql_affected_rows(&gMysql)==1)
			{
				uImportCount++;
/*
				if(uOnlyOncePerZone && !uDebug)
				{
					time_t luClock;

					//Submit job for first RR. Time for now + 5 minutes
					//This should allow for many more RRs to be added
					//here without complicating the code. A KISS hack?
					UpdateSerialNum(uZone);
					time(&luClock);
					luClock+=300;
					SubmitJob("Modify",uNSSet,cZone,0,luClock);
					uOnlyOncePerZone=0;
				}
*/
			}
		}

	}
	printf("htmlMassResourceImport() end\n");
	printf("%u of %u tZone.cZone found. %u resource lines found %u imported.\n",
		uZoneFoundCount,uZoneCount,uResourceCount,uImportCount);
	exit(0);

}//void htmlMassResourceImport(void)


void htmlZonesFromBlocks(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	MYSQL_RES *res2;
	MYSQL_ROW field2;
	unsigned a,b,c,d,e;
	unsigned uCount=0,uFound=0,uNotFound=0;
	char cZone[100];
	char cPrevZone[100]={""};

	printf("Content-type: text/plain\n\n");
	printf("htmlZonesFromBlocks() start\n");

	if(uDDClient)
	{
		printf("For tClient.cLabel=%s(%u)\n",
			ForeignKey(TCLIENT,"cLabel",uDDClient),uDDClient);
		sprintf(gcQuery,"SELECT DISTINCT cLabel FROM tBlock WHERE uOwner=%u ORDER BY cLabel",
			uDDClient);
	}
	else
	{
		sprintf(gcQuery,"SELECT DISTINCT cLabel FROM tBlock ORDER BY cLabel");
		printf("For all uOwners\n");
	}
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(0);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{

		printf("field[0]=%s\n",field[0]);

		a=0;b=0;c=0;d=0;e=0;

		sscanf(field[0],"%u.%u.%u.%u/%u",&a,&b,&c,&d,&e);
		if(e<24)
			printf("Error: Must expand further: Block larger than class C\n");
		sprintf(cZone,"%u.%u.%u.in-addr.arpa",c,b,a);
		if(strcmp(cZone,cPrevZone))
		{
			sprintf(cPrevZone,"%.99s",cZone);
		}
		else
		{
			continue;
		}
		uCount++;
		sprintf(gcQuery,"SELECT uZone,uView,uSecondaryOnly FROM tZone WHERE cZone='%s'",
				cZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res2=mysql_store_result(&gMysql);
		if(mysql_num_rows(res2)>0)
		{
			while((field2=mysql_fetch_row(res2)))
			{
				uFound++;
				printf("cBlock=%s;cZone=%s;uView=%s;uSecondaryOnly=%s;\n",
					field[0],cZone,field2[1],field2[2]);
			}
		}
		else
		{
			uNotFound++;
			printf("cBlock=%s;cZone=%s; Not found in tZone.\n",field[0],cZone);
		}
		printf("\n");
		mysql_free_result(res2);
	}
	mysql_free_result(res);
	printf("htmlZonesFromBlocks() end\n");
	printf("%u zones found of %u distinct zones made from blocks. %u not found.\n",uFound,uCount,uNotFound);
	exit(0);

}//void htmlZonesFromBlocks(void)


void htmlMassCopyToOtherViews(void)
{
	char cLine[512]={"Error"};
	char cZone[512]={""};
	char cOptions[512]={""};
	unsigned uZone=0;
	unsigned uTargetZone=0;
	unsigned uView=0;
	char *cp,*cp2;
	MYSQL_RES *res;
	MYSQL_RES *res2;
	MYSQL_ROW field;
	MYSQL_ROW field2;
	unsigned uCount=0;
	unsigned uCopied=0;
	unsigned uOnlyOnce=1;
	unsigned uRRCopied=0;
	unsigned uFoundCount=0;
	time_t luClock;


	printf("Content-type: text/plain\n\n");
	printf("htmlMassCopyToOtherViews() start\n");
	while(cLine[0])
	{
		sprintf(cLine,"%.511s",ParseTextAreaLines(cMassList));
		if(cLine[0]==0) break;
		uCount++;

		if((cp=strstr(cLine,"cZone=")))
		{
			if((cp2=strchr(cp+6,';')))
			{
				*cp2=0;
				sprintf(cZone,"%.255s",cp+6);
				*cp2=';';
			}
		}
		if((cp=strstr(cLine,"uView=")))
		{
			if((cp2=strchr(cp+6,';')))
			{
				*cp2=0;
				sscanf(cp+6,"%u",&uView);
				*cp2=';';
			}
		}

		if(!cZone[0] || uView==0)
		{
			printf("Error: Incorrect line format (%s)\n",cLine);
			continue;
		}

		sprintf(gcQuery,"SELECT uZone,tZone.uNSSet,cHostMaster,"
				"uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
				"uMailServers,cMainAddress,uRegistrar,uSecondaryOnly,"
				"tNSSet.cMasterIPs,cOptions,uOwner FROM "
				"tZone,tNSSet WHERE cZone='%s' AND uView=%u "
				"AND tZone.uNSSet=tNSSet.uNSSet AND tZone.uOwner=%u"
				,cZone,uView,guCompany);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if(mysql_num_rows(res)==0)
		{
			printf("%s/(uView=%u) not found\n",cZone,uView);
		}
		else
		{
			uFoundCount++;
			if((field=mysql_fetch_row(res)))
			{
				sscanf(field[0],"%u",&uZone);
				sscanf(field[1],"%u",&uNSSet);
				sprintf(cHostmaster,"%.99s",field[2]);
				sscanf(field[3],"%u",&uSerial);
				sscanf(field[4],"%u",&uExpire);
				sscanf(field[5],"%u",&uRefresh);
				sscanf(field[6],"%u",&uTTL);
				sscanf(field[7],"%u",&uRetry);
				sscanf(field[8],"%u",&uZoneTTL);
				sscanf(field[9],"%u",&uMailServers);
				sprintf(cMainAddress,"%.99s",field[10]);
				sscanf(field[11],"%u",&uRegistrar);
				sscanf(field[12],"%u",&uSecondaryOnly);
				sprintf(cMasterIPs,"%.99s",field[13]);
				sprintf(cOptions,"%.512s",field[14]);
				sscanf(field[15],"%u",&uOwner);
			}
		}
                mysql_free_result(res);

		if(uZone==0)
		{
			printf("Error: Unexpected uZone value for (%s)\n",cLine);
			continue;
		}

		sprintf(gcQuery,"SELECT uView FROM tView WHERE uView!=%u",uView);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		while((field=mysql_fetch_row(res)))
		{
			uOnlyOnce=1;

			sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=%s",
				cZone,field[0]);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				printf("%s\n",mysql_error(&gMysql));
				exit(0);
			}
			res2=mysql_store_result(&gMysql);
			if((field2=mysql_fetch_row(res2)))
			{
				sscanf(field2[0],"%u",&uTargetZone);
				mysql_free_result(res2);
				goto DoNotInsertZone;
			}
			mysql_free_result(res2);

			sprintf(gcQuery,"INSERT INTO tZone SET cZone='%s',uView=%s,uNSSet=%u,"
					"cHostmaster='%s',uSerial=%u,uExpire=%u,uRefresh=%u,uTTL=%u,"
					"uRetry=%u,uZoneTTL=%u,uMailServers=%u,cMainAddress='%s',"
					"uRegistrar=%u,uSecondaryOnly=%u,cOptions='%.512s',uOwner=%u,"
					"uCreatedDate=UNIX_TIMESTAMP(NOW())",
					cZone
					,field[0]
					,uNSSet
					,cHostmaster
					,uSerial
					,uExpire
					,uRefresh,uTTL
					,uRetry
					,uZoneTTL
					,uMailServers
					,cMainAddress
					,uRegistrar
					,uSecondaryOnly
					,cOptions
					,uOwner);
			//printf("%s\n",gcQuery);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				printf("%s\n",mysql_error(&gMysql));
				exit(0);
			}
			if((uTargetZone=mysql_insert_id(&gMysql)))
			{
				UpdateSerialNum(uTargetZone);
				time(&luClock);
				luClock+=500;
				SubmitJob("New",uNSSet,cZone,0,luClock);
				uCopied++;
			}
DoNotInsertZone:
			//debug only
			//printf("DoNotInsertZone: uZone=%u uTargetZone=%u\n",uZone,uTargetZone);
			sprintf(gcQuery,"SELECT cName,uTTL,uRRType,cParam1,cParam2,uOwner FROM tResource WHERE uZone=%u",
					uZone);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				printf("%s\n",mysql_error(&gMysql));
				exit(0);
			}
			res2=mysql_store_result(&gMysql);
			while((field2=mysql_fetch_row(res2)))
			{
				sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%s',"
						"uTTL=%s,uRRType=%s,cParam1='%s',cParam2='%s',"
						"cComment='htmlMassCopyToOtherViews()',uOwner=%s,"
						"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
						uTargetZone,
						field2[0],
						field2[1],
						field2[2],
						field2[3],
						field2[4],
						field2[5]);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
				{
					printf("%s\n",mysql_error(&gMysql));
					exit(0);
				}
				if(mysql_affected_rows(&gMysql) && uOnlyOnce)
				{
					UpdateSerialNum(uTargetZone);
					time(&luClock);
					luClock+=600;
					SubmitJob("Mod",uNSSet,cZone,0,luClock);
					uRRCopied++;
					uOnlyOnce=0;
				}
			}
			mysql_free_result(res2);

		}
                mysql_free_result(res);
	}
	printf("htmlMassCopyToOtherViews() end\n");
	printf("%u of %u found. %u not found. %u copied, %u RR copied\n",
				uFoundCount,uCount,uCount-uFoundCount,uCopied,uRRCopied);
	exit(0);

}//void htmlMassCopyToOtherViews(void)


void htmlSecondaryServiceCleanup(void)
{
	printf("Content-type: text/plain\n\n");
	printf("htmlSecondaryServiceCleanup() start\n");
	printf("htmlSecondaryServiceCleanup() end\n");
	exit(0);

}//void htmlSecondaryServiceCleanup(void)


//input format came from somehting similar to this on actual zone files
//grep PTR import1/*in-addr.arpa* | cut -f 2 -d / 
void htmlMassPTRCheck(void)
{
	char cLine[512]={"ERROR"};
	char cQuery[512];
	char *cp;
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uFound=0,uTotal=0,uFix=0,uFirstLine=1,uIgnorecParam1=0;

	printf("Content-type: text/plain\n\n");
	printf("htmlMassPTRCheck() start\n");
	while(cLine[0])
	{
		char cArpaZone[256]={"ERROR"};
		char cFQDN[256]={"ERROR"};
		unsigned uLastOctet=0;
		unsigned uZone=0;

		sprintf(cLine,"%.255s",ParseTextAreaLines(cMassList));
		//ParseTextAreaLines() required break;
		if(cLine[0]==0) break;

		//Comments ignore
		if(cLine[0]=='#') continue;
		if(cLine[0]==';') continue;

		if(uFirstLine)
		{
			if(strstr(cLine,"Ignore-cParam1"))
			{
				if(strstr(cLine,"Fix"))
				{
					uFix=1;
					printf("uFix set.\n");
				}
				uIgnorecParam1=1;
				printf("Ignore-cParam1 set.\n");
				continue;
			}
		}
		else
		{
			uFirstLine=0;
		}

		if((cp=strchr(cLine,':')))
		{
			*cp=0;
			sprintf(cArpaZone,"%.255s",cLine);
			sscanf(cp+1,"%u PTR %s",&uLastOctet,cFQDN);
		}

		if(uIgnorecParam1)
			sprintf(cQuery,"SELECT tZone.uZone FROM tZone,tResource "
			"WHERE tResource.uZone=tZone.uZone AND tZone.cZone='%s' "
			"AND tResource.cName='%u' AND tResource.uRRType=7",
				cArpaZone,uLastOctet);
		else
			sprintf(cQuery,"SELECT tZone.uZone FROM tZone,tResource "
			"WHERE tResource.uZone=tZone.uZone AND tZone.cZone='%s' "
			"AND tResource.cName='%u' AND tResource.cParam1='%s' AND tResource.uRRType=7",
				cArpaZone,uLastOctet,cFQDN);
		//Debug only
		//printf("%s\n",cQuery);
		mysql_query(&gMysql,cQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(0);
		}
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			uFound++;
		}
		else
		{
			if(uIgnorecParam1)
			{
				MYSQL_RES *res2;
				MYSQL_ROW field2;
				sprintf(cQuery,"SELECT tZone.uZone FROM tZone WHERE tZone.cZone='%s'",cArpaZone);
				mysql_query(&gMysql,cQuery);
				if(mysql_errno(&gMysql))
				{
					printf("%s\n",mysql_error(&gMysql));
					exit(0);
				}
				res2=mysql_store_result(&gMysql);
				if((field2=mysql_fetch_row(res2)))
					sscanf(field2[0],"%u",&uZone);
				mysql_free_result(res2);
				if(uFix && uZone)
				{
					printf("%u.%s PTR %s not found, adding.\n",
						uLastOctet,cArpaZone,cFQDN);
					sprintf(cQuery,"INSERT INTO tResource SET uZone=%u,cName='%u',uRRType=7,"
							"cParam1='%s',cComment='htmlMassPTRCheck()',uOwner=1,uCreatedBy=1,"
							"uCreatedDate=UNIX_TIMESTAMP(NOW())",
							uZone,uLastOctet,cFQDN);
					//Debug only
					//printf("%s\n",cQuery);
					mysql_query(&gMysql,cQuery);
					if(mysql_errno(&gMysql))
					{
						printf("%s\n",mysql_error(&gMysql));
						exit(0);
					}
					if(mysql_affected_rows(&gMysql)==1)
					{
						UpdateSerialNum(uZone);
						SubmitJob("Modify",1,cArpaZone,0,0);
					}
				}
				else
				{
					printf("%u.%s PTR not found\n",uLastOctet,cArpaZone);
				}
			}
			else
			{
				printf("%u.%s PTR %s not found\n",uLastOctet,cArpaZone,cFQDN);
			}
		}
		mysql_free_result(res);
		uTotal++;
	}
	printf("htmlMassPTRCheck() end\n");
	printf("Found %u of %u total (%u).\n",uFound,uTotal,(uTotal-uFound));
	exit(0);

}//void htmlMassPTRCheck(void)


void htmlMassCheckZone(void)
{
	char cLine[100]={"ERROR"};
	printf("Content-type: text/plain\n\n");
	printf("htmlMassCheckZone() start\n");
	while(cLine[0])
	{
		char cCommand[512]={"ERROR"};
		FILE *fp;

		sprintf(cLine,"%.99s",ParseTextAreaLines(cMassList));
		//ParseTextAreaLines() required break;
		if(cLine[0]==0) break;

		sprintf(cCommand,"named-checkzone %.99s /usr/local/idns/named.d/master/external/%c/%.99s",
			cLine,cLine[0],cLine);
		if((fp=popen(gcQuery,"r")))
		{
			while(fgets(gcQuery,512,fp))
			printf("%s",gcQuery);
			pclose(fp);
		}
		else
			printf("Error: could not run named-checkzone.\n");
	}
	printf("htmlMassCheckZone() end\n");
	exit(0);

}//void htmlMassCheckZone(void)


void tZoneNavList(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	char cSearchSecOnly[16]={"&uSearchSecOnly"};

	unsigned uCount=0;

	if(!cSearch[0])
	{
        	printf("<p><u>tZoneNavList</u><br>\n");
        	printf("Must restrict via cSearch\n");
		return;
	}

	if(uSearchSecOnly)
		ExtSelectSearch("tZone","uZone,cZone,(SELECT tView.cLabel FROM tView WHERE tView.uView=tZone.uView)",
					"cZone",cSearch,"uSecondaryOnly=1",20);
	else
		ExtSelectSearch("tZone","uZone,cZone,(SELECT tView.cLabel FROM tView WHERE tView.uView=tZone.uView)",
					"cZone",cSearch,NULL,20);

        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
        {
        	printf("<p><u>tZoneNavList</u><br>\n");
                printf("%s",mysql_error(&gMysql));
                return;
        }

        res=mysql_store_result(&gMysql);
	printf("<p><u>tZoneNavList</u><br>\n");
	if(mysql_num_rows(res))
	{	
		if(!uSearchSecOnly) cSearchSecOnly[0]=0;
	        while((field=mysql_fetch_row(res)))
		{
			uCount++;
			printf("<a class=darkLink href=?gcFunction=tZone&uZone=%s&cSearch=%s%s>%s [%s]</a><br>\n",
				field[0]
				,cURLEncode(cSearch)
				,cSearchSecOnly
				,field[1],field[2]);
			if(uCount>=100)
			{
				printf("More than 100 records: You must refine your search further<br>\n");
				break;
			}
	        }
	}
	else
		printf("No results found<br>");	

        mysql_free_result(res);

}//void tZoneNavList(void)


void tZoneContextInfo(void)
{
        printf("<u>Record Context Info</u><br>");
	tNSSetMembers(uNSSet);

}//void tZoneContextInfo(void)


void htmlCheckSOA(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	char cTmpFile[100]={"/tmp/iDNS.soadig"};
	pid_t uPID;

	uPID=getpid();
	sprintf(cTmpFile,"/tmp/iDNS.soadig.%u",uPID);


	sprintf(gcQuery,"SELECT tNS.cFQDN FROM tNSSet,tNS WHERE tNSSet.uNSSet=tNS.uNSSet AND"
			" tNSSet.uNSSet=%u ORDER BY tNS.cFQDN",uNSSet);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		sprintf(gcQuery,"dig @%s SOA %s >> %.99s 2>&1",
					field[0],cZone,cTmpFile);
		system(gcQuery);
	}
	mysql_free_result(res);

	Header_ism3("htmlCheckSOA()",0);
	printf("</center><pre>%s<blockquote>",cZone);
	PassDirectHtml(cTmpFile);
	printf("</blockquote></pre>");
	printf("<input type=hidden name=gcFunction value=tZoneTools>");
	unlink(cTmpFile);
	Footer_ism3();

}//void htmlCheckSOA(void)


//hard coded default install path should be replaced by GetConfiguration someday
void htmlMasterZoneFile(void)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	char cSearchSecOnly[16]={"&uSearchSecOnly"};

	sprintf(gcQuery,"SELECT cLabel FROM tView WHERE uView=%u",uView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
		printf("%s",mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		char cFile[256]={""};
		struct stat statInfo;

		if(!uSearchSecOnly) cSearchSecOnly[0]=0;
		sprintf(cFile,"/usr/local/idns/named.d/master/%s/%.1s/%s",field[0],cZone,cZone);

		if(!stat(cFile,&statInfo))
		{

			Header_ism3("htmlMasterZoneFile()",0);
			printf("</center><pre>%s<blockquote>",cFile);
			PassDirectHtmlLineNum(cFile);
			printf("</blockquote></pre>");
			printf("<input type=hidden name=gcFunction value=tZoneTools>");
			printf("Back w/search link: <a class=darkLink href=?gcFunction=tZone&uZone=%u&cSearch=%s%s>%s [%s]</a><br>\n",
				uZone
				,cURLEncode(cSearch)
				,cSearchSecOnly
				,cZone
				,field[0]);
			Footer_ism3();
		}
		else
		{
			mysql_free_result(res);
			tZone("<blink>Possible error</blink>: File not found. See command line for building all files");
		}
	}
	else
	{
		mysql_free_result(res);
		tZone("<blink>Error</blink>: Zone not found");
	}

}//void htmlMasterZoneFile(void)


void htmlExternalNSRecords(void)
{
	char cCommand[256]={""};

	sprintf(cCommand,"/usr/bin/dig @8.8.8.8 %s > /tmp/htmlExternalNSRecords.txt 2>&1",cZone);

	if(!system(cCommand))
	{

			Header_ism3("htmlExternalNSRecords()",0);
			printf("<pre><blockquote>");
			printf("</center><pre>%s<blockquote>",cCommand);
			PassDirectHtml("/tmp/htmlExternalNSRecords.txt");
			printf("</blockquote></pre>");
			printf("<input type=hidden name=gcFunction value=tZoneTools>");
			printf("Back w/search link: <a class=darkLink href=?gcFunction=tZone&uZone=%u&cSearch=%s>%s</a><br>\n",
				uZone
				,cURLEncode(cSearch)
				,cZone);
			Footer_ism3();
	}
	else
	{
		tZone("<blink>Error</blink>: /usr/bin/dig");
	}
}//void htmlExternalNSRecords(void)


//hard coded default install path should be replaced by GetConfiguration someday
void htmlMasterZonesCheck(void)
{
	char cSystem[128];
	char cTmpFile[100]={"/tmp/iDNS.mzc"};
	pid_t uPID;

	uPID=getpid();
	sprintf(cTmpFile,"/tmp/iDNS.mzc.%u",uPID);
	sprintf(cSystem,"grep %s /usr/local/idns/named.d/master.zones > %.99s 2>&1",
				cZone,cTmpFile);
	system(cSystem);

	Header_ism3("htmlMasterZonesCheck()",0);
	printf("</center><pre>%s<blockquote>",cZone);
	PassDirectHtml(cTmpFile);
	printf("</blockquote></pre>");
	printf("<input type=hidden name=gcFunction value=tZoneTools>");
	unlink(cTmpFile);
	Footer_ism3();

}//void htmlMasterZonesCheck(void)


//hard coded default install path should be replaced by GetConfiguration someday
void htmlMasterNamedCheckZone(void)
{
	char cSystem[256];
	char cView[100]={"external"};
	char cTmpFile[100]={"/tmp/iDNS.ncz"};
        MYSQL_RES *res;
        MYSQL_ROW field;
	pid_t uPID;

	sprintf(gcQuery,"SELECT cLabel FROM tView WHERE uView=%u",uView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
		printf("%s",mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sprintf(cView,"%.99s",field[0]);
	mysql_free_result(res);

	uPID=getpid();
	sprintf(cTmpFile,"/tmp/iDNS.ncz.%u",uPID);
	sprintf(cSystem,"/usr/sbin/named-checkzone %.64s /usr/local/idns/named.d/master/%.64s/%.1s/%.64s > %s 2>&1",
		cZone,cView,cZone,cZone,cTmpFile);
	system(cSystem);

	Header_ism3("htmlMasterNamedCheckZone()",0);
	printf("</center><pre>%s<blockquote>",cSystem);
	PassDirectHtml(cTmpFile);
	printf("</blockquote></pre>");
	printf("<input type=hidden name=gcFunction value=tZoneTools>");
	unlink(cTmpFile);
	Footer_ism3();

}//void htmlMasterNamedCheckZone(void)


void CloneZone(char *cSourceZone,char *cTargetZone,unsigned uView)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uSrcZone=0;

	//Sanity check
	sprintf(gcQuery,"SELECT uZone FROM tZone WHERE cZone='%s' AND uView=%u AND uOwner=%u",
			cTargetZone,uView,guCompany);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);

	if(mysql_num_rows(res))
		tZone("<blink>Error:</blink> Target zone must exist and be owned by your company");
	
	mysql_free_result(res);

	sprintf(gcQuery,"SELECT uNSSet,cHostmaster,uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,uMailServers, "
	//sprintf(gcQuery,"SELECT uNameServer,cHostmaster,uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,uMailServers, "
			"cMainAddress,uRegistrar,uSecondaryOnly,cOptions,uOwner,uZone "
			"FROM tZone WHERE cZone='%s' AND uView=%u",cSourceZone,uView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		sscanf(field[14],"%u",&uSrcZone);
		sprintf(gcQuery,"INSERT INTO tZone SET cZone='%s',uNSSet=%s,cHostmaster='%s',uSerial=%s,uExpire=%s,uRefresh=%s,uTTL=%s,uRetry=%s,"
		//sprintf(gcQuery,"INSERT INTO tZone SET uNameServer=%s,cHostmaster='%s',uSerial=%s,uExpire=%s,uRefresh=%s,uTTL=%s,uRetry=%s,"
				"uZoneTTL=%s,uMailServers=%s,uView=%u,cMainAddress='%s',uRegistrar=%s,uSecondaryOnly=%s,"
				"cOptions='%s',uOwner=%s,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
				cTargetZone
				,field[0]
				,field[1]
				,field[2]
				,field[3]
				,field[4]
				,field[5]
				,field[6]
				,field[7]
				,field[8]
				,uView
				,field[9]
				,field[10]
				,field[11]
				,field[12]
				,field[13]
				,guLoginClient
				);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		uZone=mysql_insert_id(&gMysql);

		mysql_free_result(res);

		sprintf(gcQuery,"SELECT cName,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4,cComment,uOwner FROM tResource WHERE uZone=%u",uSrcZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		res=mysql_store_result(&gMysql);
		while((field=mysql_fetch_row(res)))
		{
			sprintf(gcQuery,"INSERT INTO tResource SET cName='%s',uTTL=%s,uRRType=%s,cParam1='%s',cParam2='%s',cParam3='%s',cParam4='%s',"
					"cComment='%s',uOwner=%s,uZone=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					field[0]
					,field[1]
					,field[2]
					,field[3]
					,field[4]
					,field[5]
					,field[6]
					,field[7]
					,field[8]
					,uZone
					,guLoginClient
					);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));

		}
	}	
	if(SubmitJob("New",uNSSet,cTargetZone,0,0))
		htmlPlainTextError(mysql_error(&gMysql));

	tZone("Zone cloned OK");

}//void CloneZone(char *cSourceZone,char *cTargetZone);


char *GetRRType(unsigned uRRType);
char *cPrintNSList(FILE *zfp,char *cuNSSet);
void PrintMXList(FILE *zfp,char *cuMailServers);


unsigned OnLineZoneCheck(unsigned uZone,unsigned uCalledMode,unsigned uCalledFrom)
{
	return(0);
	//This function will create a zonefile online and run named-checkzone
	MYSQL_RES *res;
	MYSQL_ROW field;
	MYSQL_RES *res2;
	MYSQL_ROW field2;
	FILE *zfp;
	FILE *dnfp;
	unsigned uRRType=0;
	char cTTL[50]={""};
	char cZoneFile[100]={""};
	char cZone[256]={""};
	static char cMessage[512]={""};

	//Test if named-checkzone can be run, otherwise return 0
	if(access("/usr/sbin/named-checkzone",X_OK)==-1) return(0); //Ticket #100

	sprintf(cZone,"%.255s",ForeignKey("tZone","cZone",uZone));
	char *cp;
	if((cp=strchr(cZone,'/'))==NULL)
	{
		sprintf(cZoneFile,"/tmp/%s",cZone);
	}
	else
	{
		sprintf(cZoneFile,"/tmp/%s",cp+1);
	}

	if((zfp=fopen(cZoneFile,"w"))==NULL)
		htmlPlainTextError("fopen() failed for temp zonefile");

	if((dnfp=fopen("/dev/null","w"))==NULL)
		htmlPlainTextError("fopen() failed for /dev/null");

	sprintf(gcQuery,"SELECT tZone.cZone,tZone.uZone,tZone.uNSSet,tZone.cHostmaster,"
			"tZone.uSerial,tZone.uTTL,tZone.uExpire,tZone.uRefresh,tZone.uRetry,tZone.uZoneTTL,"
			"tZone.uMailServers,tZone.cMainAddress,tView.cLabel FROM tZone,tNSSet,tNS,tView"
			" WHERE tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND"
			" tZone.uView=tView.uView AND tZone.uZone=%u",uZone);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		char *cp;
		char cFirstNS[100]={""};
		
		//0 cZone
		//1 uZone

		//2 uNameServer
		//3 cHostmaster
		//4 uSerial
		//
		//5 uTTL default TTL macro directive $TTL. See bind 8.2.x docs
		//
		//6 uExpire
		//7 uRefresh
		//8 uRetry
		//9 uZoneTTL is the negative response caching value in the SOA.
		//  See bind 8.2.x docs
		//
		//10 uMailServers
		//11 cMainAddress
		//12 tView.cLabel
		//13 tView.cMaster
		//14 tView.uOrder
		if((cp=strchr(field[3],' '))) *cp=0;

		fprintf(zfp,"; %s\n",field[0]);
		fprintf(zfp,"$TTL %s\n",field[5]);
	
		//MASTER HIDDEN support
		sprintf(cFirstNS,"%.99s",cPrintNSList(dnfp,field[2]));
		if(cFirstNS[0])
			fprintf(zfp,
			"@ IN SOA %s. %s. (\n",cFirstNS,field[3]);
/*		else
			fprintf(zfp,
			"@ IN SOA %s. %s. (\n",cMasterNS,field[3]);*/
		fprintf(zfp,"\t\t\t%s\t;serial\n",field[4]);
		fprintf(zfp,"\t\t\t%s\t\t;slave refresh\n",field[7]);
		fprintf(zfp,"\t\t\t%s\t\t;slave retry\n",field[8]);
		fprintf(zfp,"\t\t\t%s\t\t;slave expiration\n",field[6]);
		fprintf(zfp,"\t\t\t%s )\t\t;negative ttl\n\n",
			field[9]);

		//ns
		cPrintNSList(zfp,field[2]);

		//mx1
		PrintMXList(zfp,field[10]);
		//in a 0.0.0.0 is the null IP number
		if(field[11][0] && field[11][0]!='0')
			fprintf(zfp,"\t\tA %s\n;\n",field[11]);
		fprintf(zfp,";\n");

		//TODO
		if(!strcmp(field[0]+strlen(field[0])-5,".arpa"))
			sprintf(gcQuery,"SELECT cName,uTTL,uRRType,cParam1,cParam2 FROM tResourceTest WHERE uZone=%u ORDER BY uResource",uZone);
		else
			sprintf(gcQuery,"SELECT cName,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4 FROM tResourceTest WHERE uZone=%u ORDER BY cName",uZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
		res2=mysql_store_result(&gMysql);
		while((field2=mysql_fetch_row(res2)))
		{
			char cRRType[9]="";

			sscanf(field2[2],"%u",&uRRType);
			sprintf(cRRType,"%.8s",GetRRType(uRRType));

			if(field2[1][0]!='0') strcpy(cTTL,field2[1]);

			//Do not write TTL if cName is a $GENERATE line
			if(strstr(field2[0],"$GENERATE")==NULL)
			{
				if(!strcmp(cRRType,"SRV"))
					fprintf(zfp,"%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
							field2[0],
							cTTL,
							cRRType,
							field2[3],
							field2[4],
							field2[5],
							field2[6]);
				else if(!strcmp(cRRType,"NAPTR"))
					fprintf(zfp,"%s\t%s\t%s\t%s\t%s\t(%s\t%s)\n",
							field2[0],
							cTTL,
							cRRType,
							field2[3],
							field2[4],
							field2[5],
							field2[6]);
				else if(1)
					fprintf(zfp,"%s\t%s\t%s\t%s\t%s\n",
							field2[0],
							cTTL,
							cRRType,
							field2[3],
							field2[4]);
			}
			else
			{
				fprintf(zfp,"%s\t%s\t%s\t%s\n",
						field2[0],
						cRRType,
						field2[3],
						field2[4]);
			}
		}
		
		mysql_free_result(res2);
		fclose(zfp);

		sprintf(gcQuery,"/usr/sbin/named-checkzone %s %s 2>&1 > /dev/null",field[0],cZoneFile);
		if(system(gcQuery))
		{
			char cLine[1024]={""};
			sprintf(gcQuery,"/usr/sbin/named-checkzone %s %s 2>&1",field[0],cZoneFile);

			if((zfp=popen(gcQuery,"r"))==NULL)
				htmlPlainTextError("popen() failed");
			
			//zone clonetest.com/IN: loading master file /tmp/clonetest.com: CNAME and other data
			while(fgets(cLine,sizeof cLine,zfp)!=NULL)
			{
				if(strstr(cLine,"zone"))
				{
					char *cp;
					cp=strstr(cLine,cZoneFile);
					cp=cp+strlen(cZoneFile)+2; //2 more chars ': '
					if(uCalledFrom)
					{
						guMode=uCalledMode;
						sprintf(cMessage,"<blink>Error: </blink> RR error: %s (if delegation conflict remove RR)",cp);
						tZone(cMessage);
					}
					else
					{
						guMode=uCalledMode;
						sprintf(cMessage,"<blink>Error: </blink> RR error: %s",cp);
						tResource(cMessage);
					}
				}
			}
			pclose(zfp);
			//unlink(cZoneFile);
			return(1);
		}
	}
	//unlink(cZoneFile);

	return(0);

}//unsigned OnLineZoneCheck(void)


unsigned SelectRRType(char *cRRType)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uRRType=0;

	sprintf(gcQuery,"SELECT uRRType from tRRType WHERE cLabel='%s'",cRRType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uRRType);
	mysql_free_result(res);

	return(uRRType);
	
}//unsigned SelectRRType(char *cRRType)


void CreatetResourceTest(void)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS tResourceTest ( uResource INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cName VARCHAR(100) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, uTTL INT UNSIGNED NOT NULL DEFAULT 0, uRRType INT UNSIGNED NOT NULL DEFAULT 0, cParam1 VARCHAR(255) NOT NULL DEFAULT '', cParam2 VARCHAR(255) NOT NULL DEFAULT '', cComment TEXT NOT NULL DEFAULT '', uZone INT UNSIGNED NOT NULL DEFAULT 0,index (uZone), cParam3 VARCHAR(255) NOT NULL DEFAULT '', cParam4 VARCHAR(255) NOT NULL DEFAULT '' )");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
}//void CreatetRestResource(void)

char *ParseTextAreaLines2(char *cTextArea);

void PrepDelToolsTestData(unsigned uNumIPs)
{
	char cNS[100]={""};
	char cName[100]={""};
	char cParam1[100]={""};
	unsigned uA,uB,uC,uD,uE;
	unsigned uIPBlockFormat=0;
	char cNServers[4096]={""};

	CreatetResourceTest();
	sprintf(gcQuery,"DELETE FROM tResourceTest WHERE uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	sprintf(gcQuery,"INSERT INTO tResourceTest (uResource,cName,uOwner,uCreatedBy,uCreatedDate,uModBy,"
			"uModDate,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4,cComment,uZone) "
			"SELECT uResource,cName,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate,uTTL,uRRType,"
			"cParam1,cParam2,cParam3,cParam4,cComment,uZone FROM tResource WHERE "
			"uZone=%u",uZone);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	if(strchr(cIPBlock,'/'))
	{
		sscanf(cIPBlock,"%u.%u.%u.%u/%u",&uA,&uB,&uC,&uD,&uE);
		uIPBlockFormat=IP_BLOCK_CIDR;
	}
	else if(strchr(cIPBlock,'-'))
	{
		sscanf(cIPBlock,"%u.%u.%u.%u-%u",&uA,&uB,&uC,&uD,&uE);
		uIPBlockFormat=IP_BLOCK_DASH;
	}

	sprintf(cNServers,"%.4095s",cNSList);
	
	while(1)
	{
		sprintf(cNS,"%.99s",ParseTextAreaLines2(cNServers));
		if(!cNS[0]) break;
		if(uIPBlockFormat==IP_BLOCK_CIDR)
			sprintf(cName,"%u/%u",uD,uE);
		else if(uIPBlockFormat==IP_BLOCK_DASH)
			sprintf(cName,"%u-%u",uD,uE);

		sprintf(gcQuery,"INSERT INTO tResourceTest SET uZone=%u,cName='%s',uTTL=%u,"
					"uRRType=2,cParam1='%s',cComment='Delegation (%s)',"
					"uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					uZone
					,cName
					,uDelegationTTL
					,cNS
					,cIPBlock
					,uOwner
					,guLoginClient);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
	}

	//$GENERATE 0-255 $ CNAME $.0/24.21.68.217.in-addr.arpa.
	if(uIPBlockFormat==IP_BLOCK_CIDR)
		sprintf(cParam1,"$.%u/%u.%u.%u.%u.in-addr.arpa.",
				uD
				,uE
				,uC
				,uB
				,uA
			       );
	else if(uIPBlockFormat==IP_BLOCK_DASH)
	{
		sprintf(cParam1,"$.%u-%u.%u.%u.%u.in-addr.arpa.",
				uD
				,uE
				,uC
				,uB
				,uA
			       );
	}
	sprintf(gcQuery,"INSERT INTO tResourceTest SET uZone=%u,cName='$GENERATE %u-%u $',"
			"uRRType=5,cParam1='%s',cComment='Delegation (%s)',uOwner=%u,"
			"uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			uZone
			,uD
			,(uD+uNumIPs)
			,cParam1
			,cIPBlock
			,uOwner
			,guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

}//void PrepDelToolsTestData(unsigned uNumIPs)


void PassDirectHtmlLineNum(char *file)
{
	FILE *fp;
	char buffer[1024];

	if((fp=fopen(file,"r"))!=NULL)
	{
		register int n=1;

		while(fgets(buffer,1024,fp)!=NULL)
			fprintf(stdout,"(%d) %s",n++,buffer);

		fclose(fp);
	}

}//void PassDirectHtmlLineNum(char *file)


unsigned uGetZoneSearchGroup(const char *gcUser)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	unsigned uGroup=0;

	sprintf(gcQuery,"SELECT uGroup FROM tGroup WHERE cLabel='%sZone'",gcUser);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
        res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		if(field[0]!=NULL)
			sscanf(field[0],"%u",&uGroup);
	}
	mysql_free_result(res);

	return(uGroup);

}//unsigned uGetZoneSearchGroup(const char *gcUser)


char *cNSFromWhois(char const *cZone,char const *cuNSSet)
{
	FILE *fp;
	char cCommand[256];
	unsigned uIn=1;

	char static cNSList[256]={""};
	sprintf(cNSList,"whois error1");
	sprintf(cCommand,"/usr/bin/dig @8.8.8.8 %.64s ns 2>/dev/null|"
			"grep IN | grep NS |grep -v '^;'|"
			"awk '{print $5}'|"
			"sed -e 's/.$//g' | sort -u | tr '[:space:]' ',' | sed -e 's/,$//g'",cZone);
	if((fp=popen(cCommand,"r")))
	{
		cNSList[0]=0;
		fgets(cNSList,255,fp);
		pclose(fp);
		if(!cNSList[0])
		{
			sprintf(cNSList,"no info for %.64s",cZone);
		}
		else
		{
			MYSQL_RES *res;
			MYSQL_ROW field;
			sprintf(gcQuery,"SELECT cFQDN FROM tNS WHERE uNSSet=%s AND uNSType=4",cuNSSet);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
				sprintf(cNSList,"%.254s",mysql_error(&gMysql));
			res=mysql_store_result(&gMysql);
			while((field=mysql_fetch_row(res)))
			{
				uIn=uIn && strstr(cNSList,field[0]);
				if(!uIn) break;
			}
			mysql_free_result(res);
			if(uIn)
				sprintf(cNSList,"NS records match NSSet");
		}

	}
	return(cNSList);
}//char *cNSFromWhois()


