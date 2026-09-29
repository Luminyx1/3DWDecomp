#pragma once

#include <math/seadMatrix.h>

namespace sead {
class Viewport;
}

namespace al {
/// Bundles what a view needs to render with a camera: its name, viewport and matrices.
class CameraViewRenderInfo {
public:
    CameraViewRenderInfo(const char* pName, const sead::Viewport* pViewport, const sead::Matrix34f* pViewMtx,
                         const sead::Matrix44f* pProjMtx);

    const char* mName;                // _0
    const sead::Viewport* mViewport;  // _8
    const sead::Matrix34f* mViewMtx;  // _10
    const sead::Matrix44f* mProjMtx;  // _18
};
}  // namespace al
