/*
FILE
	svn ID removed
AUTHOR/LEGAL
	(C) 2001-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file included.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/file.h>
#include <ctype.h>

#include "cgi.h"

#include <mysql/mysql.h>
#ifdef LIBOATH
	#include <liboath/oath.h>
#endif
char *crypt(const char *key, const char *salt);

//This creates a dependency on provided tRRType.txt table fie
#define RRTYPE_A 1
#define RRTYPE_NS 2
#define RRTYPE_MX 3 
#define RRTYPE_HINFO 4
#define RRTYPE_CNAME 5
#define RRTYPE_TXT 6
#define RRTYPE_PTR 7
#define RRTYPE_SRV 8
#define RRTYPE_AAAA 9
#define RRTYPE_NAPTR 10
#define RRTYPE_SPF 11

//Job queue
//mysqlISP constants
//tJob.uJobStatus
#define mysqlISP_RemotelyQueued 7
#define mysqlISP_Waiting 10
//tInstance.uStatus:
#define mysqlISP_Deployed 4
#define mysqlISP_Canceled 5
#define mysqlISP_OnHold 6
//
//unxsVZ universal job status conversion in progress
//tJobStatus contants
#define unxsVZ_uWAITING 	1
#define unxsVZ_uRUNNING 	2
#define unxsVZ_uDONEOK		3
#define unxsVZ_uDONEERROR	4
#define unxsVZ_uSUSPENDED	5
#define unxsVZ_uREDUNDANT	6
#define unxsVZ_uCANCELED	7
#define unxsVZ_uREMOTEWAITING	10
#define unxsVZ_uERROR		14



#define TEXT_CONNECT_ISP 0
#define TEXT_CONNECT_APACHE 1
#define TEXT_CONNECT_UNXSVZ 2

#include <unistd.h>
#include <locale.h>
#include <monetary.h>

#include "language.h"
#include "local.h"

extern char gcHost[];
extern char gcHostname[];
extern char gcUser[];
#define PERMLEVEL
extern int guPermLevel; 
extern unsigned guLoginClient; 
extern unsigned guReseller; 
extern unsigned guCompany;

extern char gcFunction[];
extern unsigned guListMode; 
extern char gcQuery[];
extern char *gcQstr;
extern char *gcBuildInfo;
extern char *gcRADStatus;
extern MYSQL gMysql; 
extern MYSQL gMysql2; 
extern unsigned long gluRowid;
extern unsigned guStart;
extern unsigned guEnd;
extern unsigned guI;
extern unsigned guN;
extern char gcCommand[];
extern char gcFilter[];
extern char gcFind[];
extern char gcTable[];
extern unsigned guMode;
extern int guError;
extern char gcErrormsg[];

extern pentry entries[];
extern int x;


void iDNS(const char *cResult);
void ConnectDb(void);
unsigned TextConnectDb(void);
unsigned TextConnectExtDb(MYSQL *Mysql, unsigned uMode);
void Footer_ism3(void);
void Header_ism3(char *cMsg, int iJs);
void ProcessControlVars(pentry entries[], int x);
void OpenRow(const char *cFieldLabel, const char *cColor);
void OpenFieldSet(char *cLabel, unsigned uWidth);
void CloseFieldSet(void);
void LoadConf(void);
void NoSuchFunction(void);
void tTablePullDown(const char *cTableName, const char *cFieldName,
                        const char *cOrderby, unsigned uSelector, unsigned uMode);
void tTablePullDownOwner(const char *cTableName, const char *cFieldName,
                        const char *cOrderby, unsigned uSelector, unsigned uMode);
void tTablePullDownReadOnly(const char *cTableName, const char *cFieldName,
                        const char *cOrderby, unsigned uSelector);
int ReadPullDown(const char *cTableName,const char *cFieldName,const char *cLabel);
char *TextAreaSave(char *cField);
char *TransformAngleBrackets(char *cField);
char *EncodeDoubleQuotes(char *cField);
void YesNoPullDown(char *cFieldName,unsigned uSelect,unsigned uMode);
void YesNo(unsigned uSelect);
int ReadYesNoPullDown(const char *cLabel);
const char *ForeignKey(const char *cTableName, const char *cFieldName, unsigned uKey);
void GetClientOwner(unsigned uClient, unsigned *uOwner);
void ExtMainShell(int argc, char *argv[]);
void jsCalendarInput(char *cInputName,char *cValue,unsigned uMode);
long luGetModDate(char *cTableName, unsigned uTablePK);
long luGetCreatedDate(char *cTableName, unsigned uTablePK);
void DashBoard(const char *cOptionalMsg);
void htmlPlainTextError(const char *cError);
int SubmitJob(const char *cCommand, unsigned uNSSetArg, const char *cZoneArg,
		                                unsigned uPriorityArg, time_t luTimeArg);
char *cURLEncode(char *cURL);
void iDNSLog(unsigned uTablePK, char *cTableName, char *cLogEntry);
void GetConfiguration(const char *cName, char *cValue,unsigned uHtml);
unsigned uAllowMod(const unsigned uOwner, const unsigned uCreatedBy);
unsigned uAllowDel(const unsigned uOwner, const unsigned uCreatedBy);
void ExtListSelect(const char *cTable,const char *cVarList);
void ExtSelect(const char *cTable,const char *cVarList,unsigned uMaxResults);
void ExtSelectSearch(	const char *cTable,
			const char *cVarList,
			const char *cSearchField,
			const char *cSearch,
			const char *cExtraCond,
			unsigned uMaxResults);
void ExtSelectRow(const char *cTable,const char *cVarList,unsigned uRow);
void ExtListSelectPublic(const char *cTable,const char *cVarList);
void ExtSelectPublic(const char *cTable,const char *cVarList);
void ExtSelectRowPublic(const char *cTable,const char *cVarList,unsigned uRow);

void tTablePullDownResellers(unsigned uSelector,unsigned uMode);

int PopulateArpaZone(const char *cZone, const char *cIPNum, const unsigned uHtmlMode, 
					const unsigned uFromZone, const unsigned uZoneOwner, const unsigned uNSSet);
int PopulateArpaZoneIPv6(const char *cIPv6ArpaZoneName, const char *cIPv6PTR, const char *cHostname, const unsigned uHtmlMode, 
					const unsigned uFromZone, const unsigned uZoneOwner, const unsigned uNSSet);
int AddNewArpaZone(const char *cArpaZone, unsigned uExtNSSet, char *cExtHostmaster, unsigned uExtOwner);
int AutoAddPTRResource(const unsigned d,const char *cDomain,const unsigned uInZone,const unsigned uSourceZoneOwner);//tresourcefunc.h
int AutoAddPTRResourceIPv6(const char *cIPv6PTR,const char *cDomain,const unsigned uInZone,const unsigned uSourceZoneOwner);//tresourcefunc.h

 //Standard tInputFunc functions
char *WordToLower(char *cInput);
char *IPNumber(char *cInput);
char *IPv4All(char *cInput);
char *IPv4Range(char *cInput);
char *IPv4CIDR(char *cInput);
char *FQDomainName(char *cInput);
char *EmailInput(char *cInput);
char *cMoneyInput(char *cInput);
char *cMoneyDisplay(char *cInput);

 //Standard tValidFunc functions
const char *EmptyString(const char *cInput);
const char *BadIPNum(const char *cInput);
const char *IsZero(const unsigned uInput);

 //External pagination form processing vars
void PageMachine(char *cFuncName, int iLmode, char *cMsg);

 //Place ModuleCommands() and Module() prototypes here
#define ISPNAME "OpenISP"
#define ISPURL "www.openisp.net"
#define ADMIN 9

//tZone
int tZoneCommands(pentry entries[], int x);
void tZone(const char *results);
void ProcesstZoneVars(pentry entries[], int x);
void tZoneContent(void);
void tZoneInputContent(void);
void tZoneInput(unsigned uMode);
void tZoneList(void);
void NewtZone(unsigned uMode);
void ModtZone(void);
void CreatetZone(void);
void DeletetZone(void);
void ExttZoneGetHook(entry gentries[], int x);
void ExttZoneNavBar(void);
unsigned OnLineZoneCheck(unsigned uZone,unsigned uCalledMode,unsigned uCalledFrom);
void CreatetResourceTest(void);

//tResource
int tResourceCommands(pentry entries[], int x);
void tResource(const char *results);
void ProcesstResourceVars(pentry entries[], int x);
void tResourceContent(void);
void tResourceInputContent(void);
void tResourceInput(unsigned uMode);
void tResourceList(void);
void NewtResource(unsigned uMode);
void ModtResource(void);
void CreatetResource(void);
void DeletetResource(void);
void ExttResourceGetHook(entry gentries[], int x);
void ExttResourceNavBar(void);
unsigned uGetSearchGroup(const char *gcUser);


//tRRType
int tRRTypeCommands(pentry entries[], int x);
void tRRType(const char *results);
void ProcesstRRTypeVars(pentry entries[], int x);
void tRRTypeContent(void);
void tRRTypeInputContent(void);
void tRRTypeInput(unsigned uMode);
void tRRTypeList(void);
void NewtRRType(unsigned uMode);
void ModtRRType(void);
void CreatetRRType(void);
void DeletetRRType(void);
void ExttRRTypeGetHook(entry gentries[], int x);
void ExttRRTypeNavBar(void);

//tJob
int tJobCommands(pentry entries[], int x);
void tJob(const char *results);
void ProcesstJobVars(pentry entries[], int x);
void tJobContent(void);
void tJobInputContent(void);
void tJobInput(unsigned uMode);
void tJobList(void);
void NewtJob(unsigned uMode);
void ModtJob(void);
void CreatetJob(void);
void DeletetJob(void);
void ExttJobGetHook(entry gentries[], int x);
void ExttJobNavBar(void);

//tMailServer
int tMailServerCommands(pentry entries[], int x);
void tMailServer(const char *results);
void ProcesstMailServerVars(pentry entries[], int x);
void tMailServerContent(void);
void tMailServerInputContent(void);
void tMailServerInput(unsigned uMode);
void tMailServerList(void);
void NewtMailServer(unsigned uMode);
void ModtMailServer(void);
void CreatetMailServer(void);
void DeletetMailServer(void);
void ExttMailServerGetHook(entry gentries[], int x);
void ExttMailServerNavBar(void);

//tNSType
int tNSTypeCommands(pentry entries[], int x);
void tNSType(const char *results);
void ProcesstNSTypeVars(pentry entries[], int x);
void tNSTypeContent(void);
void tNSTypeInputContent(void);
void tNSTypeInput(unsigned uMode);
void tNSTypeList(void);
void NewtNSType(unsigned uMode);
void ModtNSType(void);
void CreatetNSType(void);
void DeletetNSType(void);
void ExttNSTypeGetHook(entry gentries[], int x);
void ExttNSTypeNavBar(void);

//tNSSet
int tNSSetCommands(pentry entries[], int x);
void tNSSet(const char *results);
void ProcesstNSSetVars(pentry entries[], int x);
void tNSSetContent(void);
void tNSSetInputContent(void);
void tNSSetInput(unsigned uMode);
void tNSSetList(void);
void NewtNSSet(unsigned uMode);
void ModtNSSet(void);
void CreatetNSSet(void);
void DeletetNSSet(void);
void ExttNSSetGetHook(entry gentries[], int x);
void ExttNSSetNavBar(void);

//tNS
int tNSCommands(pentry entries[], int x);
void tNS(const char *results);
void ProcesstNSVars(pentry entries[], int x);
void tNSContent(void);
void tNSInputContent(void);
void tNSInput(unsigned uMode);
void tNSList(void);
void NewtNS(unsigned uMode);
void ModtNS(void);
void CreatetNS(void);
void DeletetNS(void);
void ExttNSGetHook(entry gentries[], int x);
void ExttNSNavBar(void);

//tServer
int tServerCommands(pentry entries[], int x);
void tServer(const char *results);
void ProcesstServerVars(pentry entries[], int x);
void tServerContent(void);
void tServerInputContent(void);
void tServerInput(unsigned uMode);
void tServerList(void);
void NewtServer(unsigned uMode);
void ModtServer(void);
void CreatetServer(void);
void DeletetServer(void);
void ExttServerGetHook(entry gentries[], int x);
void ExttServerNavBar(void);

//tConfiguration
int tConfigurationCommands(pentry entries[], int x);
void tConfiguration(const char *results);
void ProcesstConfigurationVars(pentry entries[], int x);
void tConfigurationContent(void);
void tConfigurationInputContent(void);
void tConfigurationInput(unsigned uMode);
void tConfigurationList(void);
void NewtConfiguration(unsigned uMode);
void ModtConfiguration(void);
void CreatetConfiguration(void);
void DeletetConfiguration(void);
void ExttConfigurationGetHook(entry gentries[], int x);
void ExttConfigurationNavBar(void);

//tTemplate
int tTemplateCommands(pentry entries[], int x);
void tTemplate(const char *results);
void ProcesstTemplateVars(pentry entries[], int x);
void tTemplateContent(void);
void tTemplateInputContent(void);
void tTemplateInput(unsigned uMode);
void tTemplateList(void);
void NewtTemplate(unsigned uMode);
void ModtTemplate(void);
void CreatetTemplate(void);
void DeletetTemplate(void);
void ExttTemplateGetHook(entry gentries[], int x);
void ExttTemplateNavBar(void);

//tTemplateSet
int tTemplateSetCommands(pentry entries[], int x);
void tTemplateSet(const char *results);
void ProcesstTemplateSetVars(pentry entries[], int x);
void tTemplateSetContent(void);
void tTemplateSetInputContent(void);
void tTemplateSetInput(unsigned uMode);
void tTemplateSetList(void);
void NewtTemplateSet(unsigned uMode);
void ModtTemplateSet(void);
void CreatetTemplateSet(void);
void DeletetTemplateSet(void);
void ExttTemplateSetGetHook(entry gentries[], int x);
void ExttTemplateSetNavBar(void);

//tTemplateType
int tTemplateTypeCommands(pentry entries[], int x);
void tTemplateType(const char *results);
void ProcesstTemplateTypeVars(pentry entries[], int x);
void tTemplateTypeContent(void);
void tTemplateTypeInputContent(void);
void tTemplateTypeInput(unsigned uMode);
void tTemplateTypeList(void);
void NewtTemplateType(unsigned uMode);
void ModtTemplateType(void);
void CreatetTemplateType(void);
void DeletetTemplateType(void);
void ExttTemplateTypeGetHook(entry gentries[], int x);
void ExttTemplateTypeNavBar(void);

//tLog
int tLogCommands(pentry entries[], int x);
void tLog(const char *results);
void ProcesstLogVars(pentry entries[], int x);
void tLogContent(void);
void tLogInputContent(void);
void tLogInput(unsigned uMode);
void tLogList(void);
void NewtLog(unsigned uMode);
void ModtLog(void);
void CreatetLog(void);
void DeletetLog(void);
void ExttLogGetHook(entry gentries[], int x);
void ExttLogNavBar(void);

//tLogType
int tLogTypeCommands(pentry entries[], int x);
void tLogType(const char *results);
void ProcesstLogTypeVars(pentry entries[], int x);
void tLogTypeContent(void);
void tLogTypeInputContent(void);
void tLogTypeInput(unsigned uMode);
void tLogTypeList(void);
void NewtLogType(unsigned uMode);
void ModtLogType(void);
void CreatetLogType(void);
void DeletetLogType(void);
void ExttLogTypeGetHook(entry gentries[], int x);
void ExttLogTypeNavBar(void);

//tBlock
int tBlockCommands(pentry entries[], int x);
void tBlock(const char *results);
void ProcesstBlockVars(pentry entries[], int x);
void tBlockContent(void);
void tBlockInputContent(void);
void tBlockInput(unsigned uMode);
void tBlockList(void);
void NewtBlock(unsigned uMode);
void ModtBlock(void);
void CreatetBlock(void);
void DeletetBlock(void);
void ExttBlockGetHook(entry gentries[], int x);
void ExttBlockNavBar(void);

//tView
int tViewCommands(pentry entries[], int x);
void tView(const char *results);
void ProcesstViewVars(pentry entries[], int x);
void tViewContent(void);
void tViewInputContent(void);
void tViewInput(unsigned uMode);
void tViewList(void);
void NewtView(unsigned uMode);
void ModtView(void);
void CreatetView(void);
void DeletetView(void);
void ExttViewGetHook(entry gentries[], int x);
void ExttViewNavBar(void);

//tRegistrar
int tRegistrarCommands(pentry entries[], int x);
void tRegistrar(const char *results);
void ProcesstRegistrarVars(pentry entries[], int x);
void tRegistrarContent(void);
void tRegistrarInputContent(void);
void tRegistrarInput(unsigned uMode);
void tRegistrarList(void);
void NewtRegistrar(unsigned uMode);
void ModtRegistrar(void);
void CreatetRegistrar(void);
void DeletetRegistrar(void);
void ExttRegistrarGetHook(entry gentries[], int x);
void ExttRegistrarNavBar(void);

//tGlossary
int tGlossaryCommands(pentry entries[], int x);
void tGlossary(const char *results);
void ProcesstGlossaryVars(pentry entries[], int x);
void tGlossaryContent(void);
void tGlossaryInputContent(void);
void tGlossaryInput(unsigned uMode);
void tGlossaryList(void);
void NewtGlossary(unsigned uMode);
void ModtGlossary(void);
void CreatetGlossary(void);
void DeletetGlossary(void);
void ExttGlossaryGetHook(entry gentries[], int x);
void ExttGlossaryNavBar(void);

//tGroupType
int tGroupTypeCommands(pentry entries[], int x);
void tGroupType(const char *results);
void ProcesstGroupTypeVars(pentry entries[], int x);
void tGroupTypeContent(void);
void tGroupTypeInputContent(void);
void tGroupTypeInput(unsigned uMode);
void tGroupTypeList(void);
void NewtGroupType(unsigned uMode);
void ModtGroupType(void);
void CreatetGroupType(void);
void DeletetGroupType(void);
void ExttGroupTypeGetHook(entry gentries[], int x);
void ExttGroupTypeNavBar(void);

//tGroup
int tGroupCommands(pentry entries[], int x);
void tGroup(const char *results);
void ProcesstGroupVars(pentry entries[], int x);
void tGroupContent(void);
void tGroupInputContent(void);
void tGroupInput(unsigned uMode);
void tGroupList(void);
void NewtGroup(unsigned uMode);
void ModtGroup(void);
void CreatetGroup(void);
void DeletetGroup(void);
void ExttGroupGetHook(entry gentries[], int x);
void ExttGroupNavBar(void);

//tGroupGlue
int tGroupGlueCommands(pentry entries[], int x);
void tGroupGlue(const char *results);
void ProcesstGroupGlueVars(pentry entries[], int x);
void tGroupGlueContent(void);
void tGroupGlueInputContent(void);
void tGroupGlueInput(unsigned uMode);
void tGroupGlueList(void);
void NewtGroupGlue(unsigned uMode);
void ModtGroupGlue(void);
void CreatetGroupGlue(void);
void DeletetGroupGlue(void);
void ExttGroupGlueGetHook(entry gentries[], int x);
void ExttGroupGlueNavBar(void);
void tTableMultiplePullDown(const char *cTableName,const char *cFieldName,const char *cOrderby);


//tZoneImport
int tZoneImportCommands(pentry entries[], int x);
void tZoneImport(const char *results);
void ProcesstZoneImportVars(pentry entries[], int x);
void tZoneImportContent(void);
void tZoneImportInputContent(void);
void tZoneImportInput(unsigned uMode);
void tZoneImportList(void);
void NewtZoneImport(unsigned uMode);
void ModtZoneImport(void);
void CreatetZoneImport(void);
void DeletetZoneImport(void);
void ExttZoneImportGetHook(entry gentries[], int x);
void ExttZoneImportNavBar(void);

//tResourceImport
int tResourceImportCommands(pentry entries[], int x);
void tResourceImport(const char *results);
void ProcesstResourceImportVars(pentry entries[], int x);
void tResourceImportContent(void);
void tResourceImportInputContent(void);
void tResourceImportInput(unsigned uMode);
void tResourceImportList(void);
void NewtResourceImport(unsigned uMode);
void ModtResourceImport(void);
void CreatetResourceImport(void);
void DeletetResourceImport(void);
void ExttResourceImportGetHook(entry gentries[], int x);
void ExttResourceImportNavBar(void);

//tMonthHit
int tMonthHitCommands(pentry entries[], int x);
void tMonthHit(const char *results);
void ProcesstMonthHitVars(pentry entries[], int x);
void tMonthHitContent(void);
void tMonthHitInputContent(void);
void tMonthHitInput(unsigned uMode);
void tMonthHitList(void);
void NewtMonthHit(unsigned uMode);
void ModtMonthHit(void);
void CreatetMonthHit(void);
void DeletetMonthHit(void);
void ExttMonthHitGetHook(entry gentries[], int x);
void ExttMonthHitNavBar(void);

//tMonth
int tMonthCommands(pentry entries[], int x);
void tMonth(const char *results);
void ProcesstMonthVars(pentry entries[], int x);
void tMonthContent(void);
void tMonthInputContent(void);
void tMonthInput(unsigned uMode);
void tMonthList(void);
void NewtMonth(unsigned uMode);
void ModtMonth(void);
void CreatetMonth(void);
void DeletetMonth(void);
void ExttMonthGetHook(entry gentries[], int x);
void ExttMonthNavBar(void);

//tLogMonth
int tLogMonthCommands(pentry entries[], int x);
void tLogMonth(const char *results);
void ProcesstLogMonthVars(pentry entries[], int x);
void tLogMonthContent(void);
void tLogMonthInputContent(void);
void tLogMonthInput(unsigned uMode);
void tLogMonthList(void);
void NewtLogMonth(unsigned uMode);
void ModtLogMonth(void);
void CreatetLogMonth(void);
void DeletetLogMonth(void);
void ExttLogMonthGetHook(entry gentries[], int x);
void ExttLogMonthNavBar(void);

//tHit
int tHitCommands(pentry entries[], int x);
void tHit(const char *results);
void ProcesstHitVars(pentry entries[], int x);
void tHitContent(void);
void tHitInputContent(void);
void tHitInput(unsigned uMode);
void tHitList(void);
void NewtHit(unsigned uMode);
void ModtHit(void);
void CreatetHit(void);
void DeletetHit(void);
void ExttHitGetHook(entry gentries[], int x);
void ExttHitNavBar(void);

//tHitMonth
int tHitMonthCommands(pentry entries[], int x);
void tHitMonth(const char *results);
void ProcesstHitMonthVars(pentry entries[], int x);
void tHitMonthContent(void);
void tHitMonthInputContent(void);
void tHitMonthInput(unsigned uMode);
void tHitMonthList(void);
void NewtHitMonth(unsigned uMode);
void ModtHitMonth(void);
void CreatetHitMonth(void);
void DeletetHitMonth(void);
void ExttHitMonthGetHook(entry gentries[], int x);
void ExttHitMonthNavBar(void);

//tDeletedZone
int tDeletedZoneCommands(pentry entries[], int x);
void tDeletedZone(const char *results);
void ProcesstDeletedZoneVars(pentry entries[], int x);
void tDeletedZoneContent(void);
void tDeletedZoneInputContent(void);
void tDeletedZoneInput(unsigned uMode);
void tDeletedZoneList(void);
void NewtDeletedZone(unsigned uMode);
void ModtDeletedZone(void);
void CreatetDeletedZone(void);
void DeletetDeletedZone(void);
void ExttDeletedZoneGetHook(entry gentries[], int x);
void ExttDeletedZoneNavBar(void);

//tDeletedResource
int tDeletedResourceCommands(pentry entries[], int x);
void tDeletedResource(const char *results);
void ProcesstDeletedResourceVars(pentry entries[], int x);
void tDeletedResourceContent(void);
void tDeletedResourceInputContent(void);
void tDeletedResourceInput(unsigned uMode);
void tDeletedResourceList(void);
void NewtDeletedResource(unsigned uMode);
void ModtDeletedResource(void);
void CreatetDeletedResource(void);
void DeletetDeletedResource(void);
void ExttDeletedResourceGetHook(entry gentries[], int x);
void ExttDeletedResourceNavBar(void);

//tCOSProfile
int tCOSProfileCommands(pentry entries[], int x);
void tCOSProfile(const char *results);
void ProcesstCOSProfileVars(pentry entries[], int x);
void tCOSProfileContent(void);
void tCOSProfileInputContent(void);
void tCOSProfileInput(unsigned uMode);
void tCOSProfileList(void);
void NewtCOSProfile(unsigned uMode);
void ModtCOSProfile(void);
void CreatetCOSProfile(void);
void DeletetCOSProfile(void);
void ExttCOSProfileGetHook(entry gentries[], int x);
void ExttCOSProfileNavBar(void);

//tRegistration
int tRegistrationCommands(pentry entries[], int x);
void tRegistration(const char *results);
void ProcesstRegistrationVars(pentry entries[], int x);
void tRegistrationContent(void);
void tRegistrationInputContent(void);
void tRegistrationInput(unsigned uMode);
void tRegistrationList(void);
void NewtRegistration(unsigned uMode);
void ModtRegistration(void);
void CreatetRegistration(void);
void DeletetRegistration(void);
void ExttRegistrationGetHook(entry gentries[], int x);
void ExttRegistrationNavBar(void);

//tBlacklist
int tBlacklistCommands(pentry entries[], int x);
void tBlacklist(const char *results);
void ProcesstBlacklistVars(pentry entries[], int x);
void tBlacklistContent(void);
void tBlacklistInputContent(void);
void tBlacklistInput(unsigned uMode);
void tBlacklistList(void);
void NewtBlacklist(unsigned uMode);
void ModtBlacklist(void);
void CreatetBlacklist(void);
void DeletetBlacklist(void);
void ExttBlacklistGetHook(entry gentries[], int x);
void ExttBlacklistNavBar(void);

//tClient
int tClientCommands(pentry entries[], int x);
void tClient(const char *results);
void ProcesstClientVars(pentry entries[], int x);
void tClientContent(void);
void tClientInputContent(void);
void tClientInput(unsigned uMode);
void tClientList(void);
void NewtClient(unsigned uMode);
void ModtClient(void);
void CreatetClient(void);
void DeletetClient(void);
void ExttClientGetHook(entry gentries[], int x);
void ExttClientNavBar(void);

//tAuthorize
int tAuthorizeCommands(pentry entries[], int x);
void tAuthorize(const char *results);
void ProcesstAuthorizeVars(pentry entries[], int x);
void tAuthorizeContent(void);
void tAuthorizeInputContent(void);
void tAuthorizeInput(unsigned uMode);
void tAuthorizeList(void);
void NewtAuthorize(unsigned uMode);
void ModtAuthorize(void);
void CreatetAuthorize(void);
void DeletetAuthorize(void);
void ExttAuthorizeGetHook(entry gentries[], int x);
void ExttAuthorizeNavBar(void);

//In-line code macros
//Common
#define _RUN_QUERY mysql_query(&gMysql,gcQuery);if(mysql_errno(&gMysql))

//MySQL run query only w/error checking
//HTML
#define MYSQL_RUN _RUN_QUERY htmlPlainTextError(mysql_error(&gMysql))
#define MYSQL_RUN_STORE(res) MYSQL_RUN;res=mysql_store_result(&gMysql)

//Common - This macro shouldn't be used directly, as is part of the others only
#define macro_mySQLQueryBasic mysql_query(&gMysql,gcQuery);\
				if(mysql_errno(&gMysql))

//MySQL run query only w/error checking
//HTML
#define macro_mySQLQueryHTMLError macro_mySQLQueryBasic \
					htmlPlainTextError(mysql_error(&gMysql))
//Text
#define macro_mySQLQueryTextError macro_mySQLQueryBasic\
					{\
						fprintf(stderr,"%s\n",mysql_error(&gMysql));\
						exit(1);\
					}

//Text with return() instead of exit()
//return(1); if MySQL error
#define macro_mySQLRunReturnInt macro_mySQLQueryBasic\
				{\
					fprintf(stderr,"%s\n",mysql_error(&gMysql));\
					return(1);\
				}
//return void; if MySQL error
#define macro_mySQLRunReturnVoid macro_mySQLQueryBasic\
				{\
					fprintf(stderr,"%s\n",mysql_error(&gMysql));\
					return;\
				}

//MySQL run query and store result w/error checking
//HTML
#define macro_mySQLRunAndStore(res) macro_mySQLQueryHTMLError;\
					res=mysql_store_result(&gMysql)
//Text
#define macro_mySQLRunAndStoreText(res) macro_mySQLQueryTextError;\
					res=mysql_store_result(&gMysql)

//MySQL run query and store result w/error checking (Text); uses return() call instead of exit()
//return(1); if MySQL error
#define macro_mySQLRunAndStoreTextIntRet(res) macro_mySQLRunReturnInt;\
						res=mysql_store_result(&gMysql)
//return; if MySQL error
#define macro_mySQLRunAndStoreTextVoidRet(res) macro_mySQLRunReturnVoid;\
						res=mysql_store_result(&gMysql)
#define cLOGFILE "/var/log/unxsbindlog"

//MySQL run query and store result w/error checking
//HTML
#define MYSQL_RUN_STORE(res) MYSQL_RUN;res=mysql_store_result(&gMysql)
//Text
#define MYSQL_RUN_STORE_TEXT(res) MYSQL_RUN_TEXT;res=mysql_store_result(&gMysql)
//Text with return() instead of exit()
//return(1); if MySQL error
#define MYSQL_RUN_TEXT_RETURN _RUN_QUERY{fprintf(stderr,"%s\n",mysql_error(&gMysql));return(1);}
//return; if MySQL error
#define MYSQL_RUN_TEXT_RET_VOID _RUN_QUERY{fprintf(stderr,"%s\n",mysql_error(&gMysql));return;}

//MySQL run query and store result w/error checking (Text); uses return() call instead of exit()
//return(1); if MySQL error
#define MYSQL_RUN_STORE_TEXT_RETURN(res) MYSQL_RUN_TEXT_RETURN res=mysql_store_result(&gMysql)
//return; if MySQL error
#define MYSQL_RUN_STORE_TEXT_RET_VOID(res) MYSQL_RUN_TEXT_RET_VOID res=mysql_store_result(&gMysql)

