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
-- Table structure for table `tTemplateSet`
--

DROP TABLE IF EXISTS `tTemplateSet`;
CREATE TABLE `tTemplateSet` (
  `uTemplateSet` int(10) unsigned NOT NULL auto_increment,
  `cLabel` varchar(32) NOT NULL default '',
  `uOwner` int(10) unsigned NOT NULL default '0',
  `uCreatedBy` int(10) unsigned NOT NULL default '0',
  `uCreatedDate` int(10) unsigned NOT NULL default '0',
  `uModBy` int(10) unsigned NOT NULL default '0',
  `uModDate` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uTemplateSet`),
  KEY `uOwner` (`uOwner`)
) ENGINE=MyISAM AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tTemplateSet`
--

LOCK TABLES `tTemplateSet` WRITE;
/*!40000 ALTER TABLE `tTemplateSet` DISABLE KEYS */;
INSERT INTO `tTemplateSet` VALUES (1,'Plain',1,1,1163026243,0,0);
/*!40000 ALTER TABLE `tTemplateSet` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2010-04-27 22:30:39
