#include "Library/Math/MatrixUtil.hpp"

#include <nn/util/util_MathTypes.h>

#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * Makes a matrix from a rotation in degrees and a translation.
 * @param pOutMtx Receives the matrix.
 * @param rRotate Rotation in degrees.
 * @param rTrans Translation.
 */
void makeMtxRotateTrans(sead::Matrix34f* pOutMtx, const sead::Vector3f& rRotate,
                        const sead::Vector3f& rTrans) {
    sead::Vector3f rotateRad(sead::Mathf::deg2rad(rRotate.x), sead::Mathf::deg2rad(rRotate.y),
                             sead::Mathf::deg2rad(rRotate.z));
    pOutMtx->makeRT(rotateRad, rTrans);
}

/**
 * Makes a rotation matrix from two axis directions, deriving the third one.
 * @param pOutMtx Receives the matrix.
 * @param rVectorA First axis direction.
 * @param rVectorB Second axis direction.
 * @param axisA Index of the first axis.
 * @param axisB Index of the second axis.
 */
void makeMtxFromTwoAxis(sead::Matrix34f* pOutMtx, const sead::Vector3f& rVectorA,
                        const sead::Vector3f& rVectorB, s32 axisA, s32 axisB) {
    sead::Vector3f dir[3];

    s32 axisC;

    if (axisA == 0) {
        axisC = axisB == 1 ? 2 : 1;
    } else if (axisA == 1) {
        axisC = axisB == 0 ? 2 : 0;
    } else {
        axisC = axisB == 0 ? 1 : 0;
    }

    normalize(&dir[axisA], rVectorA);
    dir[axisB] = rVectorB;

    dir[axisC].setCross(dir[(axisC + 1) % 3], dir[(axisC + 2) % 3]);
    normalize(&dir[axisC]);

    dir[axisB].setCross(dir[(axisB + 1) % 3], dir[(axisB + 2) % 3]);

    pOutMtx->setBase(0, dir[0]);  // side
    pOutMtx->setBase(1, dir[1]);  // up
    pOutMtx->setBase(2, dir[2]);  // front
}

/**
 * Makes a rotation matrix from a front direction, using an up direction as support.
 * @param pOutMtx Receives the matrix.
 * @param rFront Front direction.
 * @param rUp Up direction.
 */
void makeMtxFrontUp(sead::Matrix34f* pOutMtx, const sead::Vector3f& rFront,
                    const sead::Vector3f& rUp) {
    sead::Vector3f frontNorm = rFront;
    normalize(&frontNorm);

    sead::Vector3f side;
    side.setCross(rUp, frontNorm);
    normalize(&side);

    sead::Vector3f upNorm;
    upNorm.setCross(frontNorm, side);

    pOutMtx->setBase(0, side);
    pOutMtx->setBase(1, upNorm);
    pOutMtx->setBase(2, frontNorm);
}

/**
 * Makes a rotation matrix from a front direction, using a side direction as support.
 * @param pOutMtx Receives the matrix.
 * @param rFront Front direction.
 * @param rSide Side direction.
 */
void makeMtxFrontSide(sead::Matrix34f* pOutMtx, const sead::Vector3f& rFront,
                      const sead::Vector3f& rSide) {
    sead::Vector3f frontNorm = rFront;
    normalize(&frontNorm);

    sead::Vector3f up;
    up.setCross(frontNorm, rSide);
    normalize(&up);

    sead::Vector3f sideNorm;
    sideNorm.setCross(up, frontNorm);

    pOutMtx->setBase(0, sideNorm);
    pOutMtx->setBase(1, up);
    pOutMtx->setBase(2, frontNorm);
}

/**
 * Makes a rotation matrix from an up direction, using a front direction as support.
 * @param pOutMtx Receives the matrix.
 * @param rUp Up direction.
 * @param rFront Front direction.
 */
void makeMtxUpFront(sead::Matrix34f* pOutMtx, const sead::Vector3f& rUp,
                    const sead::Vector3f& rFront) {
    sead::Vector3f upNorm = rUp;
    normalize(&upNorm);

    sead::Vector3f side;
    side.setCross(rUp, rFront);
    normalize(&side);

    sead::Vector3f frontNorm;
    frontNorm.setCross(side, upNorm);

    pOutMtx->setBase(0, side);
    pOutMtx->setBase(1, upNorm);
    pOutMtx->setBase(2, frontNorm);
}

/**
 * Makes a rotation matrix from an up direction, using a side direction as support.
 * @param pOutMtx Receives the matrix.
 * @param rUp Up direction.
 * @param rSide Side direction.
 */
void makeMtxUpSide(sead::Matrix34f* pOutMtx, const sead::Vector3f& rUp,
                   const sead::Vector3f& rSide) {
    sead::Vector3f upNorm = rUp;
    normalize(&upNorm);

    sead::Vector3f front;
    front.setCross(rSide, rUp);
    normalize(&front);

    sead::Vector3f sideNorm;
    sideNorm.setCross(upNorm, front);

    pOutMtx->setBase(0, sideNorm);
    pOutMtx->setBase(1, upNorm);
    pOutMtx->setBase(2, front);
}

/**
 * Makes a rotation matrix from a side direction, using an up direction as support.
 * @param pOutMtx Receives the matrix.
 * @param rSide Side direction.
 * @param rUp Up direction.
 */
void makeMtxSideUp(sead::Matrix34f* pOutMtx, const sead::Vector3f& rSide,
                   const sead::Vector3f& rUp) {
    makeMtxFromTwoAxis(pOutMtx, rSide, rUp, 0, 1);
}

/**
 * Makes a rotation matrix from a side direction, using a front direction as support.
 * @param pOutMtx Receives the matrix.
 * @param rSide Side direction.
 * @param rFront Front direction.
 */
void makeMtxSideFront(sead::Matrix34f* pOutMtx, const sead::Vector3f& rSide,
                      const sead::Vector3f& rFront) {
    makeMtxFromTwoAxis(pOutMtx, rSide, rFront, 0, 2);
}

/**
 * Makes a rotation matrix from a front direction, picking a support axis automatically.
 * @param pOutMtx Receives the matrix.
 * @param rFront Front direction.
 */
void makeMtxFrontNoSupport(sead::Matrix34f* pOutMtx, const sead::Vector3f& rFront) {
    bool isYAxis = getMaxAbsElementIndex(rFront) == 1;

    sead::Vector3f up;
    up.x = isYAxis ? sead::Vector3f::ez.x : sead::Vector3f::ey.x;
    up.y = isYAxis ? sead::Vector3f::ez.y : sead::Vector3f::ey.y;
    up.z = isYAxis ? sead::Vector3f::ez.z : sead::Vector3f::ey.z;

    makeMtxFrontUp(pOutMtx, rFront, up);
}

/**
 * Makes a matrix from a front direction and a position, picking a support axis automatically.
 * @param pOutMtx Receives the matrix.
 * @param rFront Front direction.
 * @param rPos Position.
 */
void makeMtxFrontNoSupportPos(sead::Matrix34f* pOutMtx, const sead::Vector3f& rFront,
                              const sead::Vector3f& rPos) {
    makeMtxFrontNoSupport(pOutMtx, rFront);
    pOutMtx->setBase(3, rPos);
}

/**
 * Makes a rotation matrix from an up direction, picking a support axis automatically.
 * @param pOutMtx Receives the matrix.
 * @param rUp Up direction.
 */
void makeMtxUpNoSupport(sead::Matrix34f* pOutMtx, const sead::Vector3f& rUp) {
    bool isZAxis = getMaxAbsElementIndex(rUp) == 2;

    sead::Vector3f front;
    front.x = isZAxis ? sead::Vector3f::ex.x : sead::Vector3f::ez.x;
    front.y = isZAxis ? sead::Vector3f::ex.y : sead::Vector3f::ez.y;
    front.z = isZAxis ? sead::Vector3f::ex.z : sead::Vector3f::ez.z;

    makeMtxUpFront(pOutMtx, rUp, front);
}

/**
 * Makes a matrix from an up direction and a position, picking a support axis automatically.
 * @param pOutMtx Receives the matrix.
 * @param rUp Up direction.
 * @param rPos Position.
 */
void makeMtxUpNoSupportPos(sead::Matrix34f* pOutMtx, const sead::Vector3f& rUp,
                           const sead::Vector3f& rPos) {
    makeMtxUpNoSupport(pOutMtx, rUp);
    pOutMtx->setBase(3, rPos);
}

/**
 * Makes a rotation matrix from a side direction, picking a support axis automatically.
 * @param pOutMtx Receives the matrix.
 * @param rSide Side direction.
 */
void makeMtxSideNoSupport(sead::Matrix34f* pOutMtx, const sead::Vector3f& rSide) {
    bool isYAxis = getMaxAbsElementIndex(rSide) == 1;

    sead::Vector3f up;

    if (isYAxis) {
        up = sead::Vector3f::ez;
    } else {
        up = sead::Vector3f::ey;
    }

    makeMtxSideUp(pOutMtx, rSide, up);
}

/**
 * Makes a matrix from a side direction and a position, picking a support axis automatically.
 * @param pOutMtx Receives the matrix.
 * @param rSide Side direction.
 * @param rPos Position.
 */
void makeMtxSideNoSupportPos(sead::Matrix34f* pOutMtx, const sead::Vector3f& rSide,
                             const sead::Vector3f& rPos) {
    makeMtxSideNoSupport(pOutMtx, rSide);
    pOutMtx->setBase(3, rPos);
}

/**
 * Makes a matrix from a rotation and a position.
 * @param pOutMtx Receives the matrix.
 * @param rQuat Rotation.
 * @param rPos Position.
 */
void makeMtxQuatPos(sead::Matrix34f* pOutMtx, const sead::Quatf& rQuat,
                    const sead::Vector3f& rPos) {
    pOutMtx->fromQuat(rQuat);
    pOutMtx->setTranslation(rPos);
}

/**
 * Makes a matrix from a rotation, a scale and a position.
 * @param pOutMtx Receives the matrix.
 * @param rQuat Rotation.
 * @param rScale Scale.
 * @param rPos Position.
 */
void makeMtxQuatScalePos(sead::Matrix34f* pOutMtx, const sead::Quatf& rQuat,
                         const sead::Vector3f& rScale, const sead::Vector3f& rPos) {
    pOutMtx->fromQuat(rQuat);
    pOutMtx->scaleBases(rScale.x, rScale.y, rScale.z);
    pOutMtx->setTranslation(rPos);
}

/**
 * Makes a 4x4 matrix from a rotation, a scale and a position.
 * @param pOutMtx Receives the matrix.
 * @param rQuat Rotation.
 * @param rScale Scale.
 * @param rPos Position.
 */
void makeMtxQuatScalePos(sead::Matrix44f* pOutMtx, const sead::Quatf& rQuat,
                         const sead::Vector3f& rScale, const sead::Vector3f& rPos) {
    pOutMtx->fromQuat(rQuat);
    pOutMtx->scaleBases(rScale.x, rScale.y, rScale.z, 1.0f);
    pOutMtx->setCol(3, sead::Vector4f(rPos.x, rPos.y, rPos.z, 1.0f));
}

/**
 * Makes a matrix from front and up directions and a position.
 * @param pOutMtx Receives the matrix.
 * @param rFront Front direction.
 * @param rUp Up direction.
 * @param rPos Position.
 */
void makeMtxFrontUpPos(sead::Matrix34f* pOutMtx, const sead::Vector3f& rFront,
                       const sead::Vector3f& rUp, const sead::Vector3f& rPos) {
    makeMtxFrontUp(pOutMtx, rFront, rUp);
    pOutMtx->setBase(3, rPos);
}

/**
 * Makes a matrix from front and side directions and a position.
 * @param pOutMtx Receives the matrix.
 * @param rFront Front direction.
 * @param rSide Side direction.
 * @param rPos Position.
 */
void makeMtxFrontSidePos(sead::Matrix34f* pOutMtx, const sead::Vector3f& rFront,
                         const sead::Vector3f& rSide, const sead::Vector3f& rPos) {
    makeMtxFrontSide(pOutMtx, rFront, rSide);
    pOutMtx->setBase(3, rPos);
}

/**
 * Makes a matrix from up and front directions and a position.
 * @param pOutMtx Receives the matrix.
 * @param rUp Up direction.
 * @param rFront Front direction.
 * @param rPos Position.
 */
void makeMtxUpFrontPos(sead::Matrix34f* pOutMtx, const sead::Vector3f& rUp,
                       const sead::Vector3f& rFront, const sead::Vector3f& rPos) {
    makeMtxUpFront(pOutMtx, rUp, rFront);
    pOutMtx->setBase(3, rPos);
}

/**
 * Makes a matrix from up and side directions and a position.
 * @param pOutMtx Receives the matrix.
 * @param rUp Up direction.
 * @param rSide Side direction.
 * @param rPos Position.
 */
void makeMtxUpSidePos(sead::Matrix34f* pOutMtx, const sead::Vector3f& rUp,
                      const sead::Vector3f& rSide, const sead::Vector3f& rPos) {
    makeMtxUpSide(pOutMtx, rUp, rSide);
    pOutMtx->setBase(3, rPos);
}

/**
 * Makes a matrix from side and up directions and a position.
 * @param pOutMtx Receives the matrix.
 * @param rSide Side direction.
 * @param rUp Up direction.
 * @param rPos Position.
 */
void makeMtxSideUpPos(sead::Matrix34f* pOutMtx, const sead::Vector3f& rSide,
                      const sead::Vector3f& rUp, const sead::Vector3f& rPos) {
    makeMtxSideUp(pOutMtx, rSide, rUp);
    pOutMtx->setBase(3, rPos);
}

/**
 * Makes a matrix from side and front directions and a position.
 * @param pOutMtx Receives the matrix.
 * @param rSide Side direction.
 * @param rFront Front direction.
 * @param rPos Position.
 */
void makeMtxSideFrontPos(sead::Matrix34f* pOutMtx, const sead::Vector3f& rSide,
                         const sead::Vector3f& rFront, const sead::Vector3f& rPos) {
    makeMtxSideFront(pOutMtx, rSide, rFront);
    pOutMtx->setBase(3, rPos);
}

/**
 * Makes a matrix following a base matrix with a local offset and rotation.
 * @param pOutMtx Receives the matrix.
 * @param rBaseMtx Base matrix.
 * @param rTrans Local translation.
 * @param rRotate Local rotation in degrees.
 */
void makeMtxFollowTarget(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rBaseMtx,
                         const sead::Vector3f& rTrans, const sead::Vector3f& rRotate) {
    sead::Matrix34f rotationMatrix;
    sead::Vector3f rotateRad(sead::Mathf::deg2rad(rRotate.x), sead::Mathf::deg2rad(rRotate.y),
                             sead::Mathf::deg2rad(rRotate.z));
    rotationMatrix.makeR(rotateRad);

    sead::Matrix34f translationMatrix;
    translationMatrix.makeRT({0.0f, 0.0f, 0.0f}, rTrans);

    sead::Matrix34f pose = rotationMatrix * translationMatrix;
    *pOutMtx = rBaseMtx * pose;
}

/**
 * Multiplies a 3x4 matrix, extended with a (0, 0, 0, 1) row, by a 4x4 matrix.
 * @param rOut Receives the product.
 * @param rA Left 3x4 matrix.
 * @param rB Right 4x4 matrix.
 */
static inline void multiplyMtx34Mtx44(sead::Matrix44f& rOut, const sead::Matrix34f& rA,
                                      const sead::Matrix44f& rB) {
    float32x4_t a0 = vld1q_f32(rA.m[0]);
    float32x4_t a1 = vld1q_f32(rA.m[1]);
    float32x4_t a2 = vld1q_f32(rA.m[2]);

    float32x4_t b0 = vld1q_f32(rB.m[0]);
    float32x4_t b1 = vld1q_f32(rB.m[1]);
    float32x4_t b2 = vld1q_f32(rB.m[2]);
    float32x4_t b3 = vld1q_f32(rB.m[3]);

    float32x4_t c0 = vmulq_laneq_f32(b0, a0, 0);
    c0 = vfmaq_laneq_f32(c0, b1, a0, 1);
    c0 = vfmaq_laneq_f32(c0, b2, a0, 2);
    c0 = vfmaq_laneq_f32(c0, b3, a0, 3);

    float32x4_t c1 = vmulq_laneq_f32(b0, a1, 0);
    c1 = vfmaq_laneq_f32(c1, b1, a1, 1);
    c1 = vfmaq_laneq_f32(c1, b2, a1, 2);
    c1 = vfmaq_laneq_f32(c1, b3, a1, 3);

    float32x4_t c2 = vmulq_laneq_f32(b0, a2, 0);
    c2 = vfmaq_laneq_f32(c2, b1, a2, 1);
    c2 = vfmaq_laneq_f32(c2, b2, a2, 2);
    c2 = vfmaq_laneq_f32(c2, b3, a2, 3);

    vst1q_f32(rOut.m[0], c0);
    vst1q_f32(rOut.m[1], c1);
    vst1q_f32(rOut.m[2], c2);
    vst1q_f32(rOut.m[3], b3);
}

/**
 * Makes a projection matrix mapping a rectangle facing a direction to unit texture space.
 * @param pOutMtx Receives the matrix.
 * @param rSize Size of the projected rectangle.
 * @param rFront Projection direction.
 * @param rUp Up direction of the rectangle.
 */
void makeMtxProj(sead::Matrix44f* pOutMtx, const sead::Vector2f& rSize,
                 const sead::Vector3f& rFront, const sead::Vector3f& rUp) {
    sead::Matrix44f scaleMtx(rSize.x, 0.0f, 0.0f, 0.0f, 0.0f, rSize.y, 0.0f, 0.0f, 0.0f, 0.0f,
                             1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    sead::Matrix34f mtx = sead::Matrix34f::ident;

    sead::Vector3f front = rFront;
    normalize(&front);
    sead::Vector3f side;
    side.setCross(rUp, front);
    normalize(&side);
    sead::Vector3f up;
    up.setCross(front, side);

    mtx.setBase(0, side);
    mtx.setBase(1, up);
    mtx.setBase(2, front);
    multiplyMtx34Mtx44(*pOutMtx, mtx, scaleMtx);
    pOutMtx->setInverse(*pOutMtx);
}

/**
 * Makes a projection matrix for a rectangle projected along the pose's up axis.
 * @param pOutMtx Receives the matrix.
 * @param rQuat Pose rotation.
 * @param rSize Size of the projected rectangle.
 * @param rPos Center of the rectangle.
 */
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

/**
 * Makes a projection matrix for a rectangle projected along the pose's front axis.
 * @param pOutMtx Receives the matrix.
 * @param rQuat Pose rotation.
 * @param rSize Size of the projected rectangle.
 * @param rPos Center of the rectangle.
 */
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

/**
 * Makes a projection matrix for a rectangle projected along the pose's side axis.
 * @param pOutMtx Receives the matrix.
 * @param rQuat Pose rotation.
 * @param rSize Size of the projected rectangle.
 * @param rPos Center of the rectangle.
 */
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

/**
 * Makes a projection matrix for a rectangle projected along the world Y axis.
 * @param pOutMtx Receives the matrix.
 * @param rSize Size of the projected rectangle.
 * @param rPos Center of the rectangle.
 */
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

/**
 * Rotates a matrix around its local X axis.
 * @param pOutMtx Receives the matrix.
 * @param rBaseMtx Base matrix.
 * @param angle Angle in degrees.
 */
void rotateMtxXDirDegree(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rBaseMtx, f32 angle) {
    sead::Matrix34f rotationMatrix;
    rotationMatrix.makeR(sead::Mathf::deg2rad(angle) * sead::Vector3f::ex);

    *pOutMtx = rBaseMtx * rotationMatrix;
}

/**
 * Rotates a matrix around its local Y axis.
 * @param pOutMtx Receives the matrix.
 * @param rBaseMtx Base matrix.
 * @param angle Angle in degrees.
 */
void rotateMtxYDirDegree(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rBaseMtx, f32 angle) {
    sead::Matrix34f rotationMatrix;
    rotationMatrix.makeR(sead::Mathf::deg2rad(angle) * sead::Vector3f::ey);

    *pOutMtx = rBaseMtx * rotationMatrix;
}

/**
 * Rotates a matrix around its local Z axis.
 * @param pOutMtx Receives the matrix.
 * @param rBaseMtx Base matrix.
 * @param angle Angle in degrees.
 */
void rotateMtxZDirDegree(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rBaseMtx, f32 angle) {
    sead::Matrix34f rotationMatrix;
    rotationMatrix.makeR(sead::Mathf::deg2rad(angle) * sead::Vector3f::ez);

    *pOutMtx = rBaseMtx * rotationMatrix;
}

/**
 * Rotates a matrix around its X axis through a center in local space.
 * @param pOutMtx Receives the matrix.
 * @param rMtx Source matrix.
 * @param rCenter Rotation center in local space.
 * @param degree Angle in degrees.
 */
void rotateMtxCenterPosXDirDegree(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtx,
                                  const sead::Vector3f& rCenter, f32 degree) {
    sead::Vector3f axis;
    rMtx.getBase(axis, 0);
    rotateMtxCenterPosAxisDegree(pOutMtx, rMtx, rCenter, axis, degree);
}

/**
 * Rotates a matrix around an axis through a center in local space.
 * @param pOutMtx Receives the matrix.
 * @param rMtx Source matrix.
 * @param rCenter Rotation center in local space.
 * @param rAxis Rotation axis.
 * @param degree Angle in degrees.
 */
void rotateMtxCenterPosAxisDegree(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtx,
                                  const sead::Vector3f& rCenter, const sead::Vector3f& rAxis,
                                  f32 degree) {
    sead::Quatf quat = sead::Quatf::unit;
    makeQuatRotateDegree(&quat, rAxis, degree);
    rotateMtxCenterPosQuat(pOutMtx, rMtx, rCenter, quat);
}

/**
 * Rotates a matrix around its Y axis through a center in local space.
 * @param pOutMtx Receives the matrix.
 * @param rMtx Source matrix.
 * @param rCenter Rotation center in local space.
 * @param degree Angle in degrees.
 */
void rotateMtxCenterPosYDirDegree(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtx,
                                  const sead::Vector3f& rCenter, f32 degree) {
    sead::Vector3f axis;
    rMtx.getBase(axis, 1);
    rotateMtxCenterPosAxisDegree(pOutMtx, rMtx, rCenter, axis, degree);
}

/**
 * Rotates a matrix around its Z axis through a center in local space.
 * @param pOutMtx Receives the matrix.
 * @param rMtx Source matrix.
 * @param rCenter Rotation center in local space.
 * @param degree Angle in degrees.
 */
void rotateMtxCenterPosZDirDegree(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtx,
                                  const sead::Vector3f& rCenter, f32 degree) {
    sead::Vector3f axis;
    rMtx.getBase(axis, 2);
    rotateMtxCenterPosAxisDegree(pOutMtx, rMtx, rCenter, axis, degree);
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
 * Turns the rotation of a matrix so that its X axis turns toward a direction, keeping the
 * translation.
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
 * Turns the rotation of a matrix so that its Y axis turns toward a direction, keeping the
 * translation.
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
 * Turns the rotation of a matrix so that its Z axis turns toward a direction, keeping the
 * translation.
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

/**
 * Calculates the camera position from a view matrix.
 * @param pOutPos Receives the position.
 * @param rViewMtx View matrix.
 */
void calcCameraPosFromViewMtx(sead::Vector3f* pOutPos, const sead::Matrix34f& rViewMtx) {
    sead::Vector3f pos;
    pos.x = -rViewMtx.m[0][0] * rViewMtx.m[0][3] - rViewMtx.m[1][0] * rViewMtx.m[1][3] -
            rViewMtx.m[2][0] * rViewMtx.m[2][3];
    pos.y = -rViewMtx.m[0][1] * rViewMtx.m[0][3] - rViewMtx.m[1][1] * rViewMtx.m[1][3] -
            rViewMtx.m[2][1] * rViewMtx.m[2][3];
    pos.z = -rViewMtx.m[0][2] * rViewMtx.m[0][3] - rViewMtx.m[1][2] * rViewMtx.m[1][3] -
            rViewMtx.m[2][2] * rViewMtx.m[2][3];
    *pOutPos = pos;
}

/**
 * Transforms a vector by a matrix.
 * @param pOutVec Receives the vector.
 * @param rMtx Source matrix.
 * @param rVec Vector to transform.
 */
void calcMtxMul(sead::Vector3f* pOutVec, const sead::Matrix34f& rMtx, const sead::Vector3f& rVec) {
    pOutVec->setMul(rMtx, rVec);
}

/**
 * Transforms a vector by a column-major 4x3 matrix.
 * @param pOutVec Receives the vector.
 * @param rMtx Source matrix.
 * @param rVec Vector to transform.
 */
void calcMtxMul(sead::Vector3f* pOutVec, const Matrix43f& rMtx, const sead::Vector3f& rVec) {
    pOutVec->x =
        rMtx.m[0][0] * rVec.x + rMtx.m[1][0] * rVec.y + rMtx.m[2][0] * rVec.z + rMtx.m[3][0];
    pOutVec->y =
        rMtx.m[0][1] * rVec.x + rMtx.m[1][1] * rVec.y + rMtx.m[2][1] * rVec.z + rMtx.m[3][1];
    pOutVec->z =
        rMtx.m[0][2] * rVec.x + rMtx.m[1][2] * rVec.y + rMtx.m[2][2] * rVec.z + rMtx.m[3][2];
}

/**
 * Calculates the scale of each matrix base.
 * @param pOutMtx Receives the scale.
 * @param rMtx Source matrix.
 */
void calcMtxScale(sead::Vector3f* pOutMtx, const sead::Matrix34f& rMtx) {
    pOutMtx->x = sead::Mathf::sqrt(rMtx.m[0][0] * rMtx.m[0][0] + rMtx.m[1][0] * rMtx.m[1][0] +
                                  rMtx.m[2][0] * rMtx.m[2][0]);

    pOutMtx->y = sead::Mathf::sqrt(rMtx.m[0][1] * rMtx.m[0][1] + rMtx.m[1][1] * rMtx.m[1][1] +
                                  rMtx.m[2][1] * rMtx.m[2][1]);

    pOutMtx->z = sead::Mathf::sqrt(rMtx.m[0][2] * rMtx.m[0][2] + rMtx.m[1][2] * rMtx.m[1][2] +
                                  rMtx.m[2][2] * rMtx.m[2][2]);
}

/**
 * Calculates the scale of each base of a column-major 4x3 matrix.
 * @param pOutMtx Receives the scale.
 * @param rMtx Source matrix.
 */
void calcMtxScale(sead::Vector3f* pOutMtx, const Matrix43f& rMtx) {
    pOutMtx->x = sead::Mathf::sqrt(rMtx.m[0][0] * rMtx.m[0][0] + rMtx.m[0][1] * rMtx.m[0][1] +
                                  rMtx.m[0][2] * rMtx.m[0][2]);

    pOutMtx->y = sead::Mathf::sqrt(rMtx.m[1][0] * rMtx.m[1][0] + rMtx.m[1][1] * rMtx.m[1][1] +
                                  rMtx.m[1][2] * rMtx.m[1][2]);

    pOutMtx->z = sead::Mathf::sqrt(rMtx.m[2][0] * rMtx.m[2][0] + rMtx.m[2][1] * rMtx.m[2][1] +
                                  rMtx.m[2][2] * rMtx.m[2][2]);
}

/**
 * Removes the scale from a matrix, leaving near-zero bases untouched.
 * @param pOutMtx Receives the matrix.
 * @param rMtx Source matrix.
 */
void normalizeMtxScale(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtx) {
    sead::Vector3f scale;
    calcMtxScale(&scale, rMtx);

    if (!isNearZero(scale.x, 0.001f)) {
        scale.x = 1.0f / scale.x;
    }

    if (!isNearZero(scale.y, 0.001f)) {
        scale.y = 1.0f / scale.y;
    }

    if (!isNearZero(scale.z, 0.001f)) {
        scale.z = 1.0f / scale.z;
    }

    *pOutMtx = rMtx;
    pOutMtx->scaleBases(scale.x, scale.y, scale.z);
}

/**
 * Removes the scale from a matrix, or resets its rotation if a base is near zero.
 * @param pOutMtx Receives the matrix.
 * @param rMtx Source matrix.
 * @return Whether the scale could be removed.
 */
bool tryNormalizeMtxScaleOrIdentity(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtx) {
    sead::Vector3f scale;
    calcMtxScale(&scale, rMtx);

    if (!isNearZero(scale.x, 0.001f) && !isNearZero(scale.y, 0.001f) &&
        !isNearZero(scale.z, 0.001f)) {
        f32 xInv = 1.0f / scale.x;
        f32 yInv = 1.0f / scale.y;
        f32 zInv = 1.0f / scale.z;

        *pOutMtx = rMtx;
        pOutMtx->scaleBases(xInv, yInv, zInv);
        return true;
    }

    sead::Vector3f base = rMtx.getBase(3);
    pOutMtx->makeIdentity();
    pOutMtx->setBase(3, base);

    return false;
}

/**
 * Converts a world position into a matrix's local space, ignoring the scale.
 * @param pOutVec Receives the vector.
 * @param rMtx Source matrix.
 * @param rTrans World position.
 */
void calcMtxLocalTrans(sead::Vector3f* pOutVec, const sead::Matrix34f& rMtx,
                       const sead::Vector3f& rTrans) {
    sead::Vector3f diff = rTrans - rMtx.getTranslation();
    pOutVec->x = diff.x * rMtx.m[0][0] + diff.y * rMtx.m[1][0] + diff.z * rMtx.m[2][0];
    pOutVec->y = diff.x * rMtx.m[0][1] + diff.y * rMtx.m[1][1] + diff.z * rMtx.m[2][1];
    pOutVec->z = diff.x * rMtx.m[0][2] + diff.y * rMtx.m[1][2] + diff.z * rMtx.m[2][2];
}

/**
 * Converts a local offset of a matrix into a world position.
 * @param pOutVec Receives the vector.
 * @param rMtx Source matrix.
 * @param rOffset Offset in the matrix's local space.
 */
void calcTransLocalOffsetByMtx(sead::Vector3f* pOutVec, const sead::Matrix34f& rMtx,
                               const sead::Vector3f& rOffset) {
    sead::Vector3f rotated(
        rMtx.m[0][0] * rOffset.x + rMtx.m[0][1] * rOffset.y + rMtx.m[0][2] * rOffset.z,
        rMtx.m[1][0] * rOffset.x + rMtx.m[1][1] * rOffset.y + rMtx.m[1][2] * rOffset.z,
        rMtx.m[2][0] * rOffset.x + rMtx.m[2][1] * rOffset.y + rMtx.m[2][2] * rOffset.z);
    *pOutVec = sead::Vector3f::zero + rotated + rMtx.getTranslation();
}

/**
 * Scales the bases of a matrix.
 * @param pOutMtx Matrix to scale.
 * @param rScale Scale.
 */
void preScaleMtx(sead::Matrix34f* pOutMtx, const sead::Vector3f& rScale) {
    pOutMtx->scaleBases(rScale.x, rScale.y, rScale.z);
}

/**
 * Copies a matrix and moves its translation by an offset in local space.
 * @param pOutMtx Receives the matrix.
 * @param rMtx Source matrix.
 * @param rOffset Offset in the matrix's local space.
 */
void addTransMtxLocalOffset(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtx,
                            const sead::Vector3f& rOffset) {
    for (s32 i = 0; i < 3; i++) {
        pOutMtx->m[i][0] = rMtx.m[i][0];
        pOutMtx->m[i][1] = rMtx.m[i][1];
        pOutMtx->m[i][2] = rMtx.m[i][2];
        pOutMtx->m[i][3] = rMtx.m[i][3] + (rMtx.m[i][0] * rOffset.x + rMtx.m[i][1] * rOffset.y +
                                         rMtx.m[i][2] * rOffset.z);
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

/**
 * Blends the translation of two matrices linearly.
 * @param pOutMtx Receives the translation.
 * @param rMtxA First matrix.
 * @param rMtxB Second matrix.
 * @param rate Blend rate from the first to the second matrix.
 */
void blendMtxTrans(sead::Matrix34f* pOutMtx, const sead::Matrix34f& rMtxA,
                   const sead::Matrix34f& rMtxB, f32 rate) {
    sead::Vector3f transA = rMtxA.getTranslation();
    sead::Vector3f transB = rMtxB.getTranslation();
    pOutMtx->setTranslation(transA * (1.0f - rate) + transB * rate);
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

/**
 * Calculates the rotation axis of a rotation matrix.
 * @param pOutAxis Receives the rotation axis.
 * @param rMtx Source matrix.
 * @return Whether the axis is zero.
 */
bool calcRotAxisOrZero(sead::Vector3f* pOutAxis, const sead::Matrix34f& rMtx) {
    pOutAxis->set(rMtx.m[2][1] - rMtx.m[1][2], rMtx.m[0][2] - rMtx.m[2][0],
                  rMtx.m[1][0] - rMtx.m[0][1]);
    return !tryNormalizeOrZero(pOutAxis);
}

/**
 * Inverts an orthonormal matrix.
 * @param pOut Receives the result.
 * @param rMtx Source matrix.
 */
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

    pOut->m[0][3] = -pOut->m[0][0] * rMtx.m[0][3] - pOut->m[0][1] * rMtx.m[1][3] -
                    pOut->m[0][2] * rMtx.m[2][3];
    pOut->m[1][3] = -pOut->m[1][0] * rMtx.m[0][3] - pOut->m[1][1] * rMtx.m[1][3] -
                    pOut->m[1][2] * rMtx.m[2][3];
    pOut->m[2][3] = -pOut->m[2][0] * rMtx.m[0][3] - pOut->m[2][1] * rMtx.m[1][3] -
                    pOut->m[2][2] * rMtx.m[2][3];
}

/**
 * Calculates the near and far clip distances from an inverse projection matrix.
 * @param pOutNear Receives the near clip distance.
 * @param pOutFar Receives the far clip distance.
 * @param rInvProj Inverse projection matrix.
 */
void calcNearFarByInvProjection(f32* pOutNear, f32* pOutFar, const sead::Matrix44f& rInvProj) {
    const sead::Vector3f nearPos(0.0f, 0.0f, -1.0f);
    const sead::Vector3f farPos(0.0f, 0.0f, 1.0f);

    f32 nearZ = rInvProj.m[2][0] * nearPos.x + rInvProj.m[2][1] * nearPos.y +
                rInvProj.m[2][2] * nearPos.z + rInvProj.m[2][3];
    f32 nearW = rInvProj.m[3][0] * nearPos.x + rInvProj.m[3][1] * nearPos.y +
                rInvProj.m[3][2] * nearPos.z + rInvProj.m[3][3];
    f32 farZ = rInvProj.m[2][0] * farPos.x + rInvProj.m[2][1] * farPos.y +
               rInvProj.m[2][2] * farPos.z + rInvProj.m[2][3];
    f32 farW = rInvProj.m[3][0] * farPos.x + rInvProj.m[3][1] * farPos.y +
               rInvProj.m[3][2] * farPos.z + rInvProj.m[3][3];

    *pOutNear = -nearZ / nearW;
    *pOutFar = -farZ / farW;
}

/**
 * Moves an inertia tensor away from the center of mass (parallel axis theorem).
 * @param pOut Receives the result.
 * @param rTensor Inertia tensor.
 * @param rPos Offset from the center of mass.
 * @param mass Mass.
 */
void calcMovedInertiaTensor(sead::Matrix33f* pOut, const sead::Matrix33f& rTensor,
                            const sead::Vector3f& rPos, f32 mass) {
    f32 x = rPos.x;
    f32 y = rPos.y;
    f32 z = rPos.z;
    f32 yy = y * y;
    f32 zz = z * z;
    f32 xy = y * x;
    f32 xx = x * x;
    f32 xz = z * x;
    f32 yz = z * y;
    *pOut = rTensor;
    pOut->m[0][0] += (yy + zz) * mass;
    pOut->m[0][1] -= xy * mass;
    pOut->m[0][2] -= xz * mass;
    pOut->m[1][0] -= xy * mass;
    pOut->m[1][1] += (xx + zz) * mass;
    pOut->m[1][2] -= yz * mass;
    pOut->m[2][0] -= xz * mass;
    pOut->m[2][1] -= yz * mass;
    pOut->m[2][2] += (xx + yy) * mass;
}

/**
 * Moves an inertia tensor back to the center of mass (parallel axis theorem).
 * @param pOut Receives the result.
 * @param rTensor Moved inertia tensor.
 * @param rPos Offset from the center of mass.
 * @param mass Mass.
 */
void calcInertiaTensorByMovedTensorAndCenter(sead::Matrix33f* pOut,
                                             const sead::Matrix33f& rTensor,
                                             const sead::Vector3f& rPos, f32 mass) {
    f32 x = rPos.x;
    f32 y = rPos.y;
    f32 z = rPos.z;
    f32 yy = y * y;
    f32 zz = z * z;
    f32 xy = y * x;
    f32 xx = x * x;
    f32 xz = z * x;
    f32 yz = z * y;
    *pOut = rTensor;
    pOut->m[0][0] -= (yy + zz) * mass;
    pOut->m[0][1] += xy * mass;
    pOut->m[0][2] += xz * mass;
    pOut->m[1][0] += xy * mass;
    pOut->m[1][1] -= (xx + zz) * mass;
    pOut->m[1][2] += yz * mass;
    pOut->m[2][0] += xz * mass;
    pOut->m[2][1] += yz * mass;
    pOut->m[2][2] -= (xx + yy) * mass;
}

/**
 * Calculates the inertia tensor of a solid sphere.
 * @param pOutMtx Receives the tensor.
 * @param radius Sphere radius.
 * @param mass Mass.
 */
void calcInertiaTensorSphere(sead::Matrix33f* pOutMtx, f32 radius, f32 mass) {
    f32 inertia = 0.4f * radius * radius * mass;
    pOutMtx->m[0][1] = 0.0f;
    pOutMtx->m[0][2] = 0.0f;
    pOutMtx->m[1][0] = 0.0f;
    pOutMtx->m[1][2] = 0.0f;
    pOutMtx->m[2][0] = 0.0f;
    pOutMtx->m[2][1] = 0.0f;
    pOutMtx->m[0][0] = inertia;
    pOutMtx->m[1][1] = inertia;
    pOutMtx->m[2][2] = inertia;
}

/**
 * Calculates the inertia tensor of a solid box.
 * @param pOutMtx Receives the tensor.
 * @param rSize Size.
 * @param mass Mass.
 */
void calcInertiaTensorBox(sead::Matrix33f* pOutMtx, const sead::Vector3f& rSize, f32 mass) {
    f32 xx = rSize.x * rSize.x;
    f32 yy = rSize.y * rSize.y;
    f32 zz = rSize.z * rSize.z;
    pOutMtx->m[0][1] = 0.0f;
    pOutMtx->m[0][2] = 0.0f;
    pOutMtx->m[1][0] = 0.0f;
    pOutMtx->m[1][2] = 0.0f;
    pOutMtx->m[2][0] = 0.0f;
    pOutMtx->m[2][1] = 0.0f;
    f32 a = (yy + zz) * (1.0f / 12.0f);
    f32 b = (xx + zz) * (1.0f / 12.0f);
    f32 c = (xx + yy) * (1.0f / 12.0f);
    pOutMtx->m[0][0] = a * mass;
    pOutMtx->m[1][1] = b * mass;
    pOutMtx->m[2][2] = c * mass;
}

/**
 * Converts an nn column-major matrix to a sead matrix.
 * @param pOutMtx Receives the matrix.
 * @param rMtx Source matrix.
 */
void makeMtx34f(sead::Matrix34f* pOutMtx, const nn::util::neon::MatrixColumnMajor4x3fType& rMtx) {
    float32x4x3_t m = rMtx._m;
    float32x4_t* out = reinterpret_cast<float32x4_t*>(pOutMtx);
    out[0] = m.val[0];
    out[1] = m.val[1];
    out[2] = m.val[2];
}

/**
 * Converts an nn column-major 4x4 matrix to a sead matrix. Does nothing in this version.
 * @param pOutMtx Receives the matrix.
 * @param rMtx Source matrix.
 */
void makeMtx44f(sead::Matrix44f* pOutMtx, const nn::util::neon::MatrixColumnMajor4x4fType& rMtx) {}

}  // namespace al
