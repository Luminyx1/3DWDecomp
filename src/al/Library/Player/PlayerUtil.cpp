#include "Library/Player/PlayerUtil.hpp"

#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Player/PlayerHolder.hpp"
#include "Project/Controller/PadRumbleKeeper.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"

namespace al {
bool isAreaTarget(const LiveActor* pActor);
bool faceToTarget(LiveActor* pActor, const sead::Vector3f& rTarget, f32 rate, f32 degree);

static inline PadRumbleKeeper* getPlayerPadRumbleKeeper(const PlayerHolder* holder, s32 index) {
    return holder->getPadRumbleKeeper(index);
}

static s32 findNearestPlayerIdFromPos(const LiveActor* actor, const sead::Vector3f& pos,
                                      f32 threshold) {
    PlayerHolder* holder = actor->getSceneInfo()->playerHolder;
    s32 playerNum = getPlayerNumMax(holder);

    f32 minDistance = sead::Mathf::maxNumber();
    s32 nearestPlayerId = -1;

    for (s32 i = 0; i < playerNum; i++) {
        LiveActor* player = holder->getPlayer(i);

        if (player == nullptr || isDead(player)) {
            continue;
        }

        const sead::Vector3f& playerPos = getTrans(player);
        f32 distance = (playerPos - pos).squaredLength();

        if (distance < minDistance) {
            minDistance = distance;
            nearestPlayerId = i;
        }
    }

    if (threshold * threshold < minDistance && threshold > 0.0f) {
        return -1;
    }

    return nearestPlayerId;
}

// TODO: flag-loop issue, this shouldn't be written like this
static s32 blackBox(s32 value) {
    __asm__("" : "+r"(value));
    return value;
}

s32 getPlayerNumMax(const LiveActor* actor) {
    return getPlayerNumMax(actor->getSceneInfo()->playerHolder);
}

s32 getPlayerNumMax(const PlayerHolder* holder) {
    return holder->getPlayerNum();
}

s32 getPlayerNumMaxComplete(const LiveActor* pActor) {
    return getPlayerNumMaxComplete(pActor->getSceneInfo()->playerHolder);
}

s32 getPlayerNumMaxComplete(const PlayerHolder* pHolder) {
    return pHolder->getPlayerNumComplete();
}

s32 getAlivePlayerNum(const LiveActor* actor) {
    return getAlivePlayerNum(actor->getSceneInfo()->playerHolder);
}

s32 getAlivePlayerNum(const PlayerHolder* holder) {
    s32 playerNum = getPlayerNumMax(holder);
    s32 alivePlayers = 0;

    for (s32 i = 0; i < playerNum; i++) {
        LiveActor* player = getPlayerActor(holder, i);

        if (isAlive(player))
            alivePlayers++;
    }

    return alivePlayers;
}

LiveActor* getPlayerActor(const LiveActor* actor, s32 index) {
    return getPlayerActor(actor->getSceneInfo()->playerHolder, index);
}

LiveActor* getPlayerActor(const PlayerHolder* holder, s32 index) {
    return tryGetPlayerActor(holder, index);
}

const sead::Vector3f& getPlayerPos(const LiveActor* actor, s32 index) {
    return getPlayerPos(actor->getSceneInfo()->playerHolder, index);
}

const sead::Vector3f& getPlayerPos(const PlayerHolder* holder, s32 index) {
    return getTrans(getPlayerActor(holder, index));
}

LiveActor* tryGetPlayerActor(const LiveActor* actor, s32 index) {
    return tryGetPlayerActor(actor->getSceneInfo()->playerHolder, index);
}

LiveActor* tryGetPlayerActor(const PlayerHolder* holder, s32 index) {
    return holder->tryGetPlayer(index);
}

bool isPlayerActorClass(const LiveActor* pActor, s32 index) {
    return isPlayerActorClass(pActor->getSceneInfo()->playerHolder, index);
}

bool isPlayerActorClass(const PlayerHolder* pHolder, s32 index) {
    return pHolder->isPlayerActorClass(index);
}

bool isPlayerDead(const LiveActor* actor, s32 index) {
    return isPlayerDead(actor->getSceneInfo()->playerHolder, index);
}

bool isPlayerDead(const PlayerHolder* holder, s32 index) {
    return isDead(getPlayerActor(holder, index));
}

bool isPlayerAreaTarget(const LiveActor* actor, s32 index) {
    return isPlayerAreaTarget(actor->getSceneInfo()->playerHolder, index);
}

bool isPlayerAreaTarget(const PlayerHolder* holder, s32 index) {
    return isAreaTarget(getPlayerActor(holder, index));
}

LiveActor* tryFindAlivePlayerActorFirst(const LiveActor* actor) {
    return tryFindAlivePlayerActorFirst(actor->getSceneInfo()->playerHolder);
}

LiveActor* tryFindAlivePlayerActorFirst(const PlayerHolder* holder) {
    u32 playerNum = getPlayerNumMax(holder);

    for (u32 i = 0; i < playerNum; i++) {
        LiveActor* player = holder->tryGetPlayer(i);

        if (!isDead(player))
            return player;
    }

    return nullptr;
}

LiveActor* findAlivePlayerActorFirst(const LiveActor* actor) {
    return findAlivePlayerActorFirst(actor->getSceneInfo()->playerHolder);
}

LiveActor* findAlivePlayerActorFirst(const PlayerHolder* holder) {
    return tryFindAlivePlayerActorFirst(holder);
}

PadRumbleKeeper* getPlayerPadRumbleKeeper(const LiveActor* actor, s32 index) {
    return getPlayerPadRumbleKeeper(actor->getSceneInfo()->playerHolder, index);
}

s32 getPlayerPort(const PlayerHolder* holder, s32 index) {
    return getPlayerPadRumbleKeeper(holder, index)->getPort();
}

s32 getPlayerPort(const LiveActor* actor, s32 index) {
    return getPlayerPort(actor->getSceneInfo()->playerHolder, index);
}

LiveActor* findAlivePlayerActorFromPort(const PlayerHolder* holder, s32 port) {
    return tryFindAlivePlayerActorFromPort(holder, port);
}

LiveActor* tryFindAlivePlayerActorFromPort(const PlayerHolder* holder, s32 port) {
    s32 playerNum = getPlayerNumMax(holder);

    for (s32 i = 0; i < playerNum; i++) {
        LiveActor* player = tryGetPlayerActor(holder, i);

        if (getPlayerPort(player, i) == port && !isPlayerDead(player, i))
            return player;
    }

    return nullptr;
}

LiveActor* findAlivePlayerActorFromPort(const LiveActor* actor, s32 port) {
    return tryFindAlivePlayerActorFromPort(actor, port);
}

LiveActor* tryFindAlivePlayerActorFromPort(const LiveActor* actor, s32 port) {
    return tryFindAlivePlayerActorFromPort(actor->getSceneInfo()->playerHolder, port);
}

s32 findNearestPlayerId(const LiveActor* actor, f32 threshold) {
    return findNearestPlayerIdFromPos(actor, getTrans(actor), threshold);
}

LiveActor* findNearestPlayerActor(const LiveActor* actor) {
    return getPlayerActor(actor, findNearestPlayerId(actor, -1.0f));
}

LiveActor* tryFindNearestPlayerActor(const LiveActor* actor) {
    s32 nearestPlayerId = findNearestPlayerId(actor, -1.0f);

    if (nearestPlayerId < 0)
        return nullptr;
    return getPlayerActor(actor, nearestPlayerId);
}

const sead::Vector3f& findNearestPlayerPos(const LiveActor* actor) {
    return getPlayerPos(actor, findNearestPlayerId(actor, -1.0f));
}

bool tryFindNearestPlayerPos(sead::Vector3f* pos, const LiveActor* actor) {
    s32 nearestPlayerId = findNearestPlayerId(actor, -1.0f);

    if (nearestPlayerId < 0)
        return false;
    LiveActor* player = getPlayerActor(actor, nearestPlayerId);

    if (player == nullptr)
        return false;

    *pos = getTrans(player);
    return true;
}

bool tryFindNearestPlayerDisatanceFromTarget(f32* distance, const LiveActor* actor,
                                             const sead::Vector3f& target) {
    s32 nearestPlayerId = findNearestPlayerIdFromPos(actor, target, -1.0f);

    if (nearestPlayerId < 0)
        return false;

    LiveActor* player = getPlayerActor(actor, nearestPlayerId);

    if (player == nullptr)
        return false;

    *distance = (getTrans(player) - target).length();
    return true;
}

bool isNearPlayer(const LiveActor* actor, f32 threshold) {
    return findNearestPlayerId(actor, threshold) >= 0;
}

bool isNearPlayerFromTarget(const LiveActor* pActor, const sead::Vector3f& rTarget,
                            f32 threshold) {
    return findNearestPlayerIdFromPos(pActor, rTarget, threshold) >= 0;
}

const sead::Vector3f& getFarPlayerPosMaxX(const LiveActor* actor) {
    PlayerHolder* holder = actor->getSceneInfo()->playerHolder;
    getTrans(actor);

    const LiveActor* farPlayer = nullptr;
    f32 maxX = -1.0f;

    for (s32 i = 0; i < getPlayerNumMax(holder); i++) {
        LiveActor* player = holder->getPlayer(i);

        if (farPlayer == nullptr || maxX < getTrans(player).x) {
            maxX = getTrans(player).x;
            farPlayer = player;
        }
    }

    return getTrans(farPlayer);
}

const sead::Vector3f& getFarPlayerPosMinX(const LiveActor* actor) {
    PlayerHolder* holder = actor->getSceneInfo()->playerHolder;
    getTrans(actor);

    const LiveActor* farPlayer = nullptr;
    f32 minX = -1.0f;

    for (s32 i = 0; i < getPlayerNumMax(holder); i++) {
        LiveActor* player = holder->getPlayer(i);

        if (farPlayer == nullptr || getTrans(player).x < minX) {
            minX = getTrans(player).x;
            farPlayer = player;
        }
    }

    return getTrans(farPlayer);
}

u32 calcPlayerListOrderByDistance(const LiveActor* actor, const LiveActor** actorList, u32 size) {
    u32 playerNum = getPlayerNumMax(actor);
    const sead::Vector3f& pos = getTrans(actor);

    f32 distances[64];

    for (s32 i = 0; i != (s32)playerNum; i++) {
        LiveActor* player = getPlayerActor(actor, i);
        f32 distance = sead::Mathf::maxNumber();

        if (!isDead(player)) {
            sead::Vector3f playerPos = getTrans(player);
            distance = (playerPos - pos).squaredLength();
        }

        distances[i] = distance;
    }

    for (s32 i = 0; (u32)i < size; i++) {
        f32 min_distance = sead::Mathf::maxNumber();
        s32 min_index = -1;

        for (u32 j = 0; j < playerNum; j++) {
            if (distances[j] <= min_distance) {
                min_distance = distances[j];
                min_index = j;
            }
        }

        if (min_index == -1)
            return i;

        actorList[i] = getPlayerActor(actor, min_index);
        distances[min_index] = sead::Mathf::maxNumber();
    }

    return size;
}

u32 calcAlivePlayerActor(const LiveActor* actor, const LiveActor** actorList, u32 size) {
    u32 playerNum = getPlayerNumMax(actor);
    s32 flag = 2;
    u32 result;
    u32 resultNum = 0;

    if (playerNum != 0) {
        for (u32 i = 0; i < playerNum; i++) {
            LiveActor* player = getPlayerActor(actor, i);

            if (!isDead(player)) {
                actorList[resultNum] = player;
                resultNum++;

                if (resultNum >= size) {
                    result = size;
                    flag = 1;
                } else {
                    flag = 0;
                }
            } else {
                flag = 4;
            }

            if ((blackBox(flag | 4) & 7) != 4)
                goto end;
        }

        flag = 2;
    }

end:
    if (flag != 2)
        resultNum = result;
    return resultNum;
}

bool faceToPlayer(LiveActor* pActor, f32 rate, f32 degree) {
    sead::Vector3f target = getTrans(getPlayerActor(pActor, findNearestPlayerId(pActor, -1.0f)));
    return faceToTarget(pActor, target, rate, degree);
}

f32 calcDistanceToPlayer(const LiveActor* pActor) {
    sead::Vector3f playerPos = getTrans(getPlayerActor(pActor, findNearestPlayerId(pActor, -1.0f)));
    return (getTrans(pActor) - playerPos).length();
}

bool isPlayerInRouteDokan(const LiveActor* pActor) {
    return (pActor != nullptr) && pActor->isInRouteDokan();
}

}  // namespace al

namespace alPlayerFunction {

void registerPlayer(al::LiveActor* actor, al::PadRumbleKeeper* padRumbleKeeper,
                    bool isPlayerActorClass) {
    actor->getSceneInfo()->playerHolder->registerPlayer(actor, padRumbleKeeper, isPlayerActorClass);
}

bool isFullPlayerHolder(al::LiveActor* actor) {
    return actor->getSceneInfo()->playerHolder->isFull();
}

s32 findPlayerHolderIndex(const al::LiveActor* actor) {
    al::PlayerHolder* playerHolder = actor->getSceneInfo()->playerHolder;
    s32 playerNum = playerHolder->getPlayerNum();

    for (s32 i = 0; i < playerNum; i++)
        if (playerHolder->getPlayer(i) == actor)
            return i;
    return 0;
}

s32 findPlayerHolderIndex(const al::HitSensor* sensor) {
    return findPlayerHolderIndex(al::getSensorHost(sensor));
}

bool isPlayerActor(const al::LiveActor* actor) {
    al::PlayerHolder* playerHolder = actor->getSceneInfo()->playerHolder;
    s32 playerNum = playerHolder->getPlayerNum();

    for (s32 i = 0; i < playerNum; i++)
        if (playerHolder->getPlayer(i) == actor)
            return true;
    return false;
}

bool isPlayerActor(const al::HitSensor* sensor) {
    return isPlayerActor(al::getSensorHost(sensor));
}

}  // namespace alPlayerFunction

namespace alPlayerFunction {

void swapPlayer(al::LiveActor* pActor, al::PadRumbleKeeper* pPadRumbleKeeper, s32 index) {
    pActor->getSceneInfo()->playerHolder->swapPlayer(pActor, pPadRumbleKeeper, index);
}

}  // namespace alPlayerFunction
