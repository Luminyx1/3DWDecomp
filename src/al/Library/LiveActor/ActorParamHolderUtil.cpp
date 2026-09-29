#include "Library/LiveActor/ActorParamHolder.hpp"
#include "Library/LiveActor/LiveActor.hpp"

namespace al {
    static const ActorParamF32 sDefaultParamF32 = {};
    static const ActorParamS32 sDefaultParamS32 = {};
    static const ActorParamMove sDefaultParamMove = {};
    static const ActorParamJump sDefaultParamJump = {};
    static const ActorParamSight sDefaultParamSight = {};
    static const ActorParamRebound sDefaultParamRebound = {};

    /**
     * @brief Finds a float parameter of an actor.
     * @param pActor The actor whose parameters to search.
     * @param pName The name of the parameter.
     * @return The parameter, or a zeroed default when the actor has no parameters.
     */
    const ActorParamF32* findActorParamF32(const LiveActor* pActor, const char* pName) {
        if (pActor->mActorParamHolder != nullptr) {
            return pActor->mActorParamHolder->findParamF32(pName);
        }

        return &sDefaultParamF32;
    }

    /**
     * @brief Finds an integer parameter of an actor.
     * @param pActor The actor whose parameters to search.
     * @param pName The name of the parameter.
     * @return The parameter, or a zeroed default when the actor has no parameters.
     */
    const ActorParamS32* findActorParamS32(const LiveActor* pActor, const char* pName) {
        if (pActor->mActorParamHolder != nullptr) {
            return pActor->mActorParamHolder->findParamS32(pName);
        }

        return &sDefaultParamS32;
    }

    /**
     * @brief Finds a movement parameter set of an actor.
     * @param pActor The actor whose parameters to search.
     * @param pName The name of the parameter set.
     * @return The parameters, or zeroed defaults when the actor has no parameters.
     */
    const ActorParamMove* findActorParamMove(const LiveActor* pActor, const char* pName) {
        if (pActor->mActorParamHolder != nullptr) {
            return pActor->mActorParamHolder->findParamMove(pName);
        }

        return &sDefaultParamMove;
    }

    /**
     * @brief Finds a jump parameter set of an actor.
     * @param pActor The actor whose parameters to search.
     * @param pName The name of the parameter set.
     * @return The parameters, or zeroed defaults when the actor has no parameters.
     */
    const ActorParamJump* findActorParamJump(const LiveActor* pActor, const char* pName) {
        if (pActor->mActorParamHolder != nullptr) {
            return pActor->mActorParamHolder->findParamJump(pName);
        }

        return &sDefaultParamJump;
    }

    /**
     * @brief Finds a sight parameter set of an actor.
     * @param pActor The actor whose parameters to search.
     * @param pName The name of the parameter set.
     * @return The parameters, or zeroed defaults when the actor has no parameters.
     */
    const ActorParamSight* findActorParamSight(const LiveActor* pActor, const char* pName) {
        if (pActor->mActorParamHolder != nullptr) {
            return pActor->mActorParamHolder->findParamSight(pName);
        }

        return &sDefaultParamSight;
    }

    /**
     * @brief Finds a rebound parameter set of an actor.
     * @param pActor The actor whose parameters to search.
     * @param pName The name of the parameter set.
     * @return The parameters, or zeroed defaults when the actor has no parameters.
     */
    const ActorParamRebound* findActorParamRebound(const LiveActor* pActor, const char* pName) {
        if (pActor->mActorParamHolder != nullptr) {
            return pActor->mActorParamHolder->findParamRebound(pName);
        }

        return &sDefaultParamRebound;
    }

    /**
     * @brief Sets every value of a movement parameter set.
     * @param pParam The parameter set to write.
     * @param moveAccel The movement acceleration.
     * @param gravity The gravity.
     * @param moveFriction The movement friction.
     * @param turnSpeedDegree The turning speed in degrees per frame.
     */
    void setActorParamMove(ActorParamMove* pParam, f32 moveAccel, f32 gravity, f32 moveFriction, f32 turnSpeedDegree) {
        pParam->mMoveAccel = moveAccel;
        pParam->mGravity = gravity;
        pParam->mMoveFriction = moveFriction;
        pParam->mTurnSpeedDegree = turnSpeedDegree;
    }
};
