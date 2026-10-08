#pragma once

#include <basis/seadTypes.h>
#include "Library/Layout/LayoutActor.hpp"

namespace al {
class ActorInitInfo;
class LayoutInitInfo;
}  // namespace al
class GameDataHolder;

/**
 * @brief Character select layout where every player picks a character before playing.
 * @note Only the members used by already-decompiled callers are declared.
 */
class PlayerEntry : public al::LayoutActor {
public:
    PlayerEntry(const al::LayoutInitInfo& rLayoutInfo, const al::ActorInitInfo& rActorInfo,
                GameDataHolder* pGameDataHolder);

    void startCharacterSelect();
    void appearWithDecidePort(s32 port);
    void resetButtonIcons();
    void hidePlayerAll();
    bool isRequestBack() const;
    bool isFinishBack() const;
    bool isAllPlayerDecided() const;
    bool isAllPlayerDecideStart() const;

private:
    u8 _121[0x150 - 0x121];
};

static_assert(sizeof(PlayerEntry) == 0x150);
