#pragma once

#include "Library/Layout/LayoutActor.hpp"

namespace al { class LayoutInitInfo; }
class GameDataHolder;

/** @brief Stamp collection parts used by the result screen. */
class ListStampParts : public al::LayoutActor {
public:
    ListStampParts(const al::LayoutInitInfo& rInfo, const char* pName,
                   const char* pPartsName, al::LayoutActor* pParent,
                   const GameDataHolder* pGameData);
    void appear() override;
    void startDecide();
    bool isDecideEnd() const;
    void setAcquiredStampNameByCourseId(int courseId);
    void setAcquiredStampNameByCharacterComplete(int characterId);

private:
    u8 mUnreconstructed[0x7f];
};

static_assert(sizeof(ListStampParts) == 0x1a0);
