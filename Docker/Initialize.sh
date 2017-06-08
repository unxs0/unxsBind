#!/bin/bash

docker exec -ti unxsbind-mysql mysql -pdoesnotmatter -e "drop database idns";
docker exec -ti unxsbind ln -s /home/unxs/unxsBind /home/unxs/iDNS;
docker exec -ti -e ISMROOT=/home/unxs unxsbind ./iDNS.cgi Initialize doesnotmatter;
