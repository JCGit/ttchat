#include "../ProxyConn.h"
#include "../CachePool.h"
#include "MessageCounter.h"
#include "MessageModel.h"
#include "GroupMessageModel.h"
#include "IM.Message.pb.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Login.pb.h"
#include "IM.Server.pb.h"
#include "UserModel.h"
#include "ProxyModel.h"
#include <time.h>
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
namespace uchat
{
namespace pushdb
{
constexpr char const* kN = "db_proxy_server";
using UserModel = CUserModel;
using GroupModel = CGroupModel;
using MessageModel = CMessageModel;
using GroupMessageModel = CGroupMessageModel;
#if !UCHAT_USE_FULL_OLD_SHIELD
void HandleGetDevicesToken(Packet* pkt, uint32_t connId) noexcept try {
    IM::Server::IMGetDeviceTokenReq msg;
    Assert(msg.ParseFromArray(pkt->GetBodyData(), pkt->GetBodyLength()),
        "pb fail")
    int64_t groupId = -1;// default not group
    if (msg.has_attach_data()) {
        IM::Message::IMMsgData msg2;
        if (msg2.ParseFromArray(
            msg.attach_data().c_str(), msg.attach_data().length())) {
            auto const t = msg2.msg_type();
            if ((IM::BaseDefine::MsgType::MSG_TYPE_GROUP_TEXT == t)
                || (IM::BaseDefine::MsgType::MSG_TYPE_GROUP_AUDIO == t)
                || (IM::BaseDefine::MsgType::MSG_TYPE_GROUP_REDPACK == t)) {
                groupId = msg2.to_session_id();
            }
        }
    }
    Debug(__func__ << ": is group msg " << (groupId >= 0))
    CacheManager* const cacheManager = CacheManager::getInstance();
    CacheConn* const cacheConn0 = cacheManager->GetCacheConn("token");
    Assert(cacheConn0, "token cache conn fail")
    std::shared_ptr<CacheConn> cacheConn(cacheConn0,
        [&cacheManager](CacheConn*& p) {
        if (p) {
            cacheManager->RelCacheConn(p);
            p = nullptr;
        }
    });
    // 对于ios 不推送
    // 对于android 由客户端处理
#   if UCHAT_HAS_GLOBAL_SHIELD_DURATION
    time_t now = ::time(nullptr);
    struct tm ctm;
    Assert(::localtime_r(&now, &ctm), "time error")
    bool isCheckShield = false;
    if (ctm.tm_hour >= uchat::conf::kGlobalShieldDurationBegin
        || ctm.tm_hour <= uchat::conf::kGlobalShieldDurationEnd) {
        isCheckShield = true;
    } else {
        isCheckShield = false;
    }
#   endif
    std::vector<string> tokenKeys;
    auto const& ids = msg.user_id();
    tokenKeys.reserve(ids.size());
    for (auto const& id : ids) {
        std::string key = "device_" + std::to_string(id);
        tokenKeys.emplace_back(std::move(key));
    }
    std::map<string, string> tokens;
    Assert(cacheConn->mget(tokenKeys, tokens), "mget fail")
    cacheConn = nullptr;
    IM::Server::IMGetDeviceTokenRsp respMsg;
    for (auto const& token0 : tokens) {
        std::string const& key = token0.first;
        size_t pos = key.find("device_");
        if (std::string::npos == pos) {
            Warning(__func__ << ": skip invalid key " << key)
            continue;
        }
        std::string const userId0 = key.substr(pos + ::strlen("device_"));
        uint32_t const userId = ::atoi(userId0.c_str());
        uint32_t shield = 0;
#       if UCHAT_HAS_GLOBAL_SHIELD_DURATION
        // Global shield: 过滤出已经设置勿打扰并且为晚上 ..:00 ~ ..:00
        // Global for non-group(use global only) and group
        // (global first then group)
        if (isCheckShield) {
            UserModel::getInstance()->getShield(userId, shield);
        }
        if (!shield && groupId >= 0) {
            // Global is false -> Check group message shield
            GroupModel::getInstance()->getShield(groupId, userId, shield);
        }
#       else
        if (groupId < 0) {
            // Non-group message
            UserModel::getInstance()->getShield(userId, shield);
        } else {
            // Group message
            GroupModel::getInstance()->fetchS(groupId, userId, shield);
        }
#       endif // if UCHAT_HAS_GLOBAL_SHIELD_DURATION else
        if (shield) {
            Debug(__func__ << ": shield is enabled => no push")
            continue;
        }
        std::string const& value = token0.second;
        pos = value.find(":");
        if (std::string::npos == pos) {
            Warning(__func__ << ": skip invalid value " << value)
            continue;
        }
        std::string const type = value.substr(0, pos);
        IM::BaseDefine::ClientType clientType;
        if ("ios" == type) {
            clientType = IM::BaseDefine::CLIENT_TYPE_IOS;
        } else if ("android" == type) {
            clientType = IM::BaseDefine::CLIENT_TYPE_ANDROID;
        } else {
            clientType = IM::BaseDefine::ClientType(0);
        }
        if (!IM::BaseDefine::ClientType_IsValid(clientType)) {
            Warning(__func__ << ": skip invalid clientType " << clientType)
            continue;
        }
        {
            IM::BaseDefine::UserTokenInfo* p = respMsg.add_user_token_info();
            p->set_user_id(userId);
            std::string const token = value.substr(pos + 1);
            p->set_token(token);
            p->set_user_type(clientType);
            uint32_t totalCnt = 0;
            MessageModel::getInstance()->getUnReadCntAll(userId, totalCnt);
            GroupMessageModel::getInstance()->getUnReadCntAll(userId, totalCnt);
            p->set_push_count(totalCnt);
            p->set_push_type(1);
        }
    }
    if (msg.has_attach_data()) {
        respMsg.set_attach_data(msg.attach_data());
    }
    Packet* resp = new Packet();
    Debug(__func__ << ": req devices token. reqCnt = " << ids.size()
        << " resCnt = " << respMsg.user_token_info_size())
    resp->SetPBMsg(&respMsg);
    resp->SetSeqNum(pkt->GetSeqNum());
    resp->SetServiceId(IM::BaseDefine::SID_OTHER);
    resp->SetCommandId(IM::BaseDefine::CID_OTHER_GET_DEVICE_TOKEN_RSP);
    CProxyConn::AddResponsePdu(connId, resp);
} catch(std::exception const& e) {
    Error0(e.what())
}
#endif // if !UCHAT_USE_FULL_OLD_SHIELD
}
}

namespace DB_PROXY {

    void getUnreadMsgCounter(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::Message::IMUnreadMsgCntReq msg;
        IM::Message::IMUnreadMsgCntRsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            CImPdu* pPduResp = new CImPdu;

            uint32_t nUserId = msg.user_id();

            list<IM::BaseDefine::UnreadInfo> lsUnreadCount;
            uint32_t nTotalCnt = 0;
            
            CMessageModel::getInstance()->getUnreadMsgCount(nUserId, nTotalCnt, lsUnreadCount);
            CGroupMessageModel::getInstance()->getUnreadMsgCount(nUserId, nTotalCnt, lsUnreadCount);
            CProxyModel::getInstance()->getUnreadBindCount(nUserId, nTotalCnt, lsUnreadCount);
            CProxyModel::getInstance()->getUnreadAggreFriendCount(nUserId, nTotalCnt, lsUnreadCount);
            
            msgResp.set_user_id(nUserId);
            msgResp.set_total_cnt(nTotalCnt);
            for(auto it= lsUnreadCount.begin(); it!=lsUnreadCount.end(); ++it)
            {
                IM::BaseDefine::UnreadInfo* pInfo = msgResp.add_unreadinfo_list();
    //            *pInfo = *it;
                pInfo->set_session_id(it->session_id());
                pInfo->set_session_type(it->session_type());
                pInfo->set_unread_cnt(it->unread_cnt());
                pInfo->set_latest_msg_id(it->latest_msg_id());
                pInfo->set_latest_msg_data(it->latest_msg_data());
                pInfo->set_latest_msg_type(it->latest_msg_type());
                pInfo->set_latest_msg_from_user_id(it->latest_msg_from_user_id());
            }
            
            
            log("userId=%d, unreadCnt=%u, totalCount=%u", nUserId, msgResp.unreadinfo_list_size(), nTotalCnt);
            msgResp.set_attach_data(msg.attach_data());
            pPduResp->SetPBMsg(&msgResp);
            pPduResp->SetSeqNum(pPdu->GetSeqNum());
            pPduResp->SetServiceId(IM::BaseDefine::SID_MSG);
            pPduResp->SetCommandId(IM::BaseDefine::CID_MSG_UNREAD_CNT_RESPONSE);
            CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
        }
        else
        {
            log("parse pb failed");
        }
    }

    void clearUnreadMsgCounter(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::Message::IMMsgDataReadAck msg;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.user_id();
            uint32_t nFromId = msg.session_id();
            IM::BaseDefine::SessionType nSessionType = msg.session_type();
            CUserModel::getInstance()->clearUserCounter(nUserId, nFromId, nSessionType);
            log("userId=%u, peerId=%u, type=%u", nFromId, nUserId, nSessionType);
        }
        else
        {
            log("parse pb failed");
        }
    }
        
    void setDevicesToken(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::Login::IMDeviceTokenReq msg;
        IM::Login::IMDeviceTokenRsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.user_id();
            string strToken = msg.device_token();
            CImPdu* pPduResp = new CImPdu;
            CacheManager* pCacheManager = CacheManager::getInstance();
            CacheConn* pCacheConn = pCacheManager->GetCacheConn("token");
            if (pCacheConn)
            {
                IM::BaseDefine::ClientType nClientType = msg.client_type();
                string strValue;
                if(IM::BaseDefine::CLIENT_TYPE_IOS == nClientType)
                {
                    strValue = "ios:"+strToken;
                }
                else if(IM::BaseDefine::CLIENT_TYPE_ANDROID == nClientType)
                {
                    strValue = "android:"+strToken;
                }
                else
                {
                    strValue = strToken;
                }
                
                string strOldValue = pCacheConn->get("device_"+int2string(nUserId));
                
                if(!strOldValue.empty())
                {
                    size_t nPos = strOldValue.find(":");
                    if(nPos!=string::npos)
                    {
                        string strOldToken = strOldValue.substr(nPos + 1);
                        string strReply = pCacheConn->get("device_"+strOldToken);
                        if (!strReply.empty()) {
                            string strNewValue("");
                            pCacheConn->set("device_"+strOldToken, strNewValue);
                        }
                    }
                }
                
                pCacheConn->set("device_"+int2string(nUserId), strValue);
                string strNewValue = int2string(nUserId);
                pCacheConn->set("device_"+strToken, strNewValue);
            
                log("setDeviceToken. userId=%u, deviceToken=%s", nUserId, strToken.c_str());
                pCacheManager->RelCacheConn(pCacheConn);
            }
            else
            {
                log("no cache connection for token");
            }
            
            log("setDeviceToken. userId=%u, deviceToken=%s", nUserId, strToken.c_str());
            msgResp.set_attach_data(msg.attach_data());
            msgResp.set_user_id(nUserId);
            pPduResp->SetPBMsg(&msgResp);
            pPduResp->SetSeqNum(pPdu->GetSeqNum());
            pPduResp->SetServiceId(IM::BaseDefine::SID_LOGIN);
            pPduResp->SetCommandId(IM::BaseDefine::CID_LOGIN_RES_DEVICETOKEN);
            CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
        }
        else
        {
            log("parse pb failed");
        }
    }


    void getDevicesToken(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::Server::IMGetDeviceTokenReq msg;
        IM::Server::IMGetDeviceTokenRsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            CacheManager* pCacheManager = CacheManager::getInstance();
            CacheConn* pCacheConn = pCacheManager->GetCacheConn("token");
            CImPdu* pPduResp = new CImPdu;
            uint32_t nCnt = msg.user_id_size();
            
            // 对于ios，不推送
            // 对于android，由客户端处理
            bool is_check_shield_status = false;
            time_t now = time(NULL);
            struct tm* _tm = localtime(&now);
            if (_tm->tm_hour >= 22 || _tm->tm_hour <=7 ) {
                    is_check_shield_status = true;
                }
            if (pCacheConn)
            {
                vector<string> vecTokens;
                for (uint32_t i=0; i<nCnt; ++i) {
                    string strKey = "device_"+int2string(msg.user_id(i));
                    vecTokens.push_back(strKey);
                }
                map<string, string> mapTokens;
                bool bRet = pCacheConn->mget(vecTokens, mapTokens);
                pCacheManager->RelCacheConn(pCacheConn);
                
                if(bRet)
                {
                    for (auto it=mapTokens.begin(); it!=mapTokens.end(); ++it) {
                        string strKey = it->first;
                        size_t nPos = strKey.find("device_");
                        if( nPos != string::npos)
                        {
                            string strUserId = strKey.substr(nPos + strlen("device_"));
                            uint32_t nUserId = string2int(strUserId);
                            string strValue = it->second;
                            nPos = strValue.find(":");
                            if(nPos!=string::npos)
                            {
                                string strType = strValue.substr(0, nPos);
                                string strToken = strValue.substr(nPos + 1);
                                IM::BaseDefine::ClientType nClientType = IM::BaseDefine::ClientType(0);
                                if(strType == "ios")
                                {
                                    // 过滤出已经设置勿打扰并且为晚上22：00～07：00
                                    uint32_t shield_status = 0;
                                    if (is_check_shield_status) {
                                        CUserModel::getInstance()->getPushShield(nUserId, &shield_status);
                                    }
                                    
                                    if (shield_status == 1) {
                                        // 对IOS处理
                                        continue;
                                    } else {
                                        nClientType = IM::BaseDefine::CLIENT_TYPE_IOS;
                                    }
                                    
                                    // nClientType = IM::BaseDefine::CLIENT_TYPE_IOS;
                                    // end
                                }
                                else if(strType == "android")
                                {
                                    nClientType = IM::BaseDefine::CLIENT_TYPE_ANDROID;
                                }
                                if(IM::BaseDefine::ClientType_IsValid(nClientType))
                                {
                                    IM::BaseDefine::UserTokenInfo* pToken = msgResp.add_user_token_info();
                                    pToken->set_user_id(nUserId);
                                    pToken->set_token(strToken);
                                    pToken->set_user_type(nClientType);
                                    uint32_t nTotalCnt = 0;
                                    CMessageModel::getInstance()->getUnReadCntAll(nUserId, nTotalCnt);
                                    CGroupMessageModel::getInstance()->getUnReadCntAll(nUserId, nTotalCnt);
                                    pToken->set_push_count(nTotalCnt);
                                    pToken->set_push_type(1);
                                }
                                else
                                {
                                    log("invalid clientType.clientType=%u", nClientType);
                                }
                            }
                            else
                            {
                                log("invalid value. value=%s", strValue.c_str());
                            }
                            
                        }
                        else
                        {
                            log("invalid key.key=%s", strKey.c_str());
                        }
                    }
                }
                else
                {
                    log("mget failed!");
                }
            }
            else
            {
                log("no cache connection for token");
            }
            
            log("req devices token.reqCnt=%u, resCnt=%u", nCnt, msgResp.user_token_info_size());
            
            msgResp.set_attach_data(msg.attach_data());
            pPduResp->SetPBMsg(&msgResp);
            pPduResp->SetSeqNum(pPdu->GetSeqNum());
            pPduResp->SetServiceId(IM::BaseDefine::SID_OTHER);
            pPduResp->SetCommandId(IM::BaseDefine::CID_OTHER_GET_DEVICE_TOKEN_RSP);
            CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
        }
        else
        {
            log("parse pb failed");
        }
    }
};


