#include <framework/seadFramework.h>
#include <framework/seadMethodTreeMgr.h>
#include <framework/seadTaskBase.h>
#include <framework/seadTaskMgr.h>
#include <prim/seadSafeString.h>
#include <resource/seadResourceMgr.h>
#include <thread/seadDelegateThread.h>

namespace sead
{
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

void TaskMgr::destroyTaskSync(TaskBase* pTask)
{
    if (mParentFramework->mMethodTreeMgr->mCS.tryLock())
    {
        doDestroyTask_(pTask);
        mParentFramework->mMethodTreeMgr->mCS.unlock();
    }
}

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

}  // namespace sead
