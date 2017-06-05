/*
FILE
	interface.h
	svn ID removed
AUTHOR
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice

*/


#include "../../mysqlrad.h"
#include "../../local.h"
#include <ctype.h>
#include <errno.h>
#include <openisp/template.h>

//libtemplate required
#define MAXPOSTVARS 64
#define MAXGETVARS 8
#define MAX_RESULTS 100

//Depend on correctly preconfigured tTemplateSet and tTemplateType:
#define uPLAINSET 1
#define uHOLIDAYSET 2
#define uIDNSORGTYPE 1
#define uIDNSADMINTYPE 2
#define uVDNSORGTYPE 3


//#define MYSQL_HAS_SUBQUERIES

#define VAR_LIST_tClient "tClient.uClient,tClient.cLabel,tClient.cInfo,tClient.cEmail,tClient.cCode,tClient.uOwner,tClient.uCreatedBy,tClient.uCreatedDate,tClient.uModBy,tClient.uModDate"

#define VAR_LIST_tAuthorize "tAuthorize.uAuthorize,tAuthorize.cLabel,tAuthorize.cIpMask,tAuthorize.uPerm,tAuthorize.uCertClient,tAuthorize.cPasswd,tAuthorize.cClrPasswd,tAuthorize.uOwner,tAuthorize.uCreatedBy,tAuthorize.uCreatedDate,tAuthorize.uModBy,tAuthorize.uModDate"


#define VAR_LIST_tResource "tResource.uResource,tResource.uZone,tResource.cName,tResource.uTTL,tResource.uRRType,tResource.cParam1,tResource.cParam2,tResource.cComment,tResource.uOwner,tResource.uCreatedBy,tResource.uCreatedDate,tResource.uModBy,tResource.uModDate"

//In main.c
const char *cUserLevel(unsigned uPermLevel);
void htmlHeader(char *cTitle, char *cTemplateName);
void htmlFooter(char *cTemplateName);
void htmlErrorPage(char *cTitle, char *cTemplateName);
void htmlForgotPage(char *cTitle, char *cTemplateName);
void htmlHelpPage(char *cTitle, char *cTemplateName);
char *IPNumber(char *cInput);
const char *cUserLevel(unsigned uPermLevel);
char *TextAreaSave(char *cField);
char *FQDomainName(char *cInput);
char *FQDomainName2(char *cInput);
void iDNSLog(unsigned uTablePK, char *cTableName, char *cLogEntry);
int ReadPullDown(const char *cTableName,const char *cFieldName,const char *cLabel);
void fpTemplate(FILE *fp,char *cTemplateName,struct t_template *template);	
void htmlPlainTextError(const char *cError);
const char *cForeignKey(const char *cTableName, const char *cFieldName, unsigned uKey);
void funcTopInfo(FILE *fp);	
void ConvertToEnglishDate(char *cDate);
char *cURLEncode(char *cURL);
void SetSessionCookie();
void GetSessionCookie();

//Global vars all declared in main.c
//libtemplate.a required
extern MYSQL gMysql;
//Multipurpose buffer
extern char gcQuery[];
//Login related
extern char gcCookie[];
extern char gcLogin[];
extern char gcPasswd[];
extern unsigned guSSLCookieLogin;
extern int guPermLevel;
extern char gcuPermLevel[];
extern unsigned guLoginUser;
extern unsigned guOrg;
extern unsigned guASPContact;
extern char gcUser[];
extern char gcName[];
extern char gcOrgName[];
extern char gcHost[];
extern unsigned guBrowserFirefox;
//new cookie cleanup
extern unsigned guCookieResource;
extern unsigned guCookieView;
extern char gcView[];
extern unsigned guCookieContact;
extern char gcCookieZone[];
extern char gcCookieCustomer[];

//

//Cgi form commands and major area function
extern char gcFunction[];
extern char gcPage[];
extern char *gcMessage;
extern char gcModStep[];
extern char gcNewStep[];
extern char gcDelStep[];
extern char gcZone[];
extern char cuView[];
extern unsigned uResource;
extern char gcCustomer[];
extern char gcInputStatus[];
extern char gcPermInputStatus[];

//Menu
//

//customer.c
void ProcessCustomerVars(pentry entries[], int x);
void CustomerGetHook(entry gentries[],int x);
void CustomerCommands(pentry entries[], int x);
void htmlCustomer(void);
void htmlCustomerPage(char *cTitle, char *cTemplateName);
void htmlCustomerWizard(unsigned uStep);
void funcCustomerContacts(FILE *fp);
void funcCompanyNavList(FILE *fp,unsigned uSetCookie);

//customeruser.c
void ProcessCustomerContactVars(pentry entries[], int x);
void CustomerContactGetHook(entry gentries[],int x);
void CustomerContactCommands(pentry entries[], int x);
void htmlCustomerContact(void);
void htmlCustomerContactPage(char *cTitle, char *cTemplateName);
void funcTablePullDownResellers(FILE *fp,unsigned uUseStatus);
void funcPermLevelDropDown(FILE *fp,unsigned uUseStatus);
char *cClientLabel(unsigned uClient);
unsigned uGetClient(char *cLabel);
void funcContactLast7DaysActivity(FILE *fp);
extern unsigned guContact;

//adminuser.c
void ProcessAdminUserVars(pentry entries[], int x);
void AdminUserGetHook(entry gentries[],int x);
void AdminUserCommands(pentry entries[], int x);
void htmlAdminUser(void);
void htmlAdminUserPage(char *cTitle, char *cTemplateName);
void GetConfiguration(const char *cName, char *cValue, unsigned uHtml);
void funcAdmLast7DaysAct(FILE *fp,unsigned uArgClient);
void funcAdminUsers(FILE *fp);

//glossary.c
void GlossaryGetHook(entry gentries[],int x);
void htmlGlossary(void);
void htmlGlossaryPage(char *cTitle, char *cTemplateName);

//zone.c
void ProcessZoneVars(pentry entries[], int x);
void ZoneGetHook(entry gentries[],int x);
void ZoneCommands(pentry entries[], int x);
void htmlZone(void);
void htmlZonePage(char *cTitle, char *cTemplateName);
void SelectZone(unsigned uSetCookie);
void UpdateZone(void);
void funcSelectZone(FILE *fp);
void funcSelectBlock(FILE *fp);
void funcSelectSecondary(FILE *fp);
void funcSelectView(FILE *fp);
void funcRRs(FILE *fp);
void funcSelectMailServers(FILE *fp);
void funcSelectRegistrar(FILE *fp);
void SerialNum(char *cSerial);
int AdminSubmitJob(char *cCommand,unsigned uNameServerArg,char *cZoneArg,
		                                unsigned uPriorityArg,long unsigned luTimeArg);
unsigned uGetuZone(char *cZone,char *cuView);
unsigned uGetuNameServer(char *cZone);
void funcSelectSecondaryYesNo(FILE *fp);
void funcOptionalGraph(FILE *fp);
#ifdef EXPERIMENTAL
void funcZoneStatus(FILE *fp);
#endif
void funcZoneNavList(FILE *fp,unsigned uSetCookie);
void funcNSSetMembers(FILE *fp);

//rr.c
void ProcessResourceVars(pentry entries[], int x);
void ResourceCommands(pentry entries[], int x);
void ResourceGetHook(entry gentries[],int x);
void htmlResource(void);
void htmlResourcePage(char *cTitle, char *cTemplateName);
void funcSelectRRType(FILE *fp, unsigned uUseStatus);
void htmlResourceWizard(unsigned uStep);
void funcMetaParam(FILE *fp);

//bulk.c
void BulkOpGetHook(entry gentries[],int x);
void ProcessBulkOpVars(pentry entries[], int x);
void htmlBulkOp(void);
void htmlBulkOpPage(char *cTitle, char *cTemplateName);
void BulkOpCommands(pentry entries[], int x);

//report.c
void ReportGetHook(entry gentries[],int x);
void ProcessReportOpVars(pentry entries[], int x);
void htmlReport(void);
void htmlReportFocus(void);
void htmlReportPage(char *cTitle, char *cTemplateName);
void ReportCommands(pentry entries[], int x);
void funcReportResults(FILE *fp);
void funcReportHitsTop20(FILE *fp);
void funcSelectMonth(FILE *fp);
void funcReportOverallChanges(FILE *fp);
void funcAvailableZones(FILE *fp);
void funcSelectHitMonth(FILE *fp);

//restorezone.c
void RestoreZoneGetHook(entry gentries[],int x);
void ProcessRestoreZoneVars(pentry entries[], int x);
void htmlRestoreZone(void);
void htmlRestoreZonePage(char *cTitle, char *cTemplateName);
void funcDeletedRRs(FILE *fp,unsigned uShowLinks);
void RestoreZoneCommands(pentry entries[], int x);

//restoreresource.c
void RestoreResourceGetHook(entry gentries[],int x);
void ProcessRestoreResourceVars(pentry entries[], int x);
void htmlRestoreResource(char *cuZone);
void htmlRestoreResourcePage(char *cTitle, char *cTemplateName);
void RestoreResourceCommands(pentry entries[], int x);
void funcRRMetaParam(FILE *fp);

//ipauth.c
void htmlIPAuth(void);
void ProcessIPAuthVars(pentry entries[], int x);
void htmlIPAuthPage(char *cTitle, char *cTemplateName);
void IPAuthGetHook(entry gentries[],int x);
void IPAuthCommands(pentry entries[], int x);
void funcIPAuthReport(FILE *fp);
void funcReportActions(FILE *fp);
void funcIgnoredLines(FILE *fp);

//dashboard.c
void htmlDashBoard(void);
void htmlDashBoardPage(char *cTitle, char *cTemplateName);
void funcContactNavList(FILE *fp,unsigned uSetCookie);
void funcZoneList(FILE *fp);
void funcTopTenidnsOrgUsers(FILE *fp);
void funcTopTenidnsAdminUsers(FILE *fp);
void funcTopTenZoneMods(FILE *fp);
void funcTopTenTraffZones(FILE *fp);
void funcRequestGraphBtn(FILE *fp);
void funcUsageGraph(FILE *fp);
void funcRemovedCompanies(FILE *fp);
void funcRemovedBlocks(FILE *fp);
void htmlIPAuthDetail(void);

