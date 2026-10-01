#include "Library/Execute/ExecutorListLayout.hpp"

#include <common/aglShaderEnum.h>
#include <gfx/seadGraphicsContext.h>

#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Layout/LayoutActor.hpp"
#include "Library/Layout/LayoutKeeper.hpp"

namespace agl {
class DrawContext;
}

namespace al {
void tryChangeShaderMode(agl::DrawContext* pContext, agl::ShaderMode mode);

/**
 * Constructs a layout draw executor list.
 * @param pListName List name.
 * @param capacity Maximum number of layouts.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListLayoutDrawBase::ExecutorListLayoutDrawBase(const char* pListName, s32 capacity,
                                                       const char* pGroupName,
                                                       const ExecuteSystemInitInfo& rInfo)
    : ExecutorListBase(pListName, pGroupName), mLayoutNumMax(capacity) {
    mLayouts = new LayoutActor*[capacity];

    for (s32 i = 0; i < mLayoutNumMax; i++) {
        mLayouts[i] = nullptr;
    }
}

/**
 * Adds a layout.
 * @param pLayout The layout.
 */
void ExecutorListLayoutDrawBase::registerLayout(LayoutActor* pLayout) {
    mLayouts[mLayoutNum] = pLayout;
    mLayoutNum++;
}

/**
 * Draws all alive layouts if any is alive.
 */
void ExecutorListLayoutDrawBase::executeList() const {
    bool isAnyAlive = false;

    for (s32 i = 0; i < mLayoutNum; i++) {
        isAnyAlive |= mLayouts[i]->isAlive();
    }

    if (mLayoutNum <= 0 || !isAnyAlive) {
        return;
    }

    startDraw();

    for (s32 i = 0; i < mLayoutNum; i++) {
        LayoutActor* layout = mLayouts[i];

        if (layout->isAlive()) {
            layout->getLayoutKeeper()->draw();
        }
    }
}

/**
 * Constructs a normal layout draw executor list.
 * @param pListName List name.
 * @param capacity Maximum number of layouts.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListLayoutDrawNormal::ExecutorListLayoutDrawNormal(const char* pListName, s32 capacity,
                                                           const char* pGroupName,
                                                           const ExecuteSystemInitInfo& rInfo)
    : ExecutorListLayoutDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Sets up the shader mode and graphics state for layout drawing.
 */
void ExecutorListLayoutDrawNormal::startDraw() const {
    tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(),
                        agl::cShaderMode_UniformRegister);
    sead::GraphicsContext context;
    context.setDepthEnable(false, false);
    context.setCullingMode(0);
    context.apply(GameFrameworkNx::getDrawContext());
}
}  // namespace al
