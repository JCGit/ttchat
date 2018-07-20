/*
 * PK10Model.h
 *
 */

#ifndef PK10_MODEL_H_
#define PK10_MODEL_H_

#include <string>
#include <list>

#include "util.h"
#include "json/json.h"
#include "public_define.h"

using namespace std;

enum ePanStatus
{
    E_PAN_TING  = 0,    // 停盘
    E_PAN_KAI   = 1,    // 开盘
};

enum eRule
{
    R_DA_XIAO_DAN_SHUANG = 1,
    R_MINGCI = 2,
    R_LONG_HU = 3,
    R_ZHUANG_XIAN = 4,
    R_GUAN_YA_HAO = 5,
    R_TEMA = 6,
    R_GUAN_YA_HE = 7,
    R_MAX = 8,
};

// 用户单次下注（用户一次下注可能包含多注）
typedef struct 
{
    uint32_t user;
    uint32_t group;
    uint32_t creator;
    eRule   rule;
    uint32_t main;
    uint32_t sub;
    uint32_t gold;
    int     result; // 盈亏
}UserPlace;


//@uint32_t : 管理/代理ID
//@Json::Value : 群规则
typedef map<uint32_t, Json::Value> PROXY_RULE_MAP;

//@uint32_t : 群
//@set<UserPlace> : 用户的每一注 列表
typedef map<uint32_t, list<UserPlace>> GROUP_PLACE_MAP;

//群状态
typedef struct
{
    uint32_t group;
    bool closed;

}GroupStatus;
//@uint32_t : 管理/代理ID
//@uint32_t : 群信息
typedef map<uint32_t, GroupStatus> PROXY_GROUP_MAP;

//@string : 开奖期号
//@bool : 是否开奖
typedef map<string, bool> OPEN_MAP;

// 记录
//@uint32_t : 期号
//@UserPlace : 每一期的下注列表
typedef map<string, list<UserPlace>> USER_GOLD_RECORD;

// 结算
typedef struct 
{
    int32_t total;
    list<UserPlace> lsPlaces;
}ConsumeItem;

//@uint32_t : 用户ID
//@ConsumeItem : 参与该用户结算的账单
typedef map<uint32_t, ConsumeItem> USER_CONSUME_MAP;

// 每一注
typedef struct 
{
    uint32_t main; 
    uint32_t sub; 
    uint32_t gold;
}COMB;

class CheckOpen
{
public:
    CheckOpen(Json::Value jOpen, Json::Value jRule, UserPlace place);
	virtual ~CheckOpen();

    int check(int32_t& nResult);

private:
    // 大小单双
    bool check_dxds(uint32_t main, uint32_t sub)
    {
        assert(main>=1 && main<=10);
        assert(sub>=1 && sub<=8);

        uint32_t nNumber = m_lsNumber[main];

        if (sub == 1)
            return nNumber > 5;
        else if (sub == 2)
            return nNumber <= 5;
        else if (sub == 3)
            return nNumber % 2 == 1;
        else if (sub == 4)
            return nNumber % 2 == 0;
        else if (sub == 5)
            return nNumber > 5 && nNumber % 2 == 1;
        else if (sub == 6)
            return nNumber > 5 && nNumber % 2 == 0;
        else if (sub == 7)
            return nNumber <= 5 && nNumber % 2 == 1;
        else if (sub == 8)
            return nNumber <= 5 && nNumber % 2 == 0;
        return false;
    }

    // 大小单双 赔率
    uint32_t odd_dxds(uint32_t main, uint32_t sub)
    {
        assert(main>=1 && main<=10);
        assert(sub>=1 && sub<=8);

        Json::Value odd = m_jRule["odd"][0];

        if (sub == 1 || sub == 2)
            return odd["dx"].asUInt();
        else if (sub == 3 || sub == 4)
            return odd["ds"].asUInt();
        else if (sub == 5)
            return odd["dd"].asUInt();
        else if (sub == 6)
            return odd["ds"].asUInt();
        else if (sub == 7)
            return odd["xd"].asUInt();
        else if (sub == 8)
            return odd["xs"].asUInt();
        return 0;
    }

    // 名次
    bool check_mc(uint32_t main, uint32_t sub)
    {
        assert(main>=1 && main<=10);
        assert(sub>=1 && sub<=10);

        return m_lsNumber[main] == sub;
    }

    // 名次 赔率
    uint32_t odd_mc(uint32_t main, uint32_t sub)
    {
        assert(main>=1 && main<=10);
        assert(sub>=1 && sub<=10);

        Json::Value odd = m_jRule["odd"][0];
        return odd["ch"].asUInt();
    }

    // 龙虎  
    bool check_lh(uint32_t main, uint32_t sub)
    {
        assert(main==1 || main==2);
        assert(sub>=1 && sub<=5);

        uint32_t nFir = 1;
        uint32_t nLast = 10;

        for (int i=1, j=10; i<j; ++i, --j)
        {
            if (sub == i)
            {
                uint32_t nRes;

                if (m_lsNumber[i] > m_lsNumber[j])
                    nRes = 1;
                else 
                    nRes = 2;

                return main==nRes;
            }
        }
    }

    // 龙虎 赔率
    uint32_t odd_lh(uint32_t main, uint32_t sub)
    {
        assert(main==1 || main==2);
        assert(sub>=1 && sub<=5);

        Json::Value odd = m_jRule["odd"][0];
        return odd["lh"].asUInt();
    }

    // 庄闲  
    bool check_zx(uint32_t main, uint32_t sub)
    {
        assert(main==1 || main==2);
        assert(sub==1 && sub==2);

        uint32_t nFir = m_lsNumber[1];
        uint32_t nSec = m_lsNumber[2];

        uint32_t nRes;

        if (nFir > nSec){
            nRes = 1;
        }else{
            nRes = 2;
        }
        return nRes==sub;
    }

    // 庄闲 赔率
    uint32_t odd_zx(uint32_t main, uint32_t sub)
    {
        assert(main==1 || main==2);
        assert(sub==1 && sub==2);

        Json::Value odd = m_jRule["odd"][1];    
        
        if (sub == 1)
            return odd["n_z"].asUInt();
        else if(sub == 2)
            return odd["n_x"].asUInt();
    }

     // 冠亚号  
    bool check_gyh(uint32_t main, uint32_t sub)
    {
        assert(main==1 || main==2);
        assert(sub>=1 && sub<=10);

        uint32_t nFir = m_lsNumber[1];
        uint32_t nSec = m_lsNumber[2];

        uint32_t n1 = sub & 0x01;
        uint32_t n2 = sub & 0x10;

        if ((n1 == nFir && n2 == nSec) ||
            (n2 == nFir && n1 == nSec))
        {
            return true;
        }
        return false;
    }

    // 冠亚号 赔率
    uint32_t odd_gyh(uint32_t main, uint32_t sub)
    {
        assert(main==1 || main==2);
        assert(sub>=1 && sub<=10);

        Json::Value odd = m_jRule["odd"][1];    
        return odd["n_gyh"].asUInt();
    }

     // 特码  
    bool check_tm(uint32_t main, uint32_t sub)
    {
        assert(main==1 || main==2);
        assert(sub>=1 && sub<=7);
        uint32_t nGYH = m_lsNumber[1] + m_lsNumber[2];

        if (sub == 1)
            return nGYH>=12;
        else if (sub == 2)
            return nGYH<=11;
        else if (sub == 3)
            return nGYH % 2 == 1;
        else if (sub == 4)
            return nGYH % 2 == 0;
        else if (sub = 5)
            return nGYH>=3 && nGYH<=7;
        else if (sub = 6)
            return nGYH>=8 && nGYH<=14;
        else if (sub = 7)
            return nGYH>=15 && nGYH<=19;
        return false;
    }

    // 特码 赔率
    uint32_t odd_tm(uint32_t main, uint32_t sub)
    {
        assert(main==1 || main==2);
        assert(sub>=1 && sub<=7);

        Json::Value odd = m_jRule["odd"][2];

        if (sub == 1 || sub == 2)
            return odd["s_dx"].asUInt();
        else if (sub == 3 || sub == 4)
            return odd["s_ds"].asUInt();
        else if (sub == 5)
            return odd["s_a"].asUInt();
        else if (sub == 6)
            return odd["s_b"].asUInt();
        else if (sub == 7)
            return odd["s_c"].asUInt();
        return 0;
    }

     // 冠亚和  
    bool check_gyh_s(uint32_t main, uint32_t sub)
    {
        assert(main==1 || main==2);
        assert(sub>=3 && sub<=19);

        uint32_t nGYH = m_lsNumber[1] + m_lsNumber[2];
        return sub==nGYH;
    }

    // 冠亚和 赔率
    uint32_t odd_gyh_s(uint32_t main, uint32_t sub)
    {
        assert(main==1 || main==2);
        assert(sub>=3 && sub<=19);

        Json::Value odd = m_jRule["odd"][2];

        string key = "s_" + int2string(sub);
        return odd[key].asUInt();
    }

    Json::Value     m_jOpen;
    Json::Value     m_jRule;
    UserPlace       m_stPlace;

    vector<uint32_t>  m_lsNumber;
};

// PK10数据处理
class PK10Model
{
public:
    PK10Model();
	virtual ~PK10Model();
    
    static PK10Model* getInstance();
    
    void initPk10Model();
    
    void pk10_open(string id, Json::Value& open);

    void setAdminID(uint32_t user_id) { m_nAdminID = user_id;}
    void setProxyRule(uint32_t user_id, uint32_t is_admin, uint32_t group_id, Json::Value rules);

    void setStatus(ePanStatus status){m_ePanStatus = status;}
    void setInitialize(bool bInit){m_bInitFromDB = bInit;};

    uint32_t place(uint32_t user, uint32_t group, uint32_t time, uint32_t rule, list<COMB>& lsComb);

    void checkClose();
    void checkInitialize();

    void record(string id, UserPlace& place);


private:

    static PK10Model*	m_pInstance;
    static bool m_bInitFromDB;

    uint32_t getGroupCreator(uint32_t nGroupId);
    void doPk10Open(string id, Json::Value open);

    void insertConsume(uint32_t user_id, UserPlace& place);

    ePanStatus  m_ePanStatus;       // 盘口状态

    uint32_t m_nAdminID;            // 管理员ID 
    
    string m_szWaitOpen;            // 等待开奖期号
    OPEN_MAP        m_openHistory;      //开奖历史
    
    PROXY_RULE_MAP  m_ruleCache;            //当前期号 各个pk群规则
    PROXY_RULE_MAP  m_resetRuleCache;       //下期 预设规则

    GROUP_PLACE_MAP m_placeCache;           //当前期号  所有下注

    PROXY_GROUP_MAP m_userGroup;            //管理/代理对应的pk群

    USER_GOLD_RECORD m_goldRecord;          //记录
    USER_CONSUME_MAP m_consumeCache;        //结算
};



#endif /* PK10_MODEL_H_ */
