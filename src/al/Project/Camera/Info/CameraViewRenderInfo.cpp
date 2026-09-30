#include "Project/Camera/Info/CameraViewRenderInfo.hpp"

namespace al {

CameraViewRenderInfo::CameraViewRenderInfo(const char* pName, const sead::Viewport* pViewport,
                                           const sead::Matrix34f* pViewMtx,
                                           const sead::Matrix44f* pProjMtx)
    : name(pName), viewport(pViewport), viewMtx(pViewMtx), projMtx(pProjMtx) {}

}  // namespace al
