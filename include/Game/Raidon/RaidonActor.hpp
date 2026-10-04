#pragma once

#include "Library/LiveActor/LiveActor.hpp"

/// Plessie, the dinosaur the player rides in Bowser's Fury.
class RaidonActor : public al::LiveActor {
public:
    virtual void init(const al::ActorInitInfo& rInfo, const char* pName);
    virtual void startPuppetActionAll(const char* pActionName);
    virtual void setPuppetInputBlendAnimWeight();
    virtual void setInputBlendAnimWeight();
    virtual bool isOnGroundRaidon() const;
    virtual bool isInWater() const;
    virtual bool isOnGroundOrWaterRaidon() const;

private:
    u8 _144[0x170 - 0x144];
};
