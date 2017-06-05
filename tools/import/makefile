#
# FILE
# 	iDNS/tools/makefile
# 	svn ID removed
# AUTHOR/LEGAL
# (C) 2001-2007 Gary Wallis under GPL License see www.fsf.org for details

CFLAGS= -DLinux -Wall
#centos
LIBS= /usr/lib/mysql/libmysqlclient.a -lz -lcrypt -lm -lssl /usr/lib/openisp/libucidr.a
#debian
#LIBS= /usr/lib/mysql/libmysqlclient.a -lz -lcrypt -lm /usr/lib/openisp/libucidr.a

all: idns-importzones idns-importarpaptrs

idns-importarpaptrs: idns-importarpaptrs.o 
	cc idns-importarpaptrs.o -o idns-importarpaptrs $(LIBS)

idns-importarpaptrs.o: idns-importarpaptrs.c ../../local.h ../../mysqlrad.h
	cc -c idns-importarpaptrs.c -o idns-importarpaptrs.o $(CFLAGS)

idns-importzones: idns-importzones.o 
	cc idns-importzones.o -o idns-importzones $(LIBS)

idns-importzones.o: idns-importzones.c ../../local.h ../../mysqlrad.h
	cc -c idns-importzones.c -o idns-importzones.o $(CFLAGS)

install-arpa: idns-importarpaptrs
	install -s idns-importarpaptrs /usr/sbin/idns-importarpaptrs
	rm -f idns-importarpaptrs

install-zones: idns-importzones
	install -s idns-importzones /usr/sbin/idns-importzones
	rm -f idns-importzones

install-all: idns-importzones idns-importarpaptrs
	install -s idns-importzones /usr/sbin/idns-importzones
	rm -f idns-importzones
	install -s idns-importarpaptrs /usr/sbin/idns-importarpaptrs
	rm -f idns-importarpaptrs

clean:
	rm -f *.o

../../local.h: ../../local.h.default
	@ if [ ! -f ../../local.h ];then cp -i ../../local.h.default ../../local.h; fi

