#include <framework/seadFaderTask.h>

#include <framework/seadFramework.h>
#include <framework/seadMethodTreeMgr.h>
#include <framework/seadTaskMgr.h>
#include <gfx/seadCamera.h>
#include <gfx/seadPrimitiveRenderer.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>

namespace sead
{
class ScreenFiller
{
public:
    explicit ScreenFiller(const LogicalFrameBuffer* pFrameBuffer)
        : mViewport(*pFrameBuffer), mProjection(1.0f, 2000.0f, mViewport), mCamera(mProjection),
          mFrameBuffer(pFrameBuffer)
    {
    }

    /**
     * Fills the whole screen with a translucent quad.
     * @param alpha the opacity of the quad
     */
    void draw(f32 alpha)
    {
        PrimitiveDrawer drawer(nullptr);
        mViewport.apply(nullptr, *mFrameBuffer);
        drawer.setProjection(&mProjection);
        drawer.setCamera(&mCamera);
        drawer.begin();

        BoundBox2f box(-320.0f, -240.0f, 320.0f, 240.0f);
        PrimitiveDrawer::QuadArg arg;
        drawer.drawQuad(arg.setBoundBox(box, 0.0f).setColor(Color4f(1.0f, 1.0f, 1.0f, alpha)));

        drawer.end();
    }

private:
    Viewport mViewport;
    OrthoProjection mProjection;
    OrthoCamera mCamera;
    const LogicalFrameBuffer* mFrameBuffer;
};

/**
 * Constructs the fader used when a task request names no fader; it switches tasks instantly.
 * @param rArg construction argument supplied by the task manager
 */
NullFaderTask::NullFaderTask(const TaskConstructArg& rArg) : FaderTaskBase(rArg, "NullFaderTask")
{
    setFrames(0, 0, 0);
}

/**
 * Constructs a fader and binds its calc nodes.
 * @param rArg construction argument supplied by the task manager
 * @param pName name of the task and its calc node
 */
FaderTaskBase::FaderTaskBase(const TaskConstructArg& rArg, const char* pName)
    : TaskBase(rArg, pName)
{
    mFaderState = cFaderState_None;
    mFromTask = nullptr;
    mWaitEndFrame = 31;
    mFadeInEndFrame = 61;
    mFadeInRequested = false;
    mAlpha = 0.0f;
    mCalcNode.setName(pName);
    mCalcNode.setPauseFlag(MethodTreeNode::cPause_Both);
    mCalcNode.bind(Delegate<FaderTaskBase>(this, &FaderTaskBase::calc), pName);
    mCalcCoreNode.bind(Delegate<FaderTaskBase>(this, &FaderTaskBase::calcCore_), "calcCore_");
}

/**
 * Receives the task created by this fader and requests the fade in when appropriate.
 * @param pTask the created task
 */
void FaderTaskBase::onCreateDone_(TaskBase* pTask)
{
    mToTask = pTask;
    pTask->mInternalFlag.setBit(0);

    switch (mStartType)
    {
    case cStartType_Create:
    case cStartType_Takeover:
    case cStartType_Push:
        fadein_();
        break;
    default:
        break;
    }
}

/**
 * Advances the fader state machine and performs the task switch once the screen is covered.
 */
void FaderTaskBase::calcCore_()
{
    switch (mFaderState)
    {
    case cFaderState_FadeOut:
        if (mFrame > mFadeOutEndFrame)
        {
            CriticalSection& cs = mTaskMgr->mCriticalSection;
            mFrame = mFadeOutEndFrame + 1;

            if (cs.tryLock())
            {
                onFadeEvent_(FadeEvent(FadeEvent::cFadeOutEnd));
                mFaderState = cFaderState_Wait;

                switch (mStartType)
                {
                case cStartType_Takeover:
                    startCreate_();
                    break;
                case cStartType_Transit:
                    mFromTask->pauseCalc(true);
                    mFromTask->pauseDraw(true);
                    mToTask->pauseCalc(false);
                    mToTask->pauseDraw(false);
                    {
                        TaskEvent event(1);
                        mFromTask->onEvent(event);
                    }

                    {
                        TaskEvent event(2);
                        mToTask->onEvent(event);
                    }

                    fadein_();
                    break;
                case cStartType_Push:
                    mFromTask->pauseCalc(true);
                    mFromTask->pauseDraw(true);
                    break;
                case cStartType_Pop:
                {
                    TaskBase* task = (mFromTask->parent() != nullptr) ? mFromTask->parent()->value() : nullptr;
                    TaskBase* to = mToTask;
                    mTaskMgr->doDestroyTask_(mFromTask);

                    if (to != nullptr)
                    {
                        while (task != mToTask)
                        {
                            mTaskMgr->doDestroyTask_(task);
                            task = (task->parent() != nullptr) ? task->parent()->value() : nullptr;
                        }
                    }

                    task->pauseCalc(false);
                    task->pauseDraw(false);
                    task->onEvent(TaskEvent(0));
                    fadein_();
                    break;
                }
                default:
                    break;
                }

                cs.unlock();
            }
        }

        break;
    case cFaderState_Wait:
        if (mFrame > mWaitEndFrame)
        {
            mFrame = mWaitEndFrame + 1;

            if (mFadeInRequested)
            {
                if (mStartType == cStartType_Takeover)
                {
                    TaskEvent event(4);
                    mToTask->onEvent(event);
                }

                onFadeEvent_(FadeEvent(FadeEvent::cFadeInStart));
                mFaderState = cFaderState_FadeIn;
            }
            else
            {
                mFrame = mWaitEndFrame + 1;
            }
        }

        break;
    case cFaderState_FadeIn:
        if (mFrame > mFadeInEndFrame)
        {
            onFadeEvent_(FadeEvent(FadeEvent::cFadeInEnd));
            mFrame = mFadeInEndFrame + 1;
            setFaderState_(cFaderState_None);

            if (mFromTask != nullptr)
            {
                mFromTask->mInternalFlag.resetBit(0);
            }

            if (mToTask != nullptr)
            {
                mToTask->mInternalFlag.resetBit(0);
            }
        }

        break;
    default:
        break;
    }
}

/**
 * Enters the task, starting idle.
 */
void FaderTaskBase::enter()
{
    setFaderState_(cFaderState_None);
}

/**
 * Sets the fader state, detaching the fader's methods when it becomes idle.
 * @param state the new state
 */
void FaderTaskBase::setFaderState_(FaderState state)
{
    mFaderState = state;

    if (state == cFaderState_None)
    {
        detachCalcImpl();
        detachDrawImpl();
    }
}

/**
 * Sets the length of each fading phase.
 * @param fadeOutFrame frames spent fading out
 * @param waitFrame frames spent waiting with the screen covered
 * @param fadeInFrame frames spent fading in
 */
void FaderTaskBase::setFrames(s32 fadeOutFrame, s32 waitFrame, s32 fadeInFrame)
{
    mFadeOutEndFrame = fadeOutFrame;
    mWaitEndFrame = fadeOutFrame + waitFrame;
    mFadeInEndFrame = fadeOutFrame + waitFrame + fadeInFrame;
}

/**
 * Starts fading to a newly created task.
 * @param rArg the creation settings of the new task
 * @return whether the fade started
 */
bool FaderTaskBase::startAsCreate_(const CreateArg& rArg)
{
    ScopedLock<CriticalSection> lock(&mTaskMgr->mCriticalSection);

    if (mFaderState != cFaderState_None)
    {
        return false;
    }

    mStartType = cStartType_Create;
    mCreateArg = rArg;

    if (!startCreate_())
    {
        return false;
    }

    mFaderState = cFaderState_Wait;
    mFrame = mFadeOutEndFrame;
    mFromTask = nullptr;
    mFadeInRequested = false;
    mAlpha = 1.0f;
    attachCalcDraw();
    return true;
}

/**
 * Requests the creation of the destination task, destroying the source task first for takeovers.
 * @return whether the request was accepted
 */
bool FaderTaskBase::startCreate_()
{
    ScopedLock<CriticalSection> lock(&mTaskMgr->mCriticalSection);

    if (mStartType == cStartType_Takeover)
    {
        mTaskMgr->doDestroyTask_(mFromTask);
        mFromTask = nullptr;
    }

    return mTaskMgr->doRequestCreateTask_(mCreateArg, &mCreateDoneSlot);
}

/**
 * Starts fading from a task to a newly created task that replaces it.
 * @param pFrom the task to replace
 * @param rArg the creation settings of the new task
 * @return whether the fade started
 */
bool FaderTaskBase::startAsTakeover_(TaskBase* pFrom, const CreateArg& rArg)
{
    ScopedLock<CriticalSection> lock(&mTaskMgr->mCriticalSection);

    if (mFaderState != cFaderState_None)
    {
        return false;
    }

    mStartType = cStartType_Takeover;

    if (!mTaskMgr->changeTaskState_(pFrom, cDying))
    {
        return false;
    }

    mFromTask = pFrom;
    pFrom->mInternalFlag.setBit(0);
    TaskEvent event(3);
    mFromTask->onEvent(event);
    mCreateArg = rArg;
    mFrame = 0;
    mFaderState = cFaderState_FadeOut;
    mFadeInRequested = false;
    mAlpha = 0.0f;
    attachCalcDraw();
    return true;
}

/**
 * Starts fading between two existing tasks.
 * @param pFrom the task to fade from
 * @param pTo the task to fade to
 * @return whether the fade started
 */
bool FaderTaskBase::startAsTransit_(TaskBase* pFrom, TaskBase* pTo)
{
    ScopedLock<CriticalSection> lock(&mTaskMgr->mCriticalSection);

    if (mFaderState != cFaderState_None)
    {
        return false;
    }

    mFromTask = pFrom;
    mStartType = cStartType_Transit;
    mToTask = pTo;
    mFromTask->mInternalFlag.setBit(0);
    mToTask->mInternalFlag.setBit(0);
    mFrame = 0;
    mFaderState = cFaderState_FadeOut;
    mFadeInRequested = false;
    mAlpha = 0.0f;
    attachCalcDraw();
    return true;
}

/**
 * Starts fading from a task to a newly created child task.
 * @param pFrom the task to fade from
 * @param rArg the creation settings of the new task
 * @return whether the fade started
 */
bool FaderTaskBase::startAsPush_(TaskBase* pFrom, const CreateArg& rArg)
{
    ScopedLock<CriticalSection> lock(&mTaskMgr->mCriticalSection);

    if (mFaderState != cFaderState_None)
    {
        return false;
    }

    mStartType = cStartType_Push;
    mCreateArg = rArg;

    if (!startCreate_())
    {
        return false;
    }

    mFromTask = pFrom;
    pFrom->mInternalFlag.setBit(0);
    mFrame = 0;
    mFaderState = cFaderState_FadeOut;
    mFadeInRequested = false;
    mAlpha = 0.0f;
    attachCalcDraw();
    return true;
}

/**
 * Starts fading from a task back to one of its ancestors, destroying the tasks in between.
 * @param pFrom the task to fade from
 * @param pTo the ancestor to fade to, or nullptr for the parent
 * @return whether the fade started
 */
bool FaderTaskBase::startAsPop_(TaskBase* pFrom, TaskBase* pTo)
{
    ScopedLock<CriticalSection> lock(&mTaskMgr->mCriticalSection);

    if (mFaderState != cFaderState_None)
    {
        return false;
    }

    mFromTask = pFrom;
    mStartType = cStartType_Pop;
    pFrom->mInternalFlag.setBit(0);
    mToTask = pTo;
    mFrame = 0;
    mFaderState = cFaderState_FadeOut;
    mFadeInRequested = false;
    mAlpha = 0.0f;
    attachCalcDraw();
    return true;
}

/**
 * Requests the fade in once the destination task is ready.
 */
void FaderTaskBase::fadein_()
{
    mFadeInRequested = true;
}

/**
 * Attaches the calc nodes to the task manager and the system calc tree.
 */
void FaderTaskBase::attachCalcImpl()
{
    mTaskMgr->mCalcDestructionTreeNode.pushBackChild(&mCalcCoreNode);
    attachMethodWithCheck(1, &mCalcNode);
    mCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
}

/**
 * Pauses or resumes this fader's calc node.
 * @param pause whether to pause
 */
void FaderTaskBase::pauseCalc(bool pause)
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
 * Pauses or resumes this fader's calc node and its children.
 * @param pause whether to pause
 */
void FaderTaskBase::pauseCalcRec(bool pause)
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
 * Pauses or resumes the children of this fader's calc node.
 * @param pause whether to pause
 */
void FaderTaskBase::pauseCalcChild(bool pause)
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
 * Updates the fade alpha for the current phase and advances the frame counter.
 */
void FaderTaskBase::calc()
{
    switch (mFaderState)
    {
    case cFaderState_FadeOut:
        if (mFadeOutEndFrame > 0)
        {
            mAlpha = static_cast<f32>(mFrame) / static_cast<f32>(mFadeOutEndFrame + 1);
        }
        else
        {
            mAlpha = 0.0f;
        }

        if (mFrame == 0)
        {
            onFadeEvent_(FadeEvent(FadeEvent::cFadeOutStart));
        }

        doCalc_();
        mFrame++;
        break;
    case cFaderState_Wait:
        doCalc_();
        mAlpha = 1.0f;
        mFrame++;
        break;
    case cFaderState_FadeIn:
        doCalc_();

        if (mFadeInEndFrame - mWaitEndFrame != 0)
        {
            mAlpha = 1.0f - static_cast<f32>(mFrame - mWaitEndFrame) /
                                static_cast<f32>(mFadeInEndFrame - mWaitEndFrame);
        }

        mFrame++;
        break;
    default:
        break;
    }
}

/**
 * Detaches the calc node from the method tree.
 */
void FaderTaskBase::detachCalcImpl()
{
    mCalcNode.detachAll();
}

/**
 * Gets the method tree node for a method type.
 * @param methodType the method type
 * @return the calc node for types 0 and 1, otherwise nullptr
 */
MethodTreeNode* FaderTaskBase::getMethodTreeNode(s32 methodType)
{
    return u32(methodType) < 2 ? &mCalcNode : nullptr;
}

/**
 * Constructs a fader that fills the screen and binds its draw node.
 * @param rArg construction argument supplied by the task manager
 * @param pName name of the task and its nodes
 */
FaderTask::FaderTask(const TaskConstructArg& rArg, const char* pName) : FaderTaskBase(rArg, pName)
{
    mDrawNode.setName(pName);
    mDrawNode.setPauseFlag(MethodTreeNode::cPause_Both);
    mDrawNode.bind(Delegate<FaderTask>(this, &FaderTask::draw), pName);
}

/**
 * Destroys the fader.
 */
FaderTask::~FaderTask() = default;

/**
 * Attaches the draw node to the final application draw tree.
 */
void FaderTask::attachDrawImpl()
{
    attachMethodWithCheck(4, &mDrawNode);
    mDrawNode.setPauseFlag(MethodTreeNode::cPause_None);
}

/**
 * Detaches the draw node from the method tree.
 */
void FaderTask::detachDrawImpl()
{
    mDrawNode.detachAll();
}

/**
 * Pauses or resumes this fader's draw node.
 * @param pause whether to pause
 */
void FaderTask::pauseDraw(bool pause)
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
 * Pauses or resumes this fader's draw node and its children.
 * @param pause whether to pause
 */
void FaderTask::pauseDrawRec(bool pause)
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
 * Pauses or resumes the children of this fader's draw node.
 * @param pause whether to pause
 */
void FaderTask::pauseDrawChild(bool pause)
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
 * Fills the final draw frame buffer with the current fade alpha.
 */
void FaderTask::draw()
{
    ScreenFiller filler(getFramework()->getMethodLogicalFrameBuffer(3));
    filler.draw(mAlpha);
}

/**
 * Gets the type of method tree manager this fader can attach to.
 * @return the runtime type info of MethodTreeMgr
 */
const RuntimeTypeInfo::Interface* FaderTask::getCorrespondingMethodTreeMgrTypeInfo() const
{
    return MethodTreeMgr::getRuntimeTypeInfoStatic();
}

/**
 * Gets the method tree node for a method type.
 * @param methodType the method type
 * @return the calc node for types 0 and 1, the draw node for types 2 to 4, otherwise nullptr
 */
MethodTreeNode* FaderTask::getMethodTreeNode(s32 methodType)
{
    switch (methodType)
    {
    case 2:
    case 3:
    case 4:
        return &mDrawNode;
    default:
        return FaderTaskBase::getMethodTreeNode(methodType);
    }
}

/**
 * Gets the type of method tree manager this fader can attach to.
 * @return the runtime type info of MethodTreeMgr
 */
const RuntimeTypeInfo::Interface* NullFaderTask::getCorrespondingMethodTreeMgrTypeInfo() const
{
    return MethodTreeMgr::getRuntimeTypeInfoStatic();
}

}  // namespace sead
