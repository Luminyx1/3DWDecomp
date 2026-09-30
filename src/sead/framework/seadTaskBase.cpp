#include "framework/seadTaskBase.h"

#include "framework/seadFramework.h"
#include "framework/seadMethodTreeMgr.h"
#include "framework/seadTaskMgr.h"
#include "prim/seadScopedLock.h"

namespace sead
{
/**
 * Constructs a task creation argument with default values.
 */
TaskBase::CreateArg::CreateArg() = default;

/**
 * Constructs a task creation argument for a task class.
 * @param rFactory Class of the task to create.
 */
TaskBase::CreateArg::CreateArg(const TaskClassID& rFactory) : factory(rFactory) {}

/**
 * Constructs a creation argument for a manager task, adjusting every root heap and only creating
 * the first.
 * @param rClassID Class of the task to create.
 */
TaskBase::MgrTaskArg::MgrTaskArg(const TaskClassID& rClassID) : CreateArg(rClassID)
{
    for (s32 i = 0, num = HeapMgr::getRootHeapNum(); i < num; i++)
    {
        heap_policies.mPolicies[i].adjust = true;
    }

    for (s32 i = 0, num = HeapMgr::getRootHeapNum(); i < num; i++)
    {
        heap_policies.mPolicies[i].dont_create = i != 0;
    }
}

/**
 * Constructs a creation argument for a system manager task.
 * @param rClassID Class of the task to create.
 */
TaskBase::SystemMgrTaskArg::SystemMgrTaskArg(const TaskClassID& rClassID) : MgrTaskArg(rClassID)
{
    tag = cSystem;
}

/**
 * Constructs a takeover argument that replaces a task with a new one.
 * @param pSrc Task to take over from.
 * @param rDst Class of the task to create.
 * @param pFader Fader to use for the transition.
 */
TaskBase::TakeoverArg::TakeoverArg(TaskBase* pSrc, const TaskClassID& rDst, FaderTaskBase* pFader)
    : CreateArg(rDst)
{
    fader = pFader;
    src_task = pSrc;
}

/**
 * Constructs a takeover argument without a source task.
 * @param rDst Class of the task to create.
 * @param pFader Fader to use for the transition.
 */
TaskBase::TakeoverArg::TakeoverArg(const TaskClassID& rDst, FaderTaskBase* pFader) : CreateArg(rDst)
{
    fader = pFader;
    src_task = nullptr;
}

/**
 * Constructs a push argument that creates a task as a child of another.
 * @param pSrc Task to push the new task onto.
 * @param rDst Class of the task to create.
 * @param pFader Fader to use for the transition.
 */
TaskBase::PushArg::PushArg(TaskBase* pSrc, const TaskClassID& rDst, FaderTaskBase* pFader)
    : CreateArg(rDst)
{
    parent = pSrc;
    fader = pFader;
    src_task = pSrc;
}

/**
 * Constructs a push argument without a source task.
 * @param rDst Class of the task to create.
 * @param pFader Fader to use for the transition.
 */
TaskBase::PushArg::PushArg(const TaskClassID& rDst, FaderTaskBase* pFader) : CreateArg(rDst)
{
    fader = pFader;
    src_task = nullptr;
}

/**
 * Constructs a task named "Task".
 * @param rArg Construction argument supplied by the task manager.
 */
TaskBase::TaskBase(const TaskConstructArg& rArg)
    : TTreeNode<TaskBase*>(this), IDisposer(), INamable(""), mParameter(rArg.param),
      mTaskListNode(this), mHeapArray(*rArg.heap_array), mTaskMgr(rArg.mgr)
{
    mInternalFlag.makeAllZero();
    mState = cCreated;
    mTag = cApp;
    setName("Task");
}

/**
 * Constructs a named task.
 * @param rArg Construction argument supplied by the task manager.
 * @param pName Name of the task.
 */
TaskBase::TaskBase(const TaskConstructArg& rArg, const char* pName)
    : TTreeNode<TaskBase*>(this), IDisposer(), INamable(""), mParameter(rArg.param),
      mTaskListNode(this), mHeapArray(*rArg.heap_array), mTaskMgr(rArg.mgr)
{
    mInternalFlag.makeAllZero();
    mState = cCreated;
    mTag = cApp;
    setName(pName);
}

/**
 * Destroys all child tasks and detaches this task from the task tree and its list.
 */
TaskBase::~TaskBase()
{
    if (mTaskMgr)
    {
        while (child())
        {
            mTaskMgr->destroyTaskSync(child()->value());
        }
    }

    mState = cDead;
    detachAll();
    mTaskListNode.erase();
}

/**
 * Attaches this task's calc method.
 */
void TaskBase::attachCalc()
{
    attachCalcImpl();
}

/**
 * Attaches this task's draw method.
 */
void TaskBase::attachDraw()
{
    attachDrawImpl();
}

/**
 * Attaches this task's calc and draw methods.
 */
void TaskBase::attachCalcDraw()
{
    attachCalcImpl();
    attachDrawImpl();
}

/**
 * Prepares the task on the prepare thread (no-op by default).
 */
void TaskBase::prepare() {}

/**
 * Attaches and unpauses the calc and draw methods, then enters the task.
 */
void TaskBase::enterCommon()
{
    attachCalcImpl();
    attachDrawImpl();
    pauseCalc(false);
    pauseDraw(false);
    enter();
}

/**
 * Called when the task starts running (no-op by default).
 */
void TaskBase::enter() {}

/**
 * Called when the task is destroyed (no-op by default).
 */
void TaskBase::exit() {}

/**
 * Handles a task event (no-op by default).
 */
void TaskBase::onEvent(const TaskEvent&) {}

/**
 * Checks whether a task is an ancestor of this task.
 * @param pTask Task to look for.
 * @return Whether pTask is an ancestor of this task.
 */
bool TaskBase::isDescendantOf(TaskBase* pTask) const
{
    for (auto* node = parent(); node; node = node->value()->parent())
    {
        if (!node->value())
        {
            return false;
        }

        if (node->value() == pTask)
        {
            return true;
        }
    }

    return false;
}

/**
 * Adjusts one of this task's heaps if it has not been adjusted yet.
 * @param index Root heap index of the heap.
 */
void TaskBase::adjustHeap(s32 index)
{
    ScopedLock<CriticalSection> lock(&mTaskMgr->mCriticalSection);
    Heap* heap = mHeapArray.getHeap(index);

    if (heap && !mHeapArray.mAdjusted[index])
    {
        mHeapArray.mAdjusted[index] = true;
        heap->adjust();
    }
}

/**
 * Adjusts one of this task's heaps while keeping some free space, without locking the task manager.
 * @param index Root heap index of the heap.
 * @param slack Number of bytes to leave free.
 */
void TaskBase::adjustHeapWithSlackWithoutLock_(s32 index, u32 slack)
{
    Heap* heap = mHeapArray.getHeap(index);

    if (!heap || mHeapArray.mAdjusted[index])
    {
        return;
    }

    mHeapArray.mAdjusted[index] = true;
    void* slackBuffer = slack != 0 ? heap->tryAlloc(slack, 8) : nullptr;
    heap->adjust();

    if (slackBuffer)
    {
        heap->free(slackBuffer);
    }
}

/**
 * Adjusts all of this task's heaps that have not been adjusted yet.
 */
void TaskBase::adjustHeapAll()
{
    ScopedLock<CriticalSection> lock(&mTaskMgr->mCriticalSection);

    for (s32 i = 0; i < HeapMgr::getRootHeapNum(); i++)
    {
        Heap* heap = mHeapArray.getHeap(i);

        if (heap && !mHeapArray.mAdjusted[i])
        {
            mHeapArray.mAdjusted[i] = true;
            heap->adjust();
        }
    }
}

/**
 * Adjusts one of this task's heaps while keeping some free space.
 * @param index Root heap index of the heap.
 * @param slack Number of bytes to leave free.
 */
void TaskBase::adjustHeapWithSlack(s32 index, u32 slack)
{
    ScopedLock<CriticalSection> lock(&mTaskMgr->mCriticalSection);
    adjustHeapWithSlackWithoutLock_(index, slack);
}

/**
 * Requests the asynchronous creation of a task.
 * @param rArg Creation argument.
 * @return Whether the request was queued.
 */
bool TaskBase::requestCreateTask(const CreateArg& rArg)
{
    return mTaskMgr->requestCreateTask(rArg);
}

/**
 * Creates a task synchronously.
 * @param rArg Creation argument.
 * @return The created task.
 */
TaskBase* TaskBase::createTaskSync(const CreateArg& rArg)
{
    return mTaskMgr->createTaskSync(rArg);
}

/**
 * Creates a task synchronously as a child of this task.
 * @param rArg Creation argument, whose parent is set to this task.
 * @return The created task.
 */
TaskBase* TaskBase::createChildTaskSync(CreateArg& rArg)
{
    rArg.parent = this;
    return mTaskMgr->createTaskSync(rArg);
}

/**
 * Requests that a new task takes over from this task.
 * @param rArg Takeover argument.
 * @return Whether the request was accepted.
 */
bool TaskBase::requestTakeover(const TakeoverArg& rArg)
{
    TakeoverArg arg = rArg;
    arg.src_task = this;
    return mTaskMgr->requestTakeover(arg);
}

/**
 * Requests a transition from this task to another task.
 * @param pNextTask Task to transition to.
 * @param pFader Fader to use, or nullptr for the null fader.
 * @return Whether the request was accepted.
 */
bool TaskBase::requestTransition(TaskBase* pNextTask, FaderTaskBase* pFader)
{
    return mTaskMgr->requestTransition(this, pNextTask, pFader);
}

/**
 * Requests that a new task is pushed as a child of this task.
 * @param rArg Push argument.
 * @return Whether the request was accepted.
 */
bool TaskBase::requestPush(const PushArg& rArg)
{
    PushArg arg = rArg;
    arg.src_task = this;
    arg.parent = this;
    return mTaskMgr->requestPush(arg);
}

/**
 * Synchronously pushes a new task as a child of this task.
 * @param rArg Push argument.
 * @return The created task, or nullptr on failure.
 */
TaskBase* TaskBase::pushSync(const PushArg& rArg)
{
    PushArg arg = rArg;
    arg.src_task = this;
    arg.parent = this;
    return mTaskMgr->pushSync(arg);
}

/**
 * Requests that this task is popped.
 * @return Whether the request was accepted.
 */
bool TaskBase::requestPop()
{
    return mTaskMgr->requestPop(this, nullptr);
}

/**
 * Marks this task's destruction as done.
 */
void TaskBase::doneDestroy()
{
    mInternalFlag.setBit(2);
}

/**
 * Gets the framework that owns the task manager.
 * @return The framework.
 */
Framework* TaskBase::getFramework() const
{
    return mTaskMgr->mParentFramework;
}

/**
 * Gets the framework's method tree manager.
 * @return The method tree manager.
 */
MethodTreeMgr* TaskBase::getMethodTreeMgr() const
{
    return mTaskMgr->mParentFramework->getMethodTreeMgr();
}

/**
 * Checks whether another task uses a compatible method tree manager.
 * @param pTask Task to check.
 * @return Whether the tasks' method tree manager types are compatible.
 */
bool TaskBase::isConnectable(TaskBase* pTask) const
{
    return getCorrespondingMethodTreeMgrTypeInfo()->isDerived(
        pTask->getCorrespondingMethodTreeMgrTypeInfo());
}

/**
 * Attaches a method tree node to the framework's method tree.
 * @param methodType Method tree to attach to.
 * @param pNode Node to attach.
 */
void TaskBase::attachMethodWithCheck(s32 methodType, MethodTreeNode* pNode)
{
    getMethodTreeMgr()->attachMethod(methodType, pNode);
}

/**
 * Pauses the calc method of child tasks (no-op by default).
 */
void TaskBase::pauseCalcChild(bool) {}

/**
 * Pauses the draw method of child tasks (no-op by default).
 */
void TaskBase::pauseDrawChild(bool) {}
}  // namespace sead
