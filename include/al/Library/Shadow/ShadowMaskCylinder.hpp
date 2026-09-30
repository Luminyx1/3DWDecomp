#pragma once

#include "Library/Shadow/ShadowMaskBase.hpp"

namespace al {

class ShadowMaskCylinder : public ShadowMaskBase {
public:
    ShadowMaskCylinder(const char* pName);

    void declare(ShadowMaskDrawCategory category) override;
    void update() override;
    void calcShadowMatrix(sead::Matrix34f* pMtx) override;
    void updateMulti() override;
    void addMulti() override;

    ShadowMaskType getShadowMaskType() const override { return ShadowMaskType::Cylinder; }

    sead::Vector3f mScale;
    sead::Vector3f _f8;
    f32 mExpXZ;
    f32 mExpY;
    f32 mDistYBase;
    bool _110;
};

}  // namespace al
