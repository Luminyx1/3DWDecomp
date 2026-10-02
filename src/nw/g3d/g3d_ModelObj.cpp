#include <nn/g3d/g3d_ModelObj.h>
#include <algorithm>
#include <cstring>
#include <new>

namespace nn::g3d {
namespace {
/**
 * @brief Round a byte count up to the required alignment.
 * @param value Byte count to align.
 * @param alignment Nonzero power-of-two alignment, in bytes.
 * @return Aligned byte count.
 */
inline size_t AlignBlock(size_t value, size_t alignment) { return (value + alignment - 1) & -alignment; }
} // namespace

namespace {
/**
 * @brief Assign a workspace block offset and update the allocation requirements.
 * @param block Block whose size is known and whose offset is assigned; empty blocks are skipped.
 * @param size Accumulated workspace size, updated to include the block.
 * @param alignment Accumulated alignment requirement, updated if the block requires more alignment.
 * @param blockAlignment Nonzero power-of-two alignment required by this block, in bytes.
 */
inline void AppendWorkspaceBlock(detail::WorkMemoryBlock& block, size_t& size, size_t& alignment,
                                 size_t blockAlignment) {
    if (block.size != 0) {
        size_t start = AlignBlock(size, blockAlignment);
        alignment = std::max(alignment, blockAlignment);
        size = start + block.size;
        block.offset = start;
    }
}

/**
 * @brief Reset an initialization argument's workspace sizes and block descriptors.
 * @tparam Argument Initialization argument type containing workspace sizes and block descriptors.
 * @param argument Initialization argument whose workspace descriptors are reset.
 */
template <class Argument> inline void InitializeWorkspace(Argument& argument) {
    for (auto& block : argument.blocks)
        block.Initialize(0);
    argument.memorySize = 0;
    argument.memoryAlignment = 0;
}

/**
 * @brief Create default initialization settings for a shape resource.
 * @param resource Shape resource to initialize; its buffering count is assigned by the caller.
 * @return Shape initialization settings with empty workspace descriptors.
 */
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

/**
 * @brief Compose two affine transforms using row-vector matrix multiplication.
 * @param output Destination for the product first * second.
 * @param first First affine transform in the composition.
 * @param second Second affine transform in the composition.
 */
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

/**
 * @brief Calculate workspace sizes, alignment, and offsets for the configured model.
 */
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

/**
 * @brief Initialize the model objects and their working storage.
 * @param argument Model resources and capacities with workspace sizes already calculated.
 * @param buffer Working storage satisfying the size and alignment calculated in argument.
 * @param bufferSize Available size of buffer, in bytes.
 * @return True if initialization succeeds; false if sizing was not performed or storage is insufficient.
 */
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

/**
 * @brief Update view-dependent shape blocks, including billboard transforms.
 * @param viewIndex Camera index selecting the view-dependent shape block.
 * @param view View transform used to calculate billboards.
 * @param bufferIndex Buffered GPU block index to update.
 */
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

/**
 * @brief Restore each bone's resource visibility and notify callbacks of changes.
 */
void ModelObj::ClearBoneVisible() {
    int count = m_Skeleton->GetBoneCount();
    for (int i = 0; i < count; ++i)
        SetBoneVisible(i, m_Skeleton->GetBone(i)->IsVisible());
}

/**
 * @brief Restore each material's resource visibility and notify callbacks of changes.
 */
void ModelObj::ClearMaterialVisible() {
    int count = m_NumMaterials;
    for (int i = 0; i < count; ++i)
        SetMaterialVisible(i, (m_Materials[i].GetResource()->ToData().flags & 1) != 0);
}

/**
 * @brief Find the largest alignment required by uninitialized model GPU blocks.
 * @param device Graphics device supplying buffer alignment requirements.
 * @return Required alignment in bytes, or 1 when no additional blocks are needed.
 */
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

/**
 * @brief Calculate the total aligned storage needed for uninitialized model GPU blocks.
 * @param device Graphics device supplying buffer sizes and alignment requirements.
 * @return Required memory-pool storage size, in bytes.
 */
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

/**
 * @brief Initialize missing skeleton, shape, and material GPU blocks in a memory pool.
 * @param device Graphics device that owns the GPU buffers.
 * @param pool Memory pool providing storage for the GPU blocks.
 * @param offset Byte offset of the reserved region within pool.
 * @param size Available size of the reserved region, in bytes.
 * @return True if every required block is initialized; false if space is insufficient or a setup fails.
 */
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

/**
 * @brief Release initialized model GPU blocks and clear their memory-pool association.
 * @param device Graphics device that owns the buffers being released.
 */
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

/**
 * @brief Update the skeleton's world matrices from the model transform.
 * @param world Root model-to-world transform.
 */
void ModelObj::CalculateWorld(const nn::util::Matrix4x3fType& world) { m_Skeleton->CalculateWorldMtx(world); }

/**
 * @brief Combine shape bounds into the model's bounding sphere for a level of detail.
 * @param lodIndex Allocated model bounding-sphere index; shapes lacking that mesh use their first mesh.
 */
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

/**
 * @brief Update a buffered skeleton matrix block.
 * @param bufferIndex Buffered skeleton GPU block index to update.
 */
void ModelObj::CalculateSkeleton(int bufferIndex) { m_Skeleton->CalculateSkeleton(bufferIndex); }

/**
 * @brief Update view-independent shape blocks and enabled shape-animation results.
 * @param bufferIndex Buffered shape GPU block index to update.
 */
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

/**
 * @brief Update every material's buffered GPU block.
 * @param bufferIndex Buffered material GPU block index to update.
 */
void ModelObj::CalculateMaterial(int bufferIndex) {
    for (int i = 0; i < m_NumMaterials; ++i)
        m_Materials[i].CalculateMaterial(bufferIndex);
}

/**
 * @brief Refresh whether any shape requires view-dependent calculations.
 */
void ModelObj::UpdateViewDependency() {
    int dependent = 0;
    for (int i = 0; i < m_NumShapes; ++i)
        if (m_Shapes[i].IsViewDependent())
            dependent = 1;
    m_ViewDependent = dependent;
}

/**
 * @brief Enable shape-animation calculations on every shape in the model.
 */
void ModelObj::SetShapeAnimCalculationEnabled() {
    int count = m_NumShapes;
    for (int i = 0; i < count; ++i)
        m_Shapes[i].SetShapeAnimCalculationEnabled();
}

/**
 * @brief Disable shape-animation calculations on every shape in the model.
 */
void ModelObj::SetShapeAnimCalculationDisabled() {
    int count = m_NumShapes;
    for (int i = 0; i < count; ++i)
        m_Shapes[i].SetShapeAnimCalculationDisabled();
}

/**
 * @brief Set the texture-change callback for every material in the model.
 * @param callback Callback receiving the changed material and texture slot; nullptr disables notifications.
 */
void ModelObj::SetTextureChangeCallback(MaterialObj::TextureChangeCallback callback) {
    int count = m_NumMaterials;
    for (int i = 0; i < count; ++i)
        m_Materials[i].SetTextureChangeCallback(callback);
}
} // namespace nn::g3d
