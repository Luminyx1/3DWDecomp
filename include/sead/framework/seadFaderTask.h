#pragma once

#include <framework/seadTaskBase.h>

namespace sead
{
class FaderTaskBase : public TaskBase
{
public:
    struct FadeEvent;

    FaderTaskBase(const TaskConstructArg& arg, const char* name);
    ~FaderTaskBase() override;

    void pauseCalc(bool b) override;
    void pauseCalcRec(bool b) override;
    void pauseCalcChild(bool b) override;
    void enter() override;
    void attachCalcImpl() override;
    void detachCalcImpl() override;
    MethodTreeNode* getMethodTreeNode(s32 method_type) override;
    virtual void doCalc_();
    virtual void onFadeEvent_(const FadeEvent& event);
    virtual void calc();

    bool startAsCreate_(const CreateArg& arg);
    bool startAsTakeover_(TaskBase* from, const CreateArg& arg);
    bool startAsTransit_(TaskBase* from, TaskBase* to);
    bool startAsPush_(TaskBase* from, const CreateArg& arg);
    bool startAsPop_(TaskBase* from, TaskBase* to);

private:
    u8 mFaderTaskBaseWork[0x290];
};

class NullFaderTask : public FaderTaskBase
{
public:
    explicit NullFaderTask(const TaskConstructArg& arg);
    ~NullFaderTask() override;

    void pauseDraw(bool b) override;
    void pauseDrawRec(bool b) override;
    void pauseDrawChild(bool b) override;
    void attachDrawImpl() override;
    void detachDrawImpl() override;
    const RuntimeTypeInfo::Interface* getCorrespondingMethodTreeMgrTypeInfo() const override;
    MethodTreeNode* getMethodTreeNode(s32 method_type) override;
    virtual void draw();
};

}  // namespace sead
