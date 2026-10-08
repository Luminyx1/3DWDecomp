#pragma once

#include <basis/seadTypes.h>
#include "Library/Sequence/Sequence.hpp"

/**
 * @brief The game's root sequence, switching between the title, stages and the menus.
 * @note Only the members used by already-decompiled callers are declared.
 */
class ProductSequence : public al::Sequence {
public:
    void setKioskStageStartParam(s32 worldId, s32 stageId);
    void requestCaptureTopBottom();
};
