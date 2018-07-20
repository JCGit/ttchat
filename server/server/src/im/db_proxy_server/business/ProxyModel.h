#pragma once
#include <list>
#include <string>
#include <memory>
#include "util.h"
#include "ImPduBase.h"
#include "json/json.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Proxy.pb.h"
namespace uchat
{
/**
 * @enum ListFriendType
 * @code
 * const uint32_t TYPE_USER_LIST_PROXY = 1;
 * const uint32_t TYPE_USER_LIST_FRIEND = 2;
 * const uint32_t TYPE_USER_LIST_SUBLINE = 3;
 * const uint32_t TYPE_USER_LIST_NEWADDER = 4;
 * @endcode
 * @sa
 * - lsfrnds
 * - doGetUserList
 */
enum class ListFriendType {
    Proxy = 1,///< proxy
    Subline = 3,///< subline of proxy
    Friend = 2, ///< all online/offline/.. friends
    NewAddRequest = 4, ///< new friend request, (time > give time)
};
}
using namespace std;

typedef struct 
{
    uint32_t nUserId;   // 用户ID
    uint32_t nClaimed;  // 抢红包金额
}UserClaimed;
class CDBConn;
class CProxyModel {
public:
    using DBConn = CDBConn;
    using DBCP = std::shared_ptr<DBConn>;
    using ListFriendType = uchat::ListFriendType;
    using UserInfo = IM::BaseDefine::UserInfo;
    using UserOperate = IM::Proxy::UserOperate;
    /**
     * Enable/disable proxy
     * @param useId this user to set proxy
     * @param tgtUid to update proxy for this user
     * @param[out] groupId set to new create groupId when @a enable is true
     * and did set proxy
     */
    uint32_t setproxy(uint32_t userId, bool enable, uint32_t tgtUid, uint32_t& groupId) noexcept;
    /**
     * List friends
     * @bug should be: query total count fisrt, then query part one time
     * @param lstUp use only when query new friends: query: `updated` >
     * @a lstUp
     * @return 0 when success, else -errno (<0)
     */
    int32_t lsfrnds(uint32_t userId, uint32_t lstUp, ListFriendType type, std::vector<UserInfo>& users) noexcept;
    /// @return count or -errno
    int32_t lsfrnds(uint32_t userId, uint32_t lstUp, ListFriendType type, DBCP const& dbc) noexcept;
    /**
     * List friend ids for @a userId
     * @param dbc captured database connection
     * @param userId query friends of this user
     * @param[out] friendsIds result friend ids @note will be cleared first
     * @return 0 when success, else -errno
     */
    int32_t lsfrnds(uint32_t userId, std::set<uint32_t>& friendsIds, DBCP const& dbc) noexcept;
    /**
     * Friend operation
     * 好友状态
     * - 0: 正常(互为好友)
     * - 1: to未同意
     * - 2: 拒绝
     * - 3: from已删除好友
     * - 4: to已删除好友
     * - 5: 互删好友
     * @return
     * - 65535: cannot get teamtalk_slave connection
     * - 31: Current state cannot agree or decline
     * - 32: No friend id is @a nProxyId, cannot delete
     * - 33: Execute SQL fail
     * - 34: Cannot add, you have that friend
     * - 35: Invalid OP
     * - 36: Cannot add/delete/... self
     * - 37: @a proxyId invalid
     * - 0: OK
     */
    uint32_t friendop(uint32_t userId, uint32_t proxyId, UserOperate operate) noexcept;
    /// Valid user
    bool validUser(uint32_t userId) noexcept;
    /// Valid user
    bool validUser(uint32_t userId, DBCP const& dbc) noexcept;
	virtual ~CProxyModel();
	static CProxyModel* getInstance();

    void getUnreadBindCount(uint32_t nUserId, uint32_t &nTotalCnt, list<IM::BaseDefine::UnreadInfo>& lsUnreadCount);
    void getUnreadAggreFriendCount(uint32_t nUserId, uint32_t &nTotalCnt, list<IM::BaseDefine::UnreadInfo>& lsUnreadCount);
    uint32_t setProxyBUGS(uint32_t nUserId, bool bEnable, string szTarget, uint32_t nTargetType, uint32_t& nDefGroup);
    uint32_t bind(uint32_t nUserId, string szTarget, IM::Proxy::UserOperate operate, uint32_t &nProxyId);
    uint32_t bind2Group(uint32_t nUserId, string szTarget, IM::Proxy::UserOperate operate, uint32_t &nProxyId, std::set<uint32_t>& realGids);


    uint32_t users(uint32_t nUserId, uint32_t nLastUpd, uint32_t nTag, list<IM::BaseDefine::UserInfo>& lsUsers);
    uint32_t redpack(uint32_t nFromId, uint32_t nToId, IM::BaseDefine::MsgType nMsgType, uint32_t nCreateTime,
                    uint32_t currency, uint32_t number, uint32_t openRule, string& szText, uint32_t& nMsgId);
    uint32_t claim_rp(uint32_t nUserId, uint32_t nSessionId, uint32_t nPackId, list<UserClaimed>& lsHistory, uint32_t &nSender, uint32_t &nOpenRule, uint32_t &nTotalNum);

protected:
private:
	CProxyModel();
    void incBindCount(uint32_t nFromId, uint32_t nToId);
    void incFriendAggreCount(uint32_t nFromId, uint32_t nToId);

private:
	static CProxyModel*	m_pInstance;
};
