#pragma once

#include "Demo/DemoObjBase.hpp"

namespace alSeFunction {
enum DemoType : s32;
}  // namespace alSeFunction

/** @brief Cutscene demo actor placed in a stage. */
class DemoCutscene : public DemoObjBase {
public:
    DemoCutscene(const char* pName, alSeFunction::DemoType type);

    void cancelSE();
    void setEndAtCutscenePos(const sead::Vector3f& rTrans, const sead::Vector3f& rFront);
    bool isWipeCloseEnd() const;
    bool isFadedOut();

    /** @brief Keep the player where it stands when the cutscene ends. */
    void setKeepPlayerPos() { mIsKeepPlayerPos = true; }

    /** @brief Hide the player during the cutscene. */
    void setHidePlayer() { mIsHidePlayer = true; }

    /** @brief Skip the wipe at the end of the cutscene. */
    void setSkipEndWipe() { mIsSkipEndWipe = true; }

    /** @brief Place the cutscene at the matrix given with overrideBaseMtx(). */
    void setUseBaseMtx() { mIsUseBaseMtx = true; }

    /**
     * @brief Marks the cutscene as directly followed by another one.
     * @param isFollowed Whether another cutscene plays right after this one.
     */
    void setFollowedByDemo(bool isFollowed) { mIsFollowedByDemo = isFollowed; }

private:
    u8 _300[0x304 - 0x300];
    bool mIsKeepPlayerPos;  // 0x304
    u8 _305;
    bool mIsHidePlayer;  // 0x306
    bool mIsSkipEndWipe;  // 0x307
    bool mIsUseBaseMtx;  // 0x308
    u8 _309[0x32c - 0x309];
    bool mIsFollowedByDemo;  // 0x32C
    u8 _32d[0x368 - 0x32d];
};
static_assert(sizeof(DemoCutscene) == 0x368);
