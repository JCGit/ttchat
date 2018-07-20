/*
 * PK10Conn.h
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */

#ifndef PK10_CONN_H_
#define PK10_CONN_H_

#include <string>
#include "Task.h"
using namespace std;

class CBCTask:public CTask
{
public:
    CBCTask(uint32_t line) : nRequestLine(line)
    {
        
    }

	virtual ~CBCTask()
    {

    }
    
	virtual void run();

private:
    uint32_t nRequestLine ;
};

void init_pk10_serv_conn(string server_addr);
void pull_pk10_data(int nLineNum, map<string, Json::Value>& fieldInfo);
void pull_pk10_request(map<string, Json::Value>& fieldInfo, uint32_t nStart, uint32_t nLineNum=1);

#endif /* PK10_CONN_H_ */
