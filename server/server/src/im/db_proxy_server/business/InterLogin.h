#pragma once
#include "LoginStrategy.h"
class CInterLoginStrategy : public CLoginStrategy {
public:
    virtual bool doRegister(const std::string& strName, const std::string& strPass, const std::string& strNickname);
    virtual bool doLogin(const std::string& strName, const std::string& strPass, IM::BaseDefine::UserInfo& user);
};
