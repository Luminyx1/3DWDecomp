#pragma once

#include "Project/Se/SeSourcePose.hpp"

namespace al {
class SeSourcePose3DPosPtr : public SeSourcePose3D {
    SEAD_RTTI_OVERRIDE(SeSourcePose3DPosPtr, SeSourcePose3D)

public:
    SeSourcePose3DPosPtr(const sead::Vector3f* pPos);

    void update() override {}

    const sead::Vector3f* get3DPosPtr() const override { return mPos; }

    const sead::Vector3f* mPos;  // _8
};
}  // namespace al
