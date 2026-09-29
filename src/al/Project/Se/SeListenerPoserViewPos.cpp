#include "Project/Se/ISeListenerParam.hpp"
#include "Project/Se/SeListenerPoser.hpp"

namespace al {
/**
 * @brief Constructs a listener poser that places the listener exactly at the camera.
 * @param rName The name of the poser.
 * @param rUnused Unused second name.
 */
SeListenerPoserViewPos::SeListenerPoserViewPos(const sead::SafeString& rName, const sead::SafeString& rUnused)
    : SeListenerPoser(rName, rUnused) {}

/**
 * @brief Computes the listener pose from the camera view matrix and position.
 * @param pMtx Output listener matrix, set to the view matrix.
 * @param pPos Output listener position, set to the view position.
 * @param rParam The listener parameters providing the camera state.
 */
void SeListenerPoserViewPos::calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos,
                                              const ISeListenerParam& rParam) {
    *pMtx = rParam.getViewMatrix();
    *pPos = rParam.getViewPos();
}
}  // namespace al
