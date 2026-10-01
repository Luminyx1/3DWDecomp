#include "Library/Layout/LayoutSystem.hpp"

#include <eui/euiScreenMgr.h>

#include "Library/Effect/EffectSystem.hpp"
#include "Library/Execute/ExecuteDirector.hpp"
#include "Library/Layout/LayoutKit.hpp"
#include "Project/Execute/ExecuteSystemInitInfo.hpp"

namespace al {
/**
 * Creates a layout kit.
 * @param pFontHolder font holder
 */
LayoutKit::LayoutKit(FontHolder* pFontHolder) : mFontHolder(pFontHolder) {}

/**
 * Destroys the layout kit and its execute director.
 */
LayoutKit::~LayoutKit() {
    if (mLayoutSystem != nullptr) {
        mLayoutSystem->getFontList()->isInvalid = true;
    }

    delete mExecuteDirector;
}

/**
 * Does nothing.
 */
void LayoutKit::createCameraParamForIcon() {}

/**
 * Creates the execute director.
 * @param requestCount maximum number of execute requests
 */
void LayoutKit::createExecuteDirector(s32 requestCount) {
    ExecuteSystemInitInfo info;
    mExecuteDirector = new ExecuteDirector(requestCount, false);
    mExecuteDirector->init(info);
}

/**
 * Creates the 2D effect system.
 */
void LayoutKit::createEffectSystem() {
    mEffectSystem = EffectSystem::initializeSystem(mDrawContext, nullptr, false);
}

/**
 * Finishes initialization by creating the executor list table.
 */
void LayoutKit::endInit() {
    if (mExecuteDirector != nullptr) {
        mExecuteDirector->createExecutorListTable();
    }
}

/**
 * Executes the layout update lists and updates the screens.
 */
void LayoutKit::update() {
    if (mExecuteDirector != nullptr) {
        mExecuteDirector->execute();
    }

    mLayoutSystem->getScreenMgr()->updateSystem();
    mLayoutSystem->getScreenMgr()->updateScreen(0);
}

/**
 * Sets the frame buffer to draw layouts to.
 * @param pRenderBuffer render buffer
 * @param pViewport viewport
 */
void LayoutKit::setFrameBuffer(const agl::RenderBuffer* pRenderBuffer,
                               const sead::Viewport* pViewport) {
    mRenderInfo->renderBuffer = pRenderBuffer;
    mRenderInfo->viewport = const_cast<sead::Viewport*>(pViewport);
}

/**
 * Draws a draw table.
 * @param pTableName draw table name
 */
void LayoutKit::draw(const char* pTableName) const {
    if (mExecuteDirector != nullptr) {
        mExecuteDirector->draw(pTableName);
    }
}

/**
 * Draws a single list of a draw table.
 * @param pTableName draw table name
 * @param pListName draw list name
 */
void LayoutKit::drawList(const char* pTableName, const char* pListName) const {
    mExecuteDirector->drawList(pTableName, pListName);
}

/**
 * Sets the draw context used for layouts and effects.
 * @param pDrawContext draw context
 */
void LayoutKit::setDrawContext(agl::DrawContext* pDrawContext) {
    mDrawContext = pDrawContext;
    getLayoutRenderInfo(mDrawInfo)->drawContext = pDrawContext;
}

/**
 * Creates empty execute system init info.
 */
ExecuteSystemInitInfo::ExecuteSystemInitInfo() {}
}  // namespace al
