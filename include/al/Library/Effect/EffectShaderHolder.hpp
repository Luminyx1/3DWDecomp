#pragma once

#include <basis/seadTypes.h>

namespace agl {
class TextureData;
}

namespace al {
class EffectShaderHolder {
public:
    void setupTextureDepth(const agl::TextureData* pTexture);

    void setDrawPathRenderStateSetCallbackMRT() { mIsDrawPathMRT = true; }

    void setDrawPathRenderStateSetCallbackSRT(bool isDrawDepthShadow) {
        mIsDrawPathDepthShadow = isDrawDepthShadow;
        mIsDrawPathMRT = false;
    }

    u8 _0[0x28];
    bool mIsDrawPathMRT;
    bool mIsDrawPathDepthShadow;
};
}  // namespace al
