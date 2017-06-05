/*
FILE
	svn ID removed
PURPOSE
	Test verifying which server is reachable before mysql_real_connect;
AUTHOR
	(C) 2010 Gary Wallis for Unixservice, LLC.
NOTES
	mysql_real_connect() handles the local PF_UNIX just fine, so we can ignore pre 
	testing in those cases.

	Once this code is tested it can replace all current unxsVZ ConnectDB() type functions.
	It was not needed before for mysqlproxy w/lua failover based installs.

	Note that local scoket via NULL DBIP1 overrides remote DBIP0 IP.
*/

#include "mysqlrad.h"
#include "local.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>


MYSQL gMysql;

//This is an important setting that depends on your network setup
#define SELECT_TIMEOUT_USEC 100

int main()
{
	//Handle quick cases first
	//Port is irrelevant here. Make it clear.
	mysql_init(&gMysql);
	if(DBIP0==NULL)
	{
		if (mysql_real_connect(&gMysql,DBIP0,DBLOGIN,DBPASSWD,DBNAME,0,DBSOCKET,0))
		{
			printf("Connected to local socket DBIP0==NULL\n");
			mysql_close(&gMysql);
			exit(0);
		}
	}
	if(DBIP1==NULL)
	{
		if (mysql_real_connect(&gMysql,DBIP1,DBLOGIN,DBPASSWD,DBNAME,0,DBSOCKET,0))
		{
			printf("Connected to local socket DBIP1==NULL\n");
			mysql_close(&gMysql);
			exit(0);
		}
	}

	//Now we can use AF_INET/IPPROTO_TCP cases (TCP connections via IP number)
	char *cPort="3306";//(*1)
	int iSock,iConRes;
	long lFcntlArg;
	struct sockaddr_in sockaddr_inMySQLServer;
	fd_set myset; 
	struct timeval tv; 
	int valopt;
	socklen_t lon; 

	//Default port should really be gathered from a different source
	//but for now we use the known MySQL server CentOS default port (*1).
	if(DBPORT!=0)
		sprintf(cPort,"%u",DBPORT);


	if(DBIP0!=NULL)
	{
		if((iSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
		{
			printf("Could not create socket\n");
			exit(1);
		}
		// Set non-blocking 
		lFcntlArg=fcntl(iSock,F_GETFL,NULL); 
		lFcntlArg|=O_NONBLOCK; 
		fcntl(iSock,F_SETFL,lFcntlArg); 

		//DBIP0 has priority if we can create a connection we
		//move forward immediately.
		memset(&sockaddr_inMySQLServer,0,sizeof(sockaddr_inMySQLServer));
		sockaddr_inMySQLServer.sin_family=AF_INET;
		sockaddr_inMySQLServer.sin_addr.s_addr=inet_addr(DBIP0);
		sockaddr_inMySQLServer.sin_port=htons(atoi(cPort));
		iConRes=connect(iSock,(struct sockaddr *)&sockaddr_inMySQLServer,sizeof(sockaddr_inMySQLServer));
		if(iConRes<0)
		{
			if(errno==EINPROGRESS)
			{
				tv.tv_sec=0; 
				tv.tv_usec=SELECT_TIMEOUT_USEC; 
				FD_ZERO(&myset); 
				FD_SET(iSock,&myset); 
				if(select(iSock+1,NULL,&myset,NULL,&tv)>0)
				{ 
					lon=sizeof(int); 
					getsockopt(iSock,SOL_SOCKET,SO_ERROR,(void*)(&valopt),&lon); 
					if(valopt)
					{ 
						fprintf(stderr, "Error in connection() %d - %s\n",valopt,strerror(valopt)); 
					} 
					else
					{
						//Valid fast connection
						close(iSock);//Don't need anymore.
						mysql_init(&gMysql);
						if(mysql_real_connect(&gMysql,DBIP0,DBLOGIN,DBPASSWD,
											DBNAME,DBPORT,DBSOCKET,0))
						{
							printf("Connected to %s:%s\n",(char *)DBIP0,cPort);
							mysql_close(&gMysql);
							exit(0);
						}
					}
				} 
				else
				{
					printf("DBIP0 else if select()\n");
				}
			} 
			else
			{
				printf("DBIP0 else if errno==EINPROGRESS\n");
			}
		}
		close(iSock);//Don't need anymore.
	}

	if(DBIP1!=NULL)
	{
		if((iSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
		{
			printf("Could not create socket\n");
			exit(1);
		}
		// Set non-blocking 
		lFcntlArg=fcntl(iSock,F_GETFL,NULL); 
		lFcntlArg|=O_NONBLOCK; 
		fcntl(iSock,F_SETFL,lFcntlArg); 

		//Fallback to DBIP1
		memset(&sockaddr_inMySQLServer,0,sizeof(sockaddr_inMySQLServer));
		sockaddr_inMySQLServer.sin_family=AF_INET;
		sockaddr_inMySQLServer.sin_addr.s_addr=inet_addr(DBIP1);
		sockaddr_inMySQLServer.sin_port=htons(atoi(cPort));
		iConRes=connect(iSock,(struct sockaddr *)&sockaddr_inMySQLServer,sizeof(sockaddr_inMySQLServer));
		if(iConRes<0)
		{
			if(errno==EINPROGRESS)
			{
				tv.tv_sec=0; 
				tv.tv_usec=SELECT_TIMEOUT_USEC; 
				FD_ZERO(&myset); 
				FD_SET(iSock,&myset); 
				if(select(iSock+1,NULL,&myset,NULL,&tv)>0)
				{ 
					lon=sizeof(int); 
					getsockopt(iSock,SOL_SOCKET,SO_ERROR,(void*)(&valopt),&lon); 
					if(valopt)
					{ 
						printf("Error in connection() %d - %s\n",valopt,strerror(valopt)); 
					} 
					else
					{
						//Valid fast connection
						close(iSock);//Don't need anymore.
						mysql_init(&gMysql);
						if(mysql_real_connect(&gMysql,DBIP1,DBLOGIN,DBPASSWD,
											DBNAME,DBPORT,DBSOCKET,0))
						{
							printf("Connected to %s:%s\n",(char *)DBIP1,cPort);
							mysql_close(&gMysql);
							exit(0);
						}
					}
				} 
				else
				{
					printf("DBIP1 else if select()\n");
				}
			} 
			else
			{
				printf("DBIP1 else if errno==EINPROGRESS\n");
			}
		}
		close(iSock);//Don't need anymore.
	}

	//Failure exit 4 cases
	if(DBIP1!=NULL && DBIP0!=NULL)
		printf("Could not connect to %s:%s or %s:%s\n",(char *)DBIP0,cPort,(char *)DBIP1,cPort);
	else if(DBIP1==NULL && DBIP0==NULL)
		printf("Could not connect. Tried to use local socket\n");
	else if(DBIP0!=NULL && DBIP1==NULL)
		printf("Could not connect to %s:%s or local socket (DBIP1)\n",(char *)DBIP0,cPort);
	else if(DBIP0==NULL && DBIP1!=NULL)
		printf("Could not connect to %s:%s or local socket (DBIP0)\n",(char *)DBIP1,cPort);
	else if(1)
		printf("Could not connect unexpected case\n");
	
	exit(1);

}//main()
