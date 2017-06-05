/*
FILE
	svn ID removed
AUTHOR
	(C) 2001-2010 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
PURPOSE
	Jobqueue processing code.
	Import/export code moved to import.c 2010-03
WORK-IN-PROGRESS
	Feb 2009. In the middle of major change: Replacing the wierd t Name Server.cList for
	a much more flexible tNSSet schema that allows mixed NS sets on the same named instance.
	(Next we can do the MX list. A lot easier and for schema reasons only.)

	Mar 2010. Convert printf processing to logfileLine() new standard unxsVZ output message/error/progress interface.
*/

#include "mysqlrad.h"

//
//Local data
//
static FILE *gLfp=NULL;
static unsigned guReconfig=0;
static unsigned guReload=0;

//
//TOC protos
//


unsigned uGetZoneOwner(unsigned uZone);
unsigned uNamedCheckConf(char *cNSSet);
char *GetRRType(unsigned uRRType);


//aux section
extern char gcBinDir[];
void ConnectMysqlServer(void);//local also used in mainfunc.h
void ExportTable(char *cTable, char *cFile);//local used in mainfunc.h for CLI command
void GenerateArpaZones(void);//local used in tzonefunc.h
void GetConfiguration(const char *cName, char *cValue, unsigned uHtml);//local used in many places
void PassDirectHtml(char *file);//local also used by tzonefunc.h
//local used in tzonefunc.h and tresourcefunc.h:
int PopulateArpaZone(const char *cZone, const char *cIPNum, const unsigned uHtmlMode, 
					const unsigned uFromZone, const unsigned uZoneOwner, const unsigned uNSSet);
int PopulateArpaZoneIPv6(const char *cIPv6ArpaZoneName, const char *cIPv6PTR, const char *cHostname, const unsigned uHtmlMode, 
					const unsigned uFromZone, const unsigned uZoneOwner, const unsigned uNSSet);
void SerialNum(char *serial);//local also used in tzonefunc.h

//Job queue processing functions
void SlaveJobQueue(char *cNameServer,char *cMasterIP);//local used in mainfunc.h for several CLI commands
void MasterJobQueue(char *cNameServer);//local used in mainfunc.h for several CLI commands
void ServerJobQueue(char *cServer);//local used in mainfunc.h for CLI command
void CreateMasterFiles(char *cMasterNS,char *cZone, unsigned uModDBFiles, 
		unsigned uModStubs,unsigned uDebug);//local used in mainfunc.h for CLI command
void CreateSlaveFiles(char *cSlaveNS,char *cZone, char *cMasterIP,
		unsigned uDebug);//local used in mainfunc.h for CLI command
void InstallNamedFiles(char *cIpNum);//local used in mainfunc.h for CLI command


int ProcessMasterJob(char *cZone,unsigned uDelete,unsigned uModify,
		unsigned uNew, unsigned uDeleteFirst, char *cMasterNS);
int ProcessSlaveJob(char *cZone,unsigned uDelete,unsigned uModify,unsigned uNew,
		unsigned uDeleteFirst, char *cMasterNS, char *cMasterIP);
char *cPrintNSList(FILE *zfp,char *cuNSSet);//local
void PrintMXList(FILE *zfp,char *cuMailServers);//local
unsigned ViewReloadZone(char *cZone);//local

//External. Used here but located in other files.
int AutoAddPTRResource(const unsigned d,const char *cDomain,const unsigned uInZone,const unsigned uSourceZoneOwner);//tresourcefunc.h
int AutoAddPTRResourceIPv6(const char *cIPv6PTR,const char *cDomain,const unsigned uInZone,const unsigned uSourceZoneOwner);//tresourcefunc.h
void UpdateSerialNum(unsigned uZone);//tzonefunc.h
unsigned TextConnectDb(void);//mysqlconnect.c
int InformExtISPJob(const char *cRemoteMsg,const char *cServer,unsigned uJob,unsigned uJobStatus);//extjobqueue.c
unsigned uGetNSSet(char *cNameServer);//import.c


void logfileLine(const char *cFunction,const char *cLogline)
{
	time_t luClock;
	char cTime[32];
	pid_t pidThis;
	const struct tm *tmTime;

	if(gLfp==NULL) return;

	pidThis=getpid();

	time(&luClock);
	tmTime=localtime(&luClock);
	strftime(cTime,31,"%b %d %T",tmTime);

        fprintf(gLfp,"%s unxsBind.%s[%u]: %s\n",cTime,cFunction,pidThis,cLogline);
	fflush(gLfp);

}//void logfileLine(char *cLogline)


char *GetRRType(unsigned uRRType)
{
	MYSQL_RES *res;
	static char cRRType[100];
	
	strcpy(cRRType,"unknown");

	sprintf(gcQuery,"SELECT cLabel FROM tRRType WHERE uRRType=%u",uRRType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	
	if(mysql_num_rows(res)==1) 
	{
		MYSQL_ROW field;
		if((field=mysql_fetch_row(res)))
			strcpy(cRRType,field[0]);

	}
	mysql_free_result(res);
	return(cRRType);

}//char *GetRRType(unsigned uRRType)


char *cPrintNSList(FILE *zfp,char *cuNSSet)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	static char cFirstNS[100]={""};
	unsigned uFirst=1;

	//Do not include HIDDEN NSs. 2 is the fixed HIDDEN TYPE
	//This ORDER BY implies that people will use a NS scheme that is alphabetical
	//in nature like ns1, ns2 etc. This will need to be looked at later ;)
	sprintf(gcQuery,"SELECT tNS.cFQDN,tNS.uNSType FROM tNSSet,tNS WHERE tNSSet.uNSSet=tNS.uNSSet AND"
			" tNS.uNSType!=2 AND tNSSet.uNSSet=%s ORDER BY tNS.cFQDN",cuNSSet);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
			fprintf(zfp,"\t\tNS %s.\n",FQDomainName(field[0]));
			if(uFirst)
			{
				sprintf(cFirstNS,"%.99s",field[0]);
				uFirst=0;
			}
	}
	mysql_free_result(res);

	return(cFirstNS);

}//char *cPrintNSList()


//We still have not changed to a tMXSet based schema.
//And we may never since no one really needs to use this old tZone time saver feature.
void PrintMXList(FILE *zfp,char *cuMailServers)
{
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT cList FROM tMailServer WHERE uMailServer=%s",
			cuMailServers);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	
	if(mysql_num_rows(res)==1) 
	{
		MYSQL_ROW field;
		register int i=0,j=0,uMX=10;

		if((field=mysql_fetch_row(res)))
		{
			//parse out
			while(field[0][i])
			{
				if(field[0][i]=='\n' || field[0][i]==0)
				{
					field[0][i]=0;
					if(strlen(FQDomainName(field[0]+j)))
					{	
						fprintf(zfp,"\t\tMX %d %s.\n",uMX,
							FQDomainName(field[0]+j));
					}
					i++;
					j=i;
					uMX+=10;
				}
				i++;
			}
			if(field[0][j] && strlen(FQDomainName(field[0]+j)))
			{	
				fprintf(zfp,"\t\tMX %d %s.\n",uMX,
					FQDomainName(field[0]+j));
			}
		}
	}
	mysql_free_result(res);

}//PrintMXList(FILE *fp,char *cuMailServers)


void CreateMasterFiles(char *cMasterNS, char *cZone, unsigned uModDBFiles, 
		unsigned uModStubs, unsigned uDebug)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	MYSQL_RES *res2;
	MYSQL_ROW field2;
	FILE *sfp=stdout;
	FILE *zfp=stdout;
	FILE *dnfp=NULL;
	unsigned uZone=0;
	unsigned uRRType=0;
	char cTTL[16]={""};
	unsigned uView=0,uCurrentView=0,uFirst=1;
	unsigned uNSSet=0;

	//printf("CreateMasterFiles() uModDBFiles=%u; uModStubs=%u\n",uModDBFiles,uModStubs);
	//0 cZone
	//1 uZone
	//2 uNSSet
	//3 cHostmaster
	//4 uSerial
	//5 uTTL
	//6 uExpire
	//7 uRefresh
	//8 uRetry
	//9 uZoneTTL
	//10 uMailServers
	//11 cMainAddress
	//12 tView.cLabel
	//13 tView.cMaster
	//14 tView.uView
	//15 tZone.cOptions

	char *cp;
	char cMasterNS2[100]={""};//cNameServer2
	char cMasterNS1[100]={""};//cNameServer1
	unsigned uNSSet2=0;
	if((cp=strchr(cMasterNS,',')))
	{
		*cp=0;
		sprintf(cMasterNS2,"%.99s",cp+1);
		logfileLine("CreateMasterFiles",cMasterNS);//cNameServer2
		logfileLine("CreateMasterFiles",cMasterNS2);//cNameServer2
		uNSSet2=uGetNSSet(cMasterNS2);//cNameServer2
		uNSSet=uGetNSSet(cMasterNS);
		sprintf(cMasterNS1,"%.99s",cMasterNS);
		*cp=',';//cMasterNS will be used again!
	}
	else
	{
		uNSSet=uGetNSSet(cMasterNS);
		sprintf(cMasterNS1,"%.99s",cMasterNS);
	}

	//debug only
	//printf("(%s) (%s) uNSSet=%u uNSSet2=%u\n",cMasterNS,cMasterNS2,uNSSet,uNSSet2);
	//return;
	if(!uNSSet)
		logfileLine("CreateMasterFiles","error uNSSet==0");

	if(uModStubs)
	{
		logfileLine("CreateMasterFiles","uModStubs");

		char cTSIGKeyNameUpdate[256]={""};
		char cTSIGKeyNameTransfer[256]={""};

		uView=0;
		uCurrentView=0;
		uFirst=1;

		//included stub zone files can only be all for new view based system

		if(uNSSet2)
			sprintf(gcQuery,"SELECT DISTINCT tZone.cZone,tZone.uZone,tZone.uNSSet,tZone.cHostmaster,"
				"tZone.uSerial,tZone.uTTL,tZone.uExpire,tZone.uRefresh,tZone.uRetry,"
				"tZone.uZoneTTL,tZone.uMailServers,tZone.cMainAddress,tView.cLabel,"
				"tView.cMaster,tView.uView,tZone.cOptions FROM tZone,tNSSet,tNS,tView WHERE"
				" tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND"
				" tZone.uView=tView.uView AND"
				" (tNSSet.uNSSet=%u OR tNSSet.uNSSet=%u) AND"
				" tNS.uNSType<4 AND tZone.uSecondaryOnly=0 ORDER BY" //4 is SLAVE last in fixed table
				" tView.uOrder,tZone.cZone",uNSSet,uNSSet2);
		else
			sprintf(gcQuery,"SELECT DISTINCT tZone.cZone,tZone.uZone,tZone.uNSSet,tZone.cHostmaster,"
				"tZone.uSerial,tZone.uTTL,tZone.uExpire,tZone.uRefresh,tZone.uRetry,"
				"tZone.uZoneTTL,tZone.uMailServers,tZone.cMainAddress,tView.cLabel,"
				"tView.cMaster,tView.uView,tZone.cOptions FROM tZone,tNSSet,tNS,tView WHERE"
				" tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND"
				" tZone.uView=tView.uView AND"
				" tNSSet.uNSSet=%u AND"
				" tNS.uNSType<4 AND tZone.uSecondaryOnly=0 ORDER BY" //4 is SLAVE last in fixed table
				" tView.uOrder,tZone.cZone",uNSSet);
		//debug only
		//printf("(uModStubs)%s\n",gcQuery);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
		res=mysql_store_result(&gMysql);
	
		if(mysql_num_rows(res)<1) 
		{
			fprintf(stdout,"No zones found for master NS %s\n",cMasterNS);
			return;
		}

		//master lock dir
		struct stat statInfo;
		if(stat("/var/run/unxsBind.lock",&statInfo))
		{
			mkdir("/var/run/unxsBind.lock",S_IRWXU);
		}
		else
		{
			fprintf(stdout,"/var/run/unxsBind.lock exists exiting\n");
			return;
		}
			
		if(!uDebug)
		{

			if(!(sfp=fopen("/usr/local/idns/named.d/master.zones","w")))
			{
				fprintf(stderr,"Could not open master.zones\n");
				exit(1);
			}
		}
		else
		{
			fprintf(stdout,"(/usr/local/idns/named.d/master.zones)\n");
		}

		//Only once for all zones. TODO this is a limitation.
		GetConfiguration("cTSIGKeyNameTransfer",cTSIGKeyNameTransfer,0);
		GetConfiguration("cTSIGKeyNameUpdate",cTSIGKeyNameUpdate,0);
		//limitation first of two if two
		char cOptionalExternalViewLine[256]={""};
		char cOptionalExternalViewLineName[256]={""};
		sprintf(cOptionalExternalViewLineName,"cOptionalExternalViewLine:%s",cMasterNS1);
		GetConfiguration(cOptionalExternalViewLineName,cOptionalExternalViewLine,0);
		if(cOptionalExternalViewLine[0])
			logfileLine("CreateMasterFiles",cOptionalExternalViewLine);
		//debug only
		//printf("%s %s\n",cOptionalExternalViewLineName,cOptionalExternalViewLine);

		while((field=mysql_fetch_row(res)))
		{
			sscanf(field[1],"%u",&uZone);
			sscanf(field[14],"%u",&uView);

			if(uCurrentView!=uView)
			{
				char cExtraForViewName[100]={""};
				char cExtraForView[1024]={""};

				if(!uFirst)
				{
					fprintf(sfp,"};\n");
				}
				uFirst=0;
				uCurrentView=uView;
				fprintf(sfp,"\n%s\n",field[13]);

				sprintf(cExtraForViewName,"cExtraForViewMaster:%.31s",field[12]);
				GetConfiguration(cExtraForViewName,cExtraForView,0);
				if(cExtraForView[0])
					fprintf(sfp,"%s\n",cExtraForView);

			}

			//write stub into include file		
			fprintf(sfp,"\n\tzone \"%s\" {\n",field[0]);

			//Special types set via cOptions
			//tZone.cOptions=//TypeForward ForwardOnly Forwarders={10.240.0.1;10.240.0.2;};
			if(field[15][0] && strstr(field[15],"//TypeForward")!=NULL)
			{
				char *cp1;

				if((cp1=strstr(field[15],"Forwarders="))!=NULL)
				{
					//Last part must end correctly. Or we ignore.
					if(*(field[15]+strlen(field[15])-1)==';')
					{
						fprintf(sfp,"\t\ttype forward;\n");
						if(strstr(field[15],"ForwardOnly")!=NULL)
							fprintf(sfp,"\t\tforward only;\n");
						else if(strstr(field[15],"ForwardFirst")!=NULL)
							fprintf(sfp,"\t\tforward first;\n");
		
						fprintf(sfp,"\t\tforwarders %s\n",cp1+11);
						fprintf(sfp,"\t};\n");
						continue;//Done with this zone
					}
					else
					{
						fprintf(sfp,"\t\ttype master;\n");
						fprintf(sfp,"//\t\tforwarders %s %c\n",cp1+11,
							*(field[15]+strlen(field[15])-1));
						
					}
				}
			}
			else
			{
				fprintf(sfp,"\t\ttype master;\n");
			}

			//allow-transfer and allow-update have global optional
			//inclusion that is overridden by cOptions if specified there.
			if(field[15][0])
				fprintf(sfp,"\t\t//tZone.cOptions\n\t\t%s\n",field[15]);
			//Note the trailing period after key
			if(cTSIGKeyNameTransfer[0])
			{
				if(!field[15][0] || !strstr(field[15],"allow-transfer"))
					fprintf(sfp,"\t\tallow-transfer { key %.99s.;};\n",
						cTSIGKeyNameTransfer);
			}
			if(cTSIGKeyNameUpdate[0])
			{
				if(!field[15][0] || !strstr(field[15],"allow-update"))
					fprintf(sfp,"\t\tallow-update { key %.99s.;};\n",
						cTSIGKeyNameUpdate);
			}

			//cOptions DotSignedExtension
			fprintf(sfp,"\t\tfile \"master/%s/%c/",field[12],field[0][0]);
			if(field[15][0] && strstr(field[15],"//DotSignedExtension")!=NULL)
				fprintf(sfp,"%s.signed",field[0]);
			else
				fprintf(sfp,"%s",field[0]);
			fprintf(sfp,"\";\n\t};\n");
		}
		if(cOptionalExternalViewLine[0] && uView==2)
		{
			fprintf(sfp,"%s\n",cOptionalExternalViewLine);
			//debug only
			//printf("%s\n",cOptionalExternalViewLine);
		}
		fprintf(sfp,"};\n");
	}
	if(sfp && !uDebug) fclose(sfp);

	//db files can be single or all
	if(uModDBFiles)
	{
		logfileLine("CreateMasterFiles","uModDBFiles");

		char cuUID[256]={""};
		char cuGID[256]={""};
		unsigned uGID=25;
		unsigned uUID=25;
		char cZoneFile[512]={""};
		unsigned uZoneOwner=0;
		GetConfiguration("cuUID",cuUID,0);
		if(cuUID[0]) sscanf(cuUID,"%u",&uUID);
		GetConfiguration("cuGID",cuGID,0);
		if(cuGID[0]) sscanf(cuGID,"%u",&uGID);


		if(uNSSet2)
		{
			if(cZone[0])	
			{
				sprintf(gcQuery,"SELECT DISTINCT tZone.cZone,tZone.uZone,tZone.uNSSet,tZone.cHostmaster,"
				"tZone.uSerial,tZone.uTTL,tZone.uExpire,tZone.uRefresh,tZone.uRetry,tZone.uZoneTTL,"
				"tZone.uMailServers,tZone.cMainAddress,tView.cLabel,tZone.cOptions FROM tZone,tNSSet,tNS,tView"
				" WHERE tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND"
					" (tNSSet.uNSSet=%u OR tNSSet.uNSSet=%u) AND"
				" tZone.uView=tView.uView AND tZone.cZone='%s'"
						,uNSSet,uNSSet2,cZone);
			}
			else
			{
				sprintf(gcQuery,"SELECT DISTINCT tZone.cZone,tZone.uZone,tZone.uNSSet,tZone.cHostmaster,"
				"tZone.uSerial,tZone.uTTL,tZone.uExpire,tZone.uRefresh,tZone.uRetry,tZone.uZoneTTL,"
				"tZone.uMailServers,tZone.cMainAddress,tView.cLabel,tZone.cOptions FROM tZone,tNSSet,tNS,tView"
				" WHERE tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND"
				" (tNSSet.uNSSet=%u OR tNSSet.uNSSet=%u)  AND"
				" tZone.uView=tView.uView ORDER BY tZone.cZone",uNSSet,uNSSet2);
			}
		}
		else
		{
			if(cZone[0])	
			{
				sprintf(gcQuery,"SELECT DISTINCT tZone.cZone,tZone.uZone,tZone.uNSSet,tZone.cHostmaster,"
				"tZone.uSerial,tZone.uTTL,tZone.uExpire,tZone.uRefresh,tZone.uRetry,tZone.uZoneTTL,"
				"tZone.uMailServers,tZone.cMainAddress,tView.cLabel,tZone.cOptions FROM tZone,tNSSet,tNS,tView"
				" WHERE tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND"
					" tNSSet.uNSSet=%u AND"
				" tZone.uView=tView.uView AND tZone.cZone='%s'"
						,uNSSet,cZone);
			}
			else
			{
				sprintf(gcQuery,"SELECT DISTINCT tZone.cZone,tZone.uZone,tZone.uNSSet,tZone.cHostmaster,"
				"tZone.uSerial,tZone.uTTL,tZone.uExpire,tZone.uRefresh,tZone.uRetry,tZone.uZoneTTL,"
				"tZone.uMailServers,tZone.cMainAddress,tView.cLabel,tZone.cOptions FROM tZone,tNSSet,tNS,tView"
				" WHERE tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND"
					" tNSSet.uNSSet=%u AND"
				" tZone.uView=tView.uView ORDER BY tZone.cZone",uNSSet);
			}
		}

		mysql_query(&gMysql,gcQuery);
		//debug only
		//printf("(uModDBFiles)%s\n",gcQuery);
		if(mysql_errno(&gMysql)) 
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
		
		res=mysql_store_result(&gMysql);
		while((field=mysql_fetch_row(res)))
		{
			char *cp;
			char cFirstNS[100]={""};
	
			//write master db file. first create dir if needed (fail ok)
			sprintf(gcQuery,"/usr/local/idns/named.d/master/%s",
					field[12]);
			if(!uDebug)
			{
				mkdir(gcQuery,0777);
				chown(gcQuery,uUID,uGID);
			}

			sprintf(gcQuery,"/usr/local/idns/named.d/master/%s/%c",
					field[12],field[0][0]);
			if(!uDebug)
			{
				mkdir(gcQuery,0777);
				chown(gcQuery,uUID,uGID);
			}

			if((cp=strchr(field[0],'/')))
			{
				*cp=0;
				sprintf(gcQuery,"/usr/local/idns/named.d/master/%s/%c/%s",
					field[12],field[0][0],field[0]);
				if(!uDebug)
				{
					mkdir(gcQuery,0777);
					chown(gcQuery,uUID,uGID);
				}
				*cp='/';
			}

			sprintf(gcQuery,"/usr/local/idns/named.d/master/%s/%c/%s",
					field[12],field[0][0],field[0]);
			sprintf(cZoneFile,"%.511s",gcQuery);
			if(!uDebug)
			{
				if(!(zfp=fopen(gcQuery,"w")))
				{
					fprintf(stderr,"Could not open master zone db file: %s\n",
						gcQuery);
					exit(1);
				}
			}
			else
			{
				fprintf(stdout,"\n(%s)\n",gcQuery);
			}
			if(!dnfp)
			{
				if(!(dnfp=fopen("/dev/null","a")))
				{
					fprintf(stderr,"Could not open /dev/null file\n");
					exit(1);
				}
			}

			//0 cZone
			//1 uZone
			//2 uNSSet
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
			//13 tZone.cOptions
			sscanf(field[1],"%u",&uZone);
	
			if((cp=strchr(field[3],' '))) *cp=0;

			fprintf(zfp,"; %s\n",field[0]);
			fprintf(zfp,"$TTL %s\n",field[5]);
	
			//MASTER HIDDEN support
			sprintf(cFirstNS,"%.99s",cPrintNSList(dnfp,field[2]));
			if(cFirstNS[0])
				fprintf(zfp,
				"@ IN SOA %s. %s. (\n",cFirstNS,field[3]);
			else
				fprintf(zfp,
				"@ IN SOA %s. %s. (\n",cMasterNS,field[3]);
			fprintf(zfp,"\t\t\t%s\t;serial\n",field[4]);
			fprintf(zfp,"\t\t\t%s\t\t;slave refresh\n",field[7]);
			fprintf(zfp,"\t\t\t%s\t\t;slave retry\n",field[8]);
			fprintf(zfp,"\t\t\t%s\t\t;slave expiration\n",field[6]);
			fprintf(zfp,"\t\t\t%s )\t\t;negative ttl\n\n",
				field[9]);

			//ns
			if(field[13][0] && strstr(field[13],"//NoNonRRNS")!=NULL)
				;
			else
				cPrintNSList(zfp,field[2]);

			//mx1
			PrintMXList(zfp,field[10]);
		
			//in a 0.0.0.0 is the null IP number
			if(field[11][0] && field[11][0]!='0')
				fprintf(zfp,"\t\tA %s\n;\n",field[11]);
			fprintf(zfp,";\n");

			//TODO
			if(!strcmp(field[0]+strlen(field[0])-5,".arpa"))
				sprintf(gcQuery,"SELECT cName,uTTL,uRRType,cParam1,cParam2 FROM"
						" tResource WHERE uZone=%u ORDER BY uResource",uZone);
			else
				sprintf(gcQuery,"SELECT cName,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4 FROM"
					" tResource WHERE uZone=%u ORDER BY cName",uZone);
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
						fprintf(zfp,"%s\t%s\t%s\t%s\t%s(\t%s\t%s)\n",
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
			if(zfp && !uDebug) fclose(zfp);

			//Check with ISC tools and report to dashboard
			int iRetVal;

			uZoneOwner=uGetZoneOwner(uZone);
			if(!strchr(cZoneFile,' '))
			{
				sprintf(gcQuery,"%s/named-checkzone -q %s %s",gcBinDir,field[0],cZoneFile);
				if((iRetVal=system(gcQuery))>0)
				{
					fprintf(stdout,"%s returned %d\n",gcQuery,iRetVal);

					//Command failed, create tLog entry
					sprintf(gcQuery,"INSERT INTO tLog SET uLogType=4,uPermLevel=12,uLoginClient=1,"
					"cLogin='JobQueue',cHost ='127.0.0.1',uTablePK='%u',cTableName='tZone',"
					"cMessage='Zone %.99s with errors',cServer='%s',uOwner=%u,uCreatedBy=1,"
					"uCreatedDate=UNIX_TIMESTAMP(NOW()),cLabel='named-checkzone'",
						uZone,field[0],cMasterNS,uZoneOwner);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
				}
			}
			else
			{
					//Command failed, create tLog entry
					sprintf(gcQuery,"INSERT INTO tLog SET uLogType=4,uPermLevel=12,uLoginClient=1,"
					"cLogin='JobQueue',cHost ='127.0.0.1',uTablePK='%u',cTableName='tZone',"
					"cMessage='Zone %.64s with space error',cServer='%s',uOwner=%u,uCreatedBy=1,"
					"uCreatedDate=UNIX_TIMESTAMP(NOW()),cLabel='named-checkzone'",
						uZone,field[0],cMasterNS,uZoneOwner);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
			}

		}
		mysql_free_result(res);

	}//if uModDBFile

	if(dnfp) fclose(dnfp);
	rmdir("/var/run/unxsBind.lock");
	logfileLine("CreateMasterFiles","Exit");

}//void CreateMasterFiles()


void CreateSlaveFiles(char *cSlaveNS, char *cZone, char *cMasterIP, unsigned uDebug)
{
	//Create all slave zone stubs in include file.
	MYSQL_RES *res;
	MYSQL_ROW field;
	FILE *fp=stdout;
	unsigned uView=0,uCurrentView=0,uFirst=1;
	char cViewXIP[256]={""};
	char cViewXIPName[100]={""};
	char cuUID[256]={""};
	char cuGID[256]={""};
	unsigned uGID=25;
	unsigned uUID=25;

	GetConfiguration("cuUID",cuUID,0);
	if(cuUID[0]) sscanf(cuUID,"%u",&uUID);
	GetConfiguration("cuGID",cuGID,0);
	if(cuGID[0]) sscanf(cuGID,"%u",&uGID);

	//Handle second optional NS
	char cSlaveNS1[100]={""};
	char cSlaveNS2[100]={""};
	char *cpNS;
	if((cpNS=strchr(cSlaveNS,',')))
	{
		sprintf(cSlaveNS2,"%.99s",cpNS+1);
		*cpNS=0;//for ns1
		sprintf(cSlaveNS1,"%.99s",cSlaveNS);
		*cpNS=',';//for return to as on entry
	}
	else
	{
		sprintf(cSlaveNS1,"%.99s",cSlaveNS);
	}
	//debug only
	//printf("CreateSlaveFiles()cSlaveNS2=%s\n",cSlaveNS2);
	logfileLine("CreateSlaveFiles",cSlaveNS1);
	if(cSlaveNS2[0])
		logfileLine("CreateSlaveFiles",cSlaveNS2);

	//servers can be both master and slave
	if(cSlaveNS2[0])
		sprintf(gcQuery,"SELECT DISTINCT"
			" tZone.cZone,tView.cLabel,tView.cSlave,tView.uView,tNSSet.cMasterIPs,tZone.cOptions,tZone.uSecondaryOnly"
			" FROM tZone,tNSSet,tNS,tView"
			" WHERE tZone.uNSSet=tNSSet.uNSSet"
			" AND tNSSet.uNSSet=tNS.uNSSet"
			" AND tNS.uNSType=4"
			" AND tZone.uView=tView.uView"
			" AND (tNS.cFQDN='%s' OR tNS.cFQDN='%s')"
			" ORDER BY tView.uOrder,tZone.cZone",cSlaveNS1,cSlaveNS2);
	else
		sprintf(gcQuery,"SELECT DISTINCT"
			" tZone.cZone,tView.cLabel,tView.cSlave,tView.uView,tNSSet.cMasterIPs,tZone.cOptions,tZone.uSecondaryOnly"
			" FROM tZone,tNSSet,tNS,tView"
			" WHERE tZone.uNSSet=tNSSet.uNSSet"
			" AND tNSSet.uNSSet=tNS.uNSSet"
			" AND tNS.uNSType=4"
			" AND tZone.uView=tView.uView"
			" AND tNS.cFQDN='%s'"
			" ORDER BY tView.uOrder,tZone.cZone",cSlaveNS1);
	mysql_query(&gMysql,gcQuery);
	//debug only
	//printf("%s\n",gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	
	if(mysql_num_rows(res)<1) 
	{
		logfileLine("CreateSlaveFiles","No zones found for slave");
		return;
	}

	//master lock dir
	struct stat statInfo;
	if(stat("/var/run/unxsBind.lock",&statInfo))
	{
		mkdir("/var/run/unxsBind.lock",S_IRWXU);
	}
	else
	{
		fprintf(stdout,"/var/run/unxsBind.lock exists exiting\n");
		return;
	}

	if(!uDebug)
	{
		if(!(fp=fopen("/usr/local/idns/named.d/slave.zones","w")))
		{
			logfileLine("CreateSlaveFiles","Could not open slave.zones");
			exit(1);
		}
	}

	char cNoViewSlaveFileName[256]={""};
	char cNoViewSlaveFile[256]={""};
	sprintf(cNoViewSlaveFileName,"cNoViewSlaveFile:%.99s",cSlaveNS1);
	GetConfiguration(cNoViewSlaveFileName,cNoViewSlaveFile,0);

	while((field=mysql_fetch_row(res)))
	{
		sscanf(field[3],"%u",&uView);
		if(uCurrentView!=uView && !cNoViewSlaveFile[0])
		{
			char cExtraForViewName[100]={""};
			char cExtraForView[1024]={""};

			uCurrentView=uView;//Actually tView.uOrder
			if(!uFirst)
				fprintf(fp,"};\n");
			uFirst=0;
			fprintf(fp,"\n%s\n",field[2]);

			sprintf(cExtraForViewName,"cExtraForViewSlave:%.31s",field[1]);
			GetConfiguration(cExtraForViewName,cExtraForView,0);
			if(cExtraForView[0])
				fprintf(fp,"%s\n",cExtraForView);

			cViewXIP[0]=0;
			//Note optional second NS must have same view settings as first
			//if not "we have a problem houston."
			sprintf(cViewXIPName,"cViewXIP:%.31s:%.31s",cSlaveNS1,field[1]);
			GetConfiguration(cViewXIPName,cViewXIP,0);
			//debug only
			//fprintf(fp,"//cViewXIPName=%s cViewXIP=%s\n",cViewXIPName,cViewXIP);
		}


		fprintf(fp,"\tzone \"%s\" {\n",field[0]);
		fprintf(fp,"\t\ttype slave;\n");
		if(field[4][0])
			fprintf(fp,"\t\tmasters { %s };\n",field[4]);
		else
			//backwards compatability
			fprintf(fp,"\t\tmasters { %s; };\n",cMasterIP);
		if(field[5][0])
			fprintf(fp,"\t\t//tZone.cOptions\n\t\t%s\n",field[5]);
		//For uSecondaryOnly=1 zones we use the default transfer-source
		if(cViewXIP[0] && field[6][0]=='0')
		{
			fprintf(fp,"\t\ttransfer-source %.16s;\n",cViewXIP);
		}
		fprintf(fp,"\t\tfile \"slave/%s/%c/%s\";\n\t};\n",
					field[1],field[0][0],field[0]);

		//Make dirs as we go
		if(!uDebug)
		{
			char *cp;

			sprintf(gcQuery,"/usr/local/idns/named.d/slave/%s",
				field[1]);
			mkdir(gcQuery,0777);
			chown(gcQuery,uUID,uGID);

			sprintf(gcQuery,"/usr/local/idns/named.d/slave/%s/%c",
				field[1],field[0][0]);
			mkdir(gcQuery,0777);
			chown(gcQuery,uUID,uGID);

			if((cp=strchr(field[0],'/')))
			{
				*cp=0;
				sprintf(gcQuery,"/usr/local/idns/named.d/slave/%s/%c/%s",
					field[1],field[0][0],field[0]);
				if(!uDebug)
				{
					mkdir(gcQuery,0777);
					chown(gcQuery,uUID,uGID);
				}
				*cp='/';
			}
		}
	}
	if(!cNoViewSlaveFile[0])
		fprintf(fp,"};\n");
	mysql_free_result(res);
	if(fp && !uDebug) fclose(fp);
	rmdir("/var/run/unxsBind.lock");
	logfileLine("CreateSlaveFiles","Exit");

}//void CreateSlaveFiles();


//Initial install function
void InstallNamedFiles(char *cIpNum)
{
	char cISMROOT[256]={""};
	char cSetupDir[16]={"setup9"};

	if(getenv("ISMROOT")!=NULL)
	{               
		strncpy(cISMROOT,getenv("ISMROOT"),255);
		gcHost[255]=0;
	}
	
	if(!cISMROOT[0])
	{
		fprintf(stdout,"You must set ISMROOT env var first. Ex. export ISMROOT=/home/joe/unxsVZ\n"
			"If the iDNS dir is located in the /home/joe/unxsVZ dir. For source"
			" code installs you may need to 'ln -s unxsBind iDNS' inside your unxsVZ tree.");
		exit(0);
	}

	fprintf(stdout,"Installing named for IP %s from %s/iDNS\n",
			IPNumber(cIpNum), cISMROOT);
	
	mkdir("/usr/local/idns",0777);
	mkdir("/usr/local/idns/named.d",0777);
	mkdir("/usr/local/idns/named.d/master",0777);
	mkdir("/usr/local/idns/named.d/slave",0777);

	fprintf(stdout,"Configuring and installing files...\n");

	//for rpm initial install, being deprecated.
	if(strcmp(cIpNum,"0.0.0.0"))
	{
		sprintf(gcQuery,"cat %s/iDNS/%s/named.conf|sed -e \"s/{{cIpNumber}}/%s/g\" > /usr/local/idns/named.conf",
			cISMROOT,cSetupDir,IPNumber(cIpNum));	
		if(system(gcQuery))
			fprintf(stdout,"Error configuring named.conf\n");
	}
	else
	{
		sprintf(gcQuery,"cat %s/iDNS/%s/named.conf > /usr/local/idns/named.conf",cISMROOT,cSetupDir);	
		if(system(gcQuery))
			fprintf(stdout,"Error configuring named.conf\n");
	}

	sprintf(gcQuery,"touch /usr/local/idns/named.d/master.zones");	
	if(system(gcQuery))
		 fprintf(stdout,"Error configuring master.zones\n");

	sprintf(gcQuery,"touch /usr/local/idns/named.d/slave.zones");	
	if(system(gcQuery))
		 fprintf(stdout,"Error configuring slave.zones\n");

	sprintf(gcQuery,"cat %s/iDNS/%s/localhost > /usr/local/idns/named.d/master/localhost",cISMROOT,cSetupDir);	
	if(system(gcQuery))
		 fprintf(stdout,"Error configuring localhost\n");

	sprintf(gcQuery,"cat %s/iDNS/%s/127.0.0 > /usr/local/idns/named.d/master/127.0.0",cISMROOT,cSetupDir);	
	if(system(gcQuery))
		 fprintf(stdout,"Error configuring 127.0.0\n");

	sprintf(gcQuery,"/usr/bin/dig . ns > /usr/local/idns/named.d/root.cache");	
	if(system(gcQuery))
	{
		fprintf(stdout,"Error configuring root.cache via dig, falling back to distribution root.cache\n");
		sprintf(gcQuery,"cat %s/iDNS/%s/root.cache > /usr/local/idns/named.d/root.cache",cISMROOT,cSetupDir);	
		if(system(gcQuery))
			fprintf(stdout,"Error configuring root.cache\n");
	}

	fprintf(stdout,"Done.\n");
	exit(0);


}//void InstallNamedFiles(char *cIpNum)


void SerialNum(char *serial)
{
	time_t luClock;
		
	//Make new today serial number
	time(&luClock);
	strftime(serial,12,"%Y%m%d00",localtime(&luClock));

}//void SerialNum(char *serial)


void ExportTable(char *cTable, char *cFile)
{
	fprintf(stdout,"ExportTable() Not implemented yet\n");
	exit(0);
}//void ExportTable(char *cTable, char *cFile)


void SlaveJobQueue(char *cNameServer, char *cMasterIP)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	register int first=1;
	char cCurrentZone[100]={" "};
	unsigned uModify=0;
	unsigned uNew=0;
	unsigned uDelete=0;
	unsigned uDeleteFirst=0;
	unsigned uChanged=0;

#ifdef cLOGFILE
	if((gLfp=fopen(cLOGFILE,"a"))==NULL)
	{
		fprintf(stderr,"Could not open logfile: %s\n",cLOGFILE);
		exit(300);
       	}
#else
	gLfp=stdout;
#endif
	
	if(TextConnectDb())
		exit(0);
	
	//Handle second optional NS
	char cNameServer2[100]={""};
	char *cp;
	if((cp=strchr(cNameServer,',')))
	{
		sprintf(cNameServer2,"%.99s",cp+1);
		*cp=0;
	}
	if(cNameServer2[0])
	{
		sprintf(gcQuery,"SELECT uJob,cJob,cZone,uMasterJob FROM tJob WHERE"
			" (cTargetServer='%s SLAVE' OR cTargetServer='%s SLAVE')"
			" AND uTime<=UNIX_TIMESTAMP(NOW()) ORDER BY cZone,uJob",cNameServer,cNameServer2);
		*cp=',';//for debug info below
	}
	else
	{
		sprintf(gcQuery,"SELECT uJob,cJob,cZone,uMasterJob FROM tJob WHERE cTargetServer='%s SLAVE'"
			" AND uTime<=UNIX_TIMESTAMP(NOW()) ORDER BY cZone,uJob",cNameServer);
	}

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stdout,"%s -1\n",mysql_error(&gMysql));
		exit(1);
	}

	res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res)) 
	{
		MYSQL_RES *res2;

		while((field=mysql_fetch_row(res)))
		{
			//If MASTER EXTERNAL delete it from job queue since
			//since it will never be 'handled.'
			sprintf(gcQuery,"SELECT uJob FROM tJob WHERE uMasterJob=%s AND"
					" cTargetServer LIKE '%% MASTER EXTERNAL'",field[3]);
			mysql_query(&gMysql,gcQuery);
			//debug only
			//fprintf(stdout,"%s\n",gcQuery);
			if(mysql_errno(&gMysql)) 
			{
				fprintf(stdout,"%s\n",mysql_error(&gMysql));
				exit(1);
			}
			res2=mysql_store_result(&gMysql);
			if(mysql_num_rows(res2)) 
			{
				sprintf(gcQuery,"DELETE FROM tJob WHERE uMasterJob=%s AND"
						" cTargetServer LIKE '%% MASTER EXTERNAL'",field[3]);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) 
				{
					fprintf(stdout,"%s\n",mysql_error(&gMysql));
					exit(1);
				}
				logfileLine("SlaveJobQueue","Deleted MASTER EXTERNAL job");
			}
			mysql_free_result(res2);
			
			//If MASTER OR MASTER HIDDEN still in queue skip.
			sprintf(gcQuery,"SELECT uJob FROM tJob WHERE uMasterJob=%s AND"
					" cTargetServer LIKE '%% MASTER%%'",field[3]);//3-uMasterJob
			mysql_query(&gMysql,gcQuery);
			//debug only
			//fprintf(stdout,"%s\n",gcQuery);
			if(mysql_errno(&gMysql)) 
			{
				fprintf(stdout,"%s\n",mysql_error(&gMysql));
				exit(1);
			}
			res2=mysql_store_result(&gMysql);
			if(mysql_num_rows(res2)>0) 
			{
				mysql_free_result(res2);
				//Ignore this job until 'REAL' MASTER
				//handles it.
				logfileLine("SlaveJobQueue","Skipping job not yet handled by MASTER");
				//debug only
				//fprintf(stdout,"d1 %s\n",field[2]);
				continue;
			}
			//debug only
			//fprintf(stdout,"d2 %s\n",field[2]);
			mysql_free_result(res2);
			
			//Start processing SLAVE jobs

			if(strcmp(field[2],cCurrentZone))
			{
				if(!first)
				{
					//debug only
					//fprintf(stdout,"uDelete=%u uModify=%u uNew=%u uDeleteFirst=%u\n",
					//	uDelete,uModify,uNew,uDeleteFirst); 
					uChanged+=ProcessSlaveJob(cCurrentZone,uDelete,uModify,
							uNew,uDeleteFirst,cNameServer,cMasterIP);
					uModify=0;
					uNew=0;
					uDelete=0;
					uDeleteFirst=0;
				}
				else
				{
					first=0;
				}
				strcpy(cCurrentZone,field[2]);
			}
			//debug only
			//fprintf(stdout,"%s\t%s\t%s\n",field[0],field[2],field[1]);
			//Allow for combinations: Modify New, Delete New. Modify overrides a Delete.
			if(strstr(field[1],"New")) uNew++;
			if(strstr(field[1],"Modify")) 
				uModify++;
			else if(strstr(field[1],"Delete")) 
				uDelete++;
			if(uDelete && !uNew) uDeleteFirst=1;

			//Remove job from queue
			sprintf(gcQuery,"DELETE FROM tJob WHERE uJob=%s",field[0]);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql)) 
			{
				fprintf(stdout,"%s\n",mysql_error(&gMysql));
				exit(1);
			}

		}
	}
	mysql_free_result(res);

	if(!first)
	{
		//debug only
		//fprintf(stdout,"uDelete=%u uModify=%u uNew=%u uDeleteFirst=%u\n",
		//				uDelete,uModify,uNew,uDeleteFirst); 
		uChanged+=ProcessSlaveJob(cCurrentZone,uDelete,uModify,
				uNew,uDeleteFirst,cNameServer,cMasterIP);
	}
	
	if(uChanged)
	{
		char cuControlPort[8]={""};
		char cCmd[100]={""};
	
		GetConfiguration("cuControlPort",cuControlPort,0);
		
		//Check to see if reconfigure is ok for slaves
		//debug only
		//fprintf(stdout,"Reconfiguring slave server...");
		if((uNamedCheckConf(cNameServer))) exit(1); //Exit without reloading the server
		if(cuControlPort[0])
			sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf -p %s reconfig > /dev/null 2>&1",
				gcBinDir,cuControlPort);
		else
			sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf reconfig > /dev/null 2>&1",gcBinDir);
		
		if(system(cCmd))
			exit(1);
		//debug only
		//fprintf(stdout,"OK\n");
	}

	exit(0);

}//void SlaveJobQueue()


void MasterJobQueue(char *cNameServer)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	register int first=1;
	char cCurrentZone[100]={" "};
	unsigned uModify=0;
	unsigned uNew=0;
	unsigned uDelete=0;
	unsigned uDeleteFirst=0;
	unsigned uChanged=0;
	char *cp;
	char cuControlPort[8]={""};
	char cCmd[100]={""};

#ifdef cLOGFILE
	if((gLfp=fopen(cLOGFILE,"a"))==NULL)
	{
		fprintf(stderr,"Could not open logfile: %s\n",cLOGFILE);
		exit(300);
       	}
#else
	gLfp=stdout;
#endif

	if(TextConnectDb())
		exit(0);

	GetConfiguration("cuControlPort",cuControlPort,0);


	//debug only
	//fprintf(stdout,"MasterJobQueue(%s %s)\n",cNameServer,cNameServer2);


	//Handle second optional NS
	char cNameServer2[100]={""};
	if((cp=strchr(cNameServer,',')))
	{
		sprintf(cNameServer2,"%.99s",cp+1);
		*cp=0;
	}
	//MASTER OR MASTER HIDDEN
	if(cNameServer2[0])
	{
		sprintf(gcQuery,"SELECT uJob,cJob,cZone,cJobData FROM tJob"
			" WHERE (cTargetServer='%s MASTER' OR"
			" cTargetServer='%s MASTER' OR"
			" cTargetServer='%s MASTER HIDDEN' OR"
			" cTargetServer='%s MASTER HIDDEN') AND"
			" uTime<=UNIX_TIMESTAMP(NOW()) ORDER BY cZone,uJob",
					cNameServer,cNameServer2,cNameServer,cNameServer2);
		*cp=',';//for debug info below
	}
	else
	{
		sprintf(gcQuery,"SELECT uJob,cJob,cZone,cJobData FROM tJob WHERE (cTargetServer='%s MASTER' OR"
			" cTargetServer='%s MASTER HIDDEN') AND uTime<=UNIX_TIMESTAMP(NOW()) ORDER BY cZone,uJob",
					cNameServer,cNameServer);
	}

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stdout,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	//debug only
	//printf("debug 1. cNameServer:%s cNameServer2:%s\n",cNameServer,cNameServer2);
	//exit(0);

	res=mysql_store_result(&gMysql);
	if(mysql_num_rows(res)) 
	{
		while((field=mysql_fetch_row(res)))
		{
			if(strcmp(field[2],cCurrentZone))
			{
				if(!first)
				{
					//debug only
					//fprintf(stdout,"uDelete=%u uModify=%u uNew=%u uDeleteFirst=%u\n",
					//	uDelete,uModify,uNew,uDeleteFirst); 
					uChanged+=ProcessMasterJob(cCurrentZone,uDelete,uModify,
						uNew,uDeleteFirst,cNameServer);
					if(uChanged==2)
					{
						guReconfig=1;
						logfileLine("MasterJobQueue.ProcessMasterJob1","guReconfig=1");
					}
					else if(uChanged==3)
					{
						guReload=1;
						logfileLine("MasterJobQueue.ProcessMasterJob1","guReload=1");
					}
					uModify=0;
					uNew=0;
					uDelete=0;
					uDeleteFirst=0;
				}
				else
				{
					first=0;
				}
				strcpy(cCurrentZone,field[2]);
			}
					//debug only
			//fprintf(stdout,"%s\t%s\t%s\n",field[0],field[2],field[1]);
			//Allow for combinations: Modify New, Delete New. Modify overrides a Delete.
			if(strstr(field[1],"New"))
			{
				uNew++;
				sprintf(cCmd,"uNew=%u",uNew);
				logfileLine("MasterJobQueue",cCmd);
			}
			if(strstr(field[1],"Modify")) 
			{
				uModify++;
				sprintf(cCmd,"uModify=%u",uModify);
				logfileLine("MasterJobQueue",cCmd);
			}
			else if(strstr(field[1],"Delete")) 
			{
				uDelete++;
				sprintf(cCmd,"uDelete=%u",uDelete);
				logfileLine("MasterJobQueue",cCmd);
			}
			if(uDelete && !uNew)
			{
				uDeleteFirst=1;
				sprintf(cCmd,"uDeleteFirst=%u",uDeleteFirst);
				logfileLine("MasterJobQueue",cCmd);
			}

			//Inform ext queue of completion. Only for MASTER
			//no error checking takes place fix this.
			if(!strncmp(field[1],"Ext",3) && (cp=strstr(field[3],"uJob=")))
			{
				char cMsg[33];
				unsigned uExtJob=0;

				TextConnectExtDb(&gMysql2,0);//uMode==0 unxsISP
				sprintf(cMsg,"iDNS.%.20s",field[1]);
				sscanf(cp+5,"%u",&uExtJob);
				InformExtISPJob(cMsg,cNameServer,
						uExtJob,mysqlISP_Deployed);
				logfileLine("MasterJobQueue.ExtJob",cMsg);
			}

			//Remove job from queue
			sprintf(gcQuery,"DELETE FROM tJob WHERE uJob=%s",field[0]);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql)) 
			{
				fprintf(stdout,"%s\n",mysql_error(&gMysql));
				exit(1);
			}

		}
	}
	mysql_free_result(res);

	if(!first)
	{
		//debug only
		//fprintf(stdout,"uDelete=%u uModify=%u uNew=%u uDeleteFirst=%u\n",
		//				uDelete,uModify,uNew,uDeleteFirst); 
		if((uChanged=ProcessMasterJob(cCurrentZone,uDelete,uModify,
				uNew,uDeleteFirst,cNameServer)))
		{
			if(uChanged==2)
			{
				guReconfig=1;
				logfileLine("MasterJobQueue.ProcessMasterJob2","guReconfig=1");
			}
			else if(uChanged==3)
			{
				guReload=1;
				logfileLine("MasterJobQueue.ProcessMasterJob2","guReload=1");
			}
		}
	}

	if(guReload)
	{
		if((uNamedCheckConf(cNameServer))) exit(1); //Will exit without server reload or reconfig
		if(cuControlPort[0])
			sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf -p %s reload > /dev/null 2>&1",
				gcBinDir,cuControlPort);
		else
			sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf reload > /dev/null 2>&1",gcBinDir);
		
		logfileLine("MasterJobQueue.guReload",cCmd);
		if(system(cCmd))
			exit(1);
	}
	else if(guReconfig)
	{
		if((uNamedCheckConf(cNameServer))) exit(1); //Will exit without server reload or reconfig
		if(cuControlPort[0])
			sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf -p %s reconfig > /dev/null 2>&1",
				gcBinDir,cuControlPort);
		else
			sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf reconfig > /dev/null 2>&1",gcBinDir);
		
		logfileLine("MasterJobQueue.guReconfig",cCmd);
		if(system(cCmd))
			exit(1);
	}
	//Create master.zones again.
	//CreateMasterFiles(cNameServer,"",0,1,0);

	exit(0);

}//void MasterJobQueue(char *cNameServer)


int ProcessMasterJob(char *cZone,unsigned uDelete,unsigned uModify,
			unsigned uNew,unsigned uDeleteFirst,char *cMasterNS)
{

	sprintf(gcQuery,"cZone=%.99s;cMasterNS=%.99s;uDelete=%u;uModify=%u;uNew=%u,uDeleteFirst=%u;",
			cZone,cMasterNS,uDelete,uModify,uNew,uDeleteFirst);
	logfileLine("ProcessMasterJob",gcQuery);
	//return 0 if nothing needs to be done
	//return 1 if zone info has changed
	//return 2 if zone is added or deleted
	//return 3 if zone is modified but we need a reload anyway
	//debug only
	//fprintf(stdout,"Queue Policy for %s: ",cZone);
	if(uDelete && !uNew)
	{
		logfileLine("ProcessMasterJob","Delete");
		//Replace master.zones named.conf include file
		//All Zones replace, DBFiles No, Stubs Yes
		CreateMasterFiles(cMasterNS,"",0,1,0);
		return(2);
	}
	else if(uDelete && uNew && !uDeleteFirst)
	{
		logfileLine("ProcessMasterJob","New then Delete");
		return(0);
	}
	else if(uNew)
	{
		//debug only
		if(uModify)
			logfileLine("ProcessMasterJob","New+Modify");
		else
			logfileLine("ProcessMasterJob","New");
		//Append to master.zones named.conf include file
		//Single zone append, DBFiles yes, Stubs yes
		CreateMasterFiles(cMasterNS,cZone,1,1,0);
		return(2);//rndc reconfig works here for both cases
	}
	else if(uModify)
	{
		//debug only
		logfileLine("ProcessMasterJob","Modify");
		//Single zone, DBFiles yes, Stubs yes
		//We will always build stubs when modyfing a zone
		//This was learned by running 2nd NS set support test case #4
		CreateMasterFiles(cMasterNS,cZone,1,1,0);
		//Single zone reload
		if(!guReload)//Set by return(3) above global
			ViewReloadZone(cZone);
		return(1);
	}
	
	return(0);

}//int ProcessMasterJob();


int ProcessSlaveJob(char *cZone,unsigned uDelete,unsigned uModify,unsigned uNew,
			unsigned uDeleteFirst, char *cMasterNS, char *cMasterIP)
{
	char cuControlPort[8]={""};
	char cCmd[100]={""};

	GetConfiguration("cuControlPort",cuControlPort,0);
	
	//debug only
	//fprintf(stdout,"Queue Policy for %s: ",cZone);
	sprintf(gcQuery,"cZone=%.99s;cNS=%.99s;cIP=%.32s;",cZone,cMasterNS,cMasterIP);
	logfileLine("ProcessSlaveJob",gcQuery);
	if(uDelete && !uNew)
	{
		//debug only
		//fprintf(stdout,"Delete for NS %s\n\n",cMasterNS);
		logfileLine("ProcessSlaveJob","Delete");
		//Replace slave.zones named.conf include file
		//All Zones replace
		CreateSlaveFiles(cMasterNS,"",cMasterIP,0);
		return(1);
	}
	else if(uDelete && uNew && !uDeleteFirst)
	{
		//debug only
		//fprintf(stdout,"New then Delete\n\n");
		logfileLine("ProcessSlaveJob","New+Delete Ignored");
		return(0);
	}
	else if(uModify)
	{
		//debug only
		if(uNew)
			logfileLine("ProcessSlaveJob","New+Modify");
		else
			logfileLine("ProcessSlaveJob","Modify");
		CreateSlaveFiles(cMasterNS,"",cMasterIP,0);
		if(cuControlPort[0])
			sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf -p %s reload > /dev/null 2>&1",
				gcBinDir,cuControlPort);
		else
			sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf reload > /dev/null 2>&1",gcBinDir);
		system(cCmd);
		ViewReloadZone(cZone);
		return(0);
	}
	else if(uNew)
	{
		//debug only
		logfileLine("ProcessSlaveJob","New");
		//Append to slave.zones named.conf include file
		//Single zone append
		CreateSlaveFiles(cMasterNS,cZone,cMasterIP,0);
		//This is a hack has to be optimized further
		if(cuControlPort[0])
			sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf -p %s reload > /dev/null 2>&1",
				gcBinDir,cuControlPort);
		else
			sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf reload > /dev/null 2>&1",gcBinDir);
		system(cCmd);
		ViewReloadZone(cZone);
		return(1);//reconfigure: new item in slave.zones
	}
	
	return(0);

}//int ProcessSlaveJob();


void GenerateArpaZones(void)
{
	MYSQL_RES *res;

	char cZone[100];
	unsigned uZone=0;
	unsigned uOwner=0;
	unsigned uNSSet=0;

	//Zone A Records
	sprintf(gcQuery,"SELECT cZone,cMainAddress,uZone,uOwner,uNSSet FROM tZone WHERE cZone NOT LIKE '%%.arpa'");
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql)) 
		tZone(mysql_error(&gMysql));
	res=mysql_store_result(&gMysql);
	
	Header_ism3("GenerateArpaZones()",0);
	fprintf(stdout,"</center><pre>\n");

	if(mysql_num_rows(res)) 
	{
		MYSQL_ROW field;
		while((field=mysql_fetch_row(res)))
		{
			strcpy(cZone,field[0]);
			sscanf(field[2],"%u",&uZone);
			sscanf(field[3],"%u",&uOwner);
			sscanf(field[4],"%u",&uNSSet);
			if( !strcmp(field[1],"0.0.0.0") || field[1][0]==0)
				continue;
			
			if(PopulateArpaZone(field[0],field[1],0,uZone,uOwner,uNSSet)) continue;

		}
	}
	mysql_free_result(res);
	
	
	//A Resource Records
	sprintf(gcQuery,"SELECT tZone.cZone,tResource.cParam1,tResource.cName,tZone.uZone,tZone.uOwner,tZone.uNSSet FROM"
			" tResource,tZone,tRRType WHERE tResource.uZone=tZone.uZone AND"
			" tResource.uRRType=tRRType.uRRType AND tRRType.cLabel='A'");
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql)) 
	{
		fprintf(stdout,"%s\n",mysql_error(&gMysql));
		Footer_ism3();
	}
	res=mysql_store_result(&gMysql);
	
	if(mysql_num_rows(res)) 
	{
		MYSQL_ROW field;
		while((field=mysql_fetch_row(res)))
		{
			//If cName=field[2] if FQDN leave as is else
			//add zone to it.
			if(field[2][strlen(field[2])-1]=='.')
				sprintf(cZone,"%s",field[2]);
			else if(field[2][0] && field[2][0]!='\t')
				sprintf(cZone,"%s.%s",field[2],field[0]);
			//default case
			else if(1)
				sprintf(cZone,"%s",field[0]);

			//debug only
			//fprintf(stdout,"(%s) (%s) (%s) [%s]\n",
			//		field[2],field[0],cZone,field[1]);
			//Footer_ism3();

			sscanf(field[3],"%u",&uZone);
			sscanf(field[4],"%u",&uOwner);
			sscanf(field[5],"%u",&uNSSet);
			if(PopulateArpaZone(cZone,field[1],0,uZone,uOwner,uNSSet)) continue;
		}
	}
	mysql_free_result(res);

	fprintf(stdout,"Remember to select and remove duplicate PTR records from .arpa zones now.\n");
	Footer_ism3();

}//void GenerateArpaZones(void)


int PopulateArpaZoneIPv6(const char *cIPv6ArpaZoneName, const char *cIPv6PTR, const char *cHostname, const unsigned uHtmlMode, 
					const unsigned uFromZone, const unsigned uZoneOwner, const unsigned uNSSet)
{
	MYSQL_RES *res2;
	MYSQL_ROW field;
	unsigned uZone=0;
	unsigned uNew=0;
	char cHostMaster[256]="hostmaster.isp.net";
	char cuView[256]="2";

	GetConfiguration("cHostMaster",cHostMaster,0);
	GetConfiguration("cuView",cuView,0);

	sprintf(gcQuery,"SELECT cZone,uZone FROM tZone WHERE cZone='%s' AND uView=%.2s",
			cIPv6ArpaZoneName,cuView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		if(uHtmlMode)
			htmlPlainTextError(mysql_error(&gMysql));
		else
			fprintf(stdout,"PopulateArpaZoneIPv6() Select Error: %s\n",mysql_error(&gMysql));
		return(1);
	}
	res2=mysql_store_result(&gMysql);
	if(!mysql_num_rows(res2)) 
	{
		if(!uHtmlMode)
			fprintf(stdout,"<font color=blue>Adding new ip6.arpa zone: %s</font>\n",cIPv6ArpaZoneName);
		//This adds view 1 nd view 2 only hardcoded please fix TODO
		if(AddNewArpaZone(cIPv6ArpaZoneName,uNSSet,cHostMaster,uZoneOwner)) 
		{
			if(uHtmlMode)
				htmlPlainTextError(mysql_error(&gMysql));
			else
				fprintf(stdout,"AddNewArpaZone() Error: %s\n",mysql_error(&gMysql));
			return(1);//tzonefunc.h
		}
		uZone=mysql_insert_id(&gMysql);
		uNew=1;

	}
	else if((field=mysql_fetch_row(res2)))
	{
		sscanf(field[1],"%u",&uZone);
	}

	mysql_free_result(res2);

	if(uZone)
	{
		unsigned uErrNum=0;

		if(!uHtmlMode)
			fprintf(stdout,"Adding RR to ip6.arpa zone: %s,uZone=%u,uZoneOwner=%u\n",
					cIPv6PTR,uZone,uZoneOwner);

		//Add PTR record 
		if((uErrNum=AutoAddPTRResourceIPv6(cIPv6PTR,cHostname,uZone,uZoneOwner)))
		{
			if(uHtmlMode)
				tZone(mysql_error(&gMysql));
			else
				if(!uHtmlMode)
					fprintf(stdout,"AutoAddPTRResourceIPv6() Error(%u): %s\n",
						uErrNum,mysql_error(&gMysql));
				else
					htmlPlainTextError(mysql_error(&gMysql));
			return(1);
		}

		if(!uNew)
			SubmitJob("Modify",uNSSet,cIPv6ArpaZoneName,0,0);
		else
			SubmitJob("New",uNSSet,cIPv6ArpaZoneName,0,0);
		UpdateSerialNum(uZone);
	}

	//TODO what is this for comments please!
	if(uNew && uFromZone)
	{
		SubmitJob("New",uNSSet,cIPv6ArpaZoneName,0,0);
	}
	return(0);

}//int PopulateArpaZoneIPv6()


int PopulateArpaZone(const char *cZone, const char *cIPNum, const unsigned uHtmlMode, 
					const unsigned uFromZone, const unsigned uZoneOwner, const unsigned uNSSet)
{
	unsigned a=0,b=0,c=0,d=0;
	MYSQL_RES *res2;
	MYSQL_ROW field;
	unsigned uZone=0;
	unsigned uNew=0;
	char cArpaZone[64];
	char cHostMaster[256]="hostmaster.isp.net";
	char cuView[256]="2";

	GetConfiguration("cHostMaster",cHostMaster,0);
	GetConfiguration("cuView",cuView,0);

	sscanf(cIPNum,"%u.%u.%u.%u",&a,&b,&c,&d);
			
	if(!uHtmlMode)
		fprintf(stdout,"<u>%s %u.%u.%u.%u</u>\n",cZone,a,b,c,d);

	if(a==0 || d==0 || a>255 || b>255 || c>255 || d>255) return(1);
	sprintf(cArpaZone,"%u.%u.%u.in-addr.arpa",c,b,a);
	sprintf(gcQuery,"SELECT cZone,uZone FROM tZone WHERE cZone='%s' AND uView=%.2s",
			cArpaZone,cuView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		if(uHtmlMode)
			htmlPlainTextError(mysql_error(&gMysql));
		else
			fprintf(stdout,"PopulateArpaZone() Select Error: %s\n",mysql_error(&gMysql));
		return(1);
	}
	res2=mysql_store_result(&gMysql);
	if(!mysql_num_rows(res2)) 
	{
		if(!uHtmlMode)
			fprintf(stdout,"<font color=blue>Adding new arpa zone: %s</font>\n",cArpaZone);
		if(AddNewArpaZone(cArpaZone,uNSSet,cHostMaster,uZoneOwner)) 
		{
			if(uHtmlMode)
				htmlPlainTextError(mysql_error(&gMysql));
			else
				fprintf(stdout,"AddNewArpaZone() Error: %s\n",mysql_error(&gMysql));
			return(1);//tzonefunc.h
		}
		uZone=mysql_insert_id(&gMysql);
		uNew=1;

	}
	else if((field=mysql_fetch_row(res2)))
	{
		sscanf(field[1],"%u",&uZone);
	}

	mysql_free_result(res2);

	if(uZone)
	{
		unsigned uErrNum=0;

		if(!uHtmlMode)
			fprintf(stdout,"Adding RR to arpa zone: d=%u,uZone=%u,uZoneOwner=%u\n",
				d,uZone,uZoneOwner);

		//Add PTR record 
		if((uErrNum=AutoAddPTRResource(d,cZone,uZone,uZoneOwner)))
		{
			if(uHtmlMode)
				tZone(mysql_error(&gMysql));
			else
				if(!uHtmlMode)
					fprintf(stdout,"AutoAddPTRResource() Error(%u): %s\n",
						uErrNum,mysql_error(&gMysql));
				else
					htmlPlainTextError(mysql_error(&gMysql));
			return(1);
		}

		if(!uNew)
			SubmitJob("Modify",uNSSet,cArpaZone,0,0);
		else
			SubmitJob("New",uNSSet,cArpaZone,0,0);
		UpdateSerialNum(uZone);
	}

	//TODO what is this for comments please!
	if(uNew && uFromZone)
	{
		SubmitJob("New",uNSSet,cArpaZone,0,0);
	}
	return(0);

}//int PopulateArpaZone()


void PassDirectHtml(char *file)
{
	FILE *fp;
	char buffer[1024];

	if((fp=fopen(file,"r"))!=NULL)
	{
		while(fgets(buffer,1024,fp)!=NULL)
			fprintf(stdout,"%s",buffer);

		fclose(fp);
	}

}//void PassDirectHtml(char *file)


void GetConfiguration(const char *cName, char *cValue, unsigned uHtml)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

        char cQuery[512];

        sprintf(cQuery,"SELECT cValue,cComment FROM tConfiguration WHERE cLabel='%s'",cName);
        mysql_query(&gMysql,cQuery);
        if(mysql_errno(&gMysql))
	{
		if(uHtml)
		{
        	        tConfiguration(mysql_error(&gMysql));
		}
		else
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
	}
        res=mysql_store_result(&gMysql);
        if((field=mysql_fetch_row(res)))
	{
		if(strcmp(field[0],"Value in cComment"))
                	sprintf(cValue,"%.255s",field[0]);
		else
                	sprintf(cValue,"%.1023s",field[1]);
	}
        mysql_free_result(res);

}//void GetConfiguration(const char *cName, char *cValue)


//cValue must be a buffer for min 512 chars
void GetGlossary(const char *cName, char *cValue)
{
        MYSQL_RES *res;
        MYSQL_ROW field;

        char cQuery[512];

        sprintf(cQuery,"SELECT cText FROM tGlossary WHERE cLabel='%s'",cName);
        mysql_query(&gMysql,cQuery);
        if(mysql_errno(&gMysql))
		tConfiguration(mysql_error(&gMysql));
        res=mysql_store_result(&gMysql);
        if((field=mysql_fetch_row(res)))
		sprintf(cValue,"%.511s",field[0]);
        mysql_free_result(res);

}//void GetGlossary(const char *cName, char *cValue)


unsigned ViewReloadZone(char *cZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uRetVal=1;
	char cuControlPort[8]={""};
	char cCmd[512]={""};

	GetConfiguration("cuControlPort",cuControlPort,0);
	if((uNamedCheckConf("ViewReloadZone() call")))
	{
		logfileLine("ViewReloadZone","uNamedCheckConf error");
		return(1);
	}
	//Multiple view rndc reload
	//Do we need to reload all views, when we might have only modified the internal view for example?
	sprintf(gcQuery,"SELECT tView.cLabel,tZone.uSecondaryOnly FROM tZone,tView WHERE"
				" tZone.uView=tView.uView AND tZone.cZone='%s'",cZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stdout,"%s",mysql_error(&gMysql));
		exit(2);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(field[1][0]=='1')
		{
			if(cuControlPort[0])
				sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf -p %s retransfer %s in %s > /tmp/unxsbindlog 2>&1",
						gcBinDir,cuControlPort,cZone,field[0]);
			else
				sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf retransfer %s in %s > /tmp/unxsbindlog 2>&1",
						gcBinDir,cZone,field[0]);
		}
		else
		{
			if(cuControlPort[0])
				sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf -p %s reload %s in %s > /tmp/unxsbindlog 2>&1",
						gcBinDir,cuControlPort,cZone,field[0]);
			else
				sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf reload %s in %s > /tmp/unxsbindlog 2>&1",
						gcBinDir,cZone,field[0]);
		}
		logfileLine("ViewReloadZone",cCmd);
		uRetVal=system(cCmd);
		if(uRetVal)
		{
			logfileLine("ViewReloadZone","uRetVal error trying straight reload");
			sprintf(cCmd,"%s/rndc -c /etc/unxsbind-rndc.conf reload >> /tmp/unxsbindlog 2>&1",gcBinDir);
			logfileLine("ViewReloadZone",cCmd);
			uRetVal=system(cCmd);
			if(uRetVal)
				logfileLine("ViewReloadZone","uRetVal error giving up!");
		}
	}
	mysql_free_result(res);

	logfileLine("ViewReloadZone","Exit");
	return(uRetVal);

}//unsigned ViewReloadZone(char *cZone)


unsigned uGetZoneOwner(unsigned uZone)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	
	unsigned uOwner=1; //Default: Root

	sprintf(gcQuery,"SELECT uOwner FROM tZone WHERE uZone=%u",uZone);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	res=mysql_store_result(&gMysql);

	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uOwner);
	
	return(uOwner);

}//unsigned uGetZoneOwner(unsigned uZone)


unsigned uNamedCheckConf(char *cNameServer)
{
	//This function runs a named-checkconf for the iDNS controlled named daemon
	//Returns 0 if OK, 1 if failure. Also it will create a tLog entry in case of error for informing
	//the admin users via the iDNS backennd dashboard.
	//
	//The idea is to not allow incomplete validation or unexpected configuration problems
	//to break a running production NS.
	unsigned uRet=0;
	int iRetVal;

	//We will check named configuration file to see if there are any issues there
	sprintf(gcQuery,"%s/named-checkconf /usr/local/idns/named.conf",gcBinDir);
	if((iRetVal=system(gcQuery)>0))
	{
		fprintf(stdout,"%s returned %d\n",gcQuery,iRetVal);	

		//Insert tLog record and skip server reload or reconfig
		sprintf(gcQuery,"INSERT INTO tLog SET uLogType=4,uPermLevel=12,uLoginClient=1,"
				"cLogin='MasterJobQueue()',cHost ='127.0.0.1',uTablePK=0,cTableName='(None)',"
				"cMessage='Server named.conf with errors',cServer='%s',uOwner=1,uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())" ,cNameServer);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
		uRet=1;
	}

	return(uRet);

}//unsigned uNamedCheckConf(void)


void ServerJobQueue(char *cServer)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	
	if(TextConnectDb())
		exit(0);

	//debug only
	fprintf(stdout,"ServerJobQueue(%s)\n",cServer);

	sprintf(gcQuery,"SELECT tZone.cZone,tNS.cFQDN,tNSType.cLabel FROM tZone,tNSSet,tNS,tNSType,tServer WHERE"
			" tZone.uNSSet=tNSSet.uNSSet AND tNSSet.uNSSet=tNS.uNSSet AND tNS.uServer=tServer.uServer AND"
			" tNS.uNSType=tNSType.uNSType AND tServer.cLabel='%s'",cServer);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stdout,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		fprintf(stdout,"%s %s %s\n",
			field[0],field[1],field[2]);
	}
	mysql_free_result(res);

	exit(0);

}//void ServerJobQueue(char *cNameServer)


