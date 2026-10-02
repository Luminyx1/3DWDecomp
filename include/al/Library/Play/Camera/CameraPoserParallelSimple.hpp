#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {

class CameraPoserParallelSimple : public CameraPoser_RS {
public:
    CameraPoserParallelSimple(const char* pName);

    void init() override;
    void loadParam(const ByamlIter& rIter) override;
    void update() override;

public:
    sead::Vector3f mLookAtOffset = {0.0f, 0.0f, 0.0f};
    f32 mDistance = 1600.0f;
    f32 mAngleH = 0.0f;
    f32 mAngleV = 30.0f;
    u8 _15c[0x4];
};

static_assert(sizeof(CameraPoserParallelSimple) == 0x160);

}  // namespace al
