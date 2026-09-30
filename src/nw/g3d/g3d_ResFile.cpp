#include <nn/g3d/g3d_ResFile.h>
#include <nn/g3d/g3d_ResSkeletalAnim.h>
#include <nn/g3d/g3d_ResShapeAnim.h>
#include <nn/g3d/g3d_ResSceneAnim.h>
#include <nn/gfx/gfx_Device.h>
#include <nn/gfx/gfx_MemoryPool.h>
#include <nn/gfx/gfx_MemoryPoolInfo.h>
#include <nn/util.h>
#include <new>

namespace nn::g3d {
static_assert(sizeof(ResSkeletalAnim) == 0x50, "Skeletal animation resource size");
static_assert(sizeof(ResMaterialAnim) == 0x70, "Material animation resource size");
static_assert(sizeof(ResShapeAnim) == 0x50, "Shape animation resource size");
static_assert(sizeof(ResSceneAnim) == 0x60, "Scene animation resource size");
// file points to a binary resource header; validate its signature and format version.
bool ResFile::IsValid(const void* file) {
    return static_cast<const nn::util::BinaryFileHeader*>(file)->IsValid(0x2020202053455246, 9, 0, 0);
}
void ResFile::Relocate() {
    if (!fileHeader.IsRelocated()) fileHeader.GetRelocationTable()->Relocate();
}
void ResFile::Unrelocate() {
    if (fileHeader.IsRelocated()) fileHeader.GetRelocationTable()->Unrelocate();
}
// file supplies mutable resource storage; relocate its pointers before returning the typed view.
ResFile* ResFile::ResCast(void* file) {
    ResFile* resource = static_cast<ResFile*>(file);
    resource->Relocate();
    resource->fileHeader.IsEndianReverse();
    return resource;
}
// callback resolves texture names; user is passed through to model and animation bindings.
BindResult ResFile::BindTexture(TextureBindCallback callback, void* user) {
    BindResult result;
    int count = modelCount;
    for (int i = 0; i < count; ++i) result.Merge(pModelArray.Get()[i].BindTexture(callback, user));
    count = materialAnimCount;
    for (int i = 0; i < count; ++i) result.Merge(pMaterialAnimArray.Get()[i].BindTexture(callback, user));
    return result;
}
void ResFile::ReleaseTexture() {
    int count = modelCount;
    for (int i = 0; i < count; ++i) pModelArray.Get()[i].ReleaseTexture();
    count = materialAnimCount;
    for (int i = 0; i < count; ++i) pMaterialAnimArray.Get()[i].ReleaseTexture();
}
// device owns model graphics objects and the optional memory pool embedded in the file.
void ResFile::Setup(nn::gfx::Device* device) {
    nn::util::ReferSymbol("SDK MW+Nintendo+NintendoWare_G3d-10_4_0-Release");
    nn::gfx::MemoryPool* pool = pMemoryPool.Get();
    if (pool != nullptr) {
        const nn::gfx::MemoryPoolInfo* info = pMemoryPoolInfo.Get();
        if (info != nullptr) {
            new (pool) nn::gfx::MemoryPool;
            pool->Initialize(device, *info);
            nn::gfx::util::SetMemoryPoolDebugLabel(pool, "g3d");
        }
    }
    int count = modelCount;
    for (int i = 0; i < count; ++i) pModelArray.Get()[i].Setup(device);
}
// device owns graphics objects; pool/offset locate external backing storage. size is unchecked here.
void ResFile::Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset, size_t size) {
    nn::util::ReferSymbol("SDK MW+Nintendo+NintendoWare_G3d-10_4_0-Release");
    if ((pMemoryPool.Get() != nullptr) && (pMemoryPoolInfo.Get() != nullptr))
        offset += static_cast<const u8*>(pMemoryPoolInfo.Get()->GetPoolMemory()) - reinterpret_cast<const u8*>(this);
    int count = modelCount;
    for (int i = 0; i < count; ++i) pModelArray.Get()[i].Setup(device, pool, offset);
}
// device owns the graphics objects to release, including an initialized embedded memory pool.
void ResFile::Cleanup(nn::gfx::Device* device) {
    int count = modelCount;
    for (int i = 0; i < count; ++i) pModelArray.Get()[i].Cleanup(device);
    if (pMemoryPoolInfo.Get() != nullptr) {
        nn::gfx::MemoryPool* pool = pMemoryPool.Get();
        if (pool->ToData()->state) {
            pool->Finalize(device);
            pool->~TMemoryPool();
        }
    }
}
void ResFile::Reset() {
    int count = modelCount;
    for (int i = 0; i < count; ++i) pModelArray.Get()[i].Reset();
    count = skeletalAnimCount;
    for (int i = 0; i < count; ++i) pSkeletalAnimArray.Get()[i].Reset();
    count = materialAnimCount;
    for (int i = 0; i < count; ++i) pMaterialAnimArray.Get()[i].Reset();
    count = shapeAnimCount;
    for (int i = 0; i < count; ++i) pShapeAnimArray.Get()[i].Reset();
    count = sceneAnimCount;
    for (int i = 0; i < count; ++i) pSceneAnimArray.Get()[i].Reset();
    pUserPtr.Clear();
}
}
