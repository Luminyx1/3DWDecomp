#pragma once
#include "Library/LiveActor/LiveActor.hpp"

class SePlayObj : public al::LiveActor {
public:
    SePlayObj(const char* pName);
    ~SePlayObj() override;
    void init(const al::ActorInitInfo& rInfo) override;
    void switchTrampleOff();
    void initWithAudioKeeper(const al::ActorInitInfo& rInfo, const char* pAudioKeeperName);
    void exeWait();
    void exeAttached();

private:
    const char* mSeName = nullptr;
    bool mIsValidClipping = false;
    float mClippingRadius = 0.0f;
    bool mIsStartSeBySwitch = false;
    bool mSwitchHandled = false;
    bool mIsAttached = false;
    bool mUpdatePose = false;
    const char* mAudioKeeperName = "SePlayObj";
};
