#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_ResSceneAnim.h>
#include <nn/g3d/g3d_ResCameraAnim.h>
#include <nn/g3d/g3d_ResLightAnim.h>
#include <nn/g3d/g3d_ResFogAnim.h>
#include <nn/gfx/gfx_Device.h>
#include <nn/gfx/gfx_MemoryPool.h>
#include <nn/gfx/gfx_CommandBuffer.h>
#include <nn/gfx/gfx_GpuAddress.h>
#include <new>

namespace nn::g3d {
static_assert(sizeof(ResMesh) == 0x38, "Mesh resource size");
static_assert(sizeof(nn::gfx::Buffer) == 0x48, "Buffer object size");

void ResSceneAnim::Release() {
    int count = mLightAnimCount;
    for (int i = 0; i < count; ++i) {
        ResLightAnim* anim = &mLightAnims[i];
        anim->lightFuncIndex = 0xff;
        anim->distanceFuncIndex = 0xff;
        anim->angleFuncIndex = 0xff;
    }
    count = mFogAnimCount;
    for (int i = 0; i < count; ++i) mFogAnims[i].fogFuncIndex = 0xff;
}
void ResSceneAnim::Reset() {
    int count = mCameraAnimCount;
    for (int i = 0; i < count; ++i)
        reinterpret_cast<ResCameraAnim*>(mCameraAnimOffset)[i].ResetCurves();
    count = mLightAnimCount;
    for (int i = 0; i < count; ++i) mLightAnims[i].ResetCurves();
    count = mFogAnimCount;
    for (int i = 0; i < count; ++i) mFogAnims[i].ResetCurves();
}
// device owns the buffers; the resource supplies its own memory pool and offset.
void ResVertex::Setup(nn::gfx::Device* device) {
    ptrdiff_t offset = memoryPoolOffset;
    nn::gfx::MemoryPool* pool = pMemoryPool.Get();
    int count = bufferCount;
    for (ptrdiff_t i = 0; i < count; ++i) {
        pVertexBufferArray.Get()[i] = &pBufferObjects.Get()[i];
        nn::gfx::Buffer** table = pVertexBufferArray.Get();
        nn::gfx::BufferInfo* info = reinterpret_cast<nn::gfx::BufferInfo*>(&pVertexBufferInfoArray.Get()[i]);
        nn::gfx::Buffer* buffer = table[i];
        new (buffer) nn::gfx::Buffer;
        info->SetGpuAccessFlags(4);
        buffer->Initialize(device, *info, pool, offset, info->GetSize());
        nn::gfx::util::SetBufferDebugLabel(buffer, "g3d_vertex");
        offset += (info->GetSize() + 7) & ~size_t(7);
    }
}
// device owns the buffers; pool/offset locate external backing storage.
void ResVertex::Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset) {
    int count = bufferCount;
    offset += memoryPoolOffset;
    for (int i = 0; i < count; ++i) {
        pVertexBufferArray.Get()[i] = &pBufferObjects.Get()[i];
        nn::gfx::BufferInfo* info = reinterpret_cast<nn::gfx::BufferInfo*>(&pVertexBufferInfoArray.Get()[i]);
        nn::gfx::Buffer* buffer = pVertexBufferArray.Get()[i];
        new (buffer) nn::gfx::Buffer;
        info->SetGpuAccessFlags(4);
        buffer->Initialize(device, *info, pool, offset, info->GetSize());
        nn::gfx::util::SetBufferDebugLabel(buffer, "g3d_vertex");
        offset += (info->GetSize() + 7) & ~size_t(7);
    }
}
// device owns each initialized vertex buffer that must be finalized.
void ResVertex::Cleanup(nn::gfx::Device* device) {
    int count = bufferCount;
    for (int i = 0; i < count; ++i) {
        pVertexBufferArray.Get()[i] = &pBufferObjects.Get()[i];
        nn::gfx::Buffer* buffer = pVertexBufferArray.Get()[i];
        if (buffer->ToData()->state) { buffer->Finalize(device); buffer->~TBuffer(); }
    }
}
void ResVertex::Reset() { Reset(0); }
// guard bit 0 preserves user data; bit 1 preserves dynamic attribute flags.
void ResVertex::Reset(u32 guard) {
    int count = bufferCount;
    for (int i = 0; i < count; ++i) {
        nn::gfx::Buffer* object = pBufferObjects.Get();
        nn::gfx::Buffer** table = pVertexBufferArray.Get();
        table[i] = object + i;
    }
    if (!(guard & 1)) pUserPtr.Clear();
    if (!(guard & 2)) {
        count = attribCount;
        for (int i = 0; i < count; ++i) pAttribArray.Get()[i].flags &= ~1;
    }
}
// indices receives one set bit for each buffer containing a dynamic vertex attribute.
void ResVertex::CalculateDynamicVertexBufferIndex(nn::util::BitFlagSet<255, void>* indices) const {
    indices->Reset();
    int count = attribCount;
    for (int i = 0; i < count; ++i) {
        const ResVertexAttribData* attrib = &pAttribArray.Get()[i];
        if (attrib->flags & 1) indices->Set(attrib->bufferIndex);
    }
}
// device owns index buffers; each mesh supplies its backing memory pool.
void ResShape::Setup(nn::gfx::Device* device) {
    if (flags & 8) return;
    int count = meshCount;
    for (int i = 0; i < count; ++i) pMeshArray.Get()[i].Setup(device);
}
// device owns this mesh's index buffer; use the memory pool stored in the resource.
void ResMesh::Setup(nn::gfx::Device* device) {
    nn::gfx::BufferInfo* info = bufferInfo;
    nn::gfx::MemoryPool* pool = memoryPool;
    nn::gfx::Buffer* object = buffer;
    new (object) nn::gfx::Buffer;
    info->SetGpuAccessFlags(8);
    object->Initialize(device, *info, pool, memoryPoolOffset, info->GetSize());
    nn::gfx::util::SetBufferDebugLabel(object, "g3d_index");
}
// device owns index buffers; pool/offset specify external backing storage.
void ResShape::Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset) {
    if (flags & 8) return;
    int count = meshCount;
    for (int i = 0; i < count; ++i) pMeshArray.Get()[i].Setup(device, pool, offset);
}
// device owns this mesh's index buffer; pool/offset specify its external backing storage.
void ResMesh::Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset) {
    nn::gfx::Buffer* object = buffer;
    nn::gfx::BufferInfo* info = bufferInfo;

    new (object) nn::gfx::Buffer;
    info->SetGpuAccessFlags(8);
    object->Initialize(device, *info, pool, memoryPoolOffset+ offset, info->GetSize());
    nn::gfx::util::SetBufferDebugLabel(object, "g3d_index");
}
// device owns the index buffers belonging to this shape's meshes.
void ResShape::Cleanup(nn::gfx::Device* device) {
    if (flags & 8) return;
    int count = meshCount;
    for (int i = 0; i < count; ++i) pMeshArray.Get()[i].Cleanup(device);
}
// device owns this mesh's initialized index buffer.
void ResMesh::Cleanup(nn::gfx::Device* device) {
    nn::gfx::Buffer* object = buffer;
    if (object->ToData()->state) { object->Finalize(device); object->~TBuffer(); }
}
void ResShape::Reset() { Reset(0); }
// guard bit 0 preserves user data while restoring embedded index buffer pointers.
void ResShape::Reset(u32 guard) {
    if (!(flags & 8)) {
        for (int i = 0; i < meshCount; ++i) {
            ResMesh* mesh = &pMeshArray.Get()[i];
            mesh->buffer = reinterpret_cast<nn::gfx::Buffer*>(mesh->subMeshes + mesh->subMeshCount);
        }
    }
    if (!(guard & 1)) pUserPtr.Clear();
}
void ResShape::ActivateDynamicVertexAttrForShapeAnim() {
    if (!keyShapeCount) return;
    const ResKeyShape* key = pKeyShapeArray.Get();
    ResVertex* vertex = pVertex.Get();
    for (int i = 0; i < 18; ++i) {
        int index = key->attribIndices[i];
        if (index) vertex->ToData().pAttribArray.Get()[index - 1].flags |= 1;
    }
}
// command receives count submeshes starting at first; instances controls repetition.
void ResMesh::DrawSubMesh(nn::gfx::CommandBuffer* command, int first, int count, int instances) const {
    ptrdiff_t offset = subMeshes[first].offset;
    const ResSubMesh* last = &subMeshes[first + count - 1];
    int indices = last->count + ((ptrdiff_t(last->offset) - offset) >> indexFormat);
    nn::gfx::GpuAddress address;
    address.ToData()->value = 0;
    address.ToData()->impl = 0;
    buffer->GetGpuAddress(&address);
    address.Offset(offset);
    command->DrawIndexed(topology, indexFormat, address, indices, baseVertex, instances, 0);
}
// command receives count submeshes starting at first; instances controls repetition; baseInstance offsets the instance index.
void ResMesh::DrawSubMesh(nn::gfx::CommandBuffer* command, int first, int count, int instances, int baseInstance) const {
    ptrdiff_t offset = subMeshes[first].offset;
    const ResSubMesh* last = &subMeshes[first + count - 1];
    int indices = last->count + ((ptrdiff_t(last->offset) - offset) >> indexFormat);
    nn::gfx::GpuAddress address;
    address.ToData()->value = 0;
    address.ToData()->impl = 0;
    buffer->GetGpuAddress(&address);
    address.Offset(offset);
    command->DrawIndexed(topology, indexFormat, address, indices, baseVertex, instances, baseInstance);
}
}
