#include <eui/euiDynamicCapturePane.h>
#include <nn/ui2d/ui2d_BuildArgSet.h>
#include <utility/aglDynamicTextureAllocator.h>
#include <utility/aglMultiFilter.h>

namespace eui {

// pResource supplies pane properties; rArgs identifies the owning layout and build context.
DynamicCapturePane::DynamicCapturePane(const nn::ui2d::ResPane* pResource,
    const nn::ui2d::BuildArgSet& rArgs)
    : Pane(pResource, rArgs), mFlags(0), m_pClearColor(nullptr), m_pMultiFilter(nullptr),
      m_pDynamicTexture(nullptr) {
    initialize_(reinterpret_cast<LayoutEx*>(rArgs.m_pPartsLayout));
}

// rOther supplies pane properties; pLayout owns the new capture resources.
DynamicCapturePane::DynamicCapturePane(const DynamicCapturePane& rOther, LayoutEx* pLayout)
    : Pane(rOther), mFlags(0), m_pClearColor(nullptr), m_pMultiFilter(nullptr),
      m_pDynamicTexture(nullptr) {
    initialize_(pLayout);
}

// Dispose of the separately allocated clear color and filter.
DynamicCapturePane::~DynamicCapturePane() {
    if (m_pClearColor) {
        delete m_pClearColor;
        m_pClearColor = nullptr;
    }

    if (m_pMultiFilter) {
        delete m_pMultiFilter;
        m_pMultiFilter = nullptr;
    }
}

// Return the current capture to its owning filter or dynamic texture allocator.
void DynamicCapturePane::freeDynamicTexture() {
    if (m_pMultiFilter && m_pMultiFilter->getResultTexture())
        m_pMultiFilter->freeResultTexture();
    else
        agl::utl::DynamicTextureAllocator::instance()->free(m_pDynamicTexture);
    m_pDynamicTexture = nullptr;
}

}  // namespace eui
