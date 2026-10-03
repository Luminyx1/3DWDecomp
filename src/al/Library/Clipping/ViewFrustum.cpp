#include "Library/Clipping/ViewFrustum.hpp"

#include <math/seadMathCalcCommon.h>
#include <new>

namespace al {
typedef sead::Line<sead::Vector3f> Line3f;

static bool calcPlaneCrossLine(Line3f* pLine, const sead::Plane3f* pPlaneA,
                               const sead::Plane3f* pPlaneB);

/**
 * Builds a plane going through three points.
 * @param pPlane resulting plane, its normal points to the side where the points turn clockwise
 * @param rOrigin first point
 * @param rPos1 second point
 * @param rPos2 third point
 */
static inline void makePlaneFromPoints(sead::Plane3f* pPlane, const sead::Vector3f& rOrigin,
                                       const sead::Vector3f& rPos1, const sead::Vector3f& rPos2) {
    sead::Vector3f normal;
    normal.setCross(rPos1 - rOrigin, rPos2 - rOrigin);
    normal.normalize();
    *pPlane = sead::Plane3f(normal, rOrigin.dot(normal));
}

/**
 * Calculates the intersection of a line and a plane.
 * @param pPos resulting position
 * @param rLine line
 * @param rPlane plane
 * @return true if the line crosses the plane at a single point
 */
static inline bool calcLineCrossPos(sead::Vector3f* pPos, const Line3f& rLine,
                                    const sead::Plane3f& rPlane) {
    f32 t;
    if (sead::Geometry::calcIntersectionLineToPlane(rLine, rPlane, &t) != 1) {
        return false;
    }

    // copy-constructed (not assigned) so the start position is copied as a whole
    new (pPos) sead::Vector3f(rLine.getPos());
    pPos->x = t * rLine.getDir().x + pPos->x;
    pPos->y = t * rLine.getDir().y + pPos->y;
    pPos->z = t * rLine.getDir().z + pPos->z;
    return true;
}

/**
 * Builds the frustum from a view matrix.
 * @param rViewMtx view matrix
 * @param fovy vertical field of view, in degrees
 * @param aspect aspect ratio
 * @param near near clip distance
 * @param far far clip distance
 */
void ViewFrustum::calcFrustumFromViewMtx(const sead::Matrix34f& rViewMtx, f32 fovy, f32 aspect,
                                         f32 near, f32 far) {
    sead::Matrix34f poseMtx;
    poseMtx.m[0][0] = -rViewMtx.m[0][0];
    poseMtx.m[0][1] = rViewMtx.m[1][0];
    poseMtx.m[0][2] = -rViewMtx.m[2][0];
    poseMtx.m[1][0] = -rViewMtx.m[0][1];
    poseMtx.m[1][1] = rViewMtx.m[1][1];
    poseMtx.m[1][2] = -rViewMtx.m[2][1];
    poseMtx.m[2][0] = -rViewMtx.m[0][2];
    poseMtx.m[2][1] = rViewMtx.m[1][2];
    poseMtx.m[2][2] = -rViewMtx.m[2][2];
    poseMtx.m[0][3] = rViewMtx.m[0][3] * -rViewMtx.m[0][0] -
                      rViewMtx.m[1][0] * rViewMtx.m[1][3] - rViewMtx.m[2][3] * rViewMtx.m[2][0];
    poseMtx.m[1][3] = rViewMtx.m[0][3] * -rViewMtx.m[0][1] -
                      rViewMtx.m[1][1] * rViewMtx.m[1][3] - rViewMtx.m[2][3] * rViewMtx.m[2][1];
    poseMtx.m[2][3] = rViewMtx.m[0][3] * -rViewMtx.m[0][2] -
                      rViewMtx.m[1][2] * rViewMtx.m[1][3] - rViewMtx.m[2][3] * rViewMtx.m[2][2];
    makePlanesFromMtx(poseMtx, fovy, aspect, near, far);
}

/**
 * Builds the six frustum planes from a camera pose matrix. All plane normals point inwards.
 * @param rPoseMtx camera pose, the z axis being the view direction
 * @param fovy vertical field of view, in degrees
 * @param aspect aspect ratio
 * @param near near clip distance
 * @param far far clip distance
 */
void ViewFrustum::makePlanesFromMtx(const sead::Matrix34f& rPoseMtx, f32 fovy, f32 aspect,
                                    f32 near, f32 far) {
    f32 halfHeight = sead::Mathf::tan(sead::Mathf::deg2rad(fovy * 0.5f)) * near;
    f32 halfWidth = halfHeight * aspect;

    sead::Vector3f cornerPxMy;
    sead::Vector3f cornerPxPy;
    sead::Vector3f cornerMxPy;
    sead::Vector3f cornerMxMy;
    cornerPxMy.setMul(rPoseMtx, sead::Vector3f(halfWidth, -halfHeight, near));
    cornerPxPy.setMul(rPoseMtx, sead::Vector3f(halfWidth, halfHeight, near));
    cornerMxPy.setMul(rPoseMtx, sead::Vector3f(-halfWidth, halfHeight, near));
    cornerMxMy.setMul(rPoseMtx, sead::Vector3f(-halfWidth, -halfHeight, near));

    sead::Vector3f pos(rPoseMtx.m[0][3], rPoseMtx.m[1][3], rPoseMtx.m[2][3]);
    makePlaneFromPoints(&mPlanes[cPlane_SidePlusY], pos, cornerPxPy, cornerMxPy);
    makePlaneFromPoints(&mPlanes[cPlane_SidePlusX], pos, cornerPxMy, cornerPxPy);
    makePlaneFromPoints(&mPlanes[cPlane_SideMinusY], pos, cornerMxMy, cornerPxMy);
    makePlaneFromPoints(&mPlanes[cPlane_SideMinusX], pos, cornerMxPy, cornerMxMy);

    sead::Vector3f front(rPoseMtx.m[0][2], rPoseMtx.m[1][2], rPoseMtx.m[2][2]);
    sead::Vector3f nearPos = front * near + pos;
    mPlanes[cPlane_Near] = sead::Plane3f(front, nearPos.dot(front));

    sead::Vector3f back = -front;
    sead::Vector3f farPos = front * far + pos;
    mPlanes[cPlane_Far] = sead::Plane3f(back, farPos.dot(back));
}

/**
 * Builds the frustum from a camera pose matrix.
 * @param rPoseMtx camera pose, the z axis being the view direction
 * @param fovy vertical field of view, in degrees
 * @param aspect aspect ratio
 * @param near near clip distance
 * @param far far clip distance
 */
void ViewFrustum::calcFrustumFromPoseMtx(const sead::Matrix34f& rPoseMtx, f32 fovy, f32 aspect,
                                         f32 near, f32 far) {
    makePlanesFromMtx(rPoseMtx, fovy, aspect, near, far);
}

/**
 * Checks whether a sphere is at least partially inside the frustum.
 * @param rPos sphere center
 * @param radius sphere radius
 * @return true if the sphere intersects the frustum
 */
bool ViewFrustum::isIntersectSphere(const sead::Vector3f& rPos, f32 radius) const {
    for (s32 i = 0; i < cPlane_Num; i++) {
        if (mPlanes[i].getNormal().dot(rPos) - mPlanes[i].getD() < -radius) {
            return false;
        }
    }

    return true;
}

/**
 * Calculates the eight corners of the frustum.
 * @param pPoints resulting corners, left untouched past the first failure
 */
void ViewFrustum::calcFrustumPoints(Points* pPoints) const {
    Line3f lineA(sead::Vector3f::zero, sead::Vector3f::ex);
    Line3f lineB(sead::Vector3f::zero, sead::Vector3f::ex);
    Line3f lineC(sead::Vector3f::zero, sead::Vector3f::ex);
    Line3f lineD(sead::Vector3f::zero, sead::Vector3f::ex);

    if (!calcPlaneCrossLine(&lineA, &mPlanes[cPlane_SidePlusX], &mPlanes[cPlane_SidePlusY])) {
        return;
    }

    if (!calcPlaneCrossLine(&lineB, &mPlanes[cPlane_SidePlusX], &mPlanes[cPlane_SideMinusY])) {
        return;
    }

    if (!calcPlaneCrossLine(&lineC, &mPlanes[cPlane_SideMinusX], &mPlanes[cPlane_SidePlusY])) {
        return;
    }

    if (!calcPlaneCrossLine(&lineD, &mPlanes[cPlane_SideMinusX], &mPlanes[cPlane_SideMinusY])) {
        return;
    }

    const sead::Plane3f& nearPlane = mPlanes[cPlane_Near];
    if (!calcLineCrossPos(&pPoints->mNearPoints[0], lineA, nearPlane)) {
        return;
    }

    if (!calcLineCrossPos(&pPoints->mNearPoints[1], lineB, nearPlane)) {
        return;
    }

    if (!calcLineCrossPos(&pPoints->mNearPoints[2], lineC, nearPlane)) {
        return;
    }

    if (!calcLineCrossPos(&pPoints->mNearPoints[3], lineD, nearPlane)) {
        return;
    }

    const sead::Plane3f& farPlane = mPlanes[cPlane_Far];
    if (!calcLineCrossPos(&pPoints->mFarPoints[0], lineA, farPlane)) {
        return;
    }

    if (!calcLineCrossPos(&pPoints->mFarPoints[1], lineB, farPlane)) {
        return;
    }

    if (!calcLineCrossPos(&pPoints->mFarPoints[2], lineC, farPlane)) {
        return;
    }

    calcLineCrossPos(&pPoints->mFarPoints[3], lineD, farPlane);
}

/**
 * Calculates the line where two planes cross.
 * @param pLine resulting line, with a normalized direction
 * @param pPlaneA first plane
 * @param pPlaneB second plane
 * @return false if the planes are parallel
 */
static bool calcPlaneCrossLine(Line3f* pLine, const sead::Plane3f* pPlaneA,
                               const sead::Plane3f* pPlaneB) {
    const sead::Vector3f& normalA = pPlaneA->getNormal();
    const sead::Vector3f& normalB = pPlaneB->getNormal();
    sead::Vector3f dir;
    dir.setCross(normalA, normalB);
    f32 lengthSq = dir.squaredLength();

    if (lengthSq < sead::Mathf::epsilon()) {
        return false;
    }

    f32 invLengthSq = 1.0f / lengthSq;
    sead::Vector3f offset = sead::Vector3f(0.0f, 0.0f, 0.0f) + normalB * pPlaneA->getD() -
                            normalA * pPlaneB->getD();

    sead::Vector3f pos;
    pos.setCross(offset, dir);
    pLine->setPos(invLengthSq * pos);
    pLine->setDir(dir);
    // sead::Line has no mutable accessor, the direction is normalized in place
    const_cast<sead::Vector3f&>(pLine->getDir()).normalize();
    return true;
}

/**
 * Calculates where the four side edges of the frustum cross a plane.
 * @param pIntersection resulting points and the edges they belong to
 * @param pPlane plane to intersect with
 * @return false if the side edges could not be calculated
 */
bool ViewFrustum::calcPlaneIntersection(PlaneIntersection* pIntersection,
                                        const sead::Plane3f* pPlane) const {
    Line3f lineA(sead::Vector3f::zero, sead::Vector3f::ex);
    Line3f lineB(sead::Vector3f::zero, sead::Vector3f::ex);
    Line3f lineC(sead::Vector3f::zero, sead::Vector3f::ex);
    Line3f lineD(sead::Vector3f::zero, sead::Vector3f::ex);

    pIntersection->mPointNum = 0;

    for (s32 i = 0; i < 4; i++) {
        pIntersection->mIsIntersectEdge[i] = false;
    }

    if (!calcPlaneCrossLine(&lineA, &mPlanes[cPlane_SidePlusX], &mPlanes[cPlane_SidePlusY])) {
        return false;
    }

    if (!calcPlaneCrossLine(&lineB, &mPlanes[cPlane_SidePlusX], &mPlanes[cPlane_SideMinusY])) {
        return false;
    }

    if (!calcPlaneCrossLine(&lineC, &mPlanes[cPlane_SideMinusX], &mPlanes[cPlane_SidePlusY])) {
        return false;
    }

    if (!calcPlaneCrossLine(&lineD, &mPlanes[cPlane_SideMinusX], &mPlanes[cPlane_SideMinusY])) {
        return false;
    }

    const Line3f* lines[4] = {&lineA, &lineB, &lineC, &lineD};
    for (s32 i = 0; i < 4; i++) {
        if (calcLineCrossPos(&pIntersection->mPoints[pIntersection->mPointNum], *lines[i],
                             *pPlane)) {
            pIntersection->mIsIntersectEdge[i] = true;
            pIntersection->mPointNum++;
        }
    }

    return true;
}
}  // namespace al
