#pragma once

#include <container/seadTList.h>

#include "Library/Draw/IUsePartsGraphics.hpp"

namespace al {
class GraphicsSystemInfo;

class PartsGraphics : public IUsePartsGraphics, public sead::TListNode<PartsGraphics*> {
public:
    PartsGraphics(GraphicsSystemInfo* pSystemInfo);
};

static_assert(sizeof(PartsGraphics) == 0x28);
}  // namespace al
