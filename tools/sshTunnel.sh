#/bin/bash

#Setup a background tunnel to not have to deal with MySQL SSL certs etc.
#
#Note that the remote MySQL server will most likey need a GRANT ALL for 
#the userbelow@at-server-itself since MySQL will see the connection as TCP from
#it's own IP. And not use the socket.
#
#Note that this example will only work with ssh keygen setup pwd'less connections
#for the user in question. In this case root.
#

#We specifcally bind to 127.0.0.1 and not use IPv6
/usr/bin/ssh -4 -f -N -L 127.0.0.1:3307:node2vm:3306 root@node2vm
