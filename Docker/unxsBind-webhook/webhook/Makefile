#
#FILE
#	/unxsBind/Docker/unxsBind-webhook/webhook/Makefile
#
#AUTHOR/LEGAL
#	(C) 2017 Gary Wallis for Unixservice, LLC.
#NOTES
GIT_VERSION := $(shell git describe --dirty --always --tags)

CFLAGS=-Wall -DGitVersion=\"$(GIT_VERSION)\"
LIBS=

all: webhook.cgi

webhook.cgi: webhook.o cgi.o
	cc webhook.o cgi.o -o webhook.cgi $(LIBS)

webhook.o: webhook.c cgi.h
	cc -c webhook.c -o webhook.o $(CFLAGS)

cgi.o: cgi.h cgi.c
	cc -c cgi.c -o cgi.o $(CFLAGS)

clean:
	rm -f *.o webhook.cgi
