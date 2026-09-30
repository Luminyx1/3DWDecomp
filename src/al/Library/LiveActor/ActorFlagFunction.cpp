#include "Library/LiveActor/Util/ActorFlagUtil.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/LiveActorFlag.hpp"
#include "Library/Shadow/ShadowKeeper.hpp"
#include "Project/Collision/Collider.hpp"

namespace al {
/**
 * Checks whether an actor is alive.
 * @param pActor The actor.
 * @return Whether the actor is alive.
 */
bool isAlive(const LiveActor* pActor) {
    return !pActor->mActorFlags->isDead;
}

/**
 * Checks whether an actor is dead.
 * @param pActor The actor.
 * @return Whether the actor is dead.
 */
bool isDead(const LiveActor* pActor) {
    return pActor->mActorFlags->isDead;
}

/**
 * Checks whether an actor is dead but still counted as alive.
 * @param pActor The actor.
 * @return The dead-alive flag.
 */
bool isDeadAlive(LiveActor* pActor) {
    return pActor->mActorFlags->isDeadAlive;
}

/**
 * Checks whether an actor counts as alive.
 * @param pActor The actor.
 * @return The dead-alive flag.
 */
bool isCountAsAlive(LiveActor* pActor) {
    return pActor->mActorFlags->isDeadAlive;
}

/**
 * Checks whether collision is disabled for an actor.
 * @param pActor The actor.
 * @return Whether collision is disabled.
 */
bool isNoCollide(const LiveActor* pActor) {
    return pActor->mActorFlags->isNoCollide;
}

/**
 * Enables animation calculation for an actor.
 * @param pActor The actor.
 */
void onCalcAnim(LiveActor* pActor) {
    pActor->mActorFlags->isOffCalcAnim = false;
}

/**
 * Disables animation calculation for an actor.
 * @param pActor The actor.
 */
void offCalcAnim(LiveActor* pActor) {
    pActor->mActorFlags->isOffCalcAnim = true;
}

/**
 * Validates the shadow of an actor, if it has one.
 * @param pActor The actor.
 */
void validateShadow(LiveActor* pActor) {
    if (pActor->mShadowKeeper != nullptr) {
        pActor->mShadowKeeper->validate();
    }
}

/**
 * Invalidates the shadow of an actor, if it has one.
 * @param pActor The actor.
 */
void invalidateShadow(LiveActor* pActor) {
    if (pActor->mShadowKeeper != nullptr) {
        pActor->mShadowKeeper->invalidate();
    }
}

/**
 * Enables collision for an actor and resets its collider.
 * @param pActor The actor.
 */
void onCollide(LiveActor* pActor) {
    pActor->mActorFlags->isNoCollide = false;
    if (pActor->mCollider != nullptr) {
        pActor->mCollider->onInvalidate();
    }
}

/**
 * Disables collision for an actor.
 * @param pActor The actor.
 */
void offCollide(LiveActor* pActor) {
    pActor->mActorFlags->isNoCollide = true;
}

/**
 * Enables material code updates for an actor.
 * @param pActor The actor.
 */
void validateMaterialCode(LiveActor* pActor) {
    pActor->mActorFlags->isValidMatCode = true;
}

/**
 * Enables ceiling, wall and floor material code updates for an actor.
 * @param pActor The actor.
 */
void validateCeilWallFloorMaterialCode(LiveActor* pActor) {
    pActor->mActorFlags->isValidCeilWallFloorMatCode = true;
}

/**
 * Checks whether an actor is an area target.
 * @param pActor The actor.
 * @return Whether the actor is an area target.
 */
bool isAreaTarget(const LiveActor* pActor) {
    return pActor->mActorFlags->isAreaTarget;
}

/**
 * Makes an actor an area target.
 * @param pActor The actor.
 */
void onAreaTarget(LiveActor* pActor) {
    pActor->mActorFlags->isAreaTarget = true;
}

/**
 * Stops an actor from being an area target.
 * @param pActor The actor.
 */
void offAreaTarget(LiveActor* pActor) {
    pActor->mActorFlags->isAreaTarget = false;
}

/**
 * Checks whether an actor updates effects, audio, collision and sensors in its movement.
 * @param pActor The actor.
 * @return Whether those updates are enabled.
 */
bool isUpdateMovementEffectAudioCollision(const LiveActor* pActor) {
    return pActor->mActorFlags->isUpdMovementEffectAudioCol;
}

/**
 * Enables effect, audio, collision and sensor updates in an actor's movement.
 * @param pActor The actor.
 */
void onUpdateMovementEffectAudioCollision(LiveActor* pActor) {
    pActor->mActorFlags->isUpdMovementEffectAudioCol = true;
}

/**
 * Disables effect, audio, collision and sensor updates in an actor's movement.
 * @param pActor The actor.
 */
void offUpdateMovementEffectAudioCollision(LiveActor* pActor) {
    pActor->mActorFlags->isUpdMovementEffectAudioCol = false;
}
}  // namespace al
