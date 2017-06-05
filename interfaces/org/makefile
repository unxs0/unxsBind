#
#FILE
#	makefile
#	svn ID removed
#AUTHOR/LEGAL
#	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
#	(C) 2010 Gary Wallis for Unixservice, LLC.
#	GPLv2 license applies. See LICENSE file included.
#PURPOSE
#	General purpose RAD3 interface makefile
#	Must customize CGINAME 

CGINAME= idnsOrg.cgi

CFLAGS=-Wall
LIBS=-L/usr/lib/mysql -L/usr/lib/openisp -lmysqlclient -ltemplate -lucidr -lz -lcrypt -lm -lssl

interface.cgi: main.o zone.o resource.o glossary.o bulkop.o mysqlconnect.o ../../cgi.o 
	cc main.o zone.o resource.o glossary.o bulkop.o mysqlconnect.o ../../cgi.o -o interface.cgi $(LIBS)

main.o: main.c interface.h ../../local.h ../../mysqlrad.h
	cc -c main.c -o main.o $(CFLAGS) 

zone.o: zone.c interface.h ../../local.h ../../mysqlrad.h
	cc -c zone.c -o zone.o $(CFLAGS) 

resource.o: resource.c interface.h ../../local.h ../../mysqlrad.h
	cc -c resource.c -o resource.o $(CFLAGS) 

glossary.o : glossary.c interface.h ../../local.h ../../mysqlrad.h
	cc -c glossary.c -o glossary.o $(CFLAGS)

bulkop.o : bulkop.c interface.h ../../local.h ../../mysqlrad.h
	cc -o bulkop.o -c bulkop.c $(CFLAGS)

mysqlconnect.o : mysqlconnect.c  ../../local.h interface.h
	cc -o mysqlconnect.o -c mysqlconnect.c $(CFLAGS)

clean:
	rm -f *.o *.cgi
	
install: interface.cgi ./templates/Footer ./templates/Login.Body
	install -m 510 -g apache -s interface.cgi /var/www/unxs/cgi-bin/$(CGINAME)
	rm -f interface.cgi


./templates/Footer: ./templates/Footer.default
	@ if [ ! -f ./templates/Footer ];then cp -i ./templates/Footer.default ./templates/Footer; fi

./templates/Login.Body: ./templates/Login.Body.default
	@ if [ ! -f ./templates/Login.Body ];then cp -i ./templates/Login.Body.default ./templates/Login.Body; fi
