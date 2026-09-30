#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_ResSkeleton.h>

namespace nn::g3d {
static_assert(sizeof(ResMaterial) == 0xa8, "Material resource size");
static_assert(sizeof(ResShape) == 0x60, "Shape resource size");
static_assert(sizeof(ResVertex) == 0x58, "Vertex resource size");
static_assert(sizeof(ResModel) == 0x78, "Model resource size");
// callback resolves each texture name; user is passed through to the callback.
BindResult ResModel::BindTexture(TextureBindCallback callback, void* user) {
    BindResult result;
    int count = materialCount;
    for (int i = 0; i < count; ++i) result.Merge(pMaterialArray.Get()[i].BindTexture(callback, user));
    return result;
}

// texture replaces bindings with the requested name; return whether any material was updated.
bool ResModel::ForceBindTexture(const TextureRef& texture, const char* name) {
    bool result = false;
    int count = materialCount;
    for (int i = 0; i < count; ++i) result |= pMaterialArray.Get()[i].ForceBindTexture(texture, name);
    return result;
}

void ResModel::ReleaseTexture() {
    int count = materialCount;
    for (int i = 0; i < count; ++i) pMaterialArray.Get()[i].ReleaseTexture();
}

void ResModel::ActivateDynamicVertexAttrForShapeAnim() {
    int count = shapeCount;
    for (int i = 0; i < count; ++i) pShapeArray.Get()[i].ActivateDynamicVertexAttrForShapeAnim();
}

// device owns the graphics objects created for this model's materials, shapes, and vertices.
void ResModel::Setup(nn::gfx::Device* device) {
    int count = materialCount;
    for (int i = 0; i < count; ++i) pMaterialArray.Get()[i].Setup(device);
    count = shapeCount;
    for (int i = 0; i < count; ++i) pShapeArray.Get()[i].Setup(device);
    count = vertexCount;
    for (int i = 0; i < count; ++i) pVertexArray.Get()[i].Setup(device);
}

// device owns graphics objects; pool and offset locate the backing geometry storage.
void ResModel::Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset) {
    int count = materialCount;
    for (int i = 0; i < count; ++i) pMaterialArray.Get()[i].Setup(device);
    count = shapeCount;
    for (int i = 0; i < count; ++i) pShapeArray.Get()[i].Setup(device, pool, offset);
    count = vertexCount;
    for (int i = 0; i < count; ++i) pVertexArray.Get()[i].Setup(device, pool, offset);
}

// device owns the graphics objects being released.
void ResModel::Cleanup(nn::gfx::Device* device) {
    int count = materialCount;
    for (int i = 0; i < count; ++i) pMaterialArray.Get()[i].Cleanup(device);
    count = shapeCount;
    for (int i = 0; i < count; ++i) pShapeArray.Get()[i].Cleanup(device);
    count = vertexCount;
    for (int i = 0; i < count; ++i) pVertexArray.Get()[i].Cleanup(device);
}

void ResModel::Reset() { Reset(0); }
// guard is forwarded to child resources; bit zero also preserves this model's user pointer.
void ResModel::Reset(u32 guard) {
    int count = materialCount;
    for (int i = 0; i < count; ++i) pMaterialArray.Get()[i].Reset(guard);
    count = shapeCount;
    for (int i = 0; i < count; ++i) pShapeArray.Get()[i].Reset(guard);
    count = vertexCount;
    for (int i = 0; i < count; ++i) pVertexArray.Get()[i].Reset(guard);
    pSkeleton.Get()->Reset(guard);
    if (!(guard & 1)) pUserPtr.Clear();
}
}
