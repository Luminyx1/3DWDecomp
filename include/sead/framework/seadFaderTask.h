#pragma once

#include <framework/seadMethodTree.h>
#include <framework/seadTaskBase.h>
#include <prim/seadDelegateEventSlot.h>

namespace sead
{
class LogicalFrameBuffer;

class FaderTaskBase : public TaskBase
{
public:
    enum StartType
    {
        cStartType_Create = 0,
        cStartType_Takeover = 1,
        cStartType_Transit = 2,
        cStartType_Push = 3,
        cStartType_Pop = 4,
        cStartType_None = 5
    };

    enum FaderState
    {
        cFaderState_None = 0,
        cFaderState_FadeOut = 1,
        cFaderState_Wait = 2,
        cFaderState_FadeIn = 3
    };

    struct FadeEvent
    {
        enum Type
        {
            cFadeInStart = 0,
            cFadeInEnd = 1,
            cFadeOutStart = 2,
            cFadeOutEnd = 3
        };

        explicit FadeEvent(Type type) : mType(type) {}

        Type mType;
    };

    FaderTaskBase(const TaskConstructArg& rArg, const char* pName);

    void pauseCalc(bool pause) override;
    void pauseCalcRec(bool pause) override;
    void pauseCalcChild(bool pause) override;
    void enter() override;
    void attachCalcImpl() override;
    void detachCalcImpl() override;
    MethodTreeNode* getMethodTreeNode(s32 methodType) override;
    virtual void doCalc_() {}
    virtual void onFadeEvent_(const FadeEvent&) {}
    virtual void calc();

    void setFrames(s32 fadeOutFrame, s32 waitFrame, s32 fadeInFrame);
    bool startAsCreate_(const CreateArg& rArg);
    bool startAsTakeover_(TaskBase* pFrom, const CreateArg& rArg);
    bool startAsTransit_(TaskBase* pFrom, TaskBase* pTo);
    bool startAsPush_(TaskBase* pFrom, const CreateArg& rArg);
    bool startAsPop_(TaskBase* pFrom, TaskBase* pTo);

    f32 getAlpha() const { return mAlpha; }

protected:
    void onCreateDone_(TaskBase* pTask);
    void calcCore_();
    void setFaderState_(FaderState state);
    bool startCreate_();
    void fadein_();

    StartType mStartType = cStartType_None;
    TaskBase* mFromTask;
    CreateArg mCreateArg;
    TaskBase* mToTask = nullptr;
    DelegateEvent<TaskBase*>::Slot mCreateDoneSlot{this, &FaderTaskBase::onCreateDone_};
    MethodTreeNode mCalcNode{nullptr};
    MethodTreeNode mCalcCoreNode{nullptr};
    s32 mFrame = 0;
    s32 mFadeOutEndFrame = 30;
    s32 mWaitEndFrame;
    s32 mFadeInEndFrame;
    f32 mAlpha;
    bool mFadeInRequested;
    FaderState mFaderState;
};

class FaderTask : public FaderTaskBase
{
public:
    FaderTask(const TaskConstructArg& rArg, const char* pName);
    ~FaderTask() override;

    void pauseDraw(bool pause) override;
    void pauseDrawRec(bool pause) override;
    void pauseDrawChild(bool pause) override;
    void attachDrawImpl() override;
    void detachDrawImpl() override;
    const RuntimeTypeInfo::Interface* getCorrespondingMethodTreeMgrTypeInfo() const override;
    MethodTreeNode* getMethodTreeNode(s32 methodType) override;
    virtual void draw();

protected:
    MethodTreeNode mDrawNode{nullptr};
};

class NullFaderTask : public FaderTaskBase
{
public:
    explicit NullFaderTask(const TaskConstructArg& rArg);

    void pauseDraw(bool) override {}
    void pauseDrawRec(bool) override {}
    void pauseDrawChild(bool) override {}
    void attachDrawImpl() override {}
    void detachDrawImpl() override {}
    const RuntimeTypeInfo::Interface* getCorrespondingMethodTreeMgrTypeInfo() const override;
    MethodTreeNode* getMethodTreeNode(s32) override { return nullptr; }
    virtual void draw() {}
};

}  // namespace sead
