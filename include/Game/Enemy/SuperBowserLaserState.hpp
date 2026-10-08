#pragma once

#include "Library/Nerve/NerveStateBase.hpp"

/**
 * @brief Laser attack state of Fury Bowser.
 * @note Only what reconstructed code needs is declared so far.
 */
class SuperBowserLaserState : public al::ActorStateBase {
public:
    void appear() override;
    virtual void forceKill();
};
