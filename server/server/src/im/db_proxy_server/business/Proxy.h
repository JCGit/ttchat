/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：Login.h
 *   创 建 者：Jacey cheung
 *   创建日期：2018年3月21日
 *   描    述：
 *
 ================================================================*/

#ifndef PROXY_H_
#define PROXY_H_

#include "ImPduBase.h"

namespace DB_PROXY {

void doSetProxy(CImPdu* pPdu, uint32_t conn_uuid);
void doBindProxy(CImPdu* pPdu, uint32_t conn_uuid);

void doGetUserList(CImPdu* pPdu, uint32_t conn_uuid);

void doProcessFriend(CImPdu* pPdu, uint32_t conn_uuid);

void doSendRedpack(CImPdu* pPdu, uint32_t conn_uuid);
void doClaimRedpack(CImPdu* pPdu, uint32_t conn_uuid);

void doPK10Consume(CImPdu* pPdu, uint32_t conn_uuid);

};


#endif /* PROXY_H_ */
