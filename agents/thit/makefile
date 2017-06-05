#
#FILE
#	makefile
#	svn ID removed
#AUTHOR/LEGAL
#	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
#	(C) 2010 Gary Wallis for Unixservice, LLC.
#	GPLv2 license applies. See LICENSE file included.
#PURPOSE
#	Makefile for special commandline only interface for named (DNS/BIND 9.3.3+) 
#	per zone named.stats logging
#

CFLAGS=-Wall
LIBS=-L/usr/lib/mysql -lmysqlclient -lssl -lz -lm

tHitCollector: main.o mysqlconnect.o
	cc main.o mysqlconnect.o -o tHitCollector $(LIBS)

main.o: main.c ../../local.h ../../mysqlrad.h
	cc -c main.c -o main.o $(CFLAGS) 

mysqlconnect.o : mysqlconnect.c  ../../local.h ../../mysqlrad.h
	cc -o mysqlconnect.o -c mysqlconnect.c $(CFLAGS)

clean:
	rm -f *.o tHitCollector
	
install: tHitCollector
	install -m 500 -s tHitCollector /usr/sbin/tHitCollector
	rm -f tHitCollector
