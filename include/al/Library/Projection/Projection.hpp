#pragma once

#include <gfx/seadProjection.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace agl {
class DrawContext;
class RenderBuffer;
}  // namespace agl

namespace al {

struct ViewFrustumPoints {
    void calcPoints(const sead::Matrix44f& rProjInvMtx);

    sead::Vector3f points[8];
};

struct ViewFrustumPlane {
    sead::Vector3f normal;
    f32 distance;
};

struct ViewFrustumPlanes {
    ViewFrustumPlane planes[6];
};

class Projection {
public:
    Projection();
    Projection(const Projection& rOther);
    Projection(f32 near, f32 far, f32 fovy, f32 aspect);

    void init();
    void calcMtx();
    void copyFrom(const Projection& rOther);
    f32 getTop() const;
    f32 getBottom() const;
    f32 getLeft() const;
    f32 getRight() const;
    f32 getNear() const;
    f32 getFar() const;
    void setProjTBLRNF(f32 top, f32 bottom, f32 left, f32 right, f32 near, f32 far);
    void setProj(f32 near, f32 far, f32 fovy, f32 aspect);
    void setFovy(f32 fovy);
    void setAspect(f32 aspect);
    void setNear(f32 near);
    void setFar(f32 far);
    f32 calcNearClipHeight();
    f32 calcNearClipWidth();
    void setTop(f32 top);
    void setBottom(f32 bottom);
    void setLeft(f32 left);
    void setRight(f32 right);
    void setOffset(const sead::Vector2f& rOffset);
    void merge(const Projection& rProj1, const Projection& rProj2);
    f32 getAspect() const;
    const sead::Vector2f& getOffset() const;
    f32 getFovy() const;
    const sead::Matrix44f& getProjMtx() const;
    const sead::Matrix44f& getProjInvMtx() const;

    sead::PerspectiveProjection& getProjectionSead() { return mBase; }

    const sead::PerspectiveProjection& getProjectionSead() const { return mBase; }

private:
    sead::PerspectiveProjection mBase;
    sead::Matrix44f mProjMtx;
    sead::Matrix44f mProjInvMtx;
    sead::Matrix44f mProjMtxStd;
    sead::Matrix44f mProjInvMtxStd;
    f32 mLeft = sead::MathCalcCommon<f32>::maxNumber();
    f32 mBottom = sead::MathCalcCommon<f32>::maxNumber();
    f32 mNear = sead::MathCalcCommon<f32>::maxNumber();
    f32 mRight = sead::MathCalcCommon<f32>::minNumber();
    f32 mTop = sead::MathCalcCommon<f32>::minNumber();
    f32 mFar = sead::MathCalcCommon<f32>::minNumber();
    f32 mFovy = 0.0f;
    f32 mFocalLength = 0.0f;
    f32 mAspect = 1.0f;
    sead::Vector2f mOffset;
};

void applyViewport(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer);
void applyViewportProjType(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer);
void calcFrustumPointsAtViewSpace(ViewFrustumPoints* pPoints, const sead::Matrix44f& rProjInvMtx);
f32 calcFrustumNearWidth(const sead::Matrix44f& rProjInvMtx);
void calcPlanesByFrustumPoints(ViewFrustumPlanes* pPlanes, const ViewFrustumPoints& rPoints);
void calcFrustumPlanesWorldSpace(ViewFrustumPlanes* pPlanes, const sead::Matrix34f& rViewInvMtx,
                                 const sead::Matrix44f& rProjInvMtx, ViewFrustumPoints* pPoints);
void calcFrustumPointsAtWorldSpace(ViewFrustumPoints* pPoints, const sead::Matrix34f& rViewInvMtx,
                                   const sead::Matrix44f& rProjInvMtx);

}  // namespace al
