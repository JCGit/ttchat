#include "util.h"
#include "Proxy.h"
#include "UserModel.h"
#include "MessageModel.h"
#include "GroupMessageModel.h"
#include "SessionModel.h"
#include "RelationModel.h"
#include "ProxyModel.h"
#include "GroupAction.h"
#include "IM.Proxy.pb.h"
#include "IM.Server.pb.h"
#include "IM.BaseDefine.pb.h"
//#include "code.pb.h"
#include "../DBPool.h"
#include "../ProxyConn.h"
#include "../base/public_define.h"
#include <algorithm>
#include "uchat/errno.hpp"
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
constexpr char const* kN = "db_proxy_server";
using GroupModel = CGroupModel;
using namespace uchat;
namespace DB_PROXY {


void doSetProxy(CImPdu* pPdu, uint32_t conn_uuid) try {
    IM::Proxy::IMProxyEnableReq msg;
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nUserId = msg.user_id();
        uint32_t nSetId = msg.set_id();
        uint32_t const enable = msg.enable();
        log("[proxy] user:%d enable target:%d,enable:%d.", nUserId,
            nSetId, enable);
        //uint32_t nDefGroup = 0;
        //uint32_t nRes = CProxyModel::getInstance()->setProxy(nUserId, nEnable==1?true:false, int2string(nSetId), 2, nDefGroup);
        uint32_t groupId = 0;
        uint32_t const resultCode = CProxyModel::getInstance()->setproxy(
            nUserId,
            enable != 0 ? true : false,
            nSetId,
            groupId);
        IM::Proxy::IMProxyEnableRsp response;
        response.set_res(resultCode);
        response.set_user_id(msg.user_id());
        response.set_enable_id(msg.set_id());
        response.set_enable(msg.enable());
        if (0 == resultCode && enable){
            response.set_group_id(groupId);
        }
        Assert(msg.has_attach_data(), "msg no attach data")
        response.set_attach_data(msg.attach_data());
        CImPdu* pPduResp = new CImPdu;
        pPduResp->SetPBMsg(&response);
        pPduResp->SetSeqNum(pPdu->GetSeqNum());
        pPduResp->SetServiceId(IM::BaseDefine::SID_PROXY);
        pPduResp->SetCommandId(IM::BaseDefine::CID_PROXY_ENABLE_RSP);
        CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
        log("[proxy] db return");
    }
} catch(std::exception const& e) {
    Error0(e.what())
}

void doBindProxy(CImPdu* pPdu, uint32_t conn_uuid)
{
    IM::Proxy::IMProxyBindReq msg;
    if (!msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength())) {
        return;
    }
    log("[bind] user:%d proxy:%s tag:%d.", msg.user_id(), msg.user_bind().c_str(), msg.bind_tag());

    uint32_t nProxyId ;
    std::set<uint32_t> realGids;
    uint32_t ret = CProxyModel::getInstance()->bind2Group(msg.user_id(), msg.user_bind(), IM::Proxy::UserOperate(msg.bind_tag()), nProxyId, realGids);

    // 绑定结果通知
    IM::Proxy::IMProxyBindRsp msgrsp;

    for(auto gid : realGids)
    {
        std::set<uint32_t> memids;
        CGroupModel::getInstance()->fetMemids(gid, memids);

        auto pTarget = msgrsp.add_targets();

        pTarget->set_group_id(gid);
        for (auto mem : memids)
        {
            pTarget->add_cur_user_id(mem);
        }
    }

    msgrsp.set_user_id(msg.user_id());
    msgrsp.set_user_bind(nProxyId);
    msgrsp.set_bind_tag(msg.bind_tag());
    msgrsp.set_attach_data(msg.attach_data());
    msgrsp.set_result_code(IM::BaseDefine::ProxyUpgradeErrType(ret));
    
    CImPdu* pPduResp = new CImPdu;
    pPduResp->SetPBMsg(&msgrsp);
    pPduResp->SetSeqNum(pPdu->GetSeqNum());
    pPduResp->SetServiceId(IM::BaseDefine::SID_PROXY);
    pPduResp->SetCommandId(IM::BaseDefine::CID_PROXY_BIND_RSP);
    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);

    log("[bind] db return");
}

void doGetUserList(CImPdu* pPdu, uint32_t conn_uuid)
{
    IM::Proxy::IMProxyUserListReq msg;
    if (!msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength())) {
        return;
    }
    log("[users] user:%d request userlist.", msg.user_id());
    std::vector<IM::BaseDefine::UserInfo> users;
    IM::Proxy::IMProxyUserListRsp resp;
    resp.set_user_id(msg.user_id());
    resp.set_attach_data(msg.attach_data());
    resp.set_latest_update_time(::time(nullptr));
    // Proxy
    users.clear();
    CProxyModel::getInstance()->lsfrnds(
        msg.user_id(),
        msg.latest_update_time(),
        uchat::ListFriendType::Proxy,
        users);
    for (auto&& user : users) {
        IM::BaseDefine::UserInfo* const userInfo = resp.add_now_proxys();
        *userInfo = std::move(user);
    }
    // Subline
    users.clear();
    CProxyModel::getInstance()->lsfrnds(
        msg.user_id(),
        msg.latest_update_time(),
        uchat::ListFriendType::Subline,
        users);
    for (auto&& user : users) {
        IM::BaseDefine::UserInfo* const userInfo = resp.add_now_subline();
        *userInfo = std::move(user);
    }
    users.clear();
    // New add friends request
    users.clear();
    CProxyModel::getInstance()->lsfrnds(
        msg.user_id(),
        msg.latest_update_time(),
        uchat::ListFriendType::NewAddRequest,
        users);
    for (auto&& user : users) {
        IM::BaseDefine::UserInfo* const userInfo = resp.add_new_friends();
        *userInfo = std::move(user);
    }
    // Online/offline/... friends
    users.clear();
    CProxyModel::getInstance()->lsfrnds(
        msg.user_id(),
        msg.latest_update_time(),
        uchat::ListFriendType::Friend,
        users);
    for (auto&& user : users) {
        IM::BaseDefine::UserInfo* const userInfo = resp.add_now_friends();
        *userInfo = std::move(user);
    }
    /// Del after send
    CImPdu* respPkt = new CImPdu;
    respPkt->SetPBMsg(&resp);
    respPkt->SetSeqNum(pPdu->GetSeqNum());
    respPkt->SetServiceId(IM::BaseDefine::SID_PROXY);
    respPkt->SetCommandId(IM::BaseDefine::CID_PROXY_USER_LIST_RSP);
    CProxyConn::AddResponsePdu(conn_uuid, respPkt);
    log("[users] db return.");
}

void doProcessFriend(CImPdu* pPdu, uint32_t conn_uuid)
{
    IM::Proxy::IMProxyFriendReq msg;
    IM::Proxy::IMProxyFriendRsp msgrsp;

    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        CImPdu* pPduResp = new CImPdu;

        log("[friend] user:%d other:%d tag:%d.", msg.user_id(), msg.user_ope(), msg.ope_type());

        msgrsp.set_user_id(msg.user_id());
        msgrsp.set_user_ope(msg.user_ope());
        msgrsp.set_ope_type(msg.ope_type());
        msgrsp.set_attach_data(msg.attach_data());
        uint32_t const ret = CProxyModel::getInstance()->friendop(
            msg.user_id(),
            msg.user_ope(),
            IM::Proxy::UserOperate(msg.ope_type()));
        Debug(__func__ << ": friendop ret " << ret);
        msgrsp.set_result_code(ret);

        pPduResp->SetPBMsg(&msgrsp);
        pPduResp->SetSeqNum(pPdu->GetSeqNum());
        pPduResp->SetServiceId(IM::BaseDefine::SID_FRIEND);
        pPduResp->SetCommandId(IM::BaseDefine::CID_FRIEND_OPERATE_RSP);
        CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
        log("[friend] db return.");
    }
}

void doSendRedpack(CImPdu* pPdu, uint32_t conn_uuid)
{
    IM::Proxy::IMProxyRedpack msg;
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nFromId = msg.from_user_id();
        uint32_t nToId = msg.to_session_id();

        // 红包信息
        uint32_t nRedCurr = msg.currency();
        uint32_t nRedNum  = msg.pack_num();
        uint32_t nRedRule = msg.pack_type();
        string szRedText = msg.content();
        uint32_t nCreateTime = msg.create_time();
 
        IM::BaseDefine::MsgType nMsgType = msg.msg_type();
        log("[redpack] user:%u send redpack.", nFromId);
        
        uint32_t nMsgId = 0;
        uint32_t nRet = CProxyModel::getInstance()->redpack(nFromId, nToId, nMsgType, nCreateTime, nRedCurr, nRedNum, nRedRule, szRedText, nMsgId);

        CImPdu* pPduResp = new CImPdu;

        if (nRet == 0)
        {
            msg.set_msg_id(nMsgId);
            pPduResp->SetPBMsg(&msg);
        }else{
            IM::Proxy::IMProxyRedpackAck ack;
            ack.set_ret_code(1);
            ack.set_user_id(nFromId);
            ack.set_attach_data(msg.attach_data());

            pPduResp->SetPBMsg(&ack);
        }
        
        pPduResp->SetSeqNum(pPdu->GetSeqNum());
        pPduResp->SetServiceId(IM::BaseDefine::SID_REDPACK);
        pPduResp->SetCommandId(IM::BaseDefine::CID_REDPACK_NOTIFY);
        CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
        log("[redpack] db return");
    }
}


void doClaimRedpack(CImPdu* pPdu, uint32_t conn_uuid)
{
    IM::Proxy::IMProxyRedpackClaimReq msg;
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        log("[redpack] user:%d claim request.", msg.user_id());

        uint32_t nUserId = msg.user_id();
        uint32_t nSessionId = msg.session_id();
        uint32_t nPackId = msg.pack_id();

        uint32_t nSender, nOpenRule, nTotalNum;
        nSender = nOpenRule = nTotalNum = 0;

        list<UserClaimed> lsUserClaim;
        uint32_t nRet = CProxyModel::getInstance()->claim_rp(nUserId, nSessionId, nPackId, lsUserClaim, nSender, nOpenRule, nTotalNum);
        if (nRet != 0)
        {
            CImPdu* pPduResp = new CImPdu;

            IM::Proxy::IMProxyRedpackClaimRsp msgrsp;

            msgrsp.set_pack_id(nPackId);
            msgrsp.set_user_id(nUserId);
            msgrsp.set_sender_id(nSender);
            msgrsp.set_pack_num(nTotalNum);
            msgrsp.set_pack_type(nOpenRule);
            msgrsp.set_last_num(nTotalNum - lsUserClaim.size());

            list<UserClaimed>::iterator it = lsUserClaim.begin();
            for (; it!=lsUserClaim.end(); ++it)
            {
                IM::Proxy::ClaimInfo* pClaim = msgrsp.add_user_claim();
                pClaim->set_currency((*it).nClaimed);

                DBUserInfo_t info;
                CUserModel::getInstance()->getUser((*it).nUserId, info);

                IM::BaseDefine::UserInfo* pUserInfo = pClaim->mutable_user_info();
                pUserInfo->set_user_id((*it).nUserId);
                pUserInfo->set_user_nick_name(info.strNick);
            }

            pPduResp->SetPBMsg(&msgrsp);
            pPduResp->SetSeqNum(pPdu->GetSeqNum());
            pPduResp->SetServiceId(IM::BaseDefine::SID_REDPACK);
            pPduResp->SetCommandId(IM::BaseDefine::CID_REDPACK_CLAIM_RSP);
            CProxyConn::AddResponsePdu(conn_uuid, pPduResp);

            log("[redpack] user:%d claim db return.", msg.user_id());
        }
    }
}

void doPK10Consume(CImPdu* pPdu, uint32_t conn_uuid)
{
    IM::Server::IMCashConsumeInsertReq msg;
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        


    }
}

};

