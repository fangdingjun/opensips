
INSERT INTO version (table_name, table_version) values ('license','1');
CREATE TABLE IF NOT EXISTS `license` (
      `i_id` int(11) NOT NULL AUTO_INCREMENT,
      `devid` varchar(64) DEFAULT NULL,
      `license` varchar(64) DEFAULT NULL,
      `activecode` varchar(64) DEFAULT NULL,
      `uuid` varchar(32) DEFAULT NULL,
      `ttl` int(11) DEFAULT NULL,
      `start_time` datetime DEFAULT NULL,
      PRIMARY KEY (`i_id`)
    ) ENGINE=MyISAM; 

    -- --------------------------------------------------------

--
-- Table structure for table `phonedevpaired`
--

INSERT INTO version (table_name, table_version) values ('phonedevpaired','1');
CREATE TABLE IF NOT EXISTS `phonedevpaired` (
      `id` int(11) NOT NULL AUTO_INCREMENT,
      `phoneid` varchar(50) DEFAULT NULL,
      `serial` varchar(20) DEFAULT NULL,
      `op_time` datetime DEFAULT NULL,
      PRIMARY KEY (`id`)
    ) ENGINE=MyISAM ;

