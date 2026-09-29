#include <framework/seadSingleScreenMethodTreeMgr.h>

namespace sead
{
/**
 * Constructs the manager and links its calc and draw method trees.
 */
SingleScreenMethodTreeMgr::SingleScreenMethodTreeMgr()
{
    mRootCalcNode.setName("sead::RootCalc");
    mSysCalcNode.setName("sead::SysCalc");
    mAppCalcNode.setName("sead::AppCalc");
    mRootDrawNode.setName("sead::RootDraw");
    mSysDrawNode.setName("sead::SysDraw");
    mAppDrawNode.setName("sead::AppDraw");
    mAppDrawFinalNode.setName("sead::AppDrawFinal");

    mRootCalcNode.pushBackChild(&mSysCalcNode);
    mRootCalcNode.pushBackChild(&mAppCalcNode);
    mRootDrawNode.pushBackChild(&mAppDrawNode);
    mRootDrawNode.pushBackChild(&mAppDrawFinalNode);
    mRootDrawNode.pushBackChild(&mSysDrawNode);

    mSysCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
    mSysDrawNode.setPauseFlag(MethodTreeNode::cPause_None);
    mAppCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
    mAppDrawNode.setPauseFlag(MethodTreeNode::cPause_None);
    mAppDrawFinalNode.setPauseFlag(MethodTreeNode::cPause_None);
    mRootCalcNode.setPauseFlag(MethodTreeNode::cPause_Both);
    mRootDrawNode.setPauseFlag(MethodTreeNode::cPause_Both);
}

/**
 * Destroys the manager and its method trees.
 */
SingleScreenMethodTreeMgr::~SingleScreenMethodTreeMgr() = default;

/**
 * Attaches a method node to the tree of a method type.
 * @param methodType the method type
 * @param pNode the node to attach
 */
void SingleScreenMethodTreeMgr::attachMethod(s32 methodType, MethodTreeNode* pNode)
{
    switch (methodType)
    {
    case 0:
        mSysCalcNode.pushBackChild(pNode);
        break;
    case 1:
        mAppCalcNode.pushBackChild(pNode);
        break;
    case 2:
        mSysDrawNode.pushFrontChild(pNode);
        break;
    case 3:
        mAppDrawNode.pushFrontChild(pNode);
        break;
    case 4:
        mAppDrawFinalNode.pushFrontChild(pNode);
        break;
    default:
        break;
    }
}

/**
 * Gets the root node of a method type.
 * @param methodType the method type
 * @return the root node, or nullptr for an unknown method type
 */
MethodTreeNode* SingleScreenMethodTreeMgr::getRootMethodTreeNode(s32 methodType)
{
    switch (methodType)
    {
    case 0:
        return &mSysCalcNode;
    case 1:
        return &mAppCalcNode;
    case 2:
        return &mSysDrawNode;
    case 3:
        return &mAppDrawNode;
    case 4:
        return &mAppDrawFinalNode;
    default:
        return nullptr;
    }
}

/**
 * Pauses or resumes every calc and draw method.
 * @param pause whether to pause
 */
void SingleScreenMethodTreeMgr::pauseAll(bool pause)
{
    if (pause)
    {
        mRootCalcNode.setPauseFlag(MethodTreeNode::cPause_Both);
        mRootDrawNode.setPauseFlag(MethodTreeNode::cPause_Both);
    }
    else
    {
        mRootCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
        mRootDrawNode.setPauseFlag(MethodTreeNode::cPause_None);
    }
}

/**
 * Pauses or resumes the application calc methods.
 * @param pause whether to pause
 */
void SingleScreenMethodTreeMgr::pauseAppCalc(bool pause)
{
    if (pause)
    {
        mAppCalcNode.setPauseFlag(MethodTreeNode::cPause_Both);
    }
    else
    {
        mAppCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
    }
}

/**
 * Calls every calc method.
 */
void SingleScreenMethodTreeMgr::calc()
{
    mRootCalcNode.call();
}

/**
 * Calls every draw method.
 */
void SingleScreenMethodTreeMgr::draw()
{
    mRootDrawNode.call();
}

}  // namespace sead
