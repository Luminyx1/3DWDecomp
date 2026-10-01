#include "Library/Play/Draw/PartsGraphics.hpp"

#include "Library/Draw/GraphicsSystemInfo.hpp"

namespace al {
/**
 * Constructs the parts graphics and registers it in the graphics system.
 * @param pSystemInfo graphics system to register in
 */
PartsGraphics::PartsGraphics(GraphicsSystemInfo* pSystemInfo)
    : sead::TListNode<PartsGraphics*>(this) {
    pSystemInfo->registPartsGraphics(this);
}

/**
 * Does nothing by default.
 */
void IUsePartsGraphics::endInit() {}

/**
 * Does nothing by default.
 * @param pInfo render info
 */
void IUsePartsGraphics::drawSystem(const GraphicsRenderInfo* pInfo) const {}
}  // namespace al
