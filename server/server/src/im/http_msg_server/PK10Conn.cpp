/*
 * PK10Conn.cpp
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */

#include "json/json.h"
#include "PK10Conn.h"
#include "PK10Model.h"
#include "DBServConn.h"
#include "HttpConn.h"
#include "ImPduBase.h"
#include "HttpClient.h"
#include "IM.Proxy.pb.h"
#include "ThreadPool.h"
#include "util.h"

using namespace HTTP;
static uint32_t g_rand_num[10] = 
{
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

static string g_pk10_server_addr;
static CThreadPool g_thread_pool;
static string g_latest_open = "";

static CLock							g_pk10_lock;
static std::map<string, Json::Value> 	g_pk10_cache;

static std::list<string> 	g_forecast_vec;

void randNumber(uint32_t nNumBuff[], uint32_t nNum)
{
	uint32_t temp[nNum];
	memcpy(temp, nNumBuff, sizeof(uint32_t)*nNum);

	uint32_t nRandCount = 0, nPos = 0;
	do 
	{
		nPos = rand() % (nNum - nRandCount);

		nNumBuff[nRandCount++] = temp[nPos];
		temp[nPos] = temp[nNum - nRandCount];
	}while(nRandCount < nNum);
}

string currTime()
{
	time_t t = time(NULL);

	char tmp[64] = {0};
	strftime(tmp, sizeof(tmp), "%Y%2m%d", localtime(&t));
	log("cur time:%ld.", t);
	return tmp;
} 

string lastTime()
{
	time_t t = time(NULL);
	t = t - 24 * 60 * 60 * 1000;

	char tmp[64] = {0};
	strftime(tmp, sizeof(tmp), "%Y%2m%d", localtime(&t));
	return tmp;
}

void checkForecast()
{
	static const uint32_t t_times = 178;

	if (g_forecast_vec.size() > 2)
		return;

	for (int i=0; i!=t_times; ++i)
	{
		string fore;
		for (int k=0; k!=10; ++k)	// 十个车道
		{
			randNumber(g_rand_num, 10);

			for (int j=0; j!=10; ++j) 	// 每个车道 十个数字
			{
				fore += (int2string(g_rand_num[j]));

				if (j != 9){
					fore += ",";
				}
			}

			if (k != 9){
				fore += ";";
			}
		}

		g_forecast_vec.push_back(fore);
	}
}

void CBCTask::run()
{
	// 网站拉取
	map<string, Json::Value> pullInfo;
	pull_pk10_data(nRequestLine, pullInfo);

	// 缓存
	g_pk10_lock.lock();

	map<string, Json::Value>::iterator it = pullInfo.begin();
	for (; it!=pullInfo.end(); ++it)
	{
		// 检测是否需要预测数据
		checkForecast();

		Json::Value& open = it->second;
		Json::Value pk10;

		pk10["dateline"] = open["dateline"];
		pk10["number"] = open["number"];
		pk10["forecast"] = g_forecast_vec.front();

		g_forecast_vec.pop_front();
		g_pk10_cache.insert(std::make_pair(it->first, pk10));
	}

	g_pk10_lock.unlock();

	// 当天未开奖，获取上一天最后一次开奖
	if (g_latest_open.empty())
	{
		map<string, Json::Value> fieldInfo;
		pull_pk10_data(-1, fieldInfo);
	}
}

void pk10_server_pull_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	CTask* pTask = new CBCTask(1);
	g_thread_pool.AddTask(pTask);
}

void pk10_server_init_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	CTask* pTask = new CBCTask(0);
	g_thread_pool.AddTask(pTask);

	PK10Model::getInstance()->initPk10Model();

	// 定时拉取最新开奖
	netlib_register_timer(pk10_server_pull_callback, NULL, 6 * 1000);
	// 关闭初始定时器
	netlib_delete_timer(pk10_server_init_callback, NULL);
}

void init_pk10_serv_conn(std::string server_addr)
{
	g_pk10_server_addr = server_addr;

	g_thread_pool.Init(1);

	netlib_register_timer(pk10_server_init_callback, NULL, 2 * 1000);
	// signal(SIGTERM, sig_handler);
}

void pull_pk10_data(int nLineNum, map<string, Json::Value>& fieldInfo)
{
	//拉取开奖记录
	string server_addr;
	if (nLineNum < 0){
		server_addr = g_pk10_server_addr + ("&date=" + lastTime());
	}else if (nLineNum == 0){
		server_addr = g_pk10_server_addr + ("&date=" + currTime());
	}else if (nLineNum == 1){
		server_addr = g_pk10_server_addr + ("&num=" + int2string(nLineNum));
	}

	CHttpClient httpClient;
	// log("[httpserv] http server addr:%s.", server_addr.c_str());

	string strResp;
	CURLcode nRet = httpClient.Get(server_addr, strResp);
	if(nRet != CURLE_OK)
	{
		log("http get bocai data error,errcode:%d,resp:%s,addr:%s.", nRet, strResp.c_str(), server_addr.c_str());
		return;
	} 

	Json::Reader reader;
    Json::Value value;
    if(!reader.parse(strResp, value)){
		log("json parse error");
        return;
    }else{
		Json::Value resp = value.get("status", "");
		if (resp.type() == Json::objectValue){
			log("website pull error:%s.", value.toStyledString().c_str());
			return;
		}

		Json::Value::Members mem = value.getMemberNames();
		for (int i=0; i!=mem.size(); ++i)
		{
			string key = mem.at(i);

			Json::Value item = value[key];
			fieldInfo.insert(std::make_pair(key, item));

			if (i == mem.size() - 1){
				g_latest_open = key;

				// 开奖
				if (nLineNum == 1){
					PK10Model::getInstance()->pk10_open(key, item);
				}
			}

			log("[httpserv]pull [%d/%d] pk10:%s date:%s number:%s.", i+1, mem.size(), key.c_str(), item["dateline"].asCString(), item["number"].asCString());
		}
	}
}

void pull_pk10_request(map<string, Json::Value>& fieldInfo, uint32_t nStart, uint32_t nLineNum/*=1*/)
{
	if (nStart == 0)
	{
		uint32_t nLatestOpen = string2int(g_latest_open);

		for(uint32_t n=nLatestOpen;;n--)
		{
			map<string, Json::Value>::iterator it = g_pk10_cache.find(int2string(n));
			if (it != g_pk10_cache.end()){
				fieldInfo.insert(std::make_pair(it->first, it->second));
			}else{
				break;
			}
		}

		// 预测值
		if (g_forecast_vec.size() > 0)
		{
			string key = int2string(nLatestOpen + 1);
			Json::Value value;
			value["forecast"] = g_forecast_vec.front();
			fieldInfo.insert(std::make_pair(key, value));
		}
	}else{

		uint32_t nCount = 0;
		for(uint32_t i=nStart;; ++i)
		{
			string key = int2string(i);
			
			map<string, Json::Value>::iterator it = g_pk10_cache.find(key);
			if (it != g_pk10_cache.end())
			{
				fieldInfo.insert(std::make_pair(key, it->second));

				nCount++;
				if (nLineNum <= nCount){
					break;
				}
			}else{
				break;
			}
		}
	}
}