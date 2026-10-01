#include "Library/Screen/ScreenPointerUtil.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Screen/ScreenPointDirector.hpp"
#include "Library/Screen/ScreenPointKeeper.hpp"
#include "Library/Screen/ScreenPointTarget.hpp"
#include "Library/Screen/ScreenPointer.hpp"

namespace al {
/**
 * Adds a screen point target to an actor and registers it to the screen point director.
 * @param pActor actor
 * @param rInfo actor init info
 * @param pName target name
 * @param radius target radius
 * @param pJointName joint to follow, or nullptr to follow the actor position
 * @param rOffset offset from the followed position or joint
 * @return the new target
 */
ScreenPointTarget* addScreenPointTarget(LiveActor* pActor, const ActorInitInfo& rInfo,
                                        const char* pName, f32 radius, const char* pJointName,
                                        const sead::Vector3f& rOffset) {
    ScreenPointKeeper* keeper = pActor->mScreenPointKeeper;
    const sead::Matrix34f* jointMtx = (pJointName != nullptr) ? getJointMtxPtr(pActor, pJointName) : nullptr;
    ScreenPointTarget* target =
        keeper->addTarget(pActor, pName, radius, getTransPtr(pActor), jointMtx, rOffset);
    ScreenPointDirector* director = rInfo.mScreenPointerDirector;
    director->registerTarget(target);
    director->setCheckGroup(target);
    return target;
}

/**
 * Collects the screen point targets hit by a segment.
 * @param pPointer screen pointer
 * @param rStart segment start
 * @param rEnd segment end
 * @return whether a target was hit
 */
bool hitCheckSegmentScreenPointTarget(ScreenPointer* pPointer, const sead::Vector3f& rStart,
                                      const sead::Vector3f& rEnd) {
    return pPointer->hitCheckSegment(rStart, rEnd);
}

/**
 * Collects the screen point targets hit by a circle on screen.
 * @param pPointer screen pointer
 * @param rPos circle center on screen
 * @param radius circle radius
 * @return whether a target was hit
 */
bool hitCheckScreenCircleScreenPointTarget(ScreenPointer* pPointer, const sead::Vector2f& rPos,
                                           f32 radius) {
    return pPointer->hitCheckScreenCircle(rPos, radius);
}

/**
 * Sends a screen point message to the host of a target.
 * @param rMsg message
 * @param pPointer screen pointer
 * @param pTarget target
 * @return whether the message was received
 */
bool sendMsgScreenPointTarget(const SensorMsg& rMsg, ScreenPointer* pPointer,
                              ScreenPointTarget* pTarget) {
    return pTarget->getHost()->receiveMsgScreenPoint(&rMsg, pPointer, pTarget);
}

/**
 * Sends a screen point message to the host of a target.
 * @param rMsg message
 * @param pPointer screen pointer
 * @param pTarget target
 * @return whether the message was received
 */
bool sendMsgScreenPointTargetSM(const SensorMsg& rMsg, ScreenPointer* pPointer,
                                ScreenPointTarget* pTarget) {
    return pTarget->getHost()->receiveMsgScreenPointSM(&rMsg, pPointer, pTarget);
}
}  // namespace al

namespace alScreenPointFunction {
/**
 * Updates every screen point target of an actor.
 * @param pActor actor
 */
void updateScreenPointAll(al::LiveActor* pActor) {
    pActor->mScreenPointKeeper->update();
}
}  // namespace alScreenPointFunction
