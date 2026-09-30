#include "Library/LiveActor/ActorParamHolderUtil.hpp"

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
static const ActorParamF32 sDefaultParamF32 = {};
static const ActorParamS32 sDefaultParamS32 = {};
static const ActorParamMove sDefaultParamMove = {};
static const ActorParamJump sDefaultParamJump = {};
static const ActorParamSight sDefaultParamSight = {};
static const ActorParamRebound sDefaultParamRebound = {};

/**
 * Finds a F32 parameter of an actor by name.
 * @param pActor The actor.
 * @param pName The parameter name.
 * @return The parameter, or a zero default if the actor has no parameters.
 */
const ActorParamF32* findActorParamF32(const LiveActor* pActor, const char* pName) {
    ActorParamHolder* holder = pActor->mActorParamHolder;
    if (holder == nullptr) {
        return &sDefaultParamF32;
    }
    return holder->findParamF32(pName);
}

/**
 * Finds a S32 parameter of an actor by name.
 * @param pActor The actor.
 * @param pName The parameter name.
 * @return The parameter, or a zero default if the actor has no parameters.
 */
const ActorParamS32* findActorParamS32(const LiveActor* pActor, const char* pName) {
    ActorParamHolder* holder = pActor->mActorParamHolder;
    if (holder == nullptr) {
        return &sDefaultParamS32;
    }
    return holder->findParamS32(pName);
}

/**
 * Finds a Move parameter of an actor by name.
 * @param pActor The actor.
 * @param pName The parameter name.
 * @return The parameter, or a zero default if the actor has no parameters.
 */
const ActorParamMove* findActorParamMove(const LiveActor* pActor, const char* pName) {
    ActorParamHolder* holder = pActor->mActorParamHolder;
    if (holder == nullptr) {
        return &sDefaultParamMove;
    }
    return holder->findParamMove(pName);
}

/**
 * Finds a Jump parameter of an actor by name.
 * @param pActor The actor.
 * @param pName The parameter name.
 * @return The parameter, or a zero default if the actor has no parameters.
 */
const ActorParamJump* findActorParamJump(const LiveActor* pActor, const char* pName) {
    ActorParamHolder* holder = pActor->mActorParamHolder;
    if (holder == nullptr) {
        return &sDefaultParamJump;
    }
    return holder->findParamJump(pName);
}

/**
 * Finds a Sight parameter of an actor by name.
 * @param pActor The actor.
 * @param pName The parameter name.
 * @return The parameter, or a zero default if the actor has no parameters.
 */
const ActorParamSight* findActorParamSight(const LiveActor* pActor, const char* pName) {
    ActorParamHolder* holder = pActor->mActorParamHolder;
    if (holder == nullptr) {
        return &sDefaultParamSight;
    }
    return holder->findParamSight(pName);
}

/**
 * Finds a Rebound parameter of an actor by name.
 * @param pActor The actor.
 * @param pName The parameter name.
 * @return The parameter, or a zero default if the actor has no parameters.
 */
const ActorParamRebound* findActorParamRebound(const LiveActor* pActor, const char* pName) {
    ActorParamHolder* holder = pActor->mActorParamHolder;
    if (holder == nullptr) {
        return &sDefaultParamRebound;
    }
    return holder->findParamRebound(pName);
}

/**
 * Sets the values of a movement parameter.
 * @param pParam The parameter.
 * @param moveAccel The movement acceleration.
 * @param gravity The gravity.
 * @param moveFriction The movement friction.
 * @param turnSpeedDegree The turn speed in degrees.
 */
void setActorParamMove(ActorParamMove* pParam, f32 moveAccel, f32 gravity, f32 moveFriction,
                       f32 turnSpeedDegree) {
    pParam->moveAccel = moveAccel;
    pParam->gravity = gravity;
    pParam->moveFriction = moveFriction;
    pParam->turnSpeedDegree = turnSpeedDegree;
}
}  // namespace al
