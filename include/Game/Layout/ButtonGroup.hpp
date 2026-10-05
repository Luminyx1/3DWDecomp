#pragma once

#include <basis/seadTypes.h>
#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutActor;
class LayoutInitInfo;
}

/**
 * @brief Layout button collection and cursor controller.
 * @note Storage after the cursor remains opaque; its size is verified from allocation sites.
 */
class alignas(8) ButtonGroup {
public:
    ButtonGroup(const al::LayoutInitInfo& rInfo, al::LayoutActor* pParent,
                const char* pLayoutName, const char* pCursorName, bool flag);
    void validate();
    void invalidate();
    bool isDecideAny() const;
    /** @brief Advances the button cursor layout. */
    void updateCursor() { mCursor->movement(); }
    void setPort(s32 port);
    void hideCursor();
    void showCursor();
    void reset();
    void select(const char* pButtonName);
    void updateAndCursorDefault(s32 port);
    bool isDecide(const char* pButtonName) const;

private:
    al::LayoutActor* mCursor;
    u8 mUnreconstructed8[0x248];
};
static_assert(sizeof(ButtonGroup) == 0x250);
