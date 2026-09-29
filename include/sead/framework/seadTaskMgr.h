#pragma once

#include <container/seadObjList.h>
#include <framework/seadHeapPolicies.h>
#include <framework/seadMethodTree.h>
#include <framework/seadTaskBase.h>
#include <heap/seadHeapMgr.h>
#include <thread/seadCriticalSection.h>
#include <thread/seadMessageQueue.h>

namespace sead
{
class DelegateThread;
class Framework;
class Heap;
class NullFaderTask;
class Thread;

class TaskMgr final : public sead::hostio::Node
{
public:
    struct InitializeArg
    {
    public:
        InitializeArg(const TaskBase::CreateArg& roottask_arg);

        u32 create_queue_size = 0x20;
        u32 prepare_stack_size = 0x10000;
        s32 prepare_priority = -1;
        const TaskBase::CreateArg& roottask_create_arg;
        Heap* heap = nullptr;
        Framework* parent_framework = nullptr;
    };

    struct TaskCreateContext
    {
        TaskCreateContext() : task(nullptr) {}

        TaskBase* task;
        TaskBase::CreateArg arg;
        DelegateEvent<TaskBase*> event;
    };

    class TaskCreateContextMgr : public ObjList<TaskCreateContext>
    {
    public:
        TaskCreateContextMgr(s32 capacity, Heap* pHeap) { allocBuffer(capacity, pHeap); }
    };

public:
    TaskMgr(const InitializeArg& arg);

    static TaskMgr* initialize(const InitializeArg& rArg);
    void initHostIO();
    void finalize();

    bool requestCreateTask(const TaskBase::CreateArg& rArg);
    TaskBase* createTaskSync(const TaskBase::CreateArg& arg);
    bool requestTakeover(const TaskBase::TakeoverArg& rArg);
    bool requestTransition(TaskBase* pFrom, TaskBase* pTo, FaderTaskBase* pFader);
    bool requestPush(const TaskBase::PushArg& rArg);
    TaskBase* pushSync(const TaskBase::PushArg& rArg);
    bool requestPop(TaskBase* pTask, FaderTaskBase* pFader);
    bool requestPop(TaskBase* pFrom, TaskBase* pTo, FaderTaskBase* pFader);
    bool popSync(TaskBase* pTask);
    void requestDestroyTask(TaskBase* pTask, FaderTaskBase* pFader);
    void destroyTaskSync(TaskBase* task);
    void destroyAllAndCreateRoot();
    TaskBase* findTask(const TaskClassID& rClassID);

    void beforeCalc();
    void afterCalc();

    template <typename T>
    T* createSingletonTaskSync(const TaskBase::CreateArg& arg)
    {
        TaskBase::CreateArg arg_ = arg;
        arg_.instance_cb = &T::setInstance_;

        TaskBase* task = createTaskSync(arg_);

        T* derived = DynamicCast<T>(task);
        SEAD_ASSERT(derived != nullptr);

        return T::instance();
    }

    void doInit_();
    void beginCreateRootTask_();
    void prepare_(Thread* pThread, MessageQueue::Element msg);
    void createHeap_(HeapArray* pHeapArray, const TaskBase::CreateArg& rArg);
    TaskBase* doCreateTask_(const TaskBase::CreateArg& rArg, HeapArray* pHeapArray);
    bool changeTaskState_(TaskBase* task, TaskBase::State state);
    bool doRequestCreateTask_(const TaskBase::CreateArg& rArg,
                              DelegateEvent<TaskBase*>::Slot* pSlot);
    void appendToList_(TaskBase::List& ls, TaskBase* task);
    void doDestroyTask_(TaskBase* task);
    bool destroyable_(TaskBase* pTask);
    void calcCreation_();
    void calcDestruction_();

    CriticalSection mCriticalSection;
    Framework* mParentFramework;
    DelegateThread* mPrepareThread;
    NullFaderTask* mNullFaderTask;
    TaskBase::List mPrepareList;
    TaskBase::List mPrepareDoneList;
    TaskBase::List mActiveList;
    TaskBase::List mStaticList;
    TaskBase::List mDyingList;
    TaskBase::List mDestroyableList;
    HeapArray mHeapArray;
    TaskCreateContextMgr* mTaskCreateContextMgr;
    u32 mMaxCreateQueueSize;
    TaskBase* mRootTask;
    TaskBase::CreateArg mRootTaskCreateArg;
    TaskMgr::InitializeArg mInitializeArg;
    MethodTreeNode mCalcDestructionTreeNode{nullptr};
};

}  // namespace sead

#define SEAD_TASK_SINGLETON(CLASS)                                                                 \
public:                                                                                            \
    class SingletonDisposer_                                                                       \
    {                                                                                              \
    public:                                                                                        \
        ~SingletonDisposer_();                                                                     \
                                                                                                   \
        bool mActive = false;                                                                      \
    };                                                                                             \
                                                                                                   \
    static CLASS* instance() { return sInstance; }                                                 \
    static void setInstance_(sead::TaskBase* task);                                                \
    static void deleteInstance();                                                                  \
                                                                                                   \
    CLASS(const CLASS&) = delete;                                                                  \
    CLASS& operator=(const CLASS&) = delete;                                                       \
    CLASS(CLASS&&) = delete;                                                                       \
    CLASS& operator=(CLASS&&) = delete;                                                            \
                                                                                                   \
protected:                                                                                         \
    static CLASS* sInstance;                                                                       \
                                                                                                   \
    friend class SingletonDisposer_;                                                               \
    SingletonDisposer_ mSingletonDisposer;

#define SEAD_TASK_SINGLETON_IMPL(CLASS)                                                            \
    CLASS::SingletonDisposer_::~SingletonDisposer_()                                               \
    {                                                                                              \
        if (mActive)                                                                               \
        {                                                                                          \
            CLASS::sInstance = nullptr;                                                            \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    void CLASS::setInstance_(sead::TaskBase* task)                                                 \
    {                                                                                              \
        if (!sInstance)                                                                            \
        {                                                                                          \
            sInstance = static_cast<CLASS*>(task);                                                 \
            sInstance->mSingletonDisposer.mActive = true;                                          \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            SEAD_ASSERT_MSG(false, "Create Singleton Twice (%s) : addr %p", #CLASS, sInstance);    \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    void CLASS::deleteInstance()                                                                   \
    {                                                                                              \
        if (sInstance)                                                                             \
        {                                                                                          \
            sInstance->mTaskMgr->destroyTaskSync(sInstance);                                       \
            sInstance = nullptr;                                                                   \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    CLASS* CLASS::sInstance = nullptr;
