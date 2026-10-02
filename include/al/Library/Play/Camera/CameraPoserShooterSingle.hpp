#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class SimpleCameraShooter;

class CameraPoserShooterSingle : public CameraPoser_RS {
public:
    CameraPoserShooterSingle(const char* pName);

    void loadParam(const ByamlIter& rIter) override;
    void update() override;

private:
    SimpleCameraShooter* mShooter;
    u8 _150[0xc];
    sead::Vector3f mOffset = sead::Vector3f::zero;
};

static_assert(sizeof(CameraPoserShooterSingle) == 0x168);

}  // namespace al
