#include "DBPool.h"
#include <iostream>
#include <sstream>
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
#include "ConfigFileReader.h"
constexpr char const* kN = "db_proxy_server";
namespace uchat
{
DBQueryResult::ConstIterator::ConstIterator() noexcept {}
DBQueryResult::ConstIterator::~ConstIterator() noexcept {}
DBQueryResult::ConstIterator::ConstIterator(
    std::shared_ptr<MYSQL_RES> const& mysqlRes,
    std::shared_ptr<std::map<std::string, uint32_t> > const& fieldNames)
    noexcept: mysqlRes(mysqlRes), fieldNames(fieldNames)
{
    if (mysqlRes) {
        this->numRows = ::mysql_num_rows(mysqlRes.get());
        if (this->numRows > 0) {
            ::mysql_data_seek(mysqlRes.get(), 0u);
            this->row = ::mysql_fetch_row(mysqlRes.get());
        }
    }
}
DBQueryResult::ConstIterator::ConstIterator(ConstIterator&& other) noexcept
{
    this->mysqlRes = other.mysqlRes;
    this->fieldNames = other.fieldNames;
    this->row = other.row;
    this->numRows = other.numRows;
    this->curRowIdx = other.curRowIdx;
    other.mysqlRes = nullptr;
    other.fieldNames = nullptr;
    other.row = nullptr;
    other.numRows = 0u;
    other.curRowIdx = 0;
}
DBQueryResult::ConstIterator::ConstIterator(ConstIterator const& other)
noexcept
{
    this->mysqlRes = other.mysqlRes;
    this->fieldNames = other.fieldNames;
    this->row = other.row;
    this->numRows = other.numRows;
    this->curRowIdx = other.curRowIdx;
}
DBQueryResult::ConstIterator& DBQueryResult::ConstIterator::operator++()
noexcept
{
    if (this->mysqlRes && this->row) {
        this->row = ::mysql_fetch_row(this->mysqlRes.get());
        ++(this->curRowIdx);
    }
    return *this;
}
DBQueryResult::ConstIterator& DBQueryResult::ConstIterator::operator--()
noexcept
{
    if (this->mysqlRes && this->row && this->curRowIdx > 0) {
        ::mysql_data_seek(mysqlRes.get(), --(this->curRowIdx));
        this->row = ::mysql_fetch_row(mysqlRes.get());
    }
    return *this;
}
DBQueryResult::ConstIterator DBQueryResult::ConstIterator::operator++(int)
noexcept
{
    auto ret = *this;
    if (this->mysqlRes && this->row) {
        this->row = ::mysql_fetch_row(this->mysqlRes.get());
        ++(this->curRowIdx);
    }
    return std::move(ret);
}
bool DBQueryResult::ConstIterator::operator!=(
    ConstIterator const& other) const noexcept
{
    return this->row != other.row;
}
SimpleVariantView DBQueryResult::ConstIterator::operator()(
    string const& fieldName) const
{
    Assert(this->row && this->fieldNames, "nil row or nil fieldNames")
    auto const it = this->fieldNames->find(fieldName);
    Assert(this->fieldNames->cend() != it, "no such fieldName " << fieldName)
    char const* const p = this->row[it->second];
    if (p) {
        return SimpleVariantView{ p, ::strlen(p) };
    } else {
        return SimpleVariantView{};
    }
}
DBQueryResult::ConstIterator DBQueryResult::begin() const noexcept
{
    if (!this->mysqlRes) {
        return ConstIterator();
    } else {
        auto ret = ConstIterator(this->mysqlRes, this->fieldNames);
    return ret;
    }
}
DBQueryResult::DBQueryResult(std::shared_ptr<MYSQL_RES> const& mysqlRes)
noexcept: mysqlRes(mysqlRes)
{
    if (!mysqlRes) {
        return;
    }
    // Map table field key to index in the result array
    uint32_t const numFields = ::mysql_num_fields(mysqlRes.get());
    if (numFields <= 0) {
        return;
    }
    this->fieldNames.reset(new std::map<std::string, uint32_t>());
    auto& fieldNames = *(this->fieldNames);
    MYSQL_FIELD* const fields = ::mysql_fetch_fields(mysqlRes.get());
    for (uint32_t i = 0; i < numFields; ++i) {
        fieldNames[fields[i].name] = i;
    }
}
DBQueryResult::DBQueryResult(DBQueryResult&& other) noexcept
{
    this->mysqlRes = other.mysqlRes;
    this->fieldNames = other.fieldNames;
    other.mysqlRes = nullptr;
    other.fieldNames = nullptr;
}
DBQueryResult& DBQueryResult::operator=(DBQueryResult&& other) noexcept
{
    if (this != &other) {
        this->mysqlRes = other.mysqlRes;
        this->fieldNames = other.fieldNames;
        other.mysqlRes = nullptr;
        other.fieldNames = nullptr;
    }
    return *this;
}
DBQueryResult::~DBQueryResult() noexcept {}
void DBReleaseDeleter::operator()(CDBConn*& p) const noexcept
{
    if (p) {
        CDBManager::getInstance()->RelDBConn(p);
        p = nullptr;
    }
}
void MySQLStmtDeleter::operator()(MYSQL_STMT*& p) const noexcept
{
    if (p) {
        ::mysql_stmt_close(p);
        p = nullptr;
    }
}
DBViewedPrepareStatement::DBViewedPrepareStatement() noexcept {}
DBViewedPrepareStatement::~DBViewedPrepareStatement() noexcept
{
    this->stmt = nullptr;
}
void DBViewedPrepareStatement::init(
    MYSQL* const mysql,
    std::string const& sql,
    uint32_t const expectedParamCount) try {
    Assert(mysql, "nil mysql")
    ::mysql_ping(mysql);
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    this->stmt = nullptr;
    this->stmt.reset(::mysql_stmt_init(mysql));
    Assert(this->stmt, "mysql_stmt_init failed")
    constexpr int OK = 0;
    Assert(OK == ::mysql_stmt_prepare(
        this->stmt.get(), sql.c_str(), sql.length()),
        "mysql_stmt_prepare failed: "
        << ::mysql_stmt_error(this->stmt.get()))
    auto const expectedParamCount0 = ::mysql_stmt_param_count(this->stmt.get());
    Assert(expectedParamCount == expectedParamCount0,
        "param count not match")
    this->expectedParamCount = expectedParamCount0;
    if (this->expectedParamCount > 0) {
        this->paramBind.clear();
        this->paramBind.shrink_to_fit();
        this->paramBind.reserve(this->expectedParamCount);
        ::memset(
            this->paramBind.data(),
            0,
            sizeof(MYSQL_BIND) * this->expectedParamCount);
    }
} catch(std::exception const& e) {
    this->reset();
    throw std::runtime_error(e.what());
}
void DBViewedPrepareStatement::__append(uint8_t const& param)
{
    Assert(this->paramBind.size() < this->expectedParamCount, "param full")
    MYSQL_BIND b;
    b.buffer_type = MYSQL_TYPE_TINY;
    b.buffer = const_cast<uint8_t*>(&param);
    b.buffer_length = sizeof(param);
    b.is_null = 0;
    b.length = 0;
    this->paramBind.emplace_back(std::move(b));
}
void DBViewedPrepareStatement::__append(uint32_t const& param)
{
    Assert(this->paramBind.size() < this->expectedParamCount, "param full")
    MYSQL_BIND b;
    b.buffer_type = MYSQL_TYPE_LONG;
    b.buffer = const_cast<uint32_t*>(&param);
    b.buffer_length = sizeof(param);
    b.is_null = 0;
    b.length = 0;
    this->paramBind.emplace_back(std::move(b));
}
void DBViewedPrepareStatement::__append(std::string const& param)
{
    Assert(this->paramBind.size() < this->expectedParamCount, "param full")
    MYSQL_BIND b;
    b.buffer_type = MYSQL_TYPE_STRING;
    b.buffer = const_cast<char*>(param.c_str());
    b.buffer_length = param.length();
    b.is_null = 0;
    //b.length = &(this->lenbuf[idx]);
    b.length = 0;
    this->paramBind.emplace_back(std::move(b));
}
DBViewedPrepareStatement& DBViewedPrepareStatement::operator<<(
    uint8_t const& param)
{
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    Assert(this->stmt, "not init")
    this->paramBind.clear();
    this->__append(param);
    return *this;
}
DBViewedPrepareStatement& DBViewedPrepareStatement::operator<<(
    uint32_t const& param)
{
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    Assert(this->stmt, "not init")
    this->paramBind.clear();
    this->__append(param);
    return *this;
}
DBViewedPrepareStatement& DBViewedPrepareStatement::operator<<(
    std::string const& param)
{
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    Assert(this->stmt, "not init")
    this->paramBind.clear();
    this->__append(param);
    return *this;
}
void DBViewedPrepareStatement::clear() noexcept
{
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    if (this->stmt) {
        this->paramBind.clear();
    }
}
void DBViewedPrepareStatement::reset() noexcept
{
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    if (this->stmt) {
        this->stmt = nullptr;
        this->paramBind.clear();
        this->paramBind.shrink_to_fit();
    }
}
DBViewedPrepareStatement& DBViewedPrepareStatement::operator,(
    uint8_t const& param)
{
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    Assert(this->stmt, "not init")
    this->__append(param);
    return *this;
}
DBViewedPrepareStatement& DBViewedPrepareStatement::operator,(
    uint32_t const& param)
{
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    Assert(this->stmt, "not init")
    this->__append(param);
    return *this;
}
DBViewedPrepareStatement& DBViewedPrepareStatement::operator,(
    std::string const& param)
{
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    Assert(this->stmt, "not init")
    this->__append(param);
    return *this;
}
uint32_t DBViewedPrepareStatement::execute()
{
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    Assert(this->stmt, "not init")
    Assert(this->expectedParamCount <= this->paramBind.size(),
        "no enough params")
    constexpr int OK = 0;
    Assert(OK == ::mysql_stmt_bind_param(
        this->stmt.get(), this->paramBind.data()),
        "mysql_stmt_bind_param failed: "
        << ::mysql_stmt_error(this->stmt.get()))
    Assert(OK == ::mysql_stmt_execute(this->stmt.get()),
        "mysql_stmt_execute failed: "
        << ::mysql_stmt_error(this->stmt.get()))
    Assert(1 == ::mysql_stmt_affected_rows(this->stmt.get()),
        "mysql_stmt_affected_rows not 1")
    return ::mysql_stmt_insert_id(this->stmt.get());
}
uint64_t DBViewedPrepareStatement::executeStatement()
{
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    Assert(this->stmt, "not init")
    Assert(this->expectedParamCount <= this->paramBind.size(),
        "no enough params")
    constexpr int OK = 0;
    Assert(OK == ::mysql_stmt_bind_param(
        this->stmt.get(), this->paramBind.data()),
        "mysql_stmt_bind_param failed: "
        << ::mysql_stmt_error(this->stmt.get()))
    Assert(OK == ::mysql_stmt_execute(this->stmt.get()),
        "mysql_stmt_execute failed: "
        << ::mysql_stmt_error(this->stmt.get()))
    return ::mysql_stmt_affected_rows(this->stmt.get());
}
#if 0
DBQueryResult DBViewedPrepareStatement::executeQuery()
{
    std::lock_guard<std::mutex> stmtMutex(this->stmtMutex);
    Assert(this->stmt, "not init")
    Assert(this->expectedParamCount <= this->paramBind.size(),
        "no enough params")
    constexpr int OK = 0;
    Assert(OK == ::mysql_stmt_bind_param(
        this->stmt.get(), this->paramBind.data()),
        "mysql_stmt_bind_param failed: "
        << ::mysql_stmt_error(this->stmt.get()))
    Assert(OK == ::mysql_stmt_execute(this->stmt.get()),
        "mysql_stmt_execute failed: "
        << ::mysql_stmt_error(this->stmt.get()))
    Assert(OK == ::mysql_stmt_store_result(this->stmt.get()),
        "mysql_stmt_store_result fail")
    ::mysql_stmt_fetch(this->stmt.get());
    MYSQL_RES* const res = ::mysql_stmt_store_result(this->stmt.get());
    Assert(res, "mysql_store_result failed: " << ::mysql_error(this->stmt->mysql))
    std::shared_ptr<MYSQL_RES> mysqlRes(res, [](MYSQL_RES*& p) {
        if (p) {
            Detail("free: " << p)
            ::mysql_free_result(p);
            p = nullptr;
        }
    });
    return uchat::DBQueryResult(mysqlRes);
}
#endif
} // namespace uchat
#define MIN_DB_CONN_CNT		2

CDBManager* CDBManager::s_db_manager = NULL;

CResultSet::CResultSet(MYSQL_RES* res)
{
	m_res = res;

	// map table field key to index in the result array
	int num_fields = mysql_num_fields(m_res);
	MYSQL_FIELD* fields = mysql_fetch_fields(m_res);
	for(int i = 0; i < num_fields; i++)
	{
	   m_key_map.insert(make_pair(fields[i].name, i));
	}
}

CResultSet::~CResultSet()
{
	if (m_res) {
		mysql_free_result(m_res);
		m_res = NULL;
	}
}

bool CResultSet::Next()
{
	m_row = mysql_fetch_row(m_res);
	if (m_row) {
		return true;
	} else {
		return false;
	}
}

int CResultSet::_GetIndex(const char* key)
{
	map<string, int>::iterator it = m_key_map.find(key);
	if (it == m_key_map.end()) {
		return -1;
	} else {
		return it->second;
	}
}

int CResultSet::GetInt(const char* key)
{
	int idx = _GetIndex(key);
	if (idx == -1) {
		return 0;
	} else {
		return atoi(m_row[idx]);
	}
}

char* CResultSet::GetString(const char* key)
{
	int idx = _GetIndex(key);
	if (idx == -1) {
		return NULL;
	} else {
		return m_row[idx];
	}
}
/////////////////////////////////////////
CPrepareStatement::CPrepareStatement()
{
	m_stmt = NULL;
	m_param_bind = NULL;
	m_param_cnt = 0;
}

CPrepareStatement::~CPrepareStatement()
{
	if (m_stmt) {
		mysql_stmt_close(m_stmt);
		m_stmt = NULL;
	}

	if (m_param_bind) {
		delete [] m_param_bind;
		m_param_bind = NULL;
	}
}

bool CPrepareStatement::Init(MYSQL* mysql, string& sql)
{
	mysql_ping(mysql);

	m_stmt = mysql_stmt_init(mysql);
	if (!m_stmt) {
		return false;
	}

	if (mysql_stmt_prepare(m_stmt, sql.c_str(), sql.size())) {
		return false;
	}

	m_param_cnt = mysql_stmt_param_count(m_stmt);
	if (m_param_cnt > 0) {
		m_param_bind = new MYSQL_BIND [m_param_cnt];
		if (!m_param_bind) {
			return false;
		}

		memset(m_param_bind, 0, sizeof(MYSQL_BIND) * m_param_cnt);
	}

	return true;
}

void CPrepareStatement::SetParam(uint32_t index, int& value)
{
	if (index >= m_param_cnt) {
		return;
	}

	m_param_bind[index].buffer_type = MYSQL_TYPE_LONG;
	m_param_bind[index].buffer = &value;
}

void CPrepareStatement::SetParam(uint32_t index, uint32_t& value)
{
	if (index >= m_param_cnt) {
		return;
	}

	m_param_bind[index].buffer_type = MYSQL_TYPE_LONG;
	m_param_bind[index].buffer = &value;
}

void CPrepareStatement::SetParam(uint32_t index, string& value)
{
	if (index >= m_param_cnt) {
		return;
	}

	m_param_bind[index].buffer_type = MYSQL_TYPE_STRING;
	m_param_bind[index].buffer = (char*)value.c_str();
	m_param_bind[index].buffer_length = value.size();
}

void CPrepareStatement::SetParam(uint32_t index, const string& value)
{
    if (index >= m_param_cnt) {
        return;
    }
    
    m_param_bind[index].buffer_type = MYSQL_TYPE_STRING;
    m_param_bind[index].buffer = (char*)value.c_str();
    m_param_bind[index].buffer_length = value.size();
}

bool CPrepareStatement::ExecuteUpdate()
{
	if (!m_stmt) {
		return false;
	}

	if (mysql_stmt_bind_param(m_stmt, m_param_bind)) {
		return false;
	}

	if (mysql_stmt_execute(m_stmt)) {
		return false;
	}

	if (mysql_stmt_affected_rows(m_stmt) == 0) {
		return false;
	}

	return true;
}

uint32_t CPrepareStatement::GetInsertId()
{
	return mysql_stmt_insert_id(m_stmt);
}
/////////////////////
CDBConn::CDBConn(CDBPool* pPool)
{
	m_pDBPool = pPool;
	m_mysql = NULL;
}

CDBConn::~CDBConn()
{

}

int CDBConn::Init()
{
	m_mysql = mysql_init(NULL);
	if (!m_mysql) {
		Error0("mysql_init failed")
		return 1;
	}

	my_bool reconnect = true;
	mysql_options(m_mysql, MYSQL_OPT_RECONNECT, &reconnect);
        //mysql_options(m_mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");
	mysql_options(m_mysql, MYSQL_SET_CHARSET_NAME, "utf8");

	if (!mysql_real_connect(m_mysql, m_pDBPool->GetDBServerIP(), m_pDBPool->GetUsername(), m_pDBPool->GetPasswrod(),
			m_pDBPool->GetDBName(), m_pDBPool->GetDBServerPort(), NULL, 0)) {
		return 2;
	}

	return 0;
}

const char* CDBConn::GetPoolName()
{
	return m_pDBPool->GetPoolName();
}
uchat::DBQueryResult CDBConn::executeQuery(std::string const& sql) noexcept
{
    if (sql.empty() || !this->m_mysql) {
        Error0("invlaid sql or mysql")
        return uchat::DBQueryResult();
    }
    ::mysql_ping(this->m_mysql);
    if (::mysql_real_query(this->m_mysql, sql.c_str(), sql.length())) {
        Error0("mysql_real_query failed: " <<
            ::mysql_error(this->m_mysql) << " sql " << sql)
        return uchat::DBQueryResult();
    }
    std::shared_ptr<MYSQL_RES> mysqlRes;
    {
        MYSQL_RES* const res = ::mysql_store_result(this->m_mysql);
        if (!res) {
            Error0("mysql_store_result failed: "
                << ::mysql_error(this->m_mysql))
            return uchat::DBQueryResult();
        }
        mysqlRes.reset(res, [](MYSQL_RES*& p) {
            if (p) {
                Detail("free: " << p)
                ::mysql_free_result(p);
                p = nullptr;
            }
        });
    }
    return uchat::DBQueryResult(mysqlRes);
}
CResultSet* CDBConn::ExecuteQuery(const char* sql_query)
{
	mysql_ping(m_mysql);

	if (mysql_real_query(m_mysql, sql_query, strlen(sql_query))) {
		return NULL;
	}

	MYSQL_RES* res = mysql_store_result(m_mysql);
	if (!res) {
		return NULL;
	}

	CResultSet* result_set = new CResultSet(res);
	return result_set;
}
bool CDBConn::ExecuteUpdate(const char* sql_query)
{
	mysql_ping(m_mysql);

	if (mysql_real_query(m_mysql, sql_query, strlen(sql_query))) {
		return false;
	}

	if (mysql_affected_rows(m_mysql) > 0) {
		return true;
	} else {
		return false;
	}
}

char* CDBConn::EscapeString(const char* content, uint32_t content_len)
{
	if (content_len > (MAX_ESCAPE_STRING_LEN >> 1)) {
		m_escape_string[0] = 0;
	} else {
		mysql_real_escape_string(m_mysql, m_escape_string, content, content_len);
	}

	return m_escape_string;
}

uint32_t CDBConn::GetInsertId()
{
	return (uint32_t)mysql_insert_id(m_mysql);
}

////////////////
CDBPool::CDBPool(const char* pool_name, const char* db_server_ip, uint16_t db_server_port,
		const char* username, const char* password, const char* db_name, int max_conn_cnt)
{
	m_pool_name = pool_name;
	m_db_server_ip = db_server_ip;
	m_db_server_port = db_server_port;
	m_username = username;
	m_password = password;
	m_db_name = db_name;
	m_db_max_conn_cnt = max_conn_cnt;
	m_db_cur_conn_cnt = MIN_DB_CONN_CNT;
}

CDBPool::~CDBPool()
{
	for (list<CDBConn*>::iterator it = m_free_list.begin(); it != m_free_list.end(); it++) {
		CDBConn* pConn = *it;
		delete pConn;
	}

	m_free_list.clear();
}

int CDBPool::Init()
{
	for (int i = 0; i < m_db_cur_conn_cnt; i++) {
		CDBConn* pDBConn = new CDBConn(this);
		int ret = pDBConn->Init();
		if (ret) {
			delete pDBConn;
			return ret;
		}

		m_free_list.push_back(pDBConn);
	}
    Info0("db pool: " << m_pool_name << " size " << m_free_list.size())
    return 0;
}

/*
 *TODO: 增加保护机制，把分配的连接加入另一个队列，这样获取连接时，如果没有空闲连接，
 *TODO: 检查已经分配的连接多久没有返回，如果超过一定时间，则自动收回连接，放在用户忘了调用释放连接的接口
 */
CDBConn* CDBPool::GetDBConn()
{
	m_free_notify.Lock();

	while (m_free_list.empty()) {
		if (m_db_cur_conn_cnt >= m_db_max_conn_cnt) {
            log("CDBPool current no db connection for %s.", m_pool_name.c_str());
			m_free_notify.Wait();
		} else {
			CDBConn* pDBConn = new CDBConn(this);
			int ret = pDBConn->Init();
			if (ret) {
				//log("Init DBConnecton failed");
				delete pDBConn;
				m_free_notify.Unlock();
				return NULL;
			} else {
				m_free_list.push_back(pDBConn);
				m_db_cur_conn_cnt++;
				//log("new db connection: %s, conn_cnt: %d", m_pool_name.c_str(), m_db_cur_conn_cnt);
			}
		}
	}

	CDBConn* pConn = m_free_list.front();
	m_free_list.pop_front();

	m_free_notify.Unlock();

	return pConn;
}

void CDBPool::RelDBConn(CDBConn* pConn)
{
	m_free_notify.Lock();

	list<CDBConn*>::iterator it = m_free_list.begin();
	for (; it != m_free_list.end(); it++) {
		if (*it == pConn) {
			break;
		}
	}

	if (it == m_free_list.end()) {
		m_free_list.push_back(pConn);
	}
    //Warning(__func__ << ": free count " << m_free_list.size()
    //    << " cur count " << m_db_cur_conn_cnt << " max count " << m_db_cur_conn_cnt)
	m_free_notify.Signal();
	m_free_notify.Unlock();
}

/////////////////
CDBManager::CDBManager()
{

}

CDBManager::~CDBManager()
{

}

CDBManager* CDBManager::getInstance()
{
	if (!s_db_manager) {
		s_db_manager = new CDBManager();
		if (s_db_manager->Init()) {
			delete s_db_manager;
			s_db_manager = NULL;
		}
	}

	return s_db_manager;
}
/*
 * 2015-01-12
 * modify by ZhangYuanhao :enable config the max connection of every instance
 *
 */
int CDBManager::Init()
{
	CConfigFileReader config_file("dbproxyserver.conf");

	char* db_instances = config_file.GetConfigName("DBInstances");

	if (!db_instances) {
		//log("not configure DBInstances");
		return 1;
	}

	char host[64];
	char port[64];
	char dbname[64];
	char username[64];
	char password[64];
    char maxconncnt[64];
	CStrExplode instances_name(db_instances, ',');

	for (uint32_t i = 0; i < instances_name.GetItemCnt(); i++) {
		char* pool_name = instances_name.GetItem(i);
		snprintf(host, 64, "%s_host", pool_name);
		snprintf(port, 64, "%s_port", pool_name);
		snprintf(dbname, 64, "%s_dbname", pool_name);
		snprintf(username, 64, "%s_username", pool_name);
		snprintf(password, 64, "%s_password", pool_name);
        snprintf(maxconncnt, 64, "%s_maxconncnt", pool_name);

		char* db_host = config_file.GetConfigName(host);
		char* str_db_port = config_file.GetConfigName(port);
		char* db_dbname = config_file.GetConfigName(dbname);
		char* db_username = config_file.GetConfigName(username);
		char* db_password = config_file.GetConfigName(password);
        char* str_maxconncnt = config_file.GetConfigName(maxconncnt);

		if (!db_host || !str_db_port || !db_dbname || !db_username || !db_password || !str_maxconncnt) {
			//log("not configure db instance: %s", pool_name);
			return 2;
		}

		int db_port = atoi(str_db_port);
        int db_maxconncnt = atoi(str_maxconncnt);
		CDBPool* pDBPool = new CDBPool(pool_name, db_host, db_port, db_username, db_password, db_dbname, db_maxconncnt);
		if (pDBPool->Init()) {
			//log("init db instance failed: %s", pool_name);
			return 3;
		}
		m_dbpool_map.insert(make_pair(pool_name, pDBPool));
	}
    if (auto const dbc = CDBManager::getInstance()->getdbconn("teamtalk_master")) {
        // IMGroup
        {
            constexpr char const* sql = "ALTER TABLE `IMGroup` ADD"
                " `announcement` VARCHAR(1024) CHARACTER SET utf8 COLLATE"
                " utf8_general_ci NOT NULL DEFAULT ''"
                " COMMENT '群公告'"
                " AFTER `pk10rules`;";
            dbc->ExecuteUpdate(sql);
        }
        {
            constexpr char const* sql = "ALTER TABLE `IMGroup` ADD"
                " `proxy_auto_add` TINYINT(1) NOT NULL DEFAULT 1"
                " COMMENT '绑定代理自动拉入群 1 是 0 否'"
                " AFTER `announcement`;";
            dbc->ExecuteUpdate(sql);
        }
        {
            constexpr char const* sql = "ALTER TABLE `IMGroup` ADD"
                " `normal_invite` TINYINT(1) NOT NULL DEFAULT 1"
                " COMMENT '允许普通群员邀请玩家入群 1 是 0 否'"
                " AFTER `proxy_auto_add`;";
            dbc->ExecuteUpdate(sql);
        }
        // IMGroupMember
        {
            constexpr char const* sql = "ALTER TABLE `IMGroupMember` CHANGE"
                " `status` `status` TINYINT(4) UNSIGNED NOT NULL DEFAULT 1"
                " COMMENT '状态, 0 正常, 1 已退出, 2 管理员';";
            dbc->ExecuteUpdate(sql);
        }
        {
            constexpr char const* sql = "ALTER TABLE `IMGroupMember`"
                " ADD `shield` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0 COMMENT"
                " '屏蔽消息, 默认 0, 0 关闭免打扰, 1 免打扰'"
                " AFTER `updated`;";
            dbc->ExecuteUpdate(sql);
        }
        {
            constexpr char const* sql = "ALTER TABLE `IMGroupMember` ADD"
                " `nick` VARCHAR(128) CHARACTER SET utf8 COLLATE"
                " utf8_general_ci NOT NULL DEFAULT ''"
                " COMMENT '昵称'"
                " AFTER `shield`;";
            dbc->ExecuteUpdate(sql);
        }
    }
	return 0;
}
CDBConn* CDBManager::GetDBConnDEPRECATED(const char* dbpool_name)
{
	map<string, CDBPool*>::iterator it = m_dbpool_map.find(dbpool_name);
	if (it == m_dbpool_map.end()) {
		return NULL;
	} else {
		return it->second->GetDBConn();
	}
}
std::shared_ptr<CDBConn> CDBManager::getdbconn(char const* const dbpoolName) noexcept
{
    if (!dbpoolName) {
        return nullptr;
    }
    auto const it = this->m_dbpool_map.find(dbpoolName);
    if (it == this->m_dbpool_map.end()) {
        return nullptr;
    } else {
        std::shared_ptr<CDBConn> const ret(it->second->GetDBConn(),
            uchat::DBReleaseDeleter{}
            /*[this](CDBConn*& p){
            if (p) {
                this->RelDBConn(p);
                p = nullptr;
            } }*/);
        return ret;
    }
}
void CDBManager::RelDBConn(CDBConn* pConn)
{
	if (!pConn) {
		return;
	}

	map<string, CDBPool*>::iterator it = m_dbpool_map.find(pConn->GetPoolName());
	if (it != m_dbpool_map.end()) {
		it->second->RelDBConn(pConn);
	}
}
