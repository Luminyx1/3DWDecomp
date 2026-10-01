#include "Library/Shader/DeferredRendering/CubeMapDrawer.hpp"

#include <math/seadMathCalcCommon.h>
#include "common/aglDrawContext.h"
#include "environment/aglCubeMap.h"

#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/SimpleModelEnv.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDrawInfo.hpp"

namespace al {
namespace {
sead::Matrix33f calcCubeMapFaceRotate(const sead::Vector3f& rAt, const sead::Vector3f& rUp) {
    sead::LookAtCamera camera(sead::Vector3f::zero, rAt, rUp);
    camera.updateViewMatrix();
    return sead::Matrix33f(camera.getMatrix());
}

CubeMapFaceInfo sCubeMapFaceInfo = {
    {{1.0f, 0.0f, 0.0f},
     {-1.0f, 0.0f, 0.0f},
     {0.0f, 1.0f, 0.0f},
     {0.0f, -1.0f, 0.0f},
     {0.0f, 0.0f, -1.0f},
     {0.0f, 0.0f, 1.0f}},
    {{0.0f, 1.0f, 0.0f},
     {0.0f, 1.0f, 0.0f},
     {0.0f, 0.0f, 1.0f},
     {0.0f, 0.0f, -1.0f},
     {0.0f, 1.0f, 0.0f},
     {0.0f, 1.0f, 0.0f}},
    {calcCubeMapFaceRotate(sCubeMapFaceInfo.mAt[0], sCubeMapFaceInfo.mUp[0]),
     calcCubeMapFaceRotate(sCubeMapFaceInfo.mAt[1], sCubeMapFaceInfo.mUp[1]),
     calcCubeMapFaceRotate(sCubeMapFaceInfo.mAt[2], sCubeMapFaceInfo.mUp[2]),
     calcCubeMapFaceRotate(sCubeMapFaceInfo.mAt[3], sCubeMapFaceInfo.mUp[3]),
     calcCubeMapFaceRotate(sCubeMapFaceInfo.mAt[4], sCubeMapFaceInfo.mUp[4]),
     calcCubeMapFaceRotate(sCubeMapFaceInfo.mAt[5], sCubeMapFaceInfo.mUp[5])},
};
}  // namespace

/**
 * Creates the camera and environment used to render cube map faces.
 * @param pGraphicsSystemInfo Graphics system info.
 */
CubeMapDrawInfo::CubeMapDrawInfo(const GraphicsSystemInfo* pGraphicsSystemInfo) {
    mSimpleModelEnv = new SimpleModelEnv();
    mSimpleModelEnv->initialize(6, pGraphicsSystemInfo, nullptr);
}

/**
 * Prepares rendering of a cube map face.
 * @param pCubeMap Cube map to render to.
 * @param face Face index.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param rPos Position of the cube map.
 * @param mipLevel Mip level to render.
 * @param isClearColor Whether the color buffer is used.
 */
void CubeMapDrawInfo::preDrawCubeMapFace(agl::env::CubeMap* pCubeMap, s32 face, f32 near, f32 far,
                                         const sead::Vector3f& rPos, s32 mipLevel,
                                         bool isClearColor) {
    sead::PerspectiveProjection projection(1.0f, 10000.0f, sead::Mathf::piHalf(), 1.0f);
    mProjection = projection;
    mProjection.setAspect(1.0f);
    mProjection.setNear(near);
    mProjection.setFar(far);

    const sead::Matrix33f& rot = sCubeMapFaceInfo.mRotate[face];
    sead::Matrix34f viewMtx(
        rot(0, 0), rot(0, 1), rot(0, 2), -(rot(0, 0) * rPos.x + rot(0, 1) * rPos.y + rot(0, 2) * rPos.z),
        rot(1, 0), rot(1, 1), rot(1, 2), -(rot(1, 0) * rPos.x + rot(1, 1) * rPos.y + rot(1, 2) * rPos.z),
        rot(2, 0), rot(2, 1), rot(2, 2), -(rot(2, 0) * rPos.x + rot(2, 1) * rPos.y + rot(2, 2) * rPos.z));
    mCamera.setDirectMatrix(viewMtx);
    mCamera.updateViewMatrix();

    agl::DrawContext* drawContext =
        GameFrameworkNx::getAglDrawContext();
    pCubeMap->begin(drawContext, isClearColor, true);
    pCubeMap->preDraw(GameFrameworkNx::getAglDrawContext(),
                      face, mipLevel);
    const agl::RenderBuffer& renderBuffer = pCubeMap->getRenderBuffer();
    mViewport.setByFrameBuffer(renderBuffer);
    mViewport.apply(GameFrameworkNx::getDrawContext(), renderBuffer);
    renderBuffer.clear(GameFrameworkNx::getDrawContext(), 7, sead::Color4f::cGray, 1.0f, 0);

    sead::Vector2f size(mViewport.getMax().x - mViewport.getMin().x,
                        mViewport.getMax().y - mViewport.getMin().y);
    mSimpleModelEnv->updateEnv(face, mCamera.getMatrix(), mProjection.getProjectionMatrix(),
                               mProjection.getOffsetDirect(), size, near, far, mProjection.getFovy(),
                               mProjection.getAspect(), size, nullptr, nullptr, nullptr, 1.0f,
                               nullptr);
    mSimpleModelEnv->prepareModelDraw(face);
}

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
