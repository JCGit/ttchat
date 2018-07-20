/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：Login.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
 *
 ================================================================*/

#include <list>
#include <string>
#include "util.h"
#include "../ProxyConn.h"
#include "../HttpClient.h"
#include "../SyncCenter.h"
#include "Login.h"
#include "UserModel.h"
#include "TokenValidator.h"
#include "json/json.h"
#include "Common.h"
#include "IM.Server.pb.h"
#include "IM.Login.pb.h"
#include "Base64.h"
#include "InterLogin.h"
#include "ExterLogin.h"
#include "CachePool.h"

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
#include<time.h>
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
constexpr char const* kN = "db_proxy_server";
CInterLoginStrategy g_loginStrategy;

hash_map<string, list<uint32_t> > g_hmLimits;
CLock g_cLimitLock;
namespace DB_PROXY {

void doRegister(CImPdu* pPdu, uint32_t conn_uuid)
{
    IM::Login::IMRegisterReq msg;
    if (!msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength())) {
        Error0(__func__ << ": pb fail")
        return;
    }

    if (msg.user_name().empty()){
        Warning(__func__ << ": no user name.")
        return;
    }

    CacheManager* const cacheManager = CacheManager::getInstance();
    CacheConn* const cacheConn = cacheManager->GetCacheConn("checkcode");
    if (!cacheConn) {
        Warning(__func__ << ": no cache connection for register")
        return;
    }

    uint32_t nRet = 0;
    string  szCheckcode;

    //注册流程重设密码
    string t_checkcode = "";
    string t_register = "";

    string checkcode_s = cacheConn->get(msg.user_name());

    uint32_t nPos = checkcode_s.find(":");
    if (nPos != string::npos)
    {
        t_checkcode = checkcode_s.substr(0, nPos);
        t_register = checkcode_s.substr(nPos + 1);
    }

    if (t_checkcode.empty())
    {
        nRet = 1;       //未获取校验码

        szCheckcode = int2string(rand() % (999999 - 100000) + 100000);

        uint32_t t_now = time(NULL);
        string val = szCheckcode + ":" + int2string(t_now);

        cacheConn->set(msg.user_name(), val);
        log("[doRegister]: user: %s checkcode value:%s.", msg.user_name().c_str(), val.c_str());
        
    }else if (msg.checkcode() != t_checkcode){
        nRet = 2;       //校验码错误

        // 服务器有缓存，但客户端请求code为nil，则默认客户端是要请求code
        if (!msg.has_checkcode()){
            cacheConn->del(msg.user_name());

            nRet = 1;   //重新获取校验码
            szCheckcode = int2string(rand() % (999999 - 100000) + 100000);

            uint32_t t_now = time(NULL);
            string val = szCheckcode + ":" + int2string(t_now);

            cacheConn->set(msg.user_name(), val);
            log("[doRegister]: user: %s reget checkcode value:%s.", msg.user_name().c_str(), val.c_str());
        }
    }else{
        uint32_t t_now = time(NULL);
        if (t_now - string2int(t_register) > 1 * 60 * 1000){
            nRet = 3;       //校验码已过期
        }else{
            // 注册成功
            nRet = 0;

            bool bRet = g_loginStrategy.doRegister(msg.user_name(), msg.password(), msg.nick_name());
            log("[doRegister]: user:%s register success.", msg.user_name().c_str());

            // 是否绑定代理
            if (msg.has_to_bind()){
                
                uint32_t nUserId = CUserModel::getInstance()->getUserID(msg.user_name());
                uint32_t nTarID = CUserModel::getInstance()->getProxyUserID(msg.to_bind());

                uint32_t nProxyId ;
                std::set<uint32_t> realGids;
                uint32_t ret = CProxyModel::getInstance()->bind2Group(nUserId, int2string(nTarID), IM::Proxy::USER_OPERATE_BIND, nProxyId, realGids);
                if (ret == 0)
                {
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

                    msgrsp.set_user_id(nUserId);
                    msgrsp.set_user_bind(nProxyId);
                    msgrsp.set_bind_tag(nTarID);
                    msgrsp.set_attach_data(msg.attach_data());
                    msgrsp.set_result_code(IM::BaseDefine::PROXY_UPGRADE_SUCCESSED);
                    
                    CImPdu* pPduResp = new CImPdu;
                    pPduResp->SetPBMsg(&msgrsp);
                    pPduResp->SetSeqNum(pPdu->GetSeqNum());
                    pPduResp->SetServiceId(IM::BaseDefine::SID_PROXY);
                    pPduResp->SetCommandId(IM::BaseDefine::CID_PROXY_BIND_RSP);
                    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
                }
                
            }
        }
        
        cacheConn->del(msg.user_name());
    }

    cacheManager->RelCacheConn(cacheConn);

    IM::Login::IMRegisterRsp msgResp;
    msgResp.set_user_name(msg.user_name());
    msgResp.set_retcode(nRet);
    msgResp.set_errstr("");
    msgResp.set_checkcode(szCheckcode);
    msgResp.set_attach_data(msg.attach_data());

    CImPdu* pPduResp = new CImPdu;
    pPduResp->SetPBMsg(&msgResp);
    pPduResp->SetSeqNum(pPdu->GetSeqNum());
    pPduResp->SetServiceId(IM::BaseDefine::SID_LOGIN);
    pPduResp->SetCommandId(IM::BaseDefine::CID_LOGIN_RES_REGISTER);
    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
}

void doLogin(CImPdu* pPdu, uint32_t conn_uuid)
{
    CImPdu* pPduResp = new CImPdu;
    
    IM::Server::IMValidateReq msg;
    IM::Server::IMValidateRsp msgResp;
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        string strDomain = msg.user_name();
        string strPass = msg.password();
        
        msgResp.set_user_name(strDomain);
        msgResp.set_attach_data(msg.attach_data());
     
        do
        {
            CAutoLock cAutoLock(&g_cLimitLock);
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            uint32_t tmNow = time(NULL);
            
            //清理超过30分钟的错误时间点记录
            /*
             清理放在这里还是放在密码错误后添加的时候呢？
             放在这里，每次都要遍历，会有一点点性能的损失。
             放在后面，可能会造成30分钟之前有10次错的，但是本次是对的就没办法再访问了。
             */
            auto itTime=lsErrorTime.begin();
            for(; itTime!=lsErrorTime.end();++itTime)
            {
                if(tmNow - *itTime > 30*60)
                {
                    break;
                }
            }
            if(itTime != lsErrorTime.end())
            {
                lsErrorTime.erase(itTime, lsErrorTime.end());
            }
            
            // 判断30分钟内密码错误次数是否大于10
            if(lsErrorTime.size() > 10)
            {
                itTime = lsErrorTime.begin();
                if(tmNow - *itTime <= 30*60)
                {
                    msgResp.set_result_code(6);
                    msgResp.set_result_string("用户名/密码错误次数太多");
                    pPduResp->SetPBMsg(&msgResp);
                    pPduResp->SetSeqNum(pPdu->GetSeqNum());
                    pPduResp->SetServiceId(IM::BaseDefine::SID_OTHER);
                    pPduResp->SetCommandId(IM::BaseDefine::CID_OTHER_VALIDATE_RSP);
                    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
                    return ;
                }
            }
        } while(false);
        
        log("%s request login.", strDomain.c_str());
        IM::BaseDefine::UserInfo cUser;
        
        log("user:%s pass:%s request login.", strDomain.c_str(), strPass.c_str());
        if(g_loginStrategy.doLogin(strDomain, strPass, cUser))
        {
            IM::BaseDefine::UserInfo* pUser = msgResp.mutable_user_info();
            pUser->set_user_id(cUser.user_id());
            pUser->set_user_gender(cUser.user_gender());
            pUser->set_department_id(cUser.department_id());
            pUser->set_user_nick_name(cUser.user_nick_name());
            pUser->set_user_domain(cUser.user_domain());
            pUser->set_avatar_url(cUser.avatar_url());
            
            pUser->set_email(cUser.email());
            pUser->set_user_tel(cUser.user_tel());
            pUser->set_user_real_name(cUser.user_real_name());
            pUser->set_status(0);

            pUser->set_member_order(cUser.member_order());
            pUser->set_binded(cUser.binded());

            pUser->set_sign_info(cUser.sign_info());
            pUser->set_birthday(cUser.birthday());
           
            msgResp.set_result_code(0);
            msgResp.set_result_string("成功");
            
            //如果登陆成功，则清除错误尝试限制
            CAutoLock cAutoLock(&g_cLimitLock);
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            lsErrorTime.clear();

            log("%s login success.", strDomain.c_str());
        }
        else
        {
            //密码错误，记录一次登陆失败
            uint32_t tmCurrent = time(NULL);
            CAutoLock cAutoLock(&g_cLimitLock);
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            lsErrorTime.push_front(tmCurrent);
            
            log("get result false");
            msgResp.set_result_code(1);
            msgResp.set_result_string("用户名/密码错误");
        }
    }
    else
    {
        msgResp.set_result_code(2);
        msgResp.set_result_string("服务端内部错误");
    }
    
    
    pPduResp->SetPBMsg(&msgResp);
    pPduResp->SetSeqNum(pPdu->GetSeqNum());
    pPduResp->SetServiceId(IM::BaseDefine::SID_OTHER);
    pPduResp->SetCommandId(IM::BaseDefine::CID_OTHER_VALIDATE_RSP);
    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
}

};

