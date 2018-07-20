#pragma once
#include <memory>
#include "IM.BaseDefine.pb.h"
#include "ImPduBase.h"
#include "public_define.h"
class CDBConn;
class CUserModel {
public:
    using DBConn = CDBConn;
    static CUserModel* getInstance();
    ~CUserModel();
    int32_t queryPublicUserInfo(
        uint32_t const reqUserId,
        std::string const& key,
        uint64_t const keyFlags,
        std::vector<IM::BaseDefine::UserInfo>& userInfo) noexcept;
    /**
     * Check whether user with @a username exists
     * @return
     * - 1 exists
     * - 0 not exists
     * - -1 error
     */
    int32_t hasUser(std::string const& username) noexcept;
    bool validUser(uint32_t const userId) noexcept;
    bool validUser(std::shared_ptr<DBConn> const& dbc, uint32_t const userId) noexcept;
    /// @note current ONLY valid whether all @a ids exists
    int32_t validUsers(std::set<uint32_t> const& ids) noexcept;
    int32_t validSingleChatable(uint32_t const fromId, uint32_t const toId) noexcept;
    void getChangedId(uint32_t& nLastTime, list<uint32_t>& lsIds);
    void getProxys(list<uint32_t>& lsIds, uint32_t& nAdminId);
    void getUsers(list<uint32_t> lsIds, list<IM::BaseDefine::UserInfo>& lsUsers);
    bool getUser(uint32_t nUserId, DBUserInfo_t& cUser);
    bool getUser(uint32_t nUserId, std::shared_ptr<CDBConn> pDBConn, DBUserInfo_t& cUser);
    
    uint32_t getUserID(std::string username);
    // 获取代理表ID
    uint32_t getProxyUserID(std::string target);

    bool updateUser(DBUserInfo_t& cUser);
    bool insertUser(DBUserInfo_t& cUser);
//    void getUserByNick(const list<string>& lsNicks, list<IM::BaseDefine::UserInfo>& lsUsers);
    void clearUserCounter(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nSessionType);
    void setCallReport(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::ClientType nClientType);

    bool updateUserSignInfo(uint32_t user_id, const string& sign_info);
    bool getUserSingInfo(uint32_t user_id, string* sign_info);
    bool updatePushShield(uint32_t user_id, uint32_t shield_status);
    /// Get non-group message push shield of @a userId
    bool getShield(uint32_t const userId, uint32_t& shield) noexcept;
    /// @deprecated use getShield() instead
    bool getPushShield(uint32_t user_id, uint32_t* shield_status);

    bool updateUserCurrency(uint32_t user_id, uint32_t nCurCurrency, list<IM::BaseDefine::IMCashFlow>& lsFlow);
    bool updateUserInfo(DBUserInfo_t& cUser);

    bool updateUserPassword(uint32_t user_id, string strPass);

private:
    CUserModel();
private:
    static CUserModel* m_pInstance;
};
