/*================================================================
 *     Copyright (c) 2014年 lanhu. All rights reserved.
 *
 *   文件名称：HandlerMap.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月02日
 *   描    述：
 *
 ================================================================*/

#include "HandlerMap.h"

#include "business/Login.h"
#include "business/MessageContent.h"
#include "business/RecentSession.h"
#include "business/UserAction.h"
#include "business/MessageCounter.h"
#include "business/GroupAction.h"
#include "business/DepartAction.h"
#include "business/FileAction.h"
#include "business/Proxy.h"
#include "IM.BaseDefine.pb.h"

using namespace IM::BaseDefine;


CHandlerMap* CHandlerMap::s_handler_instance = NULL;

/**
 *  构造函数
 */
CHandlerMap::CHandlerMap()
{

}

/**
 *  析构函数
 */
CHandlerMap::~CHandlerMap()
{

}

/**
 *  单例
 *
 *  @return 返回指向CHandlerMap的单例指针
 */
CHandlerMap* CHandlerMap::getInstance()
{
	if (!s_handler_instance) {
		s_handler_instance = new CHandlerMap();
		s_handler_instance->Init();
	}

	return s_handler_instance;
}

/**
 *  初始化函数,加载了各种commandId 对应的处理函数
 */
void CHandlerMap::Init()
{
    // Register 
    m_handler_map.insert(make_pair(uint32_t(CID_LOGIN_REQ_REGISTER), DB_PROXY::doRegister));
    
	// Login validate
	m_handler_map.insert(make_pair(uint32_t(CID_OTHER_VALIDATE_REQ), DB_PROXY::doLogin));
    m_handler_map.insert(make_pair(uint32_t(CID_LOGIN_REQ_PUSH_SHIELD), DB_PROXY::doPushShield));
    m_handler_map.insert(make_pair(uint32_t(CID_LOGIN_REQ_QUERY_PUSH_SHIELD), DB_PROXY::doQueryPushShield));
    
    // recent session
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_RECENT_CONTACT_SESSION_REQUEST), DB_PROXY::getRecentSession));
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_REMOVE_SESSION_REQ), DB_PROXY::deleteRecentSession));
    
    // users
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_USER_INFO_REQUEST), DB_PROXY::getUserInfo));
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_ALL_USER_REQUEST), DB_PROXY::getChangedUser));
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_DEPARTMENT_REQUEST), DB_PROXY::getChgedDepart));
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_CHANGE_SIGN_INFO_REQUEST), DB_PROXY::changeUserSignInfo));
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_CHANGE_INFO_REQUEST), DB_PROXY::changeUserInfo));
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_CHANGE_AVATAR_REQUEST), DB_PROXY::changeUserAvatar));
    this->m_handler_map.insert(std::make_pair(uint32_t(
        IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_PUB_USER_INFO_REQUEST),
        DB_PROXY::QueryPublicUserInfo));
    
    // message content
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_DATA), DB_PROXY::sendMessage));
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_LIST_REQUEST), DB_PROXY::getMessage));
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_UNREAD_CNT_REQUEST), DB_PROXY::getUnreadMsgCounter));
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_READ_ACK), DB_PROXY::clearUnreadMsgCounter));
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_GET_BY_MSG_ID_REQ), DB_PROXY::getMessageById));
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_GET_LATEST_MSG_ID_REQ), DB_PROXY::getLatestMsgId));
    
    // device token
    m_handler_map.insert(make_pair(uint32_t(CID_LOGIN_REQ_DEVICETOKEN), DB_PROXY::setDevicesToken));
#   if !UCHAT_USE_FULL_OLD_SHIELD
    this->m_handler_map.insert(std::make_pair(uint32_t(
        IM::BaseDefine::OtherCmdID::CID_OTHER_GET_DEVICE_TOKEN_REQ),
        uchat::pushdb::HandleGetDevicesToken));
#   else
    m_handler_map.insert(make_pair(uint32_t(CID_OTHER_GET_DEVICE_TOKEN_REQ), DB_PROXY::getDevicesToken));
#   endif
    
    //push 推送设置
    m_handler_map.insert(make_pair(uint32_t(CID_GROUP_SHIELD_GROUP_REQUEST), DB_PROXY::setGroupPush));
    m_handler_map.insert(make_pair(uint32_t(CID_OTHER_GET_SHIELD_REQ), DB_PROXY::getGroupPush));
    
    
    // group
    this->m_handler_map.insert(std::make_pair(uint32_t(
        IM::BaseDefine::GroupCmdID::CID_GROUP_NORMAL_LIST_REQUEST),
        uchat::groupdb::GetBasicGroupInfo));
    this->m_handler_map.insert(std::make_pair(uint32_t(
        IM::BaseDefine::GroupCmdID::CID_GROUP_INFO_REQUEST),
        uchat::groupdb::GetDetailGroupInfo));
    this->m_handler_map.insert(std::make_pair(uint32_t(
        IM::BaseDefine::GroupCmdID::CID_GROUP_CREATE_REQUEST),
        uchat::groupdb::CreateGroup));
    this->m_handler_map.insert(std::make_pair(uint32_t(
        IM::BaseDefine::GroupCmdID::CID_GROUP_CHANGE_MEMBER_REQUEST),
        uchat::groupdb::ModifyMember));
    m_handler_map.insert(make_pair(uint32_t(CID_GROUP_CHANGE_PK10_RULES_REQUEST), DB_PROXY::setGroupPK10));
    m_handler_map.insert(make_pair(uint32_t(CID_GROUP_PK10_RULES_REQUEST), DB_PROXY::getGroupPk10Rule));
    m_handler_map.insert(make_pair(uint32_t(CID_GROUP_PK10_PLACE_HD_REQUEST), DB_PROXY::placeGroupPK10));
    this->m_handler_map.insert(std::make_pair(uint32_t(
        IM::BaseDefine::GroupCmdID::CID_GROUP_INFO_MODIFY_REQUEST),
        uchat::groupdb::ModifyGroupInfo));
    this->m_handler_map.insert(std::make_pair(uint32_t(
        IM::BaseDefine::GroupCmdID::CID_GROUP_SHIELD_REQUEST),
        uchat::groupdb::ModifyGroupShield));

    // file
    m_handler_map.insert(make_pair(uint32_t(CID_FILE_HAS_OFFLINE_REQ), DB_PROXY::hasOfflineFile));
    m_handler_map.insert(make_pair(uint32_t(CID_FILE_ADD_OFFLINE_REQ), DB_PROXY::addOfflineFile));
    m_handler_map.insert(make_pair(uint32_t(CID_FILE_DEL_OFFLINE_REQ), DB_PROXY::delOfflineFile));

    // proxy
    m_handler_map.insert(make_pair(uint32_t(CID_PROXY_BIND_REQ), DB_PROXY::doBindProxy)); 
    m_handler_map.insert(make_pair(uint32_t(CID_PROXY_USER_LIST_REQ), DB_PROXY::doGetUserList)); 
    m_handler_map.insert(make_pair(uint32_t(CID_FRIEND_OPERATE_REQ), DB_PROXY::doProcessFriend)); 
    m_handler_map.insert(make_pair(uint32_t(CID_PROXY_CASH_CONSUME_REQUEST), DB_PROXY::doPK10Consume)); 
    this->m_handler_map.insert(std::make_pair(uint32_t(
        IM::BaseDefine::ProxyCmdID::CID_PROXY_ENABLE_REQ),
        DB_PROXY::doSetProxy));

    // redpack
    m_handler_map.insert(make_pair(uint32_t(CID_REDPACK_SEND_REQ), DB_PROXY::doSendRedpack)); 
    m_handler_map.insert(make_pair(uint32_t(CID_REDPACK_CLAIM_REQ), DB_PROXY::doClaimRedpack)); 

}

/**
 *  通过commandId获取处理函数
 *
 *  @param pdu_type commandId
 *
 *  @return 处理函数的函数指针
 */
pdu_handler_t CHandlerMap::GetHandler(uint32_t pdu_type)
{
	HandlerMap_t::iterator it = m_handler_map.find(pdu_type);
	if (it != m_handler_map.end()) {
		return it->second;
	} else {
		return NULL;
	}
}


