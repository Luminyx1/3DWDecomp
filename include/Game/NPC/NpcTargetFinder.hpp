#pragma once

#include <basis/seadTypes.h>

namespace al {
class HitSensor;
class LiveActor;
}  // namespace al

class IUseTargetFinderFilter;

namespace npc {

/**
 * @brief Kinds of targets an NPC target finder can search for (bit flags).
 */
enum NpcFindTargetType : s32 {
    NpcFindTargetType_None = 0,
    NpcFindTargetType_PlayerRegular = 1 << 0,
    NpcFindTargetType_PlayerClimb = 1 << 1,
    NpcFindTargetType_KoopaJr = 1 << 2,
    NpcFindTargetType_Cursor = 1 << 3,
    NpcFindTargetType_Ball = 1 << 4,
    NpcFindTargetType_Koura = 1 << 5,
    NpcFindTargetType_Bird = 1 << 6,
    NpcFindTargetType_Neko = 1 << 7,
    NpcFindTargetType_Player = NpcFindTargetType_PlayerRegular | NpcFindTargetType_PlayerClimb,
    NpcFindTargetType_All = -1,
};

}  // namespace npc

/**
 * @brief Sight and chase ranges used by an NPC target finder.
 * @note The fields have not been reconstructed yet.
 */
class NpcTargetFinderParam {
public:
    NpcTargetFinderParam();
    NpcTargetFinderParam(f32 sightRange, f32 _4, f32 _8, u32 _c, f32 _10, f32 _14, f32 _18,
                         f32 _1c, bool _20, u32 _24);

private:
    u8 _0[0x2c];
};

static_assert(sizeof(NpcTargetFinderParam) == 0x2c);

/**
 * @brief Searches the surroundings of an NPC for targets (players, cats, balls...).
 * @note Only what reconstructed code needs is declared so far.
 */
class NpcTargetFinder {
public:
    NpcTargetFinder(al::LiveActor* pHost, const NpcTargetFinderParam* pParam);

    void attackSensor(al::HitSensor* pSelf, al::HitSensor* pOther);
    void forceUpdate();
    void clearTarget();
    void setTargetTypePriority(const npc::NpcFindTargetType& rType, const u8& rPriority);

    /**
     * @brief Get the current target if it is of one of the given kinds.
     * @param types Kinds of target to accept (bit flags).
     * @return The target, or nullptr.
     */
    al::LiveActor* tryGetTarget(s32 types) const {
        if ((mTargetType & types) != 0 && mTarget != nullptr && mIsTargetValid) {
            return mTarget;
        }

        return nullptr;
    }

    /**
     * @brief Set the search parameters.
     * @param pParam Search parameters; must outlive the finder.
     */
    void setParam(const NpcTargetFinderParam* pParam) { mParam = pParam; }

    /**
     * @brief Set the kinds of targets to search for.
     * @param types Kinds of target (bit flags of npc::NpcFindTargetType).
     */
    void setSearchTypes(u32 types) { mSearchTypes = types; }

    al::HitSensor* getEyeSensor() const { return mEyeSensor; }

private:
    u8 _0[0x10];
    al::LiveActor* mTarget;  // 0x10
    u8 _18[0x20 - 0x18];
    s32 mTargetType;  // 0x20
    u8 _24[0x28 - 0x24];
    u32 mSearchTypes;  // 0x28
    bool mIsTargetValid;  // 0x2c
    u8 _2d[0x48 - 0x2d];
    const NpcTargetFinderParam* mParam;  // 0x48
    u8 _50[0xf0 - 0x50];
    al::HitSensor* mEyeSensor;  // 0xf0
};

namespace npc {
bool calcIsTargetInSight(const NpcTargetFinder* pFinder, const al::LiveActor* pTarget,
                         const NpcTargetFinderParam* pParam);
bool calcIsTargetInChaseRange(const NpcTargetFinder* pFinder, const al::LiveActor* pTarget,
                              const NpcTargetFinderParam* pParam);
}  // namespace npc
