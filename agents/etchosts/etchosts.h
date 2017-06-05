/*
FILE
	unxsBind/agents/etchosts/etchosts.h
	svn ID removed
AUTHOR/LEGAL
	(C) 2013-2014 Gary Wallis for Unixservice, LLC.
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

#include <mysql/mysql.h>

extern char gcQuery[];
extern MYSQL gMysql; 

//mysqlconnect.c
unsigned TextConnectDb(void);
