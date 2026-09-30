#include <eui/euiCapturePane.h>
#include <nn/ui2d/ui2d_BuildArgSet.h>
#include <nn/ui2d/ui2d_ExtUserData.h>
#include <utility/aglMultiFilter.h>
namespace eui {
// NON_MATCHING: branch relocation awaits initialize_ reconstruction.
// pResource supplies pane properties; rArgs supplies the owning layout and build context.
CapturePane::CapturePane(const nn::ui2d::ResPane* pResource, const nn::ui2d::BuildArgSet& rArgs)
    : Pane(pResource, rArgs), mFlags(0), _d3(true), _d4(0), mClearColor(nullptr),
      mMultiFilter(nullptr), mTexture(nullptr) {
    initialize_(reinterpret_cast<LayoutEx*>(rArgs.m_pPartsLayout));
}
CapturePane::~CapturePane() {
    if (mClearColor) { delete mClearColor; mClearColor = nullptr; }
    if (mMultiFilter) { delete mMultiFilter; mMultiFilter = nullptr; }
    if (mTextureMemory.isValid()) mTextureMemory.deleteGPUMemBlock();
    delete mTexture;
}
// pPane supplies the capture alpha setting; pFlags receives bit 3 when the output is opaque.
void CapturePane::setupCaptureOutputAlpha255_(nn::ui2d::Pane* pPane, sead::BitFlag<u8>* pFlags) {
    const auto* data = pPane->FindExtUserDataByName("CaptureOutputAlpha");
    if (data && *static_cast<const s32*>(data->GetData()) == 255) pFlags->set(8);
}
}
