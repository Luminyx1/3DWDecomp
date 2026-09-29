#pragma once

#include <framework/seadFramework.h>
#include <heap/seadHeap.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <prim/seadSafeString.h>

namespace sead
{
class GameFrameworkUnk
{
public:
    virtual ~GameFrameworkUnk();
    virtual void unk2() = 0;
    virtual void unk3() = 0;
    virtual void unk4() = 0;
    virtual void unk5() = 0;
    virtual void unk6() = 0;
    virtual void unk7() = 0;
    virtual void unk8() = 0;
    virtual void unk9(bool) = 0;
};

class GameFramework : public Framework
{
    SEAD_RTTI_OVERRIDE(GameFramework, Framework);

public:
    static void initialize(const Framework::InitializeArg&);

    GameFramework();
    ~GameFramework() override;

    void createSystemTasks(TaskBase* base,
                           const Framework::CreateSystemTaskArg& createSystemTaskArg) override;
    void quitRun_(Heap* heap) override;
    virtual void createControllerMgr(TaskBase* base);
    virtual void createHostIOMgr(TaskBase* base, HostIOMgr::Parameter* hostioParam, Heap* heap);
    virtual void createProcessMeter(TaskBase* base);
    virtual void createSeadMenuMgr(TaskBase* base);
    virtual void createInfLoopChecker(TaskBase* base, const TickSpan&, int);
    virtual void createCuckooClock(TaskBase* base);
    virtual float calcFps() = 0;
    virtual void saveScreenShot(const SafeString&) {}
    virtual bool isScreenShotBusy() const { return false; }
    virtual void waitStartDisplayLoop_();
    virtual void initHostIO_();

    void startDisplay();
    void lockFrameDrawContext();
    void unlockFrameDrawContext();

private:
    int mDisplayStarted = 0;
    sead::SafeString mUnk1 = "";
    sead::SafeString mUnk2 = "";
    sead::SafeString mUnk3 = "";
    GameFrameworkUnk* mUnk4 = nullptr;
    void (*mUnk5)(bool) = nullptr;
    void (*mUnk6)(bool);
};
}  // namespace sead
