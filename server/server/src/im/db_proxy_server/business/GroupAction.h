#pragma once
#include "ImPduBase.h"
namespace uchat
{
namespace groupdb
{
using Packet = CImPdu;
/**
 * 创建群组
 * @param pkt    收到的packet包指针
 * @param connId 该包过来的socket 描述符
 */
extern void CreateGroup(Packet* pkt, uint32_t connId) noexcept;
/**
 * 修改群成员
 * - 增加
 * - 删除
 * - Set as admin member
 * - Set as normal member
 */
extern void ModifyMember(Packet* pkt, uint32_t connId) noexcept;
extern void ModifyGroupInfo(Packet* pkt, uint32_t connId) noexcept;
extern void ModifyGroupShield(Packet* pkt, uint32_t connId) noexcept;
/// 获取群详细信息
extern void GetDetailGroupInfo(Packet* pkt, uint32_t connId) noexcept;
/// 获取群基本信息 类型 1 + 3
extern void GetBasicGroupInfo(Packet* pkt, uint32_t connId) noexcept;
}
}
namespace DB_PROXY {
#   if 0
    /// @deprecated use uchat::groupdb::GetGroupDetailInfo instead
    void getGroupInfo(CImPdu* pPdu, uint32_t conn_uuid);
    /// @deprecated use uchat::groupdb::ModifyMember instead
    void modifyMember(CImPdu* pPdu, uint32_t conn_uuid);
#   endif
    
    void setGroupPush(CImPdu* pPdu, uint32_t conn_uuid);
    
    void getGroupPush(CImPdu* pPdu, uint32_t conn_uuid);

    void setGroupPK10(CImPdu* pPdu, uint32_t conn_uuid);

    void getGroupPk10Rule(CImPdu* pPdu, uint32_t conn_uuid);

    void placeGroupPK10(CImPdu* pPdu, uint32_t conn_uuid);

};
