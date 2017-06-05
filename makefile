#
#FILE
#	makefile
#	svn ID removed
#
#AUTHOR/LEGAL
#	(C) 2001-2010 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
#	GPLv2 license applies. See LICENSE file included.
#NOTES
#	This is a CentOS5 rpm release version only (use CFLAG -pedantic to cleanup.)
#	For help contact support @ openisp . net
GIT_VERSION := $(shell git describe --dirty --always --tags)

CFLAGS=-Wall -DGitVersion=\"$(GIT_VERSION)\"

#LIBS= -L/usr/lib/mysql -L/usr/lib64/mysql -L/usr/lib/openisp -lz -lcrypt -lm -lssl -lucidr -lmysqlclient
#LIBS=-L/usr/lib/mysql -L/usr/lib64/mysql -L/usr/lib/openisp -L/usr/lib/oath -lmysqlclient -lz -lcrypt -lm -lssl -lucidr -ltemplate -loath
LIBS=-L/usr/lib/mysql -L/usr/lib64/mysql -L/usr/lib/openisp -lmysqlclient -lz -lcrypt -lm -lssl -lucidr -ltemplate

all: iDNS.cgi

iDNS.cgi: tzone.o tresource.o trrtype.o tjob.o tmailserver.o tconfiguration.o tnstype.o tnsset.o tns.o tserver.o ttemplate.o ttemplateset.o ttemplatetype.o tlog.o tlogtype.o tblock.o tview.o tregistrar.o tglossary.o  tgrouptype.o tgroup.o tgroupglue.o tzoneimport.o tresourceimport.o tmonthhit.o tmonth.o tlogmonth.o thit.o thitmonth.o tdeletedzone.o tdeletedresource.o tclient.o tauthorize.o  bind.o main.o import.o extjobqueue.o cgi.o mysqlconnect.o
	cc tzone.o tresource.o trrtype.o tjob.o tmailserver.o tconfiguration.o tnstype.o tnsset.o tns.o tserver.o ttemplate.o ttemplateset.o ttemplatetype.o tlog.o tlogtype.o tblock.o tview.o tregistrar.o tglossary.o  tgrouptype.o tgroup.o tgroupglue.o tzoneimport.o tresourceimport.o tmonthhit.o tmonth.o tlogmonth.o thit.o thitmonth.o tdeletedzone.o tdeletedresource.o tclient.o tauthorize.o  bind.o main.o import.o extjobqueue.o cgi.o mysqlconnect.o -o iDNS.cgi $(LIBS)

mysqlping: mysqlping.o
	cc mysqlping.c -o mysqlping $(LIBS)

mysqlping.o: mysqlping.c local.h
	cc -c mysqlping.c -o mysqlping.o $(CFLAGS)

mysqlconnect.o: mysqlconnect.c mysqlrad.h local.h
	cc -c mysqlconnect.c -o mysqlconnect.o $(CFLAGS)

tzone.o: tzone.c mysqlrad.h language.h tzonefunc.h local.h 
	cc -c tzone.c -o tzone.o $(CFLAGS)

tresource.o: tresource.c mysqlrad.h language.h tresourcefunc.h local.h
	cc -c tresource.c -o tresource.o $(CFLAGS)

trrtype.o: trrtype.c mysqlrad.h language.h trrtypefunc.h local.h
	cc -c trrtype.c -o trrtype.o $(CFLAGS)

tjob.o: tjob.c mysqlrad.h language.h tjobfunc.h local.h
	cc -c tjob.c -o tjob.o $(CFLAGS)

tmailserver.o: tmailserver.c mysqlrad.h language.h tmailserverfunc.h local.h
	cc -c tmailserver.c -o tmailserver.o $(CFLAGS)

tconfiguration.o: tconfiguration.c mysqlrad.h language.h tconfigurationfunc.h local.h
	cc -c tconfiguration.c -o tconfiguration.o $(CFLAGS)

tnstype.o: tnstype.c mysqlrad.h language.h tnstypefunc.h local.h
	cc -c tnstype.c -o tnstype.o $(CFLAGS)

tnsset.o: tnsset.c mysqlrad.h language.h tnssetfunc.h local.h
	cc -c tnsset.c -o tnsset.o $(CFLAGS)

tns.o: tns.c mysqlrad.h language.h tnsfunc.h local.h
	cc -c tns.c -o tns.o $(CFLAGS)

tserver.o: tserver.c mysqlrad.h language.h tserverfunc.h local.h
	cc -c tserver.c -o tserver.o $(CFLAGS)

ttemplate.o: ttemplate.c mysqlrad.h language.h ttemplatefunc.h local.h
	cc -c ttemplate.c -o ttemplate.o $(CFLAGS)

ttemplateset.o: ttemplateset.c mysqlrad.h language.h ttemplatesetfunc.h local.h
	cc -c ttemplateset.c -o ttemplateset.o $(CFLAGS)

ttemplatetype.o: ttemplatetype.c mysqlrad.h language.h ttemplatetypefunc.h local.h
	cc -c ttemplatetype.c -o ttemplatetype.o $(CFLAGS)

tlog.o: tlog.c mysqlrad.h language.h tlogfunc.h local.h
	cc -c tlog.c -o tlog.o $(CFLAGS)

tlogtype.o: tlogtype.c mysqlrad.h language.h tlogtypefunc.h local.h
	cc -c tlogtype.c -o tlogtype.o $(CFLAGS)

tblock.o: tblock.c mysqlrad.h language.h tblockfunc.h local.h
	cc -c tblock.c -o tblock.o $(CFLAGS)

tview.o: tview.c mysqlrad.h language.h tviewfunc.h local.h
	cc -c tview.c -o tview.o $(CFLAGS)

tregistrar.o: tregistrar.c mysqlrad.h language.h tregistrarfunc.h local.h
	cc -c tregistrar.c -o tregistrar.o $(CFLAGS)

tglossary.o: tglossary.c mysqlrad.h language.h tglossaryfunc.h local.h
	cc -c tglossary.c -o tglossary.o $(CFLAGS)

tzoneimport.o: tzoneimport.c mysqlrad.h language.h tzoneimportfunc.h local.h
	cc -c tzoneimport.c -o tzoneimport.o $(CFLAGS)

tresourceimport.o: tresourceimport.c mysqlrad.h language.h tresourceimportfunc.h local.h
	cc -c tresourceimport.c -o tresourceimport.o $(CFLAGS)

tmonthhit.o: tmonthhit.c mysqlrad.h language.h tmonthhitfunc.h local.h
	cc -c tmonthhit.c -o tmonthhit.o $(CFLAGS)

tmonth.o: tmonth.c mysqlrad.h language.h tmonthfunc.h local.h
	cc -c tmonth.c -o tmonth.o $(CFLAGS)

tlogmonth.o: tlogmonth.c mysqlrad.h language.h tlogmonthfunc.h local.h
	cc -c tlogmonth.c -o tlogmonth.o $(CFLAGS)

thit.o: thit.c mysqlrad.h language.h thitfunc.h local.h
	cc -c thit.c -o thit.o $(CFLAGS)

thitmonth.o: thitmonth.c mysqlrad.h language.h thitmonthfunc.h local.h
	cc -c thitmonth.c -o thitmonth.o $(CFLAGS)

tdeletedzone.o: tdeletedzone.c mysqlrad.h language.h tdeletedzonefunc.h local.h
	cc -c tdeletedzone.c -o tdeletedzone.o $(CFLAGS)

tdeletedresource.o: tdeletedresource.c mysqlrad.h language.h tdeletedresourcefunc.h local.h
	cc -c tdeletedresource.c -o tdeletedresource.o $(CFLAGS)

tclient.o: tclient.c mysqlrad.h language.h tclientfunc.h local.h
	cc -c tclient.c -o tclient.o $(CFLAGS)

tauthorize.o: tauthorize.c mysqlrad.h language.h tauthorizefunc.h local.h
	cc -c tauthorize.c -o tauthorize.o $(CFLAGS)

tgrouptype.o: tgrouptype.c mysqlrad.h language.h tgrouptypefunc.h local.h
	cc -c tgrouptype.c -o tgrouptype.o $(CFLAGS)

tgroup.o: tgroup.c mysqlrad.h language.h tgroupfunc.h local.h
	cc -c tgroup.c -o tgroup.o $(CFLAGS)

tgroupglue.o: tgroupglue.c mysqlrad.h language.h tgroupgluefunc.h local.h
	cc -c tgroupglue.c -o tgroupglue.o $(CFLAGS)

main.o: main.c mysqlrad.h mainfunc.h language.h local.h
	cc -c main.c -o main.o $(CFLAGS)

cgi.o: cgi.h cgi.c
	cc -c cgi.c -o cgi.o $(CFLAGS)

bind.o: bind.c mysqlrad.h
	cc -c bind.c -o bind.o $(CFLAGS)

import.o: import.c mysqlrad.h
	cc -c import.c -o import.o $(CFLAGS)

extjobqueue.o: mysqlrad.h extjobqueue.c
	cc -c extjobqueue.c -o extjobqueue.o $(CFLAGS)

local.h: local.h.default
	@ if [ ! -f local.h ];then cp -i local.h.default local.h; fi

clean:
	rm -f *.o iDNS.cgi

install: iDNS.cgi
	install -m 510 -g apache -s iDNS.cgi /var/www/unxs/cgi-bin/iDNS.cgi
	rm iDNS.cgi

#
#This is a special section for developers with a local MySQL idns database with valid data only
#

TemplateData: data/tTemplate-vdnsOrg.sql data/tTemplate-idnsAdmin.sql data/tTemplate-idnsOrg.sql

data/tTemplate-vdnsOrg.sql: interfaces/vorg/templates/DelegationTool.Body interfaces/vorg/templates/Footer interfaces/vorg/templates/Header interfaces/vorg/templates/InputParam2 interfaces/vorg/templates/InputParam3 interfaces/vorg/templates/InputParam4 interfaces/vorg/templates/OrgBulkOp.Body interfaces/vorg/templates/OrgGlossary.Body interfaces/vorg/templates/Resource.Body interfaces/vorg/templates/ResourceWizard.1 interfaces/vorg/templates/ResourceWizard.2 interfaces/vorg/templates/ResourceWizard.3 interfaces/vorg/templates/VZone.Body interfaces/vorg/templates/ZLogin.Body 
	cd interfaces/vorg; ./importTemplates.sh > /dev/null;
	cd ../../;
	mysqldump --compact --no-create-info --where="uTemplateType=3" -p idns tTemplate > data/tTemplate-vdnsOrg.sql

data/tTemplate-idnsAdmin.sql: interfaces/admin/templates/ActTableRowContact interfaces/admin/templates/ActTableRowLabelAdmin interfaces/admin/templates/ActTableRowLabelContact interfaces/admin/templates/Admin.Resource.Body interfaces/admin/templates/Admin.ResourceWizard.1 interfaces/admin/templates/Admin.ResourceWizard.2 interfaces/admin/templates/Admin.ResourceWizard.3 interfaces/admin/templates/Admin.Zone.Body interfaces/admin/templates/AdminBulkOp.Body interfaces/admin/templates/AdminLogin.Body interfaces/admin/templates/AdminUser.Body interfaces/admin/templates/AdminUserWizard.1 interfaces/admin/templates/AdminUserWizard.2 interfaces/admin/templates/AdminUserWizard.3 interfaces/admin/templates/Customer.Body interfaces/admin/templates/CustomerUser.Body interfaces/admin/templates/CustomerUserWizard.1 interfaces/admin/templates/CustomerUserWizard.2 interfaces/admin/templates/CustomerUserWizard.3 interfaces/admin/templates/CustomerUserWizard.4 interfaces/admin/templates/CustomerWizard.1 interfaces/admin/templates/CustomerWizard.2 interfaces/admin/templates/CustomerWizard.3 interfaces/admin/templates/CustomerWizard.4 interfaces/admin/templates/DashBoard.Body interfaces/admin/templates/DelegationTool.Body interfaces/admin/templates/Footer interfaces/admin/templates/Glossary.Body interfaces/admin/templates/Header interfaces/admin/templates/IPAuth.Body interfaces/admin/templates/IPAuthDetails.Body interfaces/admin/templates/IPAuthReport.Body interfaces/admin/templates/InputParam2 interfaces/admin/templates/InputParam3 interfaces/admin/templates/InputParam4 interfaces/admin/templates/LastWWActTableFooter interfaces/admin/templates/LastWWActTableTop interfaces/admin/templates/Report.Body interfaces/admin/templates/ReportFocus.Body interfaces/admin/templates/ReportHitsTop20.Footer interfaces/admin/templates/ReportHitsTop20.Header interfaces/admin/templates/ReportHitsTop20.Row interfaces/admin/templates/ReportOverallChanges interfaces/admin/templates/ReportRightPanel interfaces/admin/templates/RestoreResource.Body interfaces/admin/templates/RestoreZone.Body
	cd interfaces/admin; ./importTemplates.sh > /dev/null;
	cd ../../;
	mysqldump --compact --no-create-info --where="uTemplateType=2" -p idns tTemplate > data/tTemplate-idnsAdmin.sql

data/tTemplate-idnsOrg.sql: interfaces/org/templates/Footer interfaces/org/templates/Header interfaces/org/templates/InputParam2 interfaces/org/templates/InputParam3 interfaces/org/templates/InputParam4 interfaces/org/templates/Login.Body interfaces/org/templates/OrgBulkOp.Body interfaces/org/templates/OrgGlossary.Body interfaces/org/templates/Resource.Body interfaces/org/templates/ResourceWizard.1 interfaces/org/templates/ResourceWizard.2 interfaces/org/templates/ResourceWizard.3 interfaces/org/templates/Zone.Body
	cd interfaces/org; ./importTemplates.sh > /dev/null;
	cd ../../;
	mysqldump --compact --no-create-info --where="uTemplateType=1" -p idns tTemplate > data/tTemplate-idnsOrg.sql
