#pragma once
#include "Raidon/RaidonActor.hpp"

// Primary riding interface shared by Raidon and RaidonSurf; no instances are allocated here.
class RaidonBase : public RaidonActor {
public:
    virtual void startPuppetActionAll(const char* pActionName) = 0;
    virtual void setPuppetInputBlendAnimWeight() = 0;
    virtual void setInputBlendAnimWeight() = 0;
    virtual bool isOnGroundRaidon() const = 0;
    virtual bool isInWater() const = 0;
    virtual bool isOnGroundOrWaterRaidon() const;
};
