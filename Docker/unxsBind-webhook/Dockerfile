FROM centos:7
MAINTAINER docker@unxs.io

ENV container docker

#Prepare systemd
RUN (cd /lib/systemd/system/sysinit.target.wants/; for i in *; do [ $i == \
systemd-tmpfiles-setup.service ] || rm -f $i; done); \
rm -f /lib/systemd/system/multi-user.target.wants/*;\
rm -f /etc/systemd/system/*.wants/*;\
rm -f /lib/systemd/system/local-fs.target.wants/*; \
rm -f /lib/systemd/system/sockets.target.wants/*udev*; \
rm -f /lib/systemd/system/sockets.target.wants/*initctl*; \
rm -f /lib/systemd/system/basic.target.wants/*;\
rm -f /lib/systemd/system/anaconda.target.wants/*;
VOLUME [ "/sys/fs/cgroup" ]

#Install required packages
RUN yum -y install httpd; systemctl enable httpd.service
RUN yum -y install git gcc make mysql-devel openssl-devel
RUN yum -y install bind-utils
RUN yum -y install libtool-ltdl
RUN yum -y clean all
RUN systemctl enable httpd.service

#Start install
RUN mkdir /home/unxs
RUN mkdir -p /var/www/unxs/html
RUN mkdir -p /var/www/unxs/logs

#Modify httpd service
RUN rm -f  /etc/httpd/conf.modules.d/00-dav.conf;\
rm -f /etc/httpd/conf.modules.d/00-lua.conf;\
rm -f /etc/httpd/conf.modules.d/00-base.conf;\
rm -f /etc/httpd/conf.modules.d/00-proxy.conf;
COPY 00-base.conf /etc/httpd/conf.modules.d/00-base.conf
COPY webhook.httpd.conf /etc/httpd/conf.d/
RUN sed -i -e 's/^Listen 80/#Listen 80/g' /etc/httpd/conf/httpd.conf
RUN touch /var/www/unxs/logs/cgi.log;chown apache:apache /var/www/unxs/logs/cgi.log;

#Prepare link
RUN echo "ln -s /tmp/docker.sock /var/run/docker.sock;" >> /etc/rc.d/rc.local
RUN chmod +x /etc/rc.d/rc.local

CMD ["/usr/sbin/init"]

#Useful commands
#docker build -t unxsio/unxsbind-webhook .
#docker rm -f unxsbind-webhook;docker run --privileged --restart unless-stopped \
#	--volume /sys/fs/cgroup:/sys/fs/cgroup:ro \
#	--volume /usr/bin/docker:/usr/bin/docker \
#	--volume /var/run/docker.sock:/var/run/docker.sock \
#		--name unxsbind-webhook -p 8088:8088 -d unxsio/unxsbind-webhook
#