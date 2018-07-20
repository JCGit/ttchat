#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <list>
#include <map>
#include <set>
#include <memory>
#include "MessageModel.h"
#include "IM.BaseDefine.pb.h"
using namespace std;
class CDBConn;
const uint32_t MAX_UNREAD_COUNT = 100;
class CGroupModel {
public:
    using DBConn = CDBConn;
    using DBCP = std::shared_ptr<DBConn>;
    /// { groupId, GroupVersionInfo }
    using GroupVersionInfo = IM::BaseDefine::GroupVersionInfo;
    using GroupInfo = IM::BaseDefine::GroupInfo;
    using GroupIds = std::map<uint32_t, GroupVersionInfo>;
    using GroupModifyType = IM::BaseDefine::GroupModifyType;
    /**
     * 创建群
     * @param userId       创建者
     * @param gnm          群名
     * @param gat          群头像
     * @param gtp          群类型1,固定群;2,临时群
     * @param mids         群成员列表 为了踢出重复的 userId 使用 set 存储
     * @param[out] groupId
     * @note @a mids with or without @a userId
     */
    int32_t newgrp(uint32_t userId, std::string const& gnm, std::string const& gat, uint32_t gtp, std::set<uint32_t> const& mids, uint32_t& groupId) noexcept;
    /**
     * Add group member
     * @note caller is group owner or group allow caller to add member
     */
    int32_t addmem(uint32_t groupId, std::set<uint32_t> const& mids) noexcept;
    /**
     * Modify group name
     * @note caller should has modify group name permission
     * @param groupId the group to update
     * @param newName new group name
     * @param ids current group member ids
     * @return
     * - proto::code::Ok when success
     * - ...NoChange when same name or invalid update
     * - ...Exception when exception
     */
    int32_t modGrpnm(uint32_t groupId, std::string const& newName, std::set<uint32_t>& ids) noexcept;
    /**
     * Modify group announcement
     * @return
     * @note caller should has modify group name permission
     * - proto::code::Generic::Ok when success
     * - ...NoChange when same announcement -- error
     * - ...QueryError when query member list fail
     * - ...Exception when exception
     */
    int32_t modGrpa(uint32_t groupId, std::string const& newa, std::set<uint32_t>& ids) noexcept;
    /**
     * Modify group members
     * @note @a groupId should be id of valid group
     */
    int32_t modGrpms(uint32_t groupId, uint32_t opUserId, GroupModifyType type, std::set<uint32_t> const& upuids, std::set<uint32_t>& ids) noexcept;
    /**
     * Modify group member permission
     * @note caller should be group owner
     */
    int32_t modp(uint32_t groupId, std::set<uint32_t> const& mids, uint32_t permission) noexcept;
    /**
     * Set group shield for group member @a mid of @a groupId
     * @return
     * - proto::code::Generic::Ok when success
     * - ...NoChange when same shield
     * - ...Exception when exception
     */
    int32_t mods(uint32_t groupId, uint32_t mid, uint32_t shield) noexcept;
    /// 更新组最后聊天时间
    int32_t modGroupChatTime(uint32_t const groupId, DBCP const& dbc) noexcept;
    /**
     * Valid group @a groupId
     * @return
     * - proto::code::Valid when valid
     * - ...QueryError when query fail
     * - ...NoSuchGroup when group not found
     * - ...Exception when exception
     * @sa isValidateGroupId
     */
    int32_t validGroup(uint32_t groupId) noexcept;
    /**
     * Valid @a mid if is a valid member group @a groupId
     * @note @a groupId should be id of valid group
     * @return
     * - proto::code::Valid when valid
     * - ...QueryError when query fail
     * - ...NoSuchMember when member not found
     * - ...Exception when exception
     */
    int32_t validGrpm(uint32_t groupId, uint32_t mid) noexcept;
    /// Valid group admin @note @a groupId should be id of valid group
    int32_t validGrpa(uint32_t groupId, uint32_t aid, DBCP const& dbc) noexcept;
    /// Valid group admin @note @a groupId should be id of valid group
    int32_t validGrpa(uint32_t groupId, uint32_t aid) noexcept;
    /**
     * Valid @a oid if is owner of valid @a groupId
     * @note @a groupId should be id of valid group
     * @return
     * - proto::code::Valid when @a oid is owner of @a groupId and
     *   @a groupId not delete
     * - ...QueryError when query fail
     * - ...NoSuchGroup when group not found
     * - ...Exception when exception
     */
    int32_t validGrpo(uint32_t groupId, uint32_t oid, DBCP const& dbc) noexcept;
    int32_t validGrpo(uint32_t groupId, uint32_t oid)noexcept;
    /**
     * If proxy auto add is true: anyone can add other
     * If proxy auto add is false: only admin and owner can add
     * @note @a groupId should id of valid group
     */
    int32_t validAddp(uint32_t groupId, uint32_t tvid) noexcept;
    /**
     * If @a tods no admin: admin and owner can delete
     * If @a tods has admin: only owner can delete
     * @note @a groupId should id of valid group
     */
    int32_t validDelp(uint32_t groupId, uint32_t tvid, std::set<uint32_t> const& tods) noexcept;
    /**
     * Valid proxy_auto_add attribute
     * @note @a groupId should id of valid group
     */
    int32_t validProxyAutoAdd(uint32_t groupId) noexcept;
    /**
     * Fetch group ids of @a userId belongs
     * @param userId user id to query: creator and member
     * @param[out] gs output group ids
     * @param li 0 is no li
     */
    int32_t fetUGrpids(uint32_t userId, std::set<uint32_t>& gs, uint32_t li) noexcept;
    /// Fetch group ids of @a userId created
    int32_t fetOGrpids(uint32_t oid, std::set<uint32_t>& gs, uint32_t li) noexcept;
    /// Fetch user basic groups info
    int32_t fetUBGrpinfo(uint32_t userId, std::vector<GroupVersionInfo>& groups, uint32_t gtp) noexcept;
    /// Fetch basic groups info
    int32_t fetBGrpinfo(std::set<uint32_t> const& gs, std::vector<GroupVersionInfo>& groups, uint32_t gtp) noexcept;
    /// Fetch detail group info
    //int32_t fetDGrpinfo(GroupIds const& gs, std::vector<IM::BaseDefine::GroupInfo>& result) noexcept;
    /// Fetch detail group info for @a mid
    int32_t fetDGrpinfo(uint32_t mid, GroupIds const& gs, std::vector<GroupInfo>& result) noexcept;
    /**
     * Fetch all valid member user ids for group @a groupId
     * @param dbc use this conn to fetch
     * @param groupId this group
     * @note @a groupId should id of valid group
     * @param[out] result result member ids
     * @return proto::code::Ok when success
     */
    int32_t fetMemids(uint32_t groupId, std::set<uint32_t>& result, DBCP const& dbc) noexcept;
    /// @note @a groupId should id of valid group
    int32_t fetMemids(uint32_t groupId, std::set<uint32_t>& result) noexcept;
    /**
     * Fetch group members
     * @param[out] g (in/out) fetch group member user ids to @a gs according
     * @a gs
     * @note each group_id of @a gs must be exists and valid
     */
    //int32_t fetMs(std::vector<GroupInfo>& gs) noexcept;
    int32_t fetMs(uint32_t const reqUid, std::vector<GroupInfo>& gs, DBCP const& dbc) noexcept;
    /**
     * Fetch group members
     * @note each group_id of @a gi must be exists and valid
     */
    //int32_t fetMs(uint32_t groupId, uint32_t groupOwner, GroupInfo& gi) noexcept;
    int32_t fetMs(uint32_t const reqUid, uint32_t groupId, uint32_t groupOwner, GroupInfo& gi, DBCP const& dbc) noexcept;
    /// Get shield of group member @a mid of group @a groupId
    int32_t fetchS(uint32_t groupId, uint32_t mid, uint32_t& shield) noexcept;
protected:
    /// @note ONLY create(insert) IMGroup item
    bool newegrp(uint32_t userId, std::string const& gnm, std::string const& gat, uint8_t gtp, uint32_t& groupId) noexcept;
    /// @note remove group by @a groupId and ONLY remove group table item!
    bool deleteGroup(uint32_t groupId) noexcept;
    /// Delete group member
    int32_t delm(uint32_t groupId, std::set<uint32_t> const& ids, DBCP const& dbc) noexcept;
    /// Delete group member
    int32_t delm(uint32_t groupId, std::set<uint32_t> const& ids) noexcept;
    /**
     * @warning no rollback, any fail will cause once clearGroupMember try
     * - clearGroupMember(groupId);
     * - insertNewMember(groupId, mids);
     */
    bool rstGrpm(uint32_t groupId, std::set<uint32_t> const& mids) noexcept;
public:
    virtual ~CGroupModel();
    static CGroupModel* getInstance();
    /// @deprecated see newgrp
    uint32_t createGroup(uint32_t nUserId, const string& strGroupName, const string& strGroupAvatar, uint32_t nGroupType, set<uint32_t>& setMember);
    bool removeGroup(uint32_t nUserId, uint32_t nGroupId, list<uint32_t>& lsCurUserId);
    void getGroupByCreator(uint32_t nCreator, uint32_t nGroupType, list<uint32_t>& lsGroup);
    /**
     * @bug found => DEPRECATED => use fetchGroupInfo
     * @sa fetchGroupInfo
     */
    void getGroupInfo(map<uint32_t,IM::BaseDefine::GroupVersionInfo>& mapGroupId, list<IM::BaseDefine::GroupInfo>& lsGroupInfo);
    bool setPush(uint32_t nUserId, uint32_t nGroupId, uint32_t nType, uint32_t nStatus);
    void getPush(uint32_t nGroupId, list<uint32_t>& lsUser, list<IM::BaseDefine::ShieldStatus>& lsPush);
    /// @deprecated => use modifyGroupMember2 instead
    bool modifyGroupMember(uint32_t nUserId, uint32_t nGroupId, IM::BaseDefine::GroupModifyType nType, set<uint32_t>& setUserId,
                           list<uint32_t>& lsCurUserId);
    void getGroupUser(uint32_t nGroupId, list<uint32_t>& lsUserId);
    bool isInGroup(uint32_t nUserId, uint32_t nGroupId);
    void updateGroupChat(uint32_t nGroupId);
    bool isValidateGroupId(uint32_t nGroupId);
    uint32_t getUserJoinTime(uint32_t nGroupId, uint32_t nUserId);
    bool setGroupPk10Rules(uint32_t nUserId, uint32_t nGroupId, string szPkRules, uint32_t& nOption);
    void getGroupsPKRules(list<IM::BaseDefine::GroupPKInfo>& lsPkInfo);
    string getPk10DefRules();
private:
    CGroupModel();
    /// @deprecated => use newegrp instead
    bool insertNewGroup(uint32_t reqUserId, const string& gnm, const string& gat, uint32_t gtp, uint32_t memCnt, uint32_t& groupId);
    bool insertNewMember(uint32_t nGroupId,set<uint32_t>& setUsers);
        string GenerateGroupAvatar(uint32_t groupId);
    /// @deprecated => use validModifyPermission instead
    bool hasModifyPermission(uint32_t nUserId, uint32_t nGroupId, IM::BaseDefine::GroupModifyType nType);
    bool addmem(uint32_t nGroupId, set<uint32_t>& setUser,list<uint32_t>& lsCurUserId);
    bool delm(uint32_t nGroupId, set<uint32_t>& setUser,list<uint32_t>& lsCurUserId);
    void removeRepeatUser(uint32_t nGroupId, set<uint32_t>& setUser);
    void removeSession(uint32_t nGroupId, const set<uint32_t>& lsUser);
    bool incGroupVersion(uint32_t nGroupId);
    void clearGroupMember(uint32_t nGroupId);
    
    void fillGroupMember(list<IM::BaseDefine::GroupInfo>& lsGroups);

    string getGroupPKRule(uint32_t nGroupId);
        
private:
    static CGroupModel*	m_pInstance;
};
