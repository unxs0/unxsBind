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
#include <time.h>


void logfileLine(FILE *fp,const char *cFunction,const char *cLogline)
{

	if(!fp) return;

	time_t luClock;
	char cTime[32];
	pid_t pidThis;
	const struct tm *tmTime;

	pidThis=getpid();

	time(&luClock);
	tmTime=localtime(&luClock);
	strftime(cTime,31,"%b %d %T",tmTime);

	fprintf(fp,"%s webhook[%u]::%s %s.\n",cTime,pidThis,cFunction,cLogline);
	fflush(fp);

}//void logfileLine()


void RunForkedCommand(char *cCommand, FILE *fp)
{
	pid_t pidChild;

	pidChild=fork();
	if(pidChild!=0)
		return;

	system(cCommand);
	logfileLine(fp,"RunForkedCommand",cCommand);

}//void RunForkedCommand(char *cCommand)


int main(int iArgc, char *cArgv[])
{
	char *cGitVersion="GitVersion:"GitVersion;
	char cHost[100]={""};
	char cHostname[100]={""};

	FILE *fp=NULL;
	fp=fopen("/var/www/unxs/logs/cgi.log","a");
	logfileLine(fp,"main","start");

	gethostname(cHostname,98);

	if(getenv("REMOTE_HOST")!=NULL)
		sprintf(cHost,"%.99s",getenv("REMOTE_HOST"));

	else if(getenv("REMOTE_ADDR")!=NULL)
		sprintf(cHost,"%.99s",getenv("REMOTE_ADDR"));

	if(getenv("HTTP_X_REAL_IP")!=NULL)
		sprintf(cHost,"%.99s",getenv("HTTP_X_REAL_IP"));

	//Post with URL get items
	if(getenv("REQUEST_METHOD")!=NULL)
	{
		register int x;
		char cRedeploy[100]={""};
		//pentry entries[256];
		//int cl=0;
		entry gentries[8];
		char *gcl;

		memset(gentries,0,sizeof(gentries));

		printf("Content-type: text/text\n\n");
		gcl=getenv("QUERY_STRING");

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
		{
			logfileLine(fp,"name",gentries[x].name);
			logfileLine(fp,"value",gentries[x].val);
			if(!strcmp(gentries[x].name,"redeploy"))
				sprintf(cRedeploy,"%.99s",gentries[x].val);
		}

		printf("Thanks! %s %s\n",cHost,cHostname);
		fflush(stdout);
		if(cRedeploy[0])
		{
			char cCommand[256];
			sprintf(cCommand,"/usr/bin/docker rm -f %.99s;"
					"cd /root/Docker/Running;"
					"/usr/bin/docker-compose up -d",
							cRedeploy);
			RunForkedCommand(cCommand,fp);
		}
	}
	else
	{
		printf("Non-cgi %s %s\n",cArgv[0],cGitVersion);
		logfileLine(fp,"main","Non-cgi");
	}

	logfileLine(fp,"main","end");
	if(fp) fclose(fp);
	return(0);

}//end of main()


