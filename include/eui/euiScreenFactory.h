#pragma once

#include <basis/seadTypes.h>

namespace sead {
class Heap;
}

namespace eui {
class Screen;

class ScreenFactory {
public:
    virtual ~ScreenFactory() {}

    virtual Screen* createScreen(sead::Heap* pHeap, s32 screenId) = 0;
    virtual const char* getScreenName(s32 screenId) const = 0;
    virtual s32 getScreenNum() const = 0;

    virtual s32 getDrawTargetFromDrawUnitId(s8 drawUnitId) const { return drawUnitId != 0 ? 1 : 0; }
};
}  // namespace eui
