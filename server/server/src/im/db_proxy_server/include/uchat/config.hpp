#pragma once
#define UCHAT_USE_FULL_OLD_SHIELD 0
#define UCHAT_HAS_GLOBAL_SHIELD_DURATION 0
#define UCHAT_HAS_STRICT_ROOT_USER 0
namespace uchat
{
namespace conf
{
#if UCHAT_HAS_GLOBAL_SHIELD_DURATION
constexpr int32_t kGlobalShieldDurationBegin = 22;/// [0, 24]
constexpr int32_t kGlobalShieldDurationEnd = 7;/// next day [0, 24]
#endif
#if UCHAT_HAS_STRICT_ROOT_USER
constexpr uint32_t kRootUid = 0u;
#endif
constexpr char const* kDBName = "teamtalk";
constexpr char const* kFriendTableName = "IMFriend";
}
}
