#pragma once

#include "Library/Execute/ExecutorListBase.hpp"

namespace al {
class ExecutorActorExecuteBase;
class LiveActor;
class MultiCoreExecutorThreadBase;

class ExecutorListActorExecuteBase : public ExecutorListBase {
public:
    ExecutorListActorExecuteBase(const char* pListName, s32 capacity, const char* pGroupName);

    void executeList() const override;
    void executeListPaused() const override;
    bool isActive() const override { return mExecutorNum > 0; }
    virtual ExecutorActorExecuteBase* createExecutor(const char* pName) const = 0;

    void registerActor(LiveActor* pActor);
    void createList();

    s32 mExecutorNumMax;
    s32 mExecutorNum = 0;
    ExecutorActorExecuteBase** mExecutors;
};

class ExecutorListActorDraw : public ExecutorListActorExecuteBase {
public:
    ExecutorListActorDraw(const char* pListName, s32 capacity, const char* pGroupName);
    ExecutorActorExecuteBase* createExecutor(const char* pName) const override;
};

class ExecutorListActorMovement : public ExecutorListActorExecuteBase {
public:
    ExecutorListActorMovement(const char* pListName, s32 capacity, const char* pGroupName);
    ExecutorActorExecuteBase* createExecutor(const char* pName) const override;
};

class ExecutorListActorCalcAnim : public ExecutorListActorExecuteBase {
public:
    ExecutorListActorCalcAnim(const char* pListName, s32 capacity, const char* pGroupName);
    ExecutorActorExecuteBase* createExecutor(const char* pName) const override;
};

class ExecutorListActorMovementCalcAnim : public ExecutorListActorExecuteBase {
public:
    ExecutorListActorMovementCalcAnim(const char* pListName, s32 capacity, const char* pGroupName,
                                      MultiCoreExecutorThreadBase* pThread);
    ExecutorActorExecuteBase* createExecutor(const char* pName) const override;

    MultiCoreExecutorThreadBase* mThread;
};

class ExecutorListActorModelUpdate : public ExecutorListActorExecuteBase {
public:
    ExecutorListActorModelUpdate(const char* pListName, s32 capacity, const char* pGroupName);
    ExecutorActorExecuteBase* createExecutor(const char* pName) const override;
};
}  // namespace al
