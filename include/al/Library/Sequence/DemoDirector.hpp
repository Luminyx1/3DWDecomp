#pragma once

#include <basis/seadTypes.h>

namespace alSeFunction {
enum DemoType : s32;
}

namespace al {
class ActorInitInfo;
class AudioDirector;
class EffectSystem;
class LiveActor;

class DemoDirector {
public:
    DemoDirector(s32 maxActors);

    virtual void endInit(const ActorInitInfo& rInfo) {}
    virtual void registerActorWithGroup(LiveActor* pActor, const char* pName) {}
    virtual bool startDemo(const LiveActor* pActor, const char* pName);
    virtual void endDemo(const LiveActor* pActor, const char* pName);

    bool isActiveDemo() const;
    bool isActiveDemo(const LiveActor* pActor) const;
    bool isAnyActiveDemo() const;
    bool isOtherDemoRunning();
    void changeActiveAudioDemoType(alSeFunction::DemoType type);
    void setIsOtherDemoRunning(LiveActor* pActor, bool isRunning);
    const char* getActiveDemoName() const;
    bool requestStartDemo(const LiveActor* pActor, const char* pName);
    bool tryRequestStartDemo(const LiveActor* pActor, const char* pName);
    void requestEndDemo(const LiveActor* pActor, const char* pName);
    void addDemoActor(LiveActor* pActor);
    void removeDemoActor(LiveActor* pActor);
    LiveActor** getDemoActorList() const;
    s32 getDemoActorNum() const;
    void updateDemoActor(EffectSystem* pEffectSystem);

    bool isImmediateDemoSwitch() const { return mIsImmediateDemoSwitch; }
    void resetImmediateDemoSwitch() { mIsImmediateDemoSwitch = false; }

    const char* mActiveDemoName = nullptr;
    LiveActor** mDemoActors = nullptr;
    s32 mDemoActorNum = 0;
    s32 mDemoActorMax;
    LiveActor** mAddDemoActors = nullptr;
    s32 mAddDemoActorNum = 0;
    LiveActor* mOtherDemoActors[20];
    s32 mAudioDemoType = 0;
    bool _d4 = false;
    bool _d5 = false;
    bool mIsImmediateDemoSwitch = false;
    bool mIsUpdatingDemoActor = false;
    AudioDirector* mAudioDirector;
    bool mIsChangedAudioDemoType;
    bool _e1 = false;
    bool _e2 = false;
    bool _e3 = false;
    const LiveActor* mActiveDemoActor = nullptr;
};

static_assert(sizeof(DemoDirector) == 0xf0);
}  // namespace al
