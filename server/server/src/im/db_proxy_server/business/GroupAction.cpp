#include "../ProxyConn.h"
#include "GroupAction.h"
#include "GroupModel.h"
#include "UserModel.h"
#include "IM.Group.pb.h"
#include "IM.BaseDefine.pb.h"
#include "public_define.h"
#include "IM.Server.pb.h"
//#include "uchat/proto/code.pb.h"
//#include "code.pb.h"
#include "uchat/errno.hpp"
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
#include "json/json.h"
constexpr char const* kN = "db_proxy_server";
using GroupModel = CGroupModel;
using ProxyConn = CProxyConn;
using namespace IM;
using namespace IM::BaseDefine;
namespace uchat
{
namespace groupdb
{
void CreateGroup(Packet* pkt, uint32_t connId) noexcept try {
    Group::IMGroupCreateReq msg;
    Assert(msg.ParseFromArray(pkt->GetBodyData(), pkt->GetBodyLength()), "pb fail")
    GroupType const gtp = msg.group_type();
    Assert(GroupType_IsValid(gtp),
        "bad group type: " << uint32_t(gtp))
    std::set<uint32_t> members;
    // Add self
    uint32_t const userId = msg.user_id();
    members.insert(userId);
    Detail(__func__ << ": added self: " << userId)
    if (msg.member_id_list_size() > 0) {
        auto const& ii = msg.member_id_list();
        for (auto const& i : ii) {
            Detail(__func__ << ": add: " << i)
            members.insert(i);
        }
    }
    Info0(__func__ << ": members " << members.size() << " by " << userId)
    std::string const gnm = msg.group_name();
    std::string const gat = msg.group_avatar();
    uint32_t groupId = INVALID_VALUE;
    int32_t const code = GroupModel::getInstance()->newgrp(
        userId, gnm, gat, gtp, members, groupId);
    Group::IMGroupCreateRsp msgResp;
    msgResp.set_user_id(userId);
    msgResp.set_group_name(gnm);
    msgResp.set_result_code(code);
    if (proto::code::Generic::Ok == code) {
        for (auto const& i : members) {
            msgResp.add_user_id_list(i);
        }
        msgResp.set_group_id(groupId);
    }
    Info0(__func__ << "userId " << userId << "created group name "
        << gnm << " members " << members.size()
        << " result " << msgResp.result_code())
    msgResp.set_attach_data(msg.attach_data());
    Packet* const resp = new Packet();
    resp->SetPBMsg(&msgResp);
    resp->SetSeqNum(pkt->GetSeqNum());
    resp->SetServiceId(SID_GROUP);
    resp->SetCommandId(CID_GROUP_CREATE_RESPONSE);
    ProxyConn::AddResponsePdu(connId, resp);
} catch(std::exception const& e) {
    Error0(e.what())
}
void ModifyMember(Packet* pkt, uint32_t connId) noexcept try {
    Group::IMGroupChangeMemberReq msg;
    Assert(msg.ParseFromArray(pkt->GetBodyData(), pkt->GetBodyLength()), "pb fail")
    Assert(msg.has_attach_data(), "no attach")
    uint32_t const opUserId = msg.user_id();
    uint32_t const groupId = msg.group_id();
    GroupModifyType const type = msg.change_type();
    Assert(GroupModifyType_IsValid(type), "invalid change type " << type)
    Note(__func__ << ": op userId " << opUserId << " op group id "
        << groupId << " op type " << uint32_t(type)
        << " op users count " << msg.member_id_list_size())
    {
        auto const& ms = msg.member_id_list();
        for (auto const& m : ms) {
            Debug("op user id " << m)
        }
    }
#   if 0
    // Cache check
    Assert(GroupModel::getInstance()->isValidateGroupId(groupId),
        "invalid groupId " << groupId)
#   endif
    int32_t code = GroupModel::getInstance()->validGroup(groupId);
    Group::IMGroupChangeMemberRsp respMsg;
    if (proto::code::Generic::Valid == code) {
        auto const& mids = msg.member_id_list();
        std::set<uint32_t> upuids;
        for (auto const& mid : mids) {
            upuids.insert(mid);
        }
        std::set<uint32_t> curUserIds;
        code = GroupModel::getInstance()->modGrpms(
            groupId, opUserId, type, upuids, curUserIds);
        if (proto::code::Generic::Ok == code) {
            for (auto const& id : upuids) {
                respMsg.add_chg_user_id_list(id);
            }
            for (auto const& id : curUserIds) {
                respMsg.add_cur_user_id_list(id);
            }
        } else {
            Warning(__func__ << ": code " << code)
        }
    } else {
        Warning(__func__ << ": code " << code)
    }
    respMsg.set_user_id(opUserId);
    respMsg.set_group_id(groupId);
    respMsg.set_change_type(type);
    respMsg.set_result_code(code);
    respMsg.set_attach_data(msg.attach_data());
    Packet* resp = new Packet();
    resp->SetPBMsg(&respMsg);
    resp->SetSeqNum(pkt->GetSeqNum());
    resp->SetServiceId(ServiceID::SID_GROUP);
    resp->SetCommandId(GroupCmdID::CID_GROUP_CHANGE_MEMBER_RESPONSE);
    ProxyConn::AddResponsePdu(connId, resp);
} catch(std::exception const& e) {
    Error0(e.what())
}
void ModifyGroupInfo(Packet* pkt, uint32_t connId) noexcept try {
    Group::InfoModifyRequest msg;
    Assert(msg.ParseFromArray(pkt->GetBodyData(), pkt->GetBodyLength()), "pb fail")
    Assert(msg.has_attach(), "no attach")
    uint32_t const oid = msg.user_id();
    uint32_t const groupId = msg.group_id();
    int32_t code = GroupModel::getInstance()->validGrpo(
        groupId, oid);
    if (proto::code::Generic::Valid != code) {
        code = GroupModel::getInstance()->validGrpa(groupId, oid);
    }
    std::set<uint32_t> ids;
    if (proto::code::Generic::Valid == code) {
        switch (msg.op()) {
        case 1: {
            if (msg.has_op_data()) {
                code = GroupModel::getInstance()->modGrpnm(groupId, msg.op_data(), ids);
            } else {
                code = GroupModel::getInstance()->modGrpnm(groupId, "", ids);
            }
        } break;
        case 2: {
            if (msg.has_op_data()) {
                code = GroupModel::getInstance()->modGrpa(groupId, msg.op_data(), ids);
            } else {
                code = GroupModel::getInstance()->modGrpa(groupId, "", ids);
            }
        } break;
        default:
            Warning(__func__ << ": invalid msg op " << msg.op())
            code = proto::code::Database::InvalidGroupInfoOp;
        }
    }
    Group::InfoModifyResponse respmsg;
    respmsg.set_user_id(oid);
    respmsg.set_group_id(groupId);
    respmsg.set_op(msg.op());
    respmsg.set_code(code);
    if (code) {
        Warning(__func__ << ": code " << code)
    }
    if ((0 == code) && (!ids.empty()) && msg.has_op_data()) {
        respmsg.set_op_data(msg.op_data());
    }
    for (auto const id : ids) {
        respmsg.add_member(id);
    }
    respmsg.set_attach(msg.attach());
    Packet* const resp = new Packet();
    resp->SetPBMsg(&respmsg);
    resp->SetSeqNum(pkt->GetSeqNum());
    resp->SetServiceId(ServiceID::SID_GROUP);
    resp->SetCommandId(GroupCmdID::CID_GROUP_INFO_MODIFY_RESPONSE);
    ProxyConn::AddResponsePdu(connId, resp);
} catch(std::exception const& e) {
    Error0(e.what())
}
void ModifyGroupShield(Packet* pkt, uint32_t connId) noexcept try {
    Group::ShieldRequest msg;
    Assert(msg.ParseFromArray(pkt->GetBodyData(), pkt->GetBodyLength()), "pb fail")
    Assert(msg.has_attach(), "no attach")
    uint32_t const mid = msg.user_id();
    uint32_t const groupId = msg.group_id();
    int32_t code = GroupModel::getInstance()->validGrpm(
        groupId, mid);
    if (proto::code::Generic::Valid == code) {
        code = GroupModel::getInstance()->mods(
            groupId, mid, msg.shield());
    }
    Group::ShieldResponse respmsg;
    respmsg.set_user_id(mid);
    respmsg.set_group_id(groupId);
    respmsg.set_code(code);
    if (code) {
        Warning(__func__ << ": code " << code)
    }
    respmsg.set_attach(msg.attach());
    Packet* const resp = new Packet();
    resp->SetPBMsg(&respmsg);
    resp->SetSeqNum(pkt->GetSeqNum());
    resp->SetServiceId(ServiceID::SID_GROUP);
    resp->SetCommandId(GroupCmdID::CID_GROUP_SHIELD_RESPONSE);
    ProxyConn::AddResponsePdu(connId, resp);
} catch(std::exception const& e) {
    Error0(e.what())
}
void GetDetailGroupInfo(Packet* pkt, uint32_t connId) noexcept try {
    Group::IMGroupInfoListReq msg;
    Assert(msg.ParseFromArray(pkt->GetBodyData(), pkt->GetBodyLength()), "pb fail")
    Assert(msg.has_attach_data(), "no attach")
    GroupModel::GroupIds ginReq;
    if (msg.group_version_list_size() > 0) {
        auto const& gs = msg.group_version_list();
        for (auto const& g : gs) {
            if (proto::code::Generic::Valid
                == GroupModel::getInstance()->validGroup(g.group_id())) {
                ginReq[g.group_id()] = g;
                Debug(__func__ << ": add group id " << g.group_id())
            } else {
                Warning(__func__ << ": skip invalid group id "
                    << g.group_id())
            }
        }
    }
    uint32_t const userId = msg.user_id();
    std::vector<GroupInfo> ginResult;
    GroupModel::getInstance()->fetDGrpinfo(userId, ginReq, ginResult);
    Group::IMGroupInfoListRsp msgResp;
    msgResp.set_user_id(userId);
    for (auto&& i : ginResult) {
        GroupInfo* n = msgResp.add_group_info_list();
        *n = std::move(i);
    }
    Debug(__func__ << ": userId " << userId << " requestCount= "
        << msg.group_version_list_size()
        << " did resp " << msgResp.group_info_list_size())
    msgResp.set_attach_data(msg.attach_data());
    Packet* const resp = new Packet();
    resp->SetPBMsg(&msgResp);
    resp->SetSeqNum(pkt->GetSeqNum());
    resp->SetServiceId(SID_GROUP);
    resp->SetCommandId(CID_GROUP_INFO_RESPONSE);
    ProxyConn::AddResponsePdu(connId, resp);
} catch(std::exception const& e) {
    Error0(e.what())
}
void GetBasicGroupInfo(Packet* pkt, uint32_t connId) noexcept try {
    Group::IMNormalGroupListReq msg;
    Assert(msg.ParseFromArray(pkt->GetBodyData(), pkt->GetBodyLength()), "pb fail")
    uint32_t const userId = msg.user_id();
    std::vector<GroupVersionInfo> groupVersions;
    if (!GroupModel::getInstance()->fetUBGrpinfo(
        userId, groupVersions, GROUP_TYPE_NORMAL)) {
        Debug(__func__ << ": fetUBGrpinfo fail")
    }
    std::vector<GroupVersionInfo> groupVersions2;
    if (!GroupModel::getInstance()->fetUBGrpinfo(
        userId, groupVersions2, GROUP_TYPE_PK10)) {
        Debug(__func__ << ": fetUBGrpinfo fail")
    }
    Group::IMNormalGroupListRsp respMsg;
    respMsg.set_user_id(userId);
    for (auto&& g : groupVersions) {
        GroupVersionInfo* const p =
            respMsg.add_group_version_list();
        *p = std::move(g);
    }
    for (auto&& g : groupVersions2) {
        GroupVersionInfo* const p =
            respMsg.add_group_version_list();
        *p = std::move(g);
    }
    respMsg.set_attach_data(msg.attach_data());
    Packet* resp = new Packet();
    resp->SetPBMsg(&respMsg);
    resp->SetSeqNum(pkt->GetSeqNum());
    resp->SetServiceId(SID_GROUP);
    resp->SetCommandId(CID_GROUP_NORMAL_LIST_RESPONSE);
    ProxyConn::AddResponsePdu(connId, resp);
} catch(std::exception const& e) {
    Error0(e.what())
}
}
}
namespace DB_PROXY
{
#if 0
void getGroupInfo(CImPdu* pPdu, uint32_t conn_uuid)
{
    Group::IMGroupInfoListReq msg;
    Group::IMGroupInfoListRsp msgResp;
    if (!msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength())) {
        Warning("ParseFromArray fail")
        return;
    }
    CGroupModel::GroupIds ginReq;
    if (msg.group_version_list_size() > 0) {
        auto const& gs = msg.group_version_list();
        for (auto const& g : gs) {
            if (CGroupModel::getInstance()->isValidateGroupId(
                g.group_id())) {
                ginReq[g.group_id()] = g;
                Debug(__func__ << ": add group id " << g.group_id())
            } else {
                Warning("skip invalid group id " << g.group_id())
            }
        }
    }
    uint32_t const userId = msg.user_id();
    std::vector<GroupInfo> ginResult;
    GroupModel::getInstance()->fetDGrpinfo(
        userId, ginReq, ginResult);
    msgResp.set_user_id(userId);
    for (auto&& i : ginResult) {
        GroupInfo* n = msgResp.add_group_info_list();
        *n = std::move(i);
    }
    Info0(__func__ << ": userId " << userId << " requestCount= "
        << msg.group_version_list_size()
        << " did resp " << msgResp.group_info_list_size())
    msgResp.set_attach_data(msg.attach_data());
    CImPdu* const pkt = new CImPdu;
    pkt->SetPBMsg(&msgResp);
    pkt->SetSeqNum(pPdu->GetSeqNum());
    pkt->SetServiceId(SID_GROUP);
    pkt->SetCommandId(CID_GROUP_INFO_RESPONSE);
    CProxyConn::AddResponsePdu(conn_uuid, pkt);
}
#endif // if 0
    /**
     *  修改群成员，增加或删除
     *
     *  @param pPdu      收到的packet包指针
     *  @param conn_uuid 该包过来的socket 描述符
     */
    void modifyMember(CImPdu* pPdu, uint32_t conn_uuid)
    {
        Group::IMGroupChangeMemberReq msg;
        Group::IMGroupChangeMemberRsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.user_id();
            uint32_t nGroupId = msg.group_id();
            GroupModifyType nType = msg.change_type();
            if (GroupModifyType_IsValid(nType) &&
                CGroupModel::getInstance()->isValidateGroupId(nGroupId)) {
                
                CImPdu* pPduRes = new CImPdu;
                
                uint32_t nCnt = msg.member_id_list_size();
                set<uint32_t> setUserId;
                for(uint32_t i=0; i<nCnt;++i)
                {
                    setUserId.insert(msg.member_id_list(i));
                }
                list<uint32_t> lsCurUserId;
                bool bRet = CGroupModel::getInstance()->modifyGroupMember(nUserId, nGroupId, nType, setUserId, lsCurUserId);
                msgResp.set_user_id(nUserId);
                msgResp.set_group_id(nGroupId);
                msgResp.set_change_type(nType);
                msgResp.set_result_code(bRet?0:1);
                if(bRet)
                {
                    for(auto it=setUserId.begin(); it!=setUserId.end(); ++it)
                    {
                        msgResp.add_chg_user_id_list(*it);
                    }
                    
                    for(auto it=lsCurUserId.begin(); it!=lsCurUserId.end(); ++it)
                    {
                        msgResp.add_cur_user_id_list(*it);
                    }
                }
                //log("userId=%u, groupId=%u, result=%u, changeCount:%u, currentCount=%u",nUserId, nGroupId,  bRet?0:1, msgResp.chg_user_id_list_size(), msgResp.cur_user_id_list_size());
                msgResp.set_attach_data(msg.attach_data());
                pPduRes->SetPBMsg(&msgResp);
                pPduRes->SetSeqNum(pPdu->GetSeqNum());
                pPduRes->SetServiceId(SID_GROUP);
                pPduRes->SetCommandId(CID_GROUP_CHANGE_MEMBER_RESPONSE);
                CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
            }
            else
            {
                //log("invalid groupModifyType or groupId. userId=%u, groupId=%u, groupModifyType=%u", nUserId, nGroupId, nType);
            }
            
        }
        else
        {
            //log("parse pb failed");
        }
    }
    
    /**
     *  设置群组信息推送，屏蔽或者取消屏蔽
     *
     *  @param pPdu      收到的packet包指针
     *  @param conn_uuid 该包过来的socket 描述符
     */
    void setGroupPush(CImPdu* pPdu, uint32_t conn_uuid)
    {
        Group::IMGroupShieldReq msg;
        Group::IMGroupShieldRsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.user_id();
            uint32_t nGroupId = msg.group_id();
            uint32_t nStatus = msg.shield_status();
            if(CGroupModel::getInstance()->isValidateGroupId(nGroupId))
            {
                
                CImPdu* pPduRes = new CImPdu;
                bool bRet = CGroupModel::getInstance()->setPush(nUserId, nGroupId, IM_GROUP_SETTING_PUSH, nStatus);
                
                msgResp.set_user_id(nUserId);
                msgResp.set_group_id(nGroupId);
                msgResp.set_result_code(bRet?0:1);
            
                //log("userId=%u, groupId=%u, result=%u", nUserId, nGroupId, msgResp.result_code());
                
                msgResp.set_attach_data(msg.attach_data());
                pPduRes->SetPBMsg(&msgResp);
                pPduRes->SetSeqNum(pPdu->GetSeqNum());
                pPduRes->SetServiceId(SID_GROUP);
                pPduRes->SetCommandId(CID_GROUP_SHIELD_GROUP_RESPONSE);
                CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
            }
            else
            {
                //log("Invalid group.userId=%u, groupId=%u", nUserId, nGroupId);
            }
        }
        else
        {
            //log("parse pb failed");
        }
    }
    
    /**
     *  获取一个群的推送设置
     *
     *  @param pPdu      收到的packet包指针
     *  @param conn_uuid 该包过来的socket 描述符
     */
    void getGroupPush(CImPdu* pPdu, uint32_t conn_uuid)
    {
        Server::IMGroupGetShieldReq msg;
        Server::IMGroupGetShieldRsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nGroupId = msg.group_id();
            uint32_t nUserCnt = msg.user_id_size();
            if(CGroupModel::getInstance()->isValidateGroupId(nGroupId))
            {
                CImPdu* pPduRes = new CImPdu;
                list<uint32_t> lsUser;
                for(uint32_t i=0; i<nUserCnt; ++i)
                {
                    lsUser.push_back(msg.user_id(i));
                }
                list<ShieldStatus> lsPush;
                CGroupModel::getInstance()->getPush(nGroupId, lsUser, lsPush);
                
                msgResp.set_group_id(nGroupId);
                for (auto it=lsPush.begin(); it!=lsPush.end(); ++it) {
                    ShieldStatus* pStatus = msgResp.add_shield_status_list();
        //            *pStatus = *it;
                    pStatus->set_user_id(it->user_id());
                    pStatus->set_group_id(it->group_id());
                    pStatus->set_shield_status(it->shield_status());
                }
                
                //log("groupId=%u, count=%u", nGroupId, nUserCnt);
                
                msgResp.set_attach_data(msg.attach_data());
                pPduRes->SetPBMsg(&msgResp);
                pPduRes->SetSeqNum(pPdu->GetSeqNum());
                pPduRes->SetServiceId(SID_OTHER);
                pPduRes->SetCommandId(CID_OTHER_GET_SHIELD_RSP);
                CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
            }
            else
            {
                //log("Invalid groupId. nGroupId=%u", nGroupId);
            }
        }
        else
        {
            //log("parse pb failed");
        }
    }

    // 管理员/代理 设置PK10群规则
    void setGroupPK10(CImPdu* pPdu, uint32_t conn_uuid)
    {
        Group::IMGroupChangePK10Req msg;
        Group::IMGroupChangePK10Rsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.user_id();
            GroupModifyType modify = msg.change_type();
            uint32_t nGroupId = msg.group_id();
            string szRules = msg.pk10_rules();
            uint32_t nOption = 0;

            CGroupModel::getInstance()->setGroupPk10Rules(nUserId, nGroupId, szRules, nOption);

            msgResp.set_result_code(1);
            msgResp.set_user_id(nUserId);
            msgResp.set_is_admin(nOption==MEMBER_ADMIN?1:0);
            msgResp.set_group_id(nGroupId);
            msgResp.set_attach_data(msg.attach_data());
            msgResp.set_pk10_rules(szRules);

            // 获得所有群成员
            list<uint32_t> lsMember;
            CGroupModel::getInstance()->getGroupUser(nGroupId, lsMember);

            for (auto it=lsMember.begin(); it!=lsMember.end(); ++it)
            {
                msgResp.add_group_member_list(*it);
            }

            // 如果是管理员设置PK10规则，额外通知所有代理
            if (nOption==MEMBER_ADMIN)
            {
                uint32_t nAdminId;
                list<uint32_t> lsIds;
                CUserModel::getInstance()->getProxys(lsIds, nAdminId);

                for (auto it=lsIds.begin(); it!=lsIds.end(); ++it)
                {
                    msgResp.add_proxy_list(*it);
                }
            }

            CImPdu* pPduRes = new CImPdu;

            pPduRes->SetPBMsg(&msgResp);
            pPduRes->SetSeqNum(pPdu->GetSeqNum());
            pPduRes->SetServiceId(SID_GROUP);
            pPduRes->SetCommandId(CID_GROUP_CHANGE_PK10_RULES_RESPONSE);
            CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
        }
    }

    void getGroupPk10Rule(CImPdu* pPdu, uint32_t conn_uuid)
    {
        // 获取管理 和  所有代理 的PK群
        list<GroupPKInfo> lsPkInfo;
        CGroupModel::getInstance()->getGroupsPKRules(lsPkInfo);

        Group::IMGroupPK10RuleRsp msg;

        list<GroupPKInfo>::iterator it = lsPkInfo.begin();
        for(; it!=lsPkInfo.end(); ++it)
        {
            GroupPKInfo& info = *it;
            GroupPKInfo* pPKInfo = msg.add_pk_group();

            pPKInfo->set_user_id(info.user_id());
            pPKInfo->set_is_admin(info.is_admin());
            pPKInfo->set_group_id(info.group_id());
            pPKInfo->set_rules(info.rules());
        }

        CImPdu* pPduRes = new CImPdu;

        pPduRes->SetPBMsg(&msg);
        pPduRes->SetSeqNum(pPdu->GetSeqNum());
        pPduRes->SetServiceId(SID_GROUP);
        pPduRes->SetCommandId(CID_GROUP_PK10_RULES_RESPONSE);
        CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
    }

    void placeGroupPK10(CImPdu* pPdu, uint32_t conn_uuid)
    {
        Group::IMGroupPK10PlaceReq msg;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            // 获取总的下注金额
            uint32_t nTotalPlace = 0;

            list<IMCashFlow> lsFlow;

            for (int i=0; i!=msg.items_size(); ++i)
            {
                uint32_t nCurr = msg.items(i).gold();

                IMCashFlow flow;

                flow.set_cash(nCurr);
                flow.set_consume(CASH_CONSUME_PK10_PLACE);

                Json::Value v;
                v["user"] = msg.user_id();
                v["group"] = msg.group_id();
                v["rule"] = msg.rule();
                v["time"] = msg.time();

                flow.set_detail(v.toStyledString());
                lsFlow.push_back(flow);

                nTotalPlace += nCurr;
            }

            DBUserInfo_t cUser;
            CUserModel::getInstance()->getUser(msg.user_id(), cUser);

            CImPdu* pPduRes = new CImPdu;

            if (cUser.nCurrency < nTotalPlace)
            {
                Group::IMGroupPK10PlaceRsp resp;

                resp.set_user_id(msg.user_id());
                resp.set_result_code(4);
                resp.set_attach_data(msg.attach_data());

                pPduRes->SetPBMsg(&resp);
                pPduRes->SetSeqNum(pPdu->GetSeqNum());
                pPduRes->SetServiceId(SID_GROUP);
                pPduRes->SetCommandId(CID_GROUP_PK10_PLACE_HD_RESPONSE);
                CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
                
            }else{

                // 扣除下注金额
                CUserModel::getInstance()->updateUserCurrency(cUser.nId, cUser.nCurrency - nTotalPlace, lsFlow);

                pPduRes->SetPBMsg(&msg);
                pPduRes->SetSeqNum(pPdu->GetSeqNum());
                pPduRes->SetServiceId(SID_GROUP);
                pPduRes->SetCommandId(CID_GROUP_PK10_PLACE_HD_REQUEST);
                CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
            }
        }
    }
}

