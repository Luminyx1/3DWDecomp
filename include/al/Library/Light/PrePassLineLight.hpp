#pragma once

#include "Library/Light/LppActor.hpp"

namespace al {

class PrePassLineLight : public PrePassLightPlacementBase<LppLine> {
public:
    PrePassLineLight(const char* pName) : PrePassLightPlacementBase(pName) {}

    void init(const ActorInitInfo& rInfo) override;

    void setByBeginEnd(const sead::Vector3f& rBegin, const sead::Vector3f& rEnd);
};

static_assert(sizeof(PrePassLineLight) == 0x190);

}  // namespace al
