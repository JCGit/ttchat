#include "../ProxyConn.h"
#include "../CachePool.h"
#include "../DBPool.h"
#include "MessageContent.h"
#include "MessageModel.h"
#include "GroupMessageModel.h"
#include "Common.h"
#include "GroupModel.h"
#include "UserModel.h"
#include "ImPduBase.h"
#include "IM.Message.pb.h"
#include "IM.Group.pb.h"
#include "SessionModel.h"
#include "ProxyModel.h"
#include "RelationModel.h"
#include "json/json.h"
#include "uchat/errno.hpp"
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
using namespace uchat;
namespace DB_PROXY
{
constexpr char const* kN = "db_proxy_server";

    void getMessage(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::Message::IMGetMsgListReq msg;
  if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.user_id();
            uint32_t nPeerId = msg.session_id();
            uint32_t nMsgId = msg.msg_id_begin();
            uint32_t nMsgCnt = msg.msg_cnt();
            IM::BaseDefine::SessionType nSessionType = msg.session_type();
            if(IM::BaseDefine::SessionType_IsValid(nSessionType))
            {
                CImPdu* pPduResp = new CImPdu;
                IM::Message::IMGetMsgListRsp msgResp;

                list<IM::BaseDefine::MsgInfo> lsMsg;

                if(nSessionType == IM::BaseDefine::SESSION_TYPE_SINGLE)//获取个人消息
                {
                    CMessageModel::getInstance()->getMessage(nUserId, nPeerId, nMsgId, nMsgCnt, lsMsg);
                }
                else if(nSessionType == IM::BaseDefine::SESSION_TYPE_GROUP)//获取群消息
                {
                    if(CGroupModel::getInstance()->isInGroup(nUserId, nPeerId))
                    {
                        CGroupMessageModel::getInstance()->getMessage(nUserId, nPeerId, nMsgId, nMsgCnt, lsMsg);
                    }
                }

                msgResp.set_user_id(nUserId);
                msgResp.set_session_id(nPeerId);
                msgResp.set_msg_id_begin(nMsgId);
                msgResp.set_session_type(nSessionType);
                for(auto it=lsMsg.begin(); it!=lsMsg.end();++it)
                {
                    IM::BaseDefine::MsgInfo* pMsg = msgResp.add_msg_list();
        //            *pMsg = *it;
                    pMsg->set_msg_id(it->msg_id());
                    pMsg->set_from_session_id(it->from_session_id());
                    pMsg->set_create_time(it->create_time());
                    pMsg->set_msg_type(it->msg_type());
                    pMsg->set_msg_data(it->msg_data());
//                    log("userId=%u, peerId=%u, msgId=%u", nUserId, nPeerId, it->msg_id());
                }

                log("userId=%u, peerId=%u, msgId=%u, msgCnt=%u, count=%u", nUserId, nPeerId, nMsgId, nMsgCnt, msgResp.msg_list_size());
                msgResp.set_attach_data(msg.attach_data());
                pPduResp->SetPBMsg(&msgResp);
                pPduResp->SetSeqNum(pPdu->GetSeqNum());
                pPduResp->SetServiceId(IM::BaseDefine::SID_MSG);
                pPduResp->SetCommandId(IM::BaseDefine::CID_MSG_LIST_RESPONSE);
                CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
            }
            else
            {
                log("invalid sessionType. userId=%u, peerId=%u, msgId=%u, msgCnt=%u, sessionType=%u",
                    nUserId, nPeerId, nMsgId, nMsgCnt, nSessionType);
            }
        }
        else
        {
            log("parse pb failed");
        }
    }

void sendMessage(CImPdu* pPdu, uint32_t conn_uuid) noexcept try {
    IM::Message::IMMsgData msg;
    Assert(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()), "pb error")
    IM::BaseDefine::MsgType const msgType = msg.msg_type();
    Assert(IM::BaseDefine::MsgType_IsValid(msgType), "invalid msgType " << msgType)
    uint32_t const msgLen = msg.msg_data().length();
    Assert(0 != msgLen, "msgLen is 0")
    uint32_t const fromId = msg.from_user_id();
    uint32_t const toId = msg.to_session_id();
    uint32_t const createTime = msg.create_time();
    string const& attach_data = msg.attach_data();
    uint32_t const now = ::time(nullptr);
    uint32_t sessionId = INVALID_VALUE;
    uint32_t msgId = INVALID_VALUE;
    uint32_t peerSessionId = INVALID_VALUE;
    using MessageModel = CMessageModel;
    using GroupMessageModel = CGroupMessageModel;
    using SessionModel = CSessionModel;
    using GroupModel = CGroupModel;
    using UserModel = CUserModel;
    using RelationModel = CRelationModel;
    MessageModel* msgModel = MessageModel::getInstance();
    GroupMessageModel* groupMsgModel = GroupMessageModel::getInstance();
    switch(msgType) {
    case IM::BaseDefine::MSG_TYPE_GROUP_TEXT: {
        GroupModel* groupModel = GroupModel::getInstance();
        Assert(groupModel->isValidateGroupId(toId) && groupModel->isInGroup(fromId, toId), "invalid dest gid " << toId << " or from gid " << fromId)
        sessionId = SessionModel::getInstance()->getSessionId(fromId, toId, IM::BaseDefine::SESSION_TYPE_GROUP, false);
        if (INVALID_VALUE == sessionId) {
            sessionId = SessionModel::getInstance()->addSession(fromId, toId, IM::BaseDefine::SESSION_TYPE_GROUP);
        }
        Assert(sessionId != INVALID_VALUE, "cannot get valid sessionId for MSG_TYPE_GROUP_TEXT: " << fromId << " -> " << toId)
        msgId = groupMsgModel->getMsgId(toId);
        Assert(msgId != INVALID_VALUE, "cannot get valid msgId by dest id " << toId)
        groupMsgModel->sendMessage(fromId, toId, msgType, createTime, msgId, msg.msg_data());
        SessionModel::getInstance()->updateSession(sessionId, now);
    } break;
    case IM::BaseDefine::MSG_TYPE_GROUP_AUDIO: {
        GroupModel* groupModel = GroupModel::getInstance();
        Assert(groupModel->isValidateGroupId(toId) && groupModel->isInGroup(fromId, toId), "invalid dest gid " << toId << " or from gid " << fromId)
        sessionId = SessionModel::getInstance()->getSessionId(fromId, toId, IM::BaseDefine::SESSION_TYPE_GROUP, false);
        if (INVALID_VALUE == sessionId) {
            sessionId = SessionModel::getInstance()->addSession(fromId, toId, IM::BaseDefine::SESSION_TYPE_GROUP);
        }
        Assert(sessionId != INVALID_VALUE, "cannot get valid sessionId for MSG_TYPE_GROUP_AUDIO: " << fromId << " -> " << toId)
        msgId = groupMsgModel->getMsgId(toId);
        Assert(msgId != INVALID_VALUE, "cannot get valid msgId by dest id " << toId)
        groupMsgModel->sendAudioMessage(fromId, toId, msgType, createTime, msgId, msg.msg_data().c_str(), msgLen);
        SessionModel::getInstance()->updateSession(sessionId, now);
    } break;
    case IM::BaseDefine::MSG_TYPE_SINGLE_TEXT: {
        // Chatable valid
        Assert(fromId != toId, "cannot send to self " << fromId)
        Assert(proto::code::Valid == UserModel::getInstance()->validSingleChatable(fromId, toId), fromId << " cannot chat to " << toId)
        // Get session and send
        sessionId = SessionModel::getInstance()->getSessionId(fromId, toId, IM::BaseDefine::SESSION_TYPE_SINGLE, false);
        if (INVALID_VALUE == sessionId) {
            sessionId = SessionModel::getInstance()->addSession(fromId, toId, IM::BaseDefine::SESSION_TYPE_SINGLE);
        }
        peerSessionId = SessionModel::getInstance()->getSessionId(toId, fromId, IM::BaseDefine::SESSION_TYPE_SINGLE, false);
        if (INVALID_VALUE ==  peerSessionId) {
            peerSessionId = SessionModel::getInstance()->addSession(toId, fromId, IM::BaseDefine::SESSION_TYPE_SINGLE);
        }
        uint32_t const relateId = RelationModel::getInstance()->getRelationId(fromId, toId, true);
        Assert(sessionId != INVALID_VALUE && relateId != INVALID_VALUE, "sessionId or relateId invalid " << sessionId << " " << relateId)
        msgId = msgModel->getMsgId(relateId);
        Assert(msgId != INVALID_VALUE, "cannot get msgId by relateId " << relateId)
        msgModel->sendMessage(relateId, fromId, toId, msgType, createTime, msgId, msg.msg_data());
        SessionModel::getInstance()->updateSession(sessionId, now);
        SessionModel::getInstance()->updateSession(peerSessionId, now);
    } break;
    case IM::BaseDefine::MSG_TYPE_SINGLE_AUDIO: {
        // Chatable valid
        Assert(fromId != toId, "cannot send to self " << fromId)
        Assert(proto::code::Valid == UserModel::getInstance()->validSingleChatable(fromId, toId), fromId << " cannot chat to " << toId)
        sessionId = SessionModel::getInstance()->getSessionId(fromId, toId, IM::BaseDefine::SESSION_TYPE_SINGLE, false);
        if (INVALID_VALUE == sessionId) {
            sessionId = SessionModel::getInstance()->addSession(fromId, toId, IM::BaseDefine::SESSION_TYPE_SINGLE);
        }
        peerSessionId = SessionModel::getInstance()->getSessionId(toId, fromId, IM::BaseDefine::SESSION_TYPE_SINGLE, false);
        if (INVALID_VALUE ==  peerSessionId) {
            peerSessionId = CSessionModel::getInstance()->addSession(toId, fromId, IM::BaseDefine::SESSION_TYPE_SINGLE);
        }
        uint32_t const relateId = CRelationModel::getInstance()->getRelationId(fromId, toId, true);
        Assert(sessionId != INVALID_VALUE && relateId != INVALID_VALUE, "sessionId or relateId invalid " << sessionId << " " << relateId)
        msgId = msgModel->getMsgId(relateId);
        Assert(msgId != INVALID_VALUE, "cannot get msgId by relateId " << relateId)
        msgModel->sendAudioMessage(relateId, fromId, toId, msgType, createTime, msgId, msg.msg_data().c_str(), msgLen);
        SessionModel::getInstance()->updateSession(sessionId, now);
        SessionModel::getInstance()->updateSession(peerSessionId, now);
    } break;
    default: Assert(false, "unhandled message type " << msgType) break;
    }
    log("fromId=%u, toId=%u, type=%u, msgId=%u, sessionId=%u", fromId, toId, msgType, msgId, sessionId);
    // Response
    msg.set_msg_id(msgId);
    CImPdu* pPduResp = new CImPdu;
    pPduResp->SetPBMsg(&msg);
    pPduResp->SetSeqNum(pPdu->GetSeqNum());
    pPduResp->SetServiceId(IM::BaseDefine::SID_MSG);
    pPduResp->SetCommandId(IM::BaseDefine::CID_MSG_DATA);
    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
} catch(std::exception const& e) {
    Error0(e.what())
}

    void getMessageById(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::Message::IMGetMsgByIdReq msg;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.user_id();
            IM::BaseDefine::SessionType nType = msg.session_type();
            uint32_t nPeerId = msg.session_id();
            list<uint32_t> lsMsgId;
            uint32_t nCnt = msg.msg_id_list_size();
            for(uint32_t i=0; i<nCnt; ++i)
            {
                lsMsgId.push_back(msg.msg_id_list(i));
            }
            if (IM::BaseDefine::SessionType_IsValid(nType))
            {
                CImPdu* pPduResp = new CImPdu;
                IM::Message::IMGetMsgByIdRsp msgResp;

                list<IM::BaseDefine::MsgInfo> lsMsg;
                if(IM::BaseDefine::SESSION_TYPE_SINGLE == nType)
                {
                    CMessageModel::getInstance()->getMsgByMsgId(nUserId, nPeerId, lsMsgId, lsMsg);
                }
                else if(IM::BaseDefine::SESSION_TYPE_GROUP)
                {
                    CGroupMessageModel::getInstance()->getMsgByMsgId(nUserId, nPeerId, lsMsgId, lsMsg);
                }
                msgResp.set_user_id(nUserId);
                msgResp.set_session_id(nPeerId);
                msgResp.set_session_type(nType);
                for(auto it=lsMsg.begin(); it!=lsMsg.end(); ++it)
                {
                    IM::BaseDefine::MsgInfo* pMsg = msgResp.add_msg_list();
                    pMsg->set_msg_id(it->msg_id());
                    pMsg->set_from_session_id(it->from_session_id());
                    pMsg->set_create_time(it->create_time());
                    pMsg->set_msg_type(it->msg_type());
                    pMsg->set_msg_data(it->msg_data());
                }
                log("userId=%u, peerId=%u, sessionType=%u, reqMsgCnt=%u, resMsgCnt=%u", nUserId, nPeerId, nType, msg.msg_id_list_size(), msgResp.msg_list_size());
                msgResp.set_attach_data(msg.attach_data());
                pPduResp->SetPBMsg(&msgResp);
                pPduResp->SetSeqNum(pPdu->GetSeqNum());
                pPduResp->SetServiceId(IM::BaseDefine::SID_MSG);
                pPduResp->SetCommandId(IM::BaseDefine::CID_MSG_GET_BY_MSG_ID_RES);
                CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
            }
            else
            {
                log("invalid sessionType. fromId=%u, toId=%u, sessionType=%u, msgCnt=%u", nUserId, nPeerId, nType, nCnt);
            }
        }
        else
        {
            log("parse pb failed");
        }
    }

    void getLatestMsgId(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::Message::IMGetLatestMsgIdReq msg;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.user_id();
            IM::BaseDefine::SessionType nType = msg.session_type();
            uint32_t nPeerId = msg.session_id();
            if (IM::BaseDefine::SessionType_IsValid(nType)) {
                CImPdu* pPduResp = new CImPdu;
                IM::Message::IMGetLatestMsgIdRsp msgResp;
                msgResp.set_user_id(nUserId);
                msgResp.set_session_type(nType);
                msgResp.set_session_id(nPeerId);
                uint32_t nMsgId = INVALID_VALUE;
                if(IM::BaseDefine::SESSION_TYPE_SINGLE == nType)
                {
                    string strMsg;
                    IM::BaseDefine::MsgType nMsgType;
                    CMessageModel::getInstance()->getLastMsg(nUserId, nPeerId, nMsgId, strMsg, nMsgType, 1);
                }
                else
                {
                    string strMsg;
                    IM::BaseDefine::MsgType nMsgType;
                    uint32_t nFromId = INVALID_VALUE;
                    CGroupMessageModel::getInstance()->getLastMsg(nPeerId, nMsgId, strMsg, nMsgType, nFromId);
                }
                msgResp.set_latest_msg_id(nMsgId);
                log("userId=%u, peerId=%u, sessionType=%u, msgId=%u", nUserId, nPeerId, nType,nMsgId);
                msgResp.set_attach_data(msg.attach_data());
                pPduResp->SetPBMsg(&msgResp);
                pPduResp->SetSeqNum(pPdu->GetSeqNum());
                pPduResp->SetServiceId(IM::BaseDefine::SID_MSG);
                pPduResp->SetCommandId(IM::BaseDefine::CID_MSG_GET_LATEST_MSG_ID_RSP);
                CProxyConn::AddResponsePdu(conn_uuid, pPduResp);

            }
            else
            {
                log("invalid sessionType. userId=%u, peerId=%u, sessionType=%u", nUserId, nPeerId, nType);
            }
        }
        else
        {
            log("parse pb failed");
        }
    }
};
