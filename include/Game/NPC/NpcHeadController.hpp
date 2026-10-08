#pragma once

#include <math/seadVector.h>

/**
 * @brief Turns the head of an NPC towards a look-at target.
 * @note Only what reconstructed code needs is declared so far.
 */
class NpcHeadController {
public:
    void update();

    /**
     * @brief Set the position the head looks at.
     * @param pTarget Position to look at, or nullptr to stop looking; must stay valid.
     */
    void setLookAtTarget(const sead::Vector3f* pTarget) { mLookAtTarget = pTarget; }

private:
    u8 _0[0x30];
    const sead::Vector3f* mLookAtTarget;  // 0x30
};
