#include <framework/seadTaskMgr.h>

#include <framework/seadFaderTask.h>
#include <framework/seadFramework.h>
#include <framework/seadMethodTreeMgr.h>
#include <framework/seadTaskBase.h>
#include <heap/seadExpHeap.h>
#include <prim/seadSafeString.h>
#include <prim/seadScopedLock.h>
#include <thread/seadDelegateThread.h>

namespace sead
{
static s32 sDefaultPreparePriority = Thread::cDefaultPriority;

/**
 * Constructs a task manager initialization argument with default values.
 * @param rRootTaskArg Creation argument of the root task.
 */
TaskMgr::InitializeArg::InitializeArg(const TaskBase::CreateArg& rRootTaskArg)
    : roottask_create_arg(rRootTaskArg)
{
}

/**
 * Constructs the task manager, creates its root heaps, prepare thread and null fader, and requests
 * the root task.
 * @param rArg Initialization argument.
 */
TaskMgr::TaskMgr(const InitializeArg& rArg)
    : mParentFramework(rArg.parent_framework), mPrepareThread(nullptr), mNullFaderTask(nullptr),
      mMaxCreateQueueSize(rArg.create_queue_size), mInitializeArg(rArg)
{
    mRootTask = nullptr;

    const s32 rootHeapNum = HeapMgr::getRootHeapNum();
    for (s32 i = 0; i < rootHeapNum; i++)
    {
        mHeapArray.mHeaps[i] = ExpHeap::create(0, "sead::TaskMgr", HeapMgr::getRootHeap(i), 8,
                                               Heap::cHeapDirection_Forward, false);
    }

    doInit_();
    mRootTaskCreateArg = rArg.roottask_create_arg;
    beginCreateRootTask_();
}

/**
 * Creates the prepare thread, the null fader task and the task creation queue.
 */
void TaskMgr::doInit_()
{
    Heap* heap = mHeapArray.getPrimaryHeap();

    mPrepareThread = new (heap) DelegateThread(
        "Prepare Thread", new (heap) Delegate2<TaskMgr, Thread*, s64>(this, &TaskMgr::prepare_),
        heap,
        mInitializeArg.prepare_priority == -1 ? sDefaultPreparePriority :
                                                mInitializeArg.prepare_priority,
        MessageQueue::BlockType::Blocking, 0x7fffffff, mInitializeArg.prepare_stack_size, 0x20);
    mPrepareThread->start();

    HeapArray heapArray;
    heapArray.mHeaps[0] = heap;
    TaskConstructArg arg;
    arg.heap_array = &heapArray;
    arg.mgr = this;
    arg.param = nullptr;
    mNullFaderTask = new (heap) NullFaderTask(arg);
    mNullFaderTask->setName("NullFader");
    mNullFaderTask->mState = TaskBase::cRunning;

    mTaskCreateContextMgr = new (heap) TaskCreateContextMgr(mMaxCreateQueueSize, heap);
}

/**
 * Requests the creation of the root task.
 */
void TaskMgr::beginCreateRootTask_()
{
    mRootTask = nullptr;
    mRootTaskCreateArg.tag = TaskBase::cApp;
    mRootTaskCreateArg.created_task = &mRootTask;
    mRootTaskCreateArg.fader = nullptr;
    requestCreateTask(mRootTaskCreateArg);
}

/**
 * Creates a task manager on the heap given in the initialization argument.
 * @param rArg Initialization argument.
 * @return The created task manager.
 */
TaskMgr* TaskMgr::initialize(const InitializeArg& rArg)
{
    return new (rArg.heap) TaskMgr(rArg);
}

/**
 * Initializes host I/O (no-op in release builds).
 */
void TaskMgr::initHostIO() {}

/**
 * Destroys the prepare thread, the root task and the task manager's heaps.
 */
void TaskMgr::finalize()
{
    if (mPrepareThread != nullptr)
    {
        mPrepareThread->quitAndDestroySingleThread(false);
        delete mPrepareThread;
        mPrepareThread = nullptr;
    }

    if (mRootTask != nullptr)
    {
        destroyTaskSync(mRootTask);
        mRootTask = nullptr;
    }

    s32 rootHeapNum = HeapMgr::getRootHeapNum();
    for (s32 i = 0; i < rootHeapNum; i++)
    {
        Heap* heap = mHeapArray.mHeaps[i];
        if (heap)
        {
            heap->destroy();
            mHeapArray.mHeaps[i] = nullptr;
        }
    }
}

/**
 * Destroys a task and its children if the method tree is not in use.
 * @param pTask Task to destroy.
 */
void TaskMgr::destroyTaskSync(TaskBase* pTask)
{
    if (mParentFramework->mMethodTreeMgr->mCS.tryLock())
    {
        doDestroyTask_(pTask);
        mParentFramework->mMethodTreeMgr->mCS.unlock();
    }
}

/**
 * Prepares the first task in the prepare list and adjusts its heaps; runs on the prepare thread.
 */
void TaskMgr::prepare_(Thread*, MessageQueue::Element)
{
    TaskBase* task = nullptr;

    mCriticalSection.lock();
    if (mPrepareList.begin() != mPrepareList.end())
    {
        task = *mPrepareList.begin();
    }

    mCriticalSection.unlock();

    if (task)
    {
        ScopedCurrentHeapSetter setter(task->mHeapArray.getPrimaryHeap());
        task->prepare();

        TaskCreateContext* context = mTaskCreateContextMgr->front();
        for (; context; context = mTaskCreateContextMgr->next(context))
        {
            if (context->task == task)
            {
                break;
            }
        }

        for (s32 i = 0; i < HeapMgr::getRootHeapNum(); i++)
        {
            const HeapPolicy& policy = context->arg.heap_policies.getPolicy(i);
            if (policy.adjust)
            {
                task->adjustHeapWithSlackWithoutLock_(i, policy.adjust_slack);
            }
        }

        changeTaskState_(task, TaskBase::cPrepareDone);
    }

    Thread::yield();
}

/**
 * Requests the asynchronous creation of a task, through its fader if it has one.
 * @param rArg Creation argument.
 * @return Whether the request was accepted.
 */
bool TaskMgr::requestCreateTask(const TaskBase::CreateArg& rArg)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);
    if (rArg.fader)
    {
        return rArg.fader->startAsCreate_(rArg);
    }

    return doRequestCreateTask_(rArg, nullptr);
}

/**
 * Creates the heaps of a new task according to its heap policies.
 * @param pHeapArray Heap array to fill.
 * @param rArg Creation argument.
 */
void TaskMgr::createHeap_(HeapArray* pHeapArray, const TaskBase::CreateArg& rArg)
{
    pHeapArray->mPrimaryIndex = rArg.heap_policies.mPrimaryIndex;
    const s32 rootHeapNum = HeapMgr::getRootHeapNum();

    ScopedLock<CriticalSection> lock(&mCriticalSection);
    for (s32 i = 0; i < rootHeapNum; i++)
    {
        auto create = [&]() -> Heap* {
            const HeapPolicy& policy = rArg.heap_policies.getPolicy(i);
            if (policy.dont_create)
            {
                return nullptr;
            }

            Heap* parent = policy.parent;
            if (!parent)
            {
                parent = rArg.parent ? rArg.parent->mHeapArray.mHeaps[i] : mHeapArray.mHeaps[i];
            }

            if (!parent)
            {
                return nullptr;
            }

            const Heap::HeapDirection direction =
                policy.temporary ? Heap::cHeapDirection_Reverse : Heap::cHeapDirection_Forward;
            if (policy.create_slack != 0 && policy.size == 0)
            {
                const size_t allocatable = parent->getMaxAllocatableSize(8);
                if (allocatable <= policy.create_slack)
                {
                    return nullptr;
                }

                return ExpHeap::create(allocatable - policy.create_slack, "TaskHeap", parent, 8,
                                       direction, false);
            }

            return ExpHeap::create(policy.size, "TaskHeap", parent, 8, direction, false);
        };

        pHeapArray->mHeaps[i] = create();
    }
}

/**
 * Creates, prepares and starts a task synchronously.
 * @param rArg Creation argument.
 * @return The created task.
 */
TaskBase* TaskMgr::createTaskSync(const TaskBase::CreateArg& rArg)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    HeapArray heapArray;
    createHeap_(&heapArray, rArg);
    TaskBase* task = doCreateTask_(rArg, &heapArray);

    const s32 rootHeapNum = HeapMgr::getRootHeapNum();
    for (s32 i = 0; i < rootHeapNum; i++)
    {
        Heap* heap = task->mHeapArray.mHeaps[i];
        if (heap)
        {
            heap->setName(task->getName());
        }
    }

    if (rArg.instance_cb)
    {
        rArg.instance_cb(task);
    }

    task->mState = TaskBase::cPrepare;
    {
        ScopedCurrentHeapSetter setter(heapArray.getPrimaryHeap());
        task->prepare();
    }

    for (s32 i = 0; i < HeapMgr::getRootHeapNum(); i++)
    {
        const HeapPolicy& policy = rArg.heap_policies.getPolicy(i);
        if (policy.adjust)
        {
            task->adjustHeapWithSlackWithoutLock_(i, policy.adjust_slack);
        }
    }

    changeTaskState_(task, TaskBase::cPrepareDone);
    changeTaskState_(task, TaskBase::cRunning);

    if (rArg.create_callback)
    {
        DelegateEvent<TaskBase*> event;
        event.connect(*rArg.create_callback);
        event.emit(task);
    }

    if (rArg.created_task)
    {
        *rArg.created_task = task;
    }

    return task;
}

/**
 * Constructs a task with its heaps and attaches it to its parent.
 * @param rArg Creation argument.
 * @param pHeapArray Heaps of the new task.
 * @return The constructed task.
 */
TaskBase* TaskMgr::doCreateTask_(const TaskBase::CreateArg& rArg, HeapArray* pHeapArray)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    TaskClassID classID = rArg.factory;
    TaskBase::Tag tag = rArg.tag;

    TaskBase* task;
    {
        ScopedCurrentHeapSetter setter(pHeapArray->getPrimaryHeap());
        TaskConstructArg arg;
        arg.heap_array = pHeapArray;
        arg.mgr = this;
        arg.param = rArg.parameter;
        task = classID.create(arg);
    }

    task->mClassID = classID;
    task->mTag = tag;

    if (rArg.parent)
    {
        rArg.parent->pushBackChild(task);
    }

    return task;
}

/**
 * Changes the state of a task and moves it between the task lists.
 * @param pTask Task to update.
 * @param state New state.
 * @return Whether the state was changed.
 */
bool TaskMgr::changeTaskState_(TaskBase* pTask, TaskBase::State state)
{
    sead::ScopedLock<CriticalSection> lock{&mCriticalSection};

    if (pTask->mState == state)
    {
        return false;
    }

    switch (state)
    {
    case TaskBase::cPrepare:
        if (pTask->mState != TaskBase::cCreated)
        {
            return false;
        }

        pTask->mState = TaskBase::cPrepare;
        appendToList_(mPrepareList, pTask);

        if (mPrepareThread == nullptr ||
            mPrepareThread->sendMessage(1, MessageQueue::BlockType::NonBlocking))
        {
            return true;
        }

        return false;

    case TaskBase::cPrepareDone:
        pTask->mState = TaskBase::cPrepareDone;
        pTask->mTaskListNode.erase();

        return true;

    case TaskBase::cRunning:
        pTask->mState = TaskBase::cRunning;
        pTask->mTaskListNode.erase();
        appendToList_(mActiveList, pTask);

        pTask->enterCommon();

        return true;

    case TaskBase::cDying:
        pTask->mState = TaskBase::cDying;

        return true;

    case TaskBase::cDestroyable:
        if (pTask->mState != TaskBase::cRunning)
        {
            return false;
        }

        pTask->mState = TaskBase::cDestroyable;
        pTask->detachCalcImpl();
        pTask->detachDrawImpl();
        appendToList_(mDestroyableList, pTask);

        return true;

    case TaskBase::cDead:
        pTask->exit();
        pTask->mState = TaskBase::cDead;
        pTask->mTaskListNode.erase();

        return true;

    default:
        return false;
    }
}

/**
 * Queues a task creation request.
 * @param rArg Creation argument.
 * @param pSlot Slot to connect to the creation event, or nullptr.
 * @return Whether the request was queued.
 */
bool TaskMgr::doRequestCreateTask_(const TaskBase::CreateArg& rArg,
                                   DelegateEvent<TaskBase*>::Slot* pSlot)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    TaskCreateContext* context = mTaskCreateContextMgr->emplaceBack();
    if (!context)
    {
        return false;
    }

    context->arg = rArg;
    DelegateEvent<TaskBase*>::Slot* callback = rArg.create_callback;
    if (pSlot)
    {
        context->event.connect(*pSlot);
    }

    if (callback)
    {
        context->event.connect(*callback);
    }

    return true;
}

/**
 * Inserts a task into a task list, ordered by tag.
 * @param rList List to insert into.
 * @param pTask Task to insert.
 */
void TaskMgr::appendToList_(TaskBase::List& rList, TaskBase* pTask)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    pTask->mTaskListNode.erase();
    for (auto it = rList.begin(); it != rList.end(); ++it)
    {
        if ((*it)->mTag < pTask->mTag)
        {
            TaskBase* task = *it;
            task->mTaskListNode.mList->insertBefore(&task->mTaskListNode, &pTask->mTaskListNode);
            return;
        }
    }

    rList.pushBack(&pTask->mTaskListNode);
}

/**
 * Requests that a new task takes over from an existing one.
 * @param rArg Takeover argument.
 * @return Whether the request was accepted.
 */
bool TaskMgr::requestTakeover(const TaskBase::TakeoverArg& rArg)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    FaderTaskBase* fader = rArg.fader;
    TaskBase* src = rArg.src_task;
    if (!fader)
    {
        fader = mNullFaderTask;
    }

    if (src->mInternalFlag.isOnBit(0))
    {
        return false;
    }

    return fader->startAsTakeover_(src, rArg);
}

/**
 * Requests a transition between two tasks.
 * @param pFrom Task to transition from.
 * @param pTo Task to transition to.
 * @param pFader Fader to use, or nullptr for the null fader.
 * @return Whether the request was accepted.
 */
bool TaskMgr::requestTransition(TaskBase* pFrom, TaskBase* pTo, FaderTaskBase* pFader)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    if (!pFader)
    {
        pFader = mNullFaderTask;
    }

    if (pFrom->mInternalFlag.isOnBit(0) || pTo->mInternalFlag.isOnBit(0))
    {
        return false;
    }

    return pFader->startAsTransit_(pFrom, pTo);
}

/**
 * Requests that a new task is pushed as a child of its source task.
 * @param rArg Push argument.
 * @return Whether the request was accepted.
 */
bool TaskMgr::requestPush(const TaskBase::PushArg& rArg)
{
    if (!rArg.src_task || !rArg.parent || rArg.src_task != rArg.parent)
    {
        return false;
    }

    ScopedLock<CriticalSection> lock(&mCriticalSection);

    FaderTaskBase* fader = rArg.fader;
    if (!fader)
    {
        fader = mNullFaderTask;
    }

    if (rArg.src_task->mInternalFlag.isOnBit(0))
    {
        return false;
    }

    return fader->startAsPush_(rArg.src_task, rArg);
}

/**
 * Pauses the source task and synchronously creates a task as its child.
 * @param rArg Push argument.
 * @return The created task, or nullptr if the argument is invalid.
 */
TaskBase* TaskMgr::pushSync(const TaskBase::PushArg& rArg)
{
    if (!rArg.src_task || !rArg.parent || rArg.src_task != rArg.parent)
    {
        return nullptr;
    }

    ScopedLock<CriticalSection> lock(&mCriticalSection);

    TaskBase* src = rArg.src_task;
    src->pauseCalc(true);
    src->pauseDraw(true);
    return createTaskSync(rArg);
}

/**
 * Requests that a task is popped.
 * @param pTask Task to pop.
 * @param pFader Fader to use, or nullptr for the null fader.
 * @return Whether the request was accepted.
 */
bool TaskMgr::requestPop(TaskBase* pTask, FaderTaskBase* pFader)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    if (!pFader)
    {
        pFader = mNullFaderTask;
    }

    if (pTask->mInternalFlag.isOnBit(0))
    {
        return false;
    }

    return pFader->startAsPop_(pTask, nullptr);
}

/**
 * Synchronously destroys a task and resumes its parent.
 * @param pTask Task to pop.
 * @return Whether the task was popped.
 */
bool TaskMgr::popSync(TaskBase* pTask)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    if (pTask->mInternalFlag.isOnBit(0) || !pTask->parent())
    {
        return false;
    }

    TaskBase* parentTask = pTask->parent()->value();
    if (!parentTask)
    {
        return false;
    }

    doDestroyTask_(pTask);
    parentTask->pauseCalc(false);
    parentTask->pauseDraw(false);

    TaskEvent event;
    parentTask->onEvent(event);
    return true;
}

/**
 * Destroys a task, its children and its heaps.
 * @param pTask Task to destroy.
 */
void TaskMgr::doDestroyTask_(TaskBase* pTask)
{
    sead::ScopedLock<CriticalSection> lock{&mCriticalSection};

    TreeNode* node = pTask->child();
    while (node != nullptr)
    {
        doDestroyTask_(static_cast<TTreeNode<TaskBase*>*>(node)->value());
        node = pTask->child();
    }

    if (changeTaskState_(pTask, TaskBase::cDead))
    {
        pTask->detachAll();

        HeapArray heapArray(pTask->mHeapArray);
        s32 rootHeapNum = HeapMgr::getRootHeapNum();
        for (s32 i = 0; i < rootHeapNum; i++)
        {
            Heap* heap = heapArray.mHeaps[i];
            if (heap != nullptr)
            {
                heap->destroy();
            }
        }
    }
}

/**
 * Requests that tasks are popped until an ancestor is reached.
 * @param pFrom Task to pop.
 * @param pTo Ancestor to return to.
 * @param pFader Fader to use, or nullptr for the null fader.
 * @return Whether the request was accepted.
 */
bool TaskMgr::requestPop(TaskBase* pFrom, TaskBase* pTo, FaderTaskBase* pFader)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    if (!pFader)
    {
        pFader = mNullFaderTask;
    }

    if (pFrom->mInternalFlag.isOnBit(0) || !pFrom->isDescendantOf(pTo))
    {
        return false;
    }

    return pFader->startAsPop_(pFrom, pTo);
}

/**
 * Marks a task and its children for destruction.
 * @param pTask Task to destroy.
 */
void TaskMgr::requestDestroyTask(TaskBase* pTask, FaderTaskBase*)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    if (!pTask->mInternalFlag.isOn(6))
    {
        pTask->mInternalFlag.setBit(1);
        pTask->onDestroy();
    }

    for (auto* child = pTask->child(); child; child = pTask->child())
    {
        requestDestroyTask(child->value(), nullptr);
    }
}

/**
 * Checks whether a task and all of its children can be destroyed.
 * @param pTask Task to check.
 * @return Whether the task can be destroyed.
 */
bool TaskMgr::destroyable_(TaskBase* pTask)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    if (!pTask->mInternalFlag.isOnAll(6) || pTask->mState != TaskBase::cRunning)
    {
        return false;
    }

    for (auto* child = pTask->child(); child; child = pTask->child())
    {
        if (!destroyable_(child->value()))
        {
            return false;
        }
    }

    return true;
}

/**
 * Processes the first queued task creation request.
 */
void TaskMgr::calcCreation_()
{
    if (!mCriticalSection.tryLock())
    {
        return;
    }

    TaskCreateContext* context = mTaskCreateContextMgr->front();
    if (context)
    {
        TaskBase* task = context->task;
        if (task)
        {
            if (task->mState == TaskBase::cPrepareDone)
            {
                changeTaskState_(task, TaskBase::cRunning);
                if (context->arg.created_task)
                {
                    *context->arg.created_task = task;
                }

                context->event.emit(task);
                mTaskCreateContextMgr->erase(context);
            }
        }
        else
        {
            HeapArray heapArray;
            createHeap_(&heapArray, context->arg);
            task = doCreateTask_(context->arg, &heapArray);

            const s32 rootHeapNum = HeapMgr::getRootHeapNum();
            for (s32 i = 0; i < rootHeapNum; i++)
            {
                Heap* heap = task->mHeapArray.mHeaps[i];
                if (heap)
                {
                    heap->setName(task->getName());
                }
            }

            if (context->arg.instance_cb)
            {
                context->arg.instance_cb(task);
            }

            context->task = task;
            changeTaskState_(task, TaskBase::cPrepare);
        }
    }

    mCriticalSection.unlock();
}

/**
 * Destroys the tasks that were marked for destruction.
 */
void TaskMgr::calcDestruction_()
{
    mCalcDestructionTreeNode.call();

    if (!mCriticalSection.tryLock())
    {
        return;
    }

    for (auto it = mActiveList.begin(); it != mActiveList.end();)
    {
        TaskBase* task = *it;
        ++it;
        if (task->mInternalFlag.isOnBit(1) && destroyable_(task))
        {
            changeTaskState_(task, TaskBase::cDestroyable);
        }
    }

    while (mDestroyableList.begin() != mDestroyableList.end())
    {
        TaskBase* task = *mDestroyableList.begin();
        task->detachCalcImpl();
        task->detachDrawImpl();
        task->detachAll();
        doDestroyTask_(task);
    }

    mCriticalSection.unlock();
}

/**
 * Destroys every task, resets the task manager and requests a new root task.
 */
void TaskMgr::destroyAllAndCreateRoot()
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    if (mRootTask)
    {
        destroyTaskSync(mRootTask);
    }

    for (TaskBase::ListNode* node = &*mPrepareList.robustBegin();
         node != &*mPrepareList.robustEnd();)
    {
        auto* next = static_cast<TaskBase::ListNode*>(node->next());
        mPrepareList.erase(node);
        node = next;
    }

    for (TaskBase::ListNode* node = &*mActiveList.robustBegin(); node != &*mActiveList.robustEnd();)
    {
        auto* next = static_cast<TaskBase::ListNode*>(node->next());
        mActiveList.erase(node);
        node = next;
    }

    for (TaskBase::ListNode* node = &*mDyingList.robustBegin(); node != &*mDyingList.robustEnd();)
    {
        auto* next = static_cast<TaskBase::ListNode*>(node->next());
        mDyingList.erase(node);
        node = next;
    }

    for (TaskBase::ListNode* node = &*mDestroyableList.robustBegin();
         node != &*mDestroyableList.robustEnd();)
    {
        auto* next = static_cast<TaskBase::ListNode*>(node->next());
        mDestroyableList.erase(node);
        node = next;
    }

    for (s32 i = 0; i < HeapMgr::getRootHeapNum(); i++)
    {
        Heap* heap = mHeapArray.mHeaps[i];
        if (heap)
        {
            heap->freeAll();
        }
    }

    doInit_();
    beginCreateRootTask_();
}

/**
 * Finds a running task by class.
 * @param rClassID Class to look for.
 * @return The first running task of that class, or nullptr.
 */
TaskBase* TaskMgr::findTask(const TaskClassID& rClassID)
{
    ScopedLock<CriticalSection> lock(&mCriticalSection);

    for (auto it = mActiveList.begin(); it != mActiveList.end(); ++it)
    {
        TaskBase* task = *it;
        if (task->mState == TaskBase::cRunning && task->mClassID == rClassID)
        {
            return task;
        }
    }

    return nullptr;
}

/**
 * Runs the pre-calc processing (task creation).
 */
void TaskMgr::beforeCalc()
{
    calcCreation_();
}

/**
 * Runs the post-calc processing (task destruction).
 */
void TaskMgr::afterCalc()
{
    calcDestruction_();
}

}  // namespace sead
