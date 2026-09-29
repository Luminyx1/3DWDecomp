#include "Project/Se/ISeListenerParam.hpp"
#include "Project/Se/SeListenerPoser.hpp"

namespace al {
/**
 * @brief Constructs a listener poser that places the listener at the camera plus a view-space offset.
 * @param rName The name of the poser.
 * @param rUnused Unused second name.
 * @param rOffset The listener offset, expressed in view space.
 */
SeListenerPoserViewPosOffset::SeListenerPoserViewPosOffset(const sead::SafeString& rName,
                                                           const sead::SafeString& rUnused,
                                                           const sead::Vector3f& rOffset)
    : SeListenerPoser(rName, rUnused), mOffset(rOffset) {}

/**
 * @brief Computes the listener pose from the camera, shifted by the view-space offset.
 * @param pMtx Output listener matrix: the view rotation with the offset translation.
 * @param pPos Output listener position in world space.
 * @param rParam The listener parameters providing the camera state.
 */
void SeListenerPoserViewPosOffset::calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos,
                                                    const ISeListenerParam& rParam) {
    const sead::Matrix34f& viewMtx = rParam.getViewMatrix();
    sead::Vector3f side(viewMtx(0, 0), viewMtx(0, 1), viewMtx(0, 2));
    sead::Vector3f up(viewMtx(1, 0), viewMtx(1, 1), viewMtx(1, 2));
    sead::Vector3f front(viewMtx(2, 0), viewMtx(2, 1), viewMtx(2, 2));

    f32 x = side.dot(rParam.getViewPos()) + mOffset.x;
    f32 y = up.dot(rParam.getViewPos()) + mOffset.y;
    f32 z = front.dot(rParam.getViewPos()) + mOffset.z;

    *pPos = side * x + up * y + front * z;

    *pMtx = viewMtx;
    (*pMtx)(0, 3) = -x;
    (*pMtx)(1, 3) = -y;
    (*pMtx)(2, 3) = -z;
}
}  // namespace al
