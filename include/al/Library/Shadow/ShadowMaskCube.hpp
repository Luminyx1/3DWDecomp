#pragma once

#include <agl/common/aglTextureData.h>

#include "Library/Shadow/ShadowMaskBase.hpp"

namespace agl {
class TextureSampler;
}

namespace al {

class ShadowMaskCube : public ShadowMaskBase {
public:
    ShadowMaskCube(const char* pName);
    ~ShadowMaskCube() override;

    void declare(ShadowMaskDrawCategory category) override;
    void update() override;
    void calcShadowMatrix(sead::Matrix34f* pMtx) override;
    void updateMulti() override;
    void addMulti() override;

    ShadowMaskType getShadowMaskType() const override { return ShadowMaskType::Cube; }

    void tryInitTexture(const char* pTextureName);

    sead::Vector3f mScale;
    sead::Vector3f mExp;
    f32 mDistYBase;
    bool _108;
    bool _109;
    agl::TextureData mTextureData;
    agl::TextureSampler* mTextureSampler;
    const char* mTextureBaseName;
    sead::Vector2f _248;
    sead::Vector2f _250;
    sead::Vector2f _258;
    sead::Vector2f _260;
    sead::Vector2f _268;
    f32 mTextureFixedScale;
};

}  // namespace al
