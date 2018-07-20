#include "DBPool.h"
class CDBConn;
class CacheConn;
#if 0
class CAutoDB
{
public:
    CAutoDB(const char* pDBName, CDBConn** pDBConn);
    ~CAutoDB();
private:
    CDBConn* m_pDBConn;
};
#endif
namespace uchat
{
}
/// @sa DBReleaseDeleter
using CAutoDB = std::shared_ptr<CDBConn>;

class CAutoCache
{
    CAutoCache(const char* pCacheName, CacheConn** pCacheConn);
    ~CAutoCache();
private:
    CacheConn* m_pCacheConn;
};
