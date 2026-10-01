#include <nn/ui2d/ui2d_PaneEffect.h>
#include <nn/ui2d/ui2d_TextureInfo.h>
#include <cstring>
namespace nn::ui2d::detail {
PaneEffect::PaneEffect() : mPane(nullptr) { std::memset(&mParameters, 0, 0x221); }
// map identifies a texture whose dynamic-rendering instance is returned, if present.
void* PaneEffect::GetPrivateTexturePtr(const TexMap& map) {
    return (map.m_pTextureInfo != nullptr) ? map.m_pTextureInfo->GetPrivateTextureInstancePtr() : nullptr;
}
}
