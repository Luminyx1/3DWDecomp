#pragma once

#include <math/seadVector.h>

namespace al {
class HitSensor;
class LiveActor;
class PadRumbleKeeper;
class PlayerHolder;

s32 getPlayerNumMax(const LiveActor* actor);
s32 getPlayerNumMax(const PlayerHolder* holder);
s32 getPlayerNumMaxComplete(const LiveActor* pActor);
s32 getPlayerNumMaxComplete(const PlayerHolder* pHolder);
s32 getAlivePlayerNum(const LiveActor* actor);
s32 getAlivePlayerNum(const PlayerHolder* holder);
LiveActor* getPlayerActor(const LiveActor* actor, s32 index);
LiveActor* getPlayerActor(const PlayerHolder* holder, s32 index);
const sead::Vector3f& getPlayerPos(const LiveActor* actor, s32 index);
const sead::Vector3f& getPlayerPos(const PlayerHolder* holder, s32 index);
LiveActor* tryGetPlayerActor(const LiveActor* actor, s32 index);
LiveActor* tryGetPlayerActor(const PlayerHolder* holder, s32 index);
bool isPlayerActorClass(const LiveActor* pActor, s32 index);
bool isPlayerActorClass(const PlayerHolder* pHolder, s32 index);
bool isPlayerDead(const LiveActor* actor, s32 index);
bool isPlayerDead(const PlayerHolder* holder, s32 index);
bool isPlayerAreaTarget(const LiveActor* actor, s32 index);
bool isPlayerAreaTarget(const PlayerHolder* holder, s32 index);
LiveActor* tryFindAlivePlayerActorFirst(const LiveActor* actor);
LiveActor* tryFindAlivePlayerActorFirst(const PlayerHolder* holder);
LiveActor* findAlivePlayerActorFirst(const LiveActor* actor);
LiveActor* findAlivePlayerActorFirst(const PlayerHolder* holder);
PadRumbleKeeper* getPlayerPadRumbleKeeper(const LiveActor* actor, s32 index);
s32 getPlayerPort(const PlayerHolder* holder, s32 index);
s32 getPlayerPort(const LiveActor* actor, s32 index);
LiveActor* findAlivePlayerActorFromPort(const PlayerHolder* holder, s32 port);
LiveActor* tryFindAlivePlayerActorFromPort(const PlayerHolder* holder, s32 port);
LiveActor* findAlivePlayerActorFromPort(const LiveActor* actor, s32 port);
LiveActor* tryFindAlivePlayerActorFromPort(const LiveActor* actor, s32 port);
s32 findNearestPlayerId(const LiveActor* actor, f32 threshold);
LiveActor* findNearestPlayerActor(const LiveActor* actor);
LiveActor* tryFindNearestPlayerActor(const LiveActor* actor);
const sead::Vector3f& findNearestPlayerPos(const LiveActor* actor);
bool tryFindNearestPlayerPos(sead::Vector3f* pos, const LiveActor* actor);
bool tryFindNearestPlayerDisatanceFromTarget(f32* distance, const LiveActor* actor,
                                             const sead::Vector3f& target);
bool isNearPlayer(const LiveActor* actor, f32 threshold);
bool isNearPlayerFromTarget(const LiveActor* pActor, const sead::Vector3f& rTarget, f32 threshold);
const sead::Vector3f& getFarPlayerPosMaxX(const LiveActor* actor);
const sead::Vector3f& getFarPlayerPosMinX(const LiveActor* actor);
u32 calcPlayerListOrderByDistance(const LiveActor* actor, const LiveActor** actorList, u32 size);
u32 calcAlivePlayerActor(const LiveActor* actor, const LiveActor** actorList, u32 size);
bool faceToPlayer(LiveActor* pActor, f32 rate, f32 degree);
f32 calcDistanceToPlayer(const LiveActor* pActor);
bool isPlayerInRouteDokan(const LiveActor* pActor);
}  // namespace al

namespace alPlayerFunction {
void registerPlayer(al::LiveActor* actor, al::PadRumbleKeeper* padRumbleKeeper, bool isPlayerActorClass);
bool isFullPlayerHolder(al::LiveActor* actor);
s32 findPlayerHolderIndex(const al::LiveActor* actor);
s32 findPlayerHolderIndex(const al::HitSensor* sensor);
bool isPlayerActor(const al::LiveActor* actor);
bool isPlayerActor(const al::HitSensor* sensor);
void swapPlayer(al::LiveActor* pActor, al::PadRumbleKeeper* pPadRumbleKeeper, s32 index);
}  // namespace alPlayerFunction
