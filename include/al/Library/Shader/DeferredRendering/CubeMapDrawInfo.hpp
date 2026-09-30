#pragma once

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace agl::env {
class CubeMap;
}

namespace al {
class GraphicsSystemInfo;
class SimpleModelEnv;

struct CubeMapFaceInfo {
    sead::Vector3f mAt[6];
    sead::Vector3f mUp[6];
    sead::Matrix33f mRotate[6];
};

class CubeMapDrawInfo {
public:
    CubeMapDrawInfo(const GraphicsSystemInfo* pGraphicsSystemInfo);

    void preDrawCubeMapFace(agl::env::CubeMap* pCubeMap, s32 face, f32 near, f32 far,
                            const sead::Vector3f& rPos, s32 mipLevel, bool isClearColor);

    sead::PerspectiveProjection mProjection;
    sead::DirectCamera mCamera;
    sead::Viewport mViewport;
    SimpleModelEnv* mSimpleModelEnv;
};

static_assert(sizeof(CubeMapDrawInfo) == 0x158);
}  // namespace al
