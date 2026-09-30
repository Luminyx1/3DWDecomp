#include "Library/Shader/DeferredRendering/AtmosScatterDrawer.hpp"

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Execute/ExecuteUtil.hpp"
#include "Library/Shader/DeferredRendering/AtmosScatter.hpp"

namespace al {
/**
 * Registers the atmospheric scattering drawer.
 * @param pExecuteDirector Execute director to register to.
 * @param pGraphicsSystemInfo Graphics system info.
 */
AtmosScatterDrawer::AtmosScatterDrawer(ExecuteDirector* pExecuteDirector,
                                       GraphicsSystemInfo* pGraphicsSystemInfo)
    : mGraphicsSystemInfo(pGraphicsSystemInfo) {
    registerExecutorUser(this, pExecuteDirector, "大気散乱");
}

/**
 * Draws the atmospheric scattering of the far background.
 */
void AtmosScatterDrawer::draw() const {
    GraphicsSystemInfo* info = mGraphicsSystemInfo;
    if (info->_40 != 1) {
        return;
    }
    AtmosScatter* atmosScatter = info->mAtmosScatter;
    if (!atmosScatter) {
        return;
    }
    sead::PerspectiveProjection* projection = info->mDrawProjection;
    atmosScatter->drawFarDeferred(info->mDrawViewIndex, info->mDrawGBufferArray,
                                  info->mDrawCamera->getMatrix(), projection->getProjectionMatrix(),
                                  projection->getOffsetDirect(), projection->getFovy(),
                                  projection->getAspect(), static_cast<agl::ShaderMode>(4));
}
}  // namespace al
