#include "Library/MapObj/ChildStep.hpp"

#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"

namespace {
using namespace al;

NERVE_DECL(ChildStep, Wait)
NERVES_MAKE_NOSTRUCT(ChildStep, Wait)
}  // namespace

namespace al {
/**
 * Constructs a child step of a parent actor.
 * @param pName actor name
 * @param pParent parent actor
 */
ChildStep::ChildStep(const char* pName, LiveActor* pParent) : LiveActor(pName), mParent(pParent) {}

/**
 * Initializes the child step relative to its parent.
 * @param rInfo actor init info
 */
void ChildStep::init(const ActorInitInfo& rInfo) {
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, 0);
    initNerve(this, &NrvChildStepWait, 0);
    multVecInvQuat(&mLocalTrans, mParent, getTrans(this));
    makeActorAppeared();
}

/**
 * Forwards messages to the parent.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the parent handled the message
 */
bool ChildStep::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    return mParent->receiveMsg(pMsg, pOther, pSelf);
}

/**
 * Follows the parent pose.
 */
void ChildStep::exeWait() {
    multVecPose(getTransPtr(this), mParent, mLocalTrans);
}

/**
 * Counts the linked child steps.
 * @param rInfo actor init info
 * @return child step count
 */
s32 calcChildStepCount(const ActorInitInfo& rInfo) {
    return calcLinkChildNum(rInfo, "ChildStep");
}

/**
 * Initializes the sub actor keeper of an actor with child steps.
 * @param pActor actor
 * @param rInfo actor init info
 */
void tryInitSubActorKeeperChildStep(LiveActor* pActor, const ActorInitInfo& rInfo) {
    s32 count = calcChildStepCount(rInfo);
    if (count <= 0) {
        return;
    }
    initSubActorKeeperNoFile(pActor, rInfo, count);
}

/**
 * Creates the linked child steps of an actor.
 * @param rInfo actor init info
 * @param pParent parent actor
 * @param isSyncClipping whether the child steps are clipped with the parent
 */
void createChildStep(const ActorInitInfo& rInfo, LiveActor* pParent, bool isSyncClipping) {
    s32 count = calcChildStepCount(rInfo);
    for (s32 i = 0; i < count; i++) {
        ChildStep* childStep = new ChildStep("子供足場", pParent);
        initLinksActor(childStep, rInfo, "ChildStep", i);
        childStep->_142 = true;
        if (isSyncClipping) {
            invalidateClipping(childStep);
            registerSubActorSyncClipping(pParent, childStep, false);
        }
    }
}
}  // namespace al
