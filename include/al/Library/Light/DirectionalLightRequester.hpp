#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ActorInitInfo;
class DirLightParam;

class DirectionalLightRequester : public LiveActor {
public:
    DirectionalLightRequester(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void control() override;

private:
    DirLightParam* mParam;
    sead::Vector3f mSpecularDirDegree = sead::Vector3f::zero;
    s32 mPriority = 100;
    s32 mInterpFrame = 60;
    bool mIsUsingSpecularDir = false;
};

static_assert(sizeof(DirectionalLightRequester) == 0x168);

}  // namespace al
