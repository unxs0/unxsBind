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
#include <openisp/template.h>
#ifdef cLDAPURI
	#include <lber.h>
	#include <ldap.h>
#endif

//libtemplate required
#define MAXPOSTVARS 64
#define MAXGETVARS 8

#define BO_CUSTOMER      "Backend Customer"
#define BO_RESELLER      "Backend Reseller"
#define BO_ADMIN         "Backend Admin"
#define BO_ROOT  "Backend Root"
#define BO_CUSTOMER      "Backend Customer"
#define BO_RESELLER      "Backend Reseller"
#define BO_ADMIN         "Backend Admin"
#define BO_ROOT  "Backend Root"
#define ORG_CUSTOMER     "Organization Customer"
#define ORG_WEBMASTER    "Organization Webmaster"
#define ORG_SALES        "Organization Sales Force"
#define ORG_SERVICE      "Organization Customer Service"
#define ORG_ACCT "Organization Bookkeeper"
#define ORG_ADMIN        "Organization Admin"
#define IP_BLOCK_CIDR 1
#define IP_BLOCK_DASH 2

//Depend on correctly preconfigured tTemplateSet and tTemplateType:
#define uPLAINSET 1
#define uVDNSORGTYPE 3



//In main.c
const char *cUserLevel(unsigned uPermLevel);
void htmlFatalError(const char *cErrMsg);
void textFatalError(const char *cErrMsg);
void htmlHeader(char *cTitle, char *cTemplateName);
void htmlFooter(char *cTemplateName);
void htmlErrorPage(char *cTitle, char *cTemplateName);
void htmlForgotPage(char *cTitle, char *cTemplateName);
void htmlHelpPage(char *cTitle, char *cTemplateName);
char *IPNumber(char *cInput);
const char *cUserLevel(unsigned uPermLevel);
char *TextAreaSave(char *cField);
char *FQDomainName(char *cInput);
void iDNSLog(unsigned uTablePK, char *cTableName, char *cLogEntry);
const char *ForeignKey(const char *cTableName, const char *cFieldName, unsigned uKey);
void fpTemplate(FILE *fp,char *cTemplateName,struct t_template *template);

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
extern unsigned guLoginClient;
extern unsigned guOrg;
extern char gcUser[];
extern char gcName[];
extern char gcOrgName[];
extern char gcHost[];
extern unsigned guBrowserFirefox;

//Cgi form commands and major area function
extern char gcFunction[];
extern char gcPage[];
extern char *gcMessage;
extern char gcModStep[];
extern char gcNewStep[];
extern char gcDelStep[];
extern char gcInputStatus[];
extern unsigned guZone;
extern unsigned guView;

//Menu
//

//zone.c
void ProcessZoneVars(pentry entries[], int x);
void ZoneGetHook(entry gentries[],int x);
void ZoneCommands(pentry entries[], int x);
void htmlZone(void);
void htmlZonePage(char *cTitle, char *cTemplateName);
void SelectZone(void);
void UpdateZone(void);
void funcSelectZone(FILE *fp);
void funcSelectBlock(FILE *fp);
void funcSelectSecondary(FILE *fp);
void funcRRs(FILE *fp);
void SerialNum(char *cSerial);
int OrgSubmitJob(char *cCommand,unsigned uNameServerArg,char *cZoneArg,
		                                unsigned uPriorityArg,long unsigned luTimeArg);
unsigned uGetuZone(char *cZone);
unsigned uGetuNameServer(unsigned guZone);
void funcNSSetMembers(FILE *fp);
#ifdef EXPERIMENTAL
void funcZoneStatus(FILE *fp);
#endif

//rr.c
void ProcessResourceVars(pentry entries[], int x);
void ResourceCommands(pentry entries[], int x);
void ResourceGetHook(entry gentries[],int x);
void htmlResource(void);
void htmlResourcePage(char *cTitle, char *cTemplateName);
void funcSelectRRType(FILE *fp,unsigned uUseStatus);
void htmlResourceWizard(unsigned uStep);
void funcMetaParam(FILE *fp);

//glossary.c
void GlossaryGetHook(entry gentries[],int x);
void htmlGlossary(void);
void htmlGlossaryPage(char *cTitle, char *cTemplateName);

//bulk.c
void BulkOpGetHook(entry gentries[],int x);
void ProcessBulkOpVars(pentry entries[], int x);
void htmlBulkOp(void);
void htmlBulkOpPage(char *cTitle, char *cTemplateName);
void BulkOpCommands(pentry entries[], int x);

//ldap.c
int iValidLDAPLogin(const char *cLogin, const char *cPasswd, char *cOrganization);
#ifdef cLDAPURI
	void ldapErrorLog(char *cMessage,LDAP *ld);
#endif
void logfileLine(const char *cFunction,const char *cLogline);
