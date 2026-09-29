#include "Project/Camera/Info/CameraViewRenderInfo.hpp"

namespace al {
/**
 * @brief Creates the render info from the view's data.
 * @param pName The name of the view.
 * @param pViewport The viewport the view is drawn into.
 * @param pViewMtx The view matrix of the camera.
 * @param pProjMtx The projection matrix of the camera.
 */
CameraViewRenderInfo::CameraViewRenderInfo(const char* pName, const sead::Viewport* pViewport,
                                           const sead::Matrix34f* pViewMtx, const sead::Matrix44f* pProjMtx)
    : mName(pName), mViewport(pViewport), mViewMtx(pViewMtx), mProjMtx(pProjMtx) {}
}  // namespace al
