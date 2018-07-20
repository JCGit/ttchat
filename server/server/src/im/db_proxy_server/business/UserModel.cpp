#include "UserModel.h"
#include "../DBPool.h"
#include "../CachePool.h"
#include "Common.h"
#include "SyncCenter.h"
#include "ProxyModel.h"
#include "uchat/errno.hpp"
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
//#include "code.pb.h"
#include "EncDec.h"

constexpr char const* kN = "db_proxy_server";
using UserModel = CUserModel;
using DBManager = CDBManager;
using ProxyModel = CProxyModel;
using namespace uchat;
CUserModel* CUserModel::m_pInstance = NULL;

CUserModel::CUserModel()
{

}

CUserModel::~CUserModel()
{
    
}

CUserModel* CUserModel::getInstance()
{
    if(m_pInstance == NULL)
    {
        m_pInstance = new CUserModel();
    }
    return m_pInstance;
}
int32_t UserModel::queryPublicUserInfo(
    uint32_t const reqUserId,
    std::string const& key,
    uint64_t const keyFlags,
    std::vector<IM::BaseDefine::UserInfo>& userInfos) noexcept try {
    Debug(__func__ << ": keyFlags " << keyFlags)
    uint32_t const keyFlagsLow32 = keyFlags & 0xffffffffu;
    switch(keyFlagsLow32) {
    case 0x1:
    case 0x2:
    case 0x3: break;
    default: {
        Warning(__func__ << ": invalid keyFlagsLow32 " << keyFlagsLow32)
        return uchat::errnum::kInvalidParameter;
    } break;
    }
    //
    auto const dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    if (!dbc) {
        Error0(__func__ << ": getdbconn fail: teamtalk_slave")
        return uchat::errnum::kCannotConnectDatabase;
    }
    if (!this->validUser(dbc, reqUserId)) {
        Warning(__func__ << ": invalid reqUserId " << reqUserId)
        return uchat::errnum::kUserIdNotFound;
    }
    std::string sql;
    uint32_t const keyFlagsHigh32 =
        (keyFlags & (uint64_t(0xffffffffu) << 32)) >> 32;
    switch(keyFlagsLow32) {
    case 0x1: {
        sql = "SELECT * FROM `IMUser` WHERE `name` = '" + key + "' ";
    } break;
    case 0x2: {
        sql = "SELECT * FROM `IMUser` WHERE `nick` = '" + key + "' ";
    } break;
    case 0x3: {
        sql = "SELECT * FROM `IMUser` WHERE `name` = '" + key
            + "' OR `nick` = '" + key + "' ";
    } break;
    default: {
        Warning(__func__ << ": invalid keyFlagsLow32 " << keyFlagsLow32)
        return uchat::errnum::kInvalidParameter;
    } break;
    }
    std::string filterIdsSql;
    if (keyFlagsHigh32 & 0x1) {
        /// Filter out friends
        std::set<uint32_t> friendIds;
        ProxyModel::getInstance()->lsfrnds(reqUserId, friendIds, dbc);
        if (!friendIds.empty()) {
            auto it = friendIds.cbegin();
            filterIdsSql = std::to_string(*it);
            ++it;
            for (auto end = friendIds.cend(); it != end; ++it) {
                filterIdsSql += ", " + std::to_string(*it);
            }
        }
    }
    if (keyFlagsHigh32 & 0x2) {
        if (filterIdsSql.empty()) {
            filterIdsSql += std::to_string(reqUserId);
        } else {
            filterIdsSql += ", " + std::to_string(reqUserId);
        }
    }
    if (!filterIdsSql.empty()) {
        sql += " AND `id` NOT IN(" + filterIdsSql + ")";
    }
    sql += "ORDER BY `updated` DESC LIMIT "
        + std::to_string(uchat::kMaxUserQueryLimit);
    Debug(__func__ << ": sql " << sql)
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        Error0(__func__ << ": executeQuery fail " << sql)
        return uchat::errnum::kDatabaseQueryError;
    }
    userInfos.clear();
    IM::BaseDefine::UserInfo userInfo;
    for (auto it = qres.begin(), end = qres.end(); it != end;
        ++it) {
        using NumStrView = ::uchat::NumStrView;
        userInfo.set_user_id(uint32_t(it("id").as<NumStrView>()));
        using StringView = ::uchat::boost::StringView;
        userInfo.set_user_nick_name(std::string(it("nick").as<StringView>()));
        userInfo.set_user_gender(uint32_t(it("sex").as<NumStrView>()));
        userInfo.set_avatar_url(std::string(it("avatar").as<StringView>()));
        userInfo.set_user_real_name(std::string(it("name").as<StringView>()));
        userInfo.set_member_order(uint32_t(it("memberorder").as<NumStrView>()));
        userInfo.set_sign_info(std::string(it("sign_info").as<StringView>()));
        userInfo.set_birthday(uint32_t(it("birthday").as<NumStrView>()));
        userInfos.emplace_back(std::move(userInfo));
    }
    userInfos.shrink_to_fit();
    return 0;// OK
} catch(std::exception const& e) {
    Error0(e.what())
    return uchat::errnum::kException;
}
int32_t UserModel::hasUser(std::string const& username) noexcept try {
    auto const dbc = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    Assert(dbc, "getdbconn fail: teamtalk_slave")
    std::string const sql = "SELECT COUNT(*) AS `c` FROM `IMUser`"
        " WHERE `name` = '" + username + "';";
    Debug(__func__ << ": sql: " << sql)
    auto const qres = dbc->executeQuery(sql);
    Assert(qres && !qres.isEmpty(), "query fail: " << sql)
    uint32_t const didCount = uint32_t(qres.begin()("c").as<uchat::NumStrView>());
    if (didCount > 0) {
        return 1;
    } else {
        return 0;
    }
} catch(std::exception const& e) {
    Error0(e.what())
    return -1;
}
bool UserModel::validUser(
    std::shared_ptr<DBConn> const& dbc, uint32_t const userId) noexcept
    try {
    Assert(dbc, "nil dbc")
    // Current only valid exists
    std::string const sql = "SELECT COUNT(*) AS `c` FROM `IMUser`"
        " WHERE `id` = " + std::to_string(userId);
    Debug(__func__ << ": sql: " << sql)
    auto const qres = dbc->executeQuery(sql);
    Assert(qres && !qres.isEmpty(), "query fail: " << sql)
    uint32_t const didCount = uint32_t(
        qres.begin()("c").as<uchat::NumStrView>());
    Assert(1 == didCount, "expect 1 " << " but did got " << didCount)
    return true;
} catch(std::exception const& e) {
    Error0(e.what())
    return false;
}
int32_t UserModel::validUsers(std::set<uint32_t> const& userIds) noexcept try {
    if (userIds.empty()) {
        return proto::code::Generic::Valid;
    }
    auto const dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    Assert(dbc, "getdbconn fail: teamtalk_slave")
    auto it = userIds.cbegin();
    std::string idsSql = std::to_string(*it);
    ++it;
    for (auto end = userIds.cend(); it != end; ++it) {
        idsSql += ", " + std::to_string(*it);
    }
    // Current only valid exists
    std::string const sql = "SELECT COUNT(*) AS `c` FROM `IMUser` WHERE `id` IN (" + idsSql + ");";
    Debug(__func__ << ": sql: " << sql)
    auto const qres = dbc->executeQuery(sql);
    Assert(qres && !qres.isEmpty(), "query fail: " << sql)
    uint32_t const didCount = uint32_t(qres.begin()("c").as<uchat::NumStrView>());
    if (didCount != userIds.size()) {
        Debug(__func__ << ": expect " << userIds.size() << " but did got " << didCount)
        return proto::code::Generic::Invalid;
    }
    return proto::code::Generic::Valid;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t UserModel::validSingleChatable(uint32_t const fromId, uint32_t const toId) noexcept try {
    auto const dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    Assert(dbc, "getdbconn fail: teamtalk_slave")
    // Check friend
    std::string sql = "SELECT `id` FROM `IMFriend` WHERE ((`fromId` = " + std::to_string(fromId) + " AND `toId` = " + std::to_string(toId) + ") OR (`fromId` = " + std::to_string(toId) + " AND `toId` = " + std::to_string(fromId) + ")) AND `status` IN (0)";
    Debug(__func__ << ": sql: " << sql)
    auto qres = dbc->executeQuery(sql);
    Assert(qres, "query fail: " << sql)
    if (!qres.isEmpty()) {
        // fromId and toId are frineds and status is 0
        return proto::code::Generic::Valid;
    }
    // Check proxy -- subline
    // proxy is toId
    sql = "SELECT `id` FROM `IMUser` WHERE (`id` = " + std::to_string(fromId) +" AND `binded` = " + std::to_string(toId) + " AND `binded` IN (SELECT `id` FROM `IMProxy` WHERE `id` = " + std::to_string(toId) + " AND `status` = 0))";
    Debug(__func__ << ": sql: " << sql)
    qres = dbc->executeQuery(sql);
    Assert(qres, "query fail: " << sql)
    if (!qres.isEmpty()) {
        // fromId is subline and toId is proxy and proxy status is 0
        return proto::code::Generic::Valid;
    }
    // proxy is fromId
    sql = "SELECT `id` FROM `IMUser` WHERE (`id` = " + std::to_string(toId) +" AND `binded` = " + std::to_string(fromId) + " AND `binded` IN (SELECT `id` FROM `IMProxy` WHERE `id` = " + std::to_string(fromId) + " AND `status` = 0))";
    Debug(__func__ << ": sql: " << sql)
    qres = dbc->executeQuery(sql);
    Assert(qres, "query fail: " << sql)
    if (!qres.isEmpty()) {
        // fromId is proxy and toId is subline and proxy status is 0
        return proto::code::Generic::Valid;
    }
    return proto::code::Generic::Invalid;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
bool UserModel::validUser(uint32_t const userId) noexcept try {
    auto const dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    return this->validUser(dbc, userId);
} catch(std::exception const& e) {
    Error0(e.what())
    return false;
}
void CUserModel::getChangedId(uint32_t& nLastTime, list<uint32_t> &lsIds)
{
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (pDBConn)
    {
        string strSql ;
        if(nLastTime == 0)
        {
            strSql = "select id, updated from IMUser where status != 3";
        }
        else
        {
            strSql = "select id, updated from IMUser where updated>=" + int2string(nLastTime);
        }
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next()) {
                uint32_t nId = pResultSet->GetInt("id");
                uint32_t nUpdated = pResultSet->GetInt("updated");
        	 if(nLastTime < nUpdated)
                {
                    nLastTime = nUpdated;
                }
                lsIds.push_back(nId);
  		}
            delete pResultSet;
        }
        else
        {
            log(" no result set for sql:%s", strSql.c_str());
        }
    }
    else
    {
        log("no db connection for teamtalk_slave");
    }
}

void CUserModel::getProxys(list<uint32_t>& lsIds, uint32_t& nAdminId)
{
    if (auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave")) {
        string strSql = "select id, updated from IMUser where memberorder=" + int2string(IM::BaseDefine::MEMBER_PROXY);

        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next()) {
                uint32_t nId = pResultSet->GetInt("id");
                lsIds.push_back(nId);
  		    }
              
            delete pResultSet;
        }
        else
        {
            log(" no result set for sql:%s", strSql.c_str());
        }

        strSql = "select * from IMUser where memberorder=" + int2string(IM::BaseDefine::MEMBER_ADMIN);
        pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next()) {
                nAdminId = pResultSet->GetInt("id");
  		    }
              
            delete pResultSet;
        }
    }
    else
    {
        log("no db connection for teamtalk_slave");
    }
}

void CUserModel::getUsers(list<uint32_t> lsIds, list<IM::BaseDefine::UserInfo> &lsUsers)
{
    if (lsIds.empty()) {
        log("list is empty");
        return;
    }
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (pDBConn)
    {
        string strClause;
        bool bFirst = true;
        for (auto it = lsIds.begin(); it!=lsIds.end(); ++it)
        {
            if(bFirst)
            {
                bFirst = false;
                strClause += int2string(*it);
            }
            else
            {
                strClause += ("," + int2string(*it));
            }
        }
        string  strSql = "select * from IMUser where id in (" + strClause + ")"
            + " ORDER BY `name` LIMIT "
            + std::to_string(uchat::kMaxUserQueryLimit);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next())
            {
                IM::BaseDefine::UserInfo cUser;
                cUser.set_user_id(pResultSet->GetInt("id"));
                cUser.set_user_gender(pResultSet->GetInt("sex"));
                cUser.set_user_nick_name(pResultSet->GetString("nick"));
                cUser.set_user_domain(pResultSet->GetString("domain"));
                cUser.set_user_real_name(pResultSet->GetString("name"));
                cUser.set_user_tel(pResultSet->GetString("phone"));
                cUser.set_email(pResultSet->GetString("email"));
                cUser.set_avatar_url(pResultSet->GetString("avatar"));
		cUser.set_sign_info(pResultSet->GetString("sign_info"));
             
                cUser.set_department_id(pResultSet->GetInt("departId"));
  		 cUser.set_department_id(pResultSet->GetInt("departId"));
                cUser.set_status(pResultSet->GetInt("status"));
                cUser.set_member_order(pResultSet->GetInt("memberorder"));
                cUser.set_binded(pResultSet->GetInt("binded"));
                cUser.set_birthday(pResultSet->GetInt("birthday"));

                lsUsers.push_back(cUser);
            }
            delete pResultSet;
        }
        else
        {
            log(" no result set for sql:%s", strSql.c_str());
        }
    }
    else
    {
        log("no db connection for teamtalk_slave");
    }
}

bool CUserModel::getUser(uint32_t nUserId, DBUserInfo_t &cUser)
{
    bool bRet = false;
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (pDBConn)
    {
        string strSql = "select * from IMUser where id="+int2string(nUserId);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next())
            {
                cUser.nId = pResultSet->GetInt("id");
                cUser.nSex = pResultSet->GetInt("sex");
                cUser.strNick = pResultSet->GetString("nick");
                cUser.strDomain = pResultSet->GetString("domain");
                cUser.strName = pResultSet->GetString("name");
                cUser.strTel = pResultSet->GetString("phone");
                cUser.strEmail = pResultSet->GetString("email");
                cUser.strAvatar = pResultSet->GetString("avatar");
                cUser.sign_info = pResultSet->GetString("sign_info");
                cUser.nDeptId = pResultSet->GetInt("departId");
                cUser.nStatus = pResultSet->GetInt("status");
                cUser.nCurrency = pResultSet->GetInt("currency");
                bRet = true;
            }
            delete pResultSet;
        }
        else
        {
            log("no result set for sql:%s", strSql.c_str());
        }
    }
    else
    {
        log("no db connection for teamtalk_slave");
    }
    return bRet;
}

bool CUserModel::getUser(uint32_t nUserId, std::shared_ptr<CDBConn> pDBConn, DBUserInfo_t& cUser)
{
    bool bRet = false;
    if (pDBConn)
    {
        string strSql = "select * from IMUser where id="+int2string(nUserId);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next())
            {
                cUser.nId = pResultSet->GetInt("id");
                cUser.nSex = pResultSet->GetInt("sex");
                cUser.strNick = pResultSet->GetString("nick");
                cUser.strDomain = pResultSet->GetString("domain");
                cUser.strName = pResultSet->GetString("name");
                cUser.strTel = pResultSet->GetString("phone");
                cUser.strEmail = pResultSet->GetString("email");
                cUser.strAvatar = pResultSet->GetString("avatar");
                cUser.sign_info = pResultSet->GetString("sign_info");
                cUser.nDeptId = pResultSet->GetInt("departId");
                cUser.nStatus = pResultSet->GetInt("status");
                cUser.nCurrency = pResultSet->GetInt("currency");
                bRet = true;
            }
            delete pResultSet;
        }
        else
        {
            log("no result set for sql:%s", strSql.c_str());
        }
    }
    else
    {
        log("no db connection for teamtalk_slave");
    }
    return bRet;
}

uint32_t CUserModel::getUserID(std::string username)
{
    const auto pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (pDBConn)
    {
        string strSql = "select * from IMUser where name='" + username + "'";

        uint32_t nUserId ;
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if (pResultSet)
        {
            while (pResultSet->Next()){
                nUserId = pResultSet->GetInt("id");
            }

            delete pResultSet;
            pResultSet = NULL;
        }
        return nUserId;
    }
    return 0;
}

uint32_t CUserModel::getProxyUserID(std::string target)
{
    const auto pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (pDBConn)
    {
        string strSql = "select * from IMProxy where name='" + target + "'" + " or id=" + target ;

        uint32_t nUserId ;
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if (pResultSet)
        {
            while (pResultSet->Next()){
                nUserId = pResultSet->GetInt("id");
            }

            delete pResultSet;
            pResultSet = NULL;
        }
        return nUserId;
    }
    return 0;
}

bool CUserModel::updateUser(DBUserInfo_t &cUser)
{
    bool bRet = false;
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master");
    if (pDBConn)
    {
        if (cUser.strNick.empty()){
            cUser.strNick = int2string(cUser.nId);
        }

        uint32_t nNow = (uint32_t)time(NULL);
        string strSql = "update IMUser set `sex`=" + int2string(cUser.nSex)+ ", `nick`=" + cUser.strNick +", `domain`="+ cUser.strDomain + 
                                        ", `name`=" + cUser.strName + ", `phone`=" + cUser.strTel + ", `email`=" + cUser.strEmail + 
                                        ", `avatar`=" + cUser.strAvatar + ", `sign_info`=" + cUser.sign_info +", `departId`=" + int2string(cUser.nDeptId) + 
                                        ", `status`=" + int2string(cUser.nStatus) + ", 'currency'=" + int2string(cUser.nCurrency) + ", `updated`=" + int2string(nNow) + 
                                        " where id="+int2string(cUser.nId);
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            log("updateUser: update failed:%s", strSql.c_str());
        }
    }
    else
    {
        log("no db connection for teamtalk_master");
    }
    return bRet;
}

bool CUserModel::insertUser(DBUserInfo_t &cUser)
{
    bool bRet = false;
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master");
    if (pDBConn)
    {
        if (cUser.strNick.empty()){
            cUser.strNick = int2string(cUser.nId);
        }

        string strSql = "insert into IMUser(`id`,`sex`,`nick`,`domain`,`name`,`phone`,`email`,`avatar`,`sign_info`,`departId`,`status`,`created`,`updated`) values(?,?,?,?,?,?,?,?,?,?,?,?)";
        CPrepareStatement* stmt = new CPrepareStatement();
        if (stmt->Init(pDBConn->GetMysql(), strSql))
        {
            uint32_t nNow = (uint32_t) time(NULL);
            uint32_t index = 0;
            uint32_t nGender = cUser.nSex;
            uint32_t nStatus = cUser.nStatus;
            stmt->SetParam(index++, cUser.nId);
            stmt->SetParam(index++, nGender);
            stmt->SetParam(index++, cUser.strNick);
            stmt->SetParam(index++, cUser.strDomain);
            stmt->SetParam(index++, cUser.strName);
            stmt->SetParam(index++, cUser.strTel);
            stmt->SetParam(index++, cUser.strEmail);
            stmt->SetParam(index++, cUser.strAvatar);
            
            stmt->SetParam(index++, cUser.sign_info);
            stmt->SetParam(index++, cUser.nDeptId);
            stmt->SetParam(index++, nStatus);
            stmt->SetParam(index++, nNow);
            stmt->SetParam(index++, nNow);
            bRet = stmt->ExecuteUpdate();
            
            if (!bRet)
            {
                log("insert user failed: %s", strSql.c_str());
            }
        }
        delete stmt;
    }
    else
    {
        log("no db connection for teamtalk_master");
    }
    return bRet;
}

void CUserModel::clearUserCounter(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nSessionType)
{
    if(IM::BaseDefine::SessionType_IsValid(nSessionType))
    {
        CacheManager* pCacheManager = CacheManager::getInstance();
        CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
        if (pCacheConn)
        {
            // Clear P2P msg Counter
            if(nSessionType == IM::BaseDefine::SESSION_TYPE_SINGLE)
            {
                int nRet = pCacheConn->hdel("unread_" + int2string(nUserId), int2string(nPeerId));
                if(!nRet)
                {
                    log("hdel failed %d->%d", nPeerId, nUserId);
                }
            }
            // Clear Group msg Counter
            else if(nSessionType == IM::BaseDefine::SESSION_TYPE_GROUP)
            {
                string strGroupKey = int2string(nPeerId) + GROUP_TOTAL_MSG_COUNTER_REDIS_KEY_SUFFIX;
                map<string, string> mapGroupCount;
                bool bRet = pCacheConn->hgetAll(strGroupKey, mapGroupCount);
                if(bRet)
                {
                    string strUserKey = int2string(nUserId) + "_" + int2string(nPeerId) + GROUP_USER_MSG_COUNTER_REDIS_KEY_SUFFIX;
                    string strReply = pCacheConn->hmset(strUserKey, mapGroupCount);
                    if(strReply.empty()) {
                        log("hmset %s failed !", strUserKey.c_str());
                    }
                }
                else
                {
                    log("hgetall %s failed!", strGroupKey.c_str());
                }
                
            }
            pCacheManager->RelCacheConn(pCacheConn);
        }
        else
        {
            log("no cache connection for unread");
        }
    }
    else{
        log("invalid sessionType. userId=%u, fromId=%u, sessionType=%u", nUserId, nPeerId, nSessionType);
    }
}

void CUserModel::setCallReport(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::ClientType nClientType)
{
    if(IM::BaseDefine::ClientType_IsValid(nClientType))
    {
        auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master");
        if(pDBConn)
        {
            string strSql = "insert into IMCallLog(`userId`, `peerId`, `clientType`,`created`,`updated`) values(?,?,?,?,?)";
            CPrepareStatement* stmt = new CPrepareStatement();
            if (stmt->Init(pDBConn->GetMysql(), strSql))
            {
                uint32_t nNow = (uint32_t) time(NULL);
                uint32_t index = 0;
                uint32_t nClient = (uint32_t) nClientType;
                stmt->SetParam(index++, nUserId);
                stmt->SetParam(index++, nPeerId);
                stmt->SetParam(index++, nClient);
                stmt->SetParam(index++, nNow);
                stmt->SetParam(index++, nNow);
                bool bRet = stmt->ExecuteUpdate();
                
                if (!bRet)
                {
                    log("insert report failed: %s", strSql.c_str());
                }
            }
            delete stmt;
        }
        else
        {
            log("no db connection for teamtalk_master");
        }
        
    }
    else
    {
        log("invalid clienttype. userId=%u, peerId=%u, clientType=%u", nUserId, nPeerId, nClientType);
    }
}


bool CUserModel::updateUserSignInfo(uint32_t user_id, const string& sign_info) {
   
    if (sign_info.length() > 128) {
        log("updateUserSignInfo: sign_info.length()>128.\n");
        return false;
    }
    bool rv = false;
    auto const db_conn = CDBManager::getInstance()->getdbconn("teamtalk_master");
    if (db_conn) {
        uint32_t now = (uint32_t)time(NULL);
        string str_sql = "update IMUser set `sign_info`='" + sign_info + "', `updated`=" + int2string(now) + " where id="+int2string(user_id);
        rv = db_conn->ExecuteUpdate(str_sql.c_str());
        if(!rv) {
            log("updateUserSignInfo: update failed:%s", str_sql.c_str());
        }else{
                CSyncCenter::getInstance()->updateTotalUpdate(now);
           
        }
        } else {
            log("updateUserSignInfo: no db connection for teamtalk_master");
            }
    return rv;
    }

bool CUserModel::getUserSingInfo(uint32_t user_id, string* sign_info) {
    bool rv = false;
    auto const db_conn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (db_conn) {
        string str_sql = "select sign_info from IMUser where id="+int2string(user_id);
        CResultSet* result_set = db_conn->ExecuteQuery(str_sql.c_str());
        if(result_set) {
            if (result_set->Next()) {
                *sign_info = result_set->GetString("sign_info");
                rv = true;
                }
            delete result_set;
            } else {
                        log("no result set for sql:%s", str_sql.c_str());
                   }
        } else {
                    log("no db connection for teamtalk_slave");
               }
    return rv;
   }

bool CUserModel::updatePushShield(uint32_t user_id, uint32_t shield_status) {
    bool rv = false;
    auto const db_conn = CDBManager::getInstance()->getdbconn("teamtalk_master");
    if (db_conn) {
        uint32_t now = (uint32_t)time(NULL);
        string str_sql = "update IMUser set `push_shield_status`="+ int2string(shield_status) + ", `updated`=" + int2string(now) + " where id="+int2string(user_id);
        rv = db_conn->ExecuteUpdate(str_sql.c_str());
        if(!rv) {
            log("updatePushShield: update failed:%s", str_sql.c_str());
        }
    } else {
        log("updatePushShield: no db connection for teamtalk_master");
    }
    
    return rv;
}
bool UserModel::getShield(uint32_t const userId, uint32_t& shield) noexcept try {
    auto const dbc = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    Assert(dbc, "getdbconn fail teamtalk_slave")
    std::string const sql = "SELECT `push_shield_status` FROM `IMUser` WHERE"
        " `id` = " + std::to_string(userId);
    auto const qres = dbc->executeQuery(sql);
    Assert(qres, "executeQuery fail")
    auto const it = qres.begin();
    Assert(it != qres.end(), "no such user " << userId)
    shield = uint32_t(it("push_shield_status").as<uchat::NumStrView>());
    return true;
} catch(std::exception const& e) {
    Error0(e.what());
    return false;
}
bool CUserModel::getPushShield(uint32_t user_id, uint32_t* shield_status) {
    bool rv = false;
    auto const db_conn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (db_conn) {
        string str_sql = "select push_shield_status from IMUser where id="+int2string(user_id);
        CResultSet* result_set = db_conn->ExecuteQuery(str_sql.c_str());
        if(result_set) {
            if (result_set->Next()) {
                *shield_status = result_set->GetInt("push_shield_status");
                rv = true;
            }
            delete result_set;
        } else {
            log("getPushShield: no result set for sql:%s", str_sql.c_str());
        }
    } else {
        log("getPushShield: no db connection for teamtalk_slave");
    }
    
    return rv;
}

bool CUserModel::updateUserCurrency(uint32_t user_id, uint32_t nCurCurrency, list<IM::BaseDefine::IMCashFlow>& lsFlow)
{
    // 获取用户之前的记录
    DBUserInfo_t cUser;
    getUser(user_id, cUser);

    DBUserInfo_t nUser;
    nUser.nId = user_id;
    nUser.nCurrency = nCurCurrency;

    if (updateUserInfo(nUser))
    {
        int nChange = 0;
        int nBeforce = cUser.nCurrency;

        const auto db_conn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
        if (db_conn)
        {
             // 插入流水记录
            list<IM::BaseDefine::IMCashFlow>::iterator it = lsFlow.begin();
            for(; it!=lsFlow.end(); ++it)
            {
                IM::BaseDefine::IMCashFlow& flow = *it;

                nChange += flow.cash();

                string sql = "insert into IMCurrencyRecord(type, takeoff, after, detail) values (" + int2string(flow.consume()) + ", " +
                        int2string(flow.cash()) + ", " + int2string(cUser.nCurrency + nChange) + ", " + flow.detail() + ")";

                db_conn->ExecuteUpdate(sql.c_str());    
            }
        }
    }
}

bool CUserModel::updateUserInfo(DBUserInfo_t& cUser)
{
    auto const db_conn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (db_conn) {

        vector<string> clause;
        if (cUser.nSex == 1 || cUser.nSex == 2){
            string sz = "sex=" + int2string(cUser.nSex);
            clause.push_back(sz);
        }

        if (!cUser.strNick.empty()){
            string sz = "nick='" + cUser.strNick + "'";
            clause.push_back(sz);
        }
        
        if (cUser.nBirthday != 0){
            string sz = "birthday=" + int2string(cUser.nBirthday);
            clause.push_back(sz);
        }

        if (!cUser.strAvatar.empty()){
            string sz = "avatar='" + cUser.strAvatar + "'";
            clause.push_back(sz);
        }

        if (cUser.nCurrency != 0){
            string sz = "currency=" + int2string(cUser.nCurrency);
            clause.push_back(sz);
        }

        string sql = "update IMUser set ";
        for (int i=0; i!=clause.size(); ++i)
        {
            
            sql.append(clause.at(i));

            if (i != clause.size() - 1)
            {
                sql.append(",");
            }
        }

        sql.append(" where id=" + int2string(cUser.nId));

        db_conn->ExecuteUpdate(sql.c_str());
        return true;
    } else {
        log("updateUserInfo: no db connection for teamtalk_slave");
    }
    return false;
}

bool CUserModel::updateUserPassword(uint32_t user_id, string strPass)
{
    bool bRet = false;
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (pDBConn) {
        string salt;
        string sql = "select * from IMUser where id=" + int2string(user_id);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(sql.c_str());
        if (pResultSet)
        {
            while (pResultSet->Next()){
                salt = pResultSet->GetString("salt");
            }

            delete pResultSet;
            pResultSet = NULL;
        }

        string strRealPass = strPass + salt;

        char szMd5[33];
        CMd5::MD5_Calculate(strRealPass.c_str(), strRealPass.length(), szMd5);
        sql = "update IMUser set password='" + string(szMd5) + "' where id=" + int2string(user_id);
        pDBConn->ExecuteUpdate(sql.c_str());
        bRet = true;
    }
    return bRet;
}