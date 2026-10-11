#pragma once

#include "Library/LiveActor/LiveActor.hpp"

/// Plessie, the dinosaur the player rides in Bowser's Fury.
class RaidonActor : public al::LiveActor {
public:
    RaidonActor(const char* pName, const char* pSuffix);
    /** @brief Releases the Plessie actor. */
    ~RaidonActor() override = default;
    virtual void init(const al::ActorInitInfo& rInfo, const char* pName);

private:
    u8 _144[0x170 - 0x144];
};
