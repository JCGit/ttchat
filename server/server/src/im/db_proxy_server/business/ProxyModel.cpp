#include <map>
#include <set>
#include <sstream>
#include "uchat/stringview.hpp"
#include "uchat/config.hpp"
#include "json/json.h"
#include "../DBPool.h"
#include "../CachePool.h"
#include "ProxyModel.h"
#include "AudioModel.h"
#include "SessionModel.h"
#include "RelationModel.h"
#include "MessageModel.h"
#include "UserModel.h"
#include "GroupMessageModel.h"
#include "ProxyConn.h"
#include "Common.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Group.pb.h"
#include "uchat/errno.hpp"
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
#include "uchat/simplevariantview.hpp"

constexpr char const* kN = "db_proxy_server";
using ProxyModel = CProxyModel;
using GroupModel = CGroupModel;
using DBManager = CDBManager;
using namespace std;
using namespace uchat;
using namespace uchat::boost;
using namespace IM::BaseDefine;
using namespace IM::Proxy;
uint32_t ProxyModel::setproxy(uint32_t userId, bool enable, uint32_t tgtUid, uint32_t& groupId) noexcept try {
  {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    if (!dbc) {
        Error0(__func__ << ": getdbconn teamtalk_slave fail")
        return uint32_t(Errno::CannotGetDbConnection);
    }
    ///
    uint32_t memPerm;
    {
        // Check userId == admin
        std::string const sql = "SELECT * FROM `IMUser` WHERE `id` = " + std::to_string(userId);
        Debug(__func__ << ": sql " << sql)
        auto const qres = dbc->executeQuery(sql);
        if (!qres) {
            Error0(__func__ << ": executeQuery fail: " << sql)
            return uint32_t(Errno::DatabaseQueryError);
        }
        if (qres.isEmpty()) {
            Error0(__func__ << ": cannot find userId " << userId)
            return uint32_t(Errno::UserIdNotFound);
        }
        auto const it = qres.begin();
        memPerm = uint32_t(it("memberorder").as<NumStrView>());
        if (IM::BaseDefine::MemberOrder::MEMBER_ADMIN != memPerm) {
            //return Errno::NotAdmin;
            return 1;// 非管理员，无权限设置代理
        }
    }
    ///
    uint32_t oTgtPerm;
    std::string targetUsername;
    bool hasExProxy = false;
    {
        // Query oTgtPerm and targetUsername
        std::string sql = "SELECT * FROM `IMUser` WHERE `id` = "
            + std::to_string(tgtUid);
        Debug(__func__ << ": sql " << sql)
        auto qres = dbc->executeQuery(sql);
        if (!qres) {
            Error0(__func__ << ": executeQuery fail: " << sql)
            return uint32_t(Errno::DatabaseQueryError);
        }
        auto const it = qres.begin();
        if (it == qres.end()) {
            Error0(__func__ << ": cannot find userId from `IMUser` "
                << userId)
            return uint32_t(Errno::UserIdNotFound);
        }
        oTgtPerm = uint32_t(it("memberorder").as<
            NumStrView>());
        if (oTgtPerm != MemberOrder::MEMBER_COMMON && enable) {
            Debug(__func__ << ": target user is proxy or admin")
            return 2;// 该用户已是代理或管理员
        } else if (oTgtPerm
            != IM::BaseDefine::MemberOrder::MEMBER_PROXY
            && !enable) {
            Debug(__func__ << ": target user not proxy: not need disable")
            return 3;// 该用户不是代理无需取消
        }
        targetUsername = std::string(it("name").as<StringView>());
        // Query hasExProxy
        sql = "SELECT COUNT(*) AS `c` FROM `IMProxy` WHERE `id` = " + std::to_string(tgtUid);
        qres = dbc->executeQuery(sql);
        if (!qres) {
            Error0(__func__ << ": executeQuery fail: " << sql)
            return uint32_t(Errno::DatabaseQueryError);
        }
        auto it2 = qres.begin();
        if (it2 == qres.end()) {
            Error0(__func__ << ": cannot find userId from IMProxy " << userId)
            return uint32_t(Errno::UserIdNotFound);
        }
        hasExProxy = uint32_t(it2("c").as<NumStrView>()) != 0;
    }
    ///
    auto const setTargetPermission = [&dbc, &tgtUid](
        uint32_t const perm) -> bool {
        std::string const sql = "UPDATE `IMUser` SET `memberorder` = "
            + std::to_string(perm) + " WHERE `id` = "
            + std::to_string(tgtUid);
        if (!dbc->ExecuteUpdate(sql.c_str())) {
            Error0(__func__ << ": ExecuteUpdate fail: " << sql)
            return false;
        }
        return true;
    };
    // 执行赋权: IMUser
    if (!setTargetPermission(enable ? 1u : 0u)) {
        return uint32_t(Errno::DatabaseQueryError);
    }
    // 执行赋权: IMProxy
    uint32_t const now = ::time(nullptr);
    std::string sql;
    if (hasExProxy) {
        if (enable) {
            sql = "UPDATE `IMProxy` SET `status` = 0,"
                " `updated` = " + std::to_string(now)
                + ", `name` = '" + targetUsername
                + "' WHERE `id` = " + std::to_string(tgtUid);
        } else {
            sql = "UPDATE `IMProxy` SET `status` = 1,"
                " `updated` = " + std::to_string(now)
                + ", `name` = '" + targetUsername
                + "' WHERE `id` = " + std::to_string(tgtUid);
        }
    } else {
        if (enable) {
            sql = "INSERT INTO `IMProxy` (`id`, `name`,"
                " `created`, `updated`, `status`) VALUES ("
                + std::to_string(tgtUid) + ", '"
                + targetUsername + "', "
                + std::to_string(now) + ", "
                + std::to_string(now) + ", 0);";
        } else {
            sql = "INSERT INTO `IMProxy` (`id`, `name`,"
                " `created`, `updated`, `status`) VALUES ("
                + std::to_string(tgtUid) + ", '"
                + targetUsername + "', "
                + std::to_string(now) + ", "
                + std::to_string(now) + ", 1);";
        }
    }
    Debug(__func__ << ": " << sql)
    if (!dbc->ExecuteUpdate(sql.c_str())) {
        Warning(__func__ << ": ExecuteQuery fail " << sql << ": rollback")
        // Fail to rollback
        setTargetPermission(oTgtPerm);
        return uint32_t(Errno::DatabaseQueryError);
    }
  } // NOTE before belows newgrp/removeGroup: release dbc
    //
    if (enable) {
        // 创建代理默认PK群
        std::set<uint32_t> members;
        members.insert(userId);
        groupId = INVALID_VALUE;
        GroupModel::getInstance()->newgrp(
            userId,
            "PK10",
            "",
            GroupType::GROUP_TYPE_PK10,
            members,
            groupId);
    } else {
        // 禁用代理时 禁用PK10群
        std::vector<GroupVersionInfo> gvis;
        GroupModel::getInstance()->fetUBGrpinfo(userId, gvis, GroupType::GROUP_TYPE_PK10);
        for (auto const& v : gvis) {
            std::list<uint32_t> u;
            GroupModel::getInstance()->removeGroup(userId, v.group_id(), u);
        }
    }
    return 0;// OK
} catch(std::exception const& e) {
    Error0(e.what())
    return uint32_t(Errno::Exception);
}
int32_t ProxyModel::lsfrnds(uint32_t userId, uint32_t lstUp, ListFriendType type, std::vector<UserInfo>& users) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    Assert(dbc, "getdbconn fail")
    std::string sql;
    if (ListFriendType::Proxy == type) {
        // 我是代理的下线(userId == `id`) 查找代理 ID (id of proxy == `binded`)
        // Then query the user which's name == name of proxy (should be 1)
        sql = "SELECT * FROM `IMUser` WHERE `name` IN (SELECT `name`"
            " FROM `IMProxy` WHERE `id` = (SELECT `binded` FROM"
            " `IMUser` WHERE `id` = " + std::to_string(userId)
            + " )) ORDER BY `name` LIMIT "
            + std::to_string(kMaxFriendCount)
            + ";";
    } else if (ListFriendType::Subline == type) {
        // 我作为代理 找我的下线
        sql = "SELECT * FROM `IMUser` WHERE `binded` ="
            " (SELECT `id` FROM `IMProxy` WHERE `name` ="
            " (SELECT `name` FROM `IMUser` WHERE `id` = "
            + std::to_string(userId) + " ) LIMIT 1) ORDER BY `name` LIMIT "
            + std::to_string(kMaxFriendCount) + ";";
    } else if (ListFriendType::Friend == type) {
#       if 1
        // Online/offline/... friends
        std::set<uint32_t> friendIds;
        {
            // 1/2: userId is <from>
            std::string const sql = "SELECT `toId` FROM `IMFriend` WHERE "
                "`fromId` = " + std::to_string(userId)
                + " AND (`status` = 0 OR `status` = 4) LIMIT "
                + std::to_string(kMaxFriendCount) + ";";
            DBQueryResult const qres = dbc->executeQuery(sql);
            for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
                //friendIds.emplace_back(::atoi(it("toId").c_str()));
                if (SimpleVariantView const v = it("toId")){
                // if (auto const v = it("toId")) {
                    if (auto const v2 = v.as<NumStrView>()) {
                        friendIds.insert(uint32_t(v2));
                    }
                }
            }
        }
        {
            // 2/2: userId is <to>
            std::string const sql = "SELECT `fromId` FROM `IMFriend` WHERE "
                "`toId` = " + std::to_string(userId)
                + " AND (`status` = 0 OR `status` = 3) LIMIT "
                + std::to_string(kMaxFriendCount) + ";";
            DBQueryResult const qres = dbc->executeQuery(sql);
            for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
                //friendIds.emplace_back(::atoi(it("fromId").c_str()));
                if (auto const v = it("fromId")) {
                    if (auto const v2 = v.as<NumStrView>()) {
                        friendIds.insert(uint32_t(v2));
                    }
                }
            }
        }
        if (friendIds.empty()) {
            sql = "";
        } else {
            auto it = friendIds.cbegin();
            std::string idsSql = std::to_string(*it);
            ++it;
            for (auto end = friendIds.cend(); it != end; ++it) {
                idsSql += ", " + std::to_string(*it);
            }
            sql = "SELECT * FROM `IMUser` WHERE `id` IN (";
            sql += idsSql + ") ORDER BY `name` LIMIT "
                + std::to_string(kMaxFriendCount)
                + ";";
        }
#       else
        // TMP TEST
        sql = "SELECT * FROM `IMUser` ORDER BY `name` LIMIT "
            + std::to_string(kMaxUserQueryLimit) + ";";
#       endif
    } else if (ListFriendType::NewAddRequest == type) {
        // New friends (userId is <to>)
        sql = "SELECT * FROM `IMUser` WHERE `id` IN (SELECT `fromId` "
            "FROM `IMFriend` WHERE (`toId` = ";
        sql += std::to_string(userId) + " AND `status` = 1 AND `updated` > "
            + std::to_string(lstUp) + ")) ORDER BY `name` LIMIT "
            + std::to_string(kMaxFriendCount) + ";";
    //} else if (ListFriendType::RequestAddFriend == type) {
    //    sql = "SELECT * FROM `IMUser` WHERE `id` IN (SELECT `toId` "
    //        "FROM `IMFriend` WHERE (`fromId` = ";
    //    sql += std::to_string(userId) + " AND `status` = 1 AND `updated` > "
    //        + std::to_string(lstUp) + ")) ORDER BY `name` LIMIT "
    //        + std::to_string(kMaxFriendCount) + ";";
    } else {
        sql = "";
    }
    users.clear();
    if (sql.empty()) {
        users.shrink_to_fit();
        return 0;
    }
    DBQueryResult const qres = dbc->executeQuery(sql);
    IM::BaseDefine::UserInfo userInfo;
    for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
        userInfo.set_user_id(uint32_t(it("id").as<NumStrView>()));
        userInfo.set_user_nick_name(std::string(it("nick").as<StringView>()));
        userInfo.set_user_gender(uint32_t(it("sex").as<NumStrView>()));
        userInfo.set_avatar_url(std::string(it("avatar").as<StringView>()));
        userInfo.set_user_real_name(std::string(it("name").as<StringView>()));
        userInfo.set_member_order(uint32_t(it("memberorder").as<NumStrView>()));
        userInfo.set_sign_info(std::string(it("sign_info").as<StringView>()));
        userInfo.set_birthday(uint32_t(it("birthday").as<NumStrView>()));
        users.emplace_back(std::move(userInfo));
    }
    users.shrink_to_fit();
    Debug(__func__ << ": sql " << sql << " n " << users.size())
    return 0;
} catch(std::exception const& e) {
    Error0(e.what())
    return -1;
}
int32_t ProxyModel::lsfrnds(uint32_t userId, uint32_t lstUp, ListFriendType type, DBCP const& dbc) noexcept try {
    Assert(dbc, "nil fail")
    std::string sql;
    if (ListFriendType::Proxy == type) {
        // 我是代理的下线(userId == `id`) 查找代理 ID (id of proxy == `binded`)
        // Then query the user which's name == name of proxy (should be 1)
        sql = "SELECT COUNT(*) AS ``c FROM `IMUser` WHERE `name` IN (SELECT"
            " `name` FROM `IMProxy` WHERE `id` = (SELECT `binded` FROM"
            " `IMUser` WHERE `id` = " + std::to_string(userId)
            + " )) ORDER BY `name` LIMIT "
            + std::to_string(kMaxFriendCount);
    } else if (ListFriendType::Subline == type) {
        // 我作为代理 找我的下线
        sql = "SELECT COUNT(*) AS `c` FROM `IMUser` WHERE `binded` ="
            " (SELECT `id` FROM `IMProxy` WHERE `name` ="
            " (SELECT `name` FROM `IMUser` WHERE `id` = "
            + std::to_string(userId) + " ) LIMIT 1) ORDER BY `name` LIMIT "
            + std::to_string(kMaxFriendCount) + ";";
    } else if (ListFriendType::Friend == type) {
        // Online/offline/... friends
        std::set<uint32_t> friendIds;
        {
            // 1/2: userId is <from>
            std::string const sql = "SELECT `toId` FROM `IMFriend` WHERE "
                "`fromId` = " + std::to_string(userId)
                + " AND (`status` = 0 OR `status` = 4) LIMIT "
                + std::to_string(kMaxFriendCount) + ";";
            DBQueryResult const qres = dbc->executeQuery(sql);
            for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
                //friendIds.emplace_back(::atoi(it("toId").c_str()));
                if (SimpleVariantView const v = it("toId")){
                // if (auto const v = it("toId")) {
                    if (auto const v2 = v.as<NumStrView>()) {
                        friendIds.insert(uint32_t(v2));
                    }
                }
            }
        }
        {
            // 2/2: userId is <to>
            std::string const sql = "SELECT `fromId` FROM `IMFriend` WHERE "
                "`toId` = " + std::to_string(userId)
                + " AND (`status` = 0 OR `status` = 3) LIMIT "
                + std::to_string(kMaxFriendCount) + ";";
            DBQueryResult const qres = dbc->executeQuery(sql);
            for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
                //friendIds.emplace_back(::atoi(it("fromId").c_str()));
                if (auto const v = it("fromId")) {
                    if (auto const v2 = v.as<NumStrView>()) {
                        friendIds.insert(uint32_t(v2));
                    }
                }
            }
        }
        if (friendIds.empty()) {
            sql = "";
        } else {
            auto it = friendIds.cbegin();
            std::string idsSql = std::to_string(*it);
            ++it;
            for (auto end = friendIds.cend(); it != end; ++it) {
                idsSql += ", " + std::to_string(*it);
            }
            sql = "SELECT COUNT(*) AS `c` FROM `IMUser` WHERE `id` IN (";
            sql += idsSql + ") ORDER BY `name` LIMIT "
                + std::to_string(kMaxFriendCount)
                + ";";
        }
    } else if (ListFriendType::NewAddRequest == type) {
        // New friends (userId is <to>)
        sql = "SELECT COUNT(*) AS `c` FROM `IMUser` WHERE `id` IN (SELECT"
            " `fromId` FROM `IMFriend` WHERE (`toId` = ";
        sql += std::to_string(userId) + " AND `status` = 1 AND `updated` > "
            + std::to_string(lstUp) + ")) ORDER BY `name` LIMIT "
            + std::to_string(kMaxFriendCount) + ";";
    //} else if (ListFriendType::RequestAddFriend == type) {
    //    sql = "SELECT COUNT(*) AS `c` FROM `IMUser` WHERE `id` IN (SELECT"
    //        " `toId` FROM `IMFriend` WHERE (`fromId` = ";
    //    sql += std::to_string(userId) + " AND `status` = 1 AND `updated` > "
    //        + std::to_string(lstUp) + ")) ORDER BY `name` LIMIT "
    //        + std::to_string(kMaxFriendCount) + ";";
    } else {
        sql = "";
    }
    if (sql.empty()) {
        Debug(__func__ << " empty sql not query")
        return 0;
    }
    DBQueryResult const qres = dbc->executeQuery(sql);
    auto const it = qres.begin();
    Assert(it != qres.end(), "query fail: " << sql)
    int32_t const count = int32_t(it("c").as<NumStrView>());
    Warning(__func__ << ": sql " << sql << " count " << count)
    return count;
} catch(std::exception const& e) {
    Error0(e.what())
    return -1;
}
int32_t ProxyModel::lsfrnds(uint32_t userId, std::set<uint32_t>& friendIds, DBCP const& dbc) noexcept try {
    Assert(dbc, "nil dbc")
    std::string sql;
    // Online/offline/... friends
    friendIds.clear();
    {
        // 1/2: userId is <from>
        std::string const sql = "SELECT `toId` FROM `IMFriend` WHERE "
            "`fromId` = " + std::to_string(userId)
            + " AND (`status` = 0 OR `status` = 4) LIMIT "
            + std::to_string(kMaxFriendCount) + ";";
        DBQueryResult const qres = dbc->executeQuery(sql);
        for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
            //friendIds.emplace_back(::atoi(it("toId").c_str()));
            if (SimpleVariantView const v = it("toId")){
            // if (auto const v = it("toId")) {
                if (auto const v2 = v.as<NumStrView>()) {
                    friendIds.insert(uint32_t(v2));
                }
            }
        }
    }
    {
        // 2/2: userId is <to>
        std::string const sql = "SELECT `fromId` FROM `IMFriend` WHERE "
            "`toId` = " + std::to_string(userId)
            + " AND (`status` = 0 OR `status` = 3) LIMIT "
            + std::to_string(kMaxFriendCount) + ";";
        DBQueryResult const qres = dbc->executeQuery(sql);
        for (auto it = qres.begin(), end = qres.end();
            it != end; ++it) {
            //friendIds.emplace_back(::atoi(it("fromId").c_str()));
            if (auto const v = it("fromId")) {
                if (auto const v2 = v.as<NumStrView>()) {
                    friendIds.insert(uint32_t(v2));
                }
            }
        }
    }
    return 0;// OK
} catch(std::exception const& e) {
    Error0(e.what())
    return -1;
}
uint32_t ProxyModel::friendop(uint32_t userId, uint32_t proxyId, UserOperate op) noexcept try {
    if (userId == proxyId) {
        Error0("you not need add/delete/... yourself")
        return 36;
    }
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    if (!dbc) {
        return 65535;
    }
    if (!this->validUser(proxyId, dbc)) {
        Error0(__func__ << ": friend operate dest user id not valid: " << proxyId)
        return 37;
    }
    std::vector<std::string> sqls;
    uint64_t const now = uint64_t(::time(nullptr));
    if (USER_OPERATE_FRIEND_AGGRE == op || USER_OPERATE_FRIEND_REFUSE == op) {
        // 同意 / 拒绝处理
        // Query ADD REQ
        std::string sql = "SELECT * FROM `";
        sql += conf::kDBName;
        sql += "`.`";
        sql += conf::kFriendTableName;
        sql += "` WHERE ((`status` = 1) AND ((`fromId` = ";
        sql += std::to_string(userId);
        sql += " AND `toId` = " + std::to_string(proxyId) + ")";
        sql += " OR (`fromId` = " + std::to_string(proxyId);
        sql += " AND `toId` = " + std::to_string(userId) + ")));";
        Debug("sql " << sql)
        DBQueryResult const qres = dbc->executeQuery(sql);
        bool agreeOrDeclineAble = false;
        for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
            uint32_t const existsToId = uint32_t(
                it("toId").as<NumStrView>());
            if (existsToId == userId) {
                Debug("exists: " << proxyId << " add " << existsToId <<": agreeOrDeclineAble")
                agreeOrDeclineAble = true;
                break;
            }
        }
        if (!agreeOrDeclineAble) {
            Error0("agree or decline but no ADD and `toId` == userId: " << userId)
            return 31;
        }
        if (USER_OPERATE_FRIEND_AGGRE == op) {
            // TODO notify friend
            sql = "UPDATE `IMFriend` SET `status` = 0, `updated` = "
                + std::to_string(now) + " WHERE (`fromId` = "
                + std::to_string(proxyId) + " AND `toId` = "
                + std::to_string(userId)
                + " AND `status` = 1) OR (`fromId` = "
                + std::to_string(userId) + " AND `toId` = "
                + std::to_string(proxyId)
                + " AND `status` = 1);";
            sqls.emplace_back(std::move(sql));
        } else {
            // IM::Proxy::UserOperate::USER_OPERATE_FRIEND_REFUSE
            sql = "update IMFriend set status=2, updated="
                + std::to_string(now) + " WHERE (fromId="
                + std::to_string(proxyId) + " and toId="
                + std::to_string(userId)
                + " AND `status` = 1);";
            sqls.emplace_back(std::move(sql));
        }
    } else if (USER_OPERATE_FRIEND_ADD == op) {
        // Query 0 or 4
        std::string sql = "SELECT * FROM `IMFriend`"
            " WHERE ((`status` = 0 || `status` = 4) AND ((`fromId` = "
            + std::to_string(userId)
            + " AND `toId` = " + std::to_string(proxyId) + ")"
            + " OR (`fromId` = " + std::to_string(proxyId)
            + " AND `toId` = " + std::to_string(userId) + ")));";
        Debug(__func__ << ": sql: " << sql)
        DBQueryResult qres = dbc->executeQuery(sql);
        if (!qres) {
            Error0(__func__ << ": executeQuery fail " << sql)
            return 33;
        }
        if (!qres.isEmpty()) {
            Info0(userId << "cannot add " << proxyId << ": you have that friend")
            return 34;
        }
        /*
         * 添加处理
         * - if has <from> add <to> state is 1 the update time only
         * - if has <from> add <to> state not 1(=>also not 0, 4) => set state
         *   to 1 and set update time
         * - if no <from> add <to> => insert
         *
         * Query:
         * - status in [1, 2, 3, 5]
         * - and fromId is userId
         * - and toId is proxyId
         */
        sql = "SELECT `status` FROM `IMFriend`"
            " WHERE `status` IN (1, 2, 3, 5) AND `fromId` = "
            + std::to_string(userId)
            + " AND `toId` = " + std::to_string(proxyId) + ";";
        Debug(__func__ << ": sql: " << sql)
        qres = dbc->executeQuery(sql);
        if (!qres) {
            Error0(__func__ << ": executeQuery fail " << sql)
            return 33;
        }
        if (qres.isEmpty()) {
            sql = "INSERT INTO `IMFriend` (`fromId`, `toId`, `created`,"
                " `updated`, `status`) VALUES (" + std::to_string(userId)
                + ", " + std::to_string(proxyId) + ","
                + std::to_string(now) + ", " + std::to_string(now)
                + ", " + std::to_string(1) + ");";
        } else {
            sql = "UPDATE `IMFriend` SET `status` = 1, `updated` = "
                + std::to_string(now)
                + " WHERE `fromId` = "
                + std::to_string(userId)
                + " AND `toId` = " + std::to_string(proxyId);
        }
        Debug(__func__ << ": do " << userId << " add -> " << proxyId)
        sqls.emplace_back(std::move(sql));
    } else if (USER_OPERATE_FRIEND_DELETE == op || USER_OPERATE_FRIEND_DELETE_BOTH == op) {
        // Query not both deleted
        std::string sql = "SELECT * FROM `";
        sql += conf::kDBName;
        sql += "`.`";
        sql += conf::kFriendTableName;
        sql += "` WHERE ((`status` NOT IN (2, 5) ) AND ((`fromId` = ";
        sql += std::to_string(userId);
        sql += " AND `toId` = " + std::to_string(proxyId) + ")";
        sql += " OR (`fromId` = " + std::to_string(proxyId);
        sql += " AND `toId` = " + std::to_string(userId) + ")));";
        Debug("sql: " << sql)
        DBQueryResult const qres = dbc->executeQuery(sql);
        if (!qres || qres.isEmpty()) {
            Note("DELETE FRIEND but no friend found for userId: " << userId)
            return 32;
        }
        sql = "";
        bool const delBoth = (USER_OPERATE_FRIEND_DELETE_BOTH == op);
        for (auto it = qres.begin(), end = qres.end(); it != end; ++it) {
            uint32_t curFromId = uint32_t(it("fromId").as<NumStrView>());
            uint32_t curSt = uint32_t(it("status").as<NumStrView>());
            uint32_t newState = curSt;
            if (curFromId == userId) {
                if (3 == curSt && !delBoth) {
                    Warning("i(` " << userId << "') have deleted `" << proxyId << "'")
                    continue;
                }
                if (3 == curSt || 0 == curSt || 4 == curSt) {
                    Debug("i(`" << userId << "') now delete `" << proxyId << "'")
                    newState = (delBoth ? 5 : (0 == curSt ? 3 : 5));
                } else {
                    Warning("i(`" << userId << "') cannot delete `" << proxyId << "': invalid state: " << curSt)
                    continue;
                }
                sql = "UPDATE `IMFriend` SET `status` = "
                    + std::to_string(newState)
                    + " WHERE (`fromId` = " + std::to_string(userId)
                    + " AND `toId` = " + std::to_string(proxyId)
                    + " AND `status` = " + std::to_string(curSt)
                    + "); ";
                sqls.emplace_back(std::move(sql));
            } else {
                // currentToId == userId
                if (4 == curSt && !delBoth) {
                    Warning("i(` " << userId << "') have deleted `" << proxyId << "'")
                    continue;
                }
                if (4 == curSt || 0 == curSt || 3 == curSt) {
                    Debug("i(`" << userId << "') now delete `" << proxyId << "'")
                    newState = (delBoth ? 5 : (0 == curSt ? 4 : 5));
                } else {
                    Warning("i(`" << userId << "') cannot delete `" << proxyId << "': invalid state: " << curSt)
                    continue;
                }
                sql = "UPDATE `IMFriend` SET `status` = "
                    + std::to_string(newState)
                    + " WHERE (`fromId` = " + std::to_string(proxyId)
                    + " AND `toId` = " + std::to_string(userId)
                    + " AND `status` = " + std::to_string(curSt)
                    + "); ";
                sqls.emplace_back(std::move(sql));
            }
        }
    } else {
        Error0("invalid operate: " << int32_t(op))
        return 35;
    }
    uint32_t ret = 0;
    for (auto const& sql : sqls) {
        Debug("do sql " << sql)
        if (!dbc->ExecuteUpdate(sql.c_str())) {
            ret = 33;
            break;
        }
    }
    return ret;
} catch(std::exception const& e) {
    Error0(e.what())
    return 33;
}
bool ProxyModel::validUser(uint32_t userId, DBCP const& dbc) noexcept try {
    Assert(dbc, "nil dbc")
    // Current only valid exists
    std::string const sql = "SELECT COUNT(*) AS `c` FROM `IMUser` WHERE `id` = " + std::to_string(userId);
    Debug(__func__ << ": sql: " << sql)
    auto const qres = dbc->executeQuery(sql);
    Assert(qres && !qres.isEmpty(), "query fail: " << sql)
    uint32_t didCount = uint32_t(qres.begin()("c").as<NumStrView>());
    Assert(1 == didCount, "expect 1 " << " but did got " << didCount)
    return true;
} catch(std::exception const& e) {
    Error0(e.what())
    return false;
}
bool ProxyModel::validUser(uint32_t userId) noexcept try {
    auto dbc = DBManager::getInstance()->getdbconn("teamtalk_slave");
    return this->validUser(userId, dbc);
} catch(std::exception const& e) {
    Error0(e.what())
    return false;
}

CProxyModel* CProxyModel::m_pInstance = NULL;

//const uint32_t TYPE_USER_LIST_PROXY = 1;///< proxy
//const uint32_t TYPE_USER_LIST_FRIEND = 2;///< all online/offline/... friends
//const uint32_t TYPE_USER_LIST_SUBLINE = 3;///< subline of proxy
//const uint32_t TYPE_USER_LIST_NEWADDER = 4;///< new friends
CProxyModel::CProxyModel()
{

}

CProxyModel::~CProxyModel()
{

}

CProxyModel* CProxyModel::getInstance()
{
	if (!m_pInstance) {
		m_pInstance = new CProxyModel();
	}

	return m_pInstance;
}

void CProxyModel::incBindCount(uint32_t nFromId, uint32_t nToId)
{
    CacheManager* pCacheManager = CacheManager::getInstance();
	// increase message count
	CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
	if (pCacheConn) {
		pCacheConn->hincrBy("bind_" + int2string(nToId), int2string(nFromId), 1);
		pCacheManager->RelCacheConn(pCacheConn);
	} else {
		//log("no cache connection to increase bind proxy count: %d->%d", nFromId, nToId);
	}
}

void CProxyModel::getUnreadBindCount(uint32_t nUserId, uint32_t &nTotalCnt, list<IM::BaseDefine::UnreadInfo>& lsUnreadCount)
{
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
    if (pCacheConn)
    {
        map<string, string> mapUnread;
        string strKey = "bind_" + int2string(nUserId);
        bool bRet = pCacheConn->hgetAll(strKey, mapUnread);
        pCacheManager->RelCacheConn(pCacheConn);

        if(bRet)
        {
            IM::BaseDefine::UnreadInfo cUnreadInfo;
            for (auto it = mapUnread.begin(); it != mapUnread.end(); it++) {
                cUnreadInfo.set_session_type(IM::BaseDefine::SESSION_TYPE_SINGLE);
                cUnreadInfo.set_session_id(atoi(it->first.c_str()));
                cUnreadInfo.set_unread_cnt(atoi(it->second.c_str()));

                lsUnreadCount.push_back(cUnreadInfo);
                nTotalCnt += cUnreadInfo.unread_cnt();
            }
        }
        else
        {
            //log("hgetall %s failed!", strKey.c_str());
        }
    }
    else
    {
        //log("no cache connection for unread");
    }
}

void CProxyModel::incFriendAggreCount(uint32_t nFromId, uint32_t nToId)
{
    CacheManager* pCacheManager = CacheManager::getInstance();
	// increase message count
	CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
	if (pCacheConn) {
		pCacheConn->hincrBy("aggre_" + int2string(nToId), int2string(nFromId), 1);
		pCacheManager->RelCacheConn(pCacheConn);
	} else {
		//log("no cache connection to increase bind proxy count: %d->%d", nFromId, nToId);
	}
}
    
void CProxyModel::getUnreadAggreFriendCount(uint32_t nUserId, uint32_t &nTotalCnt, list<IM::BaseDefine::UnreadInfo>& lsUnreadCount)
{
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
    if (pCacheConn)
    {
        map<string, string> mapUnread;
        string strKey = "aggre_" + int2string(nUserId);
        bool bRet = pCacheConn->hgetAll(strKey, mapUnread);
        pCacheManager->RelCacheConn(pCacheConn);

        if(bRet)
        {
            IM::BaseDefine::UnreadInfo cUnreadInfo;
            for (auto it = mapUnread.begin(); it != mapUnread.end(); it++) {
                cUnreadInfo.set_session_type(IM::BaseDefine::SESSION_TYPE_SINGLE);
                cUnreadInfo.set_session_id(atoi(it->first.c_str()));
                cUnreadInfo.set_unread_cnt(atoi(it->second.c_str()));

                lsUnreadCount.push_back(cUnreadInfo);
                nTotalCnt += cUnreadInfo.unread_cnt();
            }
        }
        else
        {
            //log("hgetall %s failed!", strKey.c_str());
        }
    }
    else
    {
        //log("no cache connection for unread");
    }
}
uint32_t CProxyModel::setProxyBUGS(uint32_t nUserId, bool bEnable, string szTarget, uint32_t nTargetType, uint32_t& nDefGroup)
{
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (pDBConn)
    {
        int nMemberOrder = -1;

        string sql = "select * from IMProxy where id=" + int2string(nUserId);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(sql.c_str());
        if (pResultSet)
        {
            while (pResultSet->Next()) {
                nMemberOrder = pResultSet->GetInt("memberorder");
            }

            delete pResultSet;
            pResultSet = NULL;
        }

        if (nMemberOrder != IM::BaseDefine::MEMBER_ADMIN)
        {
            return 1;  //非管理员，无权限设置代理
        }

        // 目标用户 权限
        string clause;
        if (nTargetType == 1){
            clause = "name=" + szTarget;
        }else if(nTargetType == 2){
            clause = "id=" + szTarget;
        }

        sql = "select * from IMUser where " + clause;
        pResultSet = pDBConn->ExecuteQuery(sql.c_str());
        if (pResultSet)
        {
            int nTargetOrder = -1;

            while (pResultSet->Next()) {
                nTargetOrder = pResultSet->GetInt("memberorder");
            }

            delete pResultSet;
            pResultSet = NULL;

            if (nTargetOrder != IM::BaseDefine::MEMBER_COMMON && bEnable == true)
            {
                return 2;  //该用户已是代理或管理员
            }else if (nTargetOrder != IM::BaseDefine::MEMBER_PROXY && bEnable == false){
                return 3;  //该用户不是代理无需取消
            }
        }

        // 执行赋权
        uint32_t nOrder = bEnable?1:0;
        sql = "update IMUser set memberorder=" + int2string(nOrder) + " where " + clause;
        pDBConn->ExecuteUpdate(sql.c_str());

        if (bEnable)
        {
            // 创建代理默认PK群
            set<uint32_t> members;
            members.insert(nUserId);
            CGroupModel::getInstance()->createGroup(nUserId, "PK10", "", IM::BaseDefine::GROUP_TYPE_PK10, members);
        }else{
            // 禁用代理时 禁用PK10群
            std::vector<IM::BaseDefine::GroupVersionInfo> lsGroup;
            CGroupModel::getInstance()->fetUBGrpinfo(
                nUserId, lsGroup, IM::BaseDefine::GROUP_TYPE_PK10);

            //for(auto it=lsGroup.begin(); it!=lsGroup.end(); ++it)
            for (auto const& v : lsGroup)
            {
                list<uint32_t> lsCurUserId;

                CGroupModel::getInstance()->removeGroup(nUserId, v.group_id(), lsCurUserId);
            }
            
        }
        return 0;
    }
}

// @nProxyId: 代理ID
uint32_t CProxyModel::bind(uint32_t nUserId, string szTarget, IM::Proxy::UserOperate operate, uint32_t &nProxyId)
{
    Assert(UserOperate_IsValid(operate), "error operate type");
    
    auto const pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
    if (pDBConn)
    {
        string szProxyName;
        
        // 校验代理是否存在
        string sql = "select * from IMProxy where name='" + szTarget + "'" + "or id=" + szTarget;
        CResultSet* pResultSet = pDBConn->ExecuteQuery(sql.c_str());
        if (pResultSet)
        {
            while (pResultSet->Next()) {
                szProxyName = pResultSet->GetString("name");
                nProxyId = pResultSet->GetInt("id");
            }

            delete pResultSet;
            pResultSet = NULL;
        }

        // 该用户不是代理
        if (szProxyName.empty()){
            return 2;
        }

        // 校验当前是否已经绑定代理
        uint32_t nCurBinded = 0;

        sql = "select * from IMUser where id=" + int2string(nUserId);
        pResultSet = pDBConn->ExecuteQuery(sql.c_str());

        if (pResultSet)
        {
            while (pResultSet->Next()){
                nCurBinded = pResultSet->GetInt("binded");
            }
            delete pResultSet;
            pResultSet = NULL;
        }

        // 重复操作
        if ((nCurBinded == 0 && operate == IM::Proxy::USER_OPERATE_UNBIND) ||
        (nCurBinded != 0 && operate == IM::Proxy::USER_OPERATE_BIND))
        {
            return 1;
        }

        uint64_t t_now = (uint64_t)time(NULL);
        // 绑定/解绑
        if (operate == IM::Proxy::USER_OPERATE_UNBIND)
        {   
            sql = "update IMUser set binded=0,updated=" + int2string(t_now) + " where id=" + int2string(nUserId);
        }else if (operate == IM::Proxy::USER_OPERATE_BIND){
            sql = "update IMUser set binded=" + int2string(nProxyId) + ", updated=" + int2string(t_now) + " where id=" + int2string(nUserId);
        }

        bool bRet = pDBConn->ExecuteUpdate(sql.c_str());
        if (!bRet)
        {
            //log("[bind] db error.");
        }
        return 0;
    }

    return INVALID_UINT32;
}


uint32_t CProxyModel::bind2Group(uint32_t nUserId, string szTarget, IM::Proxy::UserOperate operate, uint32_t &nProxyId, std::set<uint32_t>& realGids)
{
    uint32_t bBind = bind(nUserId, szTarget, operate, nProxyId);
    if (bBind != 0){
        return bBind;
    }

    //解绑
    if (USER_OPERATE_BIND != operate){
        return 0;
    }

    // 绑定代理，默认拉到代理创建的群
    std::set<uint32_t> gids;
    GroupModel::getInstance()->fetOGrpids(nProxyId, gids, 1000);
    if (!gids.empty()) 
    {
        std::set<uint32_t> const m{ nUserId };

        for (auto const& gid : gids) {
            if (proto::code::Generic::Valid == GroupModel::getInstance()->validProxyAutoAdd(gid)) {
                Warning(__func__ << " auto add " << nUserId << " to proxy " << nProxyId << " group " << gid)

                gids.insert(gid);
                GroupModel::getInstance()->addmem(gid, m);
            }
        }
    }
    return bBind;
}

#if 0
uint32_t CProxyModel::users(uint32_t nUserId, uint32_t nLastUpd, uint32_t nTag, list<IM::BaseDefine::UserInfo>& lsUsers)
{
    auto const pDBConn = CDBManager::getInstance()->getdbconn(
        "teamtalk_slave");
    if (pDBConn) {

        string listSql;

        if (TYPE_USER_LIST_PROXY == nTag)
        {
            //我是代理的下线，代理ID， 代理用户ID
            listSql = "select * from IMUser where name=(select name from IMProxy where id=(select binded from IMUser where id=" + int2string(nUserId) + "))";
        }else if (TYPE_USER_LIST_FRIEND == nTag)
        {
            // 好友列表
            list<uint32_t> friendIds;

            string sql = "select toId from IMFriend where fromId=" + int2string(nUserId) + " and status=0";
            CResultSet* pResultSet = pDBConn->ExecuteQuery(sql.c_str());
            if (pResultSet)
            {
                while (pResultSet->Next()){
                    friendIds.push_back(pResultSet->GetInt("toId"));
                }

                delete pResultSet;
                pResultSet = NULL;
            }

            sql = "select fromId from IMFriend where toId=" + int2string(nUserId) + " and status=0";
            pResultSet = pDBConn->ExecuteQuery(sql.c_str());
            if (pResultSet)
            {
                while (pResultSet->Next()){
                    friendIds.push_back(pResultSet->GetInt("fromId"));
                }

                delete pResultSet;
                pResultSet = NULL;
            }

            string strClause;
            bool bFirst = true;

            list<uint32_t>::iterator it = friendIds.begin();
            for (; it!=friendIds.end(); ++it)
            {
                if (bFirst){
                    bFirst = false;
                    strClause = int2string(*it);
                }else{
                    strClause += ("," + int2string(*it));
                }
            }

            if (!strClause.empty())
            {
                listSql = "select * from IMUser where id=" + strClause;
            }

            //临时查询所有用户返回给客户端
            listSql = "select * from IMUser where id!=" + int2string(nUserId);
        }else if (TYPE_USER_LIST_SUBLINE == nTag)
        {
            // 我作为代理 找我的下线
            listSql = "select * from IMUser where binded=(select id from IMProxy where name=(select name from IMUser where id=" + int2string(nUserId) + "))";
        }else if (TYPE_USER_LIST_NEWADDER == nTag)
        {
            // 新加我为好友的用户列表
            listSql = "select * from IMUser where id=(select fromId from IMFriend where (toId=" + int2string(nUserId) + " and status=1 and updated>" + int2string(nLastUpd) + "))";
        }

        lsUsers.clear();

        if (!listSql.empty())
        {
            CResultSet* pResultSet = pDBConn->ExecuteQuery(listSql.c_str());
            if (pResultSet)
            {
                while (pResultSet->Next()){
                    IM::BaseDefine::UserInfo User;

                    User.set_user_id(pResultSet->GetInt("id"));
                    User.set_user_nick_name(pResultSet->GetString("nick"));
                    User.set_user_gender(pResultSet->GetInt("sex"));
                    User.set_avatar_url(pResultSet->GetString("avatar"));
                    User.set_user_real_name(pResultSet->GetString("name"));
                    User.set_member_order(pResultSet->GetInt("memberorder"));
                    User.set_sign_info(pResultSet->GetString("sign_info"));
                    User.set_birthday(pResultSet->GetInt("birthday"));
                    
                    lsUsers.push_back(User);
                }

                delete pResultSet;
                pResultSet = NULL;
            }
        }
        return 0;
    }
    return 65535;
}
#endif

uint32_t CProxyModel::redpack(uint32_t nFromId, uint32_t nToId, IM::BaseDefine::MsgType nMsgType, uint32_t nCreateTime,
                uint32_t currency, uint32_t number, uint32_t openRule, string& szText, uint32_t& nMsgId)
{
    DBUserInfo_t UserInfo;
    CUserModel::getInstance()->getUser(nFromId, UserInfo);

    if (nFromId == nToId)
    {
        return 1;   // 不能给自己发送红包
    }

    if (UserInfo.nCurrency < currency)
    {
        return 2;   // 余额不足
    }

    if (IM::BaseDefine::MsgType_IsValid(nMsgType))
    {
        return INVALID_UINT32;
    }

    // 红包消息ID
    uint32_t nNow = (uint32_t)time(NULL);

    CMessageModel* pMsgModel = CMessageModel::getInstance();
    CGroupMessageModel* pGroupMsgModel = CGroupMessageModel::getInstance();

    if(nMsgType == IM::BaseDefine::MSG_TYPE_GROUP_REDPACK) {
        // 群红包
        CGroupModel* pGroupModel = CGroupModel::getInstance();
        if (pGroupModel->isValidateGroupId(nToId) && pGroupModel->isInGroup(nFromId, nToId))
        {
            // 判断是否存在会话，不存在则创建 得到会话的消息ID
            uint32_t nSessionId = CSessionModel::getInstance()->getSessionId(nFromId, nToId, IM::BaseDefine::SESSION_TYPE_GROUP, false);
            if (0 == nSessionId) {
                nSessionId = CSessionModel::getInstance()->addSession(nFromId, nToId, IM::BaseDefine::SESSION_TYPE_GROUP);
            }

            if(nSessionId != 0)
            {
                nMsgId = pGroupMsgModel->getMsgId(nToId);
                if (nMsgId != 0) {
                    pGroupMsgModel->sendRedpack(nFromId, nToId, nMsgType, nCreateTime, nMsgId, currency, number, openRule, szText);
                    CSessionModel::getInstance()->updateSession(nSessionId, nNow);
                }
            }
        }
        else
        {
            //log("invalid groupId. fromId=%u, groupId=%u", nFromId, nToId);
        }
    }else if (nMsgType== IM::BaseDefine::MSG_TYPE_SINGLE_REDPACK)
    {
        // 好友红包
        uint32_t nSessionId = CSessionModel::getInstance()->getSessionId(nFromId, nToId, IM::BaseDefine::SESSION_TYPE_SINGLE, false);
        if (0 == nSessionId) {
            nSessionId = CSessionModel::getInstance()->addSession(nFromId, nToId, IM::BaseDefine::SESSION_TYPE_SINGLE);
        }

        uint32_t nPeerSessionId = CSessionModel::getInstance()->getSessionId(nToId, nFromId, IM::BaseDefine::SESSION_TYPE_SINGLE, false);
        if(0 ==  nPeerSessionId)
        {
            nPeerSessionId = CSessionModel::getInstance()->addSession(nToId, nFromId, IM::BaseDefine::SESSION_TYPE_SINGLE);
        }
        
        uint32_t nRelateId = CRelationModel::getInstance()->getRelationId(nFromId, nToId, true);
        if(nSessionId != 0 && nRelateId != 0)
        {
            nMsgId = pMsgModel->getMsgId(nRelateId);
            if(nMsgId != 0)
            {
                pMsgModel->sendRedpack(nRelateId, nFromId, nToId, nMsgType, nCreateTime, nMsgId, currency, number, openRule, szText);
                CSessionModel::getInstance()->updateSession(nSessionId, nNow);
                CSessionModel::getInstance()->updateSession(nPeerSessionId, nNow);
            }
            else
            {
                //log("msgId is invalid. fromId=%u, toId=%u, nRelateId=%u, nSessionId=%u, nMsgType=%u", nFromId, nToId, nRelateId, nSessionId, nMsgType);
            }
        }
        else{
            //log("sessionId or relateId is invalid. fromId=%u, toId=%u, nRelateId=%u, nSessionId=%u, nMsgType=%u", nFromId, nToId, nRelateId, nSessionId, nMsgType);
        }
    }

    // 红包发送成功 扣除金额
    if (nMsgId != 0)
    {
        list<IM::BaseDefine::IMCashFlow> lsFlow;

        IM::BaseDefine::IMCashFlow flow;
        flow.set_cash(-1 * currency);
        flow.set_consume(IM::BaseDefine::CASH_CONSUME_REDPACK_SEND);

        Json::Value v;
        v["from"] = nFromId;
        v["to"] = nToId;
        v["created"] = nCreateTime;
        v["currency"] = currency;
        v["number"] = number;
        v["szText"] = szText;

        flow.set_detail(v.toStyledString());

        lsFlow.push_back(flow);

        CUserModel::getInstance()->updateUserCurrency(nFromId, UserInfo.nCurrency - currency, lsFlow);
        return 0;
    }
    return INVALID_UINT32;
}


uint32_t CProxyModel::claim_rp(uint32_t nUserId, uint32_t nSessionId, uint32_t nPackId, list<UserClaimed>& lsHistory, uint32_t &nSender, uint32_t &nOpenRule, uint32_t &nTotalNum)
{
    uint32_t nRet = 0;
    IM::BaseDefine::MsgInfo msg;

    list<uint32_t> lsId;
    list<IM::BaseDefine::MsgInfo> lsMsg;

    lsId.push_back(nPackId);

    CGroupModel* pGroupModel = CGroupModel::getInstance();
    if (pGroupModel->isValidateGroupId(nSessionId) && pGroupModel->isInGroup(nUserId, nSessionId))
    {
        CGroupMessageModel::getInstance()->getMsgByMsgId(nUserId, nSessionId, lsId, lsMsg);
    }else{
        CMessageModel::getInstance()->getMsgByMsgId(nUserId, nSessionId, lsId, lsMsg);
    }

    if (msg.msg_id() == nPackId)
    {
        Json::Reader reader;
        Json::Value redpack;
        if (!reader.parse(msg.msg_data().c_str(), redpack)){
            return nRet;   // 红包数据错误
        }

        // 红包数据
        uint32_t t_currency = redpack["t_currency"].asUInt();
        uint32_t t_number = redpack["t_number"].asUInt();
        uint32_t t_openrule = redpack["t_openrule"].asUInt();
        string t_text = redpack["t_text"].asString();
        uint32_t t_msgtype = redpack["t_msgtype"].asUInt();
        uint32_t t_session = redpack["t_session"].asUInt();
        uint32_t t_sender = redpack["t_sender"].asUInt();

        auto pDBConn = CDBManager::getInstance()->getdbconn("teamtalk_slave");
        if (pDBConn) {
            // 红包已经领取了的情况
            uint32_t d_number, d_curr;
            d_number = d_curr = 0;

            string sql = "select * from IMRPClaimRecord where pack_id=" + int2string(nPackId);
            CResultSet* pResultSet = pDBConn->ExecuteQuery(sql.c_str());
            if (pResultSet)
            {
                while (pResultSet->Next()){
                    UserClaimed c;
                    c.nUserId = pResultSet->GetInt("user_id");
                    c.nClaimed = pResultSet->GetInt("currency");

                    d_curr += c.nClaimed;
                    d_number += 1;

                    lsHistory.push_back(c);
                }

                delete pResultSet;
                pResultSet = NULL;
            }

            // 有剩余金额和 个数
            if (d_curr < t_currency && d_number < t_number)
            {
                uint32_t nClaim = 0;

                // 最后一个红包，或者 好友红包
                if ((t_msgtype == IM::BaseDefine::MSG_TYPE_GROUP_REDPACK && t_number - d_number == 1) ||
                    (t_msgtype == IM::BaseDefine::MSG_TYPE_SINGLE_REDPACK && t_session == nUserId))
                {
                    nClaim = t_currency - d_curr;
                }else{
                    if (t_openrule == 1){
                        nClaim = (t_currency - d_curr) * 1.0 / (t_number - d_number);
                    }else{
                        uint32_t min_curr = max(t_currency * 1.0 / pow(t_number, 2), 0.01);  //保底值
                        uint32_t max_rand = (t_currency - d_curr) * 1.0 / (t_number - d_number) * 2;

                        nClaim = rand() % max_rand + min_curr;
                    }
                }

                uint32_t t_now = time(NULL);
                sql = "insert into IMRPClaimRecord(pack_id, user_id, currency, last_num, last_curr, created) values (" + int2string(nPackId) + ", " + 
                            int2string(nUserId) + ", " + int2string(nClaim) + ", " + int2string(t_number - d_number - 1) + ", " + 
                            int2string(t_currency - d_curr - nClaim) + ", " + int2string(t_now) + ")";
                pDBConn->ExecuteUpdate(sql.c_str());
                // FIXED: release captured first => NO USE ANYMORE
                pDBConn = nullptr;

                UserClaimed c;
                c.nUserId = nUserId;
                c.nClaimed = nClaim;
                lsHistory.push_back(c);

                // 抢红包成功，资金入账
                DBUserInfo_t UerInfo;
                CUserModel::getInstance()->getUser(nUserId, UerInfo);

                list<IM::BaseDefine::IMCashFlow> lsFlow;

                IM::BaseDefine::IMCashFlow flow;
                flow.set_cash(nClaim);
                flow.set_consume(IM::BaseDefine::CASH_CONSUME_REDPACK_GET);

                Json::Value v;
                v["from"] = t_sender;
                v["to"] = nUserId;
                v["created"] = t_now;
                v["currency"] = nClaim;
                v["number"] = t_number;
                v["szText"] = t_text;

                flow.set_detail(v.toStyledString());

                lsFlow.push_back(flow);
                
                uint32_t nCurrency = UerInfo.nCurrency + nClaim;
                CUserModel::getInstance()->updateUserCurrency(nUserId, nCurrency, lsFlow);
            }else
            {
                nRet = 1;   // 红包已领完
            }

            nSender = t_sender;
            nOpenRule = t_openrule;
            nTotalNum = t_number;
            nRet = 2;   // 领取成功
        }
    }
    return nRet;
}
