#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include "common/aglShaderEnum.h"

namespace agl {
class RenderBuffer;
}

namespace agl::env {
class CubeMap;
}

namespace sead {
class Camera;
class PerspectiveProjection;
}  // namespace sead

namespace al {
class CubeMapDrawInfo;

class CubeMapDrawer {
public:
    CubeMapDrawer(CubeMapDrawInfo* pDrawInfo, agl::ShaderMode* pShaderMode,
                  agl::env::CubeMap* pCubeMap, s32 face, f32 near, f32 far,
                  const sead::Vector3f& rPos, s32 mipLevel, bool isClearColor);
    ~CubeMapDrawer();

    const agl::RenderBuffer* getCubeMapRenderBuffer() const;
    const sead::Matrix34f& getViewMatrix() const;
    const sead::Matrix44f& getProjMatrix() const;
    const sead::Camera& getCamera() const;
    const sead::PerspectiveProjection& getProjection() const;
    f32 getFovy() const;
    f32 getAspect() const;

private:
    CubeMapDrawInfo* mDrawInfo;
    agl::env::CubeMap* mCubeMap;
    agl::ShaderMode* mShaderMode;
};
}  // namespace al
