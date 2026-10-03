#include "Library/Shader/DeferredRendering/CubeMapDrawer.hpp"

#include "common/aglDrawContext.h"
#include "environment/aglCubeMap.h"

#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDrawInfo.hpp"

namespace al {
namespace {
/**
 * Calculates the view rotation of a cube map face.
 * @param rDir Look direction and up vector of the face.
 * @return The view rotation.
 */
sead::Matrix33f calcCubeMapFaceRotate(const CubeMapFaceDir& rDir) {
    sead::LookAtCamera camera(sead::Vector3f::zero, rDir.mAt, rDir.mUp);
    camera.updateViewMatrix();
    return sead::Matrix33f(camera.getMatrix());
}
}  // namespace

CubeMapFaceInfo sCubeMapFaceInfo = {
    {{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
     {{-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
     {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
     {{0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
     {{0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
     {{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}},
    {calcCubeMapFaceRotate(sCubeMapFaceInfo.mDirs[0]),
     calcCubeMapFaceRotate(sCubeMapFaceInfo.mDirs[1]),
     calcCubeMapFaceRotate(sCubeMapFaceInfo.mDirs[2]),
     calcCubeMapFaceRotate(sCubeMapFaceInfo.mDirs[3]),
     calcCubeMapFaceRotate(sCubeMapFaceInfo.mDirs[4]),
     calcCubeMapFaceRotate(sCubeMapFaceInfo.mDirs[5])},
};

/**
 * Gets the render buffer of the cube map.
 * @return The render buffer.
 */
const agl::RenderBuffer* CubeMapDrawer::getCubeMapRenderBuffer() const {
    return &mCubeMap->getRenderBuffer();
}

/**
 * Gets the view matrix of the current face.
 * @return The view matrix.
 */
const sead::Matrix34f& CubeMapDrawer::getViewMatrix() const {
    return mDrawInfo->mCamera.getMatrix();
}

/**
 * Gets the projection matrix of the current face.
 * @return The projection matrix.
 */
const sead::Matrix44f& CubeMapDrawer::getProjMatrix() const {
    return mDrawInfo->mProjection.getProjectionMatrix();
}

/**
 * Gets the camera of the current face.
 * @return The camera.
 */
const sead::Camera& CubeMapDrawer::getCamera() const {
    return mDrawInfo->mCamera;
}

/**
 * Gets the projection of the current face.
 * @return The projection.
 */
const sead::PerspectiveProjection& CubeMapDrawer::getProjection() const {
    return mDrawInfo->mProjection;
}

/**
 * Gets the vertical field of view.
 * @return The field of view.
 */
f32 CubeMapDrawer::getFovy() const {
    return mDrawInfo->mProjection.getFovy();
}

/**
 * Gets the aspect ratio.
 * @return The aspect ratio.
 */
f32 CubeMapDrawer::getAspect() const {
    return mDrawInfo->mProjection.getAspect();
}

/**
 * Starts rendering a cube map face.
 * @param pDrawInfo Draw info.
 * @param pShaderMode Shader mode.
 * @param pCubeMap Cube map to render to.
 * @param face Face index.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param rPos Position of the cube map.
 * @param mipLevel Mip level to render.
 * @param isClearColor Whether the color buffer is used.
 */
CubeMapDrawer::CubeMapDrawer(CubeMapDrawInfo* pDrawInfo, agl::ShaderMode* pShaderMode,
                             agl::env::CubeMap* pCubeMap, s32 face, f32 near, f32 far,
                             const sead::Vector3f& rPos, s32 mipLevel, bool isClearColor)
    : mDrawInfo(pDrawInfo), mCubeMap(pCubeMap), mShaderMode(pShaderMode) {
    pDrawInfo->preDrawCubeMapFace(pCubeMap, face, near, far, rPos, mipLevel, isClearColor);
}

/**
 * Finishes rendering a cube map face.
 */
CubeMapDrawer::~CubeMapDrawer() {
    mCubeMap->postDraw(GameFrameworkNx::getAglDrawContext(),
                       true);
    mCubeMap->end(GameFrameworkNx::getAglDrawContext());
    mCubeMap = nullptr;
}
}  // namespace al
