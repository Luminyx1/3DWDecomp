#pragma once

namespace al {
    class HitSensor;
    class LiveActor;
};

namespace rc {
    /**
     * @brief Gets the default player transformation for a new or recovered player.
     * @return The default player figure type identifier.
     */
    int getPlayerFigureTypeDefault();
    bool isPlayerMini(const al::HitSensor*);
    al::LiveActor* tryFindAlivePlayerActorFirstByUserId(const al::LiveActor* pPlayer, int userId);
    int getPlayerFigureType(const al::LiveActor* pPlayer);
};
