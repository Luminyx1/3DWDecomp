#pragma once

#include "Library/LiveActor/LiveActor.hpp"
#include <math/seadMatrix.h>
#include <math/seadVector.h>

/**
 * @brief Item carried by an enemy that follows a host position until it is released.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class EnemyAttachItem : public al::LiveActor {
public:
    EnemyAttachItem(al::LiveActor* pHost, const char* pName, const char* pItemName, s32 itemType,
                    const sead::Vector3f* pFollowTrans, const sead::Matrix34f* pFollowMtx,
                    const sead::Vector3f& rOffset, bool isBubble);

    void setFollowTransPtr(const sead::Vector3f* pTrans);
    void endAttach(bool isPopUp);

private:
    u8 _144[0x44];
};

static_assert(sizeof(EnemyAttachItem) == 0x188);
