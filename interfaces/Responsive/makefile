#
#FILE
#	makefile
#	svn ID removed
#AUTHOR/LEGAL
#	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
#	(C) 2010-2015 Gary Wallis for Unixservice, LLC.
#	GPLv2 license applies. See included LICENSE file.
#PURPOSE
#	General purpose RAD3 interface makefile
#	Must customize CGINAME 
#

CGINAME= unxsDNS.cgi

CFLAGS=-Wall
//LIBS=-L/usr/lib/mysql -L/usr/lib/openisp -lmysqlclient -ltemplate -lucidr -lz -lcrypt -lm -lssl -lldap
#LIBS=-L/usr/lib/mysql -L/usr/lib/openisp -L/usr/lib/oath -L/usr/lib64 -lmysqlclient -ltemplate -lucidr -lz -lcrypt -lm -lssl -loath -lcurl
LIBS=-L/usr/lib/mysql -L/usr/lib/openisp -L/usr/lib/oath -L/usr/lib64 -lmysqlclient -ltemplate -lucidr -lz -lcrypt -lm -lssl -loath 

help:
	@echo "These are the main make options available"
	@echo "make interface.cgi"
	@echo "make install"
	@echo "make install-test"
	@echo "make install-bootstrap"
	@echo "Note! The template.default dir contents is initially copied to your local working directory ./templates."
	@echo " This means that if you are developing templates you should edit template.default files and then"
	@echo " use the new ./importDefaultTemplates.sh script"

interface.cgi: main.o glossary.o user.o zone.o ../../cgi.o mysqlconnect.o 
	cc main.o glossary.o user.o zone.o ../../cgi.o mysqlconnect.o -o interface.cgi $(LIBS)

main.o: main.c interface.h ../../local.h ../../mysqlrad.h
	cc -c main.c -o main.o $(CFLAGS) 

glossary.o: glossary.c interface.h ../../local.h ../../mysqlrad.h
	cc -c glossary.c -o glossary.o $(CFLAGS)

user.o: user.c interface.h ../../local.h ../../mysqlrad.h
	cc -c user.c -o user.o $(CFLAGS)

zone.o: zone.c interface.h ../../local.h ../../mysqlrad.h
	cc -c zone.c -o zone.o $(CFLAGS)

mysqlconnect.o: mysqlconnect.c  ../../local.h interface.h
	cc -o mysqlconnect.o -c mysqlconnect.c $(CFLAGS)

cgi.o: ../../cgi.c  ../../cgi.h
	cc -o ../../cgi.o -c ../../cgi.c $(CFLAGS)

clean:
	rm -f *.o *.cgi
	
install-test: interface.cgi ./templates/Footer 
	install -m 510 -g apache -s interface.cgi /var/www/unxs/html/test/$(CGINAME)
	rm -f interface.cgi

install: interface.cgi ./templates/Footer
	install -m 510 -g apache -s interface.cgi /var/www/unxs/html/$(CGINAME)
	rm -f interface.cgi

./templates/Footer: ./templates.default/Footer
	@ mkdir -p ./templates
	@ cp -ai ./templates.default/* ./templates/;

install-bootstrap: 
	cp -i css/*.css /var/www/unxs/html/bs/css/
	cp -i js/*.js /var/www/unxs/html/bs/js/
	cp -i fonts/* /var/www/unxs/html/bs/fonts/
