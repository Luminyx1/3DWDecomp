#pragma once

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Scene/ISceneObj.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/// Bowser Jr. when controlled by a second player in Bowser's Fury.
class PlayerKoopaJr : public al::LiveActor, public al::ISceneObj {
public:
    static PlayerKoopaJr* tryGetPlayerKoopaJr(const al::IUseSceneObjHolder* pUser);
    bool canDoIslandWarp() const;
    bool tryPraiseReaction(int);
    void hideForDemo();
    void startMidBossDemo();
    void showFromDemo();
    void resetTransformPostCutscene(const sead::Vector3f& rTrans, const sead::Vector3f& rFront);
    PlayerProperty* getProperty() { return &mProperty; }
    bool isThrowingItem() const;
    bool tryThrowStockItem(int itemType, const char* pItemName, al::LiveActor* pPlayer);

    /**
     * @brief Check whether Bowser Jr. is driven by the AI instead of a second player.
     * @return True if the AI moves Bowser Jr.
     */
    bool isAIMovement() const { return mIsUseAIMovement; }

private:
    u8 _150[0x170 - 0x150];
    PlayerProperty mProperty;  // 0x170
    u8 _1ec[0x2dc - 0x1ec];
    bool mIsUseAIMovement;  // 0x2dc
};
