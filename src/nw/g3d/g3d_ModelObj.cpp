#include <nn/g3d/g3d_ModelObj.h>
#include <algorithm>
#include <cstring>
#include <new>

namespace nn::g3d {
namespace {
// value is the current byte count; alignment is the next block's power-of-two requirement.
inline size_t AlignBlock(size_t value, size_t alignment) { return (value + alignment - 1) & -alignment; }
} // namespace

namespace {
// block receives its offset; size and alignment accumulate requirements using blockAlignment.
inline void AppendWorkspaceBlock(detail::WorkMemoryBlock& block, size_t& size, size_t& alignment,
                                 size_t blockAlignment) {
    if (block.size != 0) {
        size_t start = AlignBlock(size, blockAlignment);
        alignment = std::max(alignment, blockAlignment);
        size = start + block.size;
        block.offset = start;
    }
}

// argument receives the default workspace descriptors used before sizing its resource.
template <class Argument> inline void InitializeWorkspace(Argument& argument) {
    for (auto& block : argument.blocks)
        block.Initialize(0);
    argument.memorySize = 0;
    argument.memoryAlignment = 0;
}

// resource identifies the shape whose default view and bounding settings are initialized.
inline ShapeObj::InitializeArgument MakeShapeArgument(const ResShape* resource) {
    ShapeObj::InitializeArgument argument;
    argument.resource = resource;
    argument.viewCount = 1;
    argument.viewDependent = false;
    argument.boundingEnabled = false;
    argument.userArea = nullptr;
    InitializeWorkspace(argument);
    return argument;
}

// output receives first * second, composing the affine row-vector transforms.
inline void MultiplyWorld(nn::util::Matrix4x3fType* output, const nn::util::Matrix4x3fType& first,
                          const nn::util::Matrix4x3fType& second) {
    const float32x4_t a0 = first._m.val[0], a1 = first._m.val[1];
    const float32x4_t a2 = first._m.val[2], a3 = first._m.val[3];
    const float32x4_t b0 = second._m.val[0], b1 = second._m.val[1];
    const float32x4_t b2 = second._m.val[2], b3 = second._m.val[3];
    float32x4_t r0 = vmulq_laneq_f32(b0, a0, 0);
    float32x4_t r1 = vmulq_laneq_f32(b0, a1, 0);
    float32x4_t r2 = vmulq_laneq_f32(b0, a2, 0);
    float32x4_t r3 = vmulq_laneq_f32(b0, a3, 0);
    r0 = vfmaq_laneq_f32(r0, b1, a0, 1);
    r1 = vfmaq_laneq_f32(r1, b1, a1, 1);
    r2 = vfmaq_laneq_f32(r2, b1, a2, 1);
    r3 = vfmaq_laneq_f32(r3, b1, a3, 1);
    r0 = vfmaq_laneq_f32(r0, b2, a0, 2);
    r1 = vfmaq_laneq_f32(r1, b2, a1, 2);
    r2 = vfmaq_laneq_f32(r2, b2, a2, 2);
    r3 = vfmaq_laneq_f32(r3, b2, a3, 2);
    output->_m.val[0] = r0;
    output->_m.val[1] = r1;
    output->_m.val[2] = r2;
    output->_m.val[3] = vaddq_f32(b3, r3);
}
} // namespace

void ModelObj::InitializeArgument::CalculateMemorySize() {
    const ResModel* model = resource;
    const ResSkeleton* skeletonResource = model->GetSkeleton();
    size_t skeletonSize = 0;
    if (skeleton == nullptr) {
        SkeletonObj::InitializeArgument argument;
        argument.resource = skeletonResource;
        InitializeWorkspace(argument);
        argument.bufferCount = skeletonBufferCount;
        argument.CalculateMemorySize();
        skeletonSize = AlignBlock(argument.memorySize, 16);
    }
    int shapeCount = model->GetShapeCount();
    int materialCount = model->GetMaterialCount();
    int boneCount = skeletonResource->GetBoneCount();
    int shapeViewCount = viewCount;
    size_t shapeSize = 0;
    for (int i = 0; i < shapeCount; ++i) {
        ShapeObj::InitializeArgument argument = MakeShapeArgument(model->GetShape(i));
        argument.bufferCount = shapeBufferCount;
        argument.viewDependent =
            skeletonResource->GetBone(argument.resource->GetBoneIndex())->GetBillboardMode() != 0;
        argument.viewCount = shapeViewCount;
        argument.boundingEnabled = boundingEnabled;
        argument.userArea = shapeUserArea;
        argument.CalculateMemorySize();
        shapeSize += AlignBlock(argument.memorySize, 16);
    }
    size_t materialSize = 0;
    for (int i = 0; i < materialCount; ++i) {
        MaterialObj::InitializeArgument argument;
        argument.resource = &model->ToData().pMaterialArray.Get()[i];
        InitializeWorkspace(argument);
        argument.bufferCount = materialBufferCount;
        argument.CalculateMemorySize();
        materialSize += AlignBlock(argument.memorySize, 8);
    }
    blocks[0].Initialize(skeletonSize);
    blocks[0].alignment = 16;
    blocks[1].Initialize(shapeSize);
    blocks[1].alignment = 16;
    blocks[2].Initialize(materialSize);
    blocks[3].Initialize(sizeof(SkeletonObj));
    blocks[4].Initialize(sizeof(ShapeObj) * shapeCount);
    blocks[5].Initialize(sizeof(MaterialObj) * materialCount);
    blocks[6].Initialize(((boneCount + 31) >> 5) * 4);
    blocks[7].Initialize(((materialCount + 31) >> 5) * 4);
    blocks[8].Initialize(0);
    blocks[8].alignment = 16;
    blocks[8].size = boundingEnabled ? lodCount * sizeof(Sphere) : 0;
    memorySize = 0;
    memoryAlignment = 8;
    AppendWorkspaceBlock(blocks[0], memorySize, memoryAlignment, 16);
    AppendWorkspaceBlock(blocks[1], memorySize, memoryAlignment, 16);
    AppendWorkspaceBlock(blocks[2], memorySize, memoryAlignment, 8);
    AppendWorkspaceBlock(blocks[3], memorySize, memoryAlignment, 8);
    AppendWorkspaceBlock(blocks[4], memorySize, memoryAlignment, 8);
    AppendWorkspaceBlock(blocks[5], memorySize, memoryAlignment, 8);
    AppendWorkspaceBlock(blocks[6], memorySize, memoryAlignment, 8);
    AppendWorkspaceBlock(blocks[7], memorySize, memoryAlignment, 8);
    AppendWorkspaceBlock(blocks[8], memorySize, memoryAlignment, 16);
}

// argument selects resources and buffering; buffer supplies bufferSize bytes of working memory.
bool ModelObj::Initialize(const InitializeArgument& argument, void* buffer, size_t bufferSize) {
    if (argument.memoryAlignment == 0 || argument.memorySize > bufferSize)
        return false;
    const ResModel* model = argument.resource;
    const ResSkeleton* skeletonResource = model->GetSkeleton();
    int shapeCount = model->GetShapeCount();
    int materialCount = model->GetMaterialCount();
    int viewCount = argument.viewCount;
    m_ResModel = model;
    m_BoneVisibility = argument.blocks[6].GetPointer<u32>(buffer);
    m_MaterialVisibility = argument.blocks[7].GetPointer<u32>(buffer);
    _18 = viewCount;
    m_Flag = 0;
    m_NumShapes = shapeCount;
    m_NumMaterials = materialCount;
    m_Skeleton = nullptr;
    m_Shapes = argument.blocks[4].GetPointer<ShapeObj>(buffer);
    m_Materials = argument.blocks[5].GetPointer<MaterialObj>(buffer);
    m_pBounding = argument.blocks[8].GetPointer<Sphere>(buffer);
    m_UserData = nullptr;
    _60 = buffer;
    m_MemoryPool = nullptr;
    m_MemoryPoolOffset = 0;
    _8c = argument.lodCount;
    if (argument.skeleton != nullptr) {
        m_Skeleton = argument.skeleton;
    } else {
        SkeletonObj::InitializeArgument skeletonArgument;
        skeletonArgument.resource = skeletonResource;
        InitializeWorkspace(skeletonArgument);
        skeletonArgument.bufferCount = argument.skeletonBufferCount;
        skeletonArgument.CalculateMemorySize();
        size_t size = skeletonArgument.memorySize;
        void* skeletonObject = argument.blocks[3].GetPointer(buffer);
        void* skeletonMemory = argument.blocks[0].GetPointer(buffer);
        m_Skeleton = new (skeletonObject) SkeletonObj();
        m_Skeleton->Initialize(skeletonArgument, skeletonMemory, size);
    }
    u8* shapeMemory = argument.blocks[1].GetPointer<u8>(buffer);
    int viewDependent = 0;
    for (int i = 0; i < shapeCount; ++i) {
        ShapeObj::InitializeArgument shapeArgument = MakeShapeArgument(model->GetShape(i));
        shapeArgument.bufferCount = argument.shapeBufferCount;
        bool dependent =
            skeletonResource->GetBone(shapeArgument.resource->GetBoneIndex())->GetBillboardMode() != 0;
        if (dependent)
            viewDependent = 1;
        shapeArgument.viewDependent = dependent;
        shapeArgument.viewCount = viewCount;
        shapeArgument.boundingEnabled = argument.boundingEnabled;
        shapeArgument.userArea = argument.shapeUserArea;
        shapeArgument.CalculateMemorySize();
        size_t size = AlignBlock(shapeArgument.memorySize, 16);
        ShapeObj* shape = new (&m_Shapes[i]) ShapeObj();
        shape->Initialize(shapeArgument, shapeMemory, size);
        shapeMemory += size;
    }
    m_ViewDependent = viewDependent;
    u8* materialMemory = argument.blocks[2].GetPointer<u8>(buffer);
    for (int i = 0; i < materialCount; ++i) {
        MaterialObj::InitializeArgument materialArgument;
        materialArgument.resource = &model->ToData().pMaterialArray.Get()[i];
        InitializeWorkspace(materialArgument);
        materialArgument.bufferCount = argument.materialBufferCount;
        materialArgument.CalculateMemorySize();
        size_t size = AlignBlock(materialArgument.memorySize, 8);
        MaterialObj* material = new (&m_Materials[i]) MaterialObj();
        material->Initialize(materialArgument, materialMemory, size);
        materialMemory += size;
    }
    ClearBoneVisible();
    ClearMaterialVisible();
    if (m_pBounding != nullptr)
        new (m_pBounding) Sphere();
    return true;
}

// viewIndex selects the camera, view supplies its transform, bufferIndex selects the GPU block.
void ModelObj::CalculateView(int viewIndex, const nn::util::Matrix4x3fType& view, int bufferIndex) {
    if (m_ViewDependent == 0 || m_NumShapes == 0)
        return;
    const nn::util::Matrix4x3fType* world = m_Skeleton->GetWorldMtxArray();
    nn::util::Matrix4x3fType billboard;
    int previousBone = -1;
    for (int i = 0; i < m_NumShapes; ++i) {
        ShapeObj* shape = &m_Shapes[i];
        if ((shape->GetResource()->GetVertexSkinCount() == 0 || _88) && shape->IsViewDependent()) {
            int boneIndex = shape->GetResource()->GetBoneIndex();
            const ResBone* bone = m_Skeleton->GetBone(boneIndex);
            nn::util::Matrix4x3fType combined;
            const nn::util::Matrix4x3fType* transform;
            if (bone->GetBillboardMode() != 0) {
                int billboardIndex = bone->ToData().billboardIndex;
                if (billboardIndex != ResBone::InvalidBoneIndex) {
                    if (previousBone != billboardIndex) {
                        m_Skeleton->CalculateBillboardMtx(&billboard, view, billboardIndex, false);
                        previousBone = billboardIndex;
                    }
                    MultiplyWorld(&combined, world[boneIndex], billboard);
                    transform = &combined;
                } else {
                    if (previousBone != boneIndex) {
                        m_Skeleton->CalculateBillboardMtx(&billboard, view, boneIndex, true);
                        previousBone = boneIndex;
                    }
                    transform = &billboard;
                }
            } else {
                transform = &world[boneIndex];
            }
            shape->CalculateShape(viewIndex, *transform, bufferIndex);
        }
    }
}

void ModelObj::ClearBoneVisible() {
    int count = m_Skeleton->GetBoneCount();
    for (int i = 0; i < count; ++i)
        SetBoneVisible(i, m_Skeleton->GetBone(i)->IsVisible());
}

void ModelObj::ClearMaterialVisible() {
    int count = m_NumMaterials;
    for (int i = 0; i < count; ++i)
        SetMaterialVisible(i, (m_Materials[i].GetResource()->ToData().flags & 1) != 0);
}

// device supplies the alignment required by each uninitialized GPU block.
size_t ModelObj::GetBlockBufferAlignment(gfx::Device* device) const {
    size_t alignment = 1;
    if ((m_Skeleton != nullptr) && !m_Skeleton->IsBlockBufferValid())
        alignment = std::max(alignment, m_Skeleton->GetBlockBufferAlignment(device));
    for (int i = 0; i < m_NumShapes; ++i) {
        if (!m_Shapes[i].IsBlockBufferValid())
            alignment = std::max(alignment, m_Shapes[i].GetBlockBufferAlignment(device));
    }
    for (int i = 0; i < m_NumMaterials; ++i) {
        if (!m_Materials[i].IsBlockBufferValid())
            alignment = std::max(alignment, m_Materials[i].GetBlockBufferAlignment(device));
    }
    return alignment;
}

// device supplies the storage and alignment requirements for uninitialized GPU blocks.
size_t ModelObj::CalculateBlockBufferSize(gfx::Device* device) {
    size_t size = 0;
    if ((m_Skeleton != nullptr) && !m_Skeleton->IsBlockBufferValid())
        size = m_Skeleton->CalculateBlockBufferSize(device);
    for (int i = 0; i < m_NumShapes; ++i) {
        ShapeObj* shape = &m_Shapes[i];
        if (!shape->IsBlockBufferValid()) {
            size = AlignBlock(size, shape->GetBlockBufferAlignment(device));
            size += shape->CalculateBlockBufferSize(device);
        }
    }
    for (int i = 0; i < m_NumMaterials; ++i) {
        MaterialObj* material = &m_Materials[i];
        if (!material->IsBlockBufferValid()) {
            size = AlignBlock(size, material->GetBlockBufferAlignment(device));
            size += material->CalculateBlockBufferSize(device);
        }
    }
    return size;
}

// device owns the buffers; pool supplies size bytes beginning at offset.
bool ModelObj::SetupBlockBuffer(gfx::Device* device, gfx::MemoryPool* pool, ptrdiff_t offset, size_t size) {
    if (CalculateBlockBufferSize(device) > size)
        return false;
    m_MemoryPool = pool;
    m_MemoryPoolOffset = offset;
    size_t used = 0;
    bool success = true;
    if ((m_Skeleton != nullptr) && !m_Skeleton->IsBlockBufferValid()) {
        used = m_Skeleton->CalculateBlockBufferSize(device);
        success = m_Skeleton->SetupBlockBuffer(device, pool, offset, used);
    }
    for (int i = 0; i < m_NumShapes; ++i) {
        ShapeObj* shape = &m_Shapes[i];
        if (!shape->IsBlockBufferValid()) {
            size_t start = AlignBlock(used, shape->GetBlockBufferAlignment(device));
            size_t blockSize = shape->CalculateBlockBufferSize(device);
            success &= shape->SetupBlockBuffer(device, pool, start + offset, blockSize);
            used = start + blockSize;
        }
    }
    for (int i = 0; i < m_NumMaterials; ++i) {
        if (!m_Materials[i].IsBlockBufferValid()) {
            size_t start = AlignBlock(used, m_Materials[i].GetBlockBufferAlignment(device));
            size_t blockSize = m_Materials[i].CalculateBlockBufferSize(device);
            success &= m_Materials[i].SetupBlockBuffer(device, pool, start + offset, blockSize);
            used = start + blockSize;
        }
    }
    if (!success)
        return false;
    m_Flag |= 1;
    return true;
}

// device owns the skeleton, shape, and material buffers being released.
void ModelObj::CleanupBlockBuffer(gfx::Device* device) {
    if ((m_Skeleton != nullptr) && m_Skeleton->IsBlockBufferValid())
        m_Skeleton->CleanupBlockBuffer(device);
    for (int i = 0; i < m_NumShapes; ++i)
        if (m_Shapes[i].IsBlockBufferValid())
            m_Shapes[i].CleanupBlockBuffer(device);
    for (int i = 0; i < m_NumMaterials; ++i)
        if (m_Materials[i].IsBlockBufferValid())
            m_Materials[i].CleanupBlockBuffer(device);
    m_Flag ^= 1;
    m_MemoryPool = nullptr;
    m_MemoryPoolOffset = 0;
}

// world supplies the root transform for the model's skeleton.
void ModelObj::CalculateWorld(const nn::util::Matrix4x3fType& world) { m_Skeleton->CalculateWorldMtx(world); }

// lodIndex selects the output sphere and preferred mesh in each shape.
void ModelObj::CalculateBounding(int lodIndex) {
    if ((m_pBounding == nullptr) || m_NumShapes == 0)
        return;
    Sphere* bounding = &m_pBounding[lodIndex];
    ShapeObj* shape = &m_Shapes[0];
    if (lodIndex < shape->GetResource()->GetMeshCount()) {
        shape->CalculateBounding(m_Skeleton, lodIndex);
        *bounding = *shape->GetBounding(lodIndex);
    } else {
        shape->CalculateBounding(m_Skeleton, 0);
        *bounding = *shape->GetBounding();
    }
    int count = m_NumShapes;
    for (int i = 1; i < count; ++i) {
        shape = &m_Shapes[i];
        if (lodIndex < shape->GetResource()->GetMeshCount()) {
            shape->CalculateBounding(m_Skeleton, lodIndex);
            bounding->Merge(*bounding, *shape->GetBounding(lodIndex));
        } else {
            shape->CalculateBounding(m_Skeleton, 0);
            bounding->Merge(*bounding, *shape->GetBounding());
        }
    }
}

// bufferIndex selects the buffered skeleton block.
void ModelObj::CalculateSkeleton(int bufferIndex) { m_Skeleton->CalculateSkeleton(bufferIndex); }

// bufferIndex selects the GPU blocks for view-independent shapes and shape animation.
void ModelObj::CalculateShape(int bufferIndex) {
    if (m_NumShapes == 0)
        return;
    const nn::util::Matrix4x3fType* world = m_Skeleton->GetWorldMtxArray();
    for (int i = 0; i < m_NumShapes; ++i) {
        ShapeObj* shape = &m_Shapes[i];
        if ((shape->GetResource()->GetVertexSkinCount() == 0 || _88) && !shape->IsViewDependent())
            shape->CalculateShape(0, world[shape->GetResource()->GetBoneIndex()], bufferIndex);
        if (shape->IsShapeAnimCalculationEnabled())
            shape->CalculateShapeAnimResult(bufferIndex);
    }
}

// bufferIndex selects the buffered material blocks.
void ModelObj::CalculateMaterial(int bufferIndex) {
    for (int i = 0; i < m_NumMaterials; ++i)
        m_Materials[i].CalculateMaterial(bufferIndex);
}

void ModelObj::UpdateViewDependency() {
    int dependent = 0;
    for (int i = 0; i < m_NumShapes; ++i)
        if (m_Shapes[i].IsViewDependent())
            dependent = 1;
    m_ViewDependent = dependent;
}

void ModelObj::SetShapeAnimCalculationEnabled() {
    int count = m_NumShapes;
    for (int i = 0; i < count; ++i)
        m_Shapes[i].SetShapeAnimCalculationEnabled();
}

void ModelObj::SetShapeAnimCalculationDisabled() {
    int count = m_NumShapes;
    for (int i = 0; i < count; ++i)
        m_Shapes[i].SetShapeAnimCalculationDisabled();
}

// callback receives subsequent texture changes from every material in the model.
void ModelObj::SetTextureChangeCallback(MaterialObj::TextureChangeCallback callback) {
    int count = m_NumMaterials;
    for (int i = 0; i < count; ++i)
        m_Materials[i].SetTextureChangeCallback(callback);
}
} // namespace nn::g3d
