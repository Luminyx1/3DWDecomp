#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class PlayerActor;

/**
 * @brief Crown worn by the player with the best score of the last stage.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class PlayerCrown : public al::LiveActor {
public:
    void changeHost(PlayerActor* pHost);

    /** @brief Gets the player wearing the crown. @return The host player. */
    PlayerActor* getHost() const { return mHost; }

private:
    PlayerActor* mHost;  // 0x148
};

static_assert(sizeof(al::LiveActor) == 0x148);
