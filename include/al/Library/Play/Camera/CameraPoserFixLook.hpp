#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {

class CameraPoserFixLook : public CameraPoser_RS {
public:
    CameraPoserFixLook(const char* pName);

    void init() override;
    void start(const CameraStartInfo& rInfo) override;
    void loadParam(const ByamlIter& rIter) override;

    const sead::Vector3f* mTargetTrans;
};

static_assert(sizeof(CameraPoserFixLook) == 0x150);

}  // namespace al
