#include "Project/Se/ISeListenerParam.hpp"
#include "Project/Se/SeListenerPoser.hpp"

namespace al {
/**
 * @brief Constructs a listener poser whose backward offset follows the camera's field of view.
 * @param rName The name of the poser.
 * @param rUnused Unused second name.
 */
SeListenerPoserViewPosOffsetFovy::SeListenerPoserViewPosOffsetFovy(const sead::SafeString& rName,
                                                                   const sead::SafeString& rUnused)
    : SeListenerPoserViewPosOffset(rName, rUnused, sead::Vector3f(0.0f, 0.0f, 0.0f)) {}

/**
 * @brief Computes the listener pose, pulling the listener back further as the field of view narrows.
 * @param pMtx Output listener matrix.
 * @param pPos Output listener position in world space.
 * @param rParam The listener parameters providing the camera state.
 */
void SeListenerPoserViewPosOffsetFovy::calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos,
                                                        const ISeListenerParam& rParam) {
    f32 fovy = rParam.getFovyDegree();
    fovy = fovy < 3.0f ? 3.0f : fovy;
    fovy = fovy > 45.0f ? 45.0f : fovy;
    f32 rate = (fovy - 45.0f) / (3.0f - 45.0f);
    mOffset.z = -(0.0f + rate * 2300.0f);
    SeListenerPoserViewPosOffset::calcListenerPose(pMtx, pPos, rParam);
}
}  // namespace al
