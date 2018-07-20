/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：ClientConn.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月30日
 *   描    述：
 *
 ================================================================*/
#include "ClientConn.h"
#include "playsound.h"
#include "Common.h"
#include "EncDec.h"
#include "json/json.h"

static ConnMap_t g_client_conn_map;

ClientConn::ClientConn():
m_bOpen(false)
{
    m_pSeqAlloctor = CSeqAlloctor::getInstance();
}

ClientConn::~ClientConn()
{

}

net_handle_t ClientConn::connect(const string& strIp, uint16_t nPort, const string& strName, const string& strPass, IPacketCallback* pCallback)
{
	m_handle = netlib_connect(strIp.c_str(), nPort, imconn_callback, (void*)&g_client_conn_map);
    if (m_handle != NETLIB_INVALID_HANDLE) {
        m_pCallback = pCallback;
		g_client_conn_map.insert(make_pair(m_handle, this));
	}
    return  m_handle;
}



void ClientConn::OnConfirm()
{
    printf("client connection onconfirm.");
    if(m_pCallback)
    {
        m_pCallback->onConnect();
    }
}

void ClientConn::OnClose()
{
    log("onclose from handle=%d\n", m_handle);
    Close();
}

void ClientConn::OnTimer(uint64_t curr_tick)
{
    if (curr_tick > m_last_send_tick + CLIENT_HEARTBEAT_INTERVAL) {
        CImPdu cPdu;
        IM::Other::IMHeartBeat msg;
        cPdu.SetPBMsg(&msg);
        cPdu.SetServiceId(IM::BaseDefine::SID_OTHER);
        cPdu.SetCommandId(IM::BaseDefine::CID_OTHER_HEARTBEAT);
        uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
        cPdu.SetSeqNum(nSeqNo);
        SendPdu(&cPdu);
    }
    
    if (curr_tick > m_last_recv_tick + CLIENT_TIMEOUT) {
        log("conn to msg_server timeout\n");
        Close();
    }
}

uint32_t ClientConn::regist(const string strName, const string strPass, const string strNickname, const string strBind, const string strCode)
{
    CImPdu cPdu;
    IM::Login::IMRegisterReq msg;

    msg.set_user_name(strName);

    if (!strPass.empty()){
        char szMd5[33];
        CMd5::MD5_Calculate(strPass.c_str(), strPass.length(), szMd5);

        msg.set_password(szMd5);
    }
    
    if (!strNickname.empty()){
        msg.set_nick_name(strNickname);
    }

    if (!strBind.empty()){
        msg.set_to_bind(strBind);
    }
    
    if (!strCode.empty()){
        msg.set_checkcode(strCode);
    }
    
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_LOGIN);
    cPdu.SetCommandId(IM::BaseDefine::CID_LOGIN_REQ_REGISTER);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::password(uint32_t nUserId, const string& strPass)
{
    CImPdu cPdu;
    IM::Buddy::IMChangeInfoReq msg;

    msg.set_user_id(nUserId);

    char szMd5[33];
    CMd5::MD5_Calculate(strPass.c_str(), strPass.length(), szMd5);

    msg.set_password(szMd5);
    
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
    cPdu.SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_CHANGE_INFO_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::login(const string strName, const string strPass)
{
    CImPdu cPdu;
    IM::Login::IMLoginReq msg;
    msg.set_user_name(strName);

    char szMd5[33];
    CMd5::MD5_Calculate(strPass.c_str(), strPass.length(), szMd5);
    string strOutPass(szMd5);
    msg.set_password(szMd5);
    msg.set_online_status(IM::BaseDefine::USER_STATUS_ONLINE);
    msg.set_client_type(IM::BaseDefine::CLIENT_TYPE_WINDOWS);
    msg.set_client_version("1.0");
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_LOGIN);
    cPdu.SetCommandId(IM::BaseDefine::CID_LOGIN_REQ_USERLOGIN);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::getUser(uint32_t nUserId, uint32_t nTime)
{
    CImPdu cPdu;
    IM::Buddy::IMAllUserReq msg;
    msg.set_user_id(nUserId);
    msg.set_latest_update_time(nTime);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
    cPdu.SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_ALL_USER_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::getUserInfo(uint32_t nUserId, list<uint32_t>& lsUserId)
{
    CImPdu cPdu;
    IM::Buddy::IMUsersInfoReq msg;
    msg.set_user_id(nUserId);
    for (auto it=lsUserId.begin(); it!=lsUserId.end(); ++it) {
        msg.add_user_id_list(*it);
    }
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
    cPdu.SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_USER_INFO_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::sendMessage(uint32_t nFromId, uint32_t nToId, IM::BaseDefine::MsgType nType, const string& strMsgData)
{
    CImPdu cPdu;
    IM::Message::IMMsgData msg;
    msg.set_from_user_id(nFromId);
    msg.set_to_session_id(nToId);
    msg.set_msg_id(0);
    msg.set_create_time(time(NULL));
    msg.set_msg_type(nType);
    msg.set_msg_data(strMsgData);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::CID_MSG_DATA);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::getUnreadMsgCnt(uint32_t nUserId)
{
    CImPdu cPdu;
    IM::Message::IMUnreadMsgCntReq msg;
    msg.set_user_id(nUserId);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::CID_MSG_UNREAD_CNT_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}


uint32_t ClientConn::getRecentSession(uint32_t nUserId, uint32_t nLastTime)
{
    CImPdu cPdu;
    IM::Buddy::IMRecentContactSessionReq msg;
    msg.set_user_id(nUserId);
    msg.set_latest_update_time(nLastTime);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
    cPdu.SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_RECENT_CONTACT_SESSION_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::getMsgList(uint32_t nUserId, IM::BaseDefine::SessionType nType, uint32_t nPeerId, uint32_t nMsgId, uint32_t nMsgCnt)
{
    CImPdu cPdu;
    IM::Message::IMGetMsgListReq msg;
    msg.set_user_id(nUserId);
    msg.set_session_type(nType);
    msg.set_session_id(nPeerId);
    msg.set_msg_id_begin(nMsgId);
    msg.set_msg_cnt(nMsgCnt);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::CID_MSG_LIST_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::sendMsgAck(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nType, uint32_t nMsgId)
{
    CImPdu cPdu;
    IM::Message::IMMsgDataReadAck msg;
    msg.set_user_id(nUserId);
    msg.set_session_id(nPeerId);
    msg.set_session_type(nType);
    msg.set_msg_id(nMsgId);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::CID_MSG_READ_ACK);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

void ClientConn::Close()
{
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
	}
	ReleaseRef();
}



uint32_t ClientConn::bind(uint32_t nUser, string szTarget, uint32_t nTag/*=0*/)
{
    CImPdu cPdu;
    IM::Proxy::IMProxyBindReq msg;

    msg.set_user_id(nUser);
    msg.set_user_bind(szTarget);
    msg.set_bind_tag(nTag);

    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_PROXY);
    cPdu.SetCommandId(IM::BaseDefine::CID_PROXY_BIND_REQ);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::friendReq(uint32_t nUser, uint32_t nOperate, uint32_t operate_type)
{
    CImPdu cPdu;
    IM::Proxy::IMProxyFriendReq msg;

    msg.set_user_id(nUser);
    msg.set_user_ope(nOperate);
    msg.set_ope_type(operate_type);

    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_FRIEND);
    cPdu.SetCommandId(IM::BaseDefine::CID_FRIEND_OPERATE_REQ);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::getUserList(uint32_t nUser)
{
    CImPdu cPdu;
    IM::Proxy::IMProxyUserListReq msg;

    msg.set_user_id(nUser);
    msg.set_latest_update_time(time(NULL));

    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_PROXY);
    cPdu.SetCommandId(IM::BaseDefine::CID_PROXY_USER_LIST_REQ);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::sendRedpacket(uint32_t nFromId, uint32_t nToId, IM::BaseDefine::MsgType nType, uint32_t currency, uint32_t pack_num, uint32_t pack_type, string pack_content)
{
    CImPdu cPdu;
    IM::Proxy::IMProxyRedpack msg;

    msg.set_from_user_id(nFromId);
    msg.set_to_session_id(nToId);
    msg.set_msg_id(0);
    msg.set_create_time(time(NULL));
    msg.set_msg_type(nType);

    msg.set_currency(currency);
    msg.set_pack_num(pack_num);
    msg.set_pack_type(pack_type);
    msg.set_content(pack_content);

    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_REDPACK);
    cPdu.SetCommandId(IM::BaseDefine::CID_REDPACK_SEND_REQ);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::claimPacket(uint32_t nUserId, uint32_t pack_id)
{
    CImPdu cPdu;
    IM::Proxy::IMProxyRedpackClaimReq msg;

    msg.set_user_id(nUserId);
    msg.set_pack_id(pack_id);

    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_REDPACK);
    cPdu.SetCommandId(IM::BaseDefine::CID_REDPACK_CLAIM_REQ);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::place(uint32_t nFromId, uint32_t nToId, const string& strMsg)
{
    /*CImPdu cPdu;
    IM::Group::IMGroupPK10PlaceReq msg;

    msg.set_user_id(nFromId);
    msg.set_group_id(nToId);

    Json::Reader reader;
    Json::Value value;
    if(!reader.parse(strMsg, value)){
		log("json parse error");
        return 0;
    }else{

        msg.set_rule(value["rule"].asUInt());
        msg.set_main(value["main"].asUInt());
        msg.set_sub(value["sub"].asUInt());
        msg.set_gold(value["gold"].asUInt());
        msg.set_time(value["time"].asUInt());

        cPdu.SetPBMsg(&msg);
        cPdu.SetServiceId(IM::BaseDefine::SID_GROUP);
        cPdu.SetCommandId(IM::BaseDefine::CID_GROUP_PK10_PLACE_REQUEST);
        uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
        cPdu.SetSeqNum(nSeqNo);
        SendPdu(&cPdu);
        return nSeqNo;
    }*/ 
    return 0;
}

void ClientConn::HandlePdu(CImPdu* pPdu)
{
    //printf("pdu type = %u\n", pPdu->GetPduType());
	switch (pPdu->GetCommandId()) {
        case IM::BaseDefine::CID_OTHER_HEARTBEAT:
//		printf("Heartbeat\n");
		break;
        case IM::BaseDefine::CID_LOGIN_RES_USERLOGIN:
            _HandleLoginResponse(pPdu);
		break;
        case IM::BaseDefine::CID_LOGIN_RES_REGISTER:
            _HandleRegistResponse(pPdu);
        break;
        case IM::BaseDefine::CID_BUDDY_LIST_ALL_USER_RESPONSE:
            _HandleUser(pPdu);
        break;
        case IM::BaseDefine::CID_BUDDY_LIST_USER_INFO_RESPONSE:
            _HandleUserInfo(pPdu);
        break;
        case IM::BaseDefine::CID_MSG_DATA_ACK:
            _HandleSendMsg(pPdu);
        break;
        case IM::BaseDefine::CID_MSG_UNREAD_CNT_RESPONSE:
            _HandleUnreadCnt(pPdu);
            break;
        case IM::BaseDefine::CID_BUDDY_LIST_RECENT_CONTACT_SESSION_RESPONSE:
            _HandleRecentSession(pPdu);
            break;
        case IM::BaseDefine::CID_MSG_LIST_RESPONSE:
            _HandleMsgList(pPdu);
            break;
        case IM::BaseDefine::CID_MSG_DATA:
            _HandleMsgData(pPdu);
            break;
        case IM::BaseDefine::CID_PROXY_BIND_RSP:
            _HandleBindProxyResponse(pPdu);
            break;
        case IM::BaseDefine::CID_FRIEND_OPERATE_RSP:
            _HandleFriendOperateResponse(pPdu);
            break;
        case IM::BaseDefine::CID_FRIEND_OPERATE_NOTIFY:
            _HandleFriendOperateNotify(pPdu);
            break;
        case IM::BaseDefine::CID_PROXY_USER_LIST_RSP:
            _HandleProxyUserListResponse(pPdu);
            break;
        case IM::BaseDefine::CID_REDPACK_SEND_RSP:
            _HandleSendRedpackResponse(pPdu);
            break;
        case IM::BaseDefine::CID_REDPACK_NOTIFY:
            _HandleSendRedpackNotify(pPdu);
            break;
        case IM::BaseDefine::CID_REDPACK_CLAIM_RSP:
            break;
        case IM::BaseDefine::CID_REDPACK_CLAIM_NOTIFY:
            break;
        case IM::BaseDefine::CID_GROUP_PK10_PLACE_RESPONSE:
            _HandlePK10PlaceResponse(pPdu);
            break;
        case IM::BaseDefine::CID_BUDDY_LIST_CHANGE_INFO_RESPONE:
            _HandleModifyResponse(pPdu);
            break;
        default:
		log("wrong msg_type=%d\n", pPdu->GetCommandId());
		break;
	}
}

void ClientConn::_HandleModifyResponse(CImPdu* pPdu)
{
    IM::Buddy::IMChangeInfoRsp msgrsp;
    uint32_t nSeqNo = pPdu->GetSeqNum();

    if(msgrsp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        m_pCallback->onPassword(nSeqNo, msgrsp.user_id());
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandlePK10PlaceResponse(CImPdu* pPdu)
{
    IM::Group::IMGroupPK10PlaceRsp msgrsp;
    uint32_t nSeqNo = pPdu->GetSeqNum();

    if(msgrsp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        m_pCallback->onPK10Place(nSeqNo, msgrsp.result_code(), msgrsp.user_id());
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleBindProxyResponse(CImPdu* pPdu)
{
    IM::Proxy::IMProxyBindRsp msgrsp;
    uint32_t nSeqNo = pPdu->GetSeqNum();

    if(msgrsp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        m_pCallback->onBindProxy(nSeqNo, msgrsp.user_id(), msgrsp.user_bind(), msgrsp.bind_tag(), msgrsp.result_code());
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleFriendOperateNotify(CImPdu* pPdu)
{
    IM::Proxy::IMProxyFriendNotify notify;
    uint32_t nSeqNo = pPdu->GetSeqNum();

    if(notify.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        m_pCallback->onFriend(nSeqNo, notify.user_id(), notify.user_ope(), notify.ope_type(), notify.result_code());
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleFriendOperateResponse(CImPdu* pPdu)
{
    IM::Proxy::IMProxyFriendRsp msgrsp;
    uint32_t nSeqNo = pPdu->GetSeqNum();

    if(msgrsp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        m_pCallback->onFriend(nSeqNo, msgrsp.user_id(), msgrsp.user_ope(), msgrsp.ope_type(), msgrsp.result_code());
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleRegistResponse(CImPdu* pPdu)
{
    IM::Login::IMRegisterRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        printf("regist code:%d errstr:%s.\n", msgResp.retcode(), msgResp.errstr().c_str());
        m_pCallback->onRegist(nSeqNo, msgResp.retcode());
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleLoginResponse(CImPdu* pPdu)
{
    IM::Login::IMLoginRes msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nRet = msgResp.result_code();
        string strMsg = msgResp.result_string();
        if(nRet == 0)
        {
            m_bOpen = true;
            IM::BaseDefine::UserInfo cUser = msgResp.user_info();
            m_pCallback->onLogin(nSeqNo, nRet, strMsg, &cUser);
        }
        else
        {
            m_pCallback->onLogin(nSeqNo, nRet, strMsg);
        }
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleUser(CImPdu* pPdu)
{
    IM::Buddy::IMAllUserRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t userCnt = msgResp.user_list_size();
        printf("get %d users\n", userCnt);
        list<IM::BaseDefine::UserInfo> lsUsers;
        for(uint32_t i=0; i<userCnt; ++i)
        {
            IM::BaseDefine::UserInfo cUserInfo = msgResp.user_list(i);
            lsUsers.push_back(cUserInfo);
        }
        m_pCallback->onGetChangedUser(nSeqNo, lsUsers);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleUserInfo(CImPdu* pPdu)
{
    IM::Buddy::IMUsersInfoRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t userCnt = msgResp.user_info_list_size();
        list<IM::BaseDefine::UserInfo> lsUser;
        for (uint32_t i=0; i<userCnt; ++i) {
            IM::BaseDefine::UserInfo userInfo = msgResp.user_info_list(i);
            lsUser.push_back(userInfo);
        }
        m_pCallback->onGetUserInfo(nSeqNo, lsUser);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleSendMsg(CImPdu* pPdu)
{
    IM::Message::IMMsgDataAck msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nSendId = msgResp.user_id();
        uint32_t nRecvId = msgResp.session_id();
        uint32_t nMsgId = msgResp.msg_id();
        IM::BaseDefine::SessionType nType = msgResp.session_type();
        m_pCallback->onSendMsg(nSeqNo, nSendId, nRecvId, nType, nMsgId);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}


void ClientConn::_HandleUnreadCnt(CImPdu* pPdu)
{
    IM::Message::IMUnreadMsgCntRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        list<IM::BaseDefine::UnreadInfo> lsUnreadInfo;
        uint32_t nUserId = msgResp.user_id();
        uint32_t nTotalCnt = msgResp.total_cnt();
        uint32_t nCnt = msgResp.unreadinfo_list_size();
        for (uint32_t i=0; i<nCnt; ++i) {
            IM::BaseDefine::UnreadInfo unreadInfo = msgResp.unreadinfo_list(i);
            lsUnreadInfo.push_back(unreadInfo);
        }
        m_pCallback->onGetUnreadMsgCnt(nSeqNo, nUserId, nTotalCnt, lsUnreadInfo);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb fail");
    }
}

void ClientConn::_HandleRecentSession(CImPdu *pPdu)
{
    IM::Buddy::IMRecentContactSessionRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        list<IM::BaseDefine::ContactSessionInfo> lsSession;
        uint32_t nUserId = msgResp.user_id();
        uint32_t nCnt = msgResp.contact_session_list_size();
        for (uint32_t i=0; i<nCnt; ++i) {
            IM::BaseDefine::ContactSessionInfo session = msgResp.contact_session_list(i);
            lsSession.push_back(session);
        }
        m_pCallback->onGetRecentSession(nSeqNo, nUserId, lsSession);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleMsgList(CImPdu *pPdu)
{
    IM::Message::IMGetMsgListRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nUserId= msgResp.user_id();
        IM::BaseDefine::SessionType nSessionType = msgResp.session_type();
        uint32_t nPeerId = msgResp.session_id();
        uint32_t nMsgId = msgResp.msg_id_begin();
        uint32_t nMsgCnt = msgResp.msg_list_size();
        list<IM::BaseDefine::MsgInfo> lsMsg;
        for(uint32_t i=0; i<nMsgCnt; ++i)
        {
            IM::BaseDefine::MsgInfo msgInfo = msgResp.msg_list(i);
            lsMsg.push_back(msgInfo);
        }
        m_pCallback->onGetMsgList(nSeqNo, nUserId, nPeerId, nSessionType, nMsgId, nMsgCnt, lsMsg);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb falied");
    }
}
void ClientConn::_HandleMsgData(CImPdu* pPdu)
{
    IM::Message::IMMsgData msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        //play("message.wav");
        
        uint32_t nFromId = msg.from_user_id();
        uint32_t nToId = msg.to_session_id();
        uint32_t nMsgId = msg.msg_id();
        IM::BaseDefine::MsgType nMsgType = msg.msg_type();
        uint32_t nCreateTime = msg.create_time();
        string strMsg(msg.msg_data().c_str(), msg.msg_data().length());
        
        IM::BaseDefine::SessionType nSessionType;
        if(nMsgType == IM::BaseDefine::MSG_TYPE_SINGLE_TEXT)
        {
            nSessionType = IM::BaseDefine::SESSION_TYPE_SINGLE;
        }
        else if(nMsgType == IM::BaseDefine::MSG_TYPE_SINGLE_AUDIO)
        {
            nSessionType = IM::BaseDefine::SESSION_TYPE_SINGLE;
        }
        else if(nMsgType == IM::BaseDefine::MSG_TYPE_GROUP_TEXT)
        {
            nSessionType = IM::BaseDefine::SESSION_TYPE_GROUP;
        }
        else if(nMsgType == IM::BaseDefine::MSG_TYPE_GROUP_AUDIO)
        {
            nSessionType = IM::BaseDefine::SESSION_TYPE_GROUP;
        }
        sendMsgAck(nFromId, nToId, nSessionType, nMsgId);
        m_pCallback->onRecvMsg(nSeqNo, nFromId, nToId, nMsgId, nCreateTime, nMsgType, strMsg);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb falied");
    }
}

void ClientConn::_HandleProxyUserListResponse(CImPdu* pPdu)
{
    IM::Proxy::IMProxyUserListRsp msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();

    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        // 类型 0:添加好友通知  1:代理    2:下线    3:好友

        if (msg.new_friends_size() > 0)
        {
            printf("---------------添加好友通知--------------\n");
            for (int i=0; i!=msg.new_friends_size(); ++i)
            {
                const IM::BaseDefine::UserInfo& user = msg.new_friends(i);
                printf("USER:%s ID:%d.\n", user.user_nick_name().c_str(), user.user_id());
            }
        }else{
            printf("---------------没有人加好友---------------\n");
        }

        if (msg.now_proxys_size() > 0)
        {
            printf("---------------我的代理--------------\n");
            for (int i=0; i!=msg.now_proxys_size(); ++i)
            {
                const IM::BaseDefine::UserInfo& user = msg.now_proxys(i);
                printf("USER:%s ID:%d.\n", user.user_nick_name().c_str(), user.user_id());
            }
        }else{
            printf("---------------未绑定代理---------------\n");
        }
        
        if (msg.now_subline_size() > 0)
        {
            printf("---------------我的下线--------------\n");
            for (int i=0; i!=msg.now_subline_size(); ++i)
            {
                const IM::BaseDefine::UserInfo& user = msg.now_subline(i);
                printf("USER:%s ID:%d.\n", user.user_nick_name().c_str(), user.user_id());
            }
        }else {
            printf("---------------没有下线---------------\n");
        }

        if (msg.now_friends_size() > 0)
        {
            printf("---------------我的好友--------------\n");
            for (int i=0; i!=msg.now_friends_size(); ++i)
            {
                const IM::BaseDefine::UserInfo& user = msg.now_friends(i);
                printf("USER:%s ID:%d.\n", user.user_nick_name().c_str(), user.user_id());
            }
        }else {
            printf("---------------没有好友---------------\n");
        }
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleSendRedpackResponse(CImPdu* pPdu)
{
    IM::Proxy::IMProxyRedpackAck msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();

    uint32_t nRet = msg.ret_code();

    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        m_pCallback->onSendRedpack(nSeqNo, nRet, msg.user_id(), msg.msg_id(), msg.session_id(), msg.session_type());
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}   

void ClientConn::_HandleSendRedpackNotify(CImPdu* pPdu)
{
    IM::Proxy::IMProxyRedpack msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        // m_pCallback->onSendRedpack(nSeqNo, 1, msg.user_id(), msg.msg_id(), msg.session_id(), msg.session_type());
        printf("====================REDPACK");
        printf("id:%u time:%u from:%u to:%u currency:%u number:%u rule:%s text:%s.", msg.msg_id(), msg.create_time(), msg.from_user_id(), 
                msg.to_session_id(), msg.currency(), msg.pack_num(), msg.pack_type()==0?"rand":"aventage", msg.content().c_str());
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleClaimRedpackResponse(CImPdu* pPdu)
{
    IM::Proxy::IMProxyRedpackClaimRsp msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();

    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        char szMessage[256] = {0};
        snprintf(szMessage, 256, "PACK:%d sender:%d number:%d type:%s last:%d.\n", msg.pack_id(), msg.sender_id(),
            msg.pack_num(), msg.pack_type()==0?"随机红包":"平均红包", msg.last_num());
        printf(szMessage);

        for (int i=0; i!=msg.user_claim_size(); ++i)
        {
            const IM::Proxy::ClaimInfo& info = msg.user_claim(i);

            printf("user:%s get currency:%d.\n", info.user_info().user_nick_name().c_str(), info.currency());
        }
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleClaimRedpackNotify(CImPdu* pPdu)
{
    IM::Proxy::IMProxyRedpackNotify msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();

    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        char szMessage[256] = {0};
        if (msg.pack_status() == 0)
        {
            snprintf(szMessage, 256, "user:%s 领取了您的红包%d, 红包已领完.", msg.user_name().c_str(), msg.pack_id());
        }else{
            snprintf(szMessage, 256, "user:%s 领取了您的红包%d.", msg.user_name().c_str(), msg.pack_id());
        }
        printf(szMessage);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}