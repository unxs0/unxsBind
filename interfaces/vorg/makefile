#
#FILE
#	makefile
#	svn ID removed
#AUTHOR/LEGAL
#	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
#	(C) 2010 Gary Wallis for Unixservice, LLC.
#	GPLv2 license applies. See included LICENSE file.
#PURPOSE
#	General purpose RAD3 interface makefile
#	Must customize CGINAME 
#

CGINAME= vdnsOrg.cgi

CFLAGS=-Wall
#LIBS=-L/usr/lib/mysql -L/usr/lib/openisp -lmysqlclient -ltemplate -lucidr -lz -lcrypt -lm -lssl -lldap
LIBS=-L/usr/lib/mysql -L/usr/lib/openisp -lmysqlclient -ltemplate -lucidr -lz -lcrypt -lm -lssl

interface.cgi: main.o zone.o resource.o glossary.o bulkop.o ldap.o ../../cgi.o mysqlconnect.o
	cc main.o zone.o resource.o glossary.o bulkop.o ldap.o ../../cgi.o mysqlconnect.o -o interface.cgi $(LIBS)

main.o: main.c interface.h ../../local.h ../../mysqlrad.h
	cc -c main.c -o main.o $(CFLAGS) 

zone.o: zone.c interface.h ../../local.h ../../mysqlrad.h
	cc -c zone.c -o zone.o $(CFLAGS) 

resource.o: resource.c interface.h ../../local.h ../../mysqlrad.h
	cc -c resource.c -o resource.o $(CFLAGS) 

glossary.o: glossary.c interface.h ../../local.h ../../mysqlrad.h
	cc -c glossary.c -o glossary.o $(CFLAGS)

bulkop.o: bulkop.c interface.h ../../local.h ../../mysqlrad.h
	cc -o bulkop.o -c bulkop.c $(CFLAGS)

ldap.o: ldap.c  interface.h ../../local.h ../../mysqlrad.h
	cc -o ldap.o -c ldap.c $(CFLAGS)

mysqlconnect.o: mysqlconnect.c  ../../local.h interface.h
	cc -o mysqlconnect.o -c mysqlconnect.c $(CFLAGS)

cgi.o: ../../cgi.c  ../../cgi.h
	cc -o ../../cgi.o -c ../../cgi.c $(CFLAGS)

clean:
	rm -f *.o *.cgi
	
install: interface.cgi
	install -m 510 -g apache -s interface.cgi /var/www/unxs/cgi-bin/$(CGINAME)
	rm -f interface.cgi
