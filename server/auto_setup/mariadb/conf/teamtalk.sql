/*
Navicat MySQL Data Transfer

Source Server         : 192.168.2.128
Source Server Version : 50639
Source Host           : 192.168.2.128:3306
Source Database       : teamtalk

Target Server Type    : MYSQL
Target Server Version : 50639
File Encoding         : 65001

Date: 2018-04-21 14:43:01
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for IMAdmin
-- ----------------------------
DROP TABLE IF EXISTS `IMAdmin`;
CREATE TABLE `IMAdmin` (
  `id` mediumint(6) unsigned NOT NULL AUTO_INCREMENT,
  `uname` varchar(40) NOT NULL COMMENT '用户名',
  `pwd` char(32) NOT NULL COMMENT '密码',
  `status` tinyint(2) unsigned NOT NULL DEFAULT '0' COMMENT '用户状态 0 :正常 1:删除 可扩展',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间´',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间´',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of IMAdmin
-- ----------------------------
INSERT INTO `IMAdmin` VALUES ('1', 'admin', '21232f297a57a5a743894a0e4a801fc3', '0', '0', '0');

-- ----------------------------
-- Table structure for IMAudio
-- ----------------------------
DROP TABLE IF EXISTS `IMAudio`;
CREATE TABLE `IMAudio` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `fromId` int(11) unsigned NOT NULL COMMENT '发送者Id',
  `toId` int(11) unsigned NOT NULL COMMENT '接收者Id',
  `path` varchar(255) COLLATE utf8_general_ci DEFAULT '' COMMENT '语音存储的地址',
  `size` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '文件大小',
  `duration` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '语音时长',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_fromId_toId` (`fromId`,`toId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- ----------------------------
-- Records of IMAudio
-- ----------------------------

-- ----------------------------
-- Table structure for IMDepart
-- ----------------------------
DROP TABLE IF EXISTS `IMDepart`;
CREATE TABLE `IMDepart` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT '部门id',
  `departName` varchar(64) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '部门名称',
  `priority` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '显示优先级',
  `parentId` int(11) unsigned NOT NULL COMMENT '上级部门id',
  `status` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '状态',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  `updated` int(11) unsigned NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `idx_departName` (`departName`),
  KEY `idx_priority_status` (`priority`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- ----------------------------
-- Records of IMDepart
-- ----------------------------
INSERT INTO `IMDepart` VALUES ('1', 'dev', '0', '0', '0', '1519904880', '1519904880');

-- ----------------------------
-- Table structure for IMDiscovery
-- ----------------------------
DROP TABLE IF EXISTS `IMDiscovery`;
CREATE TABLE `IMDiscovery` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT 'id',
  `itemName` varchar(64) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '名称',
  `itemUrl` varchar(64) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT 'URL',
  `itemPriority` int(11) unsigned NOT NULL COMMENT '显示优先级',
  `status` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '状态',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  `updated` int(11) unsigned NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `idx_itemName` (`itemName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- ----------------------------
-- Records of IMDiscovery
-- ----------------------------

-- ----------------------------
-- Table structure for IMFriend
-- ----------------------------
DROP TABLE IF EXISTS `IMFriend`;
CREATE TABLE `IMFriend` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `fromId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `toId` int(11) unsigned NOT NULL COMMENT '接收用户的id',
  `created` int(11) DEFAULT NULL COMMENT '好友创建的时间',
  `updated` int(11) DEFAULT NULL COMMENT '好友的时间',
  `status` int(11) DEFAULT NULL COMMENT '代理状态 0:正常(互为好友) 1:to未同意 2:from已删除好友 3:to已删除好友 4:fromto互删好友',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=29 DEFAULT CHARSET=utf8 COMMENT='好友表';

-- ----------------------------
-- Records of IMFriend
-- ----------------------------
INSERT INTO `IMFriend` VALUES ('27', '0', '30', '1522067591', '1522067609', '0');
INSERT INTO `IMFriend` VALUES ('28', '1', '2', '1522067609', '1522292718', '0');

-- ----------------------------
-- Table structure for IMGroup
-- ----------------------------
DROP TABLE IF EXISTS `IMGroup`;
CREATE TABLE `IMGroup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(256) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '群名称',
  `avatar` varchar(256) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '群头像',
  `creator` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建者用户id',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT '群组类型，1-固定;2-临时群',
  `userCnt` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '成员人数',
  `status` tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT '是否删除,0-正常，1-删除',
  `version` int(11) unsigned NOT NULL DEFAULT '1' COMMENT '群版本号',
  `lastChated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '最后聊天时间',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_name` (`name`(191)),
  KEY `idx_creator` (`creator`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci COMMENT='IM群信息';

-- ----------------------------
-- Records of IMGroup
-- ----------------------------
INSERT INTO `IMGroup` VALUES ('1', 'f f g', '', '31', '2', '4', '0', '1', '1522122860', '1522121801', '1522121801');

-- ----------------------------
-- Table structure for IMGroupMember
-- ----------------------------
DROP TABLE IF EXISTS `IMGroupMember`;
CREATE TABLE `IMGroupMember` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `groupId` int(11) unsigned NOT NULL COMMENT '群Id',
  `userId` int(11) unsigned NOT NULL COMMENT '用户id',
  `status` tinyint(4) unsigned NOT NULL DEFAULT '1' COMMENT '是否退出群，0-正常，1-已退出',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `idx_groupId_userId_status` (`groupId`,`userId`,`status`),
  KEY `idx_userId_status_updated` (`userId`,`status`,`updated`),
  KEY `idx_groupId_updated` (`groupId`,`updated`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COMMENT='用户和群的关系表';

-- ----------------------------
-- Records of IMGroupMember
-- ----------------------------
INSERT INTO `IMGroupMember` VALUES ('1', '1', '29', '0', '1522121801', '1522121801');
INSERT INTO `IMGroupMember` VALUES ('2', '1', '31', '0', '1522121801', '1522121801');

-- ----------------------------
-- Table structure for IMGroupMessage_0
-- ----------------------------
DROP TABLE IF EXISTS `IMGroupMessage_0`;
CREATE TABLE `IMGroupMessage_0` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `groupId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `userId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '消息内容',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '2' COMMENT '群消息类型,101为群语音,2为文本',
  `status` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '消息状态',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_groupId_status_created` (`groupId`,`status`,`created`),
  KEY `idx_groupId_msgId_status_created` (`groupId`,`msgId`,`status`,`created`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci COMMENT='IM群消息表';

-- ----------------------------
-- Records of IMGroupMessage_0
-- ----------------------------

-- ----------------------------
-- Table structure for IMGroupMessage_1
-- ----------------------------
DROP TABLE IF EXISTS `IMGroupMessage_1`;
CREATE TABLE `IMGroupMessage_1` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `groupId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `userId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '消息内容',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '2' COMMENT '群消息类型,101为群语音,2为文本',
  `status` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '消息状态',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_groupId_status_created` (`groupId`,`status`,`created`),
  KEY `idx_groupId_msgId_status_created` (`groupId`,`msgId`,`status`,`created`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci COMMENT='IM群消息表';

-- ----------------------------
-- Records of IMGroupMessage_1
-- ----------------------------
INSERT INTO `IMGroupMessage_1` VALUES ('1', '1', '31', '1', 'ozG/6bzne6/cnc8vmtwBBg==', '17', '0', '1522121807', '1522121807');
INSERT INTO `IMGroupMessage_1` VALUES ('2', '1', '31', '2', 'NgzeLv+T/SwL7mQlZP3W3w==', '17', '0', '1522122860', '1522122860');

-- ----------------------------
-- Table structure for IMGroupMessage_2
-- ----------------------------
DROP TABLE IF EXISTS `IMGroupMessage_2`;
CREATE TABLE `IMGroupMessage_2` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `groupId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `userId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '消息内容',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '2' COMMENT '群消息类型,101为群语音,2为文本',
  `status` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '消息状态',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_groupId_status_created` (`groupId`,`status`,`created`),
  KEY `idx_groupId_msgId_status_created` (`groupId`,`msgId`,`status`,`created`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci COMMENT='IM群消息表';

-- ----------------------------
-- Records of IMGroupMessage_2
-- ----------------------------

-- ----------------------------
-- Table structure for IMGroupMessage_3
-- ----------------------------
DROP TABLE IF EXISTS `IMGroupMessage_3`;
CREATE TABLE `IMGroupMessage_3` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `groupId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `userId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '消息内容',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '2' COMMENT '群消息类型,101为群语音,2为文本',
  `status` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '消息状态',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_groupId_status_created` (`groupId`,`status`,`created`),
  KEY `idx_groupId_msgId_status_created` (`groupId`,`msgId`,`status`,`created`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci COMMENT='IM群消息表';

-- ----------------------------
-- Records of IMGroupMessage_3
-- ----------------------------

-- ----------------------------
-- Table structure for IMGroupMessage_4
-- ----------------------------
DROP TABLE IF EXISTS `IMGroupMessage_4`;
CREATE TABLE `IMGroupMessage_4` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `groupId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `userId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '消息内容',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '2' COMMENT '群消息类型,101为群语音,2为文本',
  `status` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '消息状态',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_groupId_status_created` (`groupId`,`status`,`created`),
  KEY `idx_groupId_msgId_status_created` (`groupId`,`msgId`,`status`,`created`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci COMMENT='IM群消息表';

-- ----------------------------
-- Records of IMGroupMessage_4
-- ----------------------------

-- ----------------------------
-- Table structure for IMGroupMessage_5
-- ----------------------------
DROP TABLE IF EXISTS `IMGroupMessage_5`;
CREATE TABLE `IMGroupMessage_5` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `groupId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `userId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '消息内容',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '2' COMMENT '群消息类型,101为群语音,2为文本',
  `status` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '消息状态',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_groupId_status_created` (`groupId`,`status`,`created`),
  KEY `idx_groupId_msgId_status_created` (`groupId`,`msgId`,`status`,`created`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci COMMENT='IM群消息表';

-- ----------------------------
-- Records of IMGroupMessage_5
-- ----------------------------

-- ----------------------------
-- Table structure for IMGroupMessage_6
-- ----------------------------
DROP TABLE IF EXISTS `IMGroupMessage_6`;
CREATE TABLE `IMGroupMessage_6` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `groupId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `userId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '消息内容',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '2' COMMENT '群消息类型,101为群语音,2为文本',
  `status` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '消息状态',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_groupId_status_created` (`groupId`,`status`,`created`),
  KEY `idx_groupId_msgId_status_created` (`groupId`,`msgId`,`status`,`created`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci COMMENT='IM群消息表';

-- ----------------------------
-- Records of IMGroupMessage_6
-- ----------------------------

-- ----------------------------
-- Table structure for IMGroupMessage_7
-- ----------------------------
DROP TABLE IF EXISTS `IMGroupMessage_7`;
CREATE TABLE `IMGroupMessage_7` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `groupId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `userId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '消息内容',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '2' COMMENT '群消息类型,101为群语音,2为文本',
  `status` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '消息状态',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_groupId_status_created` (`groupId`,`status`,`created`),
  KEY `idx_groupId_msgId_status_created` (`groupId`,`msgId`,`status`,`created`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci COMMENT='IM群消息表';

-- ----------------------------
-- Records of IMGroupMessage_7
-- ----------------------------

-- ----------------------------
-- Table structure for IMMessage_0
-- ----------------------------
DROP TABLE IF EXISTS `IMMessage_0`;
CREATE TABLE `IMMessage_0` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `relateId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `fromId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `toId` int(11) unsigned NOT NULL COMMENT '接收用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci DEFAULT '' COMMENT '消息内容',
  `type` tinyint(2) unsigned NOT NULL DEFAULT '1' COMMENT '消息类型',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0正常 1被删除',
  `updated` int(11) unsigned NOT NULL COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_relateId_status_created` (`relateId`,`status`,`created`),
  KEY `idx_relateId_status_msgId_created` (`relateId`,`status`,`msgId`,`created`),
  KEY `idx_fromId_toId_created` (`fromId`,`toId`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- ----------------------------
-- Records of IMMessage_0
-- ----------------------------
INSERT INTO `IMMessage_0` VALUES ('1', '8', '1013', '1001', '1', 'h+PUkhRXNu2bcmcrisCTYw==', '1', '0', '1522830944', '1522830944');
INSERT INTO `IMMessage_0` VALUES ('2', '8', '1013', '1001', '2', 'vDsp1W/U5StCFTyskFb2dw==', '1', '0', '1522831009', '1522831009');
INSERT INTO `IMMessage_0` VALUES ('3', '8', '1013', '1001', '3', 'brMcxcmVDxcd1IAs1z4LsXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522831016', '1522831016');
INSERT INTO `IMMessage_0` VALUES ('4', '8', '1013', '1001', '4', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522831136', '1522831136');
INSERT INTO `IMMessage_0` VALUES ('5', '16', '1003', '1007', '1', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1523433858', '1523433858');
INSERT INTO `IMMessage_0` VALUES ('6', '24', '1014', '1018', '1', 'Z5IbzvsWpO+4QXnIKbHlKg==', '1', '0', '1523605280', '1523605280');
INSERT INTO `IMMessage_0` VALUES ('7', '24', '1014', '1018', '2', 'u4Ennt6fccLCaysLZkmO+g==', '1', '0', '1523605293', '1523605293');
INSERT INTO `IMMessage_0` VALUES ('8', '32', '1022', '1020', '1', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221823', '1524221823');
INSERT INTO `IMMessage_0` VALUES ('9', '32', '1022', '1020', '2', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221823', '1524221823');
INSERT INTO `IMMessage_0` VALUES ('10', '32', '1020', '1022', '3', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524221828', '1524221828');
INSERT INTO `IMMessage_0` VALUES ('11', '32', '1020', '1022', '4', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221830', '1524221830');

-- ----------------------------
-- Table structure for IMMessage_1
-- ----------------------------
DROP TABLE IF EXISTS `IMMessage_1`;
CREATE TABLE `IMMessage_1` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `relateId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `fromId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `toId` int(11) unsigned NOT NULL COMMENT '接收用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci DEFAULT '' COMMENT '消息内容',
  `type` tinyint(2) unsigned NOT NULL DEFAULT '1' COMMENT '消息类型',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0正常 1被删除',
  `updated` int(11) unsigned NOT NULL COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_relateId_status_created` (`relateId`,`status`,`created`),
  KEY `idx_relateId_status_msgId_created` (`relateId`,`status`,`msgId`,`created`),
  KEY `idx_fromId_toId_created` (`fromId`,`toId`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=29 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- ----------------------------
-- Records of IMMessage_1
-- ----------------------------
INSERT INTO `IMMessage_1` VALUES ('1', '1', '2', '1', '1', 'uPowL/sAvS8ediVd0A+lhA==', '1', '0', '1519908349', '1519908349');
INSERT INTO `IMMessage_1` VALUES ('2', '1', '2', '1', '2', '4NoRfLgQT5y0thiVkRk6qQ==', '1', '0', '1519908355', '1519908355');
INSERT INTO `IMMessage_1` VALUES ('3', '1', '2', '1', '3', 'kh4Y5k3itimgDex4Yn/B0Q==', '1', '0', '1522208369', '1522208369');
INSERT INTO `IMMessage_1` VALUES ('4', '1', '2', '1', '4', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208373', '1522208373');
INSERT INTO `IMMessage_1` VALUES ('5', '1', '2', '1', '5', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1522208376', '1522208376');
INSERT INTO `IMMessage_1` VALUES ('6', '1', '2', '1', '6', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208376', '1522208376');
INSERT INTO `IMMessage_1` VALUES ('7', '1', '2', '1', '7', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208377', '1522208377');
INSERT INTO `IMMessage_1` VALUES ('8', '1', '2', '1', '8', '8JPkwJBnwQMitiGrTbeX2HWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208378', '1522208378');
INSERT INTO `IMMessage_1` VALUES ('9', '1', '2', '1', '9', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208378', '1522208378');
INSERT INTO `IMMessage_1` VALUES ('10', '1', '2', '1', '10', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208379', '1522208379');
INSERT INTO `IMMessage_1` VALUES ('11', '1', '2', '1', '11', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208380', '1522208380');
INSERT INTO `IMMessage_1` VALUES ('12', '1', '2', '1', '12', 'bFkYRtErdXd/IxbUWaRKpnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208380', '1522208380');
INSERT INTO `IMMessage_1` VALUES ('13', '1', '2', '1', '13', 'zL6M+McU0nfiklCiWjAkEHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208380', '1522208380');
INSERT INTO `IMMessage_1` VALUES ('14', '1', '2', '1', '14', '0MoseN2wiZ3+h2GlBkAxYhLyLS1oMWHobm8prY23DtU=', '1', '0', '1522208382', '1522208382');
INSERT INTO `IMMessage_1` VALUES ('15', '1', '2', '1', '15', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208382', '1522208382');
INSERT INTO `IMMessage_1` VALUES ('16', '1', '2', '1', '16', 'yPi1udbJkJvn+R1KCyeZuXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208383', '1522208383');
INSERT INTO `IMMessage_1` VALUES ('17', '1', '2', '1', '17', 'hsGV8BY6ZqHwe28fic46xA==', '1', '0', '1522208405', '1522208405');
INSERT INTO `IMMessage_1` VALUES ('18', '1', '2', '1', '18', 'AntIYWW768J0ZjuuE7itw1JT2JkOXFZ9QbQQd3aOxf71ucSUlvz05CaBbOBm0tk1GM93i6GHJvxTQMTT3/V3bOfAg8aiXEQ8ysm7oFgY8Shp7c+7Jymgl86RyVMpLVrhuHL6sbAvVB2/nexQhQUMy4wTsq2U0f4+5f4LlbCz6kV4YRoZ9CaO9/ipXzOh/5TSOyfwgr4vupzcnMWdwVlLd21NiufG2v+K22wOGBWfnHY5VcAHmtgw2h90HHRLm+Tc', '1', '0', '1522208421', '1522208421');
INSERT INTO `IMMessage_1` VALUES ('19', '1', '2', '1', '19', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522208577', '1522208577');
INSERT INTO `IMMessage_1` VALUES ('20', '9', '1014', '1001', '1', 's07SepWFCn2yQgXMcGZSBA==', '1', '0', '1522838887', '1522838887');
INSERT INTO `IMMessage_1` VALUES ('21', '9', '1014', '1001', '2', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522838889', '1522838889');
INSERT INTO `IMMessage_1` VALUES ('22', '9', '1014', '1001', '3', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1522838890', '1522838890');
INSERT INTO `IMMessage_1` VALUES ('23', '17', '1003', '1011', '1', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523434219', '1523434219');
INSERT INTO `IMMessage_1` VALUES ('24', '17', '1003', '1011', '2', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523506706', '1523506706');
INSERT INTO `IMMessage_1` VALUES ('25', '9', '1014', '1001', '1', '{\n   \"t_currency\" : 100,\n   \"t_msgtype\" : 3,\n   \"t_number\" : 1,\n   \"t_openrule\" : 1,\n   \"t_text\" : \"\\\"s\\\"\"\n}\n', '3', '0', '1523531468', '1523531468');
INSERT INTO `IMMessage_1` VALUES ('26', '9', '1014', '1001', '2', '{\n   \"t_currency\" : 100,\n   \"t_msgtype\" : 3,\n   \"t_number\" : 1,\n   \"t_openrule\" : 1,\n   \"t_text\" : \"\\\"s\\\"\"\n}\n', '3', '0', '1523531510', '1523531510');
INSERT INTO `IMMessage_1` VALUES ('27', '9', '1014', '1001', '3', '{\n   \"t_currency\" : 100,\n   \"t_msgtype\" : 3,\n   \"t_number\" : 1,\n   \"t_openrule\" : 1,\n   \"t_text\" : \"\\\"s\\\"\"\n}\n', '3', '0', '1523531699', '1523531699');
INSERT INTO `IMMessage_1` VALUES ('28', '25', '1014', '1015', '1', 'L20vyOMXsEAh3Zvo66/O6Q==', '1', '0', '1523605307', '1523605307');

-- ----------------------------
-- Table structure for IMMessage_2
-- ----------------------------
DROP TABLE IF EXISTS `IMMessage_2`;
CREATE TABLE `IMMessage_2` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `relateId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `fromId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `toId` int(11) unsigned NOT NULL COMMENT '接收用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci DEFAULT '' COMMENT '消息内容',
  `type` tinyint(2) unsigned NOT NULL DEFAULT '1' COMMENT '消息类型',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0正常 1被删除',
  `updated` int(11) unsigned NOT NULL COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_relateId_status_created` (`relateId`,`status`,`created`),
  KEY `idx_relateId_status_msgId_created` (`relateId`,`status`,`msgId`,`created`),
  KEY `idx_fromId_toId_created` (`fromId`,`toId`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=20 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- ----------------------------
-- Records of IMMessage_2
-- ----------------------------
INSERT INTO `IMMessage_2` VALUES ('1', '2', '4', '2', '1', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1520411936', '1520411936');
INSERT INTO `IMMessage_2` VALUES ('2', '2', '2', '4', '2', '2pEs2w9+WAoO3WAlgJWvgQ==', '1', '0', '1520412210', '1520412210');
INSERT INTO `IMMessage_2` VALUES ('3', '2', '4', '2', '3', 'ZmG6tT1YNFwkkD2Vp0U/Lw==', '1', '0', '1520493677', '1520493677');
INSERT INTO `IMMessage_2` VALUES ('4', '10', '1014', '1007', '1', 'Oi/GuQt7bPoTRdgTJ2HCvA==', '1', '0', '1522838901', '1522838901');
INSERT INTO `IMMessage_2` VALUES ('5', '10', '1014', '1007', '1', 'KkQ5JONmhIOhuTSIVsHzWu46tX+iioVIPt+/ysC+rik=', '1', '0', '1523271673', '1523271673');
INSERT INTO `IMMessage_2` VALUES ('6', '18', '1003', '1001', '1', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523434550', '1523434550');
INSERT INTO `IMMessage_2` VALUES ('7', '18', '1003', '1001', '2', '0ZqlK5iMZJzyis70LFUMzw==', '1', '0', '1523438631', '1523438631');
INSERT INTO `IMMessage_2` VALUES ('8', '18', '1003', '1001', '3', '+B7wwjQDOi70omx2J0S35g==', '1', '0', '1523438636', '1523438636');
INSERT INTO `IMMessage_2` VALUES ('9', '26', '1003', '1008', '1', 'LMH030sys/+wQAjUPiUCKA==', '1', '0', '1524043891', '1524043891');
INSERT INTO `IMMessage_2` VALUES ('10', '34', '1023', '1019', '1', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222117', '1524222117');
INSERT INTO `IMMessage_2` VALUES ('11', '34', '1023', '1019', '2', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222118', '1524222118');
INSERT INTO `IMMessage_2` VALUES ('12', '34', '1023', '1019', '3', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222118', '1524222118');
INSERT INTO `IMMessage_2` VALUES ('13', '34', '1023', '1019', '4', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222118', '1524222118');
INSERT INTO `IMMessage_2` VALUES ('14', '34', '1023', '1019', '5', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222118', '1524222118');
INSERT INTO `IMMessage_2` VALUES ('15', '34', '1023', '1019', '6', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222118', '1524222118');
INSERT INTO `IMMessage_2` VALUES ('16', '34', '1023', '1019', '7', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222119', '1524222119');
INSERT INTO `IMMessage_2` VALUES ('17', '34', '1023', '1019', '8', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222119', '1524222119');
INSERT INTO `IMMessage_2` VALUES ('18', '34', '1023', '1019', '9', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222119', '1524222119');
INSERT INTO `IMMessage_2` VALUES ('19', '34', '1023', '1019', '10', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222119', '1524222119');

-- ----------------------------
-- Table structure for IMMessage_3
-- ----------------------------
DROP TABLE IF EXISTS `IMMessage_3`;
CREATE TABLE `IMMessage_3` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `relateId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `fromId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `toId` int(11) unsigned NOT NULL COMMENT '接收用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci DEFAULT '' COMMENT '消息内容',
  `type` tinyint(2) unsigned NOT NULL DEFAULT '1' COMMENT '消息类型',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0正常 1被删除',
  `updated` int(11) unsigned NOT NULL COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_relateId_status_created` (`relateId`,`status`,`created`),
  KEY `idx_relateId_status_msgId_created` (`relateId`,`status`,`msgId`,`created`),
  KEY `idx_fromId_toId_created` (`fromId`,`toId`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=140 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- ----------------------------
-- Records of IMMessage_3
-- ----------------------------
INSERT INTO `IMMessage_3` VALUES ('1', '3', '4', '3', '1', 'mzxZ5THiRdXr3V8LaacQzQ==', '1', '0', '1520495409', '1520495409');
INSERT INTO `IMMessage_3` VALUES ('2', '11', '1003', '1015', '1', 'brMcxcmVDxcd1IAs1z4LsXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523354132', '1523354132');
INSERT INTO `IMMessage_3` VALUES ('3', '11', '1003', '1015', '2', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523443554', '1523443554');
INSERT INTO `IMMessage_3` VALUES ('4', '11', '1003', '1015', '3', 'R7ZIri94ksqx1vvNbkIuOw==', '1', '0', '1523443780', '1523443780');
INSERT INTO `IMMessage_3` VALUES ('5', '19', '1016', '1006', '1', 'OA2SsqCp7fTyWkR/daO7/w==', '1', '0', '1523603415', '1523603415');
INSERT INTO `IMMessage_3` VALUES ('6', '11', '1003', '1015', '4', '0AUBswJvBILXt+jNEHmhuQ==', '1', '0', '1523852512', '1523852512');
INSERT INTO `IMMessage_3` VALUES ('7', '27', '1003', '1014', '1', 'ecuWtLVhL//svPR9nRF2Fw==', '1', '0', '1524134582', '1524134582');
INSERT INTO `IMMessage_3` VALUES ('8', '27', '1014', '1003', '2', 'Gte9e6bTzaLMbVQrBbH3nw==', '1', '0', '1524136955', '1524136955');
INSERT INTO `IMMessage_3` VALUES ('9', '27', '1014', '1003', '3', 'n0JLep0iQjxh+gORf/Uanw==', '1', '0', '1524137525', '1524137525');
INSERT INTO `IMMessage_3` VALUES ('10', '27', '1003', '1014', '4', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524213506', '1524213506');
INSERT INTO `IMMessage_3` VALUES ('11', '35', '1023', '1021', '2', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222255', '1524222255');
INSERT INTO `IMMessage_3` VALUES ('12', '35', '1023', '1021', '3', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222255', '1524222255');
INSERT INTO `IMMessage_3` VALUES ('13', '35', '1023', '1021', '4', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222255', '1524222255');
INSERT INTO `IMMessage_3` VALUES ('14', '35', '1023', '1021', '5', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222255', '1524222255');
INSERT INTO `IMMessage_3` VALUES ('15', '35', '1023', '1021', '6', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222255', '1524222255');
INSERT INTO `IMMessage_3` VALUES ('16', '35', '1023', '1021', '7', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222256', '1524222256');
INSERT INTO `IMMessage_3` VALUES ('17', '35', '1023', '1021', '8', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222256', '1524222256');
INSERT INTO `IMMessage_3` VALUES ('18', '35', '1023', '1021', '9', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222256', '1524222256');
INSERT INTO `IMMessage_3` VALUES ('19', '35', '1023', '1021', '10', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222256', '1524222256');
INSERT INTO `IMMessage_3` VALUES ('20', '35', '1023', '1021', '11', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222256', '1524222256');
INSERT INTO `IMMessage_3` VALUES ('21', '35', '1023', '1021', '12', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222256', '1524222256');
INSERT INTO `IMMessage_3` VALUES ('22', '35', '1023', '1021', '13', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222257', '1524222257');
INSERT INTO `IMMessage_3` VALUES ('23', '35', '1023', '1021', '14', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222257', '1524222257');
INSERT INTO `IMMessage_3` VALUES ('24', '35', '1023', '1021', '15', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222257', '1524222257');
INSERT INTO `IMMessage_3` VALUES ('25', '35', '1023', '1021', '16', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222257', '1524222257');
INSERT INTO `IMMessage_3` VALUES ('26', '35', '1023', '1021', '17', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222257', '1524222257');
INSERT INTO `IMMessage_3` VALUES ('27', '35', '1023', '1021', '18', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_3` VALUES ('28', '35', '1023', '1021', '19', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_3` VALUES ('29', '35', '1023', '1021', '20', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_3` VALUES ('30', '35', '1023', '1021', '21', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_3` VALUES ('31', '35', '1023', '1021', '22', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_3` VALUES ('32', '35', '1023', '1021', '23', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_3` VALUES ('33', '35', '1023', '1021', '24', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_3` VALUES ('34', '35', '1023', '1021', '25', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222259', '1524222259');
INSERT INTO `IMMessage_3` VALUES ('35', '35', '1023', '1021', '26', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222259', '1524222259');
INSERT INTO `IMMessage_3` VALUES ('36', '35', '1023', '1021', '27', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222260', '1524222260');
INSERT INTO `IMMessage_3` VALUES ('37', '35', '1023', '1021', '28', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222260', '1524222260');
INSERT INTO `IMMessage_3` VALUES ('38', '35', '1023', '1021', '29', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222260', '1524222260');
INSERT INTO `IMMessage_3` VALUES ('39', '35', '1023', '1021', '30', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222261', '1524222261');
INSERT INTO `IMMessage_3` VALUES ('40', '35', '1023', '1021', '31', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222261', '1524222261');
INSERT INTO `IMMessage_3` VALUES ('41', '35', '1023', '1021', '32', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222261', '1524222261');
INSERT INTO `IMMessage_3` VALUES ('42', '35', '1023', '1021', '33', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222263', '1524222263');
INSERT INTO `IMMessage_3` VALUES ('43', '35', '1023', '1021', '34', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222263', '1524222263');
INSERT INTO `IMMessage_3` VALUES ('44', '35', '1023', '1021', '35', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222263', '1524222263');
INSERT INTO `IMMessage_3` VALUES ('45', '35', '1023', '1021', '36', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222263', '1524222263');
INSERT INTO `IMMessage_3` VALUES ('46', '35', '1023', '1021', '37', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222264', '1524222264');
INSERT INTO `IMMessage_3` VALUES ('47', '35', '1023', '1021', '38', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222264', '1524222264');
INSERT INTO `IMMessage_3` VALUES ('48', '35', '1023', '1021', '39', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222265', '1524222265');
INSERT INTO `IMMessage_3` VALUES ('49', '35', '1023', '1021', '40', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222265', '1524222265');
INSERT INTO `IMMessage_3` VALUES ('50', '35', '1023', '1021', '41', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222265', '1524222265');
INSERT INTO `IMMessage_3` VALUES ('51', '35', '1023', '1021', '42', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222266', '1524222266');
INSERT INTO `IMMessage_3` VALUES ('52', '35', '1023', '1021', '43', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222266', '1524222266');
INSERT INTO `IMMessage_3` VALUES ('53', '35', '1023', '1021', '44', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222266', '1524222266');
INSERT INTO `IMMessage_3` VALUES ('54', '35', '1023', '1021', '45', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222266', '1524222266');
INSERT INTO `IMMessage_3` VALUES ('55', '35', '1023', '1021', '46', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222266', '1524222266');
INSERT INTO `IMMessage_3` VALUES ('56', '35', '1023', '1021', '47', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222266', '1524222266');
INSERT INTO `IMMessage_3` VALUES ('57', '35', '1023', '1021', '48', 'brMcxcmVDxcd1IAs1z4LsXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222267', '1524222267');
INSERT INTO `IMMessage_3` VALUES ('58', '35', '1023', '1021', '49', 'brMcxcmVDxcd1IAs1z4LsXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222267', '1524222267');
INSERT INTO `IMMessage_3` VALUES ('59', '35', '1023', '1021', '50', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222267', '1524222267');
INSERT INTO `IMMessage_3` VALUES ('60', '35', '1023', '1021', '51', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222267', '1524222267');
INSERT INTO `IMMessage_3` VALUES ('61', '35', '1023', '1021', '52', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222267', '1524222267');
INSERT INTO `IMMessage_3` VALUES ('62', '35', '1023', '1021', '53', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222269', '1524222269');
INSERT INTO `IMMessage_3` VALUES ('63', '35', '1023', '1021', '54', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222269', '1524222269');
INSERT INTO `IMMessage_3` VALUES ('64', '35', '1023', '1021', '55', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222270', '1524222270');
INSERT INTO `IMMessage_3` VALUES ('65', '35', '1023', '1021', '56', 'JyC1OKfCqoYoHEtSDas1ZBLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222270', '1524222270');
INSERT INTO `IMMessage_3` VALUES ('66', '35', '1023', '1021', '57', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222270', '1524222270');
INSERT INTO `IMMessage_3` VALUES ('67', '35', '1023', '1021', '58', 'qhNqCFdnwq2CTnAZHFBZKHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222270', '1524222270');
INSERT INTO `IMMessage_3` VALUES ('68', '35', '1023', '1021', '59', 'qhNqCFdnwq2CTnAZHFBZKHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222270', '1524222270');
INSERT INTO `IMMessage_3` VALUES ('69', '35', '1023', '1021', '60', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222271', '1524222271');
INSERT INTO `IMMessage_3` VALUES ('70', '35', '1023', '1021', '61', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222271', '1524222271');
INSERT INTO `IMMessage_3` VALUES ('71', '35', '1023', '1021', '62', 'JyC1OKfCqoYoHEtSDas1ZBLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222271', '1524222271');
INSERT INTO `IMMessage_3` VALUES ('72', '35', '1023', '1021', '63', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222272', '1524222272');
INSERT INTO `IMMessage_3` VALUES ('73', '35', '1023', '1021', '64', 'zL6M+McU0nfiklCiWjAkEHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222272', '1524222272');
INSERT INTO `IMMessage_3` VALUES ('74', '35', '1023', '1021', '65', 'zL6M+McU0nfiklCiWjAkEHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222272', '1524222272');
INSERT INTO `IMMessage_3` VALUES ('75', '35', '1023', '1021', '66', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222272', '1524222272');
INSERT INTO `IMMessage_3` VALUES ('76', '35', '1023', '1021', '67', 'qhNqCFdnwq2CTnAZHFBZKHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222273', '1524222273');
INSERT INTO `IMMessage_3` VALUES ('77', '35', '1023', '1021', '68', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222273', '1524222273');
INSERT INTO `IMMessage_3` VALUES ('78', '35', '1023', '1021', '69', '0MoseN2wiZ3+h2GlBkAxYhLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222277', '1524222277');
INSERT INTO `IMMessage_3` VALUES ('79', '35', '1023', '1021', '70', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222277', '1524222277');
INSERT INTO `IMMessage_3` VALUES ('80', '35', '1023', '1021', '71', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222277', '1524222277');
INSERT INTO `IMMessage_3` VALUES ('81', '35', '1023', '1021', '72', '0MoseN2wiZ3+h2GlBkAxYhLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222278', '1524222278');
INSERT INTO `IMMessage_3` VALUES ('82', '35', '1023', '1021', '73', '0MoseN2wiZ3+h2GlBkAxYhLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222278', '1524222278');
INSERT INTO `IMMessage_3` VALUES ('83', '35', '1023', '1021', '74', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222278', '1524222278');
INSERT INTO `IMMessage_3` VALUES ('84', '35', '1023', '1021', '75', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222278', '1524222278');
INSERT INTO `IMMessage_3` VALUES ('85', '35', '1023', '1021', '76', '0MoseN2wiZ3+h2GlBkAxYhLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222279', '1524222279');
INSERT INTO `IMMessage_3` VALUES ('86', '35', '1023', '1021', '77', '0MoseN2wiZ3+h2GlBkAxYhLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222279', '1524222279');
INSERT INTO `IMMessage_3` VALUES ('87', '35', '1023', '1021', '78', '0MoseN2wiZ3+h2GlBkAxYhLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222279', '1524222279');
INSERT INTO `IMMessage_3` VALUES ('88', '35', '1023', '1021', '79', 'qhNqCFdnwq2CTnAZHFBZKHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222281', '1524222281');
INSERT INTO `IMMessage_3` VALUES ('89', '35', '1023', '1021', '80', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222281', '1524222281');
INSERT INTO `IMMessage_3` VALUES ('90', '35', '1023', '1021', '81', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222281', '1524222281');
INSERT INTO `IMMessage_3` VALUES ('91', '35', '1023', '1021', '82', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222281', '1524222281');
INSERT INTO `IMMessage_3` VALUES ('92', '35', '1023', '1021', '83', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222282', '1524222282');
INSERT INTO `IMMessage_3` VALUES ('93', '35', '1023', '1021', '84', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222282', '1524222282');
INSERT INTO `IMMessage_3` VALUES ('94', '35', '1023', '1021', '85', 'qhNqCFdnwq2CTnAZHFBZKHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222282', '1524222282');
INSERT INTO `IMMessage_3` VALUES ('95', '35', '1023', '1021', '86', 'qhNqCFdnwq2CTnAZHFBZKHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222282', '1524222282');
INSERT INTO `IMMessage_3` VALUES ('96', '35', '1023', '1021', '87', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222282', '1524222282');
INSERT INTO `IMMessage_3` VALUES ('97', '35', '1023', '1021', '88', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222283', '1524222283');
INSERT INTO `IMMessage_3` VALUES ('98', '35', '1023', '1021', '89', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222283', '1524222283');
INSERT INTO `IMMessage_3` VALUES ('99', '35', '1023', '1021', '90', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222283', '1524222283');
INSERT INTO `IMMessage_3` VALUES ('100', '35', '1023', '1021', '91', 'qhNqCFdnwq2CTnAZHFBZKHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222283', '1524222283');
INSERT INTO `IMMessage_3` VALUES ('101', '35', '1023', '1021', '92', 'qhNqCFdnwq2CTnAZHFBZKHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222284', '1524222284');
INSERT INTO `IMMessage_3` VALUES ('102', '35', '1023', '1021', '93', 'JyC1OKfCqoYoHEtSDas1ZBLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222284', '1524222284');
INSERT INTO `IMMessage_3` VALUES ('103', '35', '1023', '1021', '94', 'qhNqCFdnwq2CTnAZHFBZKHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222285', '1524222285');
INSERT INTO `IMMessage_3` VALUES ('104', '35', '1023', '1021', '95', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222285', '1524222285');
INSERT INTO `IMMessage_3` VALUES ('105', '35', '1023', '1021', '96', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222285', '1524222285');
INSERT INTO `IMMessage_3` VALUES ('106', '35', '1023', '1021', '97', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222286', '1524222286');
INSERT INTO `IMMessage_3` VALUES ('107', '35', '1023', '1021', '98', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222286', '1524222286');
INSERT INTO `IMMessage_3` VALUES ('108', '35', '1023', '1021', '99', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222286', '1524222286');
INSERT INTO `IMMessage_3` VALUES ('109', '35', '1023', '1021', '100', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222286', '1524222286');
INSERT INTO `IMMessage_3` VALUES ('110', '35', '1023', '1021', '101', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222287', '1524222287');
INSERT INTO `IMMessage_3` VALUES ('111', '35', '1023', '1021', '102', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222287', '1524222287');
INSERT INTO `IMMessage_3` VALUES ('112', '35', '1023', '1021', '103', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222287', '1524222287');
INSERT INTO `IMMessage_3` VALUES ('113', '35', '1023', '1021', '104', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222287', '1524222287');
INSERT INTO `IMMessage_3` VALUES ('114', '35', '1023', '1021', '105', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222287', '1524222287');
INSERT INTO `IMMessage_3` VALUES ('115', '35', '1023', '1021', '106', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222288', '1524222288');
INSERT INTO `IMMessage_3` VALUES ('116', '35', '1023', '1021', '107', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222288', '1524222288');
INSERT INTO `IMMessage_3` VALUES ('117', '35', '1023', '1021', '108', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222288', '1524222288');
INSERT INTO `IMMessage_3` VALUES ('118', '35', '1023', '1021', '109', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222288', '1524222288');
INSERT INTO `IMMessage_3` VALUES ('119', '35', '1023', '1021', '110', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222288', '1524222288');
INSERT INTO `IMMessage_3` VALUES ('120', '35', '1023', '1021', '111', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222289', '1524222289');
INSERT INTO `IMMessage_3` VALUES ('121', '35', '1023', '1021', '112', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222289', '1524222289');
INSERT INTO `IMMessage_3` VALUES ('122', '35', '1023', '1021', '113', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222289', '1524222289');
INSERT INTO `IMMessage_3` VALUES ('123', '35', '1023', '1021', '114', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222289', '1524222289');
INSERT INTO `IMMessage_3` VALUES ('124', '35', '1023', '1021', '115', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222289', '1524222289');
INSERT INTO `IMMessage_3` VALUES ('125', '35', '1023', '1021', '116', '8JPkwJBnwQMitiGrTbeX2HWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222290', '1524222290');
INSERT INTO `IMMessage_3` VALUES ('126', '35', '1023', '1021', '117', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222290', '1524222290');
INSERT INTO `IMMessage_3` VALUES ('127', '35', '1023', '1021', '118', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222290', '1524222290');
INSERT INTO `IMMessage_3` VALUES ('128', '35', '1023', '1021', '119', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222290', '1524222290');
INSERT INTO `IMMessage_3` VALUES ('129', '35', '1023', '1021', '120', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222291', '1524222291');
INSERT INTO `IMMessage_3` VALUES ('130', '35', '1023', '1021', '121', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222291', '1524222291');
INSERT INTO `IMMessage_3` VALUES ('131', '35', '1023', '1021', '122', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222291', '1524222291');
INSERT INTO `IMMessage_3` VALUES ('132', '35', '1023', '1021', '123', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222291', '1524222291');
INSERT INTO `IMMessage_3` VALUES ('133', '35', '1023', '1021', '124', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222291', '1524222291');
INSERT INTO `IMMessage_3` VALUES ('134', '35', '1023', '1021', '125', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222292', '1524222292');
INSERT INTO `IMMessage_3` VALUES ('135', '35', '1023', '1021', '126', 'brMcxcmVDxcd1IAs1z4LsXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222292', '1524222292');
INSERT INTO `IMMessage_3` VALUES ('136', '35', '1023', '1021', '127', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222292', '1524222292');
INSERT INTO `IMMessage_3` VALUES ('137', '35', '1023', '1021', '128', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222292', '1524222292');
INSERT INTO `IMMessage_3` VALUES ('138', '35', '1023', '1021', '129', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222293', '1524222293');
INSERT INTO `IMMessage_3` VALUES ('139', '35', '1023', '1021', '130', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222293', '1524222293');

-- ----------------------------
-- Table structure for IMMessage_4
-- ----------------------------
DROP TABLE IF EXISTS `IMMessage_4`;
CREATE TABLE `IMMessage_4` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `relateId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `fromId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `toId` int(11) unsigned NOT NULL COMMENT '接收用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci DEFAULT '' COMMENT '消息内容',
  `type` tinyint(2) unsigned NOT NULL DEFAULT '1' COMMENT '消息类型',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0正常 1被删除',
  `updated` int(11) unsigned NOT NULL COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_relateId_status_created` (`relateId`,`status`,`created`),
  KEY `idx_relateId_status_msgId_created` (`relateId`,`status`,`msgId`,`created`),
  KEY `idx_fromId_toId_created` (`fromId`,`toId`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=20 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- ----------------------------
-- Records of IMMessage_4
-- ----------------------------
INSERT INTO `IMMessage_4` VALUES ('1', '4', '31', '29', '1', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522121087', '1522121087');
INSERT INTO `IMMessage_4` VALUES ('2', '4', '29', '31', '2', 'TKZhC/j3j4wHNwmy3ba5jw==', '1', '0', '1522121314', '1522121314');
INSERT INTO `IMMessage_4` VALUES ('3', '4', '31', '29', '4', 'uKW7CmG9m/KWhevXoV1vHQ==', '1', '0', '1522121574', '1522121574');
INSERT INTO `IMMessage_4` VALUES ('4', '4', '31', '29', '6', 'NgzeLv+T/SwL7mQlZP3W3w==', '1', '0', '1522122844', '1522122844');
INSERT INTO `IMMessage_4` VALUES ('5', '12', '1003', '1006', '1', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523433041', '1523433041');
INSERT INTO `IMMessage_4` VALUES ('6', '12', '1003', '1006', '2', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523434286', '1523434286');
INSERT INTO `IMMessage_4` VALUES ('7', '12', '1003', '1006', '3', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523435285', '1523435285');
INSERT INTO `IMMessage_4` VALUES ('8', '20', '1014', '1009', '1', 'pE924uvf89lS+LhXCt7EDw==', '1', '0', '1523603547', '1523603547');
INSERT INTO `IMMessage_4` VALUES ('9', '28', '1019', '1020', '1', 'zaM5Pdbzt0pDOgE+4lr6CA==', '1', '0', '1524221070', '1524221070');
INSERT INTO `IMMessage_4` VALUES ('10', '28', '1020', '1019', '2', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221077', '1524221077');
INSERT INTO `IMMessage_4` VALUES ('11', '36', '1024', '1022', '1', 'brMcxcmVDxcd1IAs1z4LsXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222390', '1524222390');
INSERT INTO `IMMessage_4` VALUES ('12', '36', '1024', '1022', '2', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222427', '1524222427');
INSERT INTO `IMMessage_4` VALUES ('13', '36', '1022', '1024', '3', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222458', '1524222458');
INSERT INTO `IMMessage_4` VALUES ('14', '36', '1022', '1024', '4', '0MoseN2wiZ3+h2GlBkAxYhLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222492', '1524222492');
INSERT INTO `IMMessage_4` VALUES ('15', '36', '1022', '1024', '5', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222496', '1524222496');
INSERT INTO `IMMessage_4` VALUES ('16', '36', '1022', '1024', '6', 'yPi1udbJkJvn+R1KCyeZuXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222497', '1524222497');
INSERT INTO `IMMessage_4` VALUES ('17', '36', '1022', '1024', '7', '0MoseN2wiZ3+h2GlBkAxYhLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222501', '1524222501');
INSERT INTO `IMMessage_4` VALUES ('18', '28', '1020', '1019', '4', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524277997', '1524277997');
INSERT INTO `IMMessage_4` VALUES ('19', '28', '1020', '1019', '5', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524277999', '1524277999');

-- ----------------------------
-- Table structure for IMMessage_5
-- ----------------------------
DROP TABLE IF EXISTS `IMMessage_5`;
CREATE TABLE `IMMessage_5` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `relateId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `fromId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `toId` int(11) unsigned NOT NULL COMMENT '接收用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci DEFAULT '' COMMENT '消息内容',
  `type` tinyint(2) unsigned NOT NULL DEFAULT '1' COMMENT '消息类型',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0正常 1被删除',
  `updated` int(11) unsigned NOT NULL COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_relateId_status_created` (`relateId`,`status`,`created`),
  KEY `idx_relateId_status_msgId_created` (`relateId`,`status`,`msgId`,`created`),
  KEY `idx_fromId_toId_created` (`fromId`,`toId`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=24 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- ----------------------------
-- Records of IMMessage_5
-- ----------------------------
INSERT INTO `IMMessage_5` VALUES ('1', '5', '1001', '1000', '1', 'GgsgEVwZikNaYDya8iQmWw==', '1', '0', '1522640740', '1522640740');
INSERT INTO `IMMessage_5` VALUES ('2', '5', '1001', '1000', '2', 'p/vc/vcetpPmdztRYPgqJQ==', '1', '0', '1522640747', '1522640747');
INSERT INTO `IMMessage_5` VALUES ('3', '5', '1001', '1000', '3', 'PmHAdHi9ijDK1BJe2FLs2U33x1nPUgLRYJLaelB48C96XaUHBQog7LusYHMJbzzm', '1', '0', '1522640758', '1522640758');
INSERT INTO `IMMessage_5` VALUES ('4', '5', '1001', '1000', '4', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522640765', '1522640765');
INSERT INTO `IMMessage_5` VALUES ('5', '5', '1001', '1000', '5', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1522640770', '1522640770');
INSERT INTO `IMMessage_5` VALUES ('6', '5', '1001', '1000', '6', 'JEucBW/eogbCtUPpFWIfDQ==', '1', '0', '1522663584', '1522663584');
INSERT INTO `IMMessage_5` VALUES ('7', '5', '1001', '1000', '7', 'uj1c6jnpafDhrss5tKlRjw==', '1', '0', '1522663593', '1522663593');
INSERT INTO `IMMessage_5` VALUES ('8', '5', '1001', '1000', '8', 'yqrIMDJBLcaMxxyvdgU4PA==', '1', '0', '1522721271', '1522721271');
INSERT INTO `IMMessage_5` VALUES ('9', '5', '1001', '1000', '10', 'dMQDvfdu9UPtvVlDXdexgA==', '1', '0', '1522752268', '1522752268');
INSERT INTO `IMMessage_5` VALUES ('10', '5', '1001', '1000', '11', '3OX1RjLuPWwrMr1nmm9KTw==', '1', '0', '1522752384', '1522752384');
INSERT INTO `IMMessage_5` VALUES ('11', '5', '1000', '1001', '12', 'n0JLep0iQjxh+gORf/Uanw==', '1', '0', '1522812199', '1522812199');
INSERT INTO `IMMessage_5` VALUES ('12', '5', '1001', '1000', '13', 'qXCy1fT2IquSF74nLqImRA==', '1', '0', '1522833352', '1522833352');
INSERT INTO `IMMessage_5` VALUES ('13', '13', '1003', '1010', '1', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523433674', '1523433674');
INSERT INTO `IMMessage_5` VALUES ('14', '13', '1003', '1010', '2', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523433850', '1523433850');
INSERT INTO `IMMessage_5` VALUES ('15', '21', '1014', '1012', '1', 'B+HuUVixIt9S2wq8XgAH9A==', '1', '0', '1523603598', '1523603598');
INSERT INTO `IMMessage_5` VALUES ('16', '21', '1014', '1012', '2', 'gg6zvGGF42JeU1OqzPTG+Q==', '1', '0', '1523603833', '1523603833');
INSERT INTO `IMMessage_5` VALUES ('17', '29', '1022', '1019', '1', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221777', '1524221777');
INSERT INTO `IMMessage_5` VALUES ('18', '29', '1022', '1019', '2', 'brMcxcmVDxcd1IAs1z4LsXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221778', '1524221778');
INSERT INTO `IMMessage_5` VALUES ('19', '29', '1022', '1019', '3', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221778', '1524221778');
INSERT INTO `IMMessage_5` VALUES ('20', '29', '1022', '1019', '4', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524221778', '1524221778');
INSERT INTO `IMMessage_5` VALUES ('21', '29', '1022', '1019', '5', 'hTCf9/bYTPozUZsKxwg6CnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221779', '1524221779');
INSERT INTO `IMMessage_5` VALUES ('22', '29', '1022', '1019', '6', 'Lb4EK6cv+MoSEar8GFYMO3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221779', '1524221779');
INSERT INTO `IMMessage_5` VALUES ('23', '29', '1022', '1019', '7', 'VXjm7p2JeJnYHQCdF+BE3nWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221779', '1524221779');

-- ----------------------------
-- Table structure for IMMessage_6
-- ----------------------------
DROP TABLE IF EXISTS `IMMessage_6`;
CREATE TABLE `IMMessage_6` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `relateId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `fromId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `toId` int(11) unsigned NOT NULL COMMENT '接收用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci DEFAULT '' COMMENT '消息内容',
  `type` tinyint(2) unsigned NOT NULL DEFAULT '1' COMMENT '消息类型',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0正常 1被删除',
  `updated` int(11) unsigned NOT NULL COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_relateId_status_created` (`relateId`,`status`,`created`),
  KEY `idx_relateId_status_msgId_created` (`relateId`,`status`,`msgId`,`created`),
  KEY `idx_fromId_toId_created` (`fromId`,`toId`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=238 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- ----------------------------
-- Records of IMMessage_6
-- ----------------------------
INSERT INTO `IMMessage_6` VALUES ('1', '6', '1012', '1000', '1', 'W8tKH2fhcVxVHAG24e9ajQ==', '1', '0', '1522827427', '1522827427');
INSERT INTO `IMMessage_6` VALUES ('2', '14', '1003', '1012', '1', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523433809', '1523433809');
INSERT INTO `IMMessage_6` VALUES ('3', '14', '1003', '1012', '2', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523434534', '1523434534');
INSERT INTO `IMMessage_6` VALUES ('4', '22', '1014', '1017', '1', 'e2Uh3GwP45QTWcO/tH9ZjA==', '1', '0', '1523604463', '1523604463');
INSERT INTO `IMMessage_6` VALUES ('5', '22', '1017', '1014', '2', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1523604476', '1523604476');
INSERT INTO `IMMessage_6` VALUES ('6', '22', '1014', '1017', '3', 'n0JLep0iQjxh+gORf/Uanw==', '1', '0', '1523604476', '1523604476');
INSERT INTO `IMMessage_6` VALUES ('7', '22', '1014', '1017', '4', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523604479', '1523604479');
INSERT INTO `IMMessage_6` VALUES ('8', '22', '1014', '1017', '5', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1523604483', '1523604483');
INSERT INTO `IMMessage_6` VALUES ('9', '22', '1014', '1017', '6', '8hW3lPSkW4IXsySAcK2B9A==', '1', '0', '1523604487', '1523604487');
INSERT INTO `IMMessage_6` VALUES ('10', '22', '1017', '1014', '7', 'mHBtVXP0o30C9AMOM/XGVg==', '1', '0', '1523604487', '1523604487');
INSERT INTO `IMMessage_6` VALUES ('11', '22', '1017', '1014', '8', 'GWovb33xuqoGFMmrCy6ulw==', '1', '0', '1523604489', '1523604489');
INSERT INTO `IMMessage_6` VALUES ('12', '22', '1014', '1017', '9', 'PMAaAWot6At7+Fy0aPQjyg==', '1', '0', '1523604491', '1523604491');
INSERT INTO `IMMessage_6` VALUES ('13', '22', '1017', '1014', '10', '0r0ANKUNzkBYq9tti/XtbA==', '1', '0', '1523604491', '1523604491');
INSERT INTO `IMMessage_6` VALUES ('14', '22', '1014', '1017', '11', 'tAPthIA84peToIYQNKNvUA==', '1', '0', '1523604496', '1523604496');
INSERT INTO `IMMessage_6` VALUES ('15', '22', '1014', '1017', '12', 'nojpYky+dhrd8/mQmsMLVw==', '1', '0', '1523604499', '1523604499');
INSERT INTO `IMMessage_6` VALUES ('16', '22', '1014', '1017', '13', 'iLNX8WK72vxGQHhyUCLBlA==', '1', '0', '1523604502', '1523604502');
INSERT INTO `IMMessage_6` VALUES ('17', '22', '1014', '1017', '14', 'S4OGhWWcoIz1FKPo15Kf8A==', '1', '0', '1523604643', '1523604643');
INSERT INTO `IMMessage_6` VALUES ('18', '22', '1014', '1017', '15', 'PMAaAWot6At7+Fy0aPQjyg==', '1', '0', '1523604774', '1523604774');
INSERT INTO `IMMessage_6` VALUES ('19', '22', '1017', '1014', '16', 'J+CtlxP3L/8rP0vTzMG8UQ==', '1', '0', '1523604786', '1523604786');
INSERT INTO `IMMessage_6` VALUES ('20', '22', '1017', '1014', '17', 'P3ZCAqb8obUX33WRgv7duA==', '1', '0', '1523604788', '1523604788');
INSERT INTO `IMMessage_6` VALUES ('21', '22', '1017', '1014', '18', 'rsMWzZrO6CW3C8IuJOP74A==', '1', '0', '1523604789', '1523604789');
INSERT INTO `IMMessage_6` VALUES ('22', '22', '1017', '1014', '19', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523604795', '1523604795');
INSERT INTO `IMMessage_6` VALUES ('23', '22', '1014', '1017', '20', 'qx7xiXRlJ8yuYLmCXphRT1zIdZRUsFN9eOwYZegap80=', '1', '0', '1523604799', '1523604799');
INSERT INTO `IMMessage_6` VALUES ('24', '22', '1014', '1017', '21', 'gABtfcN7nik2vawEQxEwHw==', '1', '0', '1523604846', '1523604846');
INSERT INTO `IMMessage_6` VALUES ('25', '22', '1014', '1017', '22', 'tkJQ/Mx4j3kTA7oIDmN5Bw==', '1', '0', '1523604849', '1523604849');
INSERT INTO `IMMessage_6` VALUES ('26', '22', '1014', '1017', '23', 't2TeDxdo0YIVufabJVZxrQ==', '1', '0', '1523604852', '1523604852');
INSERT INTO `IMMessage_6` VALUES ('27', '22', '1017', '1014', '24', 'q629Z2fo9SKdOVoxpOPDzA==', '1', '0', '1523604853', '1523604853');
INSERT INTO `IMMessage_6` VALUES ('28', '22', '1017', '1014', '25', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606681', '1523606681');
INSERT INTO `IMMessage_6` VALUES ('29', '22', '1017', '1014', '26', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606682', '1523606682');
INSERT INTO `IMMessage_6` VALUES ('30', '22', '1017', '1014', '27', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606682', '1523606682');
INSERT INTO `IMMessage_6` VALUES ('31', '22', '1017', '1014', '28', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606683', '1523606683');
INSERT INTO `IMMessage_6` VALUES ('32', '22', '1017', '1014', '29', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606683', '1523606683');
INSERT INTO `IMMessage_6` VALUES ('33', '22', '1017', '1014', '30', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606683', '1523606683');
INSERT INTO `IMMessage_6` VALUES ('34', '22', '1017', '1014', '31', 'bFkYRtErdXd/IxbUWaRKpnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606684', '1523606684');
INSERT INTO `IMMessage_6` VALUES ('35', '22', '1017', '1014', '32', 'bFkYRtErdXd/IxbUWaRKpnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606684', '1523606684');
INSERT INTO `IMMessage_6` VALUES ('36', '22', '1017', '1014', '33', 'vjhoCUd8Dvr88PbTOqGn0nWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606684', '1523606684');
INSERT INTO `IMMessage_6` VALUES ('37', '22', '1017', '1014', '34', 'vjhoCUd8Dvr88PbTOqGn0nWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606685', '1523606685');
INSERT INTO `IMMessage_6` VALUES ('38', '22', '1017', '1014', '35', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606685', '1523606685');
INSERT INTO `IMMessage_6` VALUES ('39', '22', '1017', '1014', '36', 'vjhoCUd8Dvr88PbTOqGn0nWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606685', '1523606685');
INSERT INTO `IMMessage_6` VALUES ('40', '22', '1017', '1014', '37', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606685', '1523606685');
INSERT INTO `IMMessage_6` VALUES ('41', '22', '1017', '1014', '38', 'JyC1OKfCqoYoHEtSDas1ZBLyLS1oMWHobm8prY23DtU=', '1', '0', '1523606685', '1523606685');
INSERT INTO `IMMessage_6` VALUES ('42', '22', '1017', '1014', '39', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606685', '1523606685');
INSERT INTO `IMMessage_6` VALUES ('43', '22', '1017', '1014', '40', 'zL6M+McU0nfiklCiWjAkEHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606686', '1523606686');
INSERT INTO `IMMessage_6` VALUES ('44', '22', '1017', '1014', '41', 'qhNqCFdnwq2CTnAZHFBZKHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606686', '1523606686');
INSERT INTO `IMMessage_6` VALUES ('45', '22', '1017', '1014', '42', 'zL6M+McU0nfiklCiWjAkEHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606686', '1523606686');
INSERT INTO `IMMessage_6` VALUES ('46', '22', '1017', '1014', '43', 'JyC1OKfCqoYoHEtSDas1ZBLyLS1oMWHobm8prY23DtU=', '1', '0', '1523606686', '1523606686');
INSERT INTO `IMMessage_6` VALUES ('47', '22', '1017', '1014', '44', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523606686', '1523606686');
INSERT INTO `IMMessage_6` VALUES ('48', '22', '1014', '1017', '45', 'n0JLep0iQjxh+gORf/Uanw==', '1', '0', '1523606937', '1523606937');
INSERT INTO `IMMessage_6` VALUES ('49', '22', '1014', '1017', '46', 'U9iXg+GEhSenmAYZC5JJkg==', '1', '0', '1523606948', '1523606948');
INSERT INTO `IMMessage_6` VALUES ('50', '22', '1017', '1014', '47', 'J+CtlxP3L/8rP0vTzMG8UQ==', '1', '0', '1523606948', '1523606948');
INSERT INTO `IMMessage_6` VALUES ('51', '22', '1014', '1017', '48', 'lpGssLHydIoqzKO9CmZdBw==', '1', '0', '1523606950', '1523606950');
INSERT INTO `IMMessage_6` VALUES ('52', '22', '1017', '1014', '49', 'zbhrmHhdaIOgsOxhcpw/5Zp3w0tMTFulcAIRUouwpy8=', '1', '0', '1523606951', '1523606951');
INSERT INTO `IMMessage_6` VALUES ('53', '22', '1014', '1017', '50', 'lWAyEkUMoeg/RKqyOCQWxVzIdZRUsFN9eOwYZegap80=', '1', '0', '1523606952', '1523606952');
INSERT INTO `IMMessage_6` VALUES ('54', '22', '1017', '1014', '51', 'DRZaZ5Q7j/WzbobiYU0lkw==', '1', '0', '1523606954', '1523606954');
INSERT INTO `IMMessage_6` VALUES ('55', '22', '1014', '1017', '52', 'ng9U0vYMRb0sqiDs1CT2AQ==', '1', '0', '1523606956', '1523606956');
INSERT INTO `IMMessage_6` VALUES ('56', '22', '1017', '1014', '53', 'gZ8kk9/AqtgenAHQc6dku2ui4AaSgEMY885Dw9Ymmys=', '1', '0', '1523606958', '1523606958');
INSERT INTO `IMMessage_6` VALUES ('57', '22', '1017', '1014', '54', 'junh8C9BY+EC8e93BQ41PbyT5aWhKWUvNt1Udw1mW4KtEp5W4K8unH9wHNO62FSk', '1', '0', '1523606966', '1523606966');
INSERT INTO `IMMessage_6` VALUES ('58', '14', '1003', '1012', '3', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524218763', '1524218763');
INSERT INTO `IMMessage_6` VALUES ('59', '30', '1022', '1021', '1', 'AHTqoc3rDzvbB7I7ytE5Dus3az5s5oJBw0AEdfGCUzI=', '1', '0', '1524221795', '1524221795');
INSERT INTO `IMMessage_6` VALUES ('60', '30', '1021', '1022', '2', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221802', '1524221802');
INSERT INTO `IMMessage_6` VALUES ('61', '30', '1022', '1021', '6', 'mga1sSsNf6aHMQeE0VufXw==', '1', '0', '1524222213', '1524222213');
INSERT INTO `IMMessage_6` VALUES ('62', '30', '1021', '1022', '8', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222218', '1524222218');
INSERT INTO `IMMessage_6` VALUES ('63', '30', '1022', '1021', '9', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222225', '1524222225');
INSERT INTO `IMMessage_6` VALUES ('64', '30', '1022', '1021', '10', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222227', '1524222227');
INSERT INTO `IMMessage_6` VALUES ('65', '30', '1021', '1022', '11', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222233', '1524222233');
INSERT INTO `IMMessage_6` VALUES ('66', '30', '1021', '1022', '12', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222233', '1524222233');
INSERT INTO `IMMessage_6` VALUES ('67', '30', '1021', '1022', '13', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222233', '1524222233');
INSERT INTO `IMMessage_6` VALUES ('68', '30', '1021', '1022', '14', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222234', '1524222234');
INSERT INTO `IMMessage_6` VALUES ('69', '30', '1021', '1022', '15', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222234', '1524222234');
INSERT INTO `IMMessage_6` VALUES ('70', '30', '1021', '1022', '16', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222234', '1524222234');
INSERT INTO `IMMessage_6` VALUES ('71', '30', '1021', '1022', '17', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222234', '1524222234');
INSERT INTO `IMMessage_6` VALUES ('72', '30', '1021', '1022', '18', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222234', '1524222234');
INSERT INTO `IMMessage_6` VALUES ('73', '30', '1021', '1022', '19', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222235', '1524222235');
INSERT INTO `IMMessage_6` VALUES ('74', '30', '1021', '1022', '20', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222235', '1524222235');
INSERT INTO `IMMessage_6` VALUES ('75', '30', '1021', '1022', '21', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222235', '1524222235');
INSERT INTO `IMMessage_6` VALUES ('76', '30', '1021', '1022', '22', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222235', '1524222235');
INSERT INTO `IMMessage_6` VALUES ('77', '30', '1021', '1022', '23', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222235', '1524222235');
INSERT INTO `IMMessage_6` VALUES ('78', '30', '1021', '1022', '24', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222236', '1524222236');
INSERT INTO `IMMessage_6` VALUES ('79', '30', '1021', '1022', '25', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222236', '1524222236');
INSERT INTO `IMMessage_6` VALUES ('80', '30', '1021', '1022', '26', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222236', '1524222236');
INSERT INTO `IMMessage_6` VALUES ('81', '30', '1021', '1022', '27', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222236', '1524222236');
INSERT INTO `IMMessage_6` VALUES ('82', '30', '1021', '1022', '28', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222237', '1524222237');
INSERT INTO `IMMessage_6` VALUES ('83', '30', '1021', '1022', '29', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222237', '1524222237');
INSERT INTO `IMMessage_6` VALUES ('84', '30', '1021', '1022', '30', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222237', '1524222237');
INSERT INTO `IMMessage_6` VALUES ('85', '30', '1021', '1022', '31', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222237', '1524222237');
INSERT INTO `IMMessage_6` VALUES ('86', '30', '1021', '1022', '32', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222238', '1524222238');
INSERT INTO `IMMessage_6` VALUES ('87', '30', '1021', '1022', '33', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222238', '1524222238');
INSERT INTO `IMMessage_6` VALUES ('88', '30', '1021', '1022', '34', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222238', '1524222238');
INSERT INTO `IMMessage_6` VALUES ('89', '30', '1021', '1022', '35', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222238', '1524222238');
INSERT INTO `IMMessage_6` VALUES ('90', '30', '1021', '1022', '36', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222238', '1524222238');
INSERT INTO `IMMessage_6` VALUES ('91', '30', '1021', '1022', '37', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222239', '1524222239');
INSERT INTO `IMMessage_6` VALUES ('92', '30', '1021', '1022', '38', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222239', '1524222239');
INSERT INTO `IMMessage_6` VALUES ('93', '30', '1021', '1022', '39', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222239', '1524222239');
INSERT INTO `IMMessage_6` VALUES ('94', '30', '1021', '1022', '40', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222239', '1524222239');
INSERT INTO `IMMessage_6` VALUES ('95', '30', '1021', '1022', '41', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222240', '1524222240');
INSERT INTO `IMMessage_6` VALUES ('96', '30', '1021', '1022', '42', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222240', '1524222240');
INSERT INTO `IMMessage_6` VALUES ('97', '30', '1021', '1022', '43', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222240', '1524222240');
INSERT INTO `IMMessage_6` VALUES ('98', '30', '1021', '1022', '44', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222240', '1524222240');
INSERT INTO `IMMessage_6` VALUES ('99', '30', '1021', '1022', '45', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222241', '1524222241');
INSERT INTO `IMMessage_6` VALUES ('100', '30', '1021', '1022', '46', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222241', '1524222241');
INSERT INTO `IMMessage_6` VALUES ('101', '30', '1021', '1022', '47', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222241', '1524222241');
INSERT INTO `IMMessage_6` VALUES ('102', '30', '1021', '1022', '48', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222241', '1524222241');
INSERT INTO `IMMessage_6` VALUES ('103', '30', '1021', '1022', '49', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222242', '1524222242');
INSERT INTO `IMMessage_6` VALUES ('104', '30', '1021', '1022', '50', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222242', '1524222242');
INSERT INTO `IMMessage_6` VALUES ('105', '30', '1021', '1022', '51', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222242', '1524222242');
INSERT INTO `IMMessage_6` VALUES ('106', '30', '1021', '1022', '52', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222242', '1524222242');
INSERT INTO `IMMessage_6` VALUES ('107', '30', '1021', '1022', '53', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222242', '1524222242');
INSERT INTO `IMMessage_6` VALUES ('108', '30', '1021', '1022', '54', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222243', '1524222243');
INSERT INTO `IMMessage_6` VALUES ('109', '30', '1021', '1022', '55', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222243', '1524222243');
INSERT INTO `IMMessage_6` VALUES ('110', '30', '1021', '1022', '56', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222243', '1524222243');
INSERT INTO `IMMessage_6` VALUES ('111', '30', '1021', '1022', '57', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222243', '1524222243');
INSERT INTO `IMMessage_6` VALUES ('112', '30', '1021', '1022', '58', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222243', '1524222243');
INSERT INTO `IMMessage_6` VALUES ('113', '30', '1021', '1022', '59', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222244', '1524222244');
INSERT INTO `IMMessage_6` VALUES ('114', '30', '1021', '1022', '60', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222244', '1524222244');
INSERT INTO `IMMessage_6` VALUES ('115', '30', '1021', '1022', '61', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222244', '1524222244');
INSERT INTO `IMMessage_6` VALUES ('116', '30', '1021', '1022', '62', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222244', '1524222244');
INSERT INTO `IMMessage_6` VALUES ('117', '30', '1021', '1022', '63', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222245', '1524222245');
INSERT INTO `IMMessage_6` VALUES ('118', '30', '1021', '1022', '64', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222245', '1524222245');
INSERT INTO `IMMessage_6` VALUES ('119', '30', '1021', '1022', '65', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222245', '1524222245');
INSERT INTO `IMMessage_6` VALUES ('120', '30', '1021', '1022', '66', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222245', '1524222245');
INSERT INTO `IMMessage_6` VALUES ('121', '30', '1021', '1022', '67', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222245', '1524222245');
INSERT INTO `IMMessage_6` VALUES ('122', '30', '1021', '1022', '68', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222246', '1524222246');
INSERT INTO `IMMessage_6` VALUES ('123', '30', '1021', '1022', '69', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222246', '1524222246');
INSERT INTO `IMMessage_6` VALUES ('124', '30', '1021', '1022', '70', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222246', '1524222246');
INSERT INTO `IMMessage_6` VALUES ('125', '30', '1021', '1022', '71', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222246', '1524222246');
INSERT INTO `IMMessage_6` VALUES ('126', '30', '1021', '1022', '72', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222247', '1524222247');
INSERT INTO `IMMessage_6` VALUES ('127', '30', '1021', '1022', '73', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222247', '1524222247');
INSERT INTO `IMMessage_6` VALUES ('128', '30', '1021', '1022', '74', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222247', '1524222247');
INSERT INTO `IMMessage_6` VALUES ('129', '30', '1021', '1022', '75', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222247', '1524222247');
INSERT INTO `IMMessage_6` VALUES ('130', '30', '1021', '1022', '76', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222247', '1524222247');
INSERT INTO `IMMessage_6` VALUES ('131', '30', '1021', '1022', '77', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222248', '1524222248');
INSERT INTO `IMMessage_6` VALUES ('132', '30', '1021', '1022', '78', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222248', '1524222248');
INSERT INTO `IMMessage_6` VALUES ('133', '30', '1021', '1022', '79', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222248', '1524222248');
INSERT INTO `IMMessage_6` VALUES ('134', '30', '1021', '1022', '80', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222248', '1524222248');
INSERT INTO `IMMessage_6` VALUES ('135', '30', '1021', '1022', '81', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222248', '1524222248');
INSERT INTO `IMMessage_6` VALUES ('136', '30', '1021', '1022', '82', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222249', '1524222249');
INSERT INTO `IMMessage_6` VALUES ('137', '30', '1021', '1022', '83', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222249', '1524222249');
INSERT INTO `IMMessage_6` VALUES ('138', '30', '1021', '1022', '84', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222249', '1524222249');
INSERT INTO `IMMessage_6` VALUES ('139', '30', '1021', '1022', '85', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524222249', '1524222249');
INSERT INTO `IMMessage_6` VALUES ('140', '30', '1021', '1022', '86', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222253', '1524222253');
INSERT INTO `IMMessage_6` VALUES ('141', '30', '1021', '1022', '87', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222253', '1524222253');
INSERT INTO `IMMessage_6` VALUES ('142', '30', '1021', '1022', '88', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222254', '1524222254');
INSERT INTO `IMMessage_6` VALUES ('143', '30', '1021', '1022', '89', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222254', '1524222254');
INSERT INTO `IMMessage_6` VALUES ('144', '30', '1021', '1022', '90', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222254', '1524222254');
INSERT INTO `IMMessage_6` VALUES ('145', '30', '1021', '1022', '91', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222254', '1524222254');
INSERT INTO `IMMessage_6` VALUES ('146', '30', '1021', '1022', '92', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222254', '1524222254');
INSERT INTO `IMMessage_6` VALUES ('147', '30', '1021', '1022', '93', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222254', '1524222254');
INSERT INTO `IMMessage_6` VALUES ('148', '30', '1021', '1022', '94', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222255', '1524222255');
INSERT INTO `IMMessage_6` VALUES ('149', '30', '1021', '1022', '95', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222255', '1524222255');
INSERT INTO `IMMessage_6` VALUES ('150', '30', '1021', '1022', '96', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222255', '1524222255');
INSERT INTO `IMMessage_6` VALUES ('151', '30', '1021', '1022', '97', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222255', '1524222255');
INSERT INTO `IMMessage_6` VALUES ('152', '30', '1021', '1022', '98', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222256', '1524222256');
INSERT INTO `IMMessage_6` VALUES ('153', '30', '1021', '1022', '99', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222256', '1524222256');
INSERT INTO `IMMessage_6` VALUES ('154', '30', '1021', '1022', '100', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222256', '1524222256');
INSERT INTO `IMMessage_6` VALUES ('155', '30', '1021', '1022', '101', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222256', '1524222256');
INSERT INTO `IMMessage_6` VALUES ('156', '30', '1021', '1022', '102', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222256', '1524222256');
INSERT INTO `IMMessage_6` VALUES ('157', '30', '1021', '1022', '103', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222257', '1524222257');
INSERT INTO `IMMessage_6` VALUES ('158', '30', '1021', '1022', '104', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222257', '1524222257');
INSERT INTO `IMMessage_6` VALUES ('159', '30', '1021', '1022', '105', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222257', '1524222257');
INSERT INTO `IMMessage_6` VALUES ('160', '30', '1021', '1022', '106', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222257', '1524222257');
INSERT INTO `IMMessage_6` VALUES ('161', '30', '1021', '1022', '107', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222257', '1524222257');
INSERT INTO `IMMessage_6` VALUES ('162', '30', '1021', '1022', '108', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_6` VALUES ('163', '30', '1021', '1022', '109', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_6` VALUES ('164', '30', '1021', '1022', '110', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_6` VALUES ('165', '30', '1021', '1022', '111', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_6` VALUES ('166', '30', '1021', '1022', '112', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222258', '1524222258');
INSERT INTO `IMMessage_6` VALUES ('167', '30', '1021', '1022', '113', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222259', '1524222259');
INSERT INTO `IMMessage_6` VALUES ('168', '30', '1021', '1022', '114', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222259', '1524222259');
INSERT INTO `IMMessage_6` VALUES ('169', '30', '1021', '1022', '115', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222259', '1524222259');
INSERT INTO `IMMessage_6` VALUES ('170', '30', '1021', '1022', '116', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222259', '1524222259');
INSERT INTO `IMMessage_6` VALUES ('171', '30', '1021', '1022', '117', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222259', '1524222259');
INSERT INTO `IMMessage_6` VALUES ('172', '30', '1021', '1022', '118', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222260', '1524222260');
INSERT INTO `IMMessage_6` VALUES ('173', '30', '1021', '1022', '119', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222260', '1524222260');
INSERT INTO `IMMessage_6` VALUES ('174', '30', '1021', '1022', '120', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222260', '1524222260');
INSERT INTO `IMMessage_6` VALUES ('175', '30', '1021', '1022', '121', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222260', '1524222260');
INSERT INTO `IMMessage_6` VALUES ('176', '30', '1021', '1022', '122', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222262', '1524222262');
INSERT INTO `IMMessage_6` VALUES ('177', '30', '1021', '1022', '123', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222262', '1524222262');
INSERT INTO `IMMessage_6` VALUES ('178', '30', '1021', '1022', '124', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222263', '1524222263');
INSERT INTO `IMMessage_6` VALUES ('179', '30', '1021', '1022', '125', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222263', '1524222263');
INSERT INTO `IMMessage_6` VALUES ('180', '30', '1021', '1022', '126', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222263', '1524222263');
INSERT INTO `IMMessage_6` VALUES ('181', '30', '1021', '1022', '127', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222263', '1524222263');
INSERT INTO `IMMessage_6` VALUES ('182', '30', '1021', '1022', '128', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222263', '1524222263');
INSERT INTO `IMMessage_6` VALUES ('183', '30', '1021', '1022', '129', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222264', '1524222264');
INSERT INTO `IMMessage_6` VALUES ('184', '30', '1021', '1022', '130', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222264', '1524222264');
INSERT INTO `IMMessage_6` VALUES ('185', '30', '1021', '1022', '131', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222264', '1524222264');
INSERT INTO `IMMessage_6` VALUES ('186', '30', '1021', '1022', '132', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222264', '1524222264');
INSERT INTO `IMMessage_6` VALUES ('187', '30', '1021', '1022', '133', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222264', '1524222264');
INSERT INTO `IMMessage_6` VALUES ('188', '30', '1021', '1022', '134', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222265', '1524222265');
INSERT INTO `IMMessage_6` VALUES ('189', '30', '1021', '1022', '135', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222265', '1524222265');
INSERT INTO `IMMessage_6` VALUES ('190', '30', '1021', '1022', '136', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222265', '1524222265');
INSERT INTO `IMMessage_6` VALUES ('191', '30', '1021', '1022', '137', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222265', '1524222265');
INSERT INTO `IMMessage_6` VALUES ('192', '30', '1021', '1022', '138', '8JPkwJBnwQMitiGrTbeX2HWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222266', '1524222266');
INSERT INTO `IMMessage_6` VALUES ('193', '30', '1021', '1022', '139', '8JPkwJBnwQMitiGrTbeX2HWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222266', '1524222266');
INSERT INTO `IMMessage_6` VALUES ('194', '30', '1021', '1022', '140', '8JPkwJBnwQMitiGrTbeX2HWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222266', '1524222266');
INSERT INTO `IMMessage_6` VALUES ('195', '30', '1021', '1022', '141', '8JPkwJBnwQMitiGrTbeX2HWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222266', '1524222266');
INSERT INTO `IMMessage_6` VALUES ('196', '30', '1021', '1022', '142', '8JPkwJBnwQMitiGrTbeX2HWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222267', '1524222267');
INSERT INTO `IMMessage_6` VALUES ('197', '30', '1021', '1022', '143', '8JPkwJBnwQMitiGrTbeX2HWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222267', '1524222267');
INSERT INTO `IMMessage_6` VALUES ('198', '30', '1021', '1022', '144', '8JPkwJBnwQMitiGrTbeX2HWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222267', '1524222267');
INSERT INTO `IMMessage_6` VALUES ('199', '30', '1021', '1022', '145', '8JPkwJBnwQMitiGrTbeX2HWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222267', '1524222267');
INSERT INTO `IMMessage_6` VALUES ('200', '30', '1021', '1022', '146', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222269', '1524222269');
INSERT INTO `IMMessage_6` VALUES ('201', '30', '1021', '1022', '147', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222270', '1524222270');
INSERT INTO `IMMessage_6` VALUES ('202', '30', '1021', '1022', '148', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222270', '1524222270');
INSERT INTO `IMMessage_6` VALUES ('203', '30', '1021', '1022', '149', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222270', '1524222270');
INSERT INTO `IMMessage_6` VALUES ('204', '30', '1021', '1022', '150', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222270', '1524222270');
INSERT INTO `IMMessage_6` VALUES ('205', '30', '1021', '1022', '151', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222270', '1524222270');
INSERT INTO `IMMessage_6` VALUES ('206', '30', '1021', '1022', '152', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222270', '1524222270');
INSERT INTO `IMMessage_6` VALUES ('207', '30', '1021', '1022', '153', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222271', '1524222271');
INSERT INTO `IMMessage_6` VALUES ('208', '30', '1021', '1022', '154', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222271', '1524222271');
INSERT INTO `IMMessage_6` VALUES ('209', '30', '1021', '1022', '155', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222271', '1524222271');
INSERT INTO `IMMessage_6` VALUES ('210', '30', '1021', '1022', '156', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222271', '1524222271');
INSERT INTO `IMMessage_6` VALUES ('211', '30', '1021', '1022', '157', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222272', '1524222272');
INSERT INTO `IMMessage_6` VALUES ('212', '30', '1021', '1022', '158', 'Oi9uYQggKzQOyt/ws6kcZXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222272', '1524222272');
INSERT INTO `IMMessage_6` VALUES ('213', '30', '1021', '1022', '159', 'zL6M+McU0nfiklCiWjAkEHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222272', '1524222272');
INSERT INTO `IMMessage_6` VALUES ('214', '30', '1021', '1022', '160', 'zL6M+McU0nfiklCiWjAkEHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222272', '1524222272');
INSERT INTO `IMMessage_6` VALUES ('215', '30', '1021', '1022', '161', 'zL6M+McU0nfiklCiWjAkEHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222273', '1524222273');
INSERT INTO `IMMessage_6` VALUES ('216', '30', '1021', '1022', '162', 'zL6M+McU0nfiklCiWjAkEHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222273', '1524222273');
INSERT INTO `IMMessage_6` VALUES ('217', '30', '1021', '1022', '163', 'zL6M+McU0nfiklCiWjAkEHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222273', '1524222273');
INSERT INTO `IMMessage_6` VALUES ('218', '30', '1021', '1022', '164', 'zL6M+McU0nfiklCiWjAkEHWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222273', '1524222273');
INSERT INTO `IMMessage_6` VALUES ('219', '30', '1021', '1022', '165', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222295', '1524222295');
INSERT INTO `IMMessage_6` VALUES ('220', '30', '1021', '1022', '166', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222295', '1524222295');
INSERT INTO `IMMessage_6` VALUES ('221', '30', '1021', '1022', '167', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222296', '1524222296');
INSERT INTO `IMMessage_6` VALUES ('222', '30', '1021', '1022', '168', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222296', '1524222296');
INSERT INTO `IMMessage_6` VALUES ('223', '30', '1021', '1022', '169', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222296', '1524222296');
INSERT INTO `IMMessage_6` VALUES ('224', '30', '1021', '1022', '170', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222296', '1524222296');
INSERT INTO `IMMessage_6` VALUES ('225', '30', '1021', '1022', '171', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222296', '1524222296');
INSERT INTO `IMMessage_6` VALUES ('226', '30', '1021', '1022', '172', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222297', '1524222297');
INSERT INTO `IMMessage_6` VALUES ('227', '30', '1021', '1022', '173', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222297', '1524222297');
INSERT INTO `IMMessage_6` VALUES ('228', '30', '1021', '1022', '174', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222297', '1524222297');
INSERT INTO `IMMessage_6` VALUES ('229', '30', '1021', '1022', '175', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222297', '1524222297');
INSERT INTO `IMMessage_6` VALUES ('230', '30', '1021', '1022', '176', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222297', '1524222297');
INSERT INTO `IMMessage_6` VALUES ('231', '30', '1021', '1022', '177', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222297', '1524222297');
INSERT INTO `IMMessage_6` VALUES ('232', '30', '1021', '1022', '178', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222298', '1524222298');
INSERT INTO `IMMessage_6` VALUES ('233', '30', '1021', '1022', '179', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222298', '1524222298');
INSERT INTO `IMMessage_6` VALUES ('234', '30', '1021', '1022', '180', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222298', '1524222298');
INSERT INTO `IMMessage_6` VALUES ('235', '30', '1021', '1022', '181', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222298', '1524222298');
INSERT INTO `IMMessage_6` VALUES ('236', '30', '1021', '1022', '182', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222298', '1524222298');
INSERT INTO `IMMessage_6` VALUES ('237', '30', '1021', '1022', '183', 'D4sc2WVwtzz1GyOPKhflLnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222299', '1524222299');

-- ----------------------------
-- Table structure for IMMessage_7
-- ----------------------------
DROP TABLE IF EXISTS `IMMessage_7`;
CREATE TABLE `IMMessage_7` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `relateId` int(11) unsigned NOT NULL COMMENT '用户的关系id',
  `fromId` int(11) unsigned NOT NULL COMMENT '发送用户的id',
  `toId` int(11) unsigned NOT NULL COMMENT '接收用户的id',
  `msgId` int(11) unsigned NOT NULL COMMENT '消息ID',
  `content` varchar(4096) COLLATE utf8_general_ci DEFAULT '' COMMENT '消息内容',
  `type` tinyint(2) unsigned NOT NULL DEFAULT '1' COMMENT '消息类型',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0正常 1被删除',
  `updated` int(11) unsigned NOT NULL COMMENT '更新时间',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_relateId_status_created` (`relateId`,`status`,`created`),
  KEY `idx_relateId_status_msgId_created` (`relateId`,`status`,`msgId`,`created`),
  KEY `idx_fromId_toId_created` (`fromId`,`toId`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=88 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- ----------------------------
-- Records of IMMessage_7
-- ----------------------------
INSERT INTO `IMMessage_7` VALUES ('1', '7', '1003', '1000', '1', 'ecuWtLVhL//svPR9nRF2Fw==', '1', '0', '1522829358', '1522829358');
INSERT INTO `IMMessage_7` VALUES ('2', '7', '1003', '1000', '2', 'ecuWtLVhL//svPR9nRF2Fw==', '1', '0', '1522829482', '1522829482');
INSERT INTO `IMMessage_7` VALUES ('3', '15', '1003', '1009', '1', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523433833', '1523433833');
INSERT INTO `IMMessage_7` VALUES ('4', '15', '1003', '1009', '2', 'nGshqOKR43y8qXNsTzetzA==', '1', '0', '1523440252', '1523440252');
INSERT INTO `IMMessage_7` VALUES ('5', '15', '1003', '1009', '3', 'nGshqOKR43y8qXNsTzetzA==', '1', '0', '1523440267', '1523440267');
INSERT INTO `IMMessage_7` VALUES ('6', '15', '1003', '1009', '4', 'R7ZIri94ksqx1vvNbkIuOw==', '1', '0', '1523440686', '1523440686');
INSERT INTO `IMMessage_7` VALUES ('7', '15', '1003', '1009', '5', '0AUBswJvBILXt+jNEHmhuQ==', '1', '0', '1523441287', '1523441287');
INSERT INTO `IMMessage_7` VALUES ('8', '15', '1003', '1009', '6', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523443444', '1523443444');
INSERT INTO `IMMessage_7` VALUES ('9', '15', '1003', '1009', '7', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523443544', '1523443544');
INSERT INTO `IMMessage_7` VALUES ('10', '15', '1003', '1009', '8', 'NEr8TKwoMSUYc2NjSiIAEw==', '1', '0', '1523444002', '1523444002');
INSERT INTO `IMMessage_7` VALUES ('11', '15', '1003', '1009', '9', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1523506729', '1523506729');
INSERT INTO `IMMessage_7` VALUES ('12', '15', '1003', '1009', '10', 'dscnnibYAHCkpQenMd87GQ==', '1', '0', '1523514153', '1523514153');
INSERT INTO `IMMessage_7` VALUES ('13', '15', '1009', '1003', '11', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523514157', '1523514157');
INSERT INTO `IMMessage_7` VALUES ('14', '15', '1009', '1003', '12', 'dscnnibYAHCkpQenMd87GQ==', '1', '0', '1523514162', '1523514162');
INSERT INTO `IMMessage_7` VALUES ('15', '15', '1003', '1009', '13', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523514169', '1523514169');
INSERT INTO `IMMessage_7` VALUES ('16', '15', '1009', '1003', '14', 'n8ZPha2aUItqjdipfWJ5qg==', '1', '0', '1523514174', '1523514174');
INSERT INTO `IMMessage_7` VALUES ('17', '15', '1009', '1003', '15', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1523514429', '1523514429');
INSERT INTO `IMMessage_7` VALUES ('18', '15', '1003', '1009', '16', 'dscnnibYAHCkpQenMd87GQ==', '1', '0', '1523514459', '1523514459');
INSERT INTO `IMMessage_7` VALUES ('19', '15', '1009', '1003', '17', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523514470', '1523514470');
INSERT INTO `IMMessage_7` VALUES ('20', '15', '1009', '1003', '18', 'brMcxcmVDxcd1IAs1z4LsXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523514909', '1523514909');
INSERT INTO `IMMessage_7` VALUES ('21', '15', '1009', '1003', '19', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1523514915', '1523514915');
INSERT INTO `IMMessage_7` VALUES ('22', '15', '1009', '1003', '20', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523514920', '1523514920');
INSERT INTO `IMMessage_7` VALUES ('23', '15', '1009', '1003', '21', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523514921', '1523514921');
INSERT INTO `IMMessage_7` VALUES ('24', '15', '1009', '1003', '22', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1523515164', '1523515164');
INSERT INTO `IMMessage_7` VALUES ('25', '15', '1009', '1003', '23', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1523515166', '1523515166');
INSERT INTO `IMMessage_7` VALUES ('26', '15', '1009', '1003', '24', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523515248', '1523515248');
INSERT INTO `IMMessage_7` VALUES ('27', '15', '1009', '1003', '25', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523515599', '1523515599');
INSERT INTO `IMMessage_7` VALUES ('28', '15', '1009', '1003', '26', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523515609', '1523515609');
INSERT INTO `IMMessage_7` VALUES ('29', '15', '1009', '1003', '27', 'yPi1udbJkJvn+R1KCyeZuXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523515615', '1523515615');
INSERT INTO `IMMessage_7` VALUES ('30', '15', '1009', '1003', '28', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523515624', '1523515624');
INSERT INTO `IMMessage_7` VALUES ('31', '15', '1009', '1003', '29', 'yPi1udbJkJvn+R1KCyeZuXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523515626', '1523515626');
INSERT INTO `IMMessage_7` VALUES ('32', '15', '1009', '1003', '30', 'yPi1udbJkJvn+R1KCyeZuXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523515631', '1523515631');
INSERT INTO `IMMessage_7` VALUES ('33', '15', '1009', '1003', '31', 'M1xntyxB8Wu5zgjSswbKgXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523515634', '1523515634');
INSERT INTO `IMMessage_7` VALUES ('34', '15', '1009', '1003', '32', 'B+HuUVixIt9S2wq8XgAH9A==', '1', '0', '1523515644', '1523515644');
INSERT INTO `IMMessage_7` VALUES ('35', '15', '1009', '1003', '33', '+jor1yfGfbfgJP/WzwIJiA==', '1', '0', '1523515644', '1523515644');
INSERT INTO `IMMessage_7` VALUES ('36', '15', '1009', '1003', '34', 'kZzcyyewYZ9bqPUyEZa20g==', '1', '0', '1523515645', '1523515645');
INSERT INTO `IMMessage_7` VALUES ('37', '15', '1009', '1003', '35', '5KqzQOvpMahevhraRjo5hg==', '1', '0', '1523515646', '1523515646');
INSERT INTO `IMMessage_7` VALUES ('38', '15', '1009', '1003', '36', 'B+w1Y7/g0tUymia4pWfBRg==', '1', '0', '1523515646', '1523515646');
INSERT INTO `IMMessage_7` VALUES ('39', '15', '1009', '1003', '37', 'YMhfonWIcq7TLAv+wPWa6A==', '1', '0', '1523515647', '1523515647');
INSERT INTO `IMMessage_7` VALUES ('40', '15', '1009', '1003', '38', 'sYDnV7DRYOQDvvSpJRKgtA==', '1', '0', '1523515648', '1523515648');
INSERT INTO `IMMessage_7` VALUES ('41', '15', '1009', '1003', '39', 'muRxbdEuMCWYmeC0a1H0Tw==', '1', '0', '1523515648', '1523515648');
INSERT INTO `IMMessage_7` VALUES ('42', '15', '1009', '1003', '40', 'c25U4xhFMEm9+D7hSAbzoQ==', '1', '0', '1523515649', '1523515649');
INSERT INTO `IMMessage_7` VALUES ('43', '15', '1009', '1003', '41', 'c25U4xhFMEm9+D7hSAbzoQ==', '1', '0', '1523515649', '1523515649');
INSERT INTO `IMMessage_7` VALUES ('44', '15', '1009', '1003', '42', '3Tb0YyBWeWNy+C0bdH+/+Q==', '1', '0', '1523515649', '1523515649');
INSERT INTO `IMMessage_7` VALUES ('45', '15', '1009', '1003', '43', 'c25U4xhFMEm9+D7hSAbzoQ==', '1', '0', '1523515649', '1523515649');
INSERT INTO `IMMessage_7` VALUES ('46', '15', '1009', '1003', '44', 'c25U4xhFMEm9+D7hSAbzoQ==', '1', '0', '1523515650', '1523515650');
INSERT INTO `IMMessage_7` VALUES ('47', '15', '1009', '1003', '45', 'c25U4xhFMEm9+D7hSAbzoQ==', '1', '0', '1523515650', '1523515650');
INSERT INTO `IMMessage_7` VALUES ('48', '15', '1009', '1003', '46', 'c25U4xhFMEm9+D7hSAbzoQ==', '1', '0', '1523515651', '1523515651');
INSERT INTO `IMMessage_7` VALUES ('49', '15', '1009', '1003', '47', 'B+HuUVixIt9S2wq8XgAH9A==', '1', '0', '1523515661', '1523515661');
INSERT INTO `IMMessage_7` VALUES ('50', '15', '1009', '1003', '48', '+jor1yfGfbfgJP/WzwIJiA==', '1', '0', '1523515662', '1523515662');
INSERT INTO `IMMessage_7` VALUES ('51', '15', '1009', '1003', '49', 'kZzcyyewYZ9bqPUyEZa20g==', '1', '0', '1523515663', '1523515663');
INSERT INTO `IMMessage_7` VALUES ('52', '15', '1009', '1003', '50', '5KqzQOvpMahevhraRjo5hg==', '1', '0', '1523515664', '1523515664');
INSERT INTO `IMMessage_7` VALUES ('53', '15', '1009', '1003', '51', 'B+w1Y7/g0tUymia4pWfBRg==', '1', '0', '1523515664', '1523515664');
INSERT INTO `IMMessage_7` VALUES ('54', '15', '1009', '1003', '52', 'YMhfonWIcq7TLAv+wPWa6A==', '1', '0', '1523515665', '1523515665');
INSERT INTO `IMMessage_7` VALUES ('55', '15', '1009', '1003', '53', 'sYDnV7DRYOQDvvSpJRKgtA==', '1', '0', '1523515665', '1523515665');
INSERT INTO `IMMessage_7` VALUES ('56', '15', '1009', '1003', '54', 'muRxbdEuMCWYmeC0a1H0Tw==', '1', '0', '1523515666', '1523515666');
INSERT INTO `IMMessage_7` VALUES ('57', '15', '1009', '1003', '55', 'c25U4xhFMEm9+D7hSAbzoQ==', '1', '0', '1523515666', '1523515666');
INSERT INTO `IMMessage_7` VALUES ('58', '15', '1009', '1003', '56', 'yPi1udbJkJvn+R1KCyeZuXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523515682', '1523515682');
INSERT INTO `IMMessage_7` VALUES ('59', '15', '1009', '1003', '57', 'yPi1udbJkJvn+R1KCyeZuXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523515686', '1523515686');
INSERT INTO `IMMessage_7` VALUES ('60', '15', '1009', '1003', '58', '0MoseN2wiZ3+h2GlBkAxYhLyLS1oMWHobm8prY23DtU=', '1', '0', '1523515692', '1523515692');
INSERT INTO `IMMessage_7` VALUES ('61', '23', '1018', '1017', '1', 'UN2uZ9FcLDcw5/MFgYN4FQ==', '1', '0', '1523604705', '1523604705');
INSERT INTO `IMMessage_7` VALUES ('62', '23', '1018', '1017', '2', 'brMcxcmVDxcd1IAs1z4LsXWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523604708', '1523604708');
INSERT INTO `IMMessage_7` VALUES ('63', '23', '1017', '1018', '3', '+n07NFUEJ0GXmhn+jKrrgw==', '1', '0', '1523604732', '1523604732');
INSERT INTO `IMMessage_7` VALUES ('64', '23', '1017', '1018', '4', '8JPkwJBnwQMitiGrTbeX2HWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1523604735', '1523604735');
INSERT INTO `IMMessage_7` VALUES ('65', '23', '1017', '1018', '5', 'A+vEeeFAr3rKHYkGVRJ//g==', '1', '0', '1523604918', '1523604918');
INSERT INTO `IMMessage_7` VALUES ('66', '31', '1021', '1020', '1', 'MLnF8Avj9pVc9mdA2ijFtRLyLS1oMWHobm8prY23DtU=', '1', '0', '1524221811', '1524221811');
INSERT INTO `IMMessage_7` VALUES ('67', '31', '1020', '1021', '2', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221815', '1524221815');
INSERT INTO `IMMessage_7` VALUES ('68', '31', '1020', '1021', '3', 'gkELJinpTMWnofK9zgPoNnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524221824', '1524221824');
INSERT INTO `IMMessage_7` VALUES ('69', '31', '1021', '1020', '7', 'XJGVB14N561q7GFM8q1+U3WXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222227', '1524222227');
INSERT INTO `IMMessage_7` VALUES ('70', '31', '1021', '1020', '8', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222301', '1524222301');
INSERT INTO `IMMessage_7` VALUES ('71', '31', '1021', '1020', '9', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222301', '1524222301');
INSERT INTO `IMMessage_7` VALUES ('72', '31', '1021', '1020', '10', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222301', '1524222301');
INSERT INTO `IMMessage_7` VALUES ('73', '31', '1021', '1020', '11', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222301', '1524222301');
INSERT INTO `IMMessage_7` VALUES ('74', '31', '1021', '1020', '12', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222302', '1524222302');
INSERT INTO `IMMessage_7` VALUES ('75', '31', '1021', '1020', '13', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222302', '1524222302');
INSERT INTO `IMMessage_7` VALUES ('76', '31', '1021', '1020', '14', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222302', '1524222302');
INSERT INTO `IMMessage_7` VALUES ('77', '31', '1021', '1020', '15', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222302', '1524222302');
INSERT INTO `IMMessage_7` VALUES ('78', '31', '1021', '1020', '16', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222302', '1524222302');
INSERT INTO `IMMessage_7` VALUES ('79', '31', '1021', '1020', '17', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222303', '1524222303');
INSERT INTO `IMMessage_7` VALUES ('80', '31', '1021', '1020', '18', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222303', '1524222303');
INSERT INTO `IMMessage_7` VALUES ('81', '31', '1021', '1020', '19', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222303', '1524222303');
INSERT INTO `IMMessage_7` VALUES ('82', '31', '1021', '1020', '20', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222303', '1524222303');
INSERT INTO `IMMessage_7` VALUES ('83', '31', '1021', '1020', '21', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222303', '1524222303');
INSERT INTO `IMMessage_7` VALUES ('84', '31', '1021', '1020', '22', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222303', '1524222303');
INSERT INTO `IMMessage_7` VALUES ('85', '31', '1021', '1020', '23', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222304', '1524222304');
INSERT INTO `IMMessage_7` VALUES ('86', '31', '1021', '1020', '24', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222304', '1524222304');
INSERT INTO `IMMessage_7` VALUES ('87', '31', '1021', '1020', '25', 'xHiMKYHeH28J8UhORPMlAnWXrZFfV+UBo37vcxL/NUY=', '1', '0', '1524222304', '1524222304');

-- ----------------------------
-- Table structure for IMProxy
-- ----------------------------
DROP TABLE IF EXISTS `IMProxy`;
CREATE TABLE `IMProxy` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `name` varchar(32) DEFAULT NULL COMMENT '用户名',
  `created` int(11) DEFAULT NULL COMMENT '代理创建的时间',
  `updated` int(11) DEFAULT NULL COMMENT '代理更新的时间',
  `status` int(11) DEFAULT NULL COMMENT '代理状态 0:正常 1:禁用',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1001 DEFAULT CHARSET=utf8 COMMENT='代理表';

-- ----------------------------
-- Records of IMProxy
-- ----------------------------
INSERT INTO `IMProxy` VALUES ('1000', '18898739725', null, null, '0');

-- ----------------------------
-- Table structure for IMRecentSession
-- ----------------------------
DROP TABLE IF EXISTS `IMRecentSession`;
CREATE TABLE `IMRecentSession` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userId` int(11) unsigned NOT NULL COMMENT '用户id',
  `peerId` int(11) unsigned NOT NULL COMMENT '对方id',
  `type` tinyint(1) unsigned DEFAULT '0' COMMENT '类型，1-用户,2-群组',
  `status` tinyint(1) unsigned DEFAULT '0' COMMENT '用户:0-正常, 1-用户A删除,群组:0-正常, 1-被删除',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `idx_userId_peerId_status_updated` (`userId`,`peerId`,`status`,`updated`),
  KEY `idx_userId_peerId_type` (`userId`,`peerId`,`type`)
) ENGINE=InnoDB AUTO_INCREMENT=75 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of IMRecentSession
-- ----------------------------
INSERT INTO `IMRecentSession` VALUES ('1', '2', '1', '1', '0', '1519908349', '1522208577');
INSERT INTO `IMRecentSession` VALUES ('2', '1', '2', '1', '0', '1519908349', '1522208577');
INSERT INTO `IMRecentSession` VALUES ('3', '4', '2', '1', '0', '1520411936', '1520493677');
INSERT INTO `IMRecentSession` VALUES ('4', '2', '4', '1', '0', '1520411936', '1520493677');
INSERT INTO `IMRecentSession` VALUES ('5', '4', '3', '1', '0', '1520495409', '1520495409');
INSERT INTO `IMRecentSession` VALUES ('6', '3', '4', '1', '0', '1520495409', '1520495409');
INSERT INTO `IMRecentSession` VALUES ('7', '31', '29', '1', '0', '1522121087', '1522122844');
INSERT INTO `IMRecentSession` VALUES ('8', '29', '31', '1', '0', '1522121087', '1522122844');
INSERT INTO `IMRecentSession` VALUES ('9', '31', '1', '2', '0', '1522121807', '1522122860');
INSERT INTO `IMRecentSession` VALUES ('10', '29', '1', '2', '0', '1522121810', '1522122860');
INSERT INTO `IMRecentSession` VALUES ('11', '1001', '1000', '1', '0', '1522640740', '1522833352');
INSERT INTO `IMRecentSession` VALUES ('12', '1000', '1001', '1', '0', '1522640740', '1522833352');
INSERT INTO `IMRecentSession` VALUES ('13', '1012', '1000', '1', '0', '1522827427', '1522827427');
INSERT INTO `IMRecentSession` VALUES ('14', '1000', '1012', '1', '0', '1522827427', '1522827427');
INSERT INTO `IMRecentSession` VALUES ('15', '1003', '1000', '1', '0', '1522829358', '1522829482');
INSERT INTO `IMRecentSession` VALUES ('16', '1000', '1003', '1', '0', '1522829358', '1522829482');
INSERT INTO `IMRecentSession` VALUES ('17', '1013', '1001', '1', '0', '1522830944', '1522831136');
INSERT INTO `IMRecentSession` VALUES ('18', '1001', '1013', '1', '0', '1522830944', '1522831136');
INSERT INTO `IMRecentSession` VALUES ('19', '1014', '1001', '1', '0', '1522838887', '1523531699');
INSERT INTO `IMRecentSession` VALUES ('20', '1001', '1014', '1', '0', '1522838887', '1523531699');
INSERT INTO `IMRecentSession` VALUES ('21', '1014', '1007', '1', '0', '1522838901', '1523271673');
INSERT INTO `IMRecentSession` VALUES ('22', '1007', '1014', '1', '0', '1522838901', '1523271673');
INSERT INTO `IMRecentSession` VALUES ('23', '1003', '1015', '1', '0', '1523354132', '1523852512');
INSERT INTO `IMRecentSession` VALUES ('24', '1015', '1003', '1', '0', '1523354132', '1523852512');
INSERT INTO `IMRecentSession` VALUES ('25', '1003', '1006', '1', '0', '1523433041', '1523435285');
INSERT INTO `IMRecentSession` VALUES ('26', '1006', '1003', '1', '0', '1523433041', '1523435285');
INSERT INTO `IMRecentSession` VALUES ('27', '1003', '1010', '1', '0', '1523433674', '1523433850');
INSERT INTO `IMRecentSession` VALUES ('28', '1010', '1003', '1', '0', '1523433674', '1523433850');
INSERT INTO `IMRecentSession` VALUES ('29', '1003', '1012', '1', '0', '1523433809', '1524218763');
INSERT INTO `IMRecentSession` VALUES ('30', '1012', '1003', '1', '0', '1523433809', '1524218763');
INSERT INTO `IMRecentSession` VALUES ('31', '1003', '1009', '1', '0', '1523433833', '1523515692');
INSERT INTO `IMRecentSession` VALUES ('32', '1009', '1003', '1', '0', '1523433833', '1523515692');
INSERT INTO `IMRecentSession` VALUES ('33', '1003', '1007', '1', '0', '1523433858', '1523433858');
INSERT INTO `IMRecentSession` VALUES ('34', '1007', '1003', '1', '0', '1523433858', '1523433858');
INSERT INTO `IMRecentSession` VALUES ('35', '1003', '1011', '1', '0', '1523434219', '1523506706');
INSERT INTO `IMRecentSession` VALUES ('36', '1011', '1003', '1', '0', '1523434219', '1523506706');
INSERT INTO `IMRecentSession` VALUES ('37', '1003', '1001', '1', '0', '1523434550', '1523438636');
INSERT INTO `IMRecentSession` VALUES ('38', '1001', '1003', '1', '0', '1523434550', '1523438636');
INSERT INTO `IMRecentSession` VALUES ('39', '1016', '1006', '1', '0', '1523603415', '1523603415');
INSERT INTO `IMRecentSession` VALUES ('40', '1006', '1016', '1', '0', '1523603415', '1523603415');
INSERT INTO `IMRecentSession` VALUES ('41', '1014', '1009', '1', '0', '1523603547', '1523603547');
INSERT INTO `IMRecentSession` VALUES ('42', '1009', '1014', '1', '0', '1523603547', '1523603547');
INSERT INTO `IMRecentSession` VALUES ('43', '1014', '1012', '1', '0', '1523603598', '1523603833');
INSERT INTO `IMRecentSession` VALUES ('44', '1012', '1014', '1', '0', '1523603598', '1523603833');
INSERT INTO `IMRecentSession` VALUES ('45', '1014', '1017', '1', '0', '1523604463', '1523606966');
INSERT INTO `IMRecentSession` VALUES ('46', '1017', '1014', '1', '0', '1523604463', '1523606966');
INSERT INTO `IMRecentSession` VALUES ('47', '1018', '1017', '1', '0', '1523604705', '1523604918');
INSERT INTO `IMRecentSession` VALUES ('48', '1017', '1018', '1', '0', '1523604705', '1523604918');
INSERT INTO `IMRecentSession` VALUES ('49', '1014', '1018', '1', '0', '1523605280', '1523605293');
INSERT INTO `IMRecentSession` VALUES ('50', '1018', '1014', '1', '0', '1523605280', '1523605293');
INSERT INTO `IMRecentSession` VALUES ('51', '1014', '1015', '1', '0', '1523605307', '1523605307');
INSERT INTO `IMRecentSession` VALUES ('52', '1015', '1014', '1', '0', '1523605307', '1523605307');
INSERT INTO `IMRecentSession` VALUES ('53', '1003', '1008', '1', '0', '1524043891', '1524043891');
INSERT INTO `IMRecentSession` VALUES ('54', '1008', '1003', '1', '0', '1524043891', '1524043891');
INSERT INTO `IMRecentSession` VALUES ('55', '1003', '1014', '1', '0', '1524134582', '1524213506');
INSERT INTO `IMRecentSession` VALUES ('56', '1014', '1003', '1', '0', '1524134582', '1524213506');
INSERT INTO `IMRecentSession` VALUES ('57', '1019', '1020', '1', '0', '1524221070', '1524278021');
INSERT INTO `IMRecentSession` VALUES ('58', '1020', '1019', '1', '0', '1524221070', '1524278021');
INSERT INTO `IMRecentSession` VALUES ('59', '1022', '1019', '1', '0', '1524221777', '1524221779');
INSERT INTO `IMRecentSession` VALUES ('60', '1019', '1022', '1', '0', '1524221777', '1524221779');
INSERT INTO `IMRecentSession` VALUES ('61', '1022', '1021', '1', '0', '1524221795', '1524222299');
INSERT INTO `IMRecentSession` VALUES ('62', '1021', '1022', '1', '0', '1524221795', '1524222299');
INSERT INTO `IMRecentSession` VALUES ('63', '1021', '1020', '1', '0', '1524221811', '1524222304');
INSERT INTO `IMRecentSession` VALUES ('64', '1020', '1021', '1', '0', '1524221811', '1524222304');
INSERT INTO `IMRecentSession` VALUES ('65', '1022', '1020', '1', '0', '1524221823', '1524221904');
INSERT INTO `IMRecentSession` VALUES ('66', '1020', '1022', '1', '0', '1524221823', '1524221904');
INSERT INTO `IMRecentSession` VALUES ('67', '1023', '1022', '1', '0', '1524222092', '1524222092');
INSERT INTO `IMRecentSession` VALUES ('68', '1022', '1023', '1', '0', '1524222092', '1524222092');
INSERT INTO `IMRecentSession` VALUES ('69', '1023', '1019', '1', '0', '1524222117', '1524222119');
INSERT INTO `IMRecentSession` VALUES ('70', '1019', '1023', '1', '0', '1524222117', '1524222119');
INSERT INTO `IMRecentSession` VALUES ('71', '1023', '1021', '1', '0', '1524222140', '1524222293');
INSERT INTO `IMRecentSession` VALUES ('72', '1021', '1023', '1', '0', '1524222140', '1524222293');
INSERT INTO `IMRecentSession` VALUES ('73', '1024', '1022', '1', '0', '1524222390', '1524222501');
INSERT INTO `IMRecentSession` VALUES ('74', '1022', '1024', '1', '0', '1524222390', '1524222501');

-- ----------------------------
-- Table structure for IMRedpack
-- ----------------------------
DROP TABLE IF EXISTS `IMRedpack`;
CREATE TABLE `IMRedpack` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `sender` int(11) unsigned NOT NULL COMMENT '红包发送者',
  `currency` int(11) unsigned NOT NULL COMMENT '金额',
  `packets` int(11) unsigned NOT NULL COMMENT '个数',
  `content` varchar(64) DEFAULT NULL COMMENT '祝福语',
  `tar_type` tinyint(2) unsigned NOT NULL COMMENT '0:用户 1:群组',
  `tar_id` int(11) unsigned NOT NULL COMMENT '目标ID',
  `created` int(11) DEFAULT NULL COMMENT '红包创建的时间',
  `status` int(11) DEFAULT '0' COMMENT '红包状态 0:未领完 1:已领完',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8 COMMENT='红包表';

-- ----------------------------
-- Records of IMRedpack
-- ----------------------------
-- INSERT INTO `IMRedpack` VALUES ('1', '1000', '100', '1', '123321', '0', '1001', '1522662066', '1');
-- INSERT INTO `IMRedpack` VALUES ('2', '1000', '100', '1', '123321', '0', '1001', '1522662557', '1');
-- INSERT INTO `IMRedpack` VALUES ('3', '1000', '100', '1', '123321', '0', '1001', '1522663401', '1');

-- ----------------------------
-- Table structure for IMRelationShip
-- ----------------------------
DROP TABLE IF EXISTS `IMRelationShip`;
CREATE TABLE `IMRelationShip` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `smallId` int(11) unsigned NOT NULL COMMENT '用户A的id',
  `bigId` int(11) unsigned NOT NULL COMMENT '用户B的id',
  `status` tinyint(1) unsigned DEFAULT '0' COMMENT '用户:0-正常, 1-用户A删除,群组:0-正常, 1-被删除',
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `idx_smallId_bigId_status_updated` (`smallId`,`bigId`,`status`,`updated`)
) ENGINE=InnoDB AUTO_INCREMENT=37 DEFAULT CHARSET=utf8;


-- ----------------------------
-- Table structure for IMRPClaimRecord
-- ----------------------------
DROP TABLE IF EXISTS `IMRPClaimRecord`;
CREATE TABLE `IMRPClaimRecord` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `pack_id` int(11) unsigned NOT NULL COMMENT '红包ID',
  `user_id` int(11) unsigned NOT NULL COMMENT '用户ID',
  `currency` int(11) unsigned NOT NULL COMMENT '领取金额',
  `last_num` int(11) unsigned NOT NULL COMMENT '领取后红包剩余个数',
  `last_curr` int(11) unsigned NOT NULL COMMENT '领取后红包剩余金额',
  `created` int(11) DEFAULT NULL COMMENT '领取的时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=100000 DEFAULT CHARSET=utf8 COMMENT='红包领取记录表';

-- ----------------------------
-- Records of IMRPClaimRecord
-- ----------------------------

-- ----------------------------
-- Table structure for IMUser
-- ----------------------------
DROP TABLE IF EXISTS `IMUser`;
CREATE TABLE `IMUser` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT '用户id',
  `sex` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '1男2女0未知',
  `name` varchar(32) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '用户名',
  `domain` varchar(32) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '拼音',
  `nick` varchar(32) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '花名,绰号等',
  `password` varchar(32) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '密码',
  `salt` varchar(4) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '混淆码',
  `phone` varchar(11) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '手机号码',
  `email` varchar(64) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT 'email',
  `avatar` varchar(255) COLLATE utf8_general_ci DEFAULT '' COMMENT '自定义用户头像',
  `departId` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '所属部门Id',
  `binded` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '绑定代理id',
  `memberorder` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '会员权限 1:ADMIN 2:PROXY 3:USER',
  `status` tinyint(2) unsigned DEFAULT '0' COMMENT '1. 试用期 2. 正式 3. 离职 4.实习',
  `currency` int(11) unsigned DEFAULT '0' COMMENT '货币金额',
  `freeze` int(11) unsigned DEFAULT '0' COMMENT '冻结了的货币金额',
  `created` int(11) unsigned NOT NULL COMMENT '创建时间',
  `updated` int(11) unsigned NOT NULL COMMENT '更新时间',
  `push_shield_status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0关闭勿扰 1开启勿扰',
  `sign_info` varchar(128) COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '个性签名',
  `birthday` int(11) unsigned NOT NULL COMMENT '生日',
  PRIMARY KEY (`id`),
  KEY `idx_domain` (`domain`),
  KEY `idx_name` (`name`),
  KEY `idx_phone` (`phone`)
) ENGINE=InnoDB AUTO_INCREMENT=10001 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

