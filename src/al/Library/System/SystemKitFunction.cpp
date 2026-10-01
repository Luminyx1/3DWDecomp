#include "Library/System/SystemKit.hpp"

#include <gfx/seadViewport.h>

#include "Library/Framework/GameFrameworkNx.hpp"

namespace alSystemKitFunction {
/**
 * Applies a viewport to the top screen frame buffer.
 * @param rViewport The viewport.
 */
void applyViewportTop(const sead::Viewport& rViewport) {
    rViewport.apply(al::GameFrameworkNx::getDrawContext(),
                    *alProjectInterface::getSystemKit()->getFrameBufferTop());
}

/**
 * Applies a viewport to the bottom screen frame buffer.
 * @param rViewport The viewport.
 */
void applyViewportBtm(const sead::Viewport& rViewport) {
    rViewport.apply(al::GameFrameworkNx::getDrawContext(),
                    *alProjectInterface::getSystemKit()->getFrameBufferBtm());
}
}  // namespace alSystemKitFunction
