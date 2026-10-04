#pragma once

namespace al {
    class HitSensor;
    class LiveActor;
};

class PlayerActor;

namespace rc {
    /**
     * @brief Gets the default player transformation for a new or recovered player.
     * @return The default player figure type identifier.
     */
    int getPlayerFigureTypeDefault();
    int getPlayerCharacterNumMax();
    bool isPlayerMini(const al::HitSensor*);
    al::LiveActor* tryFindAlivePlayerActorFirstByUserId(const al::LiveActor* pPlayer, int userId);
    int getPlayerFigureType(const al::LiveActor* pPlayer);
    bool isPlayerInvincible(const al::LiveActor* pPlayer);
    bool isPlayerDamageInvalid(const al::LiveActor* pPlayer);
    void invalidatePlayerDamage(PlayerActor* pPlayer, unsigned int frame);
    void validatePlayerFlash(PlayerActor* pPlayer);
    void invalidatePlayerFlash(PlayerActor* pPlayer);
    void hidePlayerFur(const al::HitSensor* pSensor);
    void showPlayerFur(const al::HitSensor* pSensor);
    void validatePlayerDynamics(const al::HitSensor* pSensor);
    void invalidatePlayerDynamics(const al::HitSensor* pSensor);
    void resetPlayerDynamics(const al::HitSensor* pSensor);
    void validatePlayerDamage(const al::HitSensor* pSensor);
    void clearPlayerCollisionInfo(const al::HitSensor* pSensor);
    void clearPlayerExPush(const al::HitSensor* pSensor);
    void appearPlayerPrePassLight(const al::HitSensor* pSensor, const char* pName);
    void killPlayerPrePassLight(const al::HitSensor* pSensor, const char* pName);
    bool sendMsgFlingPoleDash(al::HitSensor* pReceiver, al::HitSensor* pSender, int frame);
};
