#include "Project/Se/SeListenerPoserViewPosOffset.hpp"

namespace al {

SeListenerPoserViewPosOffset::SeListenerPoserViewPosOffset(const sead::SafeString& rName,
                                                           const sead::SafeString& rGroupName,
                                                           const sead::Vector3f& rOffset)
    : SeListenerPoser(rName, rGroupName), mOffset(rOffset) {}

void SeListenerPoserViewPosOffset::calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos,
                                                    const ISeListenerParam& rParam) {
    const sead::Matrix34f& viewMtx = rParam.getViewMatrix();
    sead::Vector3f row0(viewMtx.m[0][0], viewMtx.m[0][1], viewMtx.m[0][2]);
    sead::Vector3f row1(viewMtx.m[1][0], viewMtx.m[1][1], viewMtx.m[1][2]);
    sead::Vector3f row2(viewMtx.m[2][0], viewMtx.m[2][1], viewMtx.m[2][2]);
    f32 x = mOffset.x + row0.dot(rParam.getViewPos());
    f32 y = mOffset.y + row1.dot(rParam.getViewPos());
    f32 z = mOffset.z + row2.dot(rParam.getViewPos());
    pPos->set(row0.x * x + row1.x * y + row2.x * z, row0.y * x + row1.y * y + row2.y * z,
              row0.z * x + row1.z * y + row2.z * z);
    *pMtx = viewMtx;
    pMtx->m[0][3] = -x;
    pMtx->m[1][3] = -y;
    pMtx->m[2][3] = -z;
}

}  // namespace al
