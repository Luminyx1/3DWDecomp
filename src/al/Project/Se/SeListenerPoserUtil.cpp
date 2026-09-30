#include "Project/Se/SeListenerPoser.hpp"

#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * Constructs a named listener poser.
 * @param rName Poser name.
 * @param rGroupName Group name (unused).
 */
SeListenerPoser::SeListenerPoser(const sead::SafeString& rName, const sead::SafeString& rGroupName)
    : mName(rName) {}

/**
 * Builds a view matrix looking from a position towards a target.
 * @param pMtx Output view matrix.
 * @param rPos Eye position.
 * @param rAt Position looked at.
 * @param rUp Approximate up direction.
 * @return Whether the matrix could be built (false when the directions are degenerate).
 */
bool SeListenerPoser::tryCalcViewMatrix(sead::Matrix34f* pMtx, const sead::Vector3f& rPos,
                                        const sead::Vector3f& rAt, const sead::Vector3f& rUp) {
    sead::Vector3f dir = rPos;
    dir -= rAt;

    if (isNearZero(dir, 0.001f)) {
        return false;
    }

    normalize(&dir);

    sead::Vector3f side;
    side.setCross(rUp, dir);

    if (isNearZero(side, 0.001f)) {
        return false;
    }

    normalize(&side);

    sead::Vector3f up;
    up.setCross(dir, side);

    f32 tx = -side.dot(rPos);
    f32 ty = -up.dot(rPos);
    f32 tz = -dir.dot(rPos);

    pMtx->m[0][0] = side.x;
    pMtx->m[0][1] = side.y;
    pMtx->m[0][2] = side.z;
    pMtx->m[0][3] = tx;
    pMtx->m[1][0] = up.x;
    pMtx->m[1][1] = up.y;
    pMtx->m[1][2] = up.z;
    pMtx->m[1][3] = ty;
    pMtx->m[2][0] = dir.x;
    pMtx->m[2][1] = dir.y;
    pMtx->m[2][2] = dir.z;
    pMtx->m[2][3] = tz;
    return true;
}

/**
 * Computes the direction the camera looks in from the view matrix.
 * @param pDir Output normalized look-at direction.
 */
void ISeListenerParam::calcLookAtDirNormFromViewMatrix(sead::Vector3f* pDir) const {
    const sead::Matrix34f& viewMtx = getViewMatrix();
    pDir->set(-viewMtx.m[2][0], -viewMtx.m[2][1], -viewMtx.m[2][2]);
}

/**
 * Computes the camera's up direction from the view matrix.
 * @param pDir Output normalized up direction.
 */
void ISeListenerParam::calcUpDirNormFromViewMatrix(sead::Vector3f* pDir) const {
    const sead::Matrix34f& viewMtx = getViewMatrix();
    pDir->set(viewMtx.m[1][0], viewMtx.m[1][1], viewMtx.m[1][2]);
}

}  // namespace al
