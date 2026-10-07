#pragma once

#include "Library/Scene/ISceneObj.hpp"

namespace al { class LiveActor; }

/// Scene object that tracks the cloud bonus stages of Bowser's Fury.
class CloudBonusWatcher : public al::ISceneObj {
public:
    bool tryRegisterActor(al::LiveActor*, int);
    bool isPlayerInCloudBonusStage() const;
};
