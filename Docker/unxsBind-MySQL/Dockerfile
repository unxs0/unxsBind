FROM centurylink/mysql
MAINTAINER docker@unxs.io

ENV MYSQL_ROOT_PASSWORD doesnotmatter
ENV MYSQL_DATABASE idns
ENV MYSQL_USER idns
ENV MYSQL_PASSWORD wsxedc

#useful commands:
#docker build -t unxsio/unxsbind-mysql .

#docker rm -f unxsbind-mysql;docker run --restart unless-stopped --name unxsbind-mysql --volume /var/lib/mysql:/var/lib/mysql -p 3306:3306 -d unxsio/unxsbind-mysql

#docker build -t unxsio/unxsbind-mysql .;docker rm -f unxsbind-mysql;docker-compose -f ../unxsbind.docker-compose.yml up -d;
