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

	FILE *fp=NULL;
	fp=fopen("/var/www/unxs/logs/cgi.log","a");

	gethostname(gcHostname,98);

	if(getenv("REMOTE_HOST")!=NULL)
		sprintf(gcHost,"%.99s",getenv("REMOTE_HOST"));

	else if(getenv("REMOTE_ADDR")!=NULL)
		sprintf(gcHost,"%.99s",getenv("REMOTE_ADDR"));

	if(getenv("HTTP_X_REAL_IP")!=NULL)
		sprintf(gcHost,"%.99s",getenv("HTTP_X_REAL_IP"));

	//Post with URL get items
	if(getenv("REQUEST_METHOD")!=NULL)
	{
		printf("Content-type: text/text\n\n");
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
		for(x=0;gentries[x].name[0]&&x<8;x++)
			fprintf(fp,"%s=%s\n",gentries[x].name,gentries[x].val);

		printf("Thanks! %s %s\n",gcHost,gcHostname);
	}
	else
	{
		printf("Non-cgi %s %s\n",cArgv[0],cGitVersion);
	}

	if(fp) fclose(fp);
	return(0);

}//end of main()

