#include "Library/Shader/DeferredRendering/CubeMapDrawInfo.hpp"

#include <math/seadMathCalcCommon.h>
#include "common/aglDrawContext.h"
#include "environment/aglCubeMap.h"

#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/SimpleModelEnv.hpp"

namespace al {
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
    sead::Matrix34f viewMtx(rot);
    viewMtx(0, 3) = -(rot(0, 0) * rPos.x + rot(0, 1) * rPos.y + rot(0, 2) * rPos.z);
    viewMtx(1, 3) = -(rot(1, 0) * rPos.x + rot(1, 1) * rPos.y + rot(1, 2) * rPos.z);
    viewMtx(2, 3) = -(rot(2, 0) * rPos.x + rot(2, 1) * rPos.y + rot(2, 2) * rPos.z);
    mCamera.setDirectMatrix(viewMtx);
    mCamera.updateViewMatrix();

    agl::DrawContext* drawContext = GameFrameworkNx::getAglDrawContext();
    pCubeMap->begin(drawContext, isClearColor, true);
    pCubeMap->preDraw(GameFrameworkNx::getAglDrawContext(), face, mipLevel);
    mViewport.setByFrameBuffer(pCubeMap->getRenderBuffer());
    mViewport.apply(GameFrameworkNx::getDrawContext(), pCubeMap->getRenderBuffer());
    pCubeMap->getRenderBuffer().clear(GameFrameworkNx::getDrawContext(), 7, sead::Color4f::cGray, 1.0f, 0);

    sead::Vector2f size(mViewport.getMax().x - mViewport.getMin().x,
                        mViewport.getMax().y - mViewport.getMin().y);
    mSimpleModelEnv->updateEnv(face, mCamera.getMatrix(), mProjection.getProjectionMatrix(),
                               mProjection.getOffsetDirect(), size, near, far, mProjection.getFovy(),
                               mProjection.getAspect(), size, nullptr, nullptr, nullptr, 1.0f,
                               nullptr);
    mSimpleModelEnv->prepareModelDraw(face);
}
}  // namespace al
