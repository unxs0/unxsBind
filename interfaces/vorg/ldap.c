/*
FILE
	ldap.c
	svn ID removed
PURPOSE
	Provide for LDAP based login for mysqlRAD2/3
	based software.
AUTHOR/LEGAL
	(C) 2009-2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See included LICENSE file.
NOTES
	tClient must have the LDAP provided OU.
	LDAP schema must be setup in accordance with:
	cFilter, cSearchDN, caAttrs, cLinePattern, cPrefixPattern and others.
*/

#include "interface.h"

#ifdef cLDAPURI

//local protos
void ldapErrorLog(char *cMessage,LDAP *ld);
void logfileLine(const char *cFunction,const char *cLogline);

//Must provide on call to at least a strlen(tClient.cLabel)-1 char cOrganization buffer.
//Depending on whether an OpenLDAP or an AD LDAP server is used
//cLogin must be passed with domain information in different formats
//Ex1 AD: jonhdoe@unixservice.com
//Ex2 OpenLDAP: CN=johndoe,OU=members,DC=unixservice,DC=com
//Returns 1 on valid bind and non zero length cOrganization was set.
//cOrganizaton can be passed preset for simple cases where we only have one Org
//or for testing broken LDAP schemas.
int iValidLDAPLogin(const char *cLogin, const char *cPasswd, char *cOrganization)
{
	if(cLogin[0]==0 || cPasswd[0]==0)
		return(0);

#ifdef DEBUG_LDAP
	//debug only
	char cLogEntry[256];
#endif

	LDAP *ld;
	LDAPMessage *ldapMsg;
	LDAPMessage *ldapEntry;
	BerElement *berElement;
	int iDesiredVersion=LDAP_VERSION3;
	struct berval structBervalCredentials;
	struct berval **structBervals;
	struct timeval structTimeval;
	int  iRes,i;
	char *cpAttr;
	char *cp;
	char *cp2;

	//Most of these should probably be set via tConfiguration
	//for utmost flexibilty.
	char *cFilterPrefix=cLDAPFILTERPREFIX;
	char *cFilterSuffix=cLDAPFILTERSUFFIX;
	char cFilter[256];
	char *cSearchDN=cLDAPSEARCHDN;
	char *cURI=cLDAPURI;
	char *caAttrs[8]={cLDAPATTR0,NULL};
	char *cLinePattern=cLDAPLINEPATTERN;
	char *cPrefixPattern=cLDAPPREFIXPATTERN;
	char *cLoginPrefix=cLDAPLOGINPREFIX;
	char *cLoginSuffix=cLDAPLOGINSUFFIX;
	char cFQLogin[256];
	char cFQOrg[256];
	char *cOrgPrefix=cLDAPORGPREFIX;
	char *cOrgSuffix=cLDAPORGSUFFIX;

	//Initialize LDAP data structure
	ldap_initialize(&ld,cURI);
	if(ld==NULL)
	{
		ldapErrorLog("ldap_initialize() failed",NULL);
		return(0);
	}

	//set the LDAP version to be 3
	if(ldap_set_option(ld,LDAP_OPT_PROTOCOL_VERSION,&iDesiredVersion)!=LDAP_OPT_SUCCESS)
	{
		ldapErrorLog("ldap_set_option()",ld);
		return(0);
	}

	//Connect/bind to LDAP server
	structBervalCredentials.bv_val=(char *)cPasswd;
	structBervalCredentials.bv_len=strlen(cPasswd);
	sprintf(cFQLogin,"%.99s%.32s%.99s",cLoginPrefix,cLogin,cLoginSuffix);
	if(ldap_sasl_bind_s(ld,cFQLogin,NULL,&structBervalCredentials,NULL,NULL,NULL)!=LDAP_SUCCESS)
	{
		ldapErrorLog("ldap_sasl_bind_s()",ld);
		return(0);
	}

	structTimeval.tv_sec=1;
	structTimeval.tv_usec=0;
	//Initiate sync search
	sprintf(cFilter,"%.99s%.32s%.99s",cFilterPrefix,cLogin,cFilterSuffix);
	if(ldap_search_ext_s(ld,cSearchDN,LDAP_SCOPE_SUBTREE,cFilter,caAttrs,0,NULL,NULL,
							&structTimeval,0,&ldapMsg)!=LDAP_SUCCESS)
	{
		ldapErrorLog("ldap_search_ext_s()",ld);
		return(0);
	}

	//Iterate through the returned entries
	unsigned ucPrefixPatternLen=strlen(cPrefixPattern);
	for(ldapEntry=ldap_first_entry(ld,ldapMsg);ldapEntry!=NULL;ldapEntry=ldap_next_entry(ld,ldapEntry))
	{

		for(cpAttr=ldap_first_attribute(ld,ldapEntry,&berElement);cpAttr!=NULL;
					cpAttr=ldap_next_attribute(ld,ldapEntry,berElement))
		{
			if((structBervals=ldap_get_values_len(ld,ldapEntry,cpAttr))!=NULL)
			{
				for(i=0;structBervals[i]!=NULL;i++)
				{
#ifdef DEBUG_LDAP_L0
	//debug only
	sprintf(cLogEntry,"%.255s",structBervals[i]->bv_val);
	logfileLine("structBervals",cLogEntry);
#endif
					if(strstr(structBervals[i]->bv_val,cLinePattern))
					{
						if((cp=strstr(structBervals[i]->bv_val,cPrefixPattern)))
						{
#ifdef DEBUG_LDAP_L0
	//debug only
	sprintf(cLogEntry,"%.255s",cLinePattern);
	logfileLine("cLinePattern match",cLogEntry);
#endif
							if((cp2=strchr(cp+ucPrefixPatternLen,',')))
								*cp2=0;
							sprintf(cOrganization,"%.99s",cp+ucPrefixPatternLen);
						}
					}
				}
				ldap_value_free_len(structBervals);
			}
			ldap_memfree(cpAttr);
		}

		if(berElement!=NULL)
			ber_free(berElement,0);
	}

	//Cleanup (TODO is ldapEntry mem cleaned above? Check)
	ldap_msgfree(ldapMsg);
	//Ignore errors
	iRes=ldap_unbind_ext_s(ld,NULL,NULL);

	if(cOrganization[0])
	{
		sprintf(cFQOrg,"%s%s%s",cOrgPrefix,cOrganization,cOrgSuffix);
		sprintf(cOrganization,"%.99s",cFQOrg);

#ifdef DEBUG_LDAP
		//debug only
		sprintf(cLogEntry,"%.99s/%.32s/%.99s",cFQLogin,cPasswd,cOrganization);
		logfileLine("iValidLDAPLogin",cLogEntry);
#endif
		return(1);
	}
	else
	{
#ifdef DEBUG_LDAP
		//debug only
		sprintf(cLogEntry,"%.99s/%.32s",cFQLogin,cPasswd);
		logfileLine("iValidLDAPLogin",cLogEntry);
#endif
		return(0);
	}

}//int iValidLDAPLogin()


void ldapErrorLog(char *cMessage,LDAP *ld)
{
	int iResultCode;
	char cLogEntry[256];

	ldap_get_option(ld,LDAP_OPT_RESULT_CODE,&iResultCode);
	//Attempt to log to tLog
	sprintf(cLogEntry,"%s:%s",cMessage,ldap_err2string(iResultCode));
	iDNSLog(0,"ldapErrorLog",cLogEntry);
#ifdef DEBUG_LDAP
	logfileLine("ldapErrorLog",cLogEntry);
#endif
	
}//void ldapErrorLog(char *cMessage,LDAP *ld)


#ifdef DEBUG_LDAP
void logfileLine(const char *cFunction,const char *cLogline)
{
	time_t luClock;
	char cTime[32];
	pid_t pidThis;
	const struct tm *tmTime;
	static FILE *gLfp=NULL;

	if(gLfp==NULL)
	{
		if((gLfp=fopen(cVDNSORGLOGFILE,"a"))==NULL)
		{
			sprintf(gcQuery,"Could not open logfile: %s\n",cVDNSORGLOGFILE);
			htmlPlainTextError(gcQuery);
       		}
	}

	pidThis=getpid();

	time(&luClock);
	tmTime=localtime(&luClock);
	strftime(cTime,31,"%b %d %T",tmTime);

        fprintf(gLfp,"%s vdnsOrg.%s[%u]: %s\n",cTime,cFunction,pidThis,cLogline);
	fflush(gLfp);

}//void logfileLine(char *cLogline)
#endif

#endif
