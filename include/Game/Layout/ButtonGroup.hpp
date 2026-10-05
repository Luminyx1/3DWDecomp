#pragma once

#include <basis/seadTypes.h>

namespace al {
class LayoutActor;
class LayoutInitInfo;
}

/**
 * @brief Layout button collection and cursor controller.
 * @note The storage remains opaque; its size is verified from constructor allocation sites.
 */
class alignas(8) ButtonGroup {
public:
    ButtonGroup(const al::LayoutInitInfo& rInfo, al::LayoutActor* pParent,
                const char* pLayoutName, const char* pCursorName, bool flag);
    void setPort(s32 port);
    void hideCursor();
    void showCursor();
    void reset();
    void select(const char* pButtonName);
    void updateAndCursorDefault(s32 port);
    bool isDecide(const char* pButtonName) const;

private:
    u8 mUnreconstructed[0x250];
};
static_assert(sizeof(ButtonGroup) == 0x250);
