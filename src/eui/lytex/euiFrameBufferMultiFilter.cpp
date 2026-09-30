#include <eui/euiFrameBufferMultiFilter.h>
#include <utility/aglMultiFilter.h>
#include <utility/aglDynamicTextureAllocator.h>
#include <eui/euiUtility.h>
#include <eui/euiPictureEx.h>
#include <eui/euiWindowEx.h>
#include <nn/ui2d/ui2d_Material.h>
#include <nn/ui2d/ui2d_ExtUserData.h>

namespace eui {

// Initialize the capture targets and enable the default filtering flags.
FrameBufferMultiFilter::FrameBufferMultiFilter()
    : m_pMultiFilter(nullptr), mCaptureWidth(0), mCaptureHeight(0), mFlags(5), _1F5(0),
      mTextureIndex(-1) {}

// Release the owned filter before destroying its render targets.
FrameBufferMultiFilter::~FrameBufferMultiFilter() {
    if (m_pMultiFilter) {
        delete m_pMultiFilter;
        m_pMultiFilter = nullptr;
    }
}

// pHeap owns the filter; rPane supplies capture options and material texture counts.
// pLayout provides the layout's filter resources.
void FrameBufferMultiFilter::initialize(sead::Heap* pHeap, const nn::ui2d::Pane& rPane,
                                       LayoutEx* pLayout) {
    const auto* resource = rPane.FindExtUserDataByName("FrameBufferUse");
    const auto* values = static_cast<const s32*>(resource->GetData());
    const u16 valueCount = resource->count;
    const s32 textureIndex = values[0];
    u32 textureCount = 0;
    const u8 materialCount = rPane.GetMaterialCount();

    for (int i = 0; i < materialCount; ++i) {
        const u32 count = rPane.GetMaterial(i)->mResourceCounts & 3;

        if (textureCount < count) textureCount = count;
    }

    if (textureIndex >= 0 && textureIndex < s32(textureCount)) {
        mTextureIndex = textureIndex;
        mCaptureWidth = valueCount > 1 ? values[1] * 2 : 0;
        mCaptureHeight = valueCount > 2 ? values[2] * 2 : 0;
        m_pMultiFilter = InitializeMultiFilter(pHeap, rPane, pLayout);
    }

    m_RenderBuffer.setRenderTargetColor(&m_RenderTarget);
    resource = rPane.FindExtUserDataByName("FrameBufferAlpha");

    if (resource) {
        const s32 alpha = *static_cast<const s32*>(resource->GetData());

        switch (alpha) {
        case 0: mFlags = 0; _1F5 = 0; break;
        case 255: mFlags = 1; _1F5 = 0; break;
        }
    }
}

// pPicture receives pTexture through the caller-owned pInfo wrapper.
// rDrawInfo is the drawing context, unused by this material-only operation.
void FrameBufferMultiFilter::applyTextureDataToPictureMaterial(PictureEx* pPicture,
    nn::ui2d::TextureInfo* pInfo, const agl::TextureData* pTexture,
    const DrawInfoEx& rDrawInfo) {
    SetupTextureInfoByAglTextureData(pInfo, *pTexture, nullptr);
    pPicture->GetMaterial(0)->SetTextureInfo(mTextureIndex, pInfo);
}

// pWindow receives pTexture in each material containing the selected texture slot.
// pInfo is the caller-owned texture wrapper; rDrawInfo is unused here.
void FrameBufferMultiFilter::applyTextureDataToWindowMaterial(WindowEx* pWindow,
    nn::ui2d::TextureInfo* pInfo, const agl::TextureData* pTexture,
    const DrawInfoEx& rDrawInfo) {
    SetupTextureInfoByAglTextureData(pInfo, *pTexture, nullptr);
    const u8 count = pWindow->GetMaterialCount();

    for (int i = 0; i < count; ++i) {
        auto* material = pWindow->GetMaterial(i);
        const int index = mTextureIndex;

        if (int(material->mResourceCounts & 3) > index)
            material->SetTextureInfo(index, pInfo);
    }
}

// pTexture is the captured texture to release when no filter result is owned.
void FrameBufferMultiFilter::freeResultTexture(const agl::TextureData* pTexture) {
    if (m_pMultiFilter && m_pMultiFilter->getResultTexture())
        m_pMultiFilter->freeResultTexture();
    else
        agl::utl::DynamicTextureAllocator::instance()->free(pTexture);
}

}  // namespace eui
