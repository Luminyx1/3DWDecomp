#include <nn/ui2d/ui2d_Material.h>
#include <nn/ui2d/ui2d_AnimTransform.h>
namespace nn::ui2d {
Material::Material() { Initialize(); }
void Material::Initialize() {
    mBlackColor = 0;
    mWhiteColor = 0xffffffff;
    mResourceCapacity &= 0xffc00000;
    mResourceCounts &= 0xffc00000;
    mUserShaderConstantBufferInformation = nullptr;
    mDetailedCombiner = nullptr;
    mName = nullptr;
    mUserShader = nullptr;
    m_pTexMaps = nullptr;
    mShaderInfo = nullptr;
    mFlags = 0xe0;
}
size_t Material::GetVertexShaderConstantBufferSize() const {
    return mUserShaderConstantBufferInformation ? size_t(mUserShaderConstantBufferInformation->vertexSize) + 0x230 : 0x230;
}
size_t Material::GetPixelShaderConstantBufferSize() const {
    return mUserShaderConstantBufferInformation ? size_t(mUserShaderConstantBufferInformation->pixelSize) + 0x90 : 0x90;
}
size_t Material::GetGeometryShaderConstantBufferSize() const {
    return mUserShaderConstantBufferInformation ? mUserShaderConstantBufferInformation->geometrySize : 0;
}
size_t Material::GetPixelShaderDetailedCombinerConstantBufferSize() const { return 0xe0; }
size_t Material::GetPixelShaderCombinerUserShaderConstantBufferSize() const { return 0x220; }
bool Material::IsUseFramebufferTexture() const { return (mResourceCounts >> 17) & 1; }
// animation receives this material as a binding target.
void Material::BindAnimation(AnimTransform* animation) { animation->BindMaterial(this); }
// animation removes this material from its binding targets.
void Material::UnbindAnimation(AnimTransform* animation) { animation->UnbindMaterial(this); }
}
