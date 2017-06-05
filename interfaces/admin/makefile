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

CGINAME=idnsAdmin.cgi

CFLAGS=-Wall
LIBS=-L/usr/lib/mysql -L/usr/lib/openisp -lmysqlclient -ltemplate -lucidr -lz -lcrypt -lm -lssl

interface.cgi: main.o adminuser.o customer.o customercontact.o glossary.o zone.o resource.o bulkop.o report.o \
		restorezone.o restoreresource.o ipauth.o dashboard.o mysqlconnect.o ../../cgi.o 
	cc main.o adminuser.o customer.o customercontact.o glossary.o zone.o resource.o bulkop.o report.o \
	restorezone.o restoreresource.o ipauth.o dashboard.o mysqlconnect.o ../../cgi.o -o interface.cgi $(LIBS)

main.o: main.c interface.h ../../local.h ../../mysqlrad.h
	cc -c main.c -o main.o $(CFLAGS) 

customercontact.o: customercontact.c interface.h ../../local.h ../../mysqlrad.h
	cc -c customercontact.c -o customercontact.o $(CFLAGS) 

customer.o: customer.c interface.h ../../local.h ../../mysqlrad.h
	cc -c customer.c -o customer.o $(CFLAGS)

adminuser.o : adminuser.c interface.h ../../local.h ../../mysqlrad.h
	cc -c adminuser.c -o adminuser.o $(CFLAGS)	

glossary.o : glossary.c interface.h ../../local.h ../../mysqlrad.h
	cc -c glossary.c -o glossary.o $(CFLAGS)

zone.o : zone.c interface.h ../../local.h ../../mysqlrad.h
	cc -c zone.c -o zone.o $(CFLAGS)
	
resource.o : resource.c interface.h ../../local.h ../../mysqlrad.h
	cc -c resource.c -o resource.o $(CFLAGS) 

bulkop.o : bulkop.c interface.h ../../local.h ../../mysqlrad.h
	cc -c bulkop.c -o bulkop.o $(CFLAGS)

report.o : report.c interface.h ../../local.h ../../mysqlrad.h
	cc -c report.c -o report.o $(CFLAGS)

restorezone.o : restorezone.c interface.h ../../local.h ../../mysqlrad.h
	cc -c restorezone.c -o restorezone.o $(CFLAGS)

restoreresource.o : restoreresource.c interface.h ../../local.h ../../mysqlrad.h
	cc -c restoreresource.c -o restoreresource.o $(CFLAGS)

ipauth.o : ipauth.c interface.h ../../local.h ../../mysqlrad.h
	cc -c ipauth.c -o ipauth.o $(CFLAGS)

dashboard.o : dashboard.c interface.h ../../local.h ../../mysqlrad.h
	cc -c dashboard.c -o dashboard.o $(CFLAGS)

mysqlconnect.o : mysqlconnect.c  ../../local.h interface.h
	cc -o mysqlconnect.o -c mysqlconnect.c $(CFLAGS)

clean:
	rm -f *.o *.cgi
	
install: interface.cgi
	install -m 510 -g apache -s interface.cgi /var/www/unxs/cgi-bin/$(CGINAME)
	rm -f interface.cgi

