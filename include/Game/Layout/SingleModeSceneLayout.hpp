#pragma once

#include "Library/Layout/LayoutActor.hpp"
#include "Library/Scene/ISceneObj.hpp"

namespace al {
class LiveActor;
}  // namespace al

/**
 * @brief Receiver of items put into the item stock.
 * @note Name not confirmed.
 */
class IUseItemStock {
public:
    virtual void stockItem(const al::LiveActor* pItem, const al::LiveActor* pPlayer,
                           s32 itemType) = 0;
    virtual void stockItemSilent(const al::LiveActor* pItem, const al::LiveActor* pPlayer,
                                 s32 itemType) = 0;
};

/**
 * @brief The HUD layout of Bowser's Fury.
 * @note Only what reconstructed code needs is declared so far.
 */
class SingleModeSceneLayout : public al::LayoutActor, public IUseItemStock, public al::ISceneObj {
public:
    void forceHideShineCounter();
    void setDisableAreaName(bool isDisable);
};
