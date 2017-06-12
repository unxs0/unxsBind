/*
FILE 
	webhook.c
PURPOSE
	Provide endpoint for GitHub and Docker HUb webhook callbacks.
LEGAL
	(C) 2017 Gary Wallis for Unixservice, LLC.
	GPLv2 License Applies
OTHER
HELP

*/

#include <stdio.h>
#include "cgi.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main(int iArgc, char *cArgv[])
{
	char *cGitVersion="GitVersion:"GitVersion;
	pentry entries[256];
	entry gentries[8];
	char *gcl;
	int cl=0;
	char gcHost[100]={""};
	char gcHostname[100]={""};
	register int x;

	gethostname(gcHostname,98);

	if(getenv("REMOTE_HOST")!=NULL)
		sprintf(gcHost,"%.99s",getenv("REMOTE_HOST"));

	else if(getenv("REMOTE_ADDR")!=NULL)
		sprintf(gcHost,"%.99s",getenv("REMOTE_ADDR"));

	if(getenv("HTTP_X_REAL_IP")!=NULL)
		sprintf(gcHost,"%.99s",getenv("HTTP_X_REAL_IP"));

	//Get method interface
	if(getenv("REQUEST_METHOD")!=NULL)
	{
		if(strcmp(getenv("REQUEST_METHOD"),"POST"))
		{
		        gcl = getenv("QUERY_STRING");

		        for(x=0;gcl[0] != '\0' && x<8;x++)
			{
       	        		getword(gentries[x].val,gcl,'&');
       	        		plustospace(gentries[x].val);
       	        		unescape_url(gentries[x].val);
       	        		getword(gentries[x].name,gentries[x].val,'=');
				//basic anti hacker
				escape_shell_cmd(gentries[x].val);
			}
		}//end get method interface section
	}

	//Post method interface
	if(getenv("CONTENT_LENGTH")==NULL)
	{
		printf("Non-cgi %s %s\n",cArgv[0],cGitVersion);
		return(0);
	}
	cl = atoi(getenv("CONTENT_LENGTH"));
	for(x=0;cl && (!feof(stdin)) && x<256 ;x++)
	{
		entries[x].val = fmakeword(stdin,'&',&cl);
		plustospace(entries[x].val);
		unescape_url(entries[x].val);
		entries[x].name = makeword(entries[x].val,'=');

	}//end post method interface section

	printf("Content-type: text/text\n\n");
	for(x=0;gentries[x].name[0]&&x<8;x++)
		printf("%s=%s\n",gentries[x].name,gentries[x].val);
	for(x=0;entries[x].name[0]&&x<256;x++)
		printf("%s=%s\n",entries[x].name,entries[x].val);
	return(0);

}//end of main()

