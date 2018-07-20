/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：Client.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月20日
*   描    述：
*
================================================================*/
#include "Client.h"
#include "HttpClient.h"
#include "Common.h"
#include "json/json.h"
#include "ClientConn.h"


static ClientConn*  g_pConn = NULL;

static bool         g_bLogined = false;

CClient::CClient(const string& strName, const string& strPass, const string strDomain):
m_strName(strName),
m_strPass(strPass),
m_strLoginDomain(strDomain),
m_nLastGetUser(0)
{
}

CClient::~CClient()
{
}

void CClient::TimerCallback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    if (g_bLogined) {
        uint64_t cur_time = get_tick_count();
        g_pConn->OnTimer(cur_time);
    }
}



void CClient::onError(uint32_t nSeqNo, uint32_t nCmd,const string& strMsg)
{
    g_imlog.Error("get error:%d, msg:%s", nCmd, strMsg.c_str());
}

void CClient::connect()
{
    CHttpClient httpClient;
    string strUrl = m_strLoginDomain + "/msg_server";
    string strResp;
    CURLcode nRet = httpClient.Get(strUrl, strResp);
    if(nRet != CURLE_OK)
    {
        printf("login falied. access url:%s error\n", strUrl.c_str());
        PROMPTION;
        return;
    }
    Json::Reader reader;
    Json::Value value;
    if(!reader.parse(strResp, value))
    {
        printf("login falied. parse response error:%s\n", strResp.c_str());
        PROMPTION;
        return;
    }
    string strPriorIp, strBackupIp;
    uint16_t nPort;
    try {
        uint32_t nRet = value["code"].asUInt();
        if(nRet != 0)
        {
            string strMsg = value["msg"].asString();
            printf("login falied. errorMsg:%s\n", strMsg.c_str());
            PROMPTION;
            return;
        }
        strPriorIp = value["priorIP"].asString();
        strBackupIp = value["backupIP"].asString();
        nPort = atoi(value["port"].asString().c_str());
        
    } catch (std::runtime_error msg) {
        printf("login falied. get json error:%s\n", strResp.c_str());
        PROMPTION;
        return;
    }

    g_pConn = new ClientConn();
    m_nHandle = g_pConn->connect(strPriorIp.c_str(), nPort, m_strName, m_strPass, this);
    if(m_nHandle != INVALID_SOCKET)
    {
        netlib_register_timer(CClient::TimerCallback, NULL, 1000);
    }
    else
    {
        printf("invalid socket handle\n");
    }
}

void CClient::onConnect()
{
    printf("socket连接成功\n");
    // login(m_strName, m_strPass);
}

void CClient::close()
{
    printf("client closed activity.\n");
    g_pConn->Close();
}

void CClient::onClose()
{
    printf("client closed.\n");
}

uint32_t CClient::regist(const string strName, const string strPass, const string strNickname, const string strBind, const string strCode)
{
    return g_pConn->regist(strName, strPass, strNickname, strBind, strCode);
}

uint32_t CClient::login(const string strName, const string strPass)
{
    return g_pConn->login(strName, strPass);
}
    
uint32_t CClient::password(const string& strPass)
{
    return g_pConn->password(m_cSelfInfo.user_id(), strPass);
}

uint32_t CClient::bind(string szTarget, uint32_t nTag/*=0*/)
{
    return g_pConn->bind(m_cSelfInfo.user_id(), szTarget, nTag);
}

uint32_t CClient::friendReq(uint32_t nOperateId, uint32_t nOperateType)
{
    return g_pConn->friendReq(m_cSelfInfo.user_id(), nOperateId, nOperateType); 
}

uint32_t CClient::getUserList()
{
    return g_pConn->getUserList(m_cSelfInfo.user_id()); 
}

uint32_t CClient::sendRedpacket(uint32_t nToId, IM::BaseDefine::MsgType nType, uint32_t currency, uint32_t pack_num, uint32_t pack_type, string pack_content)
{
    return g_pConn->sendRedpacket(m_cSelfInfo.user_id(), nToId, nType, currency, pack_num, pack_type, pack_content);    
}

uint32_t CClient::claimPacket(uint32_t pack_id)
{
    return g_pConn->claimPacket(m_cSelfInfo.user_id(), pack_id);
}

void CClient::onBindProxy(uint32_t nSeqNo, uint32_t nUserId, uint32_t nProxyId, uint32_t ope, uint32_t rst)
{  
    printf("user:%d bind proxy:%d operate:%d result:%d.\n", nUserId, nProxyId, ope, rst);
}

void CClient::onFriend(uint32_t nSeqNo, uint32_t nUserId, uint32_t nOperateId, uint32_t ope, uint32_t rst)
{
    char szOperate[32] = {0};
    switch(ope)
    {
        case IM::Proxy::USER_OPERATE_FRIEND_ADD:
            snprintf(szOperate, 32, "add");
            break;
        case IM::Proxy::USER_OPERATE_FRIEND_DELETE:
            snprintf(szOperate, 32, "delete");
            break;
        case IM::Proxy::USER_OPERATE_FRIEND_AGGRE:
            snprintf(szOperate, 32, "agree");
            break;
        case IM::Proxy::USER_OPERATE_FRIEND_REFUSE:
            snprintf(szOperate, 32, "refuse");
            break;
        defautlt:
            break;
    }

    printf("user:%d operate:%s other:%d result:%d.\n", nUserId, szOperate, nOperateId, rst);
}

void CClient::onRegist(uint32_t nSeqNo, uint32_t nResultCode)
{
    printf("client onregist.....\n");
    printf("%d.",  nResultCode);
}

void CClient::onLogin(uint32_t nSeqNo, uint32_t nResultCode, string& strMsg, IM::BaseDefine::UserInfo* pUser)
{
    if(nResultCode != 0)
    {
        printf("login failed.errorCode=%u, msg=%s\n",nResultCode, strMsg.c_str());
        return;
    }
    if(pUser)
    {
        m_cSelfInfo = *pUser;
        g_bLogined = true;
        printf("user:%s login success.\n", pUser->user_nick_name().c_str());
    }
    else
    {
        printf("pUser is null\n");
    }
}

uint32_t CClient::getChangedUser()
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getUser(nUserId, m_nLastGetUser);
}

void CClient::onGetChangedUser(uint32_t nSeqNo, const list<IM::BaseDefine::UserInfo> &lsUser)
{
    for(auto it=lsUser.begin(); it!=lsUser.end(); ++it)
    {
        IM::BaseDefine::UserInfo* pUserInfo = new IM::BaseDefine::UserInfo();
        *pUserInfo = *it;
        uint32_t nUserId = pUserInfo->user_id();
        string strNick = pUserInfo->user_nick_name();
        if(it->status() != 3)
        {
            auto it1 = m_mapId2UserInfo.find(nUserId);
            if(it1 == m_mapId2UserInfo.end())
            {
                m_mapId2UserInfo.insert(pair<uint32_t, IM::BaseDefine::UserInfo*>(nUserId, pUserInfo));
            }
            else
            {
                delete it1->second;
                m_mapId2UserInfo[nUserId] = pUserInfo;
            }
            
            auto it2 = m_mapNick2UserInfo.find(strNick);
            if(it2 == m_mapNick2UserInfo.end())
            {
                m_mapNick2UserInfo.insert(pair<string, IM::BaseDefine::UserInfo*>(strNick, pUserInfo));
            }
            else
            {
                delete it1->second;
                m_mapNick2UserInfo[strNick] = pUserInfo;
            }
        }
    }
}

uint32_t CClient::getUserInfo(list<uint32_t>& lsUserId)
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getUserInfo(nUserId, lsUserId);
}

void CClient::onGetUserInfo(uint32_t nSeqNo, const list<IM::BaseDefine::UserInfo> &lsUser)
{
    
}

uint32_t CClient::sendMsg(uint32_t nToId, IM::BaseDefine::MsgType nType, const string &strMsg)
{
    uint32_t nFromId = m_cSelfInfo.user_id();
    return g_pConn->sendMessage(nFromId, nToId, nType, strMsg);
}

void CClient::onSendMsg(uint32_t nSeqNo, uint32_t nSendId, uint32_t nRecvId, IM::BaseDefine::SessionType nType, uint32_t nMsgId)
{
    printf("send msg succes. seqNo:%u, msgId:%u\n", nSeqNo, nMsgId);
}

void CClient::onPK10Place(uint32_t nSeqNo, uint32_t nResultCode, uint32_t nUserId)
{
    printf("user:%d pk10 place result:%d.", nUserId, nResultCode);
}

void CClient::onPassword(uint32_t nSeqNo, uint32_t nUserId)
{
    printf("user:%d change password success.", nUserId);
}

uint32_t CClient::place(uint32_t nToId, const string& strMsg)
{
    uint32_t nFromId = m_cSelfInfo.user_id();
    return g_pConn->place(nFromId, nToId, strMsg);
}


uint32_t CClient::getUnreadMsgCnt()
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getUnreadMsgCnt(nUserId);
}

void CClient::onGetUnreadMsgCnt(uint32_t nSeqNo, uint32_t nUserId, uint32_t nTotalCnt, const list<IM::BaseDefine::UnreadInfo>& lsUnreadCnt)
{
    
}

uint32_t CClient::getRecentSession()
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getRecentSession(nUserId, m_nLastGetSession);
}

void CClient::onGetRecentSession(uint32_t nSeqNo, uint32_t nUserId, const list<IM::BaseDefine::ContactSessionInfo> &lsSession)
{
    
}

uint32_t CClient::getMsgList(IM::BaseDefine::SessionType nType, uint32_t nPeerId, uint32_t nMsgId, uint32_t nMsgCnt)
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getMsgList(nUserId, nType, nPeerId, nMsgId, nMsgCnt);
}

void CClient::onGetMsgList(uint32_t nSeqNo, uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nType, uint32_t nMsgId, uint32_t nMsgCnt, const list<IM::BaseDefine::MsgInfo> &lsMsg)
{
    
}

void CClient::onRecvMsg(uint32_t nSeqNo, uint32_t nFromId, uint32_t nToId, uint32_t nMsgId, uint32_t nCreateTime, IM::BaseDefine::MsgType nMsgType, const string &strMsgData)
{
    
}

void CClient::printUserInfo()
{
    printf("+---------------------+\n");
    printf("|        用户名        |\n");
    printf("+---------------------+\n");
    printf("USER ID:%d.\n", m_cSelfInfo.user_id());
    printf("USER GENDER:%d.\n", m_cSelfInfo.user_gender());
    printf("USER Nickname:%s.\n", m_cSelfInfo.user_nick_name().c_str());
    printf("USER Realname:%s.\n", m_cSelfInfo.user_real_name().c_str());
    printf("MEMBER order:%d.\n", m_cSelfInfo.member_order());
    printf("BINDED :%d.\n", m_cSelfInfo.binded());
}

void CClient::onGetProxyUserList(uint32_t nSeqNo, uint32_t nType, const list<IM::BaseDefine::UserInfo>& lsUser)
{
    switch(nType)
    {
        case 0:
            printf("+----------请求添加您为好友------------+\n");
            break;
        case 1:
            printf("+----------我的代理------------+\n");
            break;
        case 2:
            printf("+----------我的下线------------+\n");
            break;
        case 3:
            printf("+----------我的好友------------+\n");
            break;
        defautlt:
            break;
    }

}

void CClient::onSendRedpack(uint32_t nSeqNo, uint32_t nResultCode, uint32_t nUserId, uint32_t nSessionId, uint32_t nPackId, IM::BaseDefine::SessionType nType)
{
    if (nResultCode == 0)
    {
        printf("USER:%u redpack:%u session:%u type:%u.", nUserId, nPackId, nSessionId, nType);
    }else{

        printf("send redpack failed.");
    }
}