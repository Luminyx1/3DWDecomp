#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class SePlayRail : public LiveActor {
public:
    SePlayRail(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void appear() override;
    void control() override;

    void startFirstStepSe();

    const char* mSeName = nullptr;
    sead::Vector3f mRailClippingPos = {0.0f, 0.0f, 0.0f};
    bool mIsValidSe = false;
};
}  // namespace al
