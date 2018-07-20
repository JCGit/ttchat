#pragma once
#define     GROUP_TOTAL_MSG_COUNTER_REDIS_KEY_SUFFIX    "_im_group_msg"
#define     GROUP_USER_MSG_COUNTER_REDIS_KEY_SUFFIX     "_im_user_group"
#define     GROUP_COUNTER_SUBKEY_COUNTER_FIELD          "count"
namespace uchat
{
constexpr uint32_t kMaxFriendCount = 1000;
constexpr uint32_t kMaxUserQueryLimit = 3000;///< Limit of one query
constexpr uint32_t kMaxGroupQueryLimit = 500;///< Limit of one query
}
