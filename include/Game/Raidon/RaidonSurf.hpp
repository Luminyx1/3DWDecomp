#pragma once

#include "Library/Scene/ISceneObj.hpp"
#include "Raidon/RaidonBase.hpp"

/// Plessie in her surfing form; registered as a scene object while she exists.
class RaidonSurf : public RaidonBase, public al::ISceneObj {
public:
    void startPuppetActionAll(const char* pActionName) override;
    void setPuppetInputBlendAnimWeight() override;
    void setInputBlendAnimWeight() override;
    bool isOnGroundRaidon() const override;
    bool isInWater() const override;
    bool isOnGroundOrWaterRaidon() const override;
    void forceSpawn(bool isForce);
};
