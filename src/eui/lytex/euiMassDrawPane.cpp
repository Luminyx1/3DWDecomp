#include <eui/euiMassDrawPane.h>
#include <eui/euiUtility.h>
#include <nn/ui2d/ui2d_Material.h>

namespace eui {

namespace {
// rDestination receives the common prefix of rSource without changing its allocation.
template <typename T>
void copyBuffer(const sead::Buffer<T>& rSource, sead::Buffer<T>& rDestination) {
    if (&rDestination == &rSource) return;
    const int count = rSource.size() < rDestination.size() ? rSource.size() : rDestination.size();
    const T* source = rSource.getBufferPtr();
    T* destination = rDestination.getBufferPtr();
    for (int i = 0; i < count; ++i) destination[i] = source[i];
}
}

// textureCount is the number of texture slots in the picture material.
MassDrawPane::MassDrawPane(u8 textureCount) : PictureEx(textureCount) {}

// rTexture initializes the shared texture used by the repeated pictures.
MassDrawPane::MassDrawPane(const nn::ui2d::TextureInfo& rTexture) : PictureEx(rTexture) {}

// pResource contains the picture; pOverride supplies resource overrides;
// rArgs provides the layout build context.
MassDrawPane::MassDrawPane(const nn::ui2d::ResPicture* pResource,
    const nn::ui2d::ResPicture* pOverride, const nn::ui2d::BuildArgSet& rArgs)
    : PictureEx(pResource, pOverride, rArgs) {}

MassDrawPane::~MassDrawPane() = default;

// rOther supplies the picture and instance buffers to duplicate into the layout heap.
MassDrawPane::MassDrawPane(const MassDrawPane& rOther) : PictureEx(rOther) {
    if (rOther.mAlphas.getBufferPtr()) {
        initialize(GetNwAllocatorHeap(), rOther.mAlphas.size());
        copyBuffer(rOther.mAlphas, mAlphas);
        copyBuffer(rOther.mIndices, mIndices);
        copyBuffer(rOther.mPositions, mPositions);
    }
}

// pHeap owns the instance buffers; count is the number of repeated pictures.
void MassDrawPane::initialize(sead::Heap* pHeap, int count) {
    mAlphas.tryAllocBuffer(count, pHeap);
    mAlphas.fill(0);
    mIndices.tryAllocBuffer(count, pHeap);
    mIndices.fill(0);
    mPositions.tryAllocBuffer(count, pHeap);
    mPositions.fill(sead::Vector2f::zero);
}

// rDrawInfo and rContext provide calculation state; force forwards the update request.
void MassDrawPane::Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext,
                            bool force) {
    Pane::Calculate(rDrawInfo, rContext, force);
}

// Convert the first material's texture wrapper into the agl texture description.
void MassDrawPane::initializeTextureData_() {
    const auto* textureInfo = Pane::GetMaterial()->GetFirstTexMap()[0].m_pTextureInfo;
    SetupAglTextureDataByTextureInfo(&mTexture, *textureInfo);
}

}  // namespace eui
