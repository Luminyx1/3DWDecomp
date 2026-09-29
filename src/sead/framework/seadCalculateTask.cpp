#include "framework/seadCalculateTask.h"

#include "framework/seadMethodTreeMgr.h"
#include "prim/seadScopedLock.h"

namespace sead
{
/**
 * Constructs a calculate task and binds its calc node to calc().
 * @param rArg Construction argument supplied by the task manager.
 */
CalculateTask::CalculateTask(const TaskConstructArg& rArg) : TaskBase(rArg)
{
    mCalcNode.bind(sead::Delegate<CalculateTask>{this, &CalculateTask::calc}, "CalculateTask");
}

/**
 * Constructs a named calculate task and binds its calc node to calc().
 * @param rArg Construction argument supplied by the task manager.
 * @param pName Name of the task and its calc node.
 */
CalculateTask::CalculateTask(const TaskConstructArg& rArg, const char* pName)
    : TaskBase(rArg, pName)
{
    mCalcNode.bind(sead::Delegate<CalculateTask>{this, &CalculateTask::calc}, pName);
}

/**
 * Destroys the calculate task.
 */
CalculateTask::~CalculateTask() = default;

/**
 * Gets the type of method tree manager this task can attach to.
 * @return The runtime type info of MethodTreeMgr.
 */
const RuntimeTypeInfo::Interface* CalculateTask::getCorrespondingMethodTreeMgrTypeInfo() const
{
    return MethodTreeMgr::getRuntimeTypeInfoStatic();
}

/**
 * Runs the task's per-frame calculation (no-op by default).
 */
void CalculateTask::calc() {}

/**
 * Attaches the calc node to the system tree, the parent task's calc node or the app tree.
 */
void CalculateTask::attachCalcImpl()
{
    ScopedLock<CriticalSection> lock(&getMethodTreeMgr()->mCS);

    TaskBase* parentTask = parent() ? parent()->value() : nullptr;
    if (mTag == cSystem)
    {
        attachMethodWithCheck(0, &mCalcNode);
    }
    else if (parentTask)
    {
        parentTask->getMethodTreeNode(1)->pushBackChild(&mCalcNode);
    }
    else
    {
        attachMethodWithCheck(1, &mCalcNode);
    }
}

/**
 * Attaches the draw method (calculate tasks have none).
 */
void CalculateTask::attachDrawImpl() {}

/**
 * Detaches the calc node from the method tree.
 */
void CalculateTask::detachCalcImpl()
{
    mCalcNode.detachAll();
}

/**
 * Detaches the draw method (calculate tasks have none).
 */
void CalculateTask::detachDrawImpl() {}

/**
 * Pauses or resumes this task's calc node.
 * @param pause Whether to pause.
 */
void CalculateTask::pauseCalc(bool pause)
{
    if (pause)
    {
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_Self);
    }
    else
    {
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
    }
}

/**
 * Pauses the draw method (calculate tasks have none).
 */
void CalculateTask::pauseDraw(bool) {}

/**
 * Pauses or resumes this task's calc node and its children.
 * @param pause Whether to pause.
 */
void CalculateTask::pauseCalcRec(bool pause)
{
    if (pause)
    {
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_Both);
    }
    else
    {
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
    }
}

/**
 * Pauses the draw method recursively (calculate tasks have none).
 */
void CalculateTask::pauseDrawRec(bool) {}

/**
 * Pauses or resumes the children of this task's calc node.
 * @param pause Whether to pause.
 */
void CalculateTask::pauseCalcChild(bool pause)
{
    if (pause)
    {
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_Child);
    }
    else
    {
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
    }
}

/**
 * Pauses the child draw methods (calculate tasks have none).
 */
void CalculateTask::pauseDrawChild(bool) {}

/**
 * Gets the method tree node for a method type.
 * @param methodType Method type.
 * @return The calc node for types 0 and 1, otherwise nullptr.
 */
MethodTreeNode* CalculateTask::getMethodTreeNode(s32 methodType)
{
    return u32(methodType) < 2 ? &mCalcNode : nullptr;
}
}  // namespace sead
