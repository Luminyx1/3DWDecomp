#include "Library/Projection/Projection.hpp"

#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>

namespace al {

void Projection::init() {
    mOffset = sead::Vector2f::zero;
    mProjMtx = sead::Matrix44f::ident;
    mProjInvMtx = sead::Matrix44f::ident;
    mProjMtxStd = sead::Matrix44f::ident;
    mProjInvMtxStd = sead::Matrix44f::ident;
    mFovy = 0.0f;
    mFocalLength = 0.0f;
    mAspect = 1.0f;
}

void Projection::calcMtx() {
    mProjMtx = mBase.getDeviceProjectionMatrix();
    mProjInvMtx.setInverse(mBase.getDeviceProjectionMatrix());
    mProjMtxStd = mProjMtx;
    mProjInvMtxStd = mProjInvMtx;
}

Projection::Projection(const Projection& rOther) {
    init();
    copyFrom(rOther);
}

void Projection::copyFrom(const Projection& rOther) {
    setProjTBLRNF(rOther.getTop(), rOther.getBottom(), rOther.getLeft(), rOther.getRight(),
                  rOther.getNear(), rOther.getFar());
    calcMtx();
}

f32 Projection::getTop() const {
    return mBase.getTop();
}

f32 Projection::getBottom() const {
    return mBase.getBottom();
}

f32 Projection::getLeft() const {
    return mBase.getLeft();
}

f32 Projection::getRight() const {
    return mBase.getRight();
}

f32 Projection::getNear() const {
    return mBase.getNear();
}

f32 Projection::getFar() const {
    return mBase.getFar();
}

void Projection::setProjTBLRNF(f32 top, f32 bottom, f32 left, f32 right, f32 near, f32 far) {
    mNear = near;
    mFar = far;
    mBase.setNear(near);
    mBase.setFar(far);
    mBase.setTBLR(top, bottom, left, right);

    f32 height = top - bottom;
    f32 width = right - left;
    mTop = top;
    mBottom = bottom;
    mLeft = left;
    mRight = right;
    f32 halfHeight = height * 0.5f;
    mAspect = width / height;
    mFovy = sead::Mathf::atan2(halfHeight, mBase.getNear()) * 2.0f;
    mFocalLength = sead::Mathf::tan(mFovy * 0.5f);

    sead::Vector2f offset;
    offset.x = width == 0.0f ? 0.0f : (left + right) * 0.5f / width;
    offset.y = height == 0.0f ? 0.0f : (top + bottom) * 0.5f / height;
    mBase.setOffset(offset);
    mOffset = offset;
}

Projection::Projection(f32 near, f32 far, f32 fovy, f32 aspect) : mBase(near, far, fovy, aspect) {
    init();
    setProj(near, far, fovy, aspect);
    calcMtx();
}

void Projection::setProj(f32 near, f32 far, f32 fovy, f32 aspect) {
    mBase.set(near, far, fovy, aspect);
    mAspect = aspect;
    setFovy(fovy);
    setNear(near);
    setFar(far);

    f32 height = calcNearClipHeight();
    f32 width = calcNearClipWidth();
    f32 offsetX = width * mOffset.x;
    f32 offsetY = height * mOffset.y;
    f32 halfHeight = height * 0.5f;
    f32 halfWidth = width * 0.5f;
    f32 top = halfHeight + offsetY;
    f32 bottom = offsetY - halfHeight;
    f32 left = offsetX - halfWidth;
    f32 right = halfWidth + offsetX;
    mTop = top;
    mBottom = bottom;
    mLeft = left;
    mRight = right;
}

void Projection::setFovy(f32 fovy) {
    mFovy = fovy;
    mFocalLength = sead::Mathf::tan(fovy * 0.5f);
}

void Projection::setAspect(f32 aspect) {
    mBase.setAspect(aspect);
    mAspect = aspect;
}

void Projection::setNear(f32 near) {
    mBase.setNear(near);
    mNear = near;
}

void Projection::setFar(f32 far) {
    mBase.setFar(far);
    mFar = far;
}

f32 Projection::calcNearClipHeight() {
    return 2 * getNear() * mFocalLength;
}

f32 Projection::calcNearClipWidth() {
    return calcNearClipHeight() * mBase.getAspect();
}

void Projection::setTop(f32 top) {
    mTop = top;
}

void Projection::setBottom(f32 bottom) {
    mBottom = bottom;
}

void Projection::setLeft(f32 left) {
    mLeft = left;
}

void Projection::setRight(f32 right) {
    mRight = right;
}

void Projection::setOffset(const sead::Vector2f& rOffset) {
    mBase.setOffset(rOffset);
    mOffset = rOffset;
}

f32 Projection::getAspect() const {
    return mBase.getAspect();
}

bool isUsingViewportHalfZ() {
    return false;
}

bool isProjectionReverse() {
    return false;
}

bool isMakeLinearDepthProjReverseInfinite() {
    return false;
}

const sead::Vector2f& Projection::getOffset() const {
    return mBase.getOffsetDirect();
}

f32 Projection::getFovy() const {
    return mBase.getFovy();
}

const sead::Matrix44f& Projection::getProjMtx() const {
    return mBase.getDeviceProjectionMatrix();
}

const sead::Matrix44f& Projection::getProjInvMtx() const {
    return mProjInvMtx;
}

void setClipControlStd() {}

void setClipControl() {}

/**
 * Applies a viewport covering the whole render buffer.
 * @param pContext Draw context.
 * @param rBuffer Render buffer.
 */
void applyViewport(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer) {
    sead::Viewport viewport(rBuffer);
    viewport.apply(pContext, rBuffer);
}

/**
 * Applies a viewport covering the whole render buffer for the current projection type.
 * @param pContext Draw context.
 * @param rBuffer Render buffer.
 */
void applyViewportProjType(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer) {
    sead::Viewport viewport(rBuffer);
    viewport.apply(pContext, rBuffer);
}

static void calcPlaneByPoints(ViewFrustumPlane* pPlane, const sead::Vector3f& rA,
                              const sead::Vector3f& rB, const sead::Vector3f& rC) {
    sead::Vector3f normal;
    normal.setCross(rB - rA, rC - rA);
    normal.normalize();
    pPlane->normal = normal;
    pPlane->distance = normal.dot(rA);
}

/**
 * Calculates the six planes of a view frustum from its corner points.
 * @param pPlanes Receives the planes.
 * @param rPoints Corner points of the frustum.
 */
void calcPlanesByFrustumPoints(ViewFrustumPlanes* pPlanes, const ViewFrustumPoints& rPoints) {
    const sead::Vector3f* p = rPoints.points;
    calcPlaneByPoints(&pPlanes->planes[0], p[1], p[6], p[5]);
    calcPlaneByPoints(&pPlanes->planes[1], p[0], p[4], p[7]);
    calcPlaneByPoints(&pPlanes->planes[2], p[2], p[7], p[6]);
    calcPlaneByPoints(&pPlanes->planes[3], p[1], p[5], p[4]);
    calcPlaneByPoints(&pPlanes->planes[4], p[0], p[3], p[2]);
    calcPlaneByPoints(&pPlanes->planes[5], p[5], p[6], p[7]);
}

/**
 * Transforms the corners of the device space cube into view space.
 * @param pPoints Receives the eight corner points.
 * @param rProjInvMtx Inverse projection matrix.
 */
void calcFrustumPointsAtViewSpace(ViewFrustumPoints* pPoints, const sead::Matrix44f& rProjInvMtx) {
    pPoints->points[0].set(-1.0f, -1.0f, -1.0f);
    pPoints->points[1].set(1.0f, -1.0f, -1.0f);
    pPoints->points[2].set(1.0f, 1.0f, -1.0f);
    pPoints->points[3].set(-1.0f, 1.0f, -1.0f);
    pPoints->points[4].set(-1.0f, -1.0f, 1.0f);
    pPoints->points[5].set(1.0f, -1.0f, 1.0f);
    pPoints->points[6].set(1.0f, 1.0f, 1.0f);
    pPoints->points[7].set(-1.0f, 1.0f, 1.0f);
    for (s32 i = 0; i < 8; i++) {
        sead::Vector3f& point = pPoints->points[i];
        f32 x = point.x;
        f32 y = point.y;
        f32 z = point.z;
        const sead::Matrix44f& m = rProjInvMtx;
        f32 invW = 1.0f / (x * m(3, 0) + y * m(3, 1) + z * m(3, 2) + m(3, 3));
        point.x = invW * (x * m(0, 0) + y * m(0, 1) + z * m(0, 2) + m(0, 3));
        point.y = invW * (x * m(1, 0) + y * m(1, 1) + z * m(1, 2) + m(1, 3));
        point.z = invW * (x * m(2, 0) + y * m(2, 1) + z * m(2, 2) + m(2, 3));
    }
}

/**
 * Calculates the width of the near plane in view space.
 * @param rProjInvMtx Inverse projection matrix.
 * @return Width of the near plane.
 */
f32 calcFrustumNearWidth(const sead::Matrix44f& rProjInvMtx) {
    const sead::Matrix44f& m = rProjInvMtx;
    f32 y = 0.0f;
    f32 leftW = 1.0f / (-1.0f * m(3, 0) + y * m(3, 1) + -1.0f * m(3, 2) + m(3, 3));
    f32 leftX = leftW * (-1.0f * m(0, 0) + y * m(0, 1) + -1.0f * m(0, 2) + m(0, 3));
    f32 rightW = 1.0f / (1.0f * m(3, 0) + y * m(3, 1) + -1.0f * m(3, 2) + m(3, 3));
    f32 rightX = rightW * (1.0f * m(0, 0) + y * m(0, 1) + -1.0f * m(0, 2) + m(0, 3));
    f32 width = leftX - rightX;
    return width > 0.0f ? width : -width;
}

/**
 * Transforms the corners of the device space cube into world space.
 * @param pPoints Receives the eight corner points.
 * @param rViewInvMtx Inverse view matrix.
 * @param rProjInvMtx Inverse projection matrix.
 */
void calcFrustumPointsAtWorldSpace(ViewFrustumPoints* pPoints, const sead::Matrix34f& rViewInvMtx,
                                   const sead::Matrix44f& rProjInvMtx) {
    calcFrustumPointsAtViewSpace(pPoints, rProjInvMtx);
    for (s32 i = 0; i < 8; i++) {
        pPoints->points[i].setMul(rViewInvMtx, pPoints->points[i]);
    }
}

/**
 * Calculates the six world space planes of a view frustum.
 * @param pPlanes Receives the planes.
 * @param rViewInvMtx Inverse view matrix.
 * @param rProjInvMtx Inverse projection matrix.
 * @param pPoints Receives the corner points if not null.
 */
void calcFrustumPlanesWorldSpace(ViewFrustumPlanes* pPlanes, const sead::Matrix34f& rViewInvMtx,
                                 const sead::Matrix44f& rProjInvMtx, ViewFrustumPoints* pPoints) {
    ViewFrustumPoints points;
    ViewFrustumPoints* target = pPoints ? pPoints : &points;
    calcFrustumPointsAtWorldSpace(target, rViewInvMtx, rProjInvMtx);
    ViewFrustumPlanes planes;
    const sead::Vector3f* p = target->points;
    calcPlaneByPoints(&planes.planes[0], p[1], p[6], p[5]);
    calcPlaneByPoints(&planes.planes[1], p[0], p[4], p[7]);
    calcPlaneByPoints(&planes.planes[2], p[2], p[7], p[6]);
    calcPlaneByPoints(&planes.planes[3], p[1], p[5], p[4]);
    calcPlaneByPoints(&planes.planes[4], p[0], p[3], p[2]);
    calcPlaneByPoints(&planes.planes[5], p[5], p[6], p[7]);
    calcPlanesByFrustumPoints(pPlanes, *target);
}

/**
 * Transforms the corners of the device space cube into view space.
 * @param rProjInvMtx Inverse projection matrix.
 */
void ViewFrustumPoints::calcPoints(const sead::Matrix44f& rProjInvMtx) {
    calcFrustumPointsAtViewSpace(this, rProjInvMtx);
}

}  // namespace al
