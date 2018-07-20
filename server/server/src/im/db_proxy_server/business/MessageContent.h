#pragma once
#include "ImPduBase.h"
namespace DB_PROXY
{
void getMessage(CImPdu* pPdu, uint32_t conn_uuid);
/// Send single or group message
void sendMessage(CImPdu* pPdu, uint32_t conn_uuid) noexcept;
void getMessageById(CImPdu* pPdu, uint32_t conn_uuid);
void getLatestMsgId(CImPdu* pPdu, uint32_t conn_uuid);
}
