#include <nn/ui2d/ui2d_TexMap.h>

namespace nn::ui2d {

TexMap::TexMap() : m_pTextureInfo(nullptr) {
    ResetSamplerSettings();
    ResetTextureInfoState();
}

// pTextureInfo supplies a non-owning texture reference; null leaves the map unbound.
TexMap::TexMap(const TextureInfo* pTextureInfo) : m_pTextureInfo(nullptr) {
    Set(pTextureInfo);
    ResetTextureInfoState();
}

TexMap::~TexMap() = default;
void TexMap::Finalize() {}

void TexMap::ResetSamplerSettings() {
    SetWrapMode(TexWrap_Clamp, TexWrap_Clamp);
    SetFilter(TexFilter_Linear, TexFilter_Linear);
}

void TexMap::ResetTextureInfoState() { mTextureInfoState = 0; }

// pTextureInfo replaces the texture and resets sampling; null leaves the map unchanged.
void TexMap::Set(const TextureInfo* pTextureInfo) {
    if (pTextureInfo != nullptr) {
        m_pTextureInfo = pTextureInfo;
        ResetSamplerSettings();
    }
}

// wrapS and wrapT select wrapping along the horizontal and vertical texture axes.
void TexMap::SetWrapMode(TexWrap wrapS, TexWrap wrapT) {
    mWrapS = wrapS;
    mWrapT = wrapT;
}

// minFilter selects minification filtering; magFilter selects magnification filtering.
void TexMap::SetFilter(TexFilter minFilter, TexFilter magFilter) {
    mMinFilter = minFilter;
    mMagFilter = magFilter;
}

// rOther supplies sampling modes; the texture reference and texture state are preserved.
void TexMap::CopySamplerSettings(const TexMap& rOther) {
    mWrapS = rOther.mWrapS;
    mWrapT = rOther.mWrapT;
    mMinFilter = rOther.mMinFilter;
    mMagFilter = rOther.mMagFilter;
}

}  // namespace nn::ui2d
