#ifndef SEAD_FRAMEWORK_H_
#define SEAD_FRAMEWORK_H_

#include <framework/seadTaskBase.h>
#include <hostio/seadHostIOMgr.h>
#include <prim/seadDelegateEventSlot.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <time/seadTickSpan.h>

namespace sead
{
class Arena;

class FrameBuffer;
class Heap;
class LogicalFrameBuffer;
class MethodTreeMgr;
class TaskMgr;

class Framework : public hostio::Node
{
    SEAD_RTTI_BASE(Framework)

public:
    struct CreateSystemTaskArg
    {
        CreateSystemTaskArg();

        HostIOMgr::Parameter* hostio_parameter = nullptr;
        Heap* heap = nullptr;
        TickSpan infloop_detection_span;
        int infloop_unk = 0x1000;
    };

    struct InitializeArg
    {
        InitializeArg();

        u64 heap_size = 0x3000000;
        Arena* arena = nullptr;
    };

    struct RunArg
    {
        RunArg();

        u32 prepare_stack_size = 0;
        s32 prepare_priority = -1;
    };

    enum ProcessPriority
    {
        cProcessPriority_Idle = 0,
        cProcessPriority_Normal = 1,
        cProcessPriority_High = 2,
        cProcessPriority_RealTime = 3
    };

public:
    static void initialize(const InitializeArg&);

    Framework();
    virtual ~Framework();

    virtual void run(Heap*, const TaskBase::CreateArg&, const RunArg&);
    virtual void createSystemTasks(TaskBase*, const CreateSystemTaskArg&);
    virtual FrameBuffer* getMethodFrameBuffer(s32) const = 0;
    virtual LogicalFrameBuffer* getMethodLogicalFrameBuffer(s32) const;
    virtual bool setProcessPriority(ProcessPriority) { return false; }

    virtual void reserveReset(void* pParameter)
    {
        mReserveReset = true;
        mResetParameter = pParameter;
    }

    virtual void initRun_(Heap*);
    virtual void quitRun_(Heap*);
    virtual void runImpl_();
    virtual MethodTreeMgr* createMethodTreeMgr_(Heap*) = 0;
    virtual void procReset_();

    MethodTreeMgr* getMethodTreeMgr() const { return mMethodTreeMgr; }

    typedef DelegateEvent<void*> ResetEvent;

    bool mReserveReset;
    void* mResetParameter;
    ResetEvent mResetEvent;
    TaskMgr* mTaskMgr;
    MethodTreeMgr* mMethodTreeMgr;
    Heap* mMethodTreeMgrHeap;
};

}  // namespace sead

#endif  // SEAD_FRAMEWORK_H_
