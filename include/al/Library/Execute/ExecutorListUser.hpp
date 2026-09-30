#pragma once

#include "Library/Execute/ExecutorListBase.hpp"

namespace al {
class IUseExecutor;

class ExecutorListIUseExecutorBase : public ExecutorListBase {
public:
    ExecutorListIUseExecutorBase(const char* pListName, s32 capacity, const char* pGroupName);

    bool isActive() const override { return mUserNum > 0; }

    void registerUser(IUseExecutor* pUser);

    s32 mUserNumMax;
    s32 mUserNum = 0;
    IUseExecutor** mUsers;
};

class ExecutorListIUseExecutorUpdate : public ExecutorListIUseExecutorBase {
public:
    ExecutorListIUseExecutorUpdate(const char* pListName, s32 capacity, const char* pGroupName);

    void executeList() const override;
};

class ExecutorListIUseExecutorDraw : public ExecutorListIUseExecutorBase {
public:
    ExecutorListIUseExecutorDraw(const char* pListName, s32 capacity, const char* pGroupName);

    void executeList() const override;
};
}  // namespace al
