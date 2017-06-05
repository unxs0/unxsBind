/*
FILE
	svn ID removed
PURPOSE
	Included in main.c. For command line interface and html main link.
AUTHOR/LEGAL
	(C) 2001-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file included.
*/

#include "local.h"
char *strptime(const char *s, const char *format, struct tm *tm);//this is missing from time.h find out why.

//data
extern unsigned guJS;//Global used by bind.c
//The below default should work for you, if not please 
//define cBinDir at tConfiguration
char gcBinDir[100]={"/usr/sbin"};
static char cTableList[36][32]={"tAuthorize","tBlock","tClient","tConfiguration","tDeletedResource","tDeletedZone","tGlossary","tGroup","tGroupGlue","tGroupType","tHit","tHitMonth","tJob","tLog","tLogMonth","tLogType","tMailServer","tMonth","tMonthHit","tNS","tNSSet","tNSType","tRRType","tRegistrar","tResource","tResourceImport","tServer","tTemplate","tTemplateSet","tTemplateType","tView","tZone","tZoneImport",""};
char cInitTableList[36][32]={ "tAuthorize","tBlock","tClient","tConfiguration","tGlossary","tGroupType","tLogType","tMailServer","tNS","tNSSet","tNSType","tRRType","tRegistrar","tResource","tServer","tTemplate","tTemplateSet","tTemplateType","tView","tZone",""};


//Please clean up ths TOC proto section.
void ExtMainShell(int argc, char *argv[]);
void Initialize(char *cPasswd);
void Backup(char *cPasswd);
void Restore(char *cPasswd, char *cTableName);
void RestoreAll(char *cPasswd);
void mySQLRootConnect(char *cPasswd);
void ImportTemplateFile(char *cTemplate, char *cFile, char *cTemplateSet, char *cTemplateType);
void ExportTemplateFiles(char *cDir, char *cTemplateSet, char *cTemplateType);
void CalledByAlias(int iArgc,char *cArgv[]);
unsigned TextConnectDb(void);
void Tutorial(void);
void NamedConf(void);
void MasterZones(void);
void Admin(void);
void ListZones(unsigned uNSSet,unsigned uNSReport);
void PerfQueryList(void);
void UpdateSchema(void);
void UpdateTables(void);
void ImportFromDb(char *cSourceDbName, char *cTargetDbName, char *cPasswd);
void MonthHitData(void);
void MonthUsageData(unsigned uSimile);
void DayUsageData(unsigned uLogType);
void NextMonthYear(char *cMonth, char *cYear, char *cNextMonth, char *cNextYear);
void ExtracttLog(char *cMonth, char *cYear, char *cPasswd, char *cTablePath);
void CreatetLogTable(char *cTableName);
void ExtracttHit(char *cMonth, char *cYear, char *cPasswd, char *cTablePath);
void CreatetHitTable(char *cTableName);
void ZeroSystem(void);
void Import(void);
void ImportAxfr(void);
void CloneZonesFromList(void);
void DropImportedZones(void);
void ImportCompanies(void);
void DropCompanies(void);
void ImportUsers(void);
void DropUsers(void);
void ImportBlocks(void);
void DropBlocks(void);
void AssociateCompaniesZones(void);
void ImportRegistrars(void);
void DropRegistrars(void);
void AssociateRegistrarsZones(void);
void MassZoneUpdate(void);
void MassZoneNSUpdate(char *cLabel);
void FixBlockOwnership(void);
void ImportSORRs(void);
void CheckAllZones(void);
char *cPrintNSList(FILE *zfp,char *cuNSSet);
void PrintMXList(FILE *zfp,char *cuMailServer);
void CreateWebZone(char *cDomain, char *cIP, char *cNSSet, char *cMailServer);
void DropZone(char *cDomain, char *cNSSet);
void CompareZones(char *cDNSServer1IP, char *cDNSServer2IP, char *cuOwner);
void PassDirectHtml(char *file);
void GetConfiguration(const char *cName, char *cValue, unsigned uHtml);
void Initialize(char *cPasswd);
void ExportTable(char *cTable, char *cFile);
time_t cDateToUnixTime(char *cDate);
unsigned NewNSSet(char *cLabel,char *cMasterIPs,char *cuOwner);
void NewNS(char *cFQDN,char *cNSType,unsigned uNSSet);
void CreatePBXZonesFromVZ(void);
void DeletePBXZones(void);

//extern bind.c protos
void CreateMasterFiles(char *cMasterNS, char *cZone,unsigned uModDBFiles,
		unsigned uModStubs,unsigned uDebug);
void CreateSlaveFiles(char *cSlaveNS,char *cZone,char *cMasterIP,unsigned uDebug);
void InstallNamedFiles(char *cIpNum);
void SlaveJobQueue(char *cNSSet, char *cMasterIP);
void MasterJobQueue(char *cNSSet);
void ServerJobQueue(char *cServer);
void ProcessExtJobQueue(char *cServer);
void ProcessVZJobQueue(void);
void ExportRRCSV(char *cCompany,char *cOutFile);

static unsigned guZeroSystem=0;
static unsigned guAddBackup=1;	
void voidAddBackup(void);
void voidhtmlListBackupLinks(void);

int iExtMainCommands(pentry entries[], int x)
{
	if(!strcmp(gcFunction,"MainTools"))
	{
		if(!strcmp(gcCommand,"Admin"))
		{
			Admin();
		}
		else if(!strcmp(gcCommand,"NamedConf"))
		{
			NamedConf();
		}
		else if(!strcmp(gcCommand,"MasterZones"))
		{
			MasterZones();
		}
		else if(!strcmp(gcCommand,"Tutorial"))
		{
			Tutorial();
		}
		else if(!strcmp(gcCommand,"Zero System"))
		{
			if(guPermLevel>=12)
				guZeroSystem=1;
			Admin();
		}
		else if(!strcmp(gcCommand,"Zero System Confirm"))
		{
			if(guPermLevel>=12)
				ZeroSystem();
			Admin();
		}
		else if(!strcmp(gcCommand,"Add Backup"))
		{
			if(guPermLevel>=12)
				voidAddBackup();
			Admin();
		}
	}
	return(0);

}//int iExtMainCommands(pentry entries[], int x)


void DashBoard(const char *cOptionalMsg)
{
	char cConfigBuffer[256]={""};

	//To handle error messages etc.
	if(cOptionalMsg[0] && strcmp(cOptionalMsg,"DashBoard"))
	{
		printf("%s\n",cOptionalMsg);
		return;
	}

        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
	time_t luClock;

	OpenFieldSet("Dashboard",100);
	
	//GetConfiguration("allzone.stats",cConfigBuffer,1);
	//if(cConfigBuffer[0])
	//	printf("<img src='%s'>\n",cConfigBuffer);

	cConfigBuffer[0]=0;
	GetConfiguration("mrcstatus",cConfigBuffer,1);
	if(cConfigBuffer[0])
	{
		OpenRow("Replication Status","black");
		printf("<img src='%s'>\n",cConfigBuffer);
	}


	OpenRow("Cluster BIND Errors (Last 10)","black");
	sprintf(gcQuery,"SELECT cMessage,GREATEST(uCreatedDate,uModDate),cServer,cLabel,uPermLevel,uTablePK FROM"
			" tLog WHERE uLogType=5 ORDER BY GREATEST(uCreatedDate,uModDate) DESC LIMIT 10");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	printf("</td></tr>\n");
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[1],"%lu",&luClock);
		printf("<td></td><td>%s</td><td colspan=2><a href=?gcFunction=tZone&uZone=%s>%s</a> %s (%s times)"
			"</td><td>%s</td></tr>\n",ctime(&luClock),mysqlField[5],mysqlField[3],
							mysqlField[0],mysqlField[4],mysqlField[2]);
	}
	mysql_free_result(mysqlRes);

	OpenRow("System Messages (Last 10)","black");
	sprintf(gcQuery,"SELECT cMessage,GREATEST(uCreatedDate,uModDate),cServer FROM tLog WHERE uLogType=4"
			" ORDER BY GREATEST(uCreatedDate,uModDate) DESC LIMIT 10");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	printf("</td></tr>\n");
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[1],"%lu",&luClock);
		printf("<td></td><td>%s</td><td colspan=2>%s</td><td>%s</td></tr>\n",
			ctime(&luClock),mysqlField[0],mysqlField[2]);
	}
	mysql_free_result(mysqlRes);

	//1-3 backend org admin interfaces
	OpenRow("General Usage (Last 10)","black");
	sprintf(gcQuery,"SELECT tLog.cLabel,GREATEST(tLog.uCreatedDate,tLog.uModDate),tLog.cLogin,tLog.cTableName,"
			"tLog.cHost,tLogType.cLabel FROM tLog,tLogType WHERE tLog.uLogType=tLogType.uLogType AND"
			" tLog.uLogType<=3 ORDER BY GREATEST(tLog.uCreatedDate,tLog.uModDate) DESC LIMIT 10");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	printf("</td></tr>\n");
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[1],"%lu",&luClock);
		printf("<td></td><td>%s</td><td>%s %s</td><td>%s %s</td><td>%s</td></tr>\n",
			ctime(&luClock),mysqlField[0],mysqlField[3],mysqlField[2],mysqlField[5],mysqlField[4]);
	}
	mysql_free_result(mysqlRes);

	//login/logout activity
	OpenRow("Login Activity (Last 10)","black");
	sprintf(gcQuery,"SELECT tLog.cLabel,GREATEST(tLog.uCreatedDate,tLog.uModDate),tLog.cServer,tLog.cHost,"
			"tLogType.cLabel FROM tLog,tLogType WHERE tLog.uLogType=tLogType.uLogType AND"
			" tLog.uLogType>=6 ORDER BY GREATEST(tLog.uCreatedDate,tLog.uModDate) DESC LIMIT 10");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	printf("</td></tr>\n");
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[1],"%lu",&luClock);
		printf("<td></td><td>%s</td><td>%s %s</td><td>%s</td><td>%s</td></tr>\n",
			ctime(&luClock),mysqlField[0],mysqlField[4],mysqlField[2],mysqlField[3]);
	}
	mysql_free_result(mysqlRes);

	OpenRow("Jobs Pending (Last 10)","black");
	sprintf(gcQuery,"SELECT cJob,GREATEST(uCreatedDate,uModDate),cZone,cTargetServer FROM tJob ORDER BY"
			" GREATEST(uCreatedDate,uModDate) DESC LIMIT 10");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	printf("</td></tr>\n");
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[1],"%lu",&luClock);
		printf("<td></td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>\n",
			ctime(&luClock),mysqlField[0],mysqlField[2],mysqlField[3]);
	}
	mysql_free_result(mysqlRes);

	CloseFieldSet();

}//void DashBoard(const char *cOptionalMsg)


void ExtMainContent(void)
{
	printf("<p><input type=hidden name=gcFunction value=MainTools>");

	OpenFieldSet("System Information",100);

	OpenRow("Hostname","black");
	printf("<td>%s</td></tr>\n",gcHostname);

	OpenRow("Build Information","black");
	printf("<td>%s</td></tr>\n",gcBuildInfo);

	OpenRow("Status","black");
	printf("<td>%s %s</td></tr>\n",gcRADStatus,REV);

	OpenRow("Application Summary","black");
	printf("<td>DNS/Bind manager for one or 100s of primary and secondary servers.</td></tr>\n");

	if(guPermLevel>9)
	{
		register unsigned int i;
		OpenRow("Table List","black");
		printf("<td>\n");
		for(i=0;cTableList[i][0];i++)
			printf("<a href=?gcFunction=%.32s>%.32s</a><br>\n",
				cTableList[i],cTableList[i]);
		printf("</td></tr>\n");

        	OpenRow("Admin Functions","black");
		printf("<td><input type=hidden name=gcFunction value=MainTools>\n");
		printf("<input class=largeButton type=submit name=gcCommand value=Admin></td></tr>\n");
	}

	CloseFieldSet();

}//void ExtMainContent(void)


void RestoreAll(char *cPasswd)
{
	char cISMROOT[256]={""};
	register int i;

	if(getenv("ISMROOT")!=NULL)
		sprintf(cISMROOT,"%.255s",getenv("ISMROOT"));
	if(!cISMROOT[0])
	{
		printf("You must set ISMROOT env var first. Ex. export ISMROOT=/home/joe/unxsVZ\n");
		exit(1);
	}
	printf("Restoring iDNS data from .txt file in %s/iDNS/data...\n\n",cISMROOT);

	//connect as root to master db
	mySQLRootConnect(cPasswd);

	sprintf(gcQuery,"USE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	for(i=0;cTableList[i][0];i++)
	{
		sprintf(gcQuery,"LOAD DATA LOCAL INFILE '%s/iDNS/data/%s.txt' REPLACE INTO TABLE %s",
			cISMROOT,cTableList[i],cTableList[i]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		printf("%s\n",cTableList[i]);
	}

	printf("\nDone\n");

}//void RestoreAll(char *cPasswd)


void Restore(char *cPasswd, char *cTableName)
{
	char cISMROOT[256]={""};

	if(getenv("ISMROOT")!=NULL)
		sprintf(cISMROOT,"%.255s",getenv("ISMROOT"));
	if(!cISMROOT[0])
	{
		printf("You must set ISMROOT env var first. Ex. export ISMROOT=/home/joe/unxsVZ\n");
		exit(1);
	}

	//connect as root to master db
	mySQLRootConnect(cPasswd);

	printf("Restoring iDNS data from .txt file in %s/iDNS/data...\n\n",cISMROOT);

	sprintf(gcQuery,"USE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	sprintf(gcQuery,"LOAD DATA LOCAL INFILE '%s/iDNS/data/%s.txt' REPLACE INTO TABLE %s",cISMROOT,cTableName,cTableName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("%s\n\nDone\n",cTableName);

}//void Restore(char *cPasswd, char *cTableName)


void Backup(char *cPasswd)
{
	register int i;
	char cISMROOT[256]={""};

	if(getenv("ISMROOT")!=NULL)
		sprintf(cISMROOT,"%.255s",getenv("ISMROOT"));
	if(!cISMROOT[0])
	{
		printf("You must set ISMROOT env var first. Ex. export ISMROOT=/home/joe/unxsVZ\n");
		exit(1);
	}
	printf("Backing up iDNS data to .txt files in %s/iDNS/data...\n\n",cISMROOT);

	//connect as root to master db
	mySQLRootConnect(cPasswd);

	sprintf(gcQuery,"USE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	for(i=0;cTableList[i][0];i++)
	{
		char cFileName[300];

		sprintf(cFileName,"%s/iDNS/data/%s.txt"
				,cISMROOT,cTableList[i]);
		unlink(cFileName);

		sprintf(gcQuery,"SELECT * INTO OUTFILE '%s' FROM %s",cFileName,cTableList[i]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		printf("%s\n",cTableList[i]);
	}


	printf("\nDone.\n");

}//void Backup(char *cPasswd)


void Initialize(char *cPasswd)
{
	char cISMROOT[256]={""};
	register int i;

	if(getuid())
	{
		printf("You must be root to run this command\n");
		exit(0);
	}

	if(getenv("ISMROOT")!=NULL)
		sprintf(cISMROOT,"%.255s",getenv("ISMROOT"));
	if(!cISMROOT[0])
	{
		printf("You must set ISMROOT env var first. Ex. export ISMROOT=/home/joe/unxsVZ\n");
		exit(1);
	}
	printf("Creating db and setting permissions, installing data from %s/iDNS...\n\n",cISMROOT);

	//connect as root to master db
	mySQLRootConnect(cPasswd);

	//Create database
	sprintf(gcQuery,"CREATE DATABASE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("0-. %s\n",mysql_error(&gMysql));
		exit(1);
	}

	//Grant access privileges.
	sprintf(gcQuery,"GRANT ALL ON %s.* to %s@localhost IDENTIFIED BY '%s'",
							DBNAME,DBLOGIN,DBPASSWD);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("1-. %s\n",mysql_error(&gMysql));
		exit(1);
	}
	
	//Change to idns db. Then initialize some tables with needed data
	sprintf(gcQuery,"USE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("2-. %s\n",mysql_error(&gMysql));
		exit(1);
	}
	
	
	//Create tables and install default data
	CreatetAuthorize();
	CreatetClient();
	CreatetZone();
	CreatetResource();
	CreatetRRType();
	CreatetJob();
	CreatetMailServer();
	CreatetNS();
	CreatetNSSet();
	CreatetNSType();
	CreatetServer();
	CreatetConfiguration();
	CreatetTemplate();
	CreatetTemplateSet();
	CreatetTemplateType();
	CreatetBlock();
	CreatetView();
	CreatetLog();
	CreatetLogType();
	CreatetRegistrar();
	CreatetZoneImport();
	CreatetResourceImport();
	CreatetMonth();
	CreatetLogMonth();
	CreatetDeletedZone();
	CreatetDeletedResource();
	CreatetHit();
	CreatetHitMonth();
	CreatetMonthHit();
	CreatetGlossary();
	CreatetGroup();
	CreatetGroupGlue();
	CreatetGroupType();

        for(i=0;cInitTableList[i][0];i++)
        {
                sprintf(gcQuery,"LOAD DATA LOCAL INFILE '%s/iDNS/data/%s.txt' REPLACE INTO TABLE %s",
			cISMROOT,cInitTableList[i],cInitTableList[i]);
                mysql_query(&gMysql,gcQuery);
                if(mysql_errno(&gMysql))
                {
                        printf("%s\n",mysql_error(&gMysql));
                        exit(1);
                }
        }

        printf("Done\n");

}//void Initialize(void)


void mySQLRootConnect(char *cPasswd)
{
        mysql_init(&gMysql);
        if (!mysql_real_connect(&gMysql,DBIP0,"root",cPasswd,"mysql",0,DBSOCKET,0))
        {
		printf("Database server unavailable\n");
		exit(1);
        }

}//void mySQLRootConnect(void)


//Import from local file into tTemplate a single record
void ExportTemplateFiles(char *cDir, char *cTemplateSet, char *cTemplateType)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
	FILE *fp;
	char cFile[256];
	unsigned uTemplateSet=0;
	unsigned uTemplateType=0;

	printf("\nExportTemplateFiles(): Start\n");

	if(TextConnectDb()) exit(0);

	//uTemplateSet
	sprintf(gcQuery,"SELECT uTemplateSet FROM tTemplateSet WHERE cLabel='%s'",cTemplateSet);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&uTemplateSet);
	mysql_free_result(mysqlRes);

	if(!uTemplateSet)
	{
		printf("Could not find tTemplateSet.clabel=%s\n",cTemplateSet);
		exit(1);
	}

	//uTemplateType
	sprintf(gcQuery,"SELECT uTemplateType FROM tTemplateType  WHERE cLabel='%s'",cTemplateType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&uTemplateType);
	mysql_free_result(mysqlRes);

	if(!uTemplateType)
	{
		printf("Could not find tTemplateType.clabel=%s\n",cTemplateSet);
		exit(1);
	}

	//Get cTemplate
	sprintf(gcQuery,"SELECT cLabel,cTemplate FROM tTemplate WHERE uTemplateSet=%u AND uTemplateType=%u",
						uTemplateSet,uTemplateType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sprintf(cFile,"%.220s/%.32s",cDir,mysqlField[0]);
		if((fp=fopen(cFile,"w"))!=NULL)
		{
			printf("Writing %s\n",cFile);
			fprintf(fp,"%s",mysqlField[1]);
			fclose(fp);
		}
		else
		{
			printf("Could not open: %s\n",cFile);
			exit(1);
		}
	}
	mysql_free_result(mysqlRes);

	printf("\nExportTemplateFiles(): End\n");

}//ExportTemplateFiles()


void ImportTemplateFile(char *cTemplate, char *cFile, char *cTemplateSet, char *cTemplateType)
{
	FILE *fp;
	unsigned uTemplate=0;
	unsigned uTemplateSet=0;
	unsigned uTemplateType=0;
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
	char cBuffer[2048]={""};

	printf("\nImportTemplateFile(): Start\n");

	if(TextConnectDb()) exit(0);

	sprintf(gcQuery,"USE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	if((fp=fopen(cFile,"r"))==NULL)
	{
		printf("Could not open %s\n",cFile);
		exit(1);
	}

	//uTemplateSet
	sprintf(gcQuery,"SELECT uTemplateSet FROM tTemplateSet WHERE cLabel='%s'",cTemplateSet);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&uTemplateSet);
	mysql_free_result(mysqlRes);

	if(!uTemplateSet)
	{
		printf("Could not find tTemplateSet.clabel=%s\n",cTemplateSet);
		exit(1);
	}

	//uTemplateType
	sprintf(gcQuery,"SELECT uTemplateType FROM tTemplateType  WHERE cLabel='%s'",cTemplateType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&uTemplateType);
	mysql_free_result(mysqlRes);

	if(!uTemplateType)
	{
		printf("Could not find tTemplateType.clabel=%s\n",cTemplateSet);
		exit(1);
	}

	//uTemplate
	sprintf(gcQuery,"SELECT uTemplate FROM tTemplate WHERE cLabel='%s' AND uTemplateSet=%u AND uTemplateType=%u",
						cTemplate,uTemplateSet,uTemplateType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&uTemplate);
	mysql_free_result(mysqlRes);


	if(uTemplate)
	{
		printf("Updating tTemplate for %s %s %s\n",cTemplate,cTemplateSet,cTemplateType);
		sprintf(cBuffer,"UPDATE tTemplate SET uModBy=1,uModDate=UNIX_TIMESTAMP(NOW()),cTemplate='',"
				"uTemplateSet=%u,uTemplateType=%u,uModBy=1 WHERE uTemplate=%u",
								uTemplateSet,uTemplateType,uTemplate);
		mysql_query(&gMysql,cBuffer);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n%.254s\n",mysql_error(&gMysql),cBuffer);
			exit(1);
		}
	}
	else
	{
		printf("Inserting new tTemplate for %s\n",cTemplate);
		sprintf(cBuffer,"INSERT INTO tTemplate SET uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),"
				"cLabel='%s',uTemplateSet=%u,uTemplateType=%u",cTemplate,uTemplateSet,uTemplateType);
		mysql_query(&gMysql,cBuffer);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n%.254s\n",mysql_error(&gMysql),cBuffer);
			exit(1);
		}
		uTemplate=mysql_insert_id(&gMysql);
	}

	while(fgets(gcQuery,1024,fp)!=NULL)
	{
		sprintf(cBuffer,"UPDATE tTemplate SET cTemplate=CONCAT(cTemplate,'%s') WHERE uTemplate=%u",
				TextAreaSave(gcQuery),uTemplate);
		mysql_query(&gMysql,cBuffer);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n%.254s\n",mysql_error(&gMysql),cBuffer);
			exit(1);
		}
	}
	fclose(fp);

	printf("\nDone\n");

}//void ImportTemplateFile()


void CalledByAlias(int iArgc,char *cArgv[])
{
	if(strstr(cArgv[0],"iDNSRSS.xml"))
	{
		MYSQL_RES *res;
		MYSQL_ROW field;
		char cRSSDate[200];
		char cLinkStart[200]={""};
           	time_t tTime,luClock;
           	struct tm *tmTime;
		char *cHTTP="http";

		time(&luClock);
           	tTime=time(NULL);
           	tmTime=gmtime(&tTime);
           	strftime(cRSSDate,sizeof(cRSSDate),"%a, %d %b %Y %T GMT",tmTime);

		if(getenv("HTTP_HOST")!=NULL)
			sprintf(gcHost,"%.99s",getenv("HTTP_HOST"));
		if(getenv("HTTPS")!=NULL)
			cHTTP="https";

		//This is the standard place. With much parsing nonsense we can
		//	do better by using env vars
		sprintf(cLinkStart,"%s://%s/cgi-bin/iDNS.cgi",cHTTP,gcHost);

		printf("Content-type: text/xml\n\n");

		//Open xml
		printf("<?xml version='1.0' encoding='UTF-8'?>\n");
		printf("<rss version='2.0'>\n");
		printf("<channel>\n");
		printf("<title>iDNS RSS tJob Errors</title>\n");
		printf("<link>http://openisp.net/iDNS</link>\n");
		printf("<description>iDNS tJob Errors</description>\n");
		printf("<lastBuildDate>%.199s</lastBuildDate>\n",cRSSDate);
		printf("<generator>iDNS RSS Generator</generator>\n");
		printf("<docs>http://blogs.law.harvard.edu/tech/rss</docs>\n");
		printf("<ttl>120</ttl>\n");

		//Loop for each tJob error
		//Connect to local mySQL
		if(TextConnectDb()) exit(0);
		sprintf(gcQuery,"SELECT uJob,cServer,cJobName,uUser,cJobData FROM tJob WHERE uJobStatus=4");
		//4 is tJobStatus Done Error(s)
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
		res=mysql_store_result(&gMysql);
		while((field=mysql_fetch_row(res)))
		{
			printf("\n<item>\n");
			printf("<title>iDNS.tJob.uJob=%s</title>\n",field[0]);
			printf("<link>%s?gcFunction=tJob&amp;uJob=%s</link>\n",cLinkStart,
							field[0]);
			printf("<description>cJobName=%s Server=%s uUser=%s\ncJobData=(%s)</description>\n",
				field[2],field[1],field[3],field[4]);
			printf("<guid isPermaLink='false'>%s-%lu</guid>\n",field[0],luClock);
			printf("<pubDate>%.199s</pubDate>\n",cRSSDate);
			printf("</item>\n");

		}
       		mysql_free_result(res);

		//Close xml
		printf("\n</channel>\n");
		printf("</rss>\n");
		mysql_close(&gMysql);
		exit(0);
	}
	else
	{
		printf("Content-type: text/plain\n\n");
		printf("Called as unsupported alias %s\n",cArgv[0]);
		exit(0);
	}

}//void CalledByAlias(int iArgc,char *cArgv[])


void PrintUsage(char *arg0)
{
	printf("iDNS (%s) CLI Menu (C) 2001-2017 Unixservice, LLC.\n",cGitVersion);
	printf("Database Ops:\n");
	printf("\tInitialize <mysql root passwd>\n");
	printf("\tBackup|RestoreAll [<mysql root passwd>]\n");
	printf("\tRestore <Restore table name> [<mysql root passwd>]\n");
	printf("Crontab Ops:\n");
	printf("\tProcessJobQueue <fqdn master ns>\n");
	printf("\tProcessServerJobQueue <fqdn server>\n");
	printf("\tProcessExtJobQueue <fqdn server as in mysqlISP>\n");
	printf("\tProcessVZJobQueue\n");
	printf("\tjobqueue master <fqdn master ns> (equivalent to ProcessJobQueue)\n");
	printf("\tjobqueue slave <fqdn slave ns> <master ip>\n");
	printf("\tMonthHitData\n");
	printf("\tMonthUsageData <uSimile>\n");
	printf("\tDayUsageData <uLogType>\n");
	printf("Special Admin Ops:\n");
	printf("\tDebugMasterFile <fqdn master ns> <cZone>\n");
	printf("\tDebugSlaveFile <fqdn slave ns> <cZone> <master ip>\n");
	printf("\tallfiles master|slave <fqdn ns> <master ip>\n");
	printf("\tinstallbind <listen ipnum>\n");
	printf("\texport <table> <filename>\n");
	printf("\tListZones [<uNSSet>] [--reportNS]\n");
	printf("\tPerfQueryList\n");
	printf("\tPrintNSList <cuNSSet>\n");
	printf("\tPrintMXList <cuMailServer>\n");
	printf("\tCreateWebZone <cDomain> <cIP> [<cNSSet group label> <cMailServer group label>]\n");
	printf("\tDropZone <cDomain> [<cNSSet group label>]\n");
	printf("\tMassZoneUpdate\n");
	printf("\tMassZoneNSUpdate <cNSSet group label>\n");
	printf("\tUpdateSchema\n");
	printf("\tUpdateTables\n");
	printf("\tImportFromDb <source mysql db> <target mysql db> <mysql root passwd>\n");
	printf("\tExtracttLog <Mon> <Year> <mysql root passwd> <path to mysql table>\n");
	printf("\tExtracttHit <Mon> <Year> <mysql root passwd> <path to mysql table>\n");
	printf("\tExample args for Extracts: Apr 2007 passwd /var/lib/mysql/idns\n");
	printf("Special Import/Export Ops (Caution):\n");
	printf("\tImportTemplateFile <tTemplate.cLabel> <filespec> <tTemplateSet.cLabel> <tTemplateType.cLabel>\n");
	printf("\tExportTemplateFiles <dir> <tTemplateSet.cLabel> <tTemplateType.cLabel>\n");
	printf("\tImportZones\n");
	printf("\tImportAxfrZones\n");
	printf("\tCloneZonesFromList\n");
	printf("\tDropImportedZones\n");
	printf("\tImportCompanies\n");
	printf("\tDropCompanies\n");
	printf("\tImportUsers\n");
	printf("\tDropUsers\n");
	printf("\tImportBlocks\n");
	printf("\tDropBlocks\n");
	printf("\tAssociateCompaniesZones\n");
	printf("\tImportRegistrars\n");
	printf("\tDropRegistrars\n");
	printf("\tAssociateRegistrarsZones\n");
	printf("\tCompareZones <DNS server1 IP> <DNS server2 IP> [<uOwner>]\n");
	printf("\tImportSORRs\n");
	printf("\tExportRRCSV <company> [out file] <mysql root passwd>\n");
	printf("\tFixBlockOwnership\n");
	printf("\tCreatePBXZonesFromVZ\n");
	printf("\tDeletePBXZones\n");
	exit(0);

}//void PrintUsage(char *arg0)


void ExtMainShell(int argc, char *argv[])
{
	//char cCmdLineACL[256]={""};

	//Pre ACL
	if(argc==3 && !strcmp(argv[1],"Initialize"))
	{
		Initialize(argv[2]);
		exit(0);
	}

	if(getuid())
	{
		printf("Sorry! must be root to run iDNS.cgi command line.\n");
		exit(1);
	}


	if(argc==2)
	{
		if(!strcmp(argv[1],"Backup"))
		{
                	Backup("");
			exit(0);
		}
		else if(!strcmp(argv[1],"PerfQueryList"))
		{
                	PerfQueryList();
			exit(0);
		}
		else if(!strcmp(argv[1],"ListZones"))
		{
                	ListZones(0,0);
			exit(0);
		}
        	else if(!strcmp(argv[1],"RestoreAll"))
		{
                	RestoreAll("");
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportZones"))
		{
			if(TextConnectDb()) exit(0);
			Import();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportAxfrZones"))
		{
			if(TextConnectDb()) exit(0);
			ImportAxfr();
			exit(0);
		}
		else if(!strcmp(argv[1],"CloneZonesFromList"))
		{
			if(TextConnectDb()) exit(0);
			CloneZonesFromList();
			exit(0);
		}
		else if(!strcmp(argv[1],"DropImportedZones"))
		{
			if(TextConnectDb()) exit(0);
			DropImportedZones();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportCompanies"))
		{
			if(TextConnectDb()) exit(0);
			ImportCompanies();
			exit(0);
		}
		else if(!strcmp(argv[1],"DropCompanies"))
		{
			if(TextConnectDb()) exit(0);
			DropCompanies();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportUsers"))
		{
			if(TextConnectDb()) exit(0);
			ImportUsers();
			exit(0);
		}
		else if(!strcmp(argv[1],"DropUsers"))
		{
			if(TextConnectDb()) exit(0);
			DropUsers();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportBlocks"))
		{
			if(TextConnectDb()) exit(0);
			ImportBlocks();
			exit(0);
		}
		else if(!strcmp(argv[1],"DropBlocks"))
		{
			if(TextConnectDb()) exit(0);
			DropBlocks();
			exit(0);
		}
		else if(!strcmp(argv[1],"AssociateCompaniesZones"))
		{
			if(TextConnectDb()) exit(0);
			AssociateCompaniesZones();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportRegistrars"))
		{
			if(TextConnectDb()) exit(0);
			ImportRegistrars();
			exit(0);
		}
		else if(!strcmp(argv[1],"DropRegistrars"))
		{
			if(TextConnectDb()) exit(0);
			DropRegistrars();
			exit(0);
		}
		else if(!strcmp(argv[1],"AssociateRegistrarsZones"))
		{
			if(TextConnectDb()) exit(0);
			AssociateRegistrarsZones();
			exit(0);
		}
		else if(!strcmp(argv[1],"MassZoneUpdate"))
		{
			if(TextConnectDb()) exit(0);
			MassZoneUpdate();
			exit(0);
		}
		else if(!strcmp(argv[1],"UpdateSchema"))
		{
			if(TextConnectDb()) exit(0);
			UpdateSchema();
			exit(0);
		}
		else if(!strcmp(argv[1],"UpdateTables"))
		{
			if(TextConnectDb()) exit(0);
			UpdateTables();
			exit(0);
		}
		else if(!strcmp(argv[1],"MonthHitData"))
		{
			if(TextConnectDb()) exit(0);
                	MonthHitData();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportSORRs"))
		{
			if(TextConnectDb()) exit(0);
			ImportSORRs();
			exit(0);
		}
		else if(!strcmp(argv[1],"CheckAllZones"))
		{
			if(TextConnectDb()) exit(0);
			CheckAllZones();
			exit(0);
		}
		else if(!strcmp(argv[1],"FixBlockOwnership"))
		{
			if(TextConnectDb()) exit(0);
			FixBlockOwnership();
			exit(0);
		}
		else if(!strcmp(argv[1],"ProcessVZJobQueue"))
		{
			ProcessVZJobQueue();
			exit(0);
		}
		else if(!strcmp(argv[1],"CreatePBXZonesFromVZ"))
		{
			CreatePBXZonesFromVZ();
			exit(0);
		}
		else if(!strcmp(argv[1],"DeletePBXZones"))
		{
			DeletePBXZones();
			exit(0);
		}
	}
	else if(argc==3)
	{
		if(!strcmp(argv[1],"ProcessJobQueue"))
		{
			MasterJobQueue(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ProcessServerJobQueue"))
		{
			ServerJobQueue(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ProcessExtJobQueue"))
		{
			ProcessExtJobQueue(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"installbind"))
		{
			InstallNamedFiles(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"Initialize"))
		{
			Initialize(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"Backup"))
		{
                	Backup(argv[2]);
			exit(0);
		}
        	else if(!strcmp(argv[1],"RestoreAll"))
		{
                	RestoreAll(argv[2]);
			exit(0);
		}
        	else if(!strcmp(argv[1],"Restore"))
		{
                	Restore(argv[2],"");
			exit(0);
		}
		else if(!strcmp(argv[1],"PrintNSList"))
		{
			if(TextConnectDb()) exit(0);
                	cPrintNSList(stdout,argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"PrintMXList"))
		{
			if(TextConnectDb()) exit(0);
                	PrintMXList(stdout,argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"DropZone"))
		{
			if(TextConnectDb()) exit(0);
			DropZone(argv[2],"");
			exit(0);
		}
		else if(!strcmp(argv[1],"MassZoneNSUpdate"))
		{
			if(TextConnectDb()) exit(0);
			MassZoneNSUpdate(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"DayUsageData"))
		{
			unsigned uLogType=0;

			if(TextConnectDb()) exit(0);
			sscanf(argv[2],"%u",&uLogType);
			if(uLogType)
			{
                		DayUsageData(uLogType);
				exit(0);
			}
		}
		else if(!strcmp(argv[1],"MonthUsageData"))
		{
			unsigned uSimile=0;
			if(TextConnectDb()) exit(0);
			sscanf(argv[2],"%u",&uSimile);
                	MonthUsageData(uSimile);
			exit(0);
		}
		else if(!strcmp(argv[1],"ListZones"))
		{
			unsigned uNSSet=0;
			sscanf(argv[2],"%u",&uNSSet);
                	ListZones(uNSSet,0);
			exit(0);
		}
		PrintUsage(argv[0]);
	}
	else if(argc==4)
	{
		if(!strcmp(argv[1],"jobqueue"))
		{
			if(!strcmp(argv[2],"master"))
				MasterJobQueue(argv[3]);
		}
		else if(!strcmp(argv[1],"export"))
		{
			ExportTable(argv[2],argv[3]);
		}
        	else if(!strcmp(argv[1],"Restore"))
		{
                	Restore(argv[3],argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"CreateWebZone"))
		{
			if(TextConnectDb()) exit(0);
			CreateWebZone(argv[2],argv[3],"","");
			exit(0);
		}
		else if(!strcmp(argv[1],"DropZone"))
		{
			if(TextConnectDb()) exit(0);
			DropZone(argv[2],argv[3]);
			exit(0);
		}
		else if(!strcmp(argv[1],"DebugMasterFile"))
		{
			if(TextConnectDb()) exit(0);
			CreateMasterFiles(argv[2],argv[3],1,1,1);
			exit(0);
		}
		else if(!strcmp(argv[1],"CompareZones"))
		{
			if(TextConnectDb()) exit(0);
			CompareZones(argv[2],argv[3],"");
			exit(0);
		}
		else if(!strcmp(argv[1],"ExportRRCSV"))
		{
			mySQLRootConnect(argv[3]);
			ExportRRCSV(argv[2],"");
		}
		else if(!strcmp(argv[1],"ListZones") && !strcmp(argv[3],"--reportNS"))
		{
			unsigned uNSSet=0;
			sscanf(argv[2],"%u",&uNSSet);
                	ListZones(uNSSet,1);
			exit(0);
		}
		PrintUsage(argv[0]);
	}
	else if(argc==5)
	{
		if(!strcmp(argv[1],"allfiles"))
		{
			if(!strcmp(argv[2],"slave"))
			{
				if(TextConnectDb()) exit(0);
				CreateSlaveFiles(argv[3],"",argv[4],0);//All
				exit(0);
			}
			else if(!strcmp(argv[2],"master"))
			{
				if(TextConnectDb()) exit(0);
				//All+DBFiles+Stubs
				CreateMasterFiles(argv[3],"",1,1,0);
				exit(0);
			}
			//Note this is missing the optional port and path that bind.c uses
			system("/usr/sbin/rndc -c /etc/unxsbind-rndc.conf reload");
			exit(0);
		}
		else if(!strcmp(argv[1],"jobqueue"))
		{
			if(!strcmp(argv[2],"slave"))
			{
				SlaveJobQueue(argv[3],argv[4]);
				exit(0);
			}
		}
		else if(!strcmp(argv[1],"CreateWebZone"))
		{
			if(TextConnectDb()) exit(0);
			CreateWebZone(argv[2],argv[3],argv[4],"");
			exit(0);
		}
		else if(!strcmp(argv[1],"DebugSlaveFile"))
		{
			if(TextConnectDb()) exit(0);
			CreateSlaveFiles(argv[2],argv[3],argv[4],1);
			exit(0);
		}
		else if(!strcmp(argv[1],"CompareZones"))
		{
			if(TextConnectDb()) exit(0);
			//Optional uOwner to limit zones to compare
			CompareZones(argv[2],argv[3],argv[4]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportFromDb"))
		{
			ImportFromDb(argv[2],argv[3],argv[4]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ExportRRCSV"))
		{
			mySQLRootConnect(argv[4]);
			ExportRRCSV(argv[2],argv[3]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ExportTemplateFiles"))
		{
                	ExportTemplateFiles(argv[2],argv[3],argv[4]);
			exit(0);
		}
		PrintUsage(argv[0]);
	}
	else if(argc==6)
	{
		if(!strcmp(argv[1],"CreateWebZone"))
		{
			if(TextConnectDb()) exit(0);
			CreateWebZone(argv[2],argv[3],argv[4],argv[5]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ExtracttLog"))
		{
                	ExtracttLog(argv[2],argv[3],argv[4],argv[5]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ExtracttHit"))
		{
                	ExtracttHit(argv[2],argv[3],argv[4],argv[5]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportTemplateFile"))
		{
                	ImportTemplateFile(argv[2],argv[3],argv[4],argv[5]);
			exit(0);
		}
		PrintUsage(argv[0]);
	}
	PrintUsage(argv[0]);

}//void ExtMainShell(int argc, char *argv[])


void Tutorial(void)
{
	Header_ism3("Tutorial",0);
	printf("</center><blockquote>");
	PassDirectHtml("idns.tutorial.txt");
	printf("</blockquote>");
	Footer_ism3();

}//void Tutorial(void)


void NamedConf(void)
{
	Header_ism3("NamedConf()",0);
	printf("</center><pre><blockquote>");
	PassDirectHtml("/usr/local/idns/named.conf");
	printf("</blockquote></pre>");
	Footer_ism3();

}//void NamedConf(void)


void MasterZones(void)
{
	Header_ism3("MasterZones()()",0);
	printf("</center><pre><blockquote>");
	PassDirectHtml("/usr/local/idns/named.d/master.zones");
	printf("</blockquote></pre>");
	Footer_ism3();

}//void MasterZones(void)


void Admin(void)
{
	Header_ism3("Admin",0);
	printf("<table width=900><tr><td>");
	printf("<form action=iDNS.cgi method=post>");
	
	printf("<input type=hidden name=gcFunction value=MainTools>");

	if(guPermLevel>7)
	{
		printf("<br><input title=\"View this system's named.conf file\" class=largeButton type=submit"
			" name=gcCommand value=NamedConf><br>");
		printf("<br><input title=\"View this system's master.zones file\" class=largeButton type=submit"
			" name=gcCommand value=MasterZones><br>");
		printf("<br><input title='Tutorial' class=largeButton type=submit name=gcCommand value=Tutorial>");
		if(guPermLevel>11 && guLoginClient==1 && !guZeroSystem)
			printf("<p><input title='DANGER truncates all tables, then installs distro init data from"
			" /usr/local/share/iDNS/data'"
			" class=lwarnButton type=submit name=gcCommand value='Zero System'>");
		if(guPermLevel>11 && guLoginClient==1 && guZeroSystem)
			printf("<p><input title='DANGER truncates all tables, then installs distro init data from"
			" /usr/local/share/iDNS/data'"
			" class=lwarnButton type=submit name=gcCommand value='Zero System Confirm'>");
	}


	if(guPermLevel>=10)
		voidhtmlListBackupLinks();

	printf("</td></tr></table></form>");
	Footer_ism3();

}//void Admin(void)


//This needs a security review
//you need to setup sudoers
//
//apache    ALL=(ALL)    NOPASSWD:/usr/bin/mysqldump
//apache    ALL=(ALL)    NOPASSWD:/usr/sbin/idns-restore.sh
//Defaults:apache !requiretty
//
void voidRestoreBackup(char *cFile)
{
	if(guPermLevel>=12)
	{
		char cPath[256];
		sprintf(cPath,"sudo /usr/sbin/idns-restore.sh /usr/local/idns/backups/%.128s",cFile);	
		if(system(cPath))
			guAddBackup=0;	
	}
	Admin();
}//void voidRestoreBackup(void)


void voidDeleteBackup(char *cFile)
{
	if(guPermLevel>=12)
	{
		char cPath[256];
		sprintf(cPath,"/usr/local/idns/backups/%.128s",cFile);	
		unlink(cPath);
	}
	Admin();
}//void voidDeleteBackup(void)

//Only allow 1 backup per hour for DevOps/Monitoring disk safety.
void voidAddBackup(void)
{
	if(guPermLevel>=10)
	{
		if(system("sudo /usr/bin/mysqldump -uidns -pwsxedc idns | gzip > "
			" /usr/local/idns/backups/idns.$(date +\"%Y%m%d%H\").mysqldump.gz"))
			guAddBackup=0;	
	}
	else
	{
		guAddBackup=0;
	}

}//void voidAddBackup(void)


void voidhtmlListBackupLinks(void)
{
	printf("\n<fieldset><legend><b>Backup Links</b></legend>");
	printf("<input class=largeButton type=submit name=gcCommand value='Add Backup'"
		" title='Add another mysqldump backup to your backup list. This only works if the server you are running"
		" your iDNS backend on is also running your backend MySQL DB'>\n");
	if(!guAddBackup)
		printf(" Failed!\n");
	printf("<br><table><tr bgcolor=black>"
		"<td><font color=white>Backup file name"
		"<td><font color=white>Size"
		"<td><font color=white>Date"
		"<td><font color=white>Restore link"
		"<td><font color=white>Delete link"
			"</tr>\n");

	#include <dirent.h> 
	DIR *d;
	struct dirent *dir;
	d=opendir("/usr/local/idns/backups/");
	if(d)
	{
		unsigned uCount=0;
		while((dir=readdir(d)) != NULL)
		{
			if(strstr(dir->d_name,".mysqldump"))
			{
				struct stat stbuf;
				char cFile[128]={""};
				sprintf(cFile,"/usr/local/idns/backups/%s",dir->d_name);
				if(!stat(cFile,&stbuf))
				{
					if(guPermLevel>=12)
						printf("<tr><td>%1$s<td>%2$lu<td>%3$s<td><a class=darkLink href=?resBackup=%1$s >Restore</a>"
						"<td><a class=darkLink href=?delBackup=%1$s ><font color=red>Delete</a></tr><br>\n",
							dir->d_name,stbuf.st_size,ctime(&stbuf.st_ctime));
					else
						printf("<tr><td>%1$s<td>%2$lu<td>%3$s<td>n/a</a>"
						"<td>n/a</tr><br>\n",
							dir->d_name,stbuf.st_size,ctime(&stbuf.st_ctime));
					uCount++;
				}
			}
		}
		closedir(d);
		if(!uCount)
			printf("Backup directory <i>/usr/local/idns/backups/</i> is empty\n");
	}
	else
	{
		printf("Directory <i>/usr/local/idns/backups/</i> does not exist or has incorrect permissions!\n");
	}
	printf("</table></fieldset>\n");

}//void voidhtmlListBackupLinks()


//List zones for a given uNSSet
char *cNSFromWhois(char const *cZone,char const *cuNSSet);//tzonefunc.h
void ListZones(unsigned uNSSet,unsigned uNSReport)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	if(TextConnectDb()) exit(0);

	if(uNSSet && !uNSReport)
		sprintf(gcQuery,"SELECT cZone,cLabel FROM tZone,tNSSet"
					" WHERE tZone.uNSSet=tNSSet.uNSSet"
					" AND tNSSet.uNSSet=%u"
					" ORDER BY tZone.cZone",uNSSet);
	else if(uNSSet && uNSReport)
		sprintf(gcQuery,"SELECT cZone,cLabel,tZone.uNSSet FROM tZone,tNSSet"
					" WHERE tZone.uNSSet=tNSSet.uNSSet"
					" AND tZone.cZone NOT LIKE '%%.arpa'"
					" AND tZone.uSecondaryOnly=0"
					" AND tNSSet.uNSSet=%u"
					" ORDER BY tZone.cZone",uNSSet);
	else
		sprintf(gcQuery,"SELECT cZone FROM tZone ORDER BY cZone");
		
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	unsigned uOnlyOnce=1;
	while((field=mysql_fetch_row(res))) 
	{
		if(uNSSet && uOnlyOnce)
		{
			if(uNSSet)
			{
				if(uNSReport)
					printf("NSSet:%s reporting NS info\n",field[1]);
				else
					printf("NSSet:%s\n",field[1]);
			}
			uOnlyOnce=0;
		}
		if(uNSSet)
		{
			if(uNSReport)
				printf("%s, %s\n",field[0],cNSFromWhois(field[0],field[2]));
			else
				printf("%s\n",field[0]);
		}
		else
		{
			printf("%s\n",field[0]);
		}
	}
	mysql_free_result(res);
	exit(0);

}//void ListZones()


void CompareZones(char *cDNSServer1IP, char *cDNSServer2IP, char *cuOwner)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cZone[255]={""};

	if(cuOwner[0])
		sprintf(gcQuery,"SELECT tRRType.cLabel,tResource.cName,tZone.cZone FROM tResource,tRRType,tZone"
				" WHERE tResource.uZone=tZone.uZone AND tResource.uRRType=tRRType.uRRType AND"
				" tZone.uOwner=%s ORDER BY tZone.cZone",cuOwner);
	else
		sprintf(gcQuery,"SELECT tRRType.cLabel,tResource.cName,tZone.cZone FROM tResource,tRRType,tZone"
				" WHERE tResource.uZone=tZone.uZone AND tResource.uRRType=tRRType.uRRType ORDER BY"
				" tZone.cZone");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res))) 
	{
		if(strcmp(cZone,field[2]))
		{
			strcpy(cZone,field[2]);
			printf("%s\n",cZone);
		}

		if(field[1][0])
			sprintf(gcQuery,"dig @%s %s %s.%s +noall +answer | awk '{print $1,$3,$4,$5,$6,$7,$8}'"
					" > /tmp/idns.cz1 2>&1",
					cDNSServer1IP,field[0],field[1],field[2]);
		else
			sprintf(gcQuery,"dig @%s %s %s +noall +answer | awk '{print $1,$3,$4,$5,$6,$7,$8}'"
					" > /tmp/idns.cz1 2>&1",
					cDNSServer1IP,field[0],field[2]);
		system(gcQuery);
		//printf("%s\n",gcQuery);

		if(field[1][0])
			sprintf(gcQuery,"dig @%s %s %s.%s +noall +answer | awk '{print $1,$3,$4,$5,$6,$7,$8}'"
					" > /tmp/idns.cz2 2>&1",
					cDNSServer2IP,field[0],field[1],field[2]);
		else
			sprintf(gcQuery,"dig @%s %s %s +noall +answer | awk '{print $1,$3,$4,$5,$6,$7,$8}'"
					" > /tmp/idns.cz2 2>&1",
					cDNSServer2IP,field[0],field[2]);
		system(gcQuery);
		//printf("%s\n",gcQuery);

		if(system("diff --ignore-all-space /tmp/idns.cz1 /tmp/idns.cz2 > /dev/null 2>&1"))
		{
			system("cat /tmp/idns.cz1 /tmp/idns.cz2");
		}
			
	}
	mysql_free_result(res);

}//void CompareZones()


void UpdateSchema(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned ucMessage=1;
	unsigned ucServer=1;
	//unsigned uNameServer=0;
	unsigned uNSSet=1;
	unsigned uZoneImportNSSet=1;
	unsigned uViewNSSet=1;
	unsigned uJobNSSet=1;
	unsigned uDeletedZoneNSSet=1;
	unsigned uClient=1;
	unsigned uClientDel=1;
	unsigned ucParam3=1;
	unsigned ucParam4=1;

	printf("UpdateSchema() start\n");

	//Gather current schema info for new columns and new or complex index mods/changes
	//Modifies do nothing bad but in the future for the sake of efficiency we should
	//	also not repeat them.
	//Organized by table not in alphabetical order yet TODO


	//tAuthorize
	//tAuthorize section
	unsigned uAuthorizecOTPSecret=0;
	unsigned uAuthorizeuOTPExpire=0;
	sprintf(gcQuery,"SHOW COLUMNS IN tAuthorize");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		printf("%s\n",mysql_error(&gMysql));
	mysql_query(&gMysql,gcQuery);
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"uOTPExpire"))
			uAuthorizeuOTPExpire=1;
		if(!strcmp(field[0],"cOTPSecret"))
			uAuthorizecOTPSecret=1;
	}
       	mysql_free_result(res);
	if(!uAuthorizeuOTPExpire)
	{
		sprintf(gcQuery,"ALTER TABLE tAuthorize ADD uOTPExpire INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			printf("%s\n",mysql_error(&gMysql));
		else
			printf("Added uOTPExpires to tAuthorize\n");
	}
	if(!uAuthorizecOTPSecret)
	{
		sprintf(gcQuery,"ALTER TABLE tAuthorize ADD cOTPSecret VARCHAR(64) NOT NULL DEFAULT ''");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			printf("%s\n",mysql_error(&gMysql));
		else
			printf("Added cOTPSecret to tAuthorize\n");
	}

	//tView
	sprintf(gcQuery,"SHOW COLUMNS IN tView");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"uNSSet")) uViewNSSet=0;
	}
       	mysql_free_result(res);
	if(uViewNSSet)
	{
		sprintf(gcQuery,"ALTER TABLE tView ADD uNSSet INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tLog
	sprintf(gcQuery,"SHOW COLUMNS IN tLog");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"cMessage")) ucMessage=0;
		else if(!strcmp(field[0],"cServer")) ucServer=0;
	}
       	mysql_free_result(res);
	if(ucMessage)
	{
		sprintf(gcQuery,"ALTER TABLE tLog ADD cMessage VARCHAR(255) NOT NULL DEFAULT ''");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}
	if(ucServer)
	{
		sprintf(gcQuery,"ALTER TABLE tLog ADD cServer VARCHAR(64) NOT NULL DEFAULT ''");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tHit
	sprintf(gcQuery,"ALTER TABLE tHit MODIFY cZone VARCHAR(255) NOT NULL DEFAULT ''");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
		printf("%s\n",mysql_error(&gMysql));

	//tJob
	sprintf(gcQuery,"SHOW COLUMNS IN tJob");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"uNSSet")) uJobNSSet=0;
	}
       	mysql_free_result(res);
	if(uJobNSSet)
	{
		sprintf(gcQuery,"ALTER TABLE tJob ADD uNSSet INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tZoneImport
	sprintf(gcQuery,"SHOW COLUMNS IN tZoneImport");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"uNSSet")) uZoneImportNSSet=0;
	}
       	mysql_free_result(res);
	if(uZoneImportNSSet)
	{
		sprintf(gcQuery,"ALTER TABLE tZoneImport ADD uNSSet INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tZone
	sprintf(gcQuery,"SHOW COLUMNS IN tZone");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"uNSSet")) uNSSet=0;
		else if(!strcmp(field[0],"uClient")) uClient=0;
		//else if(!strcmp(field[0],"uNameServer")) uNameServer=1;//Note rev logic
	}
       	mysql_free_result(res);
	if(uNSSet)
	{
		sprintf(gcQuery,"ALTER TABLE tZone ADD uNSSet INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}
	if(uClient)
	{
		sprintf(gcQuery,"ALTER TABLE tZone ADD uClient INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tDeletedZone
	sprintf(gcQuery,"SHOW COLUMNS IN tDeletedZone");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"uClient")) uClientDel=0;
		else if(!strcmp(field[0],"uNSSet")) uDeletedZoneNSSet=0;
	}
       	mysql_free_result(res);
	if(uClientDel)
	{
		sprintf(gcQuery,"ALTER TABLE tDeletedZone ADD uClient INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}
	if(uDeletedZoneNSSet)
	{
		sprintf(gcQuery,"ALTER TABLE tDeletedZone ADD uNSSet INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tDeletedResource
	sprintf(gcQuery,"SHOW COLUMNS IN tDeletedResource");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"cParam3")) ucParam3=0;
		else if(!strcmp(field[0],"cParam4")) ucParam4=0;
	}
       	mysql_free_result(res);
	if(ucParam3)
	{
		sprintf(gcQuery,"ALTER TABLE tDeletedResource ADD cParam3 VARCHAR(255) NOT NULL DEFAULT ''");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}
	if(ucParam4)
	{
		sprintf(gcQuery,"ALTER TABLE tDeletedResource ADD cParam4 VARCHAR(255) NOT NULL DEFAULT ''");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}
	
	//
	//Upgrading from very old pre tNSSet versions
	//Check for tNameServer
	/*
	unsigned utNameServer=0;
	sprintf(gcQuery,"SHOW TABLES");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"tNameServer")) utNameServer=1;
	}
       	mysql_free_result(res);
	*/
	//if(utNameServer && uNameServer)//uNameServer set in tZone check above
	if(0)//Off!
	{
		unsigned uNSSet=0;

		printf("\tStarting conversion to tNSSet version from tNameServer data\n");
		sprintf(gcQuery,"SELECT cLabel,cList,cMasterIPs,uOwner FROM tNameServer");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		res=mysql_store_result(&gMysql);
		while((field=mysql_fetch_row(res)))
		{
			printf("\t\t%s\n",field[0]);
			//Create single new tNSSet from cLabel
			uNSSet=NewNSSet(field[0],field[2],field[3]);

			//Create tNS entries from cList
			if(field[1][0])
			{
				register int i,j=0;
				char cFQDN[100];
				char cNSType[33];
				char *cp;

				//Parse each line get string for tNS.cFQDN and uNSType
				for(i=0;field[1][i];i++)
				{
					cNSType[0]=0;
					cFQDN[j++]=field[1][i];
					if(field[1][i]=='\r' || field[1][i]=='\n')
					{
						cFQDN[j-1]=0;
						if((cp=strchr(cFQDN,' ')))
						{
							*cp=0;
							sprintf(cNSType,"%.32s",cp+1);
						}
						printf("\t\t\t%s %s\n",cFQDN,cNSType);
						NewNS(cFQDN,cNSType,uNSSet);
						j=0;
					}
				}
				if(cFQDN[j])
				{
					cFQDN[j-1]=0;
					if((cp=strchr(cFQDN,' ')))
					{
						*cp=0;
						sprintf(cNSType,"%.32s",cp+1);
					}
					printf("\t\t\t%s %s\n",cFQDN,cNSType);
					NewNS(cFQDN,cNSType,uNSSet);
				}
			}
			//Not sure yet about cMasterIPs
		}
	       	mysql_free_result(res);

		//Set tZone.uNSSet from tZone.uNameServer
		sprintf(gcQuery,"SELECT tZone.uZone,tNSSet.uNSSet FROM tZone,tNameServer,tNSSet"
				" WHERE tZone.uNameServer=tNameServer.uNameServer AND tNameServer.cLabel=tNSSet.cLabel"
				" AND tZone.uNSSet=0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		res=mysql_store_result(&gMysql);
		while((field=mysql_fetch_row(res)))
		{
			sprintf(gcQuery,"UPDATE tZone SET uNSSet=%s WHERE uZone=%s",field[1],field[0]);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				printf("%s\n",mysql_error(&gMysql));
				exit(1);
			}
		}
	       	mysql_free_result(res);

		printf("\tEnd of tNameServer based conversion\n");
	}
	printf("UpdateSchema() end\n");

}//void UpdateSchema(void)


void UpdateTables(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uMax=0;
	char cCommand[256];
	char *cDBIP="localhost";
	char cSQLPath[100]={"/usr/local/share/iDNS/data"};

	//1-. We need a way to update fixed tables like tRRType
	//without disruption. Small tables can be updated directly
	//via fixed queries, but that seems like bloat to me.

	//2-. We need a way to add new template sets, like for vdnsOrg
	//for example. This can be done with special data/*.sql files.
	//These files can be included in source distributions and also
	//provided by rpm/yum. But how to we make sure we don't update
	//more than once? We can check here and then do a system call to mysql.

	//3-. This should be a first step to deprecating the data/*.txt
	//old mess we inherited 10 years ago.

	//ISMROOT should also be changed to unxsVZDataPath or similar.

	//DBIP0 must be up an running OR localhost must be up and running.
	if(DBIP0!=NULL && DBIP1!=NULL)
		cDBIP=DBIP0;

	if(getenv("ISMROOT")!=NULL)
		sprintf(cSQLPath,"%.99s/iDNS/data",getenv("ISMROOT"));
	//Else we use the unxsbind.spec path above

	printf("UpdateTables(%s) start\n",cDBIP);

	//tGlossary always updated
	sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tGlossary.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
	if(system(cCommand))
	{
		printf("Error: %s\n",cCommand);
		exit(1);
	}
	printf("Updated tGlossary.\n");

	//tRRType
	//see mysqlrad.h defines
	uMax=RRTYPE_NAPTR;
	sprintf(gcQuery,"SELECT MAX(uRRType) FROM tRRType");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		if(field[0]!=NULL) sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	if(uMax<RRTYPE_NAPTR)
	{
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tRRType.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tRRType.\n");
	}

	//tNSType
#define NSTYPE_MASTER 1
#define NSTYPE_MHIDDEN 2
#define NSTYPE_MEXTERN 3
#define NSTYPE_SLAVE 4
	//Upgrade from very old version will need these tables first
	CreatetNSType();
	CreatetNSSet();
	CreatetNS();
	CreatetServer();
	CreatetGroup();
	CreatetGroupType();
	CreatetGroupGlue();
	uMax=0;
	sprintf(gcQuery,"SELECT MAX(uNSType) FROM tNSType");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		if(field[0]!=NULL) sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	if(uMax==0 || uMax<NSTYPE_SLAVE)
	{
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tNSType.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tNSType.\n");
	}

	//tLogType
#define LOGTYPE_IDNSORG_LOGIN 8
	uMax=LOGTYPE_IDNSORG_LOGIN;
	sprintf(gcQuery,"SELECT MAX(uLogType) FROM tLogType");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		if(field[0]!=NULL) sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	if(uMax<LOGTYPE_IDNSORG_LOGIN)
	{
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tLogType.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tLogType.\n");
	}


	//tTemplateType
#define TEMPLATETYPE_VDNSORG 3
	uMax=TEMPLATETYPE_VDNSORG;
	sprintf(gcQuery,"SELECT MAX(uTemplateType) FROM tTemplateType");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		if(field[0]!=NULL) sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	if(uMax<TEMPLATETYPE_VDNSORG)
	{
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tTemplateType.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tTemplateType.\n");
	}

	//tTemplateSet
#define TEMPLATESET_PLAIN 1
	uMax=TEMPLATESET_PLAIN;
	sprintf(gcQuery,"SELECT MAX(uTemplateSet) FROM tTemplateSet");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		if(field[0]!=NULL) sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	if(uMax<TEMPLATESET_PLAIN)
	{
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tTemplateSet.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tTemplateSet.\n");
	}

	//
	//tTemplate updates --much harder problem
	//For initial testing we will keep old no type (and probably no set) templates
	//The end user will have to deal with his mods post initial install accordingly.
	//tTemplate-vdnsOrg.sql example dump:
	//mysqldump --compact --no-create-info --where="uTemplateType=3" -psecret idns tTemplate
	//					 > data/tTemplate-vdnsOrg.sql
	//see mysqlrad.h defines
	uMax=TEMPLATETYPE_VDNSORG;
	sprintf(gcQuery,"SELECT MAX(uTemplateType) FROM tTemplate");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		if(field[0]!=NULL) sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	//We are assuming that for an rpm update user, if user does not have vdnsOrg templates, that this 
	//means the user needs all the other template tables updated also.
	//Any old template will still exist but will not have a type and probably not a set either
	//unless they modified them themselves.
	if(uMax<TEMPLATETYPE_VDNSORG)
	{
		//Save old templates, this is pretty much a temp hack.
		//It is needed since the .sql files have uTemplate values.
		sprintf(gcQuery,"UPDATE tTemplate SET uTemplate=uTemplate+1000");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}

		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tTemplate-vdnsOrg.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tTemplate-vdnsOrg.\n");
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tTemplate-idnsOrg.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tTemplate-idnsOrg. Old idnsOrg templates can still be found in tTemplate.\n");
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tTemplate-idnsAdmin.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tTemplate-idnsAdmin. Old idnsAdmin templates can still be found in tTemplate.\n");
	}

	printf("UpdateTables() end\n");

}//void UpdateTables(void)


void ExtracttLog(char *cMonth, char *cYear, char *cPasswd, char *cTablePath)
{
	char cTableName[33]={""};
	char cNextMonth[4]={""};
	char cNextYear[8]={""};
	char cThisYear[8]={""};
	char cThisMonth[4]={""};
	unsigned uRows=0;
	time_t clock, uStart=0, uEnd=0;
	struct tm *structTime;
	struct stat info;

	time(&clock);
	structTime=localtime(&clock);
	strftime(cThisYear,8,"%Y",structTime);
	strftime(cThisMonth,4,"%b",structTime);

	printf("ExtracttLog() Start\n");

	printf("cThisMonth:%s cThisYear:%s\n",cThisMonth,cThisYear);

	if(!strcmp(cThisMonth,cMonth) && !strcmp(cThisYear,cYear))
	{
		fprintf(stderr,"Can't extract this months data!\n");
		exit(1);
	}

	if(stat("/usr/bin/myisampack",&info) )
	{
		fprintf(stderr,"/usr/bin/myisampack is not installed!\n");
		exit(1);
	}

	if(stat("/usr/bin/myisamchk",&info))
	{
		fprintf(stderr,"/usr/bin/myisamchk is not installed!\n");
		exit(1);
	}


	mySQLRootConnect(cPasswd);
	mysql_query(&gMysql,"USE idns");
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	sprintf(gcQuery,"INSERT INTO tLog SET cMessage='ExtracttLog() Start...',cServer='%s',uLogType=4,uOwner=1,"
				"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",gcHostname);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stderr,"%s\n",mysql_error(&gMysql));

	sprintf(cTableName,"t%.3s%.7s",cMonth,cYear);

	sprintf(gcQuery,"1-%s-%s",cMonth,cYear);
	uStart=cDateToUnixTime(gcQuery);
	printf("Start: %s",ctime(&uStart));

	if(uStart== -1 || !uStart)
	{
		fprintf(stderr,"Garbled month year input (uStart)\n");
		exit(1);
	}

	NextMonthYear(cMonth,cYear,cNextMonth,cNextYear);
	//Debug only
	//printf("%s %s to %s %s\n",cMonth,cYear,cNextMonth,cNextYear);
	//exit(0);

	sprintf(gcQuery,"1-%s-%s",cNextMonth,cNextYear);
	uEnd=cDateToUnixTime(gcQuery);
	printf("End:   %s",ctime(&uEnd));
	if(uEnd== -1 || !uEnd)
	{
		fprintf(stderr,"Garbled month year input (uEnd)\n");
		exit(1);
	}

	
	CreatetLogTable(cTableName);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Clearing data from %s...\n",cTableName);
	sprintf(gcQuery,"DELETE FROM %s",cTableName);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Getting data from tLog...\n");
	sprintf(gcQuery,"INSERT %s (uLog,cLabel,uLogType,cHash,uPermLevel,uLoginClient,cLogin,cHost,uTablePK,cTableName,"
			"uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate) SELECT uLog,cLabel,uLogType,cHash,uPermLevel,"
			"uLoginClient,cLogin,cHost,uTablePK,cTableName,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate FROM"
			" tLog WHERE uCreatedDate>=%lu AND uCreatedDate<%lu",cTableName,uStart,uEnd);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	//If 0 records inserted delete from tMonth and exit now
	uRows=mysql_affected_rows(&gMysql);

	printf("Number of rows found and inserted: %u\n",uRows);

	if(uRows)
	{

		//This looks suspect, please check TODO
		sprintf(gcQuery,"REPLACE tMonth SET cLabel='%s',uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
	}
	else
	{
		sprintf(gcQuery,"DELETE FROM tMonth WHERE cLabel='%s'",cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}

		sprintf(gcQuery,"DROP TABLE %s",cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}


		printf("Exiting early no data to be archived\n");
		exit(0);
	}

	sprintf(gcQuery,"/usr/bin/myisampack %s/%s",cTablePath,cTableName);
	if(system(gcQuery))
	{
		fprintf(stderr,"Failed: %s\n",gcQuery);
		exit(1);
	}

	sprintf(gcQuery,"/usr/bin/myisamchk -rq %s/%s",cTablePath,cTableName);
	if(system(gcQuery))
	{
		fprintf(stderr,"Failed: %s\n",gcQuery);
		exit(1);
	}

	printf("Flushing compressed RO table...\n");
	sprintf(gcQuery,"FLUSH TABLE %s",cTableName);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Removing records from tLog...\n");
	sprintf(gcQuery,"DELETE QUICK FROM tLog WHERE uCreatedDate>=%lu AND uCreatedDate<%lu AND uLogType!=5",uStart,uEnd);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}


	printf("Extracted and Archived. Table flushed: %s\n",cTableName);
	printf("ExtracttLog() End\n");
	sprintf(gcQuery,"INSERT INTO tLog SET cMessage='ExtracttLog() End',cServer='%s',uLogType=4,uOwner=1,uCreatedBy=1,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW())",gcHostname);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
	exit(0);

}//void ExtracttLog(char *cMonth, char *cYear, char *cPasswd, char *cTablePath)


time_t cDateToUnixTime(char *cDate)
{
        struct  tm locTime;
        time_t  res;

        bzero(&locTime, sizeof(struct tm));
	if(strchr(cDate,'-'))
        	strptime(cDate,"%d-%b-%Y", &locTime);
	else if(strchr(cDate,'/'))
        	strptime(cDate,"%d/%b/%Y", &locTime);
	else if(strchr(cDate,' '))
        	strptime(cDate,"%d %b %Y", &locTime);
        locTime.tm_sec = 0;
        locTime.tm_min = 0;
        locTime.tm_hour = 0;
        res = mktime(&locTime);
        return(res);
}//time_t cDateToUnixTime(char *cDate)


//Another schema dependent item
void CreatetLogTable(char *cTableName)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS %s ( uTablePK VARCHAR(32) NOT NULL DEFAULT '', cHost VARCHAR(32) NOT NULL DEFAULT '', uLoginClient INT UNSIGNED NOT NULL DEFAULT 0, cLogin VARCHAR(32) NOT NULL DEFAULT '', uPermLevel INT UNSIGNED NOT NULL DEFAULT 0, cTableName VARCHAR(32) NOT NULL DEFAULT '', uLog INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(64) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, cHash VARCHAR(32) NOT NULL DEFAULT '', uLogType INT UNSIGNED NOT NULL DEFAULT 0,index (uLogType) )",cTableName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
}//CreatetLogTable()


void ExtracttHit(char *cMonth, char *cYear, char *cPasswd, char *cTablePath)
{
	char cTableName[33]={""};
	char cNextMonth[4]={""};
	char cNextYear[8]={""};
	char cThisYear[8]={""};
	char cThisMonth[4]={""};
	unsigned uRows=0;
	time_t clock, uStart=0, uEnd=0;
	struct tm *structTime;
	struct stat info;

	time(&clock);
	structTime=localtime(&clock);
	strftime(cThisYear,8,"%Y",structTime);
	strftime(cThisMonth,4,"%b",structTime);

	printf("ExtracttHit() Start\n");
	printf("cThisMonth:%s cThisYear:%s\n",cThisMonth,cThisYear);

	if(!strcmp(cThisMonth,cMonth) && !strcmp(cThisYear,cYear))
	{
		fprintf(stderr,"Can't extract this months data!\n");
		exit(1);
	}

	if(stat("/usr/bin/myisampack",&info) )
	{
		fprintf(stderr,"/usr/bin/myisampack is not installed!\n");
		exit(1);
	}

	if(stat("/usr/bin/myisamchk",&info))
	{
		fprintf(stderr,"/usr/bin/myisamchk is not installed!\n");
		exit(1);
	}


	mySQLRootConnect(cPasswd);
	mysql_query(&gMysql,"USE idns");
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	sprintf(gcQuery,"INSERT INTO tLog SET cMessage='ExtracttHit() Start...',cServer='%s',uLogType=4,uOwner=1,"
			"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",gcHostname);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stderr,"%s\n",mysql_error(&gMysql));

	sprintf(cTableName,"t%.3s%.7sHit",cMonth,cYear);

	sprintf(gcQuery,"1-%s-%s",cMonth,cYear);
	uStart=cDateToUnixTime(gcQuery);
	printf("Start: %s",ctime(&uStart));

	if(uStart== -1 || !uStart)
	{
		fprintf(stderr,"Garbled month year input (uStart)\n");
		exit(1);
	}

	NextMonthYear(cMonth,cYear,cNextMonth,cNextYear);
	//Debug only
	//printf("%s %s to %s %s\n",cMonth,cYear,cNextMonth,cNextYear);
	//exit(0);

	sprintf(gcQuery,"1-%s-%s",cNextMonth,cNextYear);
	uEnd=cDateToUnixTime(gcQuery);
	printf("End:   %s",ctime(&uEnd));
	if(uEnd== -1 || !uEnd)
	{
		fprintf(stderr,"Garbled month year input (uEnd)\n");
		exit(1);
	}

	CreatetHitTable(cTableName);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Clearing (truncate) data from %s...\n",cTableName);
	sprintf(gcQuery,"TRUNCATE %s",cTableName);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Getting data from tHit...\n");
	sprintf(gcQuery,"INSERT %s (uHit,cZone,uHitCount,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate,uSuccess,"
			"uReferral,uNxrrset,uNxdomain,uRecursion,uFailure,cHost) SELECT uHit,cZone,uHitCount,uOwner,"
			"uCreatedBy,uCreatedDate,uModBy,uModDate,uSuccess,uReferral,uNxrrset,uNxdomain,uRecursion,"
			"uFailure,cHost FROM tHit",cTableName);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	//If 0 records inserted delete from tMonth and exit now
	uRows=mysql_affected_rows(&gMysql);

	printf("Number of rows found and inserted: %u\n",uRows);

	if(uRows)
	{

		//TODO check this
		sprintf(gcQuery,"REPLACE tMonthHit SET cLabel='%s',uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
	}
	else
	{
		sprintf(gcQuery,"DELETE FROM tMonthHit WHERE cLabel='%s'",cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}

		sprintf(gcQuery,"DROP TABLE %s",cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}


		printf("Exiting early no data to be archived\n");
		exit(0);
	}

	sprintf(gcQuery,"/usr/bin/myisampack %s/%s",cTablePath,cTableName);
	if(system(gcQuery))
	{
		fprintf(stderr,"Failed: %s\n",gcQuery);
		exit(1);
	}

	sprintf(gcQuery,"/usr/bin/myisamchk -rq %s/%s",cTablePath,cTableName);
	if(system(gcQuery))
	{
		fprintf(stderr,"Failed: %s\n",gcQuery);
		exit(1);
	}

	printf("Flushing compressed RO table...\n");
	sprintf(gcQuery,"FLUSH TABLE %s",cTableName);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Removing records from tHit...\n");
	sprintf(gcQuery,"TRUNCATE tHit");
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Extracted and Archived. Table flushed: %s\n",cTableName);
	printf("ExtracttHit() End\n");
	sprintf(gcQuery,"INSERT INTO tLog SET cMessage='ExtracttHit() End',cServer='%s',uLogType=4,uOwner=1,uCreatedBy=1,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW())",gcHostname);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
	exit(0);

}//void ExtracttHit(char *cMonth, char *cYear, char *cPasswd, char *cTablePath)


//schema dependency created here
void CreatetHitTable(char *cTableName)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS %s ( uHit INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cZone VARCHAR(255) NOT NULL DEFAULT '',INDEX (cZone), uOwner INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, uHitCount BIGINT UNSIGNED NOT NULL DEFAULT 0, uSuccess BIGINT UNSIGNED NOT NULL DEFAULT 0, uReferral BIGINT UNSIGNED NOT NULL DEFAULT 0, uNxrrset BIGINT UNSIGNED NOT NULL DEFAULT 0, uNxdomain BIGINT UNSIGNED NOT NULL DEFAULT 0, uRecursion BIGINT UNSIGNED NOT NULL DEFAULT 0, uFailure BIGINT UNSIGNED NOT NULL DEFAULT 0, cHost VARCHAR(255) NOT NULL DEFAULT '',INDEX (cHost) )",cTableName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
}//CreatetHitTable()


void NextMonthYear(char *cMonth,char *cYear,char *cNextMonth,char *cNextYear)
{
	unsigned uYear=0;

	//Preset for all but Dec cases
	sprintf(cNextYear,"%.7s",cYear);
	sscanf(cNextYear,"%u",&uYear);

	if(!strcmp(cMonth,"Jan"))
	{
		strcpy(cNextMonth,"Feb");
	}
	else if(!strcmp(cMonth,"Feb"))
	{
		strcpy(cNextMonth,"Mar");
	}
	else if(!strcmp(cMonth,"Mar"))
	{
		strcpy(cNextMonth,"Apr");
	}
	else if(!strcmp(cMonth,"Apr"))
	{
		strcpy(cNextMonth,"May");
	}
	else if(!strcmp(cMonth,"May"))
	{
		strcpy(cNextMonth,"Jun");
	}
	else if(!strcmp(cMonth,"Jun"))
	{
		strcpy(cNextMonth,"Jul");
	}
	else if(!strcmp(cMonth,"Jul"))
	{
		strcpy(cNextMonth,"Aug");
	}
	else if(!strcmp(cMonth,"Aug"))
	{
		strcpy(cNextMonth,"Sep");
	}
	else if(!strcmp(cMonth,"Sep"))
	{
		strcpy(cNextMonth,"Oct");
	}
	else if(!strcmp(cMonth,"Oct"))
	{
		strcpy(cNextMonth,"Nov");
	}
	else if(!strcmp(cMonth,"Nov"))
	{
		strcpy(cNextMonth,"Dec");
	}
	else if(!strcmp(cMonth,"Dec"))
	{
		strcpy(cNextMonth,"Jan");
		uYear++;
		sprintf(cNextYear,"%u",uYear);
	}

}//NextMonthYear()


void MonthHitData(void)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
        MYSQL_RES *mysqlRes2;
        MYSQL_ROW mysqlField2;

	printf("#MonthHitData designed for use with gnuplot\n");

	sprintf(gcQuery,"SELECT cLabel FROM tMonthHit ORDER BY uMonth");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sprintf(gcQuery,"SELECT COUNT(uHit),(SUM(uHitCount) DIV 1000) FROM %s",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
		{
			//tApr2007Hit
			printf("%.3s-%.2s\t%s\t%s\n",mysqlField[0]+1,
				mysqlField[0]+6,mysqlField2[0],mysqlField2[1]);
		}
		mysql_free_result(mysqlRes2);
	}
	mysql_free_result(mysqlRes);

}//void MonthHitData(void)


void MonthUsageData(unsigned uSimile)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
        MYSQL_RES *mysqlRes2;
        MYSQL_ROW mysqlField2;
	unsigned uNewCount;
	unsigned uModCount;
	unsigned uDelCount;
	unsigned uBackendCount,uOrgCount,uAdminCount,uErrorCount;

	if(uSimile>1)
		printf("Content-type: text/plain\n\n");

	printf("#MonthUsageData designed for use with gnuplot or MIT Simile\n");
	//2008-03-10,32,63,5 --Simile format
	if(uSimile)
		printf("#Year-Mon-Day uNewCount uModCount uDelCount uTotal"
				" uBackendCount uOrgCount uAdminCount uErrorCount\n");
	else
		printf("#Mon-Yr uNewCount uModCount uDelCount uTotal"
				" uBackendCount uOrgCount uAdminCount uErrorCount\n");


	sprintf(gcQuery,"SELECT cLabel,FROM_UNIXTIME(uCreatedDate-3600,'%%Y-%%m-%%d') FROM tMonth ORDER BY uMonth");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		uNewCount=0;
		uModCount=0;
		uDelCount=0;
		uBackendCount=0;
		uOrgCount=0;
		uAdminCount=0;
		uErrorCount=0;

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE cLabel='New'",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uNewCount);
		mysql_free_result(mysqlRes2);

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE cLabel='Mod'",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uModCount);
		mysql_free_result(mysqlRes2);

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE cLabel='Del'",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uDelCount);
		mysql_free_result(mysqlRes2);

		//Totals per interface
		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE uLogType=1",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uBackendCount);
		mysql_free_result(mysqlRes2);

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE uLogType=2",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uOrgCount);
		mysql_free_result(mysqlRes2);

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE uLogType=3",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uAdminCount);
		mysql_free_result(mysqlRes2);

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE uLogType=5",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uErrorCount);
		mysql_free_result(mysqlRes2);

		//Output row
		//tApr2007 --> Apr-07
		//2008-03-10,32,63,5
		if(uSimile)
		printf("%s,%u,%u,%u,%u,%u,%u,%u,%u\n", mysqlField[1],
				uNewCount,uModCount,uDelCount,uNewCount+uModCount+uDelCount,
				uBackendCount,uOrgCount,uAdminCount,uErrorCount);
		else
		printf("%.3s-%.2s \t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n",
				mysqlField[0]+1,mysqlField[0]+6,
				uNewCount,uModCount,uDelCount,uNewCount+uModCount+uDelCount,
				uBackendCount,uOrgCount,uAdminCount,uErrorCount);
	}
	mysql_free_result(mysqlRes);

}//void MonthUsageData(void)


void DayUsageData(unsigned uLogType)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;

	//This SQL is only good for mySQL 4.1+

	if(uLogType>100)
	{
		printf("Content-type: text/plain\n\n");
		printf("#DayUsageData designed for use with MIT Simile, uLogType=%u\n",uLogType);
		printf("#Year-Mon-Day COUNT(uLog)\n");
		sprintf(gcQuery,"SELECT FROM_UNIXTIME(uCreatedDate,'%%Y-%%m-%%d'),COUNT(uLog) FROM tLog"
				" WHERE uLogType=%u AND MONTH(FROM_UNIXTIME(uCreatedDate))=MONTH(NOW()) GROUP BY"
				" DAY(FROM_UNIXTIME(uCreatedDate)) ORDER BY uCreatedDate",uLogType-100);
	}
	else
	{
		printf("#DayUsageData designed for use with gnuplot, uLogType=%u\n",uLogType);
		printf("#Day-Mon-Yr COUNT(uLog)\n");
		sprintf(gcQuery,"SELECT FROM_UNIXTIME(uCreatedDate,'%%d-%%m-%%y'),COUNT(uLog) FROM tLog"
				" WHERE uLogType=%u AND MONTH(FROM_UNIXTIME(uCreatedDate))=MONTH(NOW()) GROUP BY"
				" DAY(FROM_UNIXTIME(uCreatedDate)) ORDER BY uCreatedDate",uLogType);
	}

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	if(mysql_num_rows(mysqlRes))
	{
        	while((mysqlField=mysql_fetch_row(mysqlRes)))
			printf("%s %s\n",mysqlField[0],mysqlField[1]);
	}
	else
	{
		//No data SELECT FROM_UNIXTIME(UNIX_TIMESTAMP(NOW()),'%d-%m-%y')
		if(uLogType>100)
			sprintf(gcQuery,"SELECT FROM_UNIXTIME(UNIX_TIMESTAMP(NOW()),'%%Y-%%m-%%d')");
		else
			sprintf(gcQuery,"SELECT FROM_UNIXTIME(UNIX_TIMESTAMP(NOW()),'%%d-%%m-%%y')");

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes=mysql_store_result(&gMysql);
        	if((mysqlField=mysql_fetch_row(mysqlRes)))
		{
			printf("%s 0\n",mysqlField[0]);
		}
	}
	mysql_free_result(mysqlRes);

}//void DayUsageData(unsigned uLogType)


void CheckAllZones(void)
{
	//
	//Run named-checkzones for all local zones and prints a summary report
	//Local zones are zones with uSecondaryOnly=0
	
	MYSQL_RES *res;
	MYSQL_ROW field;
	
	char cZoneFile[512]={""};
	
	unsigned uZonesOK=0;
	unsigned uZonesWithErrors=0;
	unsigned uTotalZones=0;
	float fSystemHealth=0.00;
	
	sprintf(gcQuery,
		"SELECT tZone.cZone,(SELECT tView.cLabel FROM tView WHERE tView.uView=tZone.uView) FROM tZone"
		" WHERE tZone.uSecondaryOnly=0 ORDER BY tZone.uZone");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	res=mysql_store_result(&gMysql);

	uTotalZones=(unsigned)mysql_num_rows(res);
	
	while((field=mysql_fetch_row(res)))
	{
		//named-checkzone e-s.co.uk /usr/local/idns/named.d/master/external/e/e-s.co.uk
		
		sprintf(cZoneFile,"/usr/local/idns/named.d/master/%s/%c/%s",field[1],field[0][0],field[0]);
		sprintf(gcQuery,"/usr/sbin/named-checkzone %s %s > /dev/null  2>&1",field[0],cZoneFile);
		
		if(system(gcQuery))
		{
			printf("Zone %s has errors. Zonefile at %s\n",field[0],cZoneFile);
			uZonesWithErrors++;
		}
		else
			uZonesOK++;
	}
	
	fSystemHealth=((float)uZonesOK*100.00)/(float)uTotalZones;
	
	printf("Summary:\n");
	printf("--------------------------------------\n");
	printf("Total zones          : %u\n",uTotalZones);
	printf("Zones with no errors : %u\n",uZonesOK);
	printf("Zones with errors    : %u\n",uZonesWithErrors);
	printf("System health is     : %.2f\n",fSystemHealth);
	
}//void CheckAllZones(void)


void ImportFromDb(char *cSourceDbName, char *cTargetDbName, char *cPasswd)
{
	printf("ImportFromDb() start\n");

	mySQLRootConnect(cPasswd);

	sprintf(gcQuery,"truncate %s.tZoneImport",cTargetDbName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	sprintf(gcQuery,"INSERT INTO %s.tZoneImport (uZone,cZone,uNSSet,cHostmaster,uSerial,"
			"uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
			"uMailServers,uView,cMainAddress,uRegistrar,uSecondaryOnly,cOptions,uClient,uOwner,"
			"uCreatedBy,uCreatedDate,uModBy,uModDate)"
			" SELECT uZone,cZone,1,cHostmaster,uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
			"uMailServers,2,cMainAddress,0,0,'',0,uOwner,"
			"uCreatedBy,uCreatedDate,uModBy,uModDate FROM %s.tZone",
								cTargetDbName,cSourceDbName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	sprintf(gcQuery,"truncate %s.tResourceImport",cTargetDbName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	sprintf(gcQuery,"INSERT INTO %1$s.tResourceImport (uZone,cName,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4,cComment,"
			"uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate)"
			" SELECT tZone.uZone,tResource.cName,tResource.uTTL,tResource.uRRType,tResource.cParam1,"
			"tResource.cParam2,'','',tResource.cComment,"
			"tResource.uOwner,tResource.uCreatedBy,tResource.uCreatedDate,tResource.uModBy,tResource.uModDate"
			" FROM %2$s.tResource,%2$s.tZone"
			" WHERE tResource.uZone=tZone.uZone",cTargetDbName,cSourceDbName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}


	printf("ImportFromDb() end\n");

}//void ImportFromDb()


void ZeroSystem(void)
{

	//Change to load distro tutorial .sql file

}//void ZeroSystem(void)


unsigned NewNSSet(char *cLabel,char *cMasterIPs,char *cuOwner)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT uNSSet FROM tNSSet WHERE cLabel='%s'",cLabel);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		unsigned uNSSet=0;

		sscanf(field[0],"%u",&uNSSet);
		printf("\t\ttNSSet record already exists...updating\n");
		sprintf(gcQuery,"UPDATE tNSSet SET cMasterIPs='%s',uOwner=%s,"
				"uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
				" WHERE uNSSet=%u",
					cMasterIPs,cuOwner,uNSSet);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		return(uNSSet);
	}
	else
	{
		sprintf(gcQuery,"INSERT tNSSet SET cLabel='%s',cMasterIPs='%s',uOwner=%s,uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())",
				cLabel,cMasterIPs,cuOwner);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		return(mysql_insert_id(&gMysql));
	}
	mysql_free_result(res);

}//unsigned NewNSSet(char *cLabel,char *cMasterIPs,char *cuOwner)


void NewNS(char *cFQDN,char *cNSType,unsigned uNSSet)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uNSType=1;

	sprintf(gcQuery,"SELECT uNSType FROM tNSType WHERE cLabel='%s'",cNSType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uNSType);
	else
		printf("\t\tCould not determine uNSType...using default\n");
	mysql_free_result(res);
		

	sprintf(gcQuery,"SELECT uNS FROM tNS WHERE cFQDN='%s' AND uNSSet=%u",cFQDN,uNSSet);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		printf("\t\ttNS record already exists...updating\n");
		sprintf(gcQuery,"UPDATE tNS SET uNSType=%u,"
				"uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
				" WHERE uNS=%s",
					uNSType,field[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
	}
	else
	{
		sprintf(gcQuery,"INSERT tNS SET cFQDN='%s',uNSType=%u,uNSSet=%u,uOwner=1,uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())",
					cFQDN,uNSType,uNSSet);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
	}
	mysql_free_result(res);

}//void NewNS(...)


//List A records for queryperf file
void PerfQueryList(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	if(TextConnectDb()) exit(0);

	sprintf(gcQuery,"SELECT tResource.cName,tZone.cZone FROM tZone,tNSSet,tResource"
			" WHERE tZone.uNSSet=tNSSet.uNSSet AND tZone.uZone=tResource.uZone"
			" AND tResource.uRRType=1 AND tResource.cName NOT LIKE '%%.' AND tResource.cName!='' AND tResource.cName!='*'"
			" AND tResource.cName!='@' AND tResource.cName!='\t'");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res))) 
	{
		//printf("%s %s\n",field[0],field[1]);
		printf("%s.%s A\n",field[0],field[1]);
	}
	mysql_free_result(res);
	exit(0);

}//void PerfQueryList(void)


void CreatePBXZonesFromVZ(void)
{
	MYSQL gMysql2;

	//debug only
	printf("CreatePBXZonesFromVZ() start\n");
	if(!TextConnectDb() && !TextConnectExtDb(&gMysql2,TEXT_CONNECT_UNXSVZ))
	{
		MYSQL_RES *res;
		MYSQL_ROW field;

		//debug only
		printf("CreatePBXZonesFromVZ() connected ok\n");


		//Quick way to make copies of tables for testing:
		//	CREATE TABLE tZoneCopy LIKE tZone;
		//	INSERT INTO tZoneCopy SELECT * FROM tZone;

		//get zone name
		//get IP of primary container
		//get IP of remote clone container
		//get port numbers if they exist default 5060
		//create zone
		//add RRs
		//	A's for primary and clone container
		//		primary 12.12.12.12
		//		backup 13.13.13.13
		//	SRV for sip primary _sip._udp 10 1 5060 primary
		//	SRV for sip backup	_sip._udp 20 1 5060 backup
		unsigned uMainPort=5060;
		unsigned uBackupPort=5060;
		unsigned uNSSet=11;//Get from tConfiguration
		char cHostmaster[100]={"dns.isp.com"};
		sprintf(gcQuery,"SELECT tContainer.cHostname,tIP.cLabel,tContainer.uContainer,tContainer.uDatacenter"
			" FROM tContainer,tIP,tGroup,tGroupGlue WHERE"
			" tContainer.uIPv4=tIP.uIP AND"
			" tGroupGlue.uGroup=tGroup.uGroup AND"
			" tGroupGlue.uContainer=tContainer.uContainer AND"
			" tGroup.cLabel='Production PBXs' AND"
			" tContainer.uStatus=1 AND"
			" tContainer.uSource=0 LIMIT 2");
		mysql_query(&gMysql2,gcQuery);
	       	if(mysql_errno(&gMysql2))
		{
			fprintf(stdout,"%s\n",mysql_error(&gMysql2));
			return;
		}
		res=mysql_store_result(&gMysql2);
	        while((field=mysql_fetch_row(res)))
		{
			MYSQL_RES *res2;
			MYSQL_ROW field2;
			unsigned uRemoteZone=0;

			//Get remote backup of current container
			char cBackupIP[32]={""};
			char cBackupName[100]={""};
			sprintf(cBackupIP,"%.31s",field[1]);//if no remote backup then use primary ip
			sprintf(gcQuery,"SELECT tContainer.cHostname,tIP.cLabel,tContainer.uContainer FROM tContainer,tIP WHERE"
					" tContainer.uIPv4=tIP.uIP AND"
					" tContainer.uSource=%s AND"
					" tContainer.uDatacenter!=%s",
						field[2],field[3]);
			mysql_query(&gMysql2,gcQuery);
			if(mysql_errno(&gMysql2)) 
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql2));
				exit(1);
			}
			res2=mysql_store_result(&gMysql2);
			if((field2=mysql_fetch_row(res2))) 
			{
				//exclude rfc1918 IPs
				unsigned uA=0,uB=0,uC=0;
				sscanf(field2[1],"%u.%u.%u.%*u",&uA,&uB,&uC);
				if( !( (uA==172 && uB>=16 && uB<=31) || (uA==192 && uB==168) || (uA=10)) )
					sprintf(cBackupIP,"%.31s",field2[1]);
				sprintf(cBackupName,"%.99s",field2[0]);
				sscanf(field2[3],"%u",&uRemoteZone);
			}


			if(uRemoteZone)
			{
				//Get ports	cOrg_SIPPort
				sprintf(gcQuery,"SELECT cValue FROM tProperty WHERE"
						" uKey=%u AND uType=3 AND"
						" cValue='cOrg_SIPPort'",uRemoteZone);
				mysql_query(&gMysql2,gcQuery);
				if(mysql_errno(&gMysql2)) 
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql2));
					exit(1);
				}
				res2=mysql_store_result(&gMysql2);
				if((field2=mysql_fetch_row(res2))) 
					sscanf(field2[0],"%u",&uBackupPort);

				sprintf(gcQuery,"SELECT cValue FROM tProperty WHERE"
						" uKey=%u AND uType=3 AND"
						" cValue='cOrg_SIPPort'",uRemoteZone);
				mysql_query(&gMysql2,gcQuery);
				if(mysql_errno(&gMysql2)) 
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql2));
					exit(1);
				}
				res2=mysql_store_result(&gMysql2);
				if((field2=mysql_fetch_row(res2))) 
					sscanf(field2[0],"%u",&uMainPort);
			}


			printf("%s %s %u; %s %s %u;\n",field[0],field[1],uMainPort,cBackupName,cBackupIP,uBackupPort);


			//If zone exists...
			sprintf(gcQuery,"SELECT uZone FROM tZoneCopy WHERE"
					" cZone='%s' AND"
					" uView=(SELECT uView FROM tView WHERE cLabel='external')",
					field[0]);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql)) 
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				exit(1);
			}
			res2=mysql_store_result(&gMysql);
			if((field2=mysql_fetch_row(res2))) 
			{
				MYSQL_RES *res3;
				MYSQL_ROW field3;

				printf("zone exists %s\n",field[0]);

				//main A record
				sprintf(gcQuery,"SELECT uResource FROM tResourceCopy WHERE"
						" cName='%s.' AND"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='A') AND"
						" uZone=%s",
							field[0],field2[0]);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					exit(1);
				}
				res3=mysql_store_result(&gMysql);
				if((field3=mysql_fetch_row(res3))) 
				{
					printf("updating A RR%s\n",field3[0]);
					sprintf(gcQuery,"UPDATE tResourceCopy SET"
						" cParam1='%s',"
						" cComment='CreatePBXZonesFromVZ',"
						" uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
						" WHERE uResource=%s",
							field[1],field3[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql)) 
					{
						fprintf(stderr,"%s\n",mysql_error(&gMysql));
						exit(1);
					}
				}
				else
				{
					sprintf(gcQuery,"INSERT INTO tResourceCopy SET"
						" uZone=%s,"
						" cName='%s.',"
						" cParam1='%s',"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='A'),"
						" cComment='CreatePBXZonesFromVZ',"
						" uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							field2[0],field[0],field[1]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql)) 
					{
						fprintf(stderr,"%s\n",mysql_error(&gMysql));
						exit(1);
					}
				}
				//end main A

				//backup A record
				sprintf(gcQuery,"SELECT uResource FROM tResourceCopy WHERE"
						" cName='backup.%s.' AND"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='A') AND"
						" uZone=%s",
							field[0],field2[0]);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					exit(1);
				}
				res3=mysql_store_result(&gMysql);
				if((field3=mysql_fetch_row(res3))) 
				{
					printf("updating backup A RR%s\n",field3[0]);
					sprintf(gcQuery,"UPDATE tResourceCopy SET"
						" cParam1='%s',"
						" cComment='CreatePBXZonesFromVZ',"
						" uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
						" WHERE uResource=%s",
							cBackupIP,field3[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql)) 
					{
						fprintf(stderr,"%s\n",mysql_error(&gMysql));
						exit(1);
					}
				}
				else
				{
					sprintf(gcQuery,"INSERT INTO tResourceCopy SET"
						" uZone=%s,"
						" cName='backup.%s.',"
						" cParam1='%s',"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='A'),"
						" cComment='CreatePBXZonesFromVZ',"
						" uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							field2[0],field[0],field[1]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql)) 
					{
						fprintf(stderr,"%s\n",mysql_error(&gMysql));
						exit(1);
					}
				}
				//end backup A

				//main SRV record
				sprintf(gcQuery,"SELECT uResource FROM tResourceCopy WHERE"
						" cName='_sip._udp.%s.' AND"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='SRV') AND"
						" uZone=%s",
							field[0],field2[0]);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					exit(1);
				}
				res3=mysql_store_result(&gMysql);
				if((field3=mysql_fetch_row(res3))) 
				{
					printf("No update for main SRV record\n");
				}
				else
				{
					sprintf(gcQuery,"INSERT INTO tResourceCopy SET"
						" uZone=%s,"
						" cName='_sip._udp.%s.',"
						" cParam1='10',"
						" cParam2='1',"
						" cParam3='%u',"
						" cParam4='%s.',"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='SRV'),"
						" cComment='CreatePBXZonesFromVZ',"
						" uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							field2[0],field[0],uMainPort,field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql)) 
					{
						fprintf(stderr,"%s\n",mysql_error(&gMysql));
						exit(1);
					}
				}
				//end main SRV

				//backup SRV record
				sprintf(gcQuery,"SELECT uResource FROM tResourceCopy WHERE"
						" cName='_sip._udp.%s.' AND"
						" cParam4='backup.%s.' AND"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='SRV') AND"
						" uZone=%s",
							field[0],field[0],field2[0]);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					exit(1);
				}
				res3=mysql_store_result(&gMysql);
				if((field3=mysql_fetch_row(res3))) 
				{
					printf("No update for backup SRV record\n");
				}
				else
				{
					sprintf(gcQuery,"INSERT INTO tResourceCopy SET"
						" uZone=%s,"
						" cName='_sip._udp.backup.%s.',"
						" cParam1='20',"
						" cParam2='1',"
						" cParam3='%u',"
						" cParam4='backup.%s.',"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='SRV'),"
						" cComment='CreatePBXZonesFromVZ',"
						" uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							field2[0],field[0],uBackupPort,field[0]);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql)) 
					{
						fprintf(stderr,"%s\n",mysql_error(&gMysql));
						exit(1);
					}
				}
				//end backup SRV
			}
			else
			{
				printf("creating zone %s\n",field[0]);
				sprintf(gcQuery,"INSERT INTO tZoneCopy SET"
						" cZone='%s',"
						" uNSSet=%u,"
						" cHostmaster='%s',"
						" uSerial=1,"
						" uExpire=604800,"
						" uRefresh=28800,"
						" uTTL=300,"
						" uRetry=7200,"
						" uZoneTTL=86400,"
						" uView=(SELECT uView FROM tView where cLabel='external'),"
						" cMainAddress='0.0.0.0',"
						" cOptions='//CreatePBXZonesFromVZ',"
						" uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
								field[0],uNSSet,cHostmaster);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					exit(1);
				}

				unsigned uZone=mysql_insert_id(&gMysql);

				//main A record
				sprintf(gcQuery,"INSERT INTO tResourceCopy SET"
						" uZone=%u,"
						" cName='%s.',"
						" cParam1='%s',"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='A'),"
						" cComment='CreatePBXZonesFromVZ',"
						" uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							uZone,field[0],field[1]);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					exit(1);
				}
				sprintf(gcQuery,"INSERT INTO tResourceCopy SET"
						" uZone=%u,"
						" cName='backup.%s.',"
						" cParam1='%s',"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='A'),"
						" cComment='CreatePBXZonesFromVZ',"
						" uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							uZone,field[0],field[1]);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					exit(1);
				}
				sprintf(gcQuery,"INSERT INTO tResourceCopy SET"
						" uZone=%u,"
						" cName='_sip._udp.%s.',"
						" cParam1='10',"
						" cParam2='1',"
						" cParam3='%u',"
						" cParam4='%s.',"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='SRV'),"
						" cComment='CreatePBXZonesFromVZ',"
						" uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							uZone,field[0],uMainPort,field[0]);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					exit(1);
				}
				sprintf(gcQuery,"INSERT INTO tResourceCopy SET"
						" uZone=%u,"
						" cName='_sip._udp.%s.',"
						" cParam1='20',"
						" cParam2='1',"
						" cParam3='%u',"
						" cParam4='backup.%s.',"
						" uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='SRV'),"
						" cComment='CreatePBXZonesFromVZ',"
						" uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							uZone,field[0],uBackupPort,field[0]);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					exit(1);
				}
			}
			mysql_free_result(res2);
		}
		mysql_free_result(res);
	}
	exit(0);

}//void CreatePBXZonesFromVZ(void)


void DeletePBXZones(void)
{
	if(TextConnectDb())
		exit(0);

	sprintf(gcQuery,"DELETE FROM tZoneCopy WHERE"
			" cOptions LIKE '%%//CreatePBXZonesFromVZ%%' AND"
			" uOwner=1 AND uCreatedBy=1");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	unsigned uNumRows=mysql_affected_rows(&gMysql);
	printf("DeletePBXZones %u zones deleted\n",uNumRows);

	sprintf(gcQuery,"DELETE FROM tResourceCopy WHERE"
			" cComment='CreatePBXZonesFromVZ' AND"
			" uOwner=1 AND uCreatedBy=1");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	uNumRows=mysql_affected_rows(&gMysql);
	printf("\t%u RRs deleted\n",uNumRows);

}//void DeletePBXZones(void)
