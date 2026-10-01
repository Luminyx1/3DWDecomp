#include <framework/seadTask.h>

#include <framework/seadMethodTreeMgr.h>
#include <prim/seadScopedLock.h>

namespace sead
{
/**
 * Constructs a task and binds its calc and draw nodes to calc() and draw().
 * @param rArg construction argument supplied by the task manager
 */
Task::Task(const TaskConstructArg& rArg) : TaskBase(rArg)
{
    mCalcNode.setPauseFlag(MethodTreeNode::cPause_Both);
    mDrawNode.setPauseFlag(MethodTreeNode::cPause_Both);
    mCalcNode.bind(Delegate<Task>{this, &Task::calc}, "Task");
    mDrawNode.bind(Delegate<Task>{this, &Task::draw}, "Task");
}

/**
 * Constructs a named task and binds its calc and draw nodes to calc() and draw().
 * @param rArg construction argument supplied by the task manager
 * @param pName name of the task and its nodes
 */
Task::Task(const TaskConstructArg& rArg, const char* pName) : TaskBase(rArg, pName)
{
    mCalcNode.setPauseFlag(MethodTreeNode::cPause_Both);
    mDrawNode.setPauseFlag(MethodTreeNode::cPause_Both);
    mCalcNode.bind(Delegate<Task>{this, &Task::calc}, pName);
    mDrawNode.bind(Delegate<Task>{this, &Task::draw}, pName);
}

/**
 * Destroys the task.
 */
Task::~Task() = default;

/**
 * Gets the type of method tree manager this task can attach to.
 * @return the runtime type info of MethodTreeMgr
 */
const RuntimeTypeInfo::Interface* Task::getCorrespondingMethodTreeMgrTypeInfo() const
{
    return MethodTreeMgr::getRuntimeTypeInfoStatic();
}

/**
 * Runs the task's per-frame calculation (no-op by default).
 */
void Task::calc() {}

/**
 * Runs the task's per-frame drawing (no-op by default).
 */
void Task::draw() {}

/**
 * Attaches the calc node to the system tree, the parent task's calc node or the app tree.
 */
void Task::attachCalcImpl()
{
    ScopedLock<CriticalSection> lock(&getMethodTreeMgr()->mCS);

    TaskBase* parentTask = (parent() != nullptr) ? parent()->value() : nullptr;

    if (mTag == cSystem)
    {
        attachMethodWithCheck(0, &mCalcNode);
    }
    else if (parentTask != nullptr)
    {
        parentTask->getMethodTreeNode(1)->pushBackChild(&mCalcNode);
    }
    else
    {
        attachMethodWithCheck(1, &mCalcNode);
    }
}

/**
 * Attaches the draw node to the system tree, the parent task's draw node or the app tree.
 */
void Task::attachDrawImpl()
{
    ScopedLock<CriticalSection> lock(&getMethodTreeMgr()->mCS);

    TaskBase* parentTask = (parent() != nullptr) ? parent()->value() : nullptr;

    if (mTag == cSystem)
    {
        attachMethodWithCheck(2, &mDrawNode);
    }
    else if (parentTask != nullptr)
    {
        parentTask->getMethodTreeNode(3)->pushFrontChild(&mDrawNode);
    }
    else
    {
        attachMethodWithCheck(3, &mDrawNode);
    }
}

/**
 * Detaches the calc node from the method tree.
 */
void Task::detachCalcImpl()
{
    mCalcNode.detachAll();
}

/**
 * Detaches the draw node from the method tree.
 */
void Task::detachDrawImpl()
{
    mDrawNode.detachAll();
}

/**
 * Pauses or resumes this task's calc node.
 * @param pause whether to pause
 */
void Task::pauseCalc(bool pause)
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
 * Pauses or resumes this task's draw node.
 * @param pause whether to pause
 */
void Task::pauseDraw(bool pause)
{
    if (pause)
    {
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_Self);
    }
    else
    {
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_None);
    }
}

/**
 * Pauses or resumes this task's calc node and its children.
 * @param pause whether to pause
 */
void Task::pauseCalcRec(bool pause)
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
 * Pauses or resumes this task's draw node and its children.
 * @param pause whether to pause
 */
void Task::pauseDrawRec(bool pause)
{
    if (pause)
    {
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_Both);
    }
    else
    {
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_None);
    }
}

/**
 * Pauses or resumes the children of this task's calc node.
 * @param pause whether to pause
 */
void Task::pauseCalcChild(bool pause)
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
 * Pauses or resumes the children of this task's draw node.
 * @param pause whether to pause
 */
void Task::pauseDrawChild(bool pause)
{
    if (pause)
    {
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_Child);
    }
    else
    {
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_None);
    }
}

/**
 * Gets the method tree node for a method type.
 * @param methodType the method type
 * @return the calc node for types 0 and 1, the draw node for types 2 to 4, otherwise nullptr
 */
MethodTreeNode* Task::getMethodTreeNode(s32 methodType)
{
    switch (methodType)
    {
    case 0:
    case 1:
        return &mCalcNode;
    case 2:
    case 3:
    case 4:
        return &mDrawNode;
    default:
        return nullptr;
    }
}

}  // namespace sead
