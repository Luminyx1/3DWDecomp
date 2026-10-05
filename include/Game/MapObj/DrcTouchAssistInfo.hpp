#pragma once
#include <math/seadVector.h>

/** @brief Touch-assist state; only the layout position is currently reconstructed. */
class DrcTouchAssistInfo {
public:
    /** @brief Returns the position used by the touch cursor.
     * @return Layout-space touch position. */
    const sead::Vector2f& getLayoutPos() const { return mLayoutPos; }
private:
    u8 mUnreconstructed0[0x1c];
    sead::Vector2f mLayoutPos;
    u8 mUnreconstructed24[0x28];
};
static_assert(sizeof(DrcTouchAssistInfo) == 0x4c);
