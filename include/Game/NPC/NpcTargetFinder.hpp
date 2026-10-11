#pragma once
#include "NPC/NpcTargetFunction.hpp"
#include "NPC/NpcTargetFinderParam.hpp"
#include "NPC/NpcTargetMetrics.hpp"
#include <container/seadObjArray.h>
#include <container/seadTreeMap.h>
#include <math/seadVector.h>
namespace al {
class LiveActor;
class HitSensor;
class SensorMsg;
class ScreenPointer;
class ScreenPointTarget;
} // namespace al
class IUseTargetFinderFilter {
public:
    /** @brief Approves a candidate. @param pActor Candidate actor. @param rType Candidate category.
     * @return Whether the candidate is accepted. */
    virtual bool acceptTarget(const al::LiveActor* pActor,
                              const npc::NpcFindTargetType& rType) const = 0;
};

class NpcTargetFinder {
public:
    NpcTargetFinder(al::LiveActor*, const NpcTargetFinderParam*);
    void changeHost(al::LiveActor*, const char*, const IUseTargetFinderFilter*);
    void attackSensor(al::HitSensor*, al::HitSensor*);
    bool receiveMsg(const al::SensorMsg*, al::HitSensor*, al::HitSensor*);
    bool receiveMsgScreenPoint(const al::SensorMsg*, al::ScreenPointer*, al::ScreenPointTarget*);
    void findTarget();
    const al::LiveActor* tryFilterTarget(const al::LiveActor*, const npc::NpcFindTargetType&) const;
    unsigned char getTargetPriority(const npc::NpcFindTargetType&) const;
    void update();
    void updateTarget();
    void forceUpdate();
    void refindTarget();
    void setTargetTypePriority(const npc::NpcFindTargetType&, const unsigned char&);
    void clearPriorityMap();
    void clearTarget();
    void setFrontDir(sead::Vector3f*);
    void setSupportUpDir(sead::Vector3f*);
    const sead::Vector3f& getTargetPos() const;
    const sead::Vector3f& getLastTargetPos() const;
    bool isInSightTarget(float) const;
    bool isInChaseRangeTarget(float) const;
    bool isInSenseAreaTarget() const;
    al::LiveActor* mHostActor;
    const IUseTargetFinderFilter* mFilter;
    const al::LiveActor* mTarget;
    const al::LiveActor* mLastTarget;
    npc::NpcFindTargetType mType;
    npc::NpcFindTargetType mLastType;
    int mAllowedTypes;
    bool mInSight;
    bool mIsInChaseRange;
    bool mHasTargetChanged;
    sead::Vector3f* mFront;
    sead::Vector3f* mGravity;
    int mGraceFrames;
    int mScanFrames;
    const NpcTargetFinderParam* mParam;
    npc::TargetMetrics* mMetrics;
    struct Candidate { /** @brief Stores a candidate. @param pActor Candidate actor. @param type
                          Candidate category. */
        Candidate(const al::LiveActor* pActor, npc::NpcFindTargetType type)
            : mActor(pActor), mType(type) {}
        const al::LiveActor* mActor;
        npc::NpcFindTargetType mType;
    };
    sead::FixedObjArray<Candidate, 5> mCandidates;
    al::HitSensor* mSensor;
    sead::FixedTreeMap<npc::NpcFindTargetType, unsigned char, 8> mPriorities;
};
