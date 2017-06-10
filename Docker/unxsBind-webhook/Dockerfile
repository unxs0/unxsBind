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
RUN yum -y clean all
RUN systemctl enable httpd.service

#Start install
RUN mkdir /home/unxs
RUN mkdir -p /var/www/unxs/html
RUN mkdir -p /var/www/unxs/logs

#Modify httpd service
COPY webhook.httpd.conf /etc/httpd/conf.d/
RUN sed -i -e 's/^Listen 80/#Listen 80/g' /etc/httpd/conf/httpd.conf

CMD ["/usr/sbin/init"]

#Useful commands
#docker build -t unxsio/unxsbind-webhook .
#docker rm -f unxsbind-webhook;docker run --privileged --restart unless-stopped --volume /sys/fs/cgroup:/sys/fs/cgroup:ro --name unxsbind-webhook -p 8088:8088 -d unxsio/unxsbind-webhook
#
