Summary: DNS BIND 9 telco quality manager with admin and end-user web interfaces. Integrated rrdtool graphics.
Name: unxsbind
Version: 3.1
Release: 1
License: GPL
Group: System Environment/Applications
Source: http://unixservice.com/source/unxsbind-3.1.tar.gz
URL: http://openisp.net/openisp/unxsBind
Distribution: unxsVZ
Vendor: Unixservice, LLC.
Packager: Unixservice Support Group <supportgrp@unixservice.com>
Requires: unxsadmin >= 1.2 , mysql-server >= 5.0.45 , bind >= 30:9.3.6 , bind-utils >= 30:9.3.6, rrdtool , chkconfig, unxstemplatelib >= 1.0 , unxscidrlib >= 1.0

%description
unxsBind iDNS provides a SaaS DNS BIND9 manager.

Main features supported:

 1-.	Manages unlimited DNS data, for an unlimited number of NS sets, for an unlimited number of DNS servers.
 2-.	Unlimited BIND views (with no BIND master/slave xfer issues or extra IPs required.)
 3-.	Database can fail and DNS services continue.
 4-.	Supports hidden masters, external masters, all masters clusters, forward zones, secondary only zones.
 5-.	Engineering and development friendly web interface backend (visible schema and variables.)
 6-.	Organization/Company contact web interfaces included. Allows non skilled zone owners to modify RRs.
 7-.	User friendly admin web interface included.
 8-.	Bulk zone and resource record operations.
 9-.	Traffic monitoring per zone or per NS set (per NS or aggregated across cluster) 
	with rrdtool graphics.
 10-. 	Wizard for CIDR based downstream delegation of arpa zones.
 11-.	Uses company-contact-role model for allowing management partitioning. Perfect for DNS as a
	service (SaaS/ASP) deployments. The root ASP manages the infrastructure, clients that are 
	companies, NGOs or other organizations have contacts (staff) that can be of diverse
	permission levels for operating on their companies zones.
 12-.	Optional automatic creation of rev dns PTR records.
 13-.	Support for upstream delegated sub class C delegated rev dns zones.
 14-.	Migration and import tools.
 15-.	Support for large SPF/DK/DKIM and other TXT like RRs.
 16-.	Support for SRV and NAPTR RRs.
 17-.	Support for IPv6 AAAA and rev dns for IPv6.
 18-.	Comprehensive DNSSEC support (including key management) will be available as soon as BIND and
	at least three TLD deployments prove stable.

%prep
%setup -q

%build
make

%pre 
#if [ "$1" = "1" ]; then
#	echo "pre: Initial install";
#elif [ "$1" = "2" ]; then
#	echo "pre: Update";
#fi


%install
make install
#mkdir section
mkdir -p /usr/local/idns
mkdir -p /usr/local/share/iDNS/data
mkdir -p /usr/local/share/iDNS/setup9
mkdir -p /usr/local/share/iDNS/admin/templates
mkdir -p /usr/local/share/iDNS/org/templates
mkdir -p /usr/local/share/iDNS/vorg/templates
mkdir -p /var/log/named
#cp files section
cp -u images/* /var/www/unxs/html/images/
cp -u interfaces/admin/templates/images/* /var/www/unxs/html/images/
cp -u interfaces/admin/templates/css/* /var/www/unxs/html/css/
cp `find ./interfaces/admin/templates/ -type f -print` /usr/local/share/iDNS/admin/templates/
cp `find ./interfaces/org/templates/ -type f -print` /usr/local/share/iDNS/org/templates/
cp `find ./interfaces/vorg/templates/ -type f -print` /usr/local/share/iDNS/vorg/templates/
cp data/*.txt /usr/local/share/iDNS/data/
cp data/*.sql /usr/local/share/iDNS/data/
cp setup9/rndc.key /etc/unxsbind-rndc.key
cp setup9/rndc.conf /etc/unxsbind-rndc.conf
cp setup9/unxsbind.init /etc/init.d/unxsbind
cp setup9/* /usr/local/share/iDNS/setup9/
cp -u setup9/DejaVuSansMono-Roman.ttf /usr/share/fonts/
cp agents/mysqlcluster/mysqlcluster.sh /usr/sbin/
#permissions section
#make section
cd interfaces/admin
make install
cd ../org
make install
cd ../vorg
make install
cd ../../agents/thit
cp bind9-genstats.sh /usr/sbin/bind9-genstats.sh
make install
cd $RPM_BUILD_DIR

%files
%doc LICENSE INSTALL
/usr/local/share/iDNS
%config(noreplace) /etc/unxsbind-rndc.key
%config(noreplace) /etc/unxsbind-rndc.conf
%config(noreplace) /usr/sbin/bind9-genstats.sh
%config(noreplace) /usr/sbin/mysqlcluster.sh
#since we modify this file in the post section it seems that
#rpm inner workings will not allow us to include the file here
#since if we do post changes are lost. very annoying.
#maybe an rpm expert can figure this out and provide a patch?
#%config(noreplace) /usr/local/idns/named.conf
#%config(noreplace) /usr/local/idns/named.d/master.zones
#/usr/local/idns/named.d/root.cache
#/usr/local/idns/named.d/master/127.0.0
#/usr/local/idns/named.d/master/localhost
#/etc/init.d/unxsbind
#/etc/cron.d/unxsbind
/var/www/unxs/cgi-bin/iDNS.cgi
/var/www/unxs/cgi-bin/idnsAdmin.cgi
/var/www/unxs/cgi-bin/idnsOrg.cgi
/var/www/unxs/cgi-bin/vdnsOrg.cgi
/usr/sbin/tHitCollector
/var/www/unxs/html/images/allzone.stats.png
/var/www/unxs/html/images/green.gif
/var/www/unxs/html/images/null.gif
/var/www/unxs/html/images/red.gif
/var/www/unxs/html/images/unxsbind.jpg
/var/www/unxs/html/images/yellow.gif
/var/www/unxs/html/images/bgrd_header_engage.gif
/var/www/unxs/html/images/bgrd_masthead.gif
/var/www/unxs/html/images/bgrd_topnav.gif
/var/www/unxs/html/images/bgrd_topnav_systxt.gif
/var/www/unxs/html/images/btn_mast_search.gif
/var/www/unxs/html/images/clear.gif
/var/www/unxs/html/css/popups.js
/var/www/unxs/html/css/styles.css
/usr/share/fonts/DejaVuSansMono-Roman.ttf
%dir /var/log/named

%post
#fix cgi group
mkdir -p /usr/local/idns/named.d/master
mkdir -p /usr/local/idns/named.d/slave
cp /var/www/unxs/html/images/green.gif /var/www/unxs/html/images/mrcstatus.gif
cp /usr/local/share/iDNS/setup9/root.cache /usr/local/idns/named.d/root.cache
cp /usr/local/share/iDNS/setup9/127.0.0 /usr/local/idns/named.d/master/127.0.0
cp /usr/local/share/iDNS/setup9/localhost /usr/local/idns/named.d/master/localhost
touch /var/log/named-idns.log
chgrp named /var/log/named-idns.log
chmod g+w /var/log/named-idns.log
chgrp apache /var/www/unxs/cgi-bin/iDNS.cgi;
chgrp apache /var/www/unxs/cgi-bin/idnsAdmin.cgi;
chgrp apache /var/www/unxs/cgi-bin/idnsOrg.cgi;
chgrp apache /var/www/unxs/cgi-bin/vdnsOrg.cgi;
chmod 500 /usr/sbin/bind9-genstats.sh;
chmod 500 /usr/sbin/mysqlcluster.sh
chmod 755 /etc/init.d/unxsbind;
chmod -R og+x /usr/local/idns
chown -R named:named /usr/local/idns
chown -R mysql:mysql /usr/local/share/iDNS/data
if [ "$1" = "1" ]; then
	#echo "post: Initial install";
	if [ -x /sbin/chkconfig ];then
		if [ -x /etc/init.d/named ];then
			/sbin/chkconfig --level 3 named off;
			if [ $? == 0 ];then
				echo "named chkconfig ok";
			fi
		fi
		if [ -x /etc/init.d/unxsbind ];then
			/sbin/chkconfig --level 3 unxsbind on;
			if [ $? == 0 ];then
				echo "unxsbind chkconfig ok";
			fi
		fi
		if [ -x /etc/init.d/httpd ];then
			/sbin/chkconfig --level 3 httpd on
			if [ $? == 0 ];then
				echo "httpd chkconfig ok";
			fi
			/etc/init.d/httpd restart > /dev/null 2>&1
			if [ $? == 0 ];then
				echo "httpd restart ok";
				cHttpdStart="1";
			fi
		fi
		if [ -x /etc/init.d/mysqld ];then
			/sbin/chkconfig --level 3 mysqld on
			if [ $? == 0 ];then
				echo "mysqld chkconfig ok";
			fi
			/etc/init.d/mysqld restart > /dev/null 2>&1
			if [ $? == 0 ];then
				echo "mysqld restart ok";
				cMySQLStart="1";
			fi
		fi
	fi
	#if mysqld has no root passwd and we started it then we will set it and finish the data initialize
	if [ -x /usr/bin/mysql ];then
		if [ "$cMySQLStart" == "1" ];then
			echo "quit" | /usr/bin/mysql  > /dev/null 2>&1;
			if [ $? == 0 ];then
				/usr/bin/mysqladmin -u root password 'ultrasecret' > /dev/null 2>&1;
				if [ $? == 0 ];then
					echo "mysqld root password set to 'ultrasecret' change ASAP!";
					export ISMROOT=/usr/local/share;
					/var/www/unxs/cgi-bin/iDNS.cgi Initialize ultrasecret > /dev/null 2>&1;
					if [ $? == 0 ];then
						echo "initialize ok";
						cInitialize="1";
					fi
				fi
			fi
			#get server's main/first IP. End user can change later if this is not correct.
	cIP=`/sbin/ifconfig|/usr/bin/head -n 2|/usr/bin/tail -n 1|/bin/awk -F'inet addr:' '{print $2}'|/bin/cut -f 1 -d " "`;
			if [ $? != 0 ] || [ "$cIP" == "" ];then
				echo "Error geting cIP";
			else
				export ISMROOT=/usr/local/share;	
				#This must be run after mysqld is started
				/var/www/unxs/cgi-bin/iDNS.cgi installbind $cIP > /dev/null 2>&1;
				if [ $? == 0 ];then
					echo "iDNS.cgi installbind ok $cIP";
				fi
			fi
		fi
	fi
	#if for some reason named.conf is missing use default
	if [ ! -f /usr/local/idns/named.conf ];then
		cp /usr/local/share/iDNS/setup9/named.conf.localhost /usr/local/idns/named.conf
		if [ $? == 0 ];then
			echo "localhost named.conf file installed ok";
			chmod 644 /usr/local/idns/named.conf;
		fi
		cWarnAboutNamedConf="1";
	fi
	#create all zone files
	/var/www/unxs/cgi-bin/iDNS.cgi allfiles master ns1.yourdomain.com 127.0.0.1 > /dev/null 2>&1;
	if [ $? == 0 ];then
		echo "allfiles ok";
		cAllfiles="1";
		/etc/init.d/unxsbind restart > /dev/null 2>&1
		if [ $? == 0 ];then
			echo "unxsbind restart ok";
			cUnxsBindStart="1";
		fi
	fi
	#rrdtool and tHit collector initializing
	if [ -x /usr/sbin/tHitCollector ];then
		#initialize main stats rrd
		/usr/sbin/tHitCollector Initialize --cZone allzone.stats > /dev/null 2>&1;
		if [ $? == 0 ];then
			echo "tHitCollector initialize ok";
		fi
		#new version of rrdtool needs fontconfig font, it was installed
		#but we need to load into cache
		if [ -x /usr/bin/fc-cache ];then
			/usr/bin/fc-cache > /dev/null 2>&1;
			if [ $? == 0 ];then
				echo "fc-cache ran ok";
			fi
		fi
		#setup main stats graph
		if [ -f /var/www/unxs/html/images/allzone.stats.png ] && [ -d /var/log/named ];then
			rm /var/www/unxs/html/images/allzone.stats.png;
			ln -s /var/log/named/allzone.stats.png /var/www/unxs/html/images/allzone.stats.png;
			if [ $? == 0 ];then
				echo "allzone.stats.png install ok";
			fi
		fi
	fi
	#let installer know what was done.
	if [ "$cUnxsBindStart" == "1" ] && [ "$cHttpdStart" == "1" ] && [ "$cMySQLStart" == "1" ] \
				&& [ "$cInitialize" == "1" ] && [ "$cAllfiles" == "1" ];then
		echo "unxsBind has been installed, intialized and httpd and named have been started.";	
		echo "You can proceed to login to your unxsBind interfaces with your browser.";	
		echo "You may need to change your iptables configuration (or run 'iptables -F'.)";	
	else 
			echo "IMPORTANT: It appears that one or more manual operations may be needed to finish";
			echo "your unxsBind installation.";
		if [ "$cUnxsBindStart" != "1" ]; then
			echo "";
			echo "WARNING: Your unxsBind named was not started, run:";	
			echo "named-checkconf /usr/local/idns/named.conf";
			echo "Fix any problems, then run:";
			echo "/etc/init.d/unxsbind start";
		fi
		if [ "$cHttpdStart" != "1" ]; then
			echo "";
			echo "WARNING: Your httpd server was not started, run:";
			echo "/etc/init.d/httpd configtest";
			echo "Then check your httpd configuration and then:";
			echo "/etc/init.d/httpd start";
		fi
		if [ "$cMySQLStart" != "1" ]; then
			echo "";
			echo "WARNING: Your mysqld server was not started, run:";
			echo "/etc/init.d/mysqld start";
			echo "Debug any problems, then, if you have not already set your MySQL root password:";
			echo "/usr/bin/mysqladmin -u root password '<mysql-root-passwd>'";	
		fi
		if [ "$cInitialize" != "1" ]; then
			echo "";
			echo "WARNING (if your MySQL already has valid iDNS data ignore):"
			echo "Your unxsBind database was not initialized, run:";
			echo "export ISMROOT=/usr/local/share";
			echo "/var/www/unxs/cgi-bin/iDNS.cgi Initialize <mysql-root-passwd>";	
			echo "Debug any problems, check via the mysql CLI, then if needed try again.";
		fi
		if [ "$cAllfiles" != "1" ]; then
			echo "";
			echo "WARNING: Initial zone file creation failed. Try running:"
			echo "/var/www/unxs/cgi-bin/iDNS.cgi allfiles master ns1.yourdomain.com 127.0.0.1";	
		fi
	fi
	#root crontab deprecated, managed via init.d unxsbind script for /etc/cron.d/unxsbind
	/bin/grep "iDNS.cgi" /var/spool/cron/root > /dev/null 2>&1;
	if [ $? == 0 ];then
		echo "";
		echo "WARNING: Placing unxsBind cron entries in the root crontab has been deprecated";
		echo "Please remove them all with 'crontab -e' and restart unxsbind via 'service unxsbind restart'";
	fi
	if [ "$cWarnAboutNamedConf" == "1" ];then
		echo "";
		echo "WARNING: You need to edit or create by hand your /usr/local/idns/named.conf ";
		echo "You may be able to run '/var/www/unxs/cgi-bin/iDNS.cgi installbind <NSIPv4>' to do this for you.";
	fi
elif [ "$1" = "2" ]; then
	#echo "post: Update";
	#update schema
	export ISMROOT=/usr/local/share;	
	if [ -x /var/www/unxs/cgi-bin/iDNS.cgi ];then
		/var/www/unxs/cgi-bin/iDNS.cgi UpdateSchema > /dev/null 2>&1;
		if [ $? == 0 ];then
			cUpdateSchema="1";
		fi
	fi
	#update tables (fixed type and template tables)
	if [ -x /var/www/unxs/cgi-bin/iDNS.cgi ];then
		/var/www/unxs/cgi-bin/iDNS.cgi UpdateTables > /dev/null 2>&1;
		if [ $? == 0 ];then
			cUpdateTables="1";
		fi
	fi
	#if for some reason named.conf is missing use default
	if [ ! -f /usr/local/idns/named.conf ];then
	cIP=`/sbin/ifconfig|/usr/bin/head -n 2|/usr/bin/tail -n 1|/bin/awk -F'inet addr:' '{print $2}'|/bin/cut -f 1 -d " "`;
		if [ $? != 0 ] || [ "$cIP" == "" ];then
			echo "Error geting cIP";
		else
			export ISMROOT=/usr/local/share;	
			#this will fail if mysql is not running.
			/var/www/unxs/cgi-bin/iDNS.cgi installbind $cIP > /dev/null 2>&1;
			if [ $? == 0 ];then
				echo "update iDNS.cgi installbind ok $cIP";
			else
				cp /usr/local/share/iDNS/setup9/named.conf.localhost /usr/local/idns/named.conf
				if [ $? == 0 ];then
					echo "update missing named.conf localhost file installed ok";
					chmod 644 /usr/local/idns/named.conf;
				fi
				cWarnAboutNamedConf="1";
			fi
		fi
	fi
	#create all zone files since we updated the db.
	if [ "$cUpdateSchema" == "1" ] && [ "$cUpdateTables" == "1" ];then
		/var/www/unxs/cgi-bin/iDNS.cgi allfiles master ns1.yourdomain.com 127.0.0.1 > /dev/null 2>&1;
		if [ $? == 0 ];then
			echo "allfiles update ok";
			cAllfiles="1";
			/etc/init.d/unxsbind restart > /dev/null 2>&1
			if [ $? == 0 ];then
				echo "unxsbind update restart ok";
				cUnxsBindStart="1";
			fi
		fi
	fi
	#let installer know what was done.
	if [ "$cUpdateSchema" == "1" ] && [ "$cUpdateTables" == "1" ];then
		echo "unxsBind progams have been updated, your MySQL schema and fixed table contents";	
		echo " have also been upgraded. Existing templates have been saved (see tTemplate.)";	
	else 
			echo "IMPORTANT: It appears that one or more manual operations may be needed to finish";
			echo "your unxsBind upgrade.";
			echo " Visit http://openisp.net/openisp/unxsVZ/wiki/InstallingBindYum for help.";
		if [ "$cUpdateSchema" != "1" ]; then
			echo "";
			echo "WARNING: Your unxsBind schema was not updated!";	
		fi
		if [ "$cUpdateTables" != "1" ]; then
			echo "";
			echo "WARNING: Your unxsBind fixed tables and templates have not been updated!";	
		fi
	fi
	#root crontab deprecated, managed via init.d unxsbind script for /etc/cron.d/unxsbind
	/bin/grep "iDNS.cgi" /var/spool/cron/root > /dev/null 2>&1;
	if [ $? == 0 ];then
		echo "";
		echo "WARNING: Placing unxsBind cron entries in the root crontab has been deprecated!";
		echo "Please remove them all with 'crontab -e' and restart unxsbind via 'service unxsbind restart'.";
	fi
	if [ "$cWarnAboutNamedConf" == "1" ];then
		echo "";
		echo "WARNING: You need to edit or create by hand your /usr/local/idns/named.conf ";
		echo "You may be able to run '/var/www/unxs/cgi-bin/iDNS.cgi installbind <NSIPv4>' to do this for you.";
	fi
fi

%preun
if [ "$1" = "0" ]; then
	echo "preun: Uninstall";
	/etc/init.d/unxsbind stop > /dev/null 2>&1;
	if [ $? == 0 ];then
		echo "unxsbind stop ok";
	fi
elif [ "$1" = "1" ]; then
	echo "preun: Update";
fi

%postun
if [ "$1" = "0" ]; then
	echo "postun: Uninstall";
elif [ "$1" = "1" ]; then
	echo "postun: Update";
	/etc/init.d/unxsbind restart > /dev/null 2>&1;
	if [ $? == 0 ];then
		echo "unxsbind restart ok";
	fi
fi

%clean


%changelog
* Tue Oct 26 2010 Gary Wallis <support@unixservice.com>
- Added AXFR import support and fixed some cNAME input validation bugs.
* Mon May 3 2010 Gary Wallis <support@unixservice.com>
- Correcting update path.
* Fri Apr 30 2010 Gary Wallis <support@unixservice.com>
- Fixing upgrade.
* Thu Apr 29 2010 Gary Wallis <support@unixservice.com>
- Fixed upgrade and requirements. Using new unxstemplate lib for interfaces. Corrected multiple DB server support. Corrected version number change. Added support for update and uninstall.
* Wed Mar 17 2010 Hugo Urquiza <support2@unixservice.com>
- New rpm release, added support for AAAA and NAPTR RRs, major version number change.
* Mon Aug 02 2009 Hugo Urquiza <support2@unixservice.com>
- Fixed directories permission and ownership, and init.d script minor update.
* Mon Jul 13 2009 Gary Wallis <support@unixservice.com>
- Adding allfiles master post install command
* Sun Jul 12 2009 Gary Wallis <support@unixservice.com>
- Fixed man install issues while adding install post script
* Sat Jul 11 2009 Gary Wallis <support@unixservice.com>
- Fixed conflict with BIND regarding /etc/rndc.key and added post install and rrdtool issue fixes
* Sat Jul 11 2009 Gary Wallis <support@unixservice.com>
- Added missing rndc.key and /etc/init.d/unxsbind among other related issues.
* Fri Jul 10 2009 Hugo Urquiza <support2@unixservice.com>
- More minor idnsOrg updates
* Fri Jul 10 2009 Gary Wallis <support@unixservice.com>
- Added config file directives for correct update behavior. This is under alpha checking for initial install and update.
* Thu Jul 09 2009 Hugo Urquiza <support2@unixservice.com>
- Added 'Delegation Tools' feature to idnsOrg. Online RR check at idnsOrg minor updates. Backend list filter queries updated.
* Thu Jul 09 2009 Gary Wallis <support@unixservice.com>
- iDNS.cgi CLI ImportZone command and tZoneImport and tResourceImport backend bug fixes part-2.
* Thu Jul 09 2009 Gary Wallis <support@unixservice.com>
- iDNS.cgi CLI ImportZone command and tZoneImport and tResourceImport backend bug fixes.
* Thu Jul 2 2009 Hugo Urquiza <support2@unixservice.com>
- Minor fixes.
* Fri Jun 26 2009 Hugo Urquiza <support2@unixservice.com>
- Fixed idnsOrg bulk importer bug.
* Tue Jun 16 2009 Hugo Urquiza <support2@unixservice.com>
- Backend 'Delegation Tools' update.
* Thu Jun 11 2009 Hugo Urquiza <support2@unixservice.com>
- Minor idnsAdmin and idnsOrg updates.
* Tue Jun 02 2009 Hugo Urquiza <support2@unixservice.com>
- Several fixes and updates
* Thu May 28 2009 Hugo Urquiza <support2@unixservice.com>
- Minor code fixes, added mysqlcluster.sh
* Mon May 18 2009 Hugo Urquiza <support2@unixservice.com>
- Added rrdtool as required package. Updated file list and mkdir section.
* Thu May 14 2009 Hugo Urquiza <support2@unixservice.com>
- Minor fixes and updates.
* Thu May 14 2009 Gary Wallis <support@unixservice.com> 
- Initial rpm release

