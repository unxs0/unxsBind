/*
FILE
	svn ID removed
PURPOSE
	Wrapper for mysql_real_connect() that supports very fast
	connect to main or alternative local.h set MySQL servers.
AUTHOR
	(C) 2010 Gary Wallis for Unixservice, LLC.
NOTES
	Based on unxsBind/mysqlping.c test code.
*/

#include "mysqlrad.h"
#include "local.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>

//This is an important setting that depends on your network setup
#define SELECT_TIMEOUT_USEC 100000

//TOC protos
void ConnectDb(void);
unsigned TextConnectDb(void);
unsigned TextConnectExtDb(MYSQL *Mysql, unsigned uMode);


//allow fqdn in local.h
int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        //herror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        sprintf(ip,"%.31s",inet_ntoa(*addr_list[i]));
        return 0;
    }
     
    return 1;
}


void ConnectDb(void)
{
	//Handle quick cases first
	//Port is irrelevant here. Make it clear.
	mysql_init(&gMysql);
	if(DBIP0==NULL)
	{
		if (mysql_real_connect(&gMysql,DBIP0,DBLOGIN,DBPASSWD,DBNAME,0,DBSOCKET,0))
			return;
	}

	//Now we can use AF_INET/IPPROTO_TCP cases (TCP connections via IP number)
	char cPort[16]={"3306"};//(*1)
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
			iDNS("Could not create ConnectDB() socket DBIP0");

		// Set non-blocking 
		lFcntlArg=fcntl(iSock,F_GETFL,NULL); 
		lFcntlArg|=O_NONBLOCK; 
		fcntl(iSock,F_SETFL,lFcntlArg); 

		//DBIP0 has priority if we can create a connection we
		//move forward immediately.
		memset(&sockaddr_inMySQLServer,0,sizeof(sockaddr_inMySQLServer));
		sockaddr_inMySQLServer.sin_family=AF_INET;
		char cDBIP0[32];
		sprintf (cDBIP0,"%.31s",DBIP0);
		if(isalpha(cDBIP0[0]))
			hostname_to_ip(DBIP0,cDBIP0);
		sockaddr_inMySQLServer.sin_addr.s_addr=inet_addr(cDBIP0);
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
					if(!valopt)
					{
						//Valid fast connection
						close(iSock);//Don't need anymore.
						if(mysql_real_connect(&gMysql,cDBIP0,DBLOGIN,DBPASSWD,
											DBNAME,DBPORT,DBSOCKET,0))
							return;
					}
				} 
			} 
		}
		close(iSock);//Don't need anymore.
	}

	if(DBIP1==NULL)
	{
		if (mysql_real_connect(&gMysql,DBIP1,DBLOGIN,DBPASSWD,DBNAME,0,DBSOCKET,0))
			return;
	}

	if(DBIP1!=NULL)
	{
		if((iSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
			iDNS("Could not create ConnectDB() socket DBIP1");

		// Set non-blocking 
		lFcntlArg=fcntl(iSock,F_GETFL,NULL); 
		lFcntlArg|=O_NONBLOCK; 
		fcntl(iSock,F_SETFL,lFcntlArg); 

		//Fallback to DBIP1
		memset(&sockaddr_inMySQLServer,0,sizeof(sockaddr_inMySQLServer));
		sockaddr_inMySQLServer.sin_family=AF_INET;
		char cDBIP1[32];
		sprintf (cDBIP1,"%.31s",DBIP1);
		if(isalpha(cDBIP1[0]))
			hostname_to_ip(DBIP1,cDBIP1);
		sockaddr_inMySQLServer.sin_addr.s_addr=inet_addr(cDBIP1);
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
					if(!valopt)
					{
						//Valid fast connection
						close(iSock);//Don't need anymore.
						if(mysql_real_connect(&gMysql,cDBIP1,DBLOGIN,DBPASSWD,
											DBNAME,DBPORT,DBSOCKET,0))
							return;
					}
				} 
			} 
		}
		close(iSock);//Don't need anymore.
	}

	//Failure exit 4 cases
	char cMessage[256];
	if(DBIP1!=NULL && DBIP0!=NULL)
		sprintf(cMessage,"Could not connect to DBIP0:%1$s or DBIP1:%1$s\n",cPort);
	else if(DBIP1==NULL && DBIP0==NULL)
		sprintf(cMessage,"Could not connect to local socket\n");
	else if(DBIP0!=NULL && DBIP1==NULL)
		sprintf(cMessage,"Could not connect to DBIP0:%s or local socket (DBIP1)\n",cPort);
	else if(DBIP0==NULL && DBIP1!=NULL)
		sprintf(cMessage,"Could not connect to DBIP1:%s or local socket (DBIP0)\n",cPort);
	else if(1)
		sprintf(cMessage,"Could not connect unexpected case\n");

	iDNS(cMessage);

}//ConnectDb()


unsigned TextConnectDb(void)
{
	//Handle quick cases first
	//Port is irrelevant here. Make it clear.
	mysql_init(&gMysql);
	if(DBIP0==NULL)
	{
		if (mysql_real_connect(&gMysql,DBIP0,DBLOGIN,DBPASSWD,DBNAME,0,DBSOCKET,0))
			return(0);
	}

	//Now we can use AF_INET/IPPROTO_TCP cases (TCP connections via IP number)
	char cPort[16]={"3306"};//(*1)
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
			printf("Could not create TextConnectDB() socket DBIP0\n");
			return(1);
		}

		// Set non-blocking 
		lFcntlArg=fcntl(iSock,F_GETFL,NULL); 
		lFcntlArg|=O_NONBLOCK; 
		fcntl(iSock,F_SETFL,lFcntlArg); 

		//DBIP0 has priority if we can create a connection we
		//move forward immediately.
		memset(&sockaddr_inMySQLServer,0,sizeof(sockaddr_inMySQLServer));
		sockaddr_inMySQLServer.sin_family=AF_INET;
		char cDBIP0[32];
		sprintf (cDBIP0,"%.31s",DBIP0);
		if(isalpha(cDBIP0[0]))
			hostname_to_ip(DBIP0,cDBIP0);
		sockaddr_inMySQLServer.sin_addr.s_addr=inet_addr(cDBIP0);
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
					if(!valopt)
					{
						//Valid fast connection
						close(iSock);//Don't need anymore.
						if(mysql_real_connect(&gMysql,cDBIP0,DBLOGIN,DBPASSWD,
											DBNAME,DBPORT,DBSOCKET,0))
							return(0);
					}
				} 
			} 
		}
		close(iSock);//Don't need anymore.
	}

	if(DBIP1==NULL)
	{
		if (mysql_real_connect(&gMysql,DBIP1,DBLOGIN,DBPASSWD,DBNAME,0,DBSOCKET,0))
			return(0);
	}

	if(DBIP1!=NULL)
	{
		if((iSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
		{
			printf("Could not create TextConnectDB() socket DBIP1\n");
			return(1);
		}

		// Set non-blocking 
		lFcntlArg=fcntl(iSock,F_GETFL,NULL); 
		lFcntlArg|=O_NONBLOCK; 
		fcntl(iSock,F_SETFL,lFcntlArg); 

		//Fallback to DBIP1
		memset(&sockaddr_inMySQLServer,0,sizeof(sockaddr_inMySQLServer));
		sockaddr_inMySQLServer.sin_family=AF_INET;
		char cDBIP1[32];
		sprintf (cDBIP1,"%.31s",DBIP1);
		if(isalpha(cDBIP1[0]))
			hostname_to_ip(DBIP1,cDBIP1);
		sockaddr_inMySQLServer.sin_addr.s_addr=inet_addr(cDBIP1);
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
					if(!valopt)
					{
						//Valid fast connection
						close(iSock);//Don't need anymore.
						if(mysql_real_connect(&gMysql,cDBIP1,DBLOGIN,DBPASSWD,
											DBNAME,DBPORT,DBSOCKET,0))
							return(0);
					}
				} 
			} 
		}
		close(iSock);//Don't need anymore.
	}

	//Failure exit 4 cases
	char cMessage[256];
	if(DBIP1!=NULL && DBIP0!=NULL)
		sprintf(cMessage,"Could not connect to DBIP0:%1$s or DBIP1:%1$s\n",cPort);
	else if(DBIP1==NULL && DBIP0==NULL)
		sprintf(cMessage,"Could not connect to local socket\n");
	else if(DBIP0!=NULL && DBIP1==NULL)
		sprintf(cMessage,"Could not connect to DBIP0:%s or local socket (DBIP1)\n",cPort);
	else if(DBIP0==NULL && DBIP1!=NULL)
		sprintf(cMessage,"Could not connect to DBIP1:%s or local socket (DBIP0)\n",cPort);
	else if(1)
		sprintf(cMessage,"Could not connect unexpected case\n");

	printf(cMessage);
	return(1);

}//unsigned TextConnectDb()


unsigned TextConnectExtDb(MYSQL *Mysql, unsigned uMode)
{
	static unsigned uOnlyOnce=0;

	//Provide some fall back if nothing in tConfiguration
	char cDbIp[256]={"127.0.0.1"};
	char cDbName[256]={"unxsisp"};
	char cDbPwd[256]={"wsxedc"};
	char cDbLogin[256]={"unxsisp"};
	char cDbPort[256]={"3306"};
	unsigned ucDbPort=3306;

	if(uOnlyOnce)
	{
		printf("TextConnectExtDB() called more than once\n");
		return(1);
	}
	uOnlyOnce=1;

	if(uMode==TEXT_CONNECT_APACHE)
	{
		GetConfiguration("cExtApacheDbIp",cDbIp,0);
		GetConfiguration("cExtApacheDbName",cDbName,0);
		GetConfiguration("cExtApacheDbPwd",cDbPwd,0);
		GetConfiguration("cExtApacheDbLogin",cDbLogin,0);
		GetConfiguration("cExtApacheDbPort",cDbPort,0);
	}
	else if(uMode==TEXT_CONNECT_UNXSVZ)
	{
		GetConfiguration("cExtVzDbIp",cDbIp,0);
		GetConfiguration("cExtVzDbName",cDbName,0);
		GetConfiguration("cExtVzDbPwd",cDbPwd,0);
		GetConfiguration("cExtVzDbLogin",cDbLogin,0);
		GetConfiguration("cExtVzDbPort",cDbPort,0);
	}
	else if(uMode==TEXT_CONNECT_ISP)
	{
		GetConfiguration("cExtJobQueueDbIp",cDbIp,0);
		GetConfiguration("cExtJobQueueDbName",cDbName,0);
		GetConfiguration("cExtJobQueueDbPwd",cDbPwd,0);
		GetConfiguration("cExtJobQueueDbLogin",cDbLogin,0);
		GetConfiguration("cExtJobQueueDbPort",cDbPort,0);
	}

	mysql_init(Mysql);
	if(!cDbIp[0])
	{
		if(mysql_real_connect(Mysql,NULL,cDbLogin,cDbPwd,cDbName,0,NULL,0))
			return(0);
	}
	else
	{

		if(cDbPort[0])
			sscanf(cDbPort,"%u",&ucDbPort);

		//Now we can use AF_INET/IPPROTO_TCP cases (TCP connections via IP number)
		int iSock,iConRes;
		long lFcntlArg;
		struct sockaddr_in sockaddr_inMySQLServer;
		fd_set myset; 
		struct timeval tv; 
		int valopt;
		socklen_t lon; 

		if((iSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
		{
			printf("Could not create TextConnectExtDB() socket\n");
			return(1);
		}

		// Set non-blocking 
		lFcntlArg=fcntl(iSock,F_GETFL,NULL); 
		lFcntlArg|=O_NONBLOCK; 
		fcntl(iSock,F_SETFL,lFcntlArg); 

		//cDbIp0 has priority if we can create a connection we
		//move forward immediately.
		memset(&sockaddr_inMySQLServer,0,sizeof(sockaddr_inMySQLServer));
		sockaddr_inMySQLServer.sin_family=AF_INET;
		sockaddr_inMySQLServer.sin_addr.s_addr=inet_addr(cDbIp);
		sockaddr_inMySQLServer.sin_port=htons(atoi(cDbPort));
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
					if(!valopt)
					{
						//Valid fast connection
						close(iSock);//Don't need anymore.
						if(mysql_real_connect(Mysql,cDbIp,cDbLogin,cDbPwd,cDbName,
								ucDbPort,NULL,0))
							return(0);
					}
				} 
			} 
		}
		close(iSock);//Don't need anymore.
	}

	//Failure exit 2 cases
	char cMessage[256];
	if(cDbIp[0])
		sprintf(cMessage,"Could not connect to %s:%s %s\n",cDbIp,cDbPort,cDbName);
	else
		sprintf(cMessage,"Could not connect to local socket\n");
	printf(cMessage);
	return(1);

}//unsigned TextConnectExtDb(MYSQL *Mysql, unsigned uMode)
