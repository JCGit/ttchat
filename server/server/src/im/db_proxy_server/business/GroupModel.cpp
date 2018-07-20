#include <sstream>
#include "../DBPool.h"
#include "../CachePool.h"
#include "GroupModel.h"
#include "ImPduBase.h"
#include "Common.h"
#include "AudioModel.h"
#include "UserModel.h"
#include "GroupMessageModel.h"
#include "public_define.h"
#include "SessionModel.h"
#include "ProxyModel.h"
#include "json/json.h"
//#include "uchat/proto/code.pb.h"
//#include "code.pb.h"
#include "uchat/errno.hpp"
#include "uchat/config.hpp"
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
constexpr char const* kN = "db_proxy_server";
using UserModel = CUserModel;
using GroupModel = CGroupModel;
using DBManager = CDBManager;
using GroupMessageModel = CGroupMessageModel;
using namespace uchat;
using namespace uchat::boost;
using namespace IM::BaseDefine;
int32_t GroupModel::newgrp(uint32_t userId, std::string const& gnm, std::string const& gat, uint32_t gtp, std::set<uint32_t> const& mids, uint32_t& groupId) noexcept try {
    Info0(__func__ << ": mids size " << mids.size())
    if (gnm.empty()) {
        Error0("empty group name")
        return proto::code::Generic::Invalid;
    }
#   if 0
    // Check group members 1/2: count: min is 2
    if (mids.cend() == mids.find(userId)) {
        Assert(!mids.empty(), "empty")
    } else {
        Assert(mids.size() > 1, "empty")
    }
#  endif // if 0
    // Check group members 1/2: count: min is 1 => not checkout
    // Check group members 2/2: exists userId
    int32_t code = UserModel::getInstance()->validUsers(mids);
    if (proto::code::Generic::Valid != code) {
        Error0(__func__ << ": invalid user id found")
        return code;
    }
    // 1/2 Create an empty IMGroup
    groupId = INVALID_VALUE;
    if (!this->newegrp(userId, gnm, gat, gtp, groupId)) {
        Error0(__func__ << ": newegrp fail")
        return proto::code::Generic::Exception;
    }
    Info0(__func__ << ": create group id: " << groupId)
    if (!GroupMessageModel::getInstance()->resetMsgId(groupId)) {
        Warning("reset msgId failed. groupId = " << groupId)
    }
    // 2/2 rstGrpm
    if (!this->rstGrpm(groupId, mids)) {
        Warning(__func__ << ": rstGrpm fail")
    }
    return proto::code::Generic::Ok;
} catch(...) {
    // Rollback
    if (groupId != INVALID_VALUE) {
        Note("Rollback to deleteGroup id " << groupId)
        this->deleteGroup(groupId);
    }
    return proto::code::Generic::Exception;
}
bool GroupModel::newegrp(uint32_t userId, std::string const& gnm, std::string const& gat, uint8_t gtp, uint32_t& groupId) noexcept try {
    groupId = INVALID_VALUE;
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_master");
    Assert(dbc, "getdbconn fail")
    //
    std::string sql = "INSERT INTO `IMGroup`(`name`, `avatar`, "
        "`creator`, `type`,`userCnt`, `status`, `version`, "
        "`lastChated`, `updated`, `created`) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    Debug("sql " << sql)
    DBViewedPrepareStatement stmt;
    stmt.init(dbc->GetMysql(), std::move(sql), 10);
    uint32_t const memCnt = 0u;
    uint32_t const state = 0u;
    uint32_t const version = 1u;
    uint32_t const lastChat = 0u;
    uint32_t const now = ::time(nullptr);
    stmt <<
        gnm,    gat,   userId,  gtp,
        memCnt, state, version, lastChat,
        now,    now;
    groupId = stmt.execute();
    return true;
} catch(std::exception const& e) {
    Error0(e.what())
    return false;
}
bool GroupModel::deleteGroup(uint32_t groupId) noexcept try {
    auto dbc = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    Assert(dbc, "getdbconn fail")
    std::string sql = "DELETE FROM `IMGroup` WHERE `id` = " + std::to_string(groupId);
    return dbc->ExecuteUpdate(sql.c_str());
} catch(...) {
    return false;
}
int32_t GroupModel::delm(uint32_t groupId, std::set<uint32_t> const& ids, DBCP const& dbc) noexcept try {
    if (ids.empty()) {
        return proto::code::Generic::Ok;
    }
    if (!dbc) {
        Error0("nil dbc")
        return proto::code::Database::NoConnection;
    }
    auto it = ids.begin();
    std::string idsSql = std::to_string(*it);
    ++it;
    for (auto end = ids.end(); it != end; ++it) {
        idsSql += ", " + std::to_string(*it);
    }
    string const sql = "UPDATE `IMGroupMember` SET `status` = 1 WHERE"
        " `groupId` = " + std::to_string(groupId)
        + " AND `userId` IN (" + idsSql + ") AND `userId` NOT IN (SELECT"
        " `creator` FROM `IMGroup` WHERE `id` = " + std::to_string(groupId)
        + " AND `status` = 0)";
    Debug(__func__ << ": " << sql)
    int32_t ret;
    if (!dbc->ExecuteUpdate(sql.c_str())) {
        Warning(__func__ << ": " << sql)
        ret = proto::code::Database::ExecuteError;
    } else {
        // Update count
        std::string const sql = "UPDATE `IMGroup` SET `userCnt` = (SELECT"
            " COUNT(*) FROM `IMGroupMember` WHERE `groupId` = "
            + std::to_string(groupId) + " AND `status` IN(0, 2))"
            ", `updated` = " + to_string(::time(nullptr)) + " WHERE"
            " `id` = " + std::to_string(groupId) + " AND `status` = 0";
        Debug(__func__ << ": " << sql)
        dbc->ExecuteUpdate(sql.c_str());
        ret = proto::code::Generic::Ok;
    }
    CacheManager* cacheManager = CacheManager::getInstance();
    CacheConn* cacheConn = cacheManager->GetCacheConn("group_member");
    if (cacheConn) {
        // 从redis中删除成员
        std::string const key = "group_member_" + std::to_string(groupId);
        for (auto const& id : ids) {
            std::string const field = std::to_string(id);
            cacheConn->hdel(key, field);
        }
        cacheManager->RelCacheConn(cacheConn);
    }
    return ret;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::delm(uint32_t groupId, std::set<uint32_t> const& ids) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_master");
    return this->delm(groupId, ids, dbc);
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
bool GroupModel::rstGrpm(uint32_t groupId, std::set<uint32_t> const& mids) noexcept try {
    this->clearGroupMember(groupId);
    Assert(proto::code::Generic::Ok == this->addmem(groupId, mids), "addmem fail")
    return true;
} catch(std::exception const& e) {
    Error0(e.what())
    // Rollback
    this->clearGroupMember(groupId);
    return false;
}
int32_t GroupModel::addmem(uint32_t groupId, std::set<uint32_t> const& mids) noexcept try {
    if (mids.empty()) {
        Warning(__func__ << ": empty mids")
        return proto::code::Generic::Invalid;
    }
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_master");
    if (!dbc) {
        Warning(__func__ << ": getdbconn fail: teamtalk_master")
        return proto::code::Database::NoConnection;
    }
    std::set<uint32_t> existsMembers;
    {
        // 获取和更新已经存在群里的用户
        auto it = mids.begin();
        std::string idsSql = std::to_string(*it);
        ++it;
        for (auto end = mids.cend(); it != end; ++it) {
            idsSql += ", " + std::to_string(*it);
        }
        std::string const sql = "SELECT `userId` FROM `IMGroupMember` "
            "WHERE `groupId` = " + std::to_string(groupId)
            + " AND `userId` IN (" + idsSql + ")";
        DBQueryResult const qres = dbc->executeQuery(sql.c_str());
        if (qres && !qres.isEmpty()) {
            for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
                if (auto const v = it("userId").as<NumStrView>()) {
                    existsMembers.insert(uint32_t(v));
                }
            }
        } else {
            Detail("no result for query exist group members sql: " << sql)
        }
    }
    uint32_t const now = ::time(nullptr);
    // 设置已经存在群中人的状态
    if (!existsMembers.empty()) {
        auto it = existsMembers.cbegin();
        std::string idsSql = std::to_string(*it);
        ++it;
        for (auto end = existsMembers.cend(); it != end; ++it) {
            idsSql += ", " + std::to_string(*it);
        }
        std::string const sql = "UPDATE `IMGroupMember` SET `status` ="
            " 0, `updated` = " + std::to_string(now)
            + " WHERE `groupId` = " + std::to_string(groupId)
            + " AND `userId` in (" + idsSql + ")";
        if (!dbc->ExecuteUpdate(sql.c_str())) {
            Error0(__func__ << ": error " << sql)
            return proto::code::Database::ExecuteError;
        }
    }
    // 插入新成员
    {
        std::string const sql = "INSERT INTO `IMGroupMember`(`groupId`, "
            "`userId`, `status`, `created`, `updated`, `nick`) VALUES"
            " (?,?,?,?,?,?)";
        uint32_t const state = 0;
        DBViewedPrepareStatement stmt;
        stmt.init(dbc->GetMysql(), sql, 6);
        for (auto it = mids.cbegin(), end = mids.cend(); it != end; ++it) {
            uint32_t const userId = *it;

            DBUserInfo_t cUser;
            bool bRet = CUserModel::getInstance()->getUser(userId, dbc, cUser);
            if (!bRet){
                continue;
            }

            std::string nick = cUser.strNick;

            if (existsMembers.cend() == existsMembers.find(userId)) {
                stmt << groupId, userId, state, now, now, nick;
                stmt.execute();
            }
        }
    }
    // Update count
    std::string const sql = "UPDATE `IMGroup` SET `userCnt` = (SELECT"
        " COUNT(*) FROM `IMGroupMember` WHERE `groupId` = "
        + std::to_string(groupId) + " AND `status` IN(0, 2))"
        ", `updated` = " + to_string(::time(nullptr)) + " WHERE"
        " `id` = " + std::to_string(groupId) + " AND `status` = 0";
    Debug(__func__ << ": " << sql)
    dbc->ExecuteUpdate(sql.c_str());
    // 更新一份到 redis 中
    CacheManager* const cacheManager = CacheManager::getInstance();
    CacheConn* cacheConn = cacheManager->GetCacheConn("group_member");
    Assert(cacheConn, "no cache connection")
    std::string const key = "group_member_" + std::to_string(groupId);
    for (auto it = mids.cbegin(), end = mids.cend(); it != end; ++it) {
        cacheConn->hset(key, std::to_string(*it), std::to_string(now));
    }
    cacheManager->RelCacheConn(cacheConn);
    return proto::code::Generic::Ok;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::modGrpnm(uint32_t groupId, std::string const& newName, std::set<uint32_t>& ids) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_master");
    Assert(dbc, "getdbconn fail: teamtalk_master")
    {
        std::string const sql = "UPDATE `IMGroup` SET `name` = '"
            + newName + "', `updated` = "
            + std::to_string(::time(nullptr)) + " WHERE `id` = "
            + std::to_string(groupId) + " AND `status` = 0 AND `name` != '"
            + newName + "';";
        if (!dbc->ExecuteUpdate(sql.c_str())) {
            Debug(__func__ << ": " << sql)
            return proto::code::Database::NoChange;
        }
    }
    return this->fetMemids(groupId, ids, dbc);
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::modGrpa(uint32_t groupId, std::string const& newa, std::set<uint32_t>& ids) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_master");
    Assert(dbc, "getdbconn fail: teamtalk_master")
    {
        std::string const sql = "UPDATE `IMGroup` SET `announcement` = '"
            + newa + "', `updated` = "
            + std::to_string(::time(nullptr)) + " WHERE `id` = "
            + std::to_string(groupId) + " AND `status` = 0";/* AND"
            " `announcement` != '" + newa + "';";*/
        if (!dbc->ExecuteUpdate(sql.c_str())) {
            Debug(__func__ << ": " << sql)
            return proto::code::Database::NoChange;
        }
    }
    return this->fetMemids(groupId, ids, dbc);
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::modGrpms(uint32_t groupId, uint32_t opUserId, GroupModifyType type, std::set<uint32_t> const& upuids, std::set<uint32_t>& ids) noexcept try {
    int32_t code;
    switch (type) {
    case GroupModifyType::GROUP_MODIFY_TYPE_ADD: {
        code = this->validAddp(groupId, opUserId);
        if (proto::code::Generic::Valid != code) {
            return code;
        }
        code = this->addmem(groupId, upuids);
    } break;
    case GroupModifyType::GROUP_MODIFY_TYPE_DEL: {
        code = this->validDelp(groupId, opUserId, ids);
        if (proto::code::Generic::Valid != code) {
            return code;
        }
        code = this->delm(groupId, upuids);
        if (proto::code::Generic::Ok == code) {
            this->removeSession(groupId, upuids);
        }
    } break;
    case GroupModifyType::GROUP_MODIFY_TYPE_EXIT: {
        code = this->validGrpm(groupId, opUserId);
        if (proto::code::Generic::Valid != code) {
            return code;
        }
        std::set<uint32_t> upuid{ opUserId };
        code = this->delm(groupId, upuid);
        if (proto::code::Generic::Ok == code) {
            this->removeSession(groupId, upuid);
        }
    } break;
    case GroupModifyType::GROUP_MODIFY_PERM_ADMIN: {
        code = this->validGrpo(groupId, opUserId);
        if (proto::code::Generic::Valid != code) {
            return code;
        }
        code = this->modp(groupId, upuids, 2u);
    } break;
    case GroupModifyType::GROUP_MODIFY_PERM_NORMAL: {
        code = this->validGrpo(groupId, opUserId);
        if (proto::code::Generic::Valid != code) {
            return code;
        }
        code = this->modp(groupId, upuids, 0u);
    } break;
    default:
        code = proto::code::Database::InvalidGroupMemOp;
        break;
    }
    if (proto::code::Generic::Ok == code) {
        return this->fetMemids(groupId, ids);
    }
    return code;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::modp(uint32_t groupId, std::set<uint32_t> const& mids, uint32_t permission) noexcept try {
    if (mids.empty()) {
        return proto::code::Database::NoChange;
    }
    if (0u != permission && 2u != permission) {
        return proto::code::Generic::Invalid;
    }
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_master");
    if (!dbc) {
        return proto::code::Database::NoConnection;
    }
    std::string idsSql;
    auto it = mids.begin();
    idsSql += std::to_string(*it);
    ++it;
    for (auto end = mids.end(); it != end; ++it) {
        idsSql += ", " + std::to_string(*it);
    }
    std::string const sql = "UPDATE `IMGroupMember` SET `status` = "
        + std::to_string(permission) + ", `updated` = "
        + std::to_string(::time(nullptr)) + " WHERE `groupId` = "
        + std::to_string(groupId) + " AND `userId` IN ("
        + idsSql + ") AND `status` IN(0, 2) AND"
        " `status` != " + std::to_string(permission) + ";";
    if (!dbc->ExecuteUpdate(sql.c_str())) {
        Debug(__func__ << ": " << sql)
        return proto::code::Database::NoChange;
    }
    return proto::code::Generic::Ok;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::mods(uint32_t groupId, uint32_t mid, uint32_t shield) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_master");
    Assert(dbc, "getdbconn fail: teamtalk_master")
    Assert(0u == shield || 1u == shield, "invalid shield " << shield)
    std::string const sql = "UPDATE `IMGroupMember` SET `shield` = "
        + std::to_string(shield) + ", `updated` = "
        + std::to_string(::time(nullptr)) + " WHERE `groupId` = "
        + std::to_string(groupId) + " AND `userId` = "
        + std::to_string(mid) + " AND `status` IN(0, 2) AND"
        " `shield` != " + std::to_string(shield) + ";";
    if (!dbc->ExecuteUpdate(sql.c_str())) {
        Debug(__func__ << ": " << sql)
        return proto::code::Database::NoChange;
    }
    return proto::code::Generic::Ok;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::modGroupChatTime(uint32_t const groupId, DBCP const &dbc) noexcept try {
    Assert(dbc, "nil dbc")
    uint32_t const now = ::time(nullptr);
    string const sql = "UPDATE `IMGroup` SET `lastChated` = "
        + std::to_string(now)
        + " WHERE `id` = " + std::to_string(groupId);
    if (!dbc->ExecuteUpdate(sql.c_str())) {
        Debug(__func__ << ": " << sql)
        return proto::code::Database::NoChange;
    }
    return proto::code::Generic::Ok;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::validGrpo(uint32_t groupId, uint32_t oid, DBCP const& dbc) noexcept try {
    if (!dbc) {
        return proto::code::Database::NoConnection;
    }
    std::string const sql = "SELECT `id` FROM `IMGroup` WHERE"
        " `id` = " + std::to_string(groupId) + " AND `creator` = "
        + std::to_string(oid) + " AND `status` = 0";
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        return proto::code::Database::QueryError;
    }
    auto const it = qres.begin();
    if (it == qres.end()) {
        return proto::code::Database::NoSuchMember;
    }
    return proto::code::Valid;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::validGrpo(uint32_t groupId, uint32_t oid) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    return this->validGrpo(groupId, oid, dbc);
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::validGroup(uint32_t groupId) noexcept try {
    if (this->isValidateGroupId(groupId)) {
        return proto::code::Generic::Valid;
    }
    auto const dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    Assert(dbc, "getdbconn fail: teamtalk_slave")
    std::string const sql = "SELECT `id` FROM `IMGroup` WHERE `id` = "
        + std::to_string(groupId) + " AND `status` = 0";
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        Debug(__func__ << ": query fail " << sql)
        return proto::code::Database::QueryError;
    }
    auto const it = qres.begin();
    if (it == qres.end()) {
        return proto::code::Database::NoSuchGroup;
    }
    return proto::code::Generic::Valid;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::validGrpm(uint32_t groupId, uint32_t mid) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    Assert(dbc, "getdbconn fail: teamtalk_slave")
    std::string const sql = "SELECT * FROM `IMGroupMember` WHERE `userId`"
        " = " + std::to_string(mid)
        + " AND `groupId` = " + std::to_string(groupId)
        + " AND `status` IN(0, 2) AND (0 != (SELECT COUNT(*) FROM"
        " `IMGroup` WHERE `id` = " + std::to_string(groupId)
        + " AND `status` = 0))";
    Debug(__func__ << ": " << sql)
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        return proto::code::Database::QueryError;
    }
    auto const it = qres.begin();
    if (it == qres.end()) {
        return proto::code::Database::NoSuchMember;
    }
    return proto::code::Valid;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::validGrpa(uint32_t groupId, uint32_t aid, DBCP const& dbc) noexcept try {
    if (!dbc) {
        return proto::code::Database::NoConnection;
    }
    std::string const sql = "SELECT `status` FROM `IMGroupMember` WHERE"
        " `userId` = " + std::to_string(aid)
        + " AND `groupId` = " + std::to_string(groupId)
        + " AND `status` = 2";
    Debug(__func__ << ": " << sql)
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        return proto::code::Database::QueryError;
    }
    auto const it = qres.begin();
    if (it == qres.end()) {
        return proto::code::Database::NoSuchMember;
    }
    return proto::code::Valid;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::validGrpa(uint32_t groupId, uint32_t aid) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    return this->validGrpa(groupId, aid, dbc);
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::validAddp(uint32_t groupId, uint32_t tvid) noexcept try {
#   if UCHAT_HAS_STRICT_ROOT_USER
    if(conf::kRootUid == tvid) {
        Debug(__func__ << ": allow strict root user id " << conf::kRootUid)
        return proto::code::Generic::Valid;
    }
#   endif
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    if (!dbc) {
        return proto::code::Database::NoConnection;
    }
    std::string const sql = "SELECT `creator`, `normal_invite` FROM"
        " `IMGroup`"
        " WHERE `id` = " + std::to_string(groupId) + " AND `status` = 0";
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        return proto::code::Database::QueryError;
    }
    auto const it = qres.begin();
    if (it == qres.end()) {
        return proto::code::Database::NoSuchGroup;
    }
    bool normalInvite = bool(uint32_t(it("normal_invite").as<NumStrView>()));
    if (normalInvite) {
        return proto::code::Generic::Valid;
    }
    int32_t const code = this->validGrpa(groupId, tvid, dbc);
    if (proto::code::Generic::Valid == code) {
        return code;
    }
    uint32_t const cId = uint32_t(it("creator").as<NumStrView>());
    if (cId == tvid) {
        return proto::code::Generic::Valid;
    }
    return proto::code::Generic::PermissionDenied;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::validDelp(uint32_t groupId, uint32_t tvid, std::set<uint32_t> const& tods) noexcept try {
#   if UCHAT_HAS_STRICT_ROOT_USER
    if(conf::kRootUid == tvid) {
        Debug(__func__ << ": allow strict root user id " << conf::kRootUid)
        return proto::code::Generic::Valid;
    }
#   endif
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    if (!dbc) {
        return proto::code::Database::NoConnection;
    }
    bool hasAdmin = false;
    for (uint32_t const& id : tods) {
        auto const code = this->validGrpa(groupId, id, dbc);
        if (proto::code::Generic::Valid == code) {
            hasAdmin = true;
            break;
        } else if (proto::code::Database::NoSuchMember != code) {
            Error0(__func__ << ": validGrpa fail " << code)
            return code;
        }
    }
    if (!hasAdmin) {
        if (proto::code::Generic::Valid == this->validGrpa(groupId, tvid, dbc)) {
            return proto::code::Generic::Valid;
        }
    }
    return this->validGrpo(groupId, tvid, dbc);
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
#if 0
int32_t GroupModel::validModifyPermission(
    uint32_t const groupId,
    uint32_t const tvid,
    IM::BaseDefine::GroupModifyType const& type) noexcept try {
#   if 0
#   if UCHAT_HAS_STRICT_ROOT_USER
    if(conf::kRootUid == userId) {
        Debug(__func__ << ": allow strict root user id "
            << conf::kRootUid)
        return proto::code::Generic::Valid;
    }
#   endif
    auto const dbc = DBManager::getInstance()->getdbconn(
        "teamtalk_slave");
    if (!dbc) {
        return proto::code::Database::NoConnection;
    }
    std::string const sql = "SELECT `creator`, `type` FROM `IMGroup`"
        " WHERE `id` = " + std::to_string(groupId) + " AND `status` = 0";
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        return proto::code::Database::QueryError;
    }
    auto const it = qres.begin();
    if (it == qres.end()) {
        return proto::code::Database::NoSuchGroup;
    }
    uint32_t const creatorId = uint32_t(
        it("creator").as<NumStrView>());
    IM::BaseDefine::GroupType const gtp = IM::BaseDefine::GroupType(
        uint32_t(it("type").as<NumStrView>()));
    if (!IM::BaseDefine::GroupType_IsValid(gtp)) {
        return proto::code::Database::NoSuchGroup;
    }
    if (IM::BaseDefine::GroupType::GROUP_TYPE_TMP == gtp
        && IM::BaseDefine::GROUP_MODIFY_TYPE_ADD == type) {
        return proto::code::Generic::Valid;
    } else if (creatorId == userId) {
        return proto::code::Generic::Valid;
    }
    return proto::code::Generic::PermissionDenied;
#   endif // if 0
    switch(type) {
    case IM::BaseDefine::GroupModifyType::GROUP_MODIFY_TYPE_ADD: {
        return this->validAddp(groupId, tvid);
    } break;
    case IM::BaseDefine::GroupModifyType::GROUP_MODIFY_TYPE_DEL: {
        return this->validDelp(groupId, tvid);
    } break;
    case IM::BaseDefine::GroupModifyType::GROUP_MODIFY_PERM_NORMAL:
    case IM::BaseDefine::GroupModifyType::GROUP_MODIFY_PERM_ADMIN:
    case IM::BaseDefine::GroupModifyType::GROUP_MODIFY_PK10_RULE: {
#       if UCHAT_HAS_STRICT_ROOT_USER
        if(conf::kRootUid == tvid) {
            Debug(__func__ << ": allow strict root user id "
                << conf::kRootUid)
            return proto::code::Generic::Valid;
        }
#       endif
        return this->validGrpo(groupId, tvid);
    } break;
    default:
        Warning(__func__ << ": unhandled GroupModifyType " << uint32_t(type))
        return proto::code::Generic::Invalid;
    }
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
#endif
int32_t GroupModel::validProxyAutoAdd(uint32_t groupId) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    if (!dbc) {
        return proto::code::Database::NoConnection;
    }
    std::string const sql = "SELECT `proxy_auto_add` FROM `IMGroup` WHERE"
        " `id` = " + std::to_string(groupId) + " AND `status` = 0";
    Debug(__func__ << ": " << sql)
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        return proto::code::Database::QueryError;
    }
    auto const it = qres.begin();
    if (it == qres.end()) {
        return proto::code::Database::NoSuchGroup;
    }
    auto paa = uint32_t(it("proxy_auto_add").as<NumStrView>());
    if (paa) {
        return proto::code::Valid;
    } else {
        return proto::code::Invalid;
    }
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::fetUGrpids(uint32_t userId, std::set<uint32_t>& gs, uint32_t li) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    if (!dbc) {
        Error0("no db connection for teamtalk_slave")
        return proto::code::Database::NoConnection;
    }
    std::string sql;
    if (li > 0) {
        sql = "SELECT `groupId` FROM `IMGroupMember` WHERE `userId` = "
            + std::to_string(userId) + " AND `status` IN (0, 2) ORDER BY"
            " `updated` DESC, `id` LIMIT " + std::to_string(li);
    } else {
        sql = "SELECT `groupId` FROM `IMGroupMember` WHERE `userId` = "
            + std::to_string(userId) + " AND `status` IN (0, 2) ORDER BY"
            " `updated` DESC, `id`;";
    }
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        Debug(__func__ << ": executeQuery error sql " << sql)
        return proto::code::Database::QueryError;
    }
    if (qres.isEmpty()) {
        return proto::code::Database::NoSuchGroup;
    }
    gs.clear();
    for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
        gs.insert(uint32_t(it("groupId").as<NumStrView>()));
    }
    return proto::code::Generic::Ok;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::fetOGrpids(uint32_t oid, std::set<uint32_t>& gs, uint32_t li) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    if (!dbc) {
        return proto::code::Database::NoConnection;
    }
    std::string sql;
    if (li > 0) {
        sql = "SELECT `id` FROM `IMGroup` WHERE `creator` = "
            + std::to_string(oid) + " AND `status` = 0 ORDER BY"
            " `updated` DESC, `id` LIMIT " + std::to_string(li);
    } else {
        sql = "SELECT `id` FROM `IMGroup` WHERE `creator` = "
            + std::to_string(oid) + " AND `status` = 0 ORDER BY"
            " `updated` DESC, `id`;";
    }
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        Debug(__func__ << ": executeQuery error sql " << sql)
        return proto::code::Database::QueryError;
    }
    if (qres.isEmpty()) {
        return proto::code::Database::NoSuchGroup;
    }
    gs.clear();
    for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
        if (auto const v = it("id").as<NumStrView>()) {
            gs.insert(uint32_t(v));
        }
    }
    return proto::code::Generic::Ok;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::fetUBGrpinfo(uint32_t userId, std::vector<GroupVersionInfo>& groups, uint32_t gtp) noexcept try {
    std::set<uint32_t> gs;
    int32_t code = this->fetUGrpids(userId, gs, 0);
    if (proto::code::Generic::Ok != code) {
        Error0(__func__ << ": fetUGrpids fail " << code)
        return code;
    }
    if (gs.empty()) {
        groups.clear();
        groups.shrink_to_fit();
        code = proto::code::Generic::Ok;
    } else {
        code = this->fetBGrpinfo(gs, groups, gtp);
    }
    return code;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::fetBGrpinfo(std::set<uint32_t> const& gs, std::vector<GroupVersionInfo>& groups, uint32_t gtp) noexcept try {
    if (gs.empty()) {
        Note(__func__ << ": no group ids")
        groups.clear();
        groups.shrink_to_fit();
        return proto::code::Generic::Ok;
    }
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    if (!dbc) {
        Error0(__func__ << ": getdbconn fail: teamtalk_slave")
        return proto::code::Database::NoConnection;
    }
    auto it = gs.cbegin();
    std::string idsSql = std::to_string(*it);
    ++it;
    for (auto end = gs.cend(); it != end; ++it) {
        idsSql += ", " + std::to_string(*it);
    }
    std::string sql = "SELECT `id`, `version` FROM `IMGroup` WHERE"
        " `id` IN (" + idsSql + ")";
    if (0 != gtp) {
        sql += " AND `type` = " + std::to_string(gtp);
    }
    sql += " ORDER BY `updated` DESC";
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        Error0("executeQuery fail " << sql)
        return proto::code::Database::ExecuteError;
    }
    groups.clear();
    for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
        GroupVersionInfo group;
        if (auto const v = it("id").as<NumStrView>()) {
            group.set_group_id(uint32_t(v));
        } else {
            Warning(__func__ << ": skip nil id")
            continue;
        }
        if (auto const v = it("version").as<NumStrView>()) {
            group.set_version(uint32_t(v));
        } else {
            Warning(__func__ << ": skip nil version")
            continue;
        }
        groups.emplace_back(std::move(group));
    }
    groups.shrink_to_fit();
    return proto::code::Generic::Ok;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
//int32_t GroupModel::fetDGrpinfo(GroupIds const& gs, std::vector<GroupInfo>& result) noexcept try {
//    Assert(!gs.empty(), "no group id")
//    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
//    Assert(dbc, "getdbconn fail")
//    std::string sql;
//    {
//        auto it = gs.cbegin();
//        std::string idsSql = std::to_string(it->first);
//        ++it;
//        for (auto end = gs.cend(); it != end; ++it) {
//            idsSql += ", " + std::to_string(it->first);
//        }
//        sql = "SELECT * FROM `IMGroup` WHERE"
//            " `id` IN (" + idsSql  + ")"
//            " AND `status` = 0"
//            " ORDER BY `updated` DESC LIMIT "
//            + std::to_string(kMaxGroupQueryLimit);
//        Info0("gs size " << gs.size() << " sql " << sql)
//    }
//    DBQueryResult qres = dbc->executeQuery(sql);
//    Assert(qres, "executeQuery fail")
//    result.clear();
//    GroupInfo gin;
//    for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
//        uint32_t queryGroupId = uint32_t(it("id").as<NumStrView>());
//        auto const g = gs.find(queryGroupId);
//        Assert(gs.cend() != g, "cannot find " << queryGroupId)
//        uint32_t queryVersion = uint32_t(it("version").as<NumStrView>());
//        if (g->second.version() < queryVersion) {
//            uint32_t const gtp = uint32_t(it("type").as<NumStrView>());
//            if (!GroupType_IsValid(gtp)) {
//                Warning("skip type invalid group")
//                continue;
//            }
//            gin.set_group_id(queryGroupId);
//            gin.set_version(queryVersion);
//            gin.set_group_name(std::string(it("name").as<StringView>()));
//            gin.set_group_avatar(std::string(it("avatar").as<StringView>()));
//            gin.set_group_creator_id(uint32_t(it("creator").as<NumStrView>()));
//            gin.set_group_type(static_cast<GroupType>(gtp));
//            //gin.set_shield_status(uint32_t(it("status").as<NumStrView>()));
//            gin.set_shield_status(0);
//            if (auto const v = it("pk10rules").as<StringView>()) {
//                gin.set_pk10_rules(std::string(v));
//            }
//            if (auto const v = it("announcement").as<StringView>()) {
//                gin.set_announcement(std::string(v));
//            } else {
//                gin.set_announcement("");
//            }
//            result.emplace_back(std::move(gin));
//        }
//    }
//    result.shrink_to_fit();
//    // FIXED: use captured dbc or release first
//    return this->fetMs(result, dbc);
//} catch(std::exception const& e) {
//    Error0(e.what())
//    return proto::code::Generic::Exception;
//}
int32_t GroupModel::fetDGrpinfo(uint32_t mid, GroupIds const& gs, std::vector<IM::BaseDefine::GroupInfo>& result) noexcept try {
    if (gs.empty()) {
        Error0("no group id")
        return proto::code::Generic::Invalid;
    }
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    if (!dbc) {
        Error0("getdbconn fail teamtalk_slave")
        return proto::code::Database::NoConnection;
    }
    std::string sql;
    {
        auto it = gs.cbegin();
        std::string idsSql = std::to_string(it->first);
        ++it;
        for (auto end = gs.cend(); it != end; ++it) {
            idsSql += ", " + std::to_string(it->first);
        }
        // To query group basic info and group message shield for request
        // group member
        sql =
            "SELECT `IMGroup`.*, `IMGroupMember`.`shield`"
            " FROM `IMGroup`"
            "  LEFT JOIN `IMGroupMember` ON ("
            "    `IMGroupMember`.`groupId` = `IMGroup`.`id`"
            "    AND `IMGroupMember`.`status` IN(0, 2)"
            "    AND `IMGroupMember`.`userId` = " + std::to_string(mid) +
            "    AND `IMGroupMember`.`groupId` IN(" + idsSql + "))"
            "  WHERE (`IMGroup`.`status` = 0 AND `IMGroup`.`id` IN ("
            + idsSql + ") ) ORDER by `IMGroup`.`updated`"
            "  DESC LIMIT " + std::to_string(kMaxGroupQueryLimit);
        Info0(__func__ << ": gs size " << gs.size() << " sql "
            << sql)
    }
    DBQueryResult const qres = dbc->executeQuery(sql);
    if (!qres) {
        Error0(__func__ << ": executeQuery fail " << sql)
        return proto::code::Database::QueryError;
    }
    result.clear();
    GroupInfo gin;
    for (auto it = qres.begin(), end = qres.end(); it != end;
        ++it) {
        uint32_t const queryGroupId = uint32_t(it("id").as<NumStrView>());
        auto const g = gs.find(queryGroupId);
        if (gs.cend() == g) {
            Warning(__func__ << ": cannot find gid " << queryGroupId)
            continue;
        }
        uint32_t const queryVersion = uint32_t(it("version").as<NumStrView>());
        if (g->second.version() < queryVersion) {
            uint32_t const gtp = uint32_t(it("type").as<NumStrView>());
            if (!GroupType_IsValid(gtp)) {
                Warning("skip type invalid group")
                continue;
            }
            gin.set_group_id(queryGroupId);
            gin.set_version(queryVersion);
            gin.set_group_name(std::string(it("name").as<StringView>()));
            gin.set_group_avatar(std::string(it("avatar").as<StringView>()));
            gin.set_group_creator_id(uint32_t(it("creator").as<NumStrView>()));
            gin.mutable_owner()->set_id(gin.group_creator_id());
            gin.set_group_type(static_cast<GroupType>(gtp));
            gin.set_shield_status(uint32_t(it("shield").as<NumStrView>()));
            if (auto const v = it("pk10rules").as<StringView>()) {
                gin.set_pk10_rules(std::string(v));
            }
            if (auto const v = it("announcement").as<StringView>()) {
                gin.set_announcement(std::string(v));
            } else {
                gin.set_announcement("");
            }
            if (auto const v = it("proxy_auto_add").as<NumStrView>()) {
                gin.set_proxy_auto_add(bool(uint32_t(v)));
            } else {
                gin.set_proxy_auto_add(true);
            }
            if (auto const v = it("normal_invite").as<NumStrView>()) {
                gin.set_normal_invite(bool(uint32_t(v)));
            } else {
                gin.set_normal_invite(true);
            }
            result.emplace_back(std::move(gin));
        } else {
            Warning(__func__ << ": invalid version: no data " << g->second.version())
        }
    }
    result.shrink_to_fit();
    // FIXED: use captured dbc or release first
    return this->fetMs(mid, result, dbc);
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::fetMemids(uint32_t groupId, std::set<uint32_t>& result, DBCP const& dbc) noexcept try {
    if (!dbc) {
        return proto::code::Database::NoConnection;
    }
    std::string const sql = "SELECT `userId` FROM `IMGroupMember` WHERE"
        " `groupId` = " + std::to_string(groupId) + " AND `status` IN(0, 2)"
        " LIMIT " + std::to_string(kMaxUserQueryLimit);
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        return proto::code::Database::QueryError;
    }
    result.clear();
    for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
        result.insert(uint32_t(it("userId").as<NumStrView>()));
    }
    return proto::code::Generic::Ok;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::fetMemids(uint32_t groupId, std::set<uint32_t>& result) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    Assert(dbc, "getdbconn fail: teamtalk_slave")
    return this->fetMemids(groupId, result, dbc);
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
//int32_t GroupModel::fetMs(std::vector<GroupInfo>& gs) noexcept try {
//    if (gs.empty()) {
//        Debug(__func__ << ": no group ids")
//        return proto::code::Generic::Ok;
//    }
//#   if 1
//    // Get old userIds
//    std::set<uint32_t> userIds;
//    for (auto& g : gs) {
//        uint32_t const groupId = g.group_id();
//        userIds.clear();
//        auto const code = this->fetMemids(groupId, userIds);
//        if (proto::code::Generic::Ok == code) {
//            for (auto const& userId : userIds) {
//                g.add_group_member_list(userId);
//            }
//        } else {
//            Error0(__func__ << ": fetchGroupMemberIds fail " << code)
//        }
//    }
//#   endif
//    for (auto& g : gs) {
//        uint32_t const groupId = g.group_id();
//        auto const code = this->fetMs(groupId, g.owner().id(), g);
//        if (proto::code::Generic::Ok != code) {
//            Error0(__func__ << ": fetchGroupMembers fail " << code)
//            return code;
//        }
//    }
//    return proto::code::Generic::Ok;
//} catch(std::exception const& e) {
//    Error0(e.what())
//    return proto::code::Generic::Exception;
//}
int32_t GroupModel::fetMs(uint32_t const reqUid, std::vector<GroupInfo>& gs, DBCP const& dbc) noexcept try {
    if (gs.empty()) {
        Debug(__func__ << ": no group ids")
        return proto::code::Generic::Ok;
    }
#   if 1
    // Get old userIds
    std::set<uint32_t> userIds;
    for (auto& g : gs) {
        uint32_t const groupId = g.group_id();
        userIds.clear();
        auto const code = this->fetMemids(groupId, userIds, dbc);
        if (proto::code::Generic::Ok == code) {
            for (auto const& userId : userIds) {
                g.add_group_member_list(userId);
            }
        } else {
            Error0(__func__ << ": fetchGroupMemberIds fail " << code)
        }
    }
#   endif
    for (auto& g : gs) {
        uint32_t const groupId = g.group_id();
        auto const code = this->fetMs(reqUid, groupId, g.owner().id(), g, dbc);
        if (proto::code::Generic::Ok != code) {
            Error0(__func__ << ": fetchGroupMembers fail " << code)
            return code;
        }
    }
    return proto::code::Generic::Ok;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
//int32_t GroupModel::fetMs(uint32_t groupId, uint32_t groupOwner, GroupInfo& gi) noexcept try {
//    auto const dbc = CDBManager::getInstance()->getdbconn("teamtalk_slave");
//    return this->fetMs(groupId, groupOwner, gi, dbc);
//} catch(std::exception const& e) {
//    Error0(e.what())
//    return proto::code::Generic::Exception;
//}
int32_t GroupModel::fetMs(uint32_t const reqUid, uint32_t groupId, uint32_t groupOwner, GroupInfo& gi, DBCP const& dbc) noexcept try {
    if (!dbc) {
        Error0(__func__ << ": nil dbc")
        return proto::code::Database::NoConnection;
    }
    std::string const sql = "SELECT `IMGroupMember`.`userId`,"
        " `IMGroupMember`.`status`, `IMGroupMember`.`nick`,"
        " `IMGroupMember`.`shield`, "
        " `IMUser`.`avatar` FROM `IMGroupMember` LEFT JOIN `IMUser`"
        " ON (`IMGroupMember`.`userId` = `IMUser`.`id`"
        "   )"
        " WHERE `groupId` = " + std::to_string(groupId)
        + " AND `IMGroupMember`.`status`IN (0, 2)";
    Debug(__func__ << ": sql: " << sql)
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        Error0(__func__ << ": executeQuery fail: " << sql)
        return proto::code::Database::QueryError;
    }
    for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
        auto const id = uint32_t(it("userId").as<NumStrView>());
        auto const st = uint32_t(it("status").as<NumStrView>());
        if (reqUid == id) {
            gi.set_shield_status(uint32_t(it("shield").as<NumStrView>()));
        }
        GroupMember* p;
        if (2u == st) {
            // Admin
            p = gi.add_admin();
        } else {
            // Normal and creator
            if (id != groupOwner) {
                p = gi.add_member();
            } else {
                p = gi.mutable_owner();
            }
        }
        p->set_id(id);
        p->set_nick(std::string(it("nick").as<boost::StringView>()));
        if (auto const v = it("avatar").as<boost::StringView>()) {
            p->set_avatar(std::string(v));
        } else {
            p->set_avatar("");
        }
        Debug(__func__ << ": id " << id << " st " << st << " owner " << groupOwner << " avatar " << p->avatar())
    }
    return proto::code::Generic::Ok;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
int32_t GroupModel::fetchS(uint32_t groupId, uint32_t mid, uint32_t& shield) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    if (!dbc) {
        Error0("getdbconn fail: teamtalk_slave")
        return proto::code::Database::NoConnection;
    }
    std::string const sql = "SELECT `shield` FROM `IMGroupMember` WHERE"
        " `groupId` = " + std::to_string(groupId) + " AND `userId` = "
        + std::to_string(mid) + " AND `status` IN(0, 2);";
    auto const qres = dbc->executeQuery(sql);
    if (!qres) {
        Error0(__func__ << ": executeQuery fail " << sql)
        return proto::code::Database::QueryError;
    }
    auto const it = qres.begin();
    if (it == qres.end()) {
        return proto::code::Database::NoSuchMember;
    }
    shield = uint32_t(it("shield").as<NumStrView>());
    return proto::code::Generic::Ok;
} catch(std::exception const& e) {
    Error0(e.what())
    return proto::code::Generic::Exception;
}
/**
 *  创建群 DEPRECATED PLEASE USE newgrp()
 *  @sa newgrp()
 *
 *  @param nUserId        创建者
 *  @param strGroupName   群名
 *  @param strGroupAvatar 群头像
 *  @param nGroupType     群类型1,固定群;2,临时群
 *  @param setMember      群成员列表，为了踢出重复的userId，使用set存储
 *
 *  @return 成功返回群Id，失败返回0;
 */
uint32_t CGroupModel::createGroup(uint32_t nUserId, const string& strGroupName, const string& strGroupAvatar, uint32_t nGroupType, set<uint32_t>& setMember)
{
    uint32_t nGroupId = INVALID_VALUE;
    do {
        if(strGroupName.empty()) {
            break;
        }
        if (setMember.empty()) {
            break;
        }
        // remove repeat user
        
        
        //insert IMGroup
        if(!insertNewGroup(nUserId, strGroupName, strGroupAvatar, nGroupType, (uint32_t)setMember.size(), nGroupId)) {
            break;
        }
        bool bRet = CGroupMessageModel::getInstance()->resetMsgId(nGroupId);
        if(!bRet)
        {
            //log("reset msgId failed. groupId=%u", nGroupId);
        }
        
        //insert IMGroupMember
        clearGroupMember(nGroupId);
        insertNewMember(nGroupId, setMember);
        
    } while (false);
    
    return nGroupId;
}

bool CGroupModel::removeGroup(uint32_t nUserId, uint32_t nGroupId, list<uint32_t>& lsCurUserId)
{
    bool bRet = false;
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master");
    set<uint32_t> setGroupUsers;
    if(pDBConn)
    {
        string strSql = "select creator from IMGroup where id="+int2string(nGroupId);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            uint32_t nCreator;
            while (pResultSet->Next()) {
                nCreator = pResultSet->GetInt("creator");
            }
            
            if(0 == nCreator || nCreator == nUserId)
            {
                //设置群组不可用。
                strSql = "update IMGroup set status=0 where id="+int2string(nGroupId);
                bRet = pDBConn->ExecuteUpdate(strSql.c_str());
            }
            delete  pResultSet;
        }
        
        if (bRet) {
            strSql = "select userId from IMGroupMember where groupId="+int2string(nGroupId);
            CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
            if(pResultSet)
            {
                while (pResultSet->Next()) {
                    uint32_t nId = pResultSet->GetInt("userId");
                    setGroupUsers.insert(nId);
                }
                delete pResultSet;
            }
        }
    }
    
    if(bRet)
    {
        bRet = delm(nGroupId, setGroupUsers, lsCurUserId);
    }
    
    return bRet;
}
void CGroupModel::getGroupInfo(map<uint32_t,IM::BaseDefine::GroupVersionInfo>& mapGroupId, list<IM::BaseDefine::GroupInfo>& lsGroupInfo)
{ 
   if (!mapGroupId.empty())
    {
        auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
        if (pDBConn)
        {
            string strClause;
            bool bFirst = true;
            for(auto it=mapGroupId.begin(); it!=mapGroupId.end(); ++it)
            {
                if(bFirst)
                {
                    bFirst = false;
                    strClause = int2string(it->first);
                }
                else
                {
                    strClause += ("," + int2string(it->first));
                }
            }
            string strSql = "select * from IMGroup where id in (" + strClause  + ") order by updated desc";
            CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
            if(pResultSet)
            {
                while (pResultSet->Next()) {
                    uint32_t nGroupId = pResultSet->GetInt("id");
                    uint32_t nVersion = pResultSet->GetInt("version");
               if(mapGroupId[nGroupId].version() < nVersion)
                    {
                        /// @bug below => DEPRECATED => use fetchGroupInfo
                        IM::BaseDefine::GroupInfo cGroupInfo;
                        cGroupInfo.set_group_id(nGroupId);
                        cGroupInfo.set_version(nVersion);
                        cGroupInfo.set_group_name(pResultSet->GetString("name"));
                        cGroupInfo.set_group_avatar(pResultSet->GetString("avatar"));
                        cGroupInfo.set_pk10_rules(pResultSet->GetString("pk10rules"));
                        IM::BaseDefine::GroupType nGroupType = IM::BaseDefine::GroupType(pResultSet->GetInt("type"));
                        if(IM::BaseDefine::GroupType_IsValid(nGroupType))
                        {
                            cGroupInfo.set_group_type(nGroupType);
                            cGroupInfo.set_group_creator_id(pResultSet->GetInt("creator"));
                            lsGroupInfo.push_back(cGroupInfo);
                        }
                        else
                        {
                            //log("invalid gtp. groupId=%u, gtp=%u", nGroupId, nGroupType);
                        }
                    }
                }
                delete pResultSet;
            }
            else
            {
                //log("no result set for sql:%s", strSql.c_str());
            }
       if(!lsGroupInfo.empty())
            {
                fillGroupMember(lsGroupInfo);
            }
        }
        else
        {
            //log("no db connection for teamtalk_slave");
        }
    }
    else
    {
        //log("no ids in map");
    }
}

bool CGroupModel::modifyGroupMember(uint32_t nUserId, uint32_t nGroupId, IM::BaseDefine::GroupModifyType nType, set<uint32_t>& setUserId, list<uint32_t>& lsCurUserId)
{
    bool bRet = false;
    if(hasModifyPermission(nUserId, nGroupId, nType))
    {
        switch (nType) {
            case IM::BaseDefine::GROUP_MODIFY_TYPE_ADD:
                bRet = addmem(nGroupId, setUserId, lsCurUserId);
                break;
            case IM::BaseDefine::GROUP_MODIFY_TYPE_DEL:
                bRet = delm(nGroupId, setUserId, lsCurUserId);
                removeSession(nGroupId, setUserId);
                break;
            default:
                //log("unknown type:%u while modify group.%u->%u", nType, nUserId, nGroupId);
                break;
        }
        //if modify group member success, need to inc the group version and clear the user count;
        if(bRet)
        {
            incGroupVersion(nGroupId);
            for (auto it=setUserId.begin(); it!=setUserId.end(); ++it) {
                uint32_t nUserId=*it;
                CUserModel::getInstance()->clearUserCounter(nUserId, nGroupId, IM::BaseDefine::SESSION_TYPE_GROUP);
            }
        }
    }
    else
    {
        //log("user:%u has no permission to modify group:%u", nUserId, nGroupId);
    }    return bRet;
}
bool CGroupModel::insertNewGroup(uint32_t nUserId, const string& strGroupName, const string& strGroupAvatar, uint32_t nGroupType, uint32_t nMemberCnt, uint32_t& nGroupId)
{
    bool bRet = false;
    nGroupId = INVALID_VALUE;
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master");
    if (pDBConn)
    {
        string strSql = "insert into IMGroup(`name`, `avatar`, `creator`, `type`,`userCnt`, `status`, `version`, `lastChated`, `updated`, `created`) "\
        "values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        
        CPrepareStatement* pStmt = new CPrepareStatement();
        if (pStmt->Init(pDBConn->GetMysql(), strSql))
        {
            uint32_t nCreated = (uint32_t)time(NULL);
            uint32_t index = 0;
            uint32_t nStatus = 0;
            uint32_t nVersion = 1;
            uint32_t nLastChat = 0;
            pStmt->SetParam(index++, strGroupName);
            pStmt->SetParam(index++, strGroupAvatar);
            pStmt->SetParam(index++, nUserId);
            pStmt->SetParam(index++, nGroupType);
            pStmt->SetParam(index++, nMemberCnt);
            pStmt->SetParam(index++, nStatus);
            pStmt->SetParam(index++, nVersion);
            pStmt->SetParam(index++, nLastChat);
            pStmt->SetParam(index++, nCreated);
            pStmt->SetParam(index++, nCreated);
            
            bRet = pStmt->ExecuteUpdate();
            if(bRet) {
                nGroupId = pStmt->GetInsertId();
            }
        }
        delete pStmt;
    }
    else
    {
        //log("no db connection for teamtalk_master");
    }
    return bRet;
}

bool CGroupModel::insertNewMember(uint32_t nGroupId, set<uint32_t>& setUsers)
{
    bool bRet = false;
    uint32_t nUserCnt = (uint32_t)setUsers.size();
    if(nGroupId != INVALID_VALUE &&  nUserCnt > 0)
    {
        auto pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
        if (pDBConn)
        {
            uint32_t nCreated = (uint32_t)time(NULL);
            // 获取 已经存在群里的用户
            string strClause;
            bool bFirst = true;
            for (auto it=setUsers.begin(); it!=setUsers.end(); ++it)
            {
                if(bFirst)
                {
                    bFirst = false;
                    strClause = int2string(*it);
                }
                else
                {
                    strClause += ("," + int2string(*it));
                }
            }
            string strSql = "select userId from IMGroupMember where groupId=" + int2string(nGroupId) + " and userId in (" + strClause + ")";
            CResultSet* pResult = pDBConn->ExecuteQuery(strSql.c_str());
            set<uint32_t> setHasUser;
            if(pResult)
            {
                while (pResult->Next()) {
                    setHasUser.insert(pResult->GetInt("userId"));
                }
                delete pResult;
            }
            else
            {
                //log("no result for sql:%s", strSql.c_str());
            }
            pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master");
            if (pDBConn)
            {
                CacheManager* pCacheManager = CacheManager::getInstance();
                CacheConn* pCacheConn = pCacheManager->GetCacheConn("group_member");
                if (pCacheConn)
                {
                    // 设置已经存在群中人的状态
                    if (!setHasUser.empty())
                    {
                        strClause.clear();
                        bFirst = true;
                        for (auto it=setHasUser.begin(); it!=setHasUser.end(); ++it) {
                            if(bFirst)
                            {
                                bFirst = false;
                                strClause = int2string(*it);
                            }
                            else
                            {
                                strClause += ("," + int2string(*it));
                            }
                        }
                        
                        strSql = "update IMGroupMember set status=0, updated="+int2string(nCreated)+" where groupId=" + int2string(nGroupId) + " and userId in (" + strClause + ")";
                        pDBConn->ExecuteUpdate(strSql.c_str());
                    }
                    strSql = "insert into IMGroupMember(`groupId`, `userId`, `status`, `created`, `updated`) values\
                    (?,?,?,?,?)";
                    
                    //插入新成员
                    auto it = setUsers.begin();
                    uint32_t nStatus = 0;
                    uint32_t nIncMemberCnt = 0;
                    for (;it != setUsers.end();)
                    {
                        uint32_t nUserId = *it;
                        if(setHasUser.find(nUserId) == setHasUser.end())
                        {
                            CPrepareStatement* pStmt = new CPrepareStatement();
                            if (pStmt->Init(pDBConn->GetMysql(), strSql))
                            {
                                uint32_t index = 0;
                                pStmt->SetParam(index++, nGroupId);
                                pStmt->SetParam(index++, nUserId);
                                pStmt->SetParam(index++, nStatus);
                                pStmt->SetParam(index++, nCreated);
                                pStmt->SetParam(index++, nCreated);
                                pStmt->ExecuteUpdate();
                                ++nIncMemberCnt;
                                delete pStmt;
                            }
                            else
                            {
                                setUsers.erase(it++);
                                delete pStmt;
                                continue;
                            }
                        }
                        ++it;
                    }
                    if(nIncMemberCnt != 0)
                    {
                        strSql = "update IMGroup set userCnt=userCnt+" + int2string(nIncMemberCnt) + " where id="+int2string(nGroupId);
                        pDBConn->ExecuteUpdate(strSql.c_str());
                    }
                    
                    //更新一份到redis中
                    string strKey = "group_member_"+int2string(nGroupId);
                    for(auto it = setUsers.begin(); it!=setUsers.end(); ++it)
                    {
                        pCacheConn->hset(strKey, int2string(*it), int2string(nCreated));
                    }
                    pCacheManager->RelCacheConn(pCacheConn);
                    bRet = true;
                }
                else
                {
                    //log("no cache connection");
                }
            }
            else
            {
                //log("no db connection for teamtalk_master");
            }
        }
        else
        {
            //log("no db connection for teamtalk_slave");
        }
    }
    return bRet;
}

void CGroupModel::getGroupByCreator(uint32_t nCreator, uint32_t nGroupType, list<uint32_t>& lsGroup)
{
    if (auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave")) {
        string strSql = "select * from IMGroup where creator=" + int2string(nCreator) + " and type=" + int2string(nGroupType);
         
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while(pResultSet->Next())
            {
                uint32_t nGroupId = pResultSet->GetInt("id");
                lsGroup.push_back(nGroupId);
            }
            delete pResultSet;
        }
        else
        {
            //log("no result set for sql:%s", strSql.c_str());
        }
    }
    else
    {
        //log("no db connection for teamtalk_slave");
    }
}

bool CGroupModel::isInGroup(uint32_t nUserId, uint32_t nGroupId)
{
    bool bRet = false;
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("group_member");
    if (pCacheConn)
    {
        string strKey = "group_member_" + int2string(nGroupId);
        string strField = int2string(nUserId);
        string strValue = pCacheConn->hget(strKey, strField);
        pCacheManager->RelCacheConn(pCacheConn);
        if(!strValue.empty())
        {
            bRet = true;
        }
    }
    else
    {
        //log("no cache connection for group_member");
    }
    return bRet;
}

bool CGroupModel::hasModifyPermission(uint32_t nUserId, uint32_t nGroupId, IM::BaseDefine::GroupModifyType nType)
{
    if(nUserId == 0) {
        return true;
    }
    
    bool bRet = false;
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if(pDBConn)
    {
        string strSql = "select creator, type from IMGroup where id="+ int2string(nGroupId);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next())
            {
                uint32_t nCreator = pResultSet->GetInt("creator");
                IM::BaseDefine::GroupType nGroupType = IM::BaseDefine::GroupType(pResultSet->GetInt("type"));
                if(IM::BaseDefine::GroupType_IsValid(nGroupType))
                {
                    if(IM::BaseDefine::GROUP_TYPE_TMP == nGroupType && IM::BaseDefine::GROUP_MODIFY_TYPE_ADD == nType)
                    {
                        bRet = true;
                        break;
                    }
                    else
                    {
                        if(nCreator == nUserId)
                        {
                            bRet = true;
                            break;
                        }
                    }
                }
            }
            delete pResultSet;
        }
        else
        {
            //log("no result for sql:%s", strSql.c_str());
        }
    }
    else
    {
        //log("no db connection for teamtalk_slave");
    }
    return bRet;
}

bool CGroupModel::addmem(uint32_t nGroupId, set<uint32_t> &setUser, list<uint32_t>& lsCurUserId)
{
    // 去掉已经存在的用户ID
    removeRepeatUser(nGroupId, setUser);
    bool bRet = insertNewMember(nGroupId, setUser);
    getGroupUser(nGroupId,lsCurUserId);
    return bRet;
}

bool CGroupModel::delm(uint32_t nGroupId, set<uint32_t> &setUser, list<uint32_t>& lsCurUserId)
{
    if(setUser.size() <= 0)
    {
        return true;
    }
    bool bRet = false;
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master");
    if(pDBConn)
    {
        CacheManager* pCacheManager = CacheManager::getInstance();
        CacheConn* pCacheConn = pCacheManager->GetCacheConn("group_member");
        if (pCacheConn)
        {
            string strClause ;
            bool bFirst = true;
            for(auto it= setUser.begin(); it!=setUser.end();++it)
            {
                if (bFirst) {
                    bFirst = false;
                    strClause = int2string(*it);
                }
                else
                {
                    strClause += ("," + int2string(*it));
                }
            }
            string strSql = "update IMGroupMember set status=1 where  groupId =" + int2string(nGroupId) + " and userId in(" + strClause + ")";
            pDBConn->ExecuteUpdate(strSql.c_str());
            
            //从redis中删除成员
            string strKey = "group_member_"+ int2string(nGroupId);
            for (auto it=setUser.begin(); it!=setUser.end(); ++it) {
                string strField = int2string(*it);
                pCacheConn->hdel(strKey, strField);
            }
            pCacheManager->RelCacheConn(pCacheConn);
            bRet = true;
        }
        else
        {
            //log("no cache connection");
        }
        if (bRet)
        {
            getGroupUser(nGroupId,lsCurUserId);
        }
    }
    else
    {
        //log("no db connection for teamtalk_master");
    }
    return bRet;
}

void CGroupModel::removeRepeatUser(uint32_t nGroupId, set<uint32_t> &setUser)
{
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("group_member");
    if (pCacheConn)
    {
        string strKey = "group_member_"+int2string(nGroupId);
        for (auto it=setUser.begin(); it!=setUser.end();) {
            string strField = int2string(*it);
            string strValue = pCacheConn->hget(strKey, strField);
            pCacheManager->RelCacheConn(pCacheConn);
            if(!strValue.empty())
            {
                setUser.erase(it++);
            }
            else
            {
                ++it;
            }
        }
    }
    else
    {
        //log("no cache connection for group_member");
    }
}

bool CGroupModel::setPush(uint32_t nUserId, uint32_t nGroupId, uint32_t nType, uint32_t nStatus)
{
    bool bRet = false;
    if(!isInGroup(nUserId, nGroupId))
    {
        //log("user:%d is not in group:%d", nUserId, nGroupId);
        return bRet;;
    }
    
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("group_set");
    if(pCacheConn)
    {
        string strGroupKey = "group_set_" + int2string(nGroupId);
        string strField = int2string(nUserId) + "_" + int2string(nType);
        int nRet = pCacheConn->hset(strGroupKey, strField, int2string(nStatus));
        pCacheManager->RelCacheConn(pCacheConn);
        if(nRet != -1)
        {
            bRet = true;
        }
    }
    else
    {
        //log("no cache connection for group_set");
    }
    return bRet;
}

void CGroupModel::getPush(uint32_t nGroupId, list<uint32_t>& lsUser, list<IM::BaseDefine::ShieldStatus>& lsPush)
{
    if (lsUser.empty()) {
        return;
    }
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("group_set");
    if(pCacheConn)
    {
        string strGroupKey = "group_set_" + int2string(nGroupId);
        map<string, string> mapResult;
        bool bRet = pCacheConn->hgetAll(strGroupKey, mapResult);
        pCacheManager->RelCacheConn(pCacheConn);
        if(bRet)
        {
            for(auto it=lsUser.begin(); it!=lsUser.end(); ++it)
            {
                string strField = int2string(*it) + "_" + int2string(IM_GROUP_SETTING_PUSH);
                auto itResult = mapResult.find(strField);
                IM::BaseDefine::ShieldStatus status;
                status.set_group_id(nGroupId);
                status.set_user_id(*it);
                if(itResult != mapResult.end())
                {
                    status.set_shield_status(string2int(itResult->second));
                }
                else
                {
                    status.set_shield_status(0);
                }
                lsPush.push_back(status);
            }
        }
        else
        {
            //log("hgetall %s failed!", strGroupKey.c_str());
        }
    }
    else
    {
        //log("no cache connection for group_set");
    }
}

void CGroupModel::getGroupUser(uint32_t nGroupId, list<uint32_t> &lsUserId)
{
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("group_member");
    if (pCacheConn)
    {
        string strKey = "group_member_" + int2string(nGroupId);
        map<string, string> mapAllUser;
        bool bRet = pCacheConn->hgetAll(strKey, mapAllUser);
        pCacheManager->RelCacheConn(pCacheConn);
        if(bRet)
        {
            for (auto it=mapAllUser.begin(); it!=mapAllUser.end(); ++it) {
                uint32_t nUserId = string2int(it->first);
                lsUserId.push_back(nUserId);
            }
        }
        else
        {
            //log("hgetall %s failed!", strKey.c_str());
        }
    }
    else
    {
        //log("no cache connection for group_member");
    }
}

//更新组最后聊天时间
void CGroupModel::updateGroupChat(uint32_t nGroupId)
{
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master");
    if(pDBConn)
    {
        uint32_t nNow = (uint32_t)time(NULL);
        string strSql = "update IMGroup set lastChated=" + int2string(nNow) + " where id=" + int2string(nGroupId);
        pDBConn->ExecuteUpdate(strSql.c_str());
    }
    else
    {
        //log("no db connection for teamtalk_master");
    }
}

bool CGroupModel::isValidateGroupId(uint32_t nGroupId)
{
    bool bRet = false;
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("group_member");
    if(pCacheConn)
    {
        string strKey = "group_member_"+int2string(nGroupId);
        bRet = pCacheConn->isExists(strKey);
        pCacheManager->RelCacheConn(pCacheConn);
    }
    return bRet;
}


void CGroupModel::removeSession(uint32_t nGroupId, const set<uint32_t> &setUser)
{
    for(auto it=setUser.begin(); it!=setUser.end(); ++it)
    {
        uint32_t nUserId=*it;
        uint32_t nSessionId = CSessionModel::getInstance()->getSessionId(nUserId, nGroupId, IM::BaseDefine::SESSION_TYPE_GROUP, false);
        CSessionModel::getInstance()->removeSession(nSessionId);
    }
}

bool CGroupModel::incGroupVersion(uint32_t nGroupId)
{
    bool bRet = false;
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master");
    if(pDBConn)
    {
        string strSql = "update IMGroup set version=version+1 where id="+int2string(nGroupId);
        if(pDBConn->ExecuteUpdate(strSql.c_str()))
        {
            bRet = true;
        }
    }
    else
    {
        //log("no db connection for teamtalk_master");
    }
    return  bRet;
}

string CGroupModel::getGroupPKRule(uint32_t nGroupId)
{
    string rules;
    if (auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master")) {
        string strSql = "select * from IMGroup where id=" + int2string(nGroupId) + " and type=" + int2string(IM::BaseDefine::GROUP_TYPE_PK10);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if (pResultSet)
        {
            while (pResultSet->Next()) {
                rules = pResultSet->GetString("pk10rules");
            }

            delete  pResultSet;
        }

        if (rules.empty()){
            rules = getPk10DefRules();
        }
    }
    else
    {
        Warning("no db connection for teamtalk_master")
    }
    return rules;
}

string CGroupModel::getPk10DefRules()
{
    Json::Value obj;
            
    obj["main"] = 1;
    obj["fly"] = 0;
    obj["close"] = 30;
    obj["water"] = 0.0;

    Json::Value rule;

    // 车号
    Json::Value num1;
    num1["ch"] = 1.0;   // 车号
    num1["dx"] = 1.0;   // 大小
    num1["ds"] = 1.0;   // 单双
    num1["dd"] = 1.0;   // 大单 
    num1["ds"] = 1.0;   // 大双 
    num1["xd"] = 1.0;   // 小单 
    num1["xs"] = 1.0;   // 小双 
    num1["lh"] = 1.0;   // 龙虎
    rule.append(num1);

    // 冠亚号 庄 闲
    Json::Value num2;
    num2["n_gyh"] = 1.0;    // 冠亚号 
    num2["n_z"] = 1.0;      // 冠亚号 [庄]
    num2["n_x"] = 1.0;      // 冠亚号 [闲]
    rule.append(num2);

    
    Json::Value num3;
    num3["s_dx"] = 1.0;     // 冠亚和 [大小]
    num3["s_ds"] = 1.0;     // 冠亚和 [单双]
    num3["s_a"] = 1.0;      // 冠亚和 [A]
    num3["s_b"] = 1.0;      // 冠亚和 [B]
    num3["s_c"] = 1.0;      // 冠亚和 [C]
    for (int i=3; i!=20; ++i)
    {
        num3["s_" + int2string(i)] = 1.0;   // 冠亚和 [3-19]
    }
    rule.append(num3);

    obj["odd"] = rule;
    return obj.toStyledString();
}

void CGroupModel::fillGroupMember(list<IM::BaseDefine::GroupInfo>& lsGroups)
{
    for (auto it=lsGroups.begin(); it!=lsGroups.end(); ++it) {
        list<uint32_t> lsUserIds;
        uint32_t nGroupId = it->group_id();
        getGroupUser(nGroupId, lsUserIds);
        for(auto itUserId=lsUserIds.begin(); itUserId!=lsUserIds.end(); ++itUserId)
        {
            it->add_group_member_list(*itUserId);
        }
    }
}

uint32_t CGroupModel::getUserJoinTime(uint32_t nGroupId, uint32_t nUserId)
{
    uint32_t nTime = 0;
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("group_member");
    if (pCacheConn)
    {
        string strKey = "group_member_" + int2string(nGroupId);
        string strField = int2string(nUserId);
        string strValue = pCacheConn->hget(strKey, strField);
        pCacheManager->RelCacheConn(pCacheConn);
        if (!strValue.empty()) {
            nTime = string2int(strValue);
        }
    }
    else
    {
        //log("no cache connection for group_member");
    }
    return  nTime;
}

void CGroupModel::clearGroupMember(uint32_t nGroupId)
{
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master");
    if(pDBConn)
    {
        string strSql = "delete from IMGroupMember where groupId="+int2string(nGroupId);
        pDBConn->ExecuteUpdate(strSql.c_str());
    }
    else
    {
        //log("no db connection for teamtalk_master");
    }
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("group_member");
    if(pCacheConn)
    {
        string strKey = "group_member_" + int2string(nGroupId);
        map<string, string> mapRet;
        bool bRet = pCacheConn->hgetAll(strKey, mapRet);
        if(bRet)
        {
            for(auto it=mapRet.begin(); it!=mapRet.end(); ++it)
            {
                pCacheConn->hdel(strKey, it->first);
            }
        }
        else
        {
            //log("hgetall %s failed", strKey.c_str());
        }
        pCacheManager->RelCacheConn(pCacheConn);
    }
    else
    {
        //log("no cache connection for group_member");
    }
}

bool CGroupModel::setGroupPk10Rules(uint32_t nUserId, uint32_t nGroupId, string szPkRules, uint32_t& nOption)
{
    if (auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_master")) {
        string strSql = "update IMGroup set pk10rules=" + szPkRules + " where id=" + int2string(nGroupId) + " and creator=" + int2string(nUserId);
        pDBConn->ExecuteUpdate(strSql.c_str());

        // 权限
        strSql = "select * from IMUser where id=" + int2string(nUserId);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if (pResultSet)
        {
            while (pResultSet->Next()) {
                nOption = pResultSet->GetInt("memberorder");
            }

            delete  pResultSet;
        }
        return true;
    }
    else
    {
        //log("no db connection for teamtalk_master");
        return false;
    }
}

void CGroupModel::getGroupsPKRules(list<IM::BaseDefine::GroupPKInfo>& lsPkInfo)
{
    // 获取所有有PK群用户ID （代理 +  管理）
    uint32_t nAdminId;
    list<uint32_t> lsProxyId;
    CUserModel::getInstance()->getProxys(lsProxyId, nAdminId);
    lsProxyId.push_back(nAdminId);

    list<uint32_t>::iterator it = lsProxyId.begin();
    for (; it!=lsProxyId.end(); ++it)
    {
        IM::BaseDefine::GroupPKInfo info;

        uint32_t nUserId = *it;

        // 用户/代理的 PK群
        list<uint32_t> lsGroupId;
        getGroupByCreator(nUserId, IM::BaseDefine::GROUP_TYPE_PK10, lsGroupId);

        if (lsGroupId.size() == 1)
        {
            uint32_t nGroupId = lsGroupId.front();
            string rule = getGroupPKRule(nGroupId);

            info.set_user_id(nUserId);
            info.set_is_admin(nUserId==nAdminId?1:0);
            info.set_group_id(nGroupId);
            info.set_rules(rule);

            lsPkInfo.push_back(info);
        }
    }
}
CGroupModel* CGroupModel::m_pInstance = NULL;

/**
 *  <#Description#>
 */
CGroupModel::CGroupModel()
{
    
}

CGroupModel::~CGroupModel()
{
    
}

CGroupModel* CGroupModel::getInstance()
{
    if (!m_pInstance) {
        m_pInstance = new CGroupModel();
    }
    return m_pInstance;
}
