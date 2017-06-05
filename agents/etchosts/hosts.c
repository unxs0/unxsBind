/*
FILE
        hosts.c
        svn ID removed

PURPOSE
        Create /etc/hosts compatible lines for A records for given zone matching string.

AUTHOR
        (C) 2013-2014, Gary Wallis for Unixservice, LLC. GPL2 Licensed
REQUIRES
	CentOS 5.2+
*/

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <mysql/mysql.h>
#include <string.h>

#include "local.h"
#include "etchosts.h"

MYSQL gMysql;
char gcQuery[1024]={""};

int main(int iArgc, char *cArgv[])
{
        MYSQL_RES *res;         
        MYSQL_ROW field;

	if(iArgc<3)
	{
		fprintf(stderr,"usage: %s <mysql rlike expression for cZone> <uView> [<NOT rlike expression for cZone>]\n",cArgv[0]);
		return(0);
	}
        if(TextConnectDb())
		return(1);

	if(iArgc>3 && cArgv[3])
		sprintf(gcQuery,"SELECT tResource.cName,tResource.cParam1,tZone.cZone"
			" FROM tResource,tZone"
			" WHERE tResource.uZone=tZone.uZone"
			" AND tZone.cZone RLIKE '%s'"
			" AND tZone.cZone NOT RLIKE '%s'"
			" AND tZone.uView=%s"
			" AND tResource.uRRType=1"
			" ORDER BY tResource.uZone",cArgv[1],cArgv[3],cArgv[2]);
	else
		sprintf(gcQuery,"SELECT tResource.cName,tResource.cParam1,tZone.cZone"
			" FROM tResource,tZone"
			" WHERE tResource.uZone=tZone.uZone"
			" AND tZone.cZone RLIKE '%s'"
			" AND tZone.uView=%s"
			" AND tResource.uRRType=1"
			" ORDER BY tResource.uZone",cArgv[1],cArgv[2]);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		fprintf(stderr,"%s\n",gcQuery);
		return(1);
	}
	res=mysql_store_result(&gMysql);
	char *cp;
	char cZone[129]={""};
	while((field=mysql_fetch_row(res)))
	{
	/*
		if(strcmp(field[2],cZone))
		{
			sprintf(cZone,"%.128s",field[2]);
			printf("#cZone=%s\n",cZone);
		}
	*/
		if((cp=strchr(field[0]+strlen(field[0])-1,'.')))
			*cp=0;
		printf("%.32s %.128s\n",field[1],field[0]);
	}
	mysql_free_result(res);
	mysql_close(&gMysql);
        return(0);

}//main()
