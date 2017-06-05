/*
FILE
	extjobqueue.h
	svn ID removed
AUTHOR
	(C) Gary Wallis and Hugo Urquiza 2001-2009.
	GPL License applies. See LICENSE file.
PURPOSE
	GUI independent code header file:
	Processing mysqlISP2 or other externally created jobs.
	Externally means not created by mysqlBind2/iDNS or it's interfaces.
NOTES
	Used temporarily via bind.c as include file
*/

//data

MYSQL mysqlext;
typedef struct {

	char cZone[100];
	unsigned ucZone;

	char cMainAddress[16];
	char cTarget[255];
	char cParkedDomains[256];
	char cNameServer[100];
	char cMailServer[100];
	char cMX1[100];
	char cMX2[100];
	char cHostmaster[100];
	unsigned uRevDns;
	unsigned uExpire;
	unsigned uRefresh;
	unsigned uTTL;
	unsigned uRetry;
	unsigned uZoneTTL;
	unsigned uMailServer;
	unsigned uNSSet;
	unsigned uPriority;
	unsigned uWeight;
	unsigned uPort;

	//New universal paramters
	char cIPv4[32];
	unsigned ucIPv4;

	char cName[100];
	unsigned ucName;

	char cRRType[32];
	unsigned ucRRType;

	char cuTTL[16];
	unsigned ucuTTL;

	char cNSSet[32];
	unsigned ucNSSet;

	char cParam1[255];
	unsigned ucParam1;

	char cParam2[255];
	unsigned ucParam2;

	char cParam3[255];
	unsigned ucParam3;

	char cParam4[255];
	unsigned ucParam4;

	//PBX SRV parameters
	unsigned uMainPort;
	unsigned uBackupPort;
	char cMainIPv4[32];
	unsigned ucMainIPv4;
	char cBackupIPv4[32];
	unsigned ucBackupIPv4;

	//shared with Target=
	//char cTarget[255];
	unsigned ucTarget;

	char cView[32];
	unsigned ucView;

	//tClient
	unsigned uISPClient; 
	char cClientName[33];//cLabel

	//Input markers
	unsigned uParamZone;
	unsigned uParamMainAddress;
	unsigned uParamTarget;
	unsigned uParamParkedDomains;
	unsigned uParamNSSet;
	unsigned uParamMailServer;
	unsigned uParamRevDns;
	unsigned uParamMX1;
	unsigned uParamMX2;
	unsigned uParamClientName;

} structExtJobParameters;

int SubmitISPJob(const char *cJobName,const char *cJobData,
		const char *cServer,unsigned uJobDate);
int InformExtJob(const char *cRemoteMsg,const char *cServer,
		unsigned uJob,unsigned uJobStatus);
void InitializeParams(structExtJobParameters *structExtParam);
void ParseExtParams(structExtJobParameters *structExtParam, char *cJobData);
void CreateNewClient(structExtJobParameters *structExtParam);

void ExtConnectDb(unsigned uHtml);
unsigned uGetClientOwner(unsigned uClient);
void ProcessExtJobQueue(char *cServer);
unsigned WebNew(structExtJobParameters *structExtParam,unsigned uJob,
		char *cServer,unsigned uClient,unsigned uOwner);
unsigned WebMod(structExtJobParameters *structExtParam,
		unsigned uZone,unsigned uJob, char *cServer,unsigned uOwner);
unsigned ModZone(structExtJobParameters *structExtParam,
		unsigned uZone,unsigned uExtJob, char *cServer,unsigned uOwner);
unsigned CancelZone(structExtJobParameters *structExtParam,
		unsigned uZone,unsigned uJob, char *cServer,unsigned uOwner);
int SubmitSingleExtJob(const char *cCommand,const char *cZoneArg, unsigned uNSSetArg,
		const char *cTargetServer, unsigned uPriorityArg, unsigned uTimeArg
	       	,unsigned *uMasterJob,unsigned uExtJob,unsigned uOwner);
int SubmitExtJob(const char *cCommand, unsigned uNSSetArg, const char *cZoneArg,
		unsigned uPriorityArg, unsigned uTimeArg, unsigned uExtJob,unsigned uOwner);
unsigned NewSimpleZone(structExtJobParameters *structExtParam,
		unsigned uJob, char *cServer,unsigned uClient,unsigned uOwner);
unsigned NewSimpleWebZone(structExtJobParameters *structExtParam,
		unsigned uJob, char *cServer,unsigned uClient,unsigned uOwner);
void CreateWebZone(char *cDomain, char *cIP, char *cNameServer,
		char *cMailServer,unsigned uClient,unsigned uOwner);
void DropZone(char *cDomain, char *cNameServer);
unsigned GetuZone(char *cLabel, char *cTable);

