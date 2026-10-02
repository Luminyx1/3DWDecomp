#include "Library/Layout/LayoutUtil.hpp"

#include <common/aglRenderBuffer.h>
#include <gfx/seadViewport.h>

#include "Library/Effect/EffectSystem.hpp"
#include "Library/Execute/ExecuteDirector.hpp"
#include "Library/Layout/LayoutInitInfo.hpp"
#include "Library/Layout/LayoutKit.hpp"
#include "Library/Layout/LayoutSystem.hpp"

namespace al {
/**
 * Initializes layout init info from a layout kit.
 * @param pInfo info to initialize
 * @param pKit layout kit
 * @param pSceneObjHolder scene object holder
 * @param pAudioDirector audio director
 * @param pLayoutSystem layout system
 * @param pMessageSystem message system
 * @param pGamePadSystem game pad system
 */
void initLayoutInitInfo(LayoutInitInfo* pInfo, const LayoutKit* pKit,
                        SceneObjHolder* pSceneObjHolder, const AudioDirector* pAudioDirector,
                        const LayoutSystem* pLayoutSystem, const MessageSystem* pMessageSystem,
                        const GamePadSystem* pGamePadSystem) {
    pInfo->init(pKit->getExecuteDirector(), pKit->getEffectSystem()->getEffectSystemInfo(),
                pSceneObjHolder, pAudioDirector, nullptr, nullptr, pLayoutSystem, pMessageSystem,
                pGamePadSystem, nullptr);
    pInfo->setDrawContext(pKit->getDrawContext());
    pInfo->setDrawInfo(pKit->getDrawInfo());
}

/**
 * Sets the render buffer layouts are drawn to.
 * @param pKit layout kit
 * @param pRenderBuffer render buffer
 */
void setRenderBuffer(LayoutKit* pKit, const agl::RenderBuffer* pRenderBuffer) {
    LayoutRenderInfo* renderInfo = getLayoutRenderInfo(pKit->getDrawInfo());
    renderInfo->renderBuffer = pRenderBuffer;
    renderInfo->viewport->setByFrameBuffer(*pRenderBuffer);
}

/**
 * Updates the layout kit.
 * @param pKit layout kit
 */
void executeUpdate(LayoutKit* pKit) {
    pKit->update();
}

/**
 * Executes a single update list.
 * @param pKit layout kit
 * @param pTableName update table name
 * @param pListName update list name
 */
void executeUpdateList(LayoutKit* pKit, const char* pTableName, const char* pListName) {
    pKit->getExecuteDirector()->executeList(pListName);
}

/**
 * Updates the 2D effects.
 * @param pKit layout kit
 */
void executeUpdateEffect(LayoutKit* pKit) {
    alEffectSystemFunction::updateEffect2D(pKit->getEffectSystem());
}

/**
 * Draws a draw table.
 * @param pKit layout kit
 * @param pTableName draw table name
 */
void executeDraw(const LayoutKit* pKit, const char* pTableName) {
    pKit->getExecuteDirector()->draw(pTableName);
}

/**
 * Draws the 2D effects.
 * @param pKit layout kit
 */
void executeDrawEffect(const LayoutKit* pKit) {
    alEffectSystemFunction::drawEffect2D(pKit->getEffectSystem(), nullptr);
}
}  // namespace al
