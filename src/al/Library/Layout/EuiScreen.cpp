#include "Library/Layout/EuiScreen.hpp"

#include <eui/euiConstantBuffer.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiMultiArcResourceAccessor.h>
#include <eui/euiNwAllocator.h>
#include <eui/euiScreenMgr.h>
#include <eui/euiUtility.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <nn/font/font_TagProcessorBase.h>
#include <nn/ui2d/ui2d_DrawInfo.h>
#include <nn/ui2d/ui2d_Layout.h>

#include "Library/Layout/LayoutInitFunction.hpp"
#include "Library/Memory/Util.hpp"

namespace al {
/**
 * Creates a screen using the current heap and a new layout.
 */
EuiScreen::EuiScreen() {
    mHeap = getCurrentHeap();
    mLayout = new eui::LayoutEx(this);
}

/**
 * Finalizes and destroys the layout and its resources.
 */
EuiScreen::~EuiScreen() {
    if (mLayout == nullptr) {
        return;
    }

    nn::ui2d::ResourceAccessor* accessor = mLayout->GetResourceAccessor();
    eui::NwAllocator::initialize(mHeap);
    mLayout->Finalize(
        reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice()));
    delete mLayout;
    mLayout = nullptr;

    if (accessor == nullptr) {
        return;
    }

    accessor->UnregisterTextureViewFromDescriptorPool(eui::UnregisterSlotForTexture, nullptr);
    eui::NwAllocator::initialize(mHeap);
    auto* resource = eui::DynamicCast<eui::MultiArcResourceAccessor>(accessor);

    if (resource != nullptr) {
        resource->Finalize(
            reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice()));
    }

    eui::NwAllocator::finalize();
}

/**
 * Sets up the draw info with the screen's layout rectangle.
 */
void EuiScreen::doSetupDrawInfo_() {
    mScreenMgr->getConstantBuffer()->setToDrawInfo(mDrawInfo);
    initDrawInfo(mDrawInfo, &mScreenMgr->mGraphicsResource, mLayout->GetLayoutRect());
}
}  // namespace al
