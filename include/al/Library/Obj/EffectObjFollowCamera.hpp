#pragma once

#include <math/seadMatrix.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class EffectObjFollowCamera : public LiveActor {
public:
    EffectObjFollowCamera(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void movementPaused(bool isPaused) override;
    void control() override;
    virtual void startAppear();
    virtual void startDisappear();

    void exeWait();
    void exeDisappear();
    bool isDisappearing();
    void restoreNerve(bool isWait);

    sead::Matrix34f mBaseMtx = sead::Matrix34f::ident;
};
}  // namespace al
