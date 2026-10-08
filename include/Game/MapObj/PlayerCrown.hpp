#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class PlayerActor;

/**
 * @brief Crown worn by the player with the best score of the last stage.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class PlayerCrown : public al::LiveActor {
public:
    PlayerCrown(const al::ActorInitInfo& rInfo, const char* pName);

    void changeHost(PlayerActor* pHost);
    bool isAttach() const;

    /** @brief Gets the player wearing the crown. @return The host player. */
    PlayerActor* getHost() const { return mHost; }

private:
    PlayerActor* mHost;  // 0x148
    u8 _150[0x2a0 - 0x150];
};

static_assert(sizeof(PlayerCrown) == 0x2a0);

static_assert(sizeof(al::LiveActor) == 0x148);
