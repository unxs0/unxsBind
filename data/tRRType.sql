-- MySQL dump 10.11
--
-- Host: localhost    Database: idns
-- ------------------------------------------------------
-- Server version	5.0.45-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `tRRType`
--

DROP TABLE IF EXISTS `tRRType`;
CREATE TABLE `tRRType` (
  `uRRType` int(10) unsigned NOT NULL auto_increment,
  `cLabel` varchar(32) NOT NULL default '',
  `uOwner` int(10) unsigned NOT NULL default '0',
  `uCreatedBy` int(10) unsigned NOT NULL default '0',
  `uCreatedDate` int(10) unsigned NOT NULL default '0',
  `uModBy` int(10) unsigned NOT NULL default '0',
  `uModDate` int(10) unsigned NOT NULL default '0',
  `uParam1` int(10) unsigned NOT NULL default '0',
  `uParam2` int(10) unsigned NOT NULL default '0',
  `cParam2Func` varchar(32) NOT NULL default '',
  `cParam1Func` varchar(32) NOT NULL default '',
  `cParam1Label` varchar(32) NOT NULL default '',
  `cParam2Label` varchar(32) NOT NULL default '',
  `cParam1Tip` varchar(100) NOT NULL default '',
  `cParam2Tip` varchar(100) NOT NULL default '',
  `cNameLabel` varchar(32) NOT NULL default '',
  `cNameTip` varchar(100) NOT NULL default '',
  `cNameFunc` varchar(32) NOT NULL default '',
  `uName` int(10) unsigned NOT NULL default '0',
  `cParam3Func` varchar(32) NOT NULL default '',
  `cParam4Func` varchar(32) NOT NULL default '',
  `cParam3Label` varchar(32) NOT NULL default '',
  `cParam4Label` varchar(32) NOT NULL default '',
  `cParam3Tip` varchar(100) NOT NULL default '',
  `cParam4Tip` varchar(100) NOT NULL default '',
  `uParam3` int(10) unsigned NOT NULL default '0',
  `uParam4` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uRRType`),
  UNIQUE KEY `cLabel` (`cLabel`),
  KEY `uOwner` (`uOwner`)
) ENGINE=MyISAM AUTO_INCREMENT=22 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tRRType`
--

LOCK TABLES `tRRType` WRITE;
/*!40000 ALTER TABLE `tRRType` DISABLE KEYS */;
INSERT INTO `tRRType` VALUES (1,'A',1,1,1164326458,1,1267802432,1,0,'','IpNumber','IP Number','','IPv4 dotted quad IP number, e.g. 200.32.12.109','','Resource Name','If empty is the zone, if partial the zone is appended, if fully qualified must end in zone name.','RRName1',1,'','','','','','',0,0),(2,'NS',1,1,1164327003,1,1267802536,1,0,'','FQNameServer','Name Server','','Fully qualified name server. E.g. ns1.isp.net.','','Resource Name','If empty is the zone, if partial the zone is appended, if fully qualified must end in zone name.','RRName1',1,'','','','','','',0,0),(3,'MX',1,1,1164327172,1,1267802617,1,1,'MxPreference','FQDomainName','Preference Number','Mail Server','A number. Lower is higher priority. Ex. 10','Fully qualified mail server. Ex. mail.isp.net.','Resource Name','If empty is the zone, if partial the zone is appended, if fully qualified must end in zone name.','RRName1',1,'','','','','','',0,0),(4,'HINFO',1,1,1164327316,1,1267802660,1,1,'HardwareType','OperatingSystem','Operating System','Hardware Type','Ex1. Solaris Ex2. Linux CentOS','Ex1. Quad Xeon Ex2. Sun Blade 2000','Resource Name','If empty is the zone, if partial the zone is appended, if fully qualified must end in zone name.','RRName1',1,'','','','','','',0,0),(5,'CNAME',1,1,1164327455,1,1267802720,1,0,'','FQCanonicalName','Canonical Name','','Usually the FQDN zone name or another FQDN zone domain.','','Resource Name','If empty is the zone, if partial the zone is appended, if fully qualified must end in zone name.','RRName1',1,'','','','','','',0,0),(6,'TXT',1,1,1164327557,1,1267802765,1,0,'','TextRecord','Text in double quotes','','Ex. \"v=spf1 mx ~all\"','','Resource Name','If empty is the zone, if partial the zone is appended, if fully qualified must end in zone name.','RRName1',1,'','','','','','',0,0),(7,'PTR',1,1,1164327757,1,1267802853,1,0,'','FQDomainName','Domain Name','','A FQDN. Ex. bart.isp.net.','','PTR Name','Usually the last octet of an IP number of a class C arpa zone.','ValidPTRName',1,'','','','','','',0,0),(8,'SRV',1,1,1195137855,1,1267802262,1,1,'Number','Number','Priority','Weight','The priority of the target host, lower value means more preferred.','A relative weight for records with the same priority.','Service','The symbolic name of the desired service. (E.g.: _sip._tcp.example.com.)','Service',1,'Port','Target','Port','Target','Port number of service.','The FQDN canonical hostname of the server providing the service.',1,1),(9,'AAAA',1,1,1267715002,1,1267802902,1,0,'','IpNumberv6','IPv6 Number','','IPv6 colon hex IP number. Must have 7 colons.','','Resource Name','If empty is the zone, if partial the zone is appended, if fully qualified must end in zone name.','RRName1',1,'','','','','','',0,0),(10,'NAPTR',1,1,1267801662,1,1267803686,1,1,'Number','Number','Order','Preference','Order number. Ex. 100','Preference number for same order if exists. Ex. 10','e164.arpa Origin or FQDN','Ex1: 4.3.2.1.5.5.5.0.0.8.1.e164.arpa. for tel:+1-800-555-1234. Ex2: example.com.','RRNameNAPTR',1,'Enum','NAPTRRegex','Flags+ENUM','Regex (+optional SRV)','Ex1: \"S\" \"SIP+D2U\" Ex2: \"U\" \"E2U+sip\"','E.g. \"!^.*$!sip:customer-service@example.com!\" _sip._udp.example.com',1,1);
/*!40000 ALTER TABLE `tRRType` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2010-04-26  9:59:10
