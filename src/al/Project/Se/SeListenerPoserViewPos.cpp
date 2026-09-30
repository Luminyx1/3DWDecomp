#include "Project/Se/SeListenerPoserViewPos.hpp"

namespace al {

SeListenerPoserViewPos::SeListenerPoserViewPos(const sead::SafeString& rName, const sead::SafeString& rGroupName)
    : SeListenerPoser(rName, rGroupName) {}

void SeListenerPoserViewPos::calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos,
                                              const ISeListenerParam& rParam) {
    *pMtx = rParam.getViewMatrix();
    *pPos = rParam.getViewPos();
}

}  // namespace al
