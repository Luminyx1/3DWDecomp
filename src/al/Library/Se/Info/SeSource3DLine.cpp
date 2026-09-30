#include "Library/Se/Info/SeSource.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Constructs a line source.
 * @param pPose Source pose.
 * @param pLine Line vector in the pose space.
 * @param pInfo Audio system information.
 */
SeSource3DLine::SeSource3DLine(SeSourcePose3DMtxBase* pPose, const sead::Vector3f* pLine, AudioSystemInfo* pInfo)
    : SeSource3D("３Ｄ線音源", pPose, pInfo), mMtxPose(pPose), mLine(pLine) {}

/**
 * Calculates the line length.
 */
void SeSource3DLine::calcPositionInitialize() {
    mLength = mLine->length();
}

/**
 * Updates the pose, the line direction and the line end position.
 */
void SeSource3DLine::calcPositionDynamic() {
    mMtxPose->update();
    mDir.setRotated(mMtxPose->get3DMtx(), *mLine);
    mEndPos.setAdd(mDir, mMtxPose->get3DPos());
    normalize(&mDir);
}

/**
 * Calculates the source position nearest to the listener.
 * @param rListenerPos Listener position.
 * @return Source position.
 */
const sead::Vector3f* SeSource3DLine::calcPosition(const sead::Vector3f& rListenerPos) {
    f32 dist = (rListenerPos - mMtxPose->get3DPos()).dot(mDir);
    if (dist <= 0.0f) {
        mPos = mMtxPose->get3DPos();
    } else if (dist >= mLength) {
        mPos = mEndPos;
    } else {
        mPos.setScaleAdd(dist, mDir, mMtxPose->get3DPos());
    }
    return &mPos;
}
}  // namespace al
