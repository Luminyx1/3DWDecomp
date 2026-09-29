#include "Library/Screen/ScreenPointerUtil.hpp"

#include <gfx/seadColor.h>

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
 * @brief Adds a screen point target to an actor and registers it.
 * @param pActor The actor.
 * @param rInfo The actor's init info.
 * @param pName The target name.
 * @param radius The target radius.
 * @param pJointName The joint the target follows, or null to follow the actor.
 * @param rOffset The offset from the followed position.
 * @return The new target.
 */
ScreenPointTarget* addScreenPointTarget(LiveActor* pActor, const ActorInitInfo& rInfo,
                                        const char* pName, f32 radius, const char* pJointName,
                                        const sead::Vector3f& rOffset) {
    ScreenPointKeeper* keeper = pActor->mScreenPointKeeper;
    const sead::Matrix34f* jointMtx = pJointName ? getJointMtxPtr(pActor, pJointName) : nullptr;
    ScreenPointTarget* target =
        keeper->addTarget(pActor, pName, radius, getTransPtr(pActor), jointMtx, rOffset);
    ScreenPointDirector* director = rInfo.mScreenPointerDirector;
    director->registerTarget(target);
    director->setCheckGroup(target);
    return target;
}

/**
 * @brief Collects the targets hit by a segment.
 * @param pPointer The pointer.
 * @param rStart The segment start.
 * @param rEnd The segment end.
 * @return True if any target was hit.
 */
bool hitCheckSegmentScreenPointTarget(ScreenPointer* pPointer, const sead::Vector3f& rStart,
                                      const sead::Vector3f& rEnd) {
    return pPointer->hitCheckSegment(rStart, rEnd);
}

/**
 * @brief Collects the targets hit by a circle on screen.
 * @param pPointer The pointer.
 * @param rPos The circle center in screen space.
 * @param radius The circle radius.
 * @return True if any target was hit.
 */
bool hitCheckScreenCircleScreenPointTarget(ScreenPointer* pPointer, const sead::Vector2f& rPos,
                                           f32 radius) {
    return pPointer->hitCheckScreenCircle(rPos, radius);
}

/**
 * @brief Sends a screen point message to the owner of a target.
 * @param rMsg The message.
 * @param pPointer The sending pointer.
 * @param pTarget The receiving target.
 * @return True if the message was handled.
 */
bool sendMsgScreenPointTarget(const SensorMsg& rMsg, ScreenPointer* pPointer,
                              ScreenPointTarget* pTarget) {
    return pTarget->mActor->receiveMsgScreenPoint(&rMsg, pPointer, pTarget);
}

/**
 * @brief Sends a screen point message to the owner of a target through its SM handler.
 * @param rMsg The message.
 * @param pPointer The sending pointer.
 * @param pTarget The receiving target.
 * @return True if the message was handled.
 */
bool sendMsgScreenPointTargetSM(const SensorMsg& rMsg, ScreenPointer* pPointer,
                                ScreenPointTarget* pTarget) {
    return pTarget->mActor->receiveMsgScreenPointSM(&rMsg, pPointer, pTarget);
}

}  // namespace al

namespace alScreenPointFunction {

/**
 * @brief Updates the positions of all screen point targets of an actor.
 * @param pActor The actor.
 */
void updateScreenPointAll(al::LiveActor* pActor) {
    pActor->mScreenPointKeeper->update();
}

}  // namespace alScreenPointFunction
