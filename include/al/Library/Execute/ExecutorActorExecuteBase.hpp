#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
class MultiCoreExecutorThreadBase;

class ExecutorActorExecuteBase {
public:
    ExecutorActorExecuteBase(const char* pName);

    virtual void execute() const = 0;
    virtual void executePaused() const;

    void registerActor(LiveActor* pActor);
    void createExecutorTable();
    void addActor(LiveActor* pActor);
    void removeActor(LiveActor* pActor);

    const char* mName;
    s32 mActorNumMax = 0;
    s32 mActorNum = 0;
    LiveActor** mActors = nullptr;
};

class ExecutorActorDraw : public ExecutorActorExecuteBase {
public:
    ExecutorActorDraw(const char* pName);
    void execute() const override;
};

class ExecutorActorMovement : public ExecutorActorExecuteBase {
public:
    ExecutorActorMovement(const char* pName);
    void execute() const override;
    void executePaused() const override;
};

class ExecutorActorCalcAnim : public ExecutorActorExecuteBase {
public:
    ExecutorActorCalcAnim(const char* pName);
    void execute() const override;
};

class ExecutorActorModelUpdate : public ExecutorActorExecuteBase {
public:
    ExecutorActorModelUpdate(const char* pName);
    void execute() const override;
    void executePaused() const override;
};

class ExecutorActorMovementCalcAnim : public ExecutorActorExecuteBase {
public:
    ExecutorActorMovementCalcAnim(const char* pName, MultiCoreExecutorThreadBase* pThread);
    void execute() const override;
    void executePaused() const override;

    MultiCoreExecutorThreadBase* mThread;
};
}  // namespace al
