#pragma once
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Scene/ISceneObj.hpp"

namespace al {
class HitSensor;
class WipeSimple;
}  // namespace al
class DemoCutscene;
class DemoObjBase;
class GigaBell;

class GigaBellManager : public al::LiveActor, public al::ISceneObj {
public:
    explicit GigaBellManager(const char*);
    static GigaBellManager* tryGetManager(const al::IUseSceneObjHolder* pHolder);
    bool tryQueueReturnCutscene(bool isQueue, bool, bool);
    bool isCutsceneDone();
    GigaBell* getGigaBellClosestTo(sead::Vector3f);
    s32 getGigaBellCount();
    GigaBell* getGigaBell(s32 index);
    bool shouldPlayUnlockCutscene();
    bool canHitBellPlessieChase();
    bool tryHitNextBellPlessieChase(al::HitSensor* pOther, al::HitSensor* pSelf);
    void setLastHitBell(GigaBell* pBell);
    void hitBellPlessieChase();
    void gigaBellCollected(GigaBell* pBell);
    DemoCutscene* getNextExplainationCutscene(s32 layerId, s32* pCutsceneId);
    al::WipeSimple* getWipeFadeBlack();
    void endCutscene(bool isSkip);
    void endReturnCutscene();
    s32 getPlessieChaseHitCount();
    sead::Vector3f getPlessieChaseBellBaseOffset();
    sead::Vector3f calcMoveOffset(s32 index);

    /** @brief Number of goal items needed to unlock the bells. @return The count. */
    s32 getGoalItemsRequired() const { return mGoalItemsRequired; }

    /** @brief Remember the goal item count shown by the last unlock cutscene. */
    void updateUnlockedGoalItems() { mUnlockedGoalItems = mGoalItemsRequired; }

    /** @brief Demo played when a bell is collected. @return The demo, or nullptr. */
    DemoObjBase* getCollectDemo() const { return mCollectDemo; }

    /** @brief Whether the camera should return to the stored pose. @return The flag. */
    bool isCameraReturn() const { return mIsCameraReturn; }

    /** @brief Clear the camera return request. */
    void clearCameraReturn() { mIsCameraReturn = false; }

private:
    s32 mGoalItemsRequired;  // 0x150
    u8 _154[0x160 - 0x154];
    s32 mUnlockedGoalItems;  // 0x160
    u8 _164[0x170 - 0x164];
    DemoObjBase* mCollectDemo;  // 0x170
    u8 _178[0x190 - 0x178];
    bool mIsCameraReturn;  // 0x190
    u8 _191[0x358 - 0x191];
};
static_assert(sizeof(GigaBellManager) == 0x358);
