#pragma once
#include <mysql.h>
#include <memory>
#include <mutex>
#include "uchat/simplevariantview.hpp"
#include "../base/util.h"
#include "ThreadPool.h"
#include "business/Common.h"
struct CDBConn;
namespace uchat
{
struct DBQueryResult {
    struct ConstIterator;
    DBQueryResult(std::shared_ptr<MYSQL_RES> const& mysqlRes = nullptr) noexcept;
    DBQueryResult(DBQueryResult&& other) noexcept;
    DBQueryResult& operator=(DBQueryResult&& other) noexcept;
    DBQueryResult(DBQueryResult const& other) = delete;
    DBQueryResult& operator=(DBQueryResult const& other) = delete;
    virtual ~DBQueryResult() noexcept;
    inline operator bool() const noexcept;
    inline bool isEmpty() const noexcept;
    ConstIterator begin() const noexcept;
    inline ConstIterator end() const noexcept;
protected:
    std::shared_ptr<MYSQL_RES> mysqlRes{ nullptr };
    /// { fieldName, idx }
    std::shared_ptr<std::map<std::string, uint32_t> > fieldNames{ nullptr };
};
struct DBQueryResult::ConstIterator {
    ConstIterator() noexcept;
    ConstIterator(
        std::shared_ptr<MYSQL_RES> const& mysqlRes,
        std::shared_ptr<std::map<std::string, uint32_t> > const& fieldNames)
        noexcept;
    ConstIterator(ConstIterator&& other) noexcept;
    ConstIterator(ConstIterator const& other) noexcept;
    ~ConstIterator() noexcept;
    inline operator bool() const noexcept;
    // ++it
    ConstIterator& operator++() noexcept;
    ConstIterator& operator--() noexcept;
    // it++
    ConstIterator operator++(int) noexcept;
    bool operator!=(ConstIterator const& other) const noexcept;
    inline bool operator==(ConstIterator const& other) const noexcept;
    /// @throw std::logic_error when fail
    SimpleVariantView operator()(std::string const& fieldName) const;
protected:
    std::shared_ptr<MYSQL_RES> mysqlRes{ nullptr };
    std::shared_ptr<std::map<std::string, uint32_t> > fieldNames{ nullptr };
    MYSQL_ROW row{ nullptr };
    uint32_t numRows{ 0u };
    int32_t curRowIdx{ 0 };
};
inline DBQueryResult::operator bool() const noexcept
{
    if (this->mysqlRes) {
        return true;
    } else {
        return false;
    }
}
inline bool DBQueryResult::isEmpty() const noexcept
{
    auto const b = this->begin();
    auto const e = this->end();
    return (b == e);
}
inline DBQueryResult::ConstIterator DBQueryResult::end() const noexcept
{
    return ConstIterator();
}
inline DBQueryResult::ConstIterator::operator bool() const noexcept
{
    if (this->row) {
        return true;
    } else {
        return false;
    }
}
inline bool DBQueryResult::ConstIterator::operator==(
    ConstIterator const& other) const noexcept
{
    return !(*this != other);
}
struct DBReleaseDeleter {
    void operator()(CDBConn*& p) const noexcept;
};
struct MySQLStmtDeleter {
    void operator()(MYSQL_STMT*& p) const noexcept;
};
/**
 * @note VIEWED params: caller should ensure the lifetime of params
 * @sa
 * - https://dev.mysql.com/doc/refman/5.7/en/mysql-stmt-execute.html
 * - CPrepareStatement
 * @todo set(idx, param)/get(idx)/...
 */
struct DBViewedPrepareStatement {
    DBViewedPrepareStatement() noexcept;
    virtual ~DBViewedPrepareStatement() noexcept;
    /// No move-cpoy, no copy
    DBViewedPrepareStatement(DBViewedPrepareStatement&&) = delete;
    DBViewedPrepareStatement(DBViewedPrepareStatement const&) = delete;
    DBViewedPrepareStatement& operator=(DBViewedPrepareStatement const&) = delete;
    DBViewedPrepareStatement& operator=(DBViewedPrepareStatement&&) = delete;
    /**
     * Re-initialize
     * @param mysql mysql view
     * @param expectedParamCount to check @a sql
     * @throw std::runtime_error when fail
     */
    void init(
        MYSQL* const mysql,
        std::string const& sql,
        uint32_t const expectedParamCount);
    /**
     * operator<< RESTART append
     * @throw std::runtime_error when fail
     */
    DBViewedPrepareStatement& operator<<(uint8_t const& param);
    DBViewedPrepareStatement& operator<<(uint32_t const& param);
    DBViewedPrepareStatement& operator<<(std::string const& param);
    /// operator, CONTINUE/FINAL append
    DBViewedPrepareStatement& operator,(uint8_t const& param);
    DBViewedPrepareStatement& operator,(uint32_t const& param);
    DBViewedPrepareStatement& operator,(std::string const& param);
    /// Clear all params
    void clear() noexcept;
    /// Release when need and reset MYSQL_STMT to nil
    void reset() noexcept;
    inline void append(uint8_t const& param, bool const reset = false);
    inline void append(uint32_t const& param, bool const reset = false);
    inline void append(std::string const& param, bool const reset = false);
    /**
     * Execute statement == insert
     * @return mysql_stmt_insert_id()
     * @throw std::runtime_error when fail
     */
    uint32_t execute();
    /**
     * Execute statement
     * @return mysql_stmt_affected_rows()
     * @throw std::runtime_error when fail
     */
    uint64_t executeStatement();
    //DBQueryResult executeQuery();
protected:
    void __append(uint8_t const& param);
    void __append(uint32_t const& param);
    void __append(std::string const& param);
    mutable std::mutex stmtMutex;
    std::unique_ptr<MYSQL_STMT, MySQLStmtDeleter> stmt{ nullptr };
    std::vector<MYSQL_BIND> paramBind;
    uint32_t expectedParamCount{ 0 };
};
inline void DBViewedPrepareStatement::append(
    uint8_t const& param, bool const reset)
{
    if (reset) {
        *this << param;
    } else {
        *this, param;
    }
}
inline void DBViewedPrepareStatement::append(
    uint32_t const& param, bool const reset)
{
    if (reset) {
        *this << param;
    } else {
        (*this), param;
    }
}
inline void DBViewedPrepareStatement::append(
    std::string const& param, bool const reset)
{
    if (reset) {
        *this << param;
    } else {
        (*this), param;
    }
}
} // namespace uchat
#define MAX_ESCAPE_STRING_LEN	10240

class CResultSet {
public:
	CResultSet(MYSQL_RES* res);
	virtual ~CResultSet();

	bool Next();
	int GetInt(const char* key);
	char* GetString(const char* key);
private:
	int _GetIndex(const char* key);

	MYSQL_RES* 			m_res;
	MYSQL_ROW			m_row;
	map<string, int>	m_key_map;
};

/**
 * TODO:
 * 用MySQL的prepare statement接口来防止SQL注入
 * 暂时只用于插入IMMessage表，因为只有那里有SQL注入的风险，
 * 以后可以把全部接口用这个来实现替换
 * @deprecated please use uchat::DBViewedPrepareStatement
 * @sa DBViewedPrepareStatement
 */
class CPrepareStatement {
public:
	CPrepareStatement();
	virtual ~CPrepareStatement();

	bool Init(MYSQL* mysql, string& sql);

	void SetParam(uint32_t index, int& value);
	void SetParam(uint32_t index, uint32_t& value);
    void SetParam(uint32_t index, string& value);
    void SetParam(uint32_t index, const string& value);

	bool ExecuteUpdate();
	uint32_t GetInsertId();
private:
	MYSQL_STMT*	m_stmt;
	MYSQL_BIND*	m_param_bind;
	uint32_t	m_param_cnt;
};
class CDBPool;
struct CDBConn {
	CDBConn(CDBPool* pDBPool);
	virtual ~CDBConn();
	int Init();
    uchat::DBQueryResult executeQuery(std::string const& sql) noexcept;
	CResultSet* ExecuteQuery(const char* sql_query);
	bool ExecuteUpdate(const char* sql_query);
	char* EscapeString(const char* content, uint32_t content_len);

	uint32_t GetInsertId();

	const char* GetPoolName();
	MYSQL* GetMysql() { return m_mysql; }
private:
	CDBPool* 	m_pDBPool;	// to get MySQL server information
	MYSQL* 		m_mysql;
	//MYSQL_RES*	m_res;
	char		m_escape_string[MAX_ESCAPE_STRING_LEN + 1];
};

class CDBPool {
public:
	CDBPool(const char* pool_name, const char* db_server_ip, uint16_t db_server_port,
			const char* username, const char* password, const char* db_name, int max_conn_cnt);
	virtual ~CDBPool();

	int Init();
	CDBConn* GetDBConn();
	void RelDBConn(CDBConn* pConn);

	const char* GetPoolName() { return m_pool_name.c_str(); }
	const char* GetDBServerIP() { return m_db_server_ip.c_str(); }
	uint16_t GetDBServerPort() { return m_db_server_port; }
	const char* GetUsername() { return m_username.c_str(); }
	const char* GetPasswrod() { return m_password.c_str(); }
	const char* GetDBName() { return m_db_name.c_str(); }
private:
	string 		m_pool_name;
	string 		m_db_server_ip;
	uint16_t	m_db_server_port;
	string 		m_username;
	string 		m_password;
	string 		m_db_name;
	int			m_db_cur_conn_cnt;
	int 		m_db_max_conn_cnt;
	list<CDBConn*>	m_free_list;
	CThreadNotify	m_free_notify;
};
// manage db pool (master for write and slave for read)
struct CDBManager {
    virtual ~CDBManager();
    static CDBManager* getInstance();
    int Init();
    /// @deprecated please use getdbconn
    CDBConn* GetDBConnDEPRECATED(const char* dbpool_name);
    std::shared_ptr<CDBConn> getdbconn(char const* const dbpoolName) noexcept;
protected:
    friend uchat::DBReleaseDeleter;
    CDBManager();
    void RelDBConn(CDBConn* pConn);
	static CDBManager*		s_db_manager;
	map<string, CDBPool*>	m_dbpool_map;
};
