#pragma once

#include <math/seadMatrix.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class MtxConnector;

class EffectObj : public LiveActor {
public:
    EffectObj(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void initAfterPlacement() override;
    void appear() override;
    void makeActorAppeared() override;
    void kill() override;
    void movementPaused(bool isPaused) override;
    const sead::Matrix34f* getBaseMtx() const override { return &mBaseMtx; }
    void control() override;

    void appearBySwitch();
    void killBySwitch();

    sead::Matrix34f mBaseMtx = sead::Matrix34f::ident;
    MtxConnector* mMtxConnector = nullptr;
    bool mIsAppeared = false;
};
}  // namespace al
