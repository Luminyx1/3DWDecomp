#include "Library/Joint/JointControllerBase.hpp"

namespace al {

/**
 * Constructs a controller with an empty joint id list.
 */
JointControllerBase::JointControllerBase() = default;

/**
 * Adds a joint id and keeps the id list sorted in ascending order.
 * @param jointId Joint index to control.
 */
void JointControllerBase::appendJointId(s32 jointId) {
    if (mJointIds.isFull()) {
        return;
    }

    mJointIds.emplaceBack(jointId);

    if (mJointIds.size() < 2) {
        return;
    }

    mJointIds.shakerSort_<s32>([](const s32* pA, const s32* pB) -> s32 {
        if (*pA < *pB) {
            return -1;
        }

        if (*pA > *pB) {
            return 1;
        }

        return 0;
    });
}

/**
 * Finds the first controlled joint id greater than a given id.
 * @param pId Receives the found id.
 * @param current Id to search after.
 * @return Whether a greater id exists.
 */
bool JointControllerBase::findNextId(s32* pId, s32 current) const {
    for (s32 i = 0; i < mJointIds.size(); i++) {
        if (*mJointIds[i] > current) {
            *pId = *mJointIds[i];
            return true;
        }
    }

    return false;
}

/**
 * Checks whether a joint id is controlled.
 * @param jointId Joint index to look for.
 * @return Whether the id is in the list.
 */
bool JointControllerBase::isExistId(s32 jointId) const {
    for (s32 i = 0; i < mJointIds.size(); i++) {
        if (*mJointIds(i) == jointId) {
            return true;
        }
    }

    return false;
}

}  // namespace al
