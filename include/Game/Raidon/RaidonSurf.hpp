#pragma once

#include "Library/Scene/ISceneObj.hpp"
#include "Raidon/RaidonActor.hpp"

/// Plessie in her surfing form; registered as a scene object while she exists.
class RaidonSurf : public RaidonActor, public al::ISceneObj {
public:
    void forceSpawn(bool isForce);
};
