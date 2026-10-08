#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"

class PlayerModelIK;
class PlayerModelHair;

/// The model actor of one of the player's figures.
class PlayerModel : public al::LiveActor {
public:
    void initJointController(const sead::Vector2f& rMashLimit, const bool* pIsMash,
                             const sead::Quatf* pQuat, f32 rate);
    void setInvincibleColor(const sead::Color4f& rColor);
    void hideFur();
    void showFur();
    void setShadowLength(f32 length);
    void validateSkirtDynamics();
    void invalidateSkirtDynamics();
    bool isExistSkirtDynamics() const;
    bool isValidSkirtDynamics() const;
    void resetSkirtDynamics();
    void validateTailDynamics();
    void invalidateTailDynamics();
    bool isExistTailDynamics() const;
    bool isValidTailDynamics() const;
    void resetTailDynamics();
    void validateHairDynamics();
    void invalidateHairDynamics();
    bool isExistHairDynamics() const;
    bool isValidHairDynamics() const;
    void resetHairDynamics();

    /** @brief Gets the index used to pick this model's retargetting info. @return Index. */
    s32 getRetargettingIndex() const { return mRetargettingIndex; }

    /** @brief Gets the name of this model's animation set (e.g. "Climb"). @return Name. */
    const char* getAnimSetName() const { return mAnimSetName; }

    /** @brief Gets the foot IK (created by createIK). @return IK, or nullptr. */
    PlayerModelIK* getIK() const { return mIK; }

    /** @brief Gets the hair controller (created by createHairCtrl). @return Controller, or nullptr. */
    PlayerModelHair* getHairCtrl() const { return mHairCtrl; }

private:
    s32 mRetargettingIndex;  // 0x144
    u8 _148[0x158 - 0x148];
    const char* mAnimSetName;  // 0x158
    u8 _160[0x190 - 0x160];
    PlayerModelIK* mIK;           // 0x190
    PlayerModelHair* mHairCtrl;   // 0x198
};
