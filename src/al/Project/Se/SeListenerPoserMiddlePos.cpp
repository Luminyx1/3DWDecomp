#include "Library/Math/MathUtil.hpp"
#include "Project/Se/ISeListenerParam.hpp"
#include "Project/Se/SeListenerPoser.hpp"

namespace al {
/**
 * @brief Constructs a listener poser placed between the camera and the point it looks at.
 * @param rName The name of the poser.
 * @param rUnused Unused second name.
 * @param baseToMiddleRatio How far along the camera-to-target distance the listener sits (0 = camera).
 */
SeListenerPoserMiddlePos::SeListenerPoserMiddlePos(const sead::SafeString& rName, const sead::SafeString& rUnused,
                                                   f32 baseToMiddleRatio)
    : SeListenerPoser(rName, rUnused), mBaseToMiddleRatio(baseToMiddleRatio) {}

/**
 * @brief Sets where between the camera and its target the listener sits.
 * @param ratio The new ratio of the camera-to-target distance (0 = camera).
 */
void SeListenerPoserMiddlePos::setBaseToMiddleRatio(f32 ratio) {
    mBaseToMiddleRatio = ratio;
}

/**
 * @brief Computes a listener pose between the camera and its target, falling back to the camera pose.
 * @param pMtx Output listener view matrix.
 * @param pPos Output listener position.
 * @param rParam The listener parameters providing the camera state.
 */
void SeListenerPoserMiddlePos::calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos,
                                                const ISeListenerParam& rParam) {
    sead::Vector3f lookAtDir;
    rParam.calcLookAtDirNormFromViewMatrix(&lookAtDir);
    sead::Vector3f upDir;
    rParam.calcUpDirNormFromViewMatrix(&upDir);

    const sead::Vector3f& targetPos = rParam.getTargetPos();
    f32 distance = (targetPos - rParam.getViewPos()).dot(lookAtDir);

    if (isNearZero(distance, 0.001f)) {
        *pMtx = rParam.getViewMatrix();
        *pPos = rParam.getViewPos();
        return;
    }

    sead::Vector3f lookAtPos = lookAtDir;
    lookAtPos *= distance;
    lookAtPos += rParam.getViewPos();

    sead::Vector3f middlePos = lookAtDir;
    middlePos *= distance * mBaseToMiddleRatio;
    middlePos += rParam.getViewPos();

    if (tryCalcViewMatrix(pMtx, middlePos, lookAtPos, upDir)) {
        *pPos = middlePos;
    } else {
        *pMtx = rParam.getViewMatrix();
        *pPos = rParam.getViewPos();
    }
}
}  // namespace al
