#
#FILE
#       unxsBind/agents/etchosts
#PURPOSE
#USE
#AUTHOR
#       (C) 2013-2014 Gary Wallis for Unixservice, LLC.
#       GPL License applies and applies to all thrid party libs used herein.
#

CFLAGS= -Wall
LIBS= -L/usr/lib/mysql -lmysqlclient -lz -lm -lssl

all: unxsBindHosts

unxsBindHosts: hosts.o mysqlconnect.o
	gcc hosts.o mysqlconnect.o -o unxsBindHosts $(LIBS)

hosts.o: hosts.c local.h etchosts.h
	gcc -c hosts.c -o hosts.o $(CFLAGS)

mysqlconnect.o: mysqlconnect.c local.h
	cc -c mysqlconnect.c -o mysqlconnect.o $(CFLAGS)

clean:
	rm -f *.o unxsBindHosts

install: unxsBindHosts
	install -s unxsBindHosts /usr/sbin/unxsBindHosts
	@ rm unxsBindHosts

local.h: local.h.default
	@ if [ ! -f local.h ];then cp -i local.h.default local.h; fi

