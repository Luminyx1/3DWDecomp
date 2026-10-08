#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}

class PlayerModelWorldMtxCallbackHolder;

/// Foot IK of a player model.
class PlayerModelIK {
public:
    PlayerModelIK(al::LiveActor* pActor, PlayerModelWorldMtxCallbackHolder* pCallbackHolder);

    /** @brief Enables the IK (if it isn't already). */
    void validate() {
        if (!mIsValid) {
            mIsValid = true;
        }
    }

    /** @brief Disables the IK (if it isn't already). */
    void invalidate() {
        if (mIsValid) {
            mIsValid = false;
        }
    }

    /** @brief Checks if the IK is enabled. @return True if enabled. */
    bool isValid() const { return mIsValid; }

private:
    al::LiveActor* mActor;  // 0x0
    bool mIsValid;          // 0x8
};
