#include "Library/Math/MatrixUtil.hpp"

#include <nn/util/util_MathTypes.h>

#include "Library/Math/MathUtil.hpp"

namespace al {

void makeMtxRotateTrans(sead::Matrix34f* outMtx, const sead::Vector3f& rotate,
                        const sead::Vector3f& trans) {
    sead::Vector3f rotateRad(sead::Mathf::deg2rad(rotate.x), sead::Mathf::deg2rad(rotate.y),
                             sead::Mathf::deg2rad(rotate.z));
    outMtx->makeRT(rotateRad, trans);
}

void makeMtxFromTwoAxis(sead::Matrix34f* outMtx, const sead::Vector3f& vectorA,
                        const sead::Vector3f& vectorB, s32 axisA, s32 axisB) {
    sead::Vector3f dir[3];

    s32 axisC;

    if (axisA == 0)
        axisC = axisB == 1 ? 2 : 1;
    else if (axisA == 1)
        axisC = axisB == 0 ? 2 : 0;
    else
        axisC = axisB == 0 ? 1 : 0;

    normalize(&dir[axisA], vectorA);
    dir[axisB] = vectorB;

    dir[axisC].setCross(dir[(axisC + 1) % 3], dir[(axisC + 2) % 3]);
    normalize(&dir[axisC]);

    dir[axisB].setCross(dir[(axisB + 1) % 3], dir[(axisB + 2) % 3]);

    outMtx->setBase(0, dir[0]);  // side
    outMtx->setBase(1, dir[1]);  // up
    outMtx->setBase(2, dir[2]);  // front
}

void makeMtxFrontUp(sead::Matrix34f* outMtx, const sead::Vector3f& front,
                    const sead::Vector3f& up) {
    sead::Vector3f frontNorm = front;
    normalize(&frontNorm);

    sead::Vector3f side;
    side.setCross(up, frontNorm);
    normalize(&side);

    sead::Vector3f upNorm;
    upNorm.setCross(frontNorm, side);

    outMtx->setBase(0, side);
    outMtx->setBase(1, upNorm);
    outMtx->setBase(2, frontNorm);
}

void makeMtxFrontSide(sead::Matrix34f* outMtx, const sead::Vector3f& front,
                      const sead::Vector3f& side) {
    sead::Vector3f frontNorm = front;
    normalize(&frontNorm);

    sead::Vector3f up;
    up.setCross(frontNorm, side);
    normalize(&up);

    sead::Vector3f sideNorm;
    sideNorm.setCross(up, frontNorm);

    outMtx->setBase(0, sideNorm);
    outMtx->setBase(1, up);
    outMtx->setBase(2, frontNorm);
}

void makeMtxUpFront(sead::Matrix34f* outMtx, const sead::Vector3f& up,
                    const sead::Vector3f& front) {
    sead::Vector3f upNorm = up;
    normalize(&upNorm);

    sead::Vector3f side;
    side.setCross(up, front);
    normalize(&side);

    sead::Vector3f frontNorm;
    frontNorm.setCross(side, upNorm);

    outMtx->setBase(0, side);
    outMtx->setBase(1, upNorm);
    outMtx->setBase(2, frontNorm);
}

void makeMtxUpSide(sead::Matrix34f* outMtx, const sead::Vector3f& up, const sead::Vector3f& side) {
    sead::Vector3f upNorm = up;
    normalize(&upNorm);

    sead::Vector3f front;
    front.setCross(side, up);
    normalize(&front);

    sead::Vector3f sideNorm;
    sideNorm.setCross(upNorm, front);

    outMtx->setBase(0, sideNorm);
    outMtx->setBase(1, upNorm);
    outMtx->setBase(2, front);
}

void makeMtxSideUp(sead::Matrix34f* outMtx, const sead::Vector3f& side, const sead::Vector3f& up) {
    makeMtxFromTwoAxis(outMtx, side, up, 0, 1);
}

void makeMtxSideFront(sead::Matrix34f* outMtx, const sead::Vector3f& side,
                      const sead::Vector3f& front) {
    makeMtxFromTwoAxis(outMtx, side, front, 0, 2);
}

void makeMtxFrontNoSupport(sead::Matrix34f* outMtx, const sead::Vector3f& front) {
    bool isYAxis = getMaxAbsElementIndex(front) == 1;

    sead::Vector3f up;
    up.x = isYAxis ? sead::Vector3f::ez.x : sead::Vector3f::ey.x;
    up.y = isYAxis ? sead::Vector3f::ez.y : sead::Vector3f::ey.y;
    up.z = isYAxis ? sead::Vector3f::ez.z : sead::Vector3f::ey.z;

    makeMtxFrontUp(outMtx, front, up);
}

void makeMtxFrontNoSupportPos(sead::Matrix34f* outMtx, const sead::Vector3f& front,
                              const sead::Vector3f& pos) {
    makeMtxFrontNoSupport(outMtx, front);
    outMtx->setBase(3, pos);
}

void makeMtxUpNoSupport(sead::Matrix34f* outMtx, const sead::Vector3f& up) {
    bool isZAxis = getMaxAbsElementIndex(up) == 2;

    sead::Vector3f front;
    front.x = isZAxis ? sead::Vector3f::ex.x : sead::Vector3f::ez.x;
    front.y = isZAxis ? sead::Vector3f::ex.y : sead::Vector3f::ez.y;
    front.z = isZAxis ? sead::Vector3f::ex.z : sead::Vector3f::ez.z;

    makeMtxUpFront(outMtx, up, front);
}

void makeMtxUpNoSupportPos(sead::Matrix34f* outMtx, const sead::Vector3f& up,
                           const sead::Vector3f& pos) {
    makeMtxUpNoSupport(outMtx, up);
    outMtx->setBase(3, pos);
}

void makeMtxSideNoSupport(sead::Matrix34f* outMtx, const sead::Vector3f& side) {
    bool isYAxis = getMaxAbsElementIndex(side) == 1;

    sead::Vector3f up;

    if (isYAxis) {
        up = sead::Vector3f::ez;
    } else {
        up = sead::Vector3f::ey;
    }

    makeMtxSideUp(outMtx, side, up);
}

void makeMtxSideNoSupportPos(sead::Matrix34f* outMtx, const sead::Vector3f& side,
                             const sead::Vector3f& pos) {
    makeMtxSideNoSupport(outMtx, side);
    outMtx->setBase(3, pos);
}

void makeMtxQuatPos(sead::Matrix34f* outMtx, const sead::Quatf& quat, const sead::Vector3f& pos) {
    outMtx->fromQuat(quat);
    outMtx->setTranslation(pos);
}

void makeMtxQuatScalePos(sead::Matrix34f* outMtx, const sead::Quatf& quat,
                         const sead::Vector3f& scale, const sead::Vector3f& pos) {
    outMtx->fromQuat(quat);
    outMtx->scaleBases(scale.x, scale.y, scale.z);
    outMtx->setTranslation(pos);
}

void makeMtxQuatScalePos(sead::Matrix44f* pOutMtx, const sead::Quatf& rQuat,
                         const sead::Vector3f& rScale, const sead::Vector3f& rPos) {
    pOutMtx->fromQuat(rQuat);
    pOutMtx->scaleBases(rScale.x, rScale.y, rScale.z, 1.0f);
    pOutMtx->setCol(3, sead::Vector4f(rPos.x, rPos.y, rPos.z, 1.0f));
}

void makeMtxFrontUpPos(sead::Matrix34f* outMtx, const sead::Vector3f& front,
                       const sead::Vector3f& up, const sead::Vector3f& pos) {
    makeMtxFrontUp(outMtx, front, up);
    outMtx->setBase(3, pos);
}

void makeMtxFrontSidePos(sead::Matrix34f* outMtx, const sead::Vector3f& front,
                         const sead::Vector3f& side, const sead::Vector3f& pos) {
    makeMtxFrontSide(outMtx, front, side);
    outMtx->setBase(3, pos);
}

void makeMtxUpFrontPos(sead::Matrix34f* outMtx, const sead::Vector3f& up,
                       const sead::Vector3f& front, const sead::Vector3f& pos) {
    makeMtxUpFront(outMtx, up, front);
    outMtx->setBase(3, pos);
}

void makeMtxUpSidePos(sead::Matrix34f* outMtx, const sead::Vector3f& up, const sead::Vector3f& side,
                      const sead::Vector3f& pos) {
    makeMtxUpSide(outMtx, up, side);
    outMtx->setBase(3, pos);
}

void makeMtxSideUpPos(sead::Matrix34f* outMtx, const sead::Vector3f& side, const sead::Vector3f& up,
                      const sead::Vector3f& pos) {
    makeMtxSideUp(outMtx, side, up);
    outMtx->setBase(3, pos);
}

void makeMtxSideFrontPos(sead::Matrix34f* outMtx, const sead::Vector3f& side,
                         const sead::Vector3f& front, const sead::Vector3f& pos) {
    makeMtxSideFront(outMtx, side, front);
    outMtx->setBase(3, pos);
}

void makeMtxFollowTarget(sead::Matrix34f* outMtx, const sead::Matrix34f& baseMtx,
                         const sead::Vector3f& trans, const sead::Vector3f& rotate) {
    sead::Matrix34f rotationMatrix;
    sead::Vector3f rotateRad(sead::Mathf::deg2rad(rotate.x), sead::Mathf::deg2rad(rotate.y),
                             sead::Mathf::deg2rad(rotate.z));
    rotationMatrix.makeR(rotateRad);

    sead::Matrix34f translationMatrix;
    translationMatrix.makeRT({0.0f, 0.0f, 0.0f}, trans);

    sead::Matrix34f pose = rotationMatrix * translationMatrix;
    *outMtx = baseMtx * pose;
}

void makeMtxProjFromQuatPoseUp(sead::Matrix44f* pOutMtx, const sead::Quatf& rQuat,
                               const sead::Vector2f& rSize, const sead::Vector3f& rPos) {
    sead::Vector3f front;
    sead::Vector3f side;
    sead::Vector3f up;
    calcQuatFront(&front, rQuat);
    calcQuatUp(&up, rQuat);
    calcQuatSide(&side, rQuat);
    sead::Vector3f origin = rPos - side * (rSize.x * 0.5f) - front * (rSize.y * 0.5f);
    pOutMtx->setCol(0, sead::Vector4f(side.x, side.y, side.z, 0.0f));
    pOutMtx->setCol(1, sead::Vector4f(front.x, front.y, front.z, 0.0f));
    pOutMtx->setCol(3, sead::Vector4f(origin.x, origin.y, origin.z, 1.0f));
    pOutMtx->setCol(2, sead::Vector4f(-up.x, -up.y, -up.z, 0.0f));
    pOutMtx->scaleBases(rSize.x, rSize.y, 1.0f, 1.0f);
    pOutMtx->setInverse(*pOutMtx);
}

void makeMtxProjFromQuatPoseFront(sead::Matrix44f* pOutMtx, const sead::Quatf& rQuat,
                                  const sead::Vector2f& rSize, const sead::Vector3f& rPos) {
    sead::Vector3f front;
    sead::Vector3f side;
    sead::Vector3f up;
    calcQuatFront(&front, rQuat);
    calcQuatUp(&up, rQuat);
    calcQuatSide(&side, rQuat);
    sead::Vector3f origin = rPos - side * (rSize.x * 0.5f) - up * (rSize.y * 0.5f);
    pOutMtx->setCol(0, sead::Vector4f(side.x, side.y, side.z, 0.0f));
    pOutMtx->setCol(1, sead::Vector4f(up.x, up.y, up.z, 0.0f));
    pOutMtx->setCol(3, sead::Vector4f(origin.x, origin.y, origin.z, 1.0f));
    pOutMtx->setCol(2, sead::Vector4f(-front.x, -front.y, -front.z, 0.0f));
    pOutMtx->scaleBases(rSize.x, rSize.y, 1.0f, 1.0f);
    pOutMtx->setInverse(*pOutMtx);
}

void makeMtxProjFromQuatPoseSide(sead::Matrix44f* pOutMtx, const sead::Quatf& rQuat,
                                 const sead::Vector2f& rSize, const sead::Vector3f& rPos) {
    sead::Vector3f front;
    sead::Vector3f side;
    sead::Vector3f up;
    calcQuatFront(&front, rQuat);
    calcQuatUp(&up, rQuat);
    calcQuatSide(&side, rQuat);
    sead::Vector3f origin = rPos - front * (rSize.x * 0.5f) - up * (rSize.y * 0.5f);
    pOutMtx->setCol(0, sead::Vector4f(front.x, front.y, front.z, 0.0f));
    pOutMtx->setCol(1, sead::Vector4f(up.x, up.y, up.z, 0.0f));
    pOutMtx->setCol(3, sead::Vector4f(origin.x, origin.y, origin.z, 1.0f));
    pOutMtx->setCol(2, sead::Vector4f(-side.x, -side.y, -side.z, 0.0f));
    pOutMtx->scaleBases(rSize.x, rSize.y, 1.0f, 1.0f);
    pOutMtx->setInverse(*pOutMtx);
}

void makeMtxProjFromUp(sead::Matrix44f* pOutMtx, const sead::Vector2f& rSize,
                       const sead::Vector3f& rPos) {
    sead::Vector3f origin = rPos;
    origin.x -= rSize.x * 0.5f;
    origin.z -= rSize.y * 0.5f;
    pOutMtx->setCol(0, sead::Vector4f(1.0f, 0.0f, 0.0f, 0.0f));
    pOutMtx->setCol(1, sead::Vector4f(0.0f, 0.0f, 1.0f, 0.0f));
    pOutMtx->setCol(3, sead::Vector4f(origin.x, origin.y, origin.z, 1.0f));
    pOutMtx->setCol(2, sead::Vector4f(0.0f, -1.0f, 0.0f, 0.0f));
    pOutMtx->scaleBases(rSize.x, rSize.y, 1.0f, 1.0f);
    pOutMtx->setInverse(*pOutMtx);
}

void rotateMtxXDirDegree(sead::Matrix34f* outMtx, const sead::Matrix34f& baseMtx, f32 angle) {
    sead::Matrix34f rotationMatrix;
    rotationMatrix.makeR(sead::Mathf::deg2rad(angle) * sead::Vector3f::ex);

    *outMtx = baseMtx * rotationMatrix;
}

void rotateMtxYDirDegree(sead::Matrix34f* outMtx, const sead::Matrix34f& baseMtx, f32 angle) {
    sead::Matrix34f rotationMatrix;
    rotationMatrix.makeR(sead::Mathf::deg2rad(angle) * sead::Vector3f::ey);

    *outMtx = baseMtx * rotationMatrix;
}

void rotateMtxZDirDegree(sead::Matrix34f* outMtx, const sead::Matrix34f& baseMtx, f32 angle) {
    sead::Matrix34f rotationMatrix;
    rotationMatrix.makeR(sead::Mathf::deg2rad(angle) * sead::Vector3f::ez);

    *outMtx = baseMtx * rotationMatrix;
}

void rotateMtxCenterPosXDirDegree(sead::Matrix34f* outMtx, const sead::Matrix34f& mtx,
                                  const sead::Vector3f& center, f32 degree) {
    sead::Vector3f axis;
    mtx.getBase(axis, 0);
    rotateMtxCenterPosAxisDegree(outMtx, mtx, center, axis, degree);
}

void rotateMtxCenterPosAxisDegree(sead::Matrix34f* outMtx, const sead::Matrix34f& mtx,
                                  const sead::Vector3f& center, const sead::Vector3f& axis,
                                  f32 degree) {
    sead::Quatf quat = sead::Quatf::unit;
    makeQuatRotateDegree(&quat, axis, degree);
    rotateMtxCenterPosQuat(outMtx, mtx, center, quat);
}

void rotateMtxCenterPosYDirDegree(sead::Matrix34f* outMtx, const sead::Matrix34f& mtx,
                                  const sead::Vector3f& center, f32 degree) {
    sead::Vector3f axis;
    mtx.getBase(axis, 1);
    rotateMtxCenterPosAxisDegree(outMtx, mtx, center, axis, degree);
}

void rotateMtxCenterPosZDirDegree(sead::Matrix34f* outMtx, const sead::Matrix34f& mtx,
                                  const sead::Vector3f& center, f32 degree) {
    sead::Vector3f axis;
    mtx.getBase(axis, 2);
    rotateMtxCenterPosAxisDegree(outMtx, mtx, center, axis, degree);
}

/**
 * Rotates a matrix by a quaternion around a center given in the matrix's local space.
 * @param pOutMtx Output matrix.
 * @param rMtx Source matrix.
 * @param rCenter Rotation center in local space.
 * @param rQuat Rotation to apply.
 */
void rotateMtxCenterPosQuat(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtx,
                            const sead::Vector3f& rCenter, const sead::Quatf& rQuat) {
    sead::Quatf mtxQuat = sead::Quatf::unit;
    rMtx.toQuat(mtxQuat);
    sead::Quatf quat = rQuat * mtxQuat;
    quat.normalize();
    sead::Vector3f worldCenter = rMtx * rCenter;
    sead::Vector3f rotatedCenter;
    rotatedCenter.setRotated(quat, rCenter);
    pOutMtx->makeQT(quat, worldCenter - rotatedCenter);
}

/**
 * Turns the rotation of a matrix so that its X axis turns toward a direction, keeping the translation.
 * @param pOutMtx Output matrix.
 * @param rMtx Source matrix.
 * @param rDir Target direction.
 * @param degree Maximum turn angle in degrees.
 * @return Result of the quaternion turn.
 */
bool turnMtxXDirDegree(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtx,
                       const sead::Vector3f& rDir, f32 degree) {
    sead::Quatf quat;
    rMtx.toQuat(quat);
    sead::Vector3f trans = rMtx.getTranslation();
    bool result = turnQuatXDirRadian(&quat, quat, rDir, sead::Mathf::deg2rad(degree));
    pOutMtx->makeQT(quat, trans);
    return result;
}

/**
 * Turns the rotation of a matrix so that its Y axis turns toward a direction, keeping the translation.
 * @param pOutMtx Output matrix.
 * @param rMtx Source matrix.
 * @param rDir Target direction.
 * @param degree Maximum turn angle in degrees.
 * @return Result of the quaternion turn.
 */
bool turnMtxYDirDegree(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtx,
                       const sead::Vector3f& rDir, f32 degree) {
    sead::Quatf quat;
    rMtx.toQuat(quat);
    sead::Vector3f trans = rMtx.getTranslation();
    bool result = turnQuatYDirRadian(&quat, quat, rDir, sead::Mathf::deg2rad(degree));
    pOutMtx->makeQT(quat, trans);
    return result;
}

/**
 * Turns the rotation of a matrix so that its Z axis turns toward a direction, keeping the translation.
 * @param pOutMtx Output matrix.
 * @param rMtx Source matrix.
 * @param rDir Target direction.
 * @param degree Maximum turn angle in degrees.
 * @return Result of the quaternion turn.
 */
bool turnMtxZDirDegree(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtx,
                       const sead::Vector3f& rDir, f32 degree) {
    sead::Quatf quat;
    rMtx.toQuat(quat);
    sead::Vector3f trans = rMtx.getTranslation();
    bool result = turnQuatZDirRadian(&quat, quat, rDir, sead::Mathf::deg2rad(degree));
    pOutMtx->makeQT(quat, trans);
    return result;
}

void calcCameraPosFromViewMtx(sead::Vector3f* outPos, const sead::Matrix34f& viewMtx) {
    sead::Vector3f pos;
    pos.x = -viewMtx.m[0][0] * viewMtx.m[0][3] - viewMtx.m[1][0] * viewMtx.m[1][3] -
            viewMtx.m[2][0] * viewMtx.m[2][3];
    pos.y = -viewMtx.m[0][1] * viewMtx.m[0][3] - viewMtx.m[1][1] * viewMtx.m[1][3] -
            viewMtx.m[2][1] * viewMtx.m[2][3];
    pos.z = -viewMtx.m[0][2] * viewMtx.m[0][3] - viewMtx.m[1][2] * viewMtx.m[1][3] -
            viewMtx.m[2][2] * viewMtx.m[2][3];
    *outPos = pos;
}

void calcMtxMul(sead::Vector3f* outVec, const sead::Matrix34f& mtx, const sead::Vector3f& vec) {
    outVec->setMul(mtx, vec);
}

void calcMtxMul(sead::Vector3f* outVec, const Matrix43f& mtx, const sead::Vector3f& vec) {
    outVec->x = mtx.m[0][0] * vec.x + mtx.m[1][0] * vec.y + mtx.m[2][0] * vec.z + mtx.m[3][0];
    outVec->y = mtx.m[0][1] * vec.x + mtx.m[1][1] * vec.y + mtx.m[2][1] * vec.z + mtx.m[3][1];
    outVec->z = mtx.m[0][2] * vec.x + mtx.m[1][2] * vec.y + mtx.m[2][2] * vec.z + mtx.m[3][2];
}

void calcMtxScale(sead::Vector3f* outMtx, const sead::Matrix34f& mtx) {
    outMtx->x = sead::Mathf::sqrt(mtx.m[0][0] * mtx.m[0][0] + mtx.m[1][0] * mtx.m[1][0] +
                                  mtx.m[2][0] * mtx.m[2][0]);

    outMtx->y = sead::Mathf::sqrt(mtx.m[0][1] * mtx.m[0][1] + mtx.m[1][1] * mtx.m[1][1] +
                                  mtx.m[2][1] * mtx.m[2][1]);

    outMtx->z = sead::Mathf::sqrt(mtx.m[0][2] * mtx.m[0][2] + mtx.m[1][2] * mtx.m[1][2] +
                                  mtx.m[2][2] * mtx.m[2][2]);
}

void calcMtxScale(sead::Vector3f* outMtx, const Matrix43f& mtx) {
    outMtx->x = sead::Mathf::sqrt(mtx.m[0][0] * mtx.m[0][0] + mtx.m[0][1] * mtx.m[0][1] +
                                  mtx.m[0][2] * mtx.m[0][2]);

    outMtx->y = sead::Mathf::sqrt(mtx.m[1][0] * mtx.m[1][0] + mtx.m[1][1] * mtx.m[1][1] +
                                  mtx.m[1][2] * mtx.m[1][2]);

    outMtx->z = sead::Mathf::sqrt(mtx.m[2][0] * mtx.m[2][0] + mtx.m[2][1] * mtx.m[2][1] +
                                  mtx.m[2][2] * mtx.m[2][2]);
}

void normalizeMtxScale(sead::Matrix34f* outMtx, const sead::Matrix34f& mtx) {
    sead::Vector3f scale;
    calcMtxScale(&scale, mtx);

    if (!isNearZero(scale.x, 0.001f) && !isNearZero(scale.y, 0.001f) &&
        !isNearZero(scale.z, 0.001f)) {
        f32 xInv = 1.0f / scale.x;
        f32 yInv = 1.0f / scale.y;
        f32 zInv = 1.0f / scale.z;

        *outMtx = mtx;
        outMtx->scaleBases(xInv, yInv, zInv);
    }
}

bool tryNormalizeMtxScaleOrIdentity(sead::Matrix34f* outMtx, const sead::Matrix34f& mtx) {
    sead::Vector3f scale;
    calcMtxScale(&scale, mtx);

    if (!isNearZero(scale.x, 0.001f) && !isNearZero(scale.y, 0.001f) &&
        !isNearZero(scale.z, 0.001f)) {
        f32 xInv = 1.0f / scale.x;
        f32 yInv = 1.0f / scale.y;
        f32 zInv = 1.0f / scale.z;

        *outMtx = mtx;
        outMtx->scaleBases(xInv, yInv, zInv);
        return true;
    }

    sead::Vector3f base = mtx.getBase(3);
    outMtx->makeIdentity();
    outMtx->setBase(3, base);

    return false;
}

void calcMtxLocalTrans(sead::Vector3f* outVec, const sead::Matrix34f& mtx,
                       const sead::Vector3f& trans) {
    sead::Vector3f diff = trans - mtx.getTranslation();
    outVec->x = diff.x * mtx.m[0][0] + diff.y * mtx.m[1][0] + diff.z * mtx.m[2][0];
    outVec->y = diff.x * mtx.m[0][1] + diff.y * mtx.m[1][1] + diff.z * mtx.m[2][1];
    outVec->z = diff.x * mtx.m[0][2] + diff.y * mtx.m[1][2] + diff.z * mtx.m[2][2];
}

void calcTransLocalOffsetByMtx(sead::Vector3f* outVec, const sead::Matrix34f& mtx,
                               const sead::Vector3f& offset) {
    sead::Vector3f rotated;
    rotated.setRotated(mtx, offset);
    *outVec = sead::Vector3f::zero + rotated + mtx.getTranslation();
}

void preScaleMtx(sead::Matrix34f* outMtx, const sead::Vector3f& scale) {
    outMtx->scaleBases(scale.x, scale.y, scale.z);
}

void addTransMtxLocalOffset(sead::Matrix34f* outMtx, const sead::Matrix34f& mtx,
                            const sead::Vector3f& offset) {
    for (s32 i = 0; i < 3; i++) {
        outMtx->m[i][0] = mtx.m[i][0];
        outMtx->m[i][1] = mtx.m[i][1];
        outMtx->m[i][2] = mtx.m[i][2];
        outMtx->m[i][3] = mtx.m[i][3] + (mtx.m[i][0] * offset.x + mtx.m[i][1] * offset.y +
                                         mtx.m[i][2] * offset.z);
    }
}

/**
 * Blends two matrices by interpolating translation linearly and rotation spherically.
 * @param pOutMtx Output matrix.
 * @param rMtxA First matrix.
 * @param rMtxB Second matrix.
 * @param rate Blend rate from the first to the second matrix.
 */
void blendMtx(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtxA, const sead::Matrix34f& rMtxB,
              f32 rate) {
    sead::Vector3f transA = rMtxA.getTranslation();
    sead::Vector3f transB = rMtxB.getTranslation();
    sead::Vector3f trans = transA * (1.0f - rate) + transB * rate;
    sead::Quatf quatA;
    sead::Quatf quatB;
    rMtxA.toQuat(quatA);
    rMtxB.toQuat(quatB);
    sead::Quatf quat;
    slerpQuat(&quat, quatA, quatB, rate);
    pOutMtx->makeQT(quat, trans);
}

/**
 * Blends the rotations of two matrices spherically and clears the translation.
 * @param pOutMtx Output matrix.
 * @param rMtxA First matrix.
 * @param rMtxB Second matrix.
 * @param rate Blend rate from the first to the second matrix.
 */
void blendMtxRotate(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtxA,
                    const sead::Matrix34f& rMtxB, f32 rate) {
    sead::Quatf quatA;
    sead::Quatf quatB;
    rMtxA.toQuat(quatA);
    rMtxB.toQuat(quatB);
    sead::Quatf quat;
    slerpQuat(&quat, quatA, quatB, rate);
    pOutMtx->fromQuat(quat);
}

void blendMtxTrans(sead::Matrix34f* outMtx, const sead::Matrix34f& mtxA,
                   const sead::Matrix34f& mtxB, f32 rate) {
    sead::Vector3f transA = mtxA.getTranslation();
    sead::Vector3f transB = mtxB.getTranslation();
    outMtx->setTranslation(transA * (1.0f - rate) + transB * rate);
}

/**
 * Calculates the angle on a local plane from a matrix axis to the direction of a target.
 * @param pMtx Matrix to use.
 * @param rTarget Target position.
 * @param dirAxis Axis index of the reference direction.
 * @param planeAxis Axis index of the plane normal.
 * @return Angle in degrees, or 0 if the plane normal is zero.
 */
f32 calcMtxLocalDirAngleOnPlaneToTarget(const sead::Matrix34f* pMtx, const sead::Vector3f& rTarget,
                                        s32 dirAxis, s32 planeAxis) {
    sead::Vector3f trans;
    pMtx->getTranslation(trans);
    sead::Vector3f targetDir = rTarget - trans;
    tryNormalizeOrZero(&targetDir);
    return calcMtxLocalDirAngleOnPlaneToDir(pMtx, targetDir, dirAxis, planeAxis);
}

/**
 * Calculates the angle on a local plane from a matrix axis to a direction.
 * @param pMtx Matrix to use.
 * @param rDir Direction to measure.
 * @param dirAxis Axis index of the reference direction.
 * @param planeAxis Axis index of the plane normal.
 * @return Angle in degrees, or 0 if the plane normal is zero.
 */
f32 calcMtxLocalDirAngleOnPlaneToDir(const sead::Matrix34f* pMtx, const sead::Vector3f& rDir,
                                     s32 dirAxis, s32 planeAxis) {
    sead::Vector3f dir;
    pMtx->getBase(dir, dirAxis);
    sead::Vector3f planeNormal;
    pMtx->getBase(planeNormal, planeAxis);
    sead::Vector3f normal;

    if (!tryNormalizeOrZero(&normal, planeNormal)) {
        return 0.0f;
    }

    return calcAngleOnPlaneDegree(rDir, dir, normal);
}

bool calcRotAxisOrZero(sead::Vector3f* outAxis, const sead::Matrix34f& mtx) {
    outAxis->set(mtx.m[2][1] - mtx.m[1][2], mtx.m[0][2] - mtx.m[2][0], mtx.m[1][0] - mtx.m[0][1]);
    return !tryNormalizeOrZero(outAxis);
}

void calcMxtInvertOrtho(sead::Matrix34f* pOut, const sead::Matrix34f& rMtx) {
    pOut->m[0][0] = rMtx.m[0][0];
    pOut->m[0][1] = rMtx.m[1][0];
    pOut->m[0][2] = rMtx.m[2][0];
    pOut->m[0][3] = 0.0f;
    pOut->m[1][0] = rMtx.m[0][1];
    pOut->m[1][1] = rMtx.m[1][1];
    pOut->m[1][2] = rMtx.m[2][1];
    pOut->m[1][3] = 0.0f;
    pOut->m[2][0] = rMtx.m[0][2];
    pOut->m[2][1] = rMtx.m[1][2];
    pOut->m[2][2] = rMtx.m[2][2];
    pOut->m[2][3] = 0.0f;

    pOut->m[0][3] = -rMtx.m[0][3] * pOut->m[0][0] - rMtx.m[1][3] * pOut->m[0][1] -
                    rMtx.m[2][3] * pOut->m[0][2];
    pOut->m[1][3] = -rMtx.m[0][3] * pOut->m[1][0] - rMtx.m[1][3] * pOut->m[1][1] -
                    rMtx.m[2][3] * pOut->m[1][2];
    pOut->m[2][3] = -rMtx.m[0][3] * pOut->m[2][0] - rMtx.m[1][3] * pOut->m[2][1] -
                    rMtx.m[2][3] * pOut->m[2][2];
}

void calcNearFarByInvProjection(f32* outNear, f32* outFar, const sead::Matrix44f& invProj) {
    f32 z = invProj.m[2][0] * 0.0f + invProj.m[2][1] * 0.0f;
    f32 w = invProj.m[3][0] * 0.0f + invProj.m[3][1] * 0.0f;
    *outNear = -(invProj.m[2][3] + (z - invProj.m[2][2])) / (invProj.m[3][3] + (w - invProj.m[3][2]));
    *outFar = -(invProj.m[2][3] + (invProj.m[2][2] + z)) / (invProj.m[3][3] + (invProj.m[3][2] + w));
}

void calcMovedInertiaTensor(sead::Matrix33f* pOut, const sead::Matrix33f& rTensor,
                            const sead::Vector3f& rPos, f32 mass) {
    f32 x = rPos.x;
    f32 y = rPos.y;
    f32 z = rPos.z;
    *pOut = rTensor;
    pOut->m[0][0] = (y * y + z * z) * mass + pOut->m[0][0];
    pOut->m[0][1] = pOut->m[0][1] - y * x * mass;
    pOut->m[0][2] = pOut->m[0][2] - z * x * mass;
    pOut->m[1][0] = pOut->m[1][0] - y * x * mass;
    pOut->m[1][1] = (x * x + z * z) * mass + pOut->m[1][1];
    pOut->m[1][2] = pOut->m[1][2] - z * y * mass;
    pOut->m[2][0] = pOut->m[2][0] - z * x * mass;
    pOut->m[2][1] = pOut->m[2][1] - z * y * mass;
    pOut->m[2][2] = (x * x + y * y) * mass + pOut->m[2][2];
}

void calcInertiaTensorByMovedTensorAndCenter(sead::Matrix33f* pOut,
                                             const sead::Matrix33f& rTensor,
                                             const sead::Vector3f& rPos, f32 mass) {
    f32 x = rPos.x;
    f32 y = rPos.y;
    f32 z = rPos.z;
    *pOut = rTensor;
    pOut->m[0][0] = pOut->m[0][0] - (y * y + z * z) * mass;
    pOut->m[0][1] = pOut->m[0][1] + y * x * mass;
    pOut->m[0][2] = pOut->m[0][2] + z * x * mass;
    pOut->m[1][0] = pOut->m[1][0] + y * x * mass;
    pOut->m[1][1] = pOut->m[1][1] - (x * x + z * z) * mass;
    pOut->m[1][2] = pOut->m[1][2] + z * y * mass;
    pOut->m[2][0] = pOut->m[2][0] + z * x * mass;
    pOut->m[2][1] = pOut->m[2][1] + z * y * mass;
    pOut->m[2][2] = pOut->m[2][2] - (x * x + y * y) * mass;
}

void calcInertiaTensorSphere(sead::Matrix33f* outMtx, f32 radius, f32 mass) {
    f32 inertia = 0.4f * radius * radius * mass;
    outMtx->m[0][1] = 0.0f;
    outMtx->m[0][2] = 0.0f;
    outMtx->m[1][0] = 0.0f;
    outMtx->m[1][2] = 0.0f;
    outMtx->m[2][0] = 0.0f;
    outMtx->m[2][1] = 0.0f;
    outMtx->m[0][0] = inertia;
    outMtx->m[1][1] = inertia;
    outMtx->m[2][2] = inertia;
}

void calcInertiaTensorBox(sead::Matrix33f* outMtx, const sead::Vector3f& size, f32 mass) {
    f32 xx = size.x * size.x;
    f32 yy = size.y * size.y;
    f32 zz = size.z * size.z;
    outMtx->m[0][1] = 0.0f;
    outMtx->m[0][2] = 0.0f;
    outMtx->m[1][0] = 0.0f;
    outMtx->m[1][2] = 0.0f;
    outMtx->m[2][0] = 0.0f;
    outMtx->m[2][1] = 0.0f;
    outMtx->m[0][0] = (yy + zz) / 12.0f * mass;
    outMtx->m[1][1] = (xx + zz) / 12.0f * mass;
    outMtx->m[2][2] = (xx + yy) / 12.0f * mass;
}

void makeMtx34f(sead::Matrix34f* outMtx, const nn::util::neon::MatrixColumnMajor4x3fType& mtx) {
    *reinterpret_cast<nn::util::neon::MatrixColumnMajor4x3fType*>(outMtx) = mtx;
}

void makeMtx44f(sead::Matrix44f* outMtx, const nn::util::neon::MatrixColumnMajor4x4fType& mtx) {}

}  // namespace al
