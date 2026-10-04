#pragma once

#include "Library/Scene/ISceneObj.hpp"

/// Scene object that tracks the cloud bonus stages of Bowser's Fury.
class CloudBonusWatcher : public al::ISceneObj {
public:
    bool isPlayerInCloudBonusStage() const;
};
