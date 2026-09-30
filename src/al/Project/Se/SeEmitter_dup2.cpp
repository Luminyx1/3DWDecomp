#include "Project/Se/SeListenerPoserMiddlePos.hpp"

#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * Constructs a listener poser placed between the camera and the point it looks at.
 * @param rName Poser name.
 * @param rGroupName Group name.
 * @param baseToMiddleRatio Position along the camera-to-target distance (0 = camera).
 */
SeListenerPoserMiddlePos::SeListenerPoserMiddlePos(const sead::SafeString& rName,
                                                   const sead::SafeString& rGroupName, f32 baseToMiddleRatio)
    : SeListenerPoser(rName, rGroupName), mBaseToMiddleRatio(baseToMiddleRatio) {}

/**
 * Sets where between the camera and its target the listener sits.
 * @param ratio Ratio of the camera-to-target distance (0 = camera).
 */
void SeListenerPoserMiddlePos::setBaseToMiddleRatio(f32 ratio) {
    mBaseToMiddleRatio = ratio;
}

/**
 * Computes a listener pose between the camera and its target, falling back to the camera pose.
 * @param pMtx Output listener view matrix.
 * @param pPos Output listener position.
 * @param rParam Listener parameters providing the camera state.
 */
void SeListenerPoserMiddlePos::calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos,
                                                const ISeListenerParam& rParam) {
    sead::Vector3f lookAtDir;
    rParam.calcLookAtDirNormFromViewMatrix(&lookAtDir);
    sead::Vector3f upDir;
    rParam.calcUpDirNormFromViewMatrix(&upDir);

    const sead::Vector3f& targetPos = rParam.getTargetPos();
    f32 dist = (targetPos - rParam.getViewPos()).dot(lookAtDir);
    if (isNearZero(dist, 0.001f)) {
        *pMtx = rParam.getViewMatrix();
        *pPos = rParam.getViewPos();
        return;
    }

    sead::Vector3f targetOnView = lookAtDir;
    targetOnView *= dist;
    targetOnView += rParam.getViewPos();

    sead::Vector3f middlePos = lookAtDir;
    middlePos *= dist * mBaseToMiddleRatio;
    middlePos += rParam.getViewPos();

    if (tryCalcViewMatrix(pMtx, middlePos, targetOnView, upDir)) {
        *pPos = middlePos;
    } else {
        *pMtx = rParam.getViewMatrix();
        *pPos = rParam.getViewPos();
    }
}

}  // namespace al
