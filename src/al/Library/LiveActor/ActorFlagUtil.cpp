#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/LiveActorFlag.hpp"
#include "Library/Shadow/ShadowKeeper.hpp"
#include "Project/Collision/Collider.hpp"

namespace al {
    /**
     * @brief Checks whether an actor is alive.
     * @param pActor The actor to check.
     * @return True if the actor is not dead.
     */
    bool isAlive(const LiveActor* pActor) {
        return !pActor->mActorFlags->isDead;
    }

    /**
     * @brief Checks whether an actor is dead.
     * @param pActor The actor to check.
     * @return True if the actor has been killed or has not appeared yet.
     */
    bool isDead(const LiveActor* pActor) {
        return pActor->mActorFlags->isDead;
    }

    /**
     * @brief Checks whether an actor is dead but still counted as alive.
     * @param pActor The actor to check.
     * @return The actor's dead-alive flag.
     */
    bool isDeadAlive(LiveActor* pActor) {
        return pActor->mActorFlags->isDeadAlive;
    }

    /**
     * @brief Checks whether an actor should be counted as alive.
     * @param pActor The actor to check.
     * @return The actor's dead-alive flag.
     */
    bool isCountAsAlive(LiveActor* pActor) {
        return pActor->mActorFlags->isDeadAlive;
    }

    /**
     * @brief Checks whether an actor's collision is turned off.
     * @param pActor The actor to check.
     * @return True if the actor does not collide.
     */
    bool isNoCollide(const LiveActor* pActor) {
        return pActor->mActorFlags->isNoCollide;
    }

    /**
     * @brief Turns on animation calculation for an actor.
     * @param pActor The actor to update.
     */
    void onCalcAnim(LiveActor* pActor) {
        pActor->mActorFlags->isOffCalcAnim = false;
    }

    /**
     * @brief Turns off animation calculation for an actor.
     * @param pActor The actor to update.
     */
    void offCalcAnim(LiveActor* pActor) {
        pActor->mActorFlags->isOffCalcAnim = true;
    }

    /**
     * @brief Enables an actor's shadow, if it has one.
     * @param pActor The actor whose shadow to enable.
     */
    void validateShadow(LiveActor* pActor) {
        if (pActor->mShadowKeeper != nullptr) {
            pActor->mShadowKeeper->validate();
        }
    }

    /**
     * @brief Disables an actor's shadow, if it has one.
     * @param pActor The actor whose shadow to disable.
     */
    void invalidateShadow(LiveActor* pActor) {
        if (pActor->mShadowKeeper != nullptr) {
            pActor->mShadowKeeper->invalidate();
        }
    }

    /**
     * @brief Turns on collision for an actor and resets its collider.
     * @param pActor The actor to update.
     */
    void onCollide(LiveActor* pActor) {
        pActor->mActorFlags->isNoCollide = false;

        if (pActor->mCollider != nullptr) {
            pActor->mCollider->onInvalidate();
        }
    }

    /**
     * @brief Turns off collision for an actor.
     * @param pActor The actor to update.
     */
    void offCollide(LiveActor* pActor) {
        pActor->mActorFlags->isNoCollide = true;
    }

    /**
     * @brief Enables material code lookups for an actor's collision.
     * @param pActor The actor to update.
     */
    void validateMaterialCode(LiveActor* pActor) {
        pActor->mActorFlags->isValidMatCode = true;
    }

    /**
     * @brief Enables separate ceiling, wall and floor material code lookups for an actor.
     * @param pActor The actor to update.
     */
    void validateCeilWallFloorMaterialCode(LiveActor* pActor) {
        pActor->mActorFlags->isValidCeilWallFloorMatCode = true;
    }

    /**
     * @brief Checks whether an actor is a target of area checks.
     * @param pActor The actor to check.
     * @return True if the actor is an area target.
     */
    bool isAreaTarget(const LiveActor* pActor) {
        return pActor->mActorFlags->isAreaTarget;
    }

    /**
     * @brief Makes an actor a target of area checks.
     * @param pActor The actor to update.
     */
    void onAreaTarget(LiveActor* pActor) {
        pActor->mActorFlags->isAreaTarget = true;
    }

    /**
     * @brief Stops an actor from being a target of area checks.
     * @param pActor The actor to update.
     */
    void offAreaTarget(LiveActor* pActor) {
        pActor->mActorFlags->isAreaTarget = false;
    }

    /**
     * @brief Checks whether an actor's movement updates its collision effects and sounds.
     * @param pActor The actor to check.
     * @return True if the update is enabled.
     */
    bool isUpdateMovementEffectAudioCollision(const LiveActor* pActor) {
        return pActor->mActorFlags->isUpdMovementEffectAudioCol;
    }

    /**
     * @brief Makes an actor's movement update its collision effects and sounds.
     * @param pActor The actor to update.
     */
    void onUpdateMovementEffectAudioCollision(LiveActor* pActor) {
        pActor->mActorFlags->isUpdMovementEffectAudioCol = true;
    }

    /**
     * @brief Stops an actor's movement from updating its collision effects and sounds.
     * @param pActor The actor to update.
     */
    void offUpdateMovementEffectAudioCollision(LiveActor* pActor) {
        pActor->mActorFlags->isUpdMovementEffectAudioCol = false;
    }
};
