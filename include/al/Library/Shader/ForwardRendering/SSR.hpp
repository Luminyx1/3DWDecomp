#pragma once

#include <basis/seadTypes.h>

namespace agl {
class TextureData;
}  // namespace agl

namespace sead {
class LookAtCamera;
}  // namespace sead

namespace al {
class FullScreenQuadModel;
class GraphicsSystemInfo;
class UniformBlock;

/**
 * Screen space reflections.
 */
class SSR {
public:
    SSR(GraphicsSystemInfo* pInfo, s32 bufferNum);
    ~SSR();

    void setCam(const sead::LookAtCamera* pCamera);
    void draw(const agl::TextureData* pTexture) const;

    void setEnable(bool isEnable) { mIsEnable = isEnable; }

    bool isEnable() const { return mIsEnable; }

private:
    GraphicsSystemInfo* mGraphicsSystemInfo;
    UniformBlock* mUniformBlock;
    const sead::LookAtCamera* mCamera = nullptr;
    bool mIsEnable = false;
    f32 _1c = 1.0f;
    FullScreenQuadModel* mFullScreenQuadModel;
};

static_assert(sizeof(SSR) == 0x28);

}  // namespace al
