#include "Library/Movement/PoseHistoryPath.hpp"

#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * Constructs an identity pose entry.
 */
PoseHistoryPath::PoseInfo::PoseInfo()
    : quat(sead::Quatf::unit), trans(sead::Vector3f::zero), name(""), distance(0.0f),
      isKeep(true) {}

/**
 * Constructs a pose entry.
 * @param rQuat Rotation.
 * @param rTrans Position.
 * @param pName Name tag of the entry.
 * @param distance Distance to the next entry.
 */
PoseHistoryPath::PoseInfo::PoseInfo(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
                                    const char* pName, f32 distance)
    : quat(rQuat), trans(rTrans), name(pName), distance(distance), isKeep(true) {}

/**
 * Constructs a history with room for a number of poses.
 * @param maxHistory Maximum number of poses.
 */
PoseHistoryPath::PoseHistoryPath(s32 maxHistory) {
    mHistory.allocBuffer(maxHistory, nullptr);
}

/**
 * Removes every pose.
 */
void PoseHistoryPath::clear() {
    mHistory.clear();
}

/**
 * Adds a pose without a name tag.
 * @param rQuat Rotation.
 * @param rTrans Position.
 * @param minDistance Distance from the previous pose below which that pose may be replaced.
 */
void PoseHistoryPath::addHistory(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
                                 f32 minDistance) {
    addHistory(rQuat, rTrans, "", minDistance);
}

/**
 * Adds a pose. The previous pose is replaced if it was closer than the minimum distance to the
 * pose before it.
 * @param rQuat Rotation.
 * @param rTrans Position.
 * @param pName Name tag of the entry.
 * @param minDistance Distance from the previous pose below which that pose may be replaced.
 */
void PoseHistoryPath::addHistory(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
                                 const char* pName, f32 minDistance) {
    PoseInfo info(rQuat, rTrans, pName, 0.0f);

    if (!mHistory.empty()) {
        if (!mHistory.back().isKeep) {
            mHistory.popBack();
        }

        f32 distance = (mHistory.back().trans - info.trans).length();

        if (distance < minDistance) {
            info.isKeep = false;
        }

        mHistory.back().distance = distance;
    }

    mHistory.forcePushBack(info);
}

/**
 * Calculates the pose at a distance back along the history.
 * @param pQuat Receives the rotation.
 * @param pTrans Receives the position.
 * @param distance Distance from the newest pose.
 */
void PoseHistoryPath::calcPoseAndTrans(sead::Quatf* pQuat, sead::Vector3f* pTrans,
                                       f32 distance) const {
    const char* name = nullptr;
    calcPoseAndTrans(pQuat, pTrans, &name, distance);
}

/**
 * Calculates the pose at a distance back along the history, interpolating between entries.
 * @param pQuat Receives the rotation.
 * @param pTrans Receives the position.
 * @param pName Receives the name tag of the entry the pose lies on.
 * @param distance Distance from the newest pose.
 */
void PoseHistoryPath::calcPoseAndTrans(sead::Quatf* pQuat, sead::Vector3f* pTrans,
                                       const char** pName, f32 distance) const {
    s32 size = mHistory.size();

    if (size < 1) {
        return;
    }

    if (size == 1) {
        const PoseInfo& info = mHistory(0);
        pQuat->set(info.quat);
        pTrans->set(info.trans);
        *pName = info.name;
        return;
    }

    const PoseInfo* info = nullptr;
    const PoseInfo* nextInfo = nullptr;
    f32 rate = 0.0f;
    f32 sum = 0.0f;

    for (s32 i = 0; i < size; i++) {
        const PoseInfo& current = mHistory(size - 1 - i);

        if (sum + current.distance > distance) {
            info = &current;

            if (i < size - 1) {
                nextInfo = mHistory.get(size - 2 - i);
                rate = (distance - sum) / current.distance;
            }

            break;
        }

        sum += current.distance;
    }

    if (info == nullptr) {
        info = mHistory.get(0);
    }

    if (nextInfo != nullptr) {
        sead::Quatf quat = info->quat;
        sead::Vector3f trans = info->trans;
        sead::Quatf nextQuat = nextInfo->quat;
        sead::Vector3f nextTrans = nextInfo->trans;
        slerpQuat(pQuat, quat, nextQuat, rate);
        lerpVec(pTrans, trans, nextTrans, rate);
        *pName = info->name;
    } else {
        pQuat->set(info->quat);
        pTrans->set(info->trans);
        *pName = info->name;
    }
}

}  // namespace al
