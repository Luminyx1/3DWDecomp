#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
template <class T>
class DeriveActorGroup;
class RollingCubeMapParts;

class RollingCubeMapPartsGenerator : public LiveActor {
public:
    RollingCubeMapPartsGenerator(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void kill() override;

    void exeDelay();
    void exeGenerate();

    DeriveActorGroup<RollingCubeMapParts>* mRollingCubeMapPartsGroup = nullptr;
    sead::Vector3f mClippingTrans = sead::Vector3f::zero;
    s32 mDelayTime = 0;
    s32 mGenerateInterval = 60;
};
}  // namespace al
