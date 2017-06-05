## unxsBind

The unxsVZ ISP DNS/IP Address management software has had
some limited success.

It does however need to be cleaned up. A lot! It is old, contains
very junky prototype code full of global vars and confusion.

Thanks to microservices technology it is has never been easier to
deliver stable complex multi server systems like unxsBind.

### Roadmap

 * Pull out of unxsVZ. Done!
 * Cleanup backend C CGI and DNS server agents.
 * Provide modern MEAN/MERN stack front ends.
 * Start DNSSEC support.
 * Start supporting dynamic updates.
 * Provide public API.
 * Provide DHCP management.
 * Provide more IPAM features especially regarding IPv6 space planning.

### Ubuntu

Required unxsBase repo action:

 * unxsBase/unxsCIDRLib: make install
 * unxsBase/unxsTemplateLib: make install
