#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class SePlayRail : public LiveActor {
public:
    SePlayRail(const char*);

    void init(const ActorInitInfo&) override;
    void appear() override;
    void control() override;

    void startFirstStepSe();

    const char* mSeName = nullptr;                      // _148
    sead::Vector3f mRailClippingInfo = {0.0f, 0.0f, 0.0f};  // _150
    bool _15c = false;
};
}  // namespace al
