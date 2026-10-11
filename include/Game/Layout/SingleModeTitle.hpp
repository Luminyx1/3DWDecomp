#pragma once
#include "Library/Layout/LayoutActor.hpp"
namespace al {
class LayoutInitInfo;
}
class GameDataHolder;
class ButtonGroup;
class SingleModeTitle : public al::LayoutActor {
public:
    SingleModeTitle(const al::LayoutInitInfo& info, const GameDataHolder* holder);
    void appear() override;
    void returnFromMenu(const char* menu);
    bool isStartNewGame() const;
    bool isEnd();
    bool isReturnToTopMenu() const { return mIsReturnToTopMenu; }

private:
    const GameDataHolder* mGameDataHolder;
    bool mIsReturnToTopMenu;
    ButtonGroup* mButtonGroup;
    void* mUnknown140;
};
static_assert(sizeof(SingleModeTitle) == 0x148);
