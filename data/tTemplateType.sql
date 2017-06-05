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
-- Table structure for table `tTemplateType`
--

DROP TABLE IF EXISTS `tTemplateType`;
CREATE TABLE `tTemplateType` (
  `uTemplateType` int(10) unsigned NOT NULL auto_increment,
  `cLabel` varchar(32) NOT NULL default '',
  `uOwner` int(10) unsigned NOT NULL default '0',
  `uCreatedBy` int(10) unsigned NOT NULL default '0',
  `uCreatedDate` int(10) unsigned NOT NULL default '0',
  `uModBy` int(10) unsigned NOT NULL default '0',
  `uModDate` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uTemplateType`),
  KEY `uOwner` (`uOwner`)
) ENGINE=MyISAM AUTO_INCREMENT=12 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tTemplateType`
--

LOCK TABLES `tTemplateType` WRITE;
/*!40000 ALTER TABLE `tTemplateType` DISABLE KEYS */;
INSERT INTO `tTemplateType` VALUES (1,'idnsOrg Interface',3,2,1163026214,1,1270815441),(2,'idnsAdmin Interface',3,2,1163026214,1,1270815457),(3,'vdnsOrg Interface',1,1,1270815416,1,1270815470);
/*!40000 ALTER TABLE `tTemplateType` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2010-04-27 22:30:28
