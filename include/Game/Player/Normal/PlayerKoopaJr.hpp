#pragma once

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Scene/ISceneObj.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/// Bowser Jr. when controlled by a second player in Bowser's Fury.
class PlayerKoopaJr : public al::LiveActor, public al::ISceneObj {
public:
    static PlayerKoopaJr* tryGetPlayerKoopaJr(const al::IUseSceneObjHolder* pUser);
    void hideForDemo();
    void showFromDemo();
    PlayerProperty* getProperty() { return &mProperty; }

private:
    u8 _150[0x170 - 0x150];
    PlayerProperty mProperty;  // 0x170
};
