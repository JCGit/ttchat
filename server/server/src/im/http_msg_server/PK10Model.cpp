
#include "PK10Model.h"
#include <string>
#include "util.h"
#include "netlib.h"
#include "DBServConn.h"
#include "IM.Group.pb.h"
#include "IM.Server.pb.h"

PK10Model* PK10Model::m_pInstance = NULL;
bool PK10Model::m_bInitFromDB = false;

using namespace HTTP;
PK10Model::PK10Model()
{
    m_ruleCache.clear();
    m_resetRuleCache.clear();
    m_placeCache.clear();
    m_userGroup.clear();
    m_goldRecord.clear();
    m_openHistory.clear();
}

PK10Model::~PK10Model()
{

}

PK10Model* PK10Model::getInstance()
{
	if (!m_pInstance) {
		m_pInstance = new PK10Model();
	}

	return m_pInstance;
}

//@to_open 即将开奖的时间
//@RETURN 当前时间
time_t nearest_to_open(time_t& to_open)
{
    time_t t;
    time(&t);

    time_t t_now = t;

    // 开奖时间段 
    struct tm q = *localtime(&t);
    if (q.tm_min % 5 == 2){
        t = t + 5 * 60;
    }

    while(true)
    {
        //这个地方不能用指针
        struct tm p = *localtime(&t);
        if (p.tm_min % 5 == 2)
        {
            to_open = t - t % 60;
            break;
        }else{
            t = t + 1 * 60;
        }
    }
    return t_now;
}

// 9:02-23:52 5min/次
void pk10_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    PK10Model* pModel = PK10Model::getInstance();

    time_t t_toOpen;
    time_t t_now = nearest_to_open(t_toOpen);

    struct tm p = *localtime(&t_now);
    if (p.tm_hour < 9)
    {
        pModel->setStatus(E_PAN_TING);
        return;
    }

    pModel->setStatus(E_PAN_KAI);
    pModel->checkClose();
}

void pk10_initDB_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    PK10Model* pModel = PK10Model::getInstance();
    pModel->checkInitialize();
}

void PK10Model::checkInitialize()
{
    if (m_bInitFromDB)
    {
        netlib_delete_timer(pk10_initDB_callback, NULL);
    }else{
        CDBServConn* pConn = get_db_serv_conn();
        if (pConn)
        {
            // 请求DB 管理员ID
            IM::Group::IMGroupPK10RuleReq msg;
            CImPdu pdu;
            pdu.SetPBMsg(&msg);
            pdu.SetServiceId(IM::BaseDefine::SID_GROUP);
            pdu.SetCommandId(IM::BaseDefine::CID_GROUP_PK10_RULES_REQUEST);
            pConn->SendPdu(&pdu); 

            m_bInitFromDB = true;
        }
    }
}

void PK10Model::checkClose()
{
    OPEN_MAP::iterator ut = m_openHistory.find(m_szWaitOpen);
    if (ut != m_openHistory.end()){
        return;     //已开奖
    }
    
    time_t t_toOpen;
    time_t t_now = nearest_to_open(t_toOpen);

    // log("pk10model forecast to close, leave:%ld.", t_toOpen - t_now);

    // 是否封盘
    GroupStatus* pAdminStatus, *pStatus;
    pAdminStatus = pStatus = NULL;

    Json::Value adminRule, userRule;

    PROXY_GROUP_MAP::iterator it = m_userGroup.find(m_nAdminID);
    if (it != m_userGroup.end()){
        pAdminStatus = &(it->second);
    }

    PROXY_RULE_MAP::iterator ir = m_ruleCache.find(m_nAdminID);
    if (ir != m_ruleCache.end()){
        adminRule = ir->second;
    }

    // 管理员是否封盘
    if (pAdminStatus && !pAdminStatus->closed)
    {
        if (t_now + adminRule["close"].asUInt() > t_toOpen){
            pAdminStatus->closed = true;
            log("PK10:%d admin:%d closed.", string2int(m_szWaitOpen), m_nAdminID);
        }
    }
    
    it = m_userGroup.begin();
    for (; it!=m_userGroup.end(); ++it)
    {
        if (it->first != m_nAdminID)
        {
            pStatus = &(it->second);

            ir = m_ruleCache.find(it->first);
            if (ir != m_ruleCache.end()){
                userRule = ir->second;
            }
 
            // 代理是否封盘
            if (pStatus && !pStatus->closed)
            {
                bool bFly = userRule["fly"].asBool();
                
                if ((bFly && pAdminStatus && pAdminStatus->closed) ||       //飞单 管理已停盘
                    (t_now + userRule["close"].asUInt() > t_toOpen))        //不飞单 已到封盘时间
                {
                    pStatus->closed = true;
                    log("PK10:%d proxy:%d group:%d closed, if fly:%d.", string2int(m_szWaitOpen), it->first, pStatus->group, bFly?1:0);
                }
            }
        }
    }
}

void PK10Model::initPk10Model()
{
    netlib_register_timer(pk10_timer_callback, NULL, 1000);
    netlib_register_timer(pk10_initDB_callback, NULL, 3000);

}

void PK10Model::pk10_open(string id, Json::Value& open)
{
    OPEN_MAP::iterator ut = m_openHistory.find(id);
    if (ut == m_openHistory.end())
    {
        // 新的一期 开奖
        m_openHistory.insert(std::make_pair(id, true));

        //结算，赔付
        doPk10Open(id, open);

        // 重置状态
        PROXY_RULE_MAP::iterator it = m_resetRuleCache.begin();
        for (; it!=m_resetRuleCache.end(); ++it)
        {
            uint32_t nId = it->first;

            PROXY_RULE_MAP::iterator ir = m_ruleCache.find(nId);
            if (ir != m_ruleCache.end()){
                ir->second = it->second;
            }
        }
        m_resetRuleCache.clear();

        PROXY_GROUP_MAP::iterator ir = m_userGroup.begin();
        for (; ir!=m_userGroup.end(); ++ir)
        {
            ir->second.closed = false;
        }

        m_szWaitOpen = std::to_string(std::stoll(id) + 1);
        log("%s open, start receive place.", m_szWaitOpen.c_str());
    }
}

void PK10Model::setProxyRule(uint32_t user_id, uint32_t is_admin, uint32_t group_id, Json::Value rules)
{
    if (is_admin){
        m_nAdminID = user_id;
    }

    PROXY_RULE_MAP::iterator it = m_ruleCache.find(user_id);
    if (it == m_ruleCache.end())
    {
        m_ruleCache.insert(std::make_pair(user_id, rules));
    }else{
        //封盘时间立即生效
        Json::Value& preRule = it->second;
        preRule["close"] = rules["close"];

        //其他设置下局生效 
        m_resetRuleCache.insert(std::make_pair(user_id, rules));
    }

    PROXY_GROUP_MAP::iterator it2 = m_userGroup.find(user_id);
    if (it2 == m_userGroup.end()){
        GroupStatus status;
        status.group = group_id;
        status.closed = false;

        m_userGroup.insert(std::make_pair(user_id, status));
    }

    m_bInitFromDB = true;

    log("pk10 user:%d set group:%d rule.", user_id, group_id);
}

uint32_t PK10Model::place(uint32_t user, uint32_t group, uint32_t time, uint32_t rule, list<COMB>& lsComb)
{
    uint32_t proxy = getGroupCreator(group);
    if (proxy == 0){
        return 1;
    }

    uint32_t nRet = 0;

    // pk10未开盘
    if (m_ePanStatus != E_PAN_KAI){
        nRet = 2;
    }else{

        // 群是否已封盘
        PROXY_GROUP_MAP::iterator it = m_userGroup.find(user);
        if (it->second.closed){
            nRet = 3;
        }else{

            list<COMB>::iterator lt = lsComb.begin();
            for (; lt!=lsComb.end(); ++lt)
            {
                COMB& comb = *lt;
                UserPlace place;

                place.user = user;
                place.group = group;
                place.creator = proxy;
                place.rule = (eRule)rule;
                place.main = comb.main;
                place.sub = comb.sub;
                place.gold = comb.gold;
                
                GROUP_PLACE_MAP::iterator ir = m_placeCache.find(group);
                if (ir == m_placeCache.end())
                {
                    list<UserPlace> lsPlace;
                    lsPlace.push_back(place);

                    m_placeCache.insert(std::make_pair(group, lsPlace));
                }else{
                    list<UserPlace>& lsPlace = ir->second;
                    lsPlace.push_back(place);
                }

                log("pk10 receive user:%d place main:%d sub:%d gold:%d.", user, place.main, place.sub, place.gold);
            }
        }
    }
    return nRet;
}

uint32_t PK10Model::getGroupCreator(uint32_t nGroupId)
{
    PROXY_GROUP_MAP::iterator it = m_userGroup.begin();
    for (; it!=m_userGroup.end(); ++it)
    {
        if (it->second.group == nGroupId){
            return it->first;
        }
    }
    return 0;
}

void PK10Model::doPk10Open(string id, Json::Value open)
{
    log("%s start conclude.", id.c_str());

    uint32_t nAdminGroupID = 0;
    PROXY_GROUP_MAP::iterator it = m_userGroup.find(m_nAdminID);
    if (it != m_userGroup.end()){
        nAdminGroupID = it->second.group;
    }

    // 每个群的每次下注
    GROUP_PLACE_MAP::iterator ir = m_placeCache.begin();
    for(; ir != m_placeCache.end(); ++ir)
    {
        // 群所有下注
        list<UserPlace>& lsGroupPlaces = ir->second;
        // 群创建者
        uint32_t nGroupCreator = getGroupCreator(ir->first);
        // 群规则
        Json::Value jGroupRule = m_ruleCache.find(nGroupCreator)->second;

        log("PK10 QI:%s RESULT:", id.c_str());
        // 每次下注
        list<UserPlace>::iterator pt = lsGroupPlaces.begin();
        for (; pt != lsGroupPlaces.end(); ++pt)
        {
            UserPlace& place = *pt; 

            CheckOpen cOpen(open, jGroupRule, place);
            cOpen.check(place.result);

            record(id, place);
            log("user:%d in group:%d place main:%d sub:%d gold:%d result:%d.", place.user, place.group, place.main, place.sub, place.gold, place.result);
        }
    }

    //当前期 结算
    list<UserPlace>& lsUserPlace = m_goldRecord[id];
    list<UserPlace>::iterator ut = lsUserPlace.begin();
    for (; ut!=lsUserPlace.end(); ++ut)
    {
        UserPlace& place = *ut;

        PROXY_RULE_MAP::iterator i = m_ruleCache.find(place.creator);
        if (i != m_ruleCache.end())
        {
            Json::Value& r = i->second;

            uint32_t other_id = r["flyed"].asUInt()==1?m_nAdminID:place.creator;
            insertConsume(other_id, place);
        }
    }

    IM::Server::IMCashConsumeInsertReq msg;

    USER_CONSUME_MAP::iterator ct = m_consumeCache.begin();
    for (; ct!=m_consumeCache.end(); ++ct)
    {
        ConsumeItem& consume = ct->second;
        list<UserPlace>& lsPlace = consume.lsPlaces;

        IM::BaseDefine::IMUserCacheFlow* pUserFlow = msg.add_user_flow();
        pUserFlow->set_user_id(ct->first);

        list<UserPlace>::iterator lt = lsPlace.begin();
        for(; lt!=lsPlace.end(); ++lt)
        {
            UserPlace& p = *lt;

            IM::BaseDefine::IMCashFlow* pFlow = pUserFlow->add_flow();

            pFlow->set_cash(p.result);
            pFlow->set_consume(IM::BaseDefine::CASH_CONSUME_PK10_PLACE);

            Json::Value v;

            v["user"] = p.user;
            v["group"] = p.group;
            v["rule"] = p.rule;
            v["main"] = p.main;
            v["sub"] = p.sub;
            v["gold"] = p.gold;
            v["result"] = p.result;

            string sz = v.toStyledString();
            pFlow->set_detail(sz);
        }
    }

    CDBServConn* pConn = get_db_serv_conn();
    if (pConn)
    {
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(IM::BaseDefine::SID_PROXY);
        pdu.SetCommandId(IM::BaseDefine::CID_PROXY_CASH_CONSUME_REQUEST);
        pConn->SendPdu(&pdu); 
    }
}

void PK10Model::insertConsume(uint32_t user_id, UserPlace& place)
{
    USER_CONSUME_MAP::iterator it = m_consumeCache.find(user_id);
    if (it == m_consumeCache.end())
    {
        ConsumeItem consume;

        consume.total = place.result;
        consume.lsPlaces.push_back(place);
        m_consumeCache.insert(std::make_pair(user_id, consume));
    }else{

        ConsumeItem& consume = it->second;

        consume.total += place.result;
        consume.lsPlaces.push_back(place);
    }
}

void PK10Model::record(string id, UserPlace& place)
{
    USER_GOLD_RECORD::iterator it = m_goldRecord.find(id);
    if (it == m_goldRecord.end())
    {
        list<UserPlace> lsPlace;
        lsPlace.push_back(place);

        m_goldRecord.insert(std::make_pair(id, lsPlace));
    }else{
        list<UserPlace>& lsPlace = it->second;
        lsPlace.push_back(place);
    }
}

CheckOpen::CheckOpen(Json::Value jOpen, Json::Value jRule, UserPlace place):
    m_jOpen(jOpen), m_jRule(jRule), m_stPlace(place)
{
    string szOpen = m_jOpen.get("number", "").asString();

    char buff[64] = {0};
    strcpy(buff, szOpen.c_str());

    char* pch = strtok(buff, ",");
    while (pch != NULL)
    {
        m_lsNumber.push_back(std::stoi(pch));
        pch = strtok(NULL, ",");
    }
}

CheckOpen::~CheckOpen()
{

}

int CheckOpen::check(int32_t& nResult)
{
    int iRet = 0;
    uint32_t odd = 0;
    eRule rule = m_stPlace.rule;
    uint32_t main = m_stPlace.main;
    uint32_t sub = m_stPlace.sub;

    switch (rule)
    {
        case R_DA_XIAO_DAN_SHUANG:
            {
                odd = odd_dxds(main, sub);
                iRet = check_dxds(main, sub);
            }
            break;
        case R_MINGCI:
            {
                odd = odd_mc(main, sub);
                iRet = check_mc(main, sub);
            }
            break;
        case R_LONG_HU:
            {
                odd = odd_lh(main, sub);
                iRet = check_lh(main, sub);
            }
            break;
        case R_ZHUANG_XIAN:
            {
                odd = odd_zx(main, sub);
                iRet = check_zx(main, sub);
            }
            break;
        case R_GUAN_YA_HAO:
            {
                odd = odd_gyh(main, sub);
                iRet = check_gyh(main, sub);
            }
            break;
        case R_TEMA:
            {
                odd = odd_tm(main, sub);
                iRet = check_tm(main, sub);
            }
            break;
        case R_GUAN_YA_HE:
            {
                odd = odd_gyh_s(main, sub);
                iRet = check_gyh_s(main, sub);
            }
            break;
        default:
            break;
    }

    int32_t iOpe = iRet?1:-1;
    nResult = iOpe * odd;
    return iOpe;
}
