#pragma once

#include "Library/Shadow/ShadowMaskBase.hpp"

namespace al {

class ShadowMaskSphere : public ShadowMaskBase {
public:
    ShadowMaskSphere(const char* pName);

    void declare(ShadowMaskDrawCategory category) override;
    void update() override;
    void calcShadowMatrix(sead::Matrix34f* pMtx) override;
    void updateMulti() override;
    void addMulti() override;

    ShadowMaskType getShadowMaskType() const override { return ShadowMaskType::Sphere; }

    f32 mScale;
    f32 mExp;
    bool mIsEnableCollisionCheck;
    f32 mCollisionCheckLength;
};

}  // namespace al
