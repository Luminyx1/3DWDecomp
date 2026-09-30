#pragma once

#include "Library/Shadow/ShadowMaskBase.hpp"

namespace al {

class ShadowMaskCastOvalCylinder : public ShadowMaskBase {
public:
    ShadowMaskCastOvalCylinder(const char* pName);

    void declare(ShadowMaskDrawCategory category) override;
    void update() override;
    void calcShadowMatrix(sead::Matrix34f* pMtx) override;
    void updateMulti() override;
    void addMulti() override;

    ShadowMaskType getShadowMaskType() const override { return ShadowMaskType::CastOvalCylinder; }

    void calcOvalWrapMtxCylinder(sead::Matrix34f* pOut, sead::Matrix34f mtx,
                                 const sead::Vector3f& rDir, f32 length) const;

    sead::Vector3f mScale;
    sead::Vector3f _f8;
    f32 mExpXZ;
    f32 mExpY;
    f32 mDistYBase;
    u8 _110[0x30];
};

static_assert(sizeof(ShadowMaskCastOvalCylinder) == 0x140);

}  // namespace al
