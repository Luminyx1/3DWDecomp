#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"
#include "NPC/NpcTargetFinder.hpp"

namespace al {
class ByamlIter;
class HitSensor;
class Nerve;
class PlacementInfo;
class SensorMsg;
}  // namespace al

class IUseNekoModeActor;
class NpcHeadController;

namespace neko {

/**
 * @brief A spot a cat can be brought to (a cat parent's drop target).
 */
class Target {
public:
    Target(const al::PlacementInfo& rInfo, s32 parentId, s32 index, const al::LiveActor* pHost);

    /** @brief Construct an unused target. */
    Target() { mTrans = sead::Vector3f::zero; }

    const sead::Vector3f* tryGetHostTrans() const;

    sead::Vector3f mTrans;
    s32 mIndex = -1;
    bool mIsActive = true;
    s32 mParentId = 0;
    s32 mId = 0;
    f32 mRange = 0.0f;
    const al::LiveActor* mHost = nullptr;
    s32 _28 = 7;
};

static_assert(sizeof(Target) == 0x30);

NpcHeadController* makeHeadController(IUseNekoModeActor* pActor, al::ByamlIter iter,
                                      NpcTargetFinder* pTargetFinder,
                                      npc::NpcFindTargetType targetType);
bool trySetNerve(al::IUseNerve* pUser, const al::Nerve* pNerve);
bool isSensorEnemyReactAttack(const al::HitSensor* pSensor);
bool isMsgNpcAttackerHitReaction(const IUseNekoModeActor* pActor, const al::SensorMsg* pMsg,
                                 const al::HitSensor* pOther, const al::HitSensor* pSelf);
bool isMsgHitReaction(const IUseNekoModeActor* pActor, const al::SensorMsg* pMsg,
                      const al::HitSensor* pOther, const al::HitSensor* pSelf);
bool isMsgMeraWanwanTrackAttack(const IUseNekoModeActor* pActor, const al::SensorMsg* pMsg,
                                const al::HitSensor* pOther, const al::HitSensor* pSelf);
void setAnimationRate(IUseNekoModeActor* pActor);
bool isActive(const IUseNekoModeActor* pActor, f32 range);
bool isInChaseRange(const IUseNekoModeActor* pActor, const al::LiveActor* pTarget);
bool isInChaseRange(const IUseNekoModeActor* pActor, const sead::Vector3f& rPos);
bool isInRange(const al::LiveActor* pActor, const sead::Vector3f& rCenter, f32 range);
bool checkGround(const al::LiveActor* pActor);

}  // namespace neko

/**
 * @brief Host actor of a cat; its behavior is implemented by IUseNekoModeActor modes.
 * @note Only what reconstructed code needs is declared so far.
 */
class Neko : public al::LiveActor {
public:
    /**
     * @brief Behavior modes of a cat.
     * @note Only the values used by reconstructed code are named.
     */
    enum Mode : s32 {
        Mode_Parent = 0,
    };

    Neko(const char* pName, Neko* pParent = nullptr);

    void init(const al::ActorInitInfo& rInfo) override;

    void initAsModelName(const al::ActorInitInfo& rInfo, const char* pModelName);
    void startAppearNormal();
    bool tryStartClipped();
    void tryEndClipped();
    bool isMode(Mode mode) const;
    bool tryStartHide();
    void startKill();
    void startSeekTarget(const neko::Target* pTarget, bool isForce);
    void setActivePosition(const sead::Vector3f& rTrans);

    s32 getUID() const { return mUID; }

    IUseNekoModeActor* getNormalModeActor() const { return mNormalModeActor; }

    IUseNekoModeActor* getModeActor() const { return mModeActor; }

    Neko* getRideNeko() const { return mRideNeko; }

    void setRideNeko(Neko* pNeko) { mRideNeko = pNeko; }

private:
    u8 _144[0x150 - 0x144];
    Neko* mRideNeko;  // 0x150
    s32 mUID;  // 0x158
    IUseNekoModeActor* mNormalModeActor;  // 0x160
    u8 _168[0x170 - 0x168];
    IUseNekoModeActor* mModeActor;  // 0x170
    u8 _178[0x1a0 - 0x178];
};

static_assert(sizeof(Neko) == 0x1a0);
