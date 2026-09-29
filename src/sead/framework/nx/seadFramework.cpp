#include <framework/seadFramework.h>
#include <framework/seadMethodTreeMgr.h>
#include <framework/seadTaskMgr.h>
#include <gfx/seadFrameBuffer.h>
#include <heap/seadExpHeap.h>
#include <heap/seadHeap.h>
#include <random/seadGlobalRandom.h>

namespace sead
{
/**
 * Constructs the system task creation settings with their defaults.
 */
Framework::CreateSystemTaskArg::CreateSystemTaskArg() = default;

/**
 * Constructs the framework.
 */
Framework::Framework() : mReserveReset(false), mResetParameter(nullptr)
{
    mTaskMgr = nullptr;
    mMethodTreeMgr = nullptr;
    mMethodTreeMgrHeap = nullptr;
}

/**
 * Destroys the framework, finalizing the task manager and releasing the method tree manager.
 */
Framework::~Framework()
{
    if (mTaskMgr != nullptr)
    {
        mTaskMgr->finalize();
        delete mTaskMgr;
        mTaskMgr = nullptr;
    }

    if (mMethodTreeMgr != nullptr)
    {
        delete mMethodTreeMgr;
        mMethodTreeMgr = nullptr;
    }

    if (mMethodTreeMgrHeap != nullptr)
    {
        mMethodTreeMgrHeap->destroy();
    }
}

/**
 * Constructs the framework initialization settings with their defaults.
 */
Framework::InitializeArg::InitializeArg() = default;

/**
 * Constructs the run settings with their defaults.
 */
Framework::RunArg::RunArg() = default;

/**
 * Initializes the heap manager, the thread manager and the global random generators.
 * @param rArg the initialization settings
 */
void Framework::initialize(const InitializeArg& rArg)
{
    if (rArg.arena)
    {
        HeapMgr::initialize(rArg.arena);
    }
    else
    {
        HeapMgr::initialize(rArg.heap_size);
    }

    Heap* heap = HeapMgr::instance()->getRootHeap(0);

    {
        Heap* threadHeap = ExpHeap::create(0, "sead::ThreadMgr", heap);

        ThreadMgr::createInstance(threadHeap);
        ThreadMgr::instance()->initialize(threadHeap);

        threadHeap->adjust();
    }

    GlobalRandom::createInstance(heap);
    GlobalRandomNonSync::createInstance(heap);
}

/**
 * Creates the method tree manager and the task manager, then runs the main loop.
 * @param pHeap the heap to create the managers from
 * @param rArg the root task creation settings
 * @param rRunArg the run settings
 */
void Framework::run(Heap* pHeap, const TaskBase::CreateArg& rArg, const RunArg& rRunArg)
{
    initRun_(pHeap);

    Heap* methodTreeMgrHeap = ExpHeap::create(0, "sead::MethodTreeMgr", pHeap);
    mMethodTreeMgr = createMethodTreeMgr_(methodTreeMgrHeap);
    methodTreeMgrHeap->adjust();
    mMethodTreeMgrHeap = methodTreeMgrHeap;

    TaskMgr::InitializeArg taskMgrArg(rArg);
    taskMgrArg.heap = pHeap;
    taskMgrArg.parent_framework = this;

    if (rRunArg.prepare_stack_size != 0)
    {
        taskMgrArg.prepare_stack_size = rRunArg.prepare_stack_size;
    }

    if (rRunArg.prepare_priority != -1)
    {
        taskMgrArg.prepare_priority = rRunArg.prepare_priority;
    }

    mTaskMgr = TaskMgr::initialize(taskMgrArg);
    runImpl_();
    quitRun_(pHeap);
}

/**
 * Creates the system tasks; the base framework has none.
 * @param pBase the parent task
 * @param rArg the system task creation settings
 */
void Framework::createSystemTasks(TaskBase* pBase, const CreateSystemTaskArg& rArg) {}

/**
 * Hook called before the task manager is created.
 * @param pHeap the heap passed to run
 */
void Framework::initRun_(Heap* pHeap) {}

/**
 * Hook called after the main loop has finished.
 * @param pHeap the heap passed to run
 */
void Framework::quitRun_(Heap* pHeap) {}

/**
 * Runs the main loop; the base framework does nothing.
 */
void Framework::runImpl_() {}

/**
 * Performs a reserved reset, notifying the reset listeners and recreating the root task.
 */
void Framework::procReset_()
{
    if (mReserveReset)
    {
        mResetEvent.emit(mResetParameter);
        mTaskMgr->destroyAllAndCreateRoot();
        mReserveReset = false;
        mResetParameter = nullptr;
    }
}

/**
 * Gets the logical frame buffer of a method.
 * @param methodType the method type
 * @return the method's frame buffer
 */
LogicalFrameBuffer* Framework::getMethodLogicalFrameBuffer(s32 methodType) const
{
    return getMethodFrameBuffer(methodType);
}

}  // namespace sead
