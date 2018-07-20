#include "InterLogin.h"
#include <random>
#include "../DBPool.h"
#include "EncDec.h"
#include "UserModel.h"
#include "uchat/stringview.hpp"
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
constexpr char const* kN = "db_proxy_server";
using InterLoginStrategy = CInterLoginStrategy;
using DBManager = CDBManager;

bool CInterLoginStrategy::doRegister(const std::string& strName, const std::string& strPass, const std::string& strNickname)
{
    bool bRet = false;
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (pDBConn) {
        string salt;
        string strSql = "select * from IMUser where name='" + strName + "'";
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next()){
                salt = pResultSet->GetString("salt");
            }

            delete  pResultSet;
            pResultSet = NULL;
        }

        bool bExist = true;
        if (salt.empty())
        {
            int nSalt = rand()%10000;
            salt = int2string(nSalt);

            bExist = false;
        }

        string strInPass = strPass + salt;
        char szMd5[33];
        CMd5::MD5_Calculate(strInPass.c_str(), strInPass.length(), szMd5);

        uint32_t t_now = time(NULL);

        if (bExist){
            string clause;

            if (!strPass.empty())
            {
                clause = "password='" + string(szMd5) + "'";
            }

            if (!strNickname.empty())
            {
                if (clause.empty())
                {
                    clause = "nick='" + strNickname + "'";
                }else{
                    clause += (", nick='" + strNickname + "'");
                }

                clause += (", updated=" + int2string(t_now));
            }
            
            strSql = "update IMUser set " + clause + " where name='" + strName + "'";
        }else{
            strSql = "insert into IMUser(name, password, nick, salt, created, updated) values('" + strName + "', '" + string(szMd5) + "', '" +
                        strNickname + "', '" + salt + "', "  + int2string(t_now) + ", " + int2string(t_now) + ")";
        }
        
        bool bSql = pDBConn->ExecuteUpdate(strSql.c_str());
        log("[register]user:%s register, current:%s, result:%s.", strName.c_str(), bExist?"exist":"not exist", bSql?"success":"fail");

        if (!bSql){
            log("[register] error execute sql:%s.", strSql.c_str());
        }

        // 保证昵称不为空
        uint32_t nUserId = CUserModel::getInstance()->getUserID(strName);

        DBUserInfo_t cUser;
        bool bRet = CUserModel::getInstance()->getUser(nUserId, cUser);
        if (bRet && cUser.strNick.empty()){

            DBUserInfo_t cNew;

            cNew.nId = nUserId;
            cNew.strNick = int2string(nUserId);

            CUserModel::getInstance()->updateUserInfo(cNew);
        }
        bRet = true;
    }
    return bRet;
}

bool CInterLoginStrategy::doLogin(const std::string &strName, const std::string &strPass, IM::BaseDefine::UserInfo& user)
{
    bool bRet = false;
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (pDBConn) {
        string strSql = "select * from IMUser where name='" + strName + "' and status=0";
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            string strResult, strSalt;
            uint32_t nId, nGender, nDeptId, nStatus, nMemberOrder, nBinded, nBirthday;
            string strNick, strAvatar, strEmail, strRealName, strTel, strDomain,strSignInfo;
            while (pResultSet->Next()) {
                nId = pResultSet->GetInt("id");
                strResult = pResultSet->GetString("password");
                strSalt = pResultSet->GetString("salt");
                strNick = pResultSet->GetString("nick");
                nGender = pResultSet->GetInt("sex");
                strRealName = pResultSet->GetString("name");
                strDomain = pResultSet->GetString("domain");
                strTel = pResultSet->GetString("phone");
                strEmail = pResultSet->GetString("email");
                strAvatar = pResultSet->GetString("avatar");
                nDeptId = pResultSet->GetInt("departId");
                nBinded = pResultSet->GetInt("binded");  
                nStatus = pResultSet->GetInt("status");
                strSignInfo = pResultSet->GetString("sign_info");
                nBirthday = pResultSet->GetInt("birthday");
                nMemberOrder = pResultSet->GetInt("memberorder");
            }

            string strInPass = strPass + strSalt;
            char szMd5[33];
            CMd5::MD5_Calculate(strInPass.c_str(), strInPass.length(), szMd5);
            string strOutPass(szMd5);
            
            log("USER:%s pass:%s login, salt:%s md5:%s, result:%s.", strName.c_str(), strPass.c_str(), strSalt.c_str(), szMd5, strResult.c_str());

            if(strOutPass == strResult)
            {
                bRet = true;
                user.set_user_id(nId);
                user.set_user_nick_name(strNick);
                user.set_user_gender(nGender);
                user.set_user_real_name(strRealName);
                user.set_user_domain(strDomain);
                user.set_user_tel(strTel);
                user.set_email(strEmail);
                user.set_avatar_url(strAvatar);
                user.set_department_id(nDeptId);
                user.set_member_order(nMemberOrder);
                user.set_binded(nBinded);
                user.set_status(nStatus);
  	            user.set_sign_info(strSignInfo);
                user.set_birthday(nBirthday);
            }
            delete  pResultSet;
        }else{
            log("USER:%s login failed, select IMUser error.", strName.c_str());
        }
    }else{
        log("DB connect of teamtalk_slave error.");
    }
    return bRet;
}
