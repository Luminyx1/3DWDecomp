#include "Library/Math/MathUtil.hpp"
#include "Project/Se/ISeListenerParam.hpp"
#include "Project/Se/SeListenerPoser.hpp"

namespace al {
/**
 * @brief Constructs a named listener poser.
 * @param rName The name of the poser.
 * @param rUnused Unused second name.
 */
SeListenerPoser::SeListenerPoser(const sead::SafeString& rName, const sead::SafeString& rUnused) : mName(rName) {}

/**
 * @brief Builds a view matrix looking from a position towards a target.
 * @param pMtx Output view matrix.
 * @param rPos The eye position.
 * @param rTarget The position looked at.
 * @param rUp The approximate up direction.
 * @return Whether the matrix could be built (false when the directions are degenerate).
 */
bool SeListenerPoser::tryCalcViewMatrix(sead::Matrix34f* pMtx, const sead::Vector3f& rPos,
                                        const sead::Vector3f& rTarget, const sead::Vector3f& rUp) {
    sead::Vector3f back = rPos;
    back -= rTarget;
    if (isNearZero(back, 0.001f)) {
        return false;
    }
    normalize(&back);

    sead::Vector3f side;
    side.setCross(rUp, back);
    if (isNearZero(side, 0.001f)) {
        return false;
    }
    normalize(&side);

    sead::Vector3f up;
    up.setCross(back, side);

    f32 tx = -side.dot(rPos);
    f32 ty = -up.dot(rPos);
    f32 tz = -back.dot(rPos);

    pMtx->m[0][0] = side.x;
    pMtx->m[0][1] = side.y;
    pMtx->m[0][2] = side.z;
    pMtx->m[0][3] = tx;
    pMtx->m[1][0] = up.x;
    pMtx->m[1][1] = up.y;
    pMtx->m[1][2] = up.z;
    pMtx->m[1][3] = ty;
    pMtx->m[2][0] = back.x;
    pMtx->m[2][1] = back.y;
    pMtx->m[2][2] = back.z;
    pMtx->m[2][3] = tz;
    return true;
}

/**
 * @brief Computes the view-space direction the camera looks in.
 * @param pDir Output normalized look-at direction.
 */
void ISeListenerParam::calcLookAtDirNormFromViewMatrix(sead::Vector3f* pDir) const {
    const sead::Matrix34f& viewMtx = getViewMatrix();
    pDir->set(-viewMtx(2, 0), -viewMtx(2, 1), -viewMtx(2, 2));
}

/**
 * @brief Computes the camera's up direction.
 * @param pDir Output normalized up direction.
 */
void ISeListenerParam::calcUpDirNormFromViewMatrix(sead::Vector3f* pDir) const {
    const sead::Matrix34f& viewMtx = getViewMatrix();
    pDir->set(viewMtx(1, 0), viewMtx(1, 1), viewMtx(1, 2));
}
}  // namespace al
