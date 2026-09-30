#include "Library/Screen/ScreenFunction.hpp"

#include <gfx/seadViewport.h>

namespace al {
/**
 * Returns the width of the display.
 * @return width in pixels
 */
s32 getDisplayWidth() {
    return 1920;
}

/**
 * Returns the height of the display.
 * @return height in pixels
 */
s32 getDisplayHeight() {
    return 1080;
}

/**
 * Returns the width of the debug menu display.
 * @return width in pixels
 */
s32 getDebugMenuDisplayWidth() {
    return 1280;
}

/**
 * Returns the height of the debug menu display.
 * @return height in pixels
 */
s32 getDebugMenuDisplayHeight() {
    return 720;
}

/**
 * Returns the width of the sub display.
 * @return width in pixels
 */
u32 getSubDisplayWidth() {
    return 1280;
}

/**
 * Returns the height of the sub display.
 * @return height in pixels
 */
u32 getSubDisplayHeight() {
    return 720;
}

/**
 * Returns the width of the layout display.
 * @return width in layout units
 */
u32 getLayoutDisplayWidth() {
    return 1280;
}

/**
 * Returns the height of the layout display.
 * @return height in layout units
 */
u32 getLayoutDisplayHeight() {
    return 720;
}

/**
 * Returns the width of the virtual display.
 * @return width in pixels
 */
s32 getVirtualDisplayWidth() {
    return 1920;
}

/**
 * Returns the height of the virtual display.
 * @return height in pixels
 */
s32 getVirtualDisplayHeight() {
    return 1080;
}

/**
 * Sets a viewport to the display rectangle.
 * @param rViewport viewport to set
 */
void getDisplayViewport(sead::Viewport& rViewport) {
    rViewport.set(0.0f, 0.0f, 1920.0f, 1080.0f);
}

/**
 * Sets a viewport to the sub display rectangle.
 * @param rViewport viewport to set
 */
void getSubDisplayViewport(sead::Viewport& rViewport) {
    rViewport.set(0.0f, 0.0f, 1280.0f, 720.0f);
}

/**
 * Creates a viewport covering the display.
 * @return the new viewport
 */
sead::Viewport* getDisplayViewport() {
    auto* viewport = new sead::Viewport();
    getDisplayViewport(*viewport);
    return viewport;
}

/**
 * Creates a viewport covering the sub display.
 * @return the new viewport
 */
sead::Viewport* getSubDisplayViewport() {
    auto* viewport = new sead::Viewport();
    getSubDisplayViewport(*viewport);
    return viewport;
}

/**
 * Checks whether a display position is on screen.
 * @param rPos display position
 * @param margin margin allowed outside the screen
 * @return whether the position is on screen
 */
bool isInScreen(const sead::Vector2f& rPos, f32 margin) {
    if (rPos.x < -margin) {
        return false;
    }
    bool isInTop = rPos.y >= -margin;
    bool isInRight = rPos.x <= margin + 1920.0f;
    bool isInBottom = rPos.y <= margin + 1080.0f;
    return isInTop & isInRight & isInBottom;
}

/**
 * Converts a layout position to a screen position.
 * @param pOut screen position
 * @param rLayoutPos layout position
 */
void calcScreenPosFromLayoutPos(sead::Vector2f* pOut, const sead::Vector2f& rLayoutPos) {
    pOut->x = rLayoutPos.x + 640.0f;
    pOut->y = 360.0f - rLayoutPos.y;
}

/**
 * Converts a screen position to a layout position.
 * @param pOut layout position
 * @param rScreenPos screen position
 */
void calcLayoutPosFromScreenPos(sead::Vector2f* pOut, const sead::Vector2f& rScreenPos) {
    pOut->x = rScreenPos.x + -640.0f;
    pOut->y = 360.0f - rScreenPos.y;
}
}  // namespace al
