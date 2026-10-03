#pragma once

namespace al {
    class HitSensor;
};

namespace rc {
    /**
     * @brief Gets the default player transformation for a new or recovered player.
     * @return The default player figure type identifier.
     */
    int getPlayerFigureTypeDefault();
    bool isPlayerMini(const al::HitSensor*);
};
