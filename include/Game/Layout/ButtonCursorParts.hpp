#pragma once

#include "Library/Layout/LayoutActor.hpp"

/**
 * @brief Selection cursor layout driven by a ButtonGroup.
 * @note Only the members used by already-decompiled callers are declared.
 */
class ButtonCursorParts : public al::LayoutActor {
public:
    void hide();
    void reset();
    bool isWait();
};
