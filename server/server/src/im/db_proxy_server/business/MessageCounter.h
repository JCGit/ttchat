#pragma once
#include "uchat/config.hpp"
#include "ImPduBase.h"
namespace uchat
{
/// @namespace pushdb database for push
namespace pushdb
{
using Packet = CImPdu;
#if !UCHAT_USE_FULL_OLD_SHIELD
extern void HandleGetDevicesToken(Packet* pkt, uint32_t connId) noexcept;
#endif
}
}
namespace DB_PROXY {

    void getUnreadMsgCounter(CImPdu* pPdu, uint32_t conn_uuid);
    void clearUnreadMsgCounter(CImPdu* pPdu, uint32_t conn_uuid);
    
    void setDevicesToken(CImPdu* pPdu, uint32_t conn_uuid);
#   if UCHAT_USE_FULL_OLD_SHIELD
    /// @deprecated use uchat::pushdb::HandleGetDevicesToken instead
    void getDevicesToken(CImPdu* pPdu, uint32_t conn_uuid);
#   endif // if UCHAT_USE_FULL_OLD_SHIELD
};
