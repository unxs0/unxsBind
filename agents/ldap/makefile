#
#FILE
#	makefile
#	svn ID removed
#AUTHOR/LEGAL
#	(C) 2009-2010 Gary Wallis for Unixservice, LLC.
#	GPLv2 license applies. See included LICENSE file.
#NOTES
#	This is a CentOS 5 version only.
#	For help contact support @ openisp . net
#REQUIRES
#	Normal unxs and openssl libs and OpenLDAP libldap
#	yum install openldap-devel.i386
CFLAGS= -Wall

LIBS= -L/usr/lib/mysql -L/usr/lib/openisp -lz -lcrypt -lm -lssl -lmysqlclient -lldap

all: unxsLDAP

unxsLDAP: ldap.o
	cc ldap.o -o unxsLDAP $(LIBS)

ldap.o: ldap.c ../../local.h
	cc -c ldap.c -o ldap.o $(CFLAGS)

clean:
	rm -f *.o unxsLDAP

install: unxsLDAP
	install -s -m 500 unxsLDAP /usr/sbin/unxsLDAP
	rm unxsLDAP
