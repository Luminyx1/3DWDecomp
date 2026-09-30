#pragma once

#include <math/seadMatrix.h>

namespace sead {
class Viewport;
}  // namespace sead

namespace al {

struct CameraViewRenderInfo {
    CameraViewRenderInfo(const char* pName, const sead::Viewport* pViewport,
                         const sead::Matrix34f* pViewMtx, const sead::Matrix44f* pProjMtx);

    const char* name;
    const sead::Viewport* viewport;
    const sead::Matrix34f* viewMtx;
    const sead::Matrix44f* projMtx;
};

}  // namespace al
