#pragma once

#include "Project/Se/SeSourcePose.hpp"

namespace al {
class SeSourcePose3DMtxPtr : public SeSourcePose3DMtxBase {
    SEAD_RTTI_OVERRIDE(SeSourcePose3DMtxPtr, SeSourcePose3DMtxBase)

public:
    SeSourcePose3DMtxPtr(const sead::Matrix34f* pMtx);

    void update() override;

    const sead::Vector3f* get3DPosPtr() const override { return &mPos; }

    const sead::Matrix34f* get3DMtxPtr() const override { return mMtx; }

    const sead::Matrix34f& get3DMtx() const override { return *get3DMtxPtr(); }

    const sead::Matrix34f* mMtx;                    // _8
    sead::Vector3f mPos = {0.0f, 0.0f, 0.0f};  // _10
};
}  // namespace al
