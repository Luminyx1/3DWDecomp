#include <nn/ui2d/ui2d_ResourceAccessor.h>
namespace nn::ui2d {
// type and pName select a resource; this overload does not request its size.
void* ResourceAccessor::FindResourceByName(u32 type, const char* pName) {
    return FindResourceByName(nullptr, type, pName);
}
// pSize receives the resource size; type and pName select the resource through the mutable implementation.
const void* ResourceAccessor::FindResourceByName(size_t* pSize, u32 type, const char* pName) const {
    return const_cast<ResourceAccessor*>(this)->FindResourceByName(pSize, type, pName);
}
// type and pName select a resource; this overload does not request its size.
const void* ResourceAccessor::FindResourceByName(u32 type, const char* pName) const {
    return FindResourceByName(nullptr, type, pName);
}
// pName is the requested render target name; the base accessor does not provide render targets.
TextureInfo* ResourceAccessor::RegisterRenderTargetTexture(const char* pName) { return nullptr; }
// pTexture is unused because the base accessor does not own render targets.
void ResourceAccessor::UnregisterRenderTargetTexture(TextureInfo* pTexture) {}
}
