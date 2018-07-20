#include "AutoPool.h"
#include "DBPool.h"
#include "CachePool.h"
#if 0
CAutoDB::CAutoDB(const char* pDBName, CDBConn** pDBConn)
{
    m_pDBConn = CDBManager::getInstance()->GetDBConn(pDBName);
    *pDBConn = m_pDBConn;
}
CAutoDB::~CAutoDB()
{
    if (m_pDBConn != NULL) {
        CDBManager::getInstance()->RelDBConn(m_pDBConn);
        m_pDBConn = NULL;
    }
}
#endif
CAutoCache::CAutoCache(const char* pCacheName, CacheConn** pCacheConn)
{
    m_pCacheConn = CacheManager::getInstance()->GetCacheConn(pCacheName);
    *pCacheConn = m_pCacheConn;
}

CAutoCache::~CAutoCache()
{
    if (m_pCacheConn != NULL) {
        CacheManager::getInstance()->RelCacheConn(m_pCacheConn);
        m_pCacheConn = NULL;
    }
}
