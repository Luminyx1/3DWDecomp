#pragma once

#include <nn/types.h>
#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_ShapeObj.h>
#include <nn/g3d/g3d_SkeletonObj.h>
#include <nn/gfx/gfx_Types.h>

namespace nn::g3d {

class MaterialObj;
class ResModel;
class ShapeObj;
class SkeletonObj;
class Sphere;

class ModelObj {
  public:
    // model identifies the changed object; index selects the bone whose visibility changed.
    using VisibilityCallback = void (*)(ModelObj* model, int index);
    const ResModel* GetResource() const { return m_ResModel; }
    // index identifies the bone; visible is its new visibility state.
    void SetBoneVisible(int index, bool visible) {
        u32 mask = 1u << (index & 31);
        bool previous = (m_BoneVisibility[static_cast<unsigned>(index) >> 5] & mask) != 0;
        m_BoneVisibility[static_cast<unsigned>(index) >> 5] =
            (m_BoneVisibility[static_cast<unsigned>(index) >> 5] & ~mask) |
            (static_cast<u32>(visible) << (index & 31));
        if ((m_VisibilityCallback != nullptr) & (previous != visible))
            m_VisibilityCallback(this, index);
    }
    SkeletonObj* GetSkeleton() const { return m_Skeleton; }

    s32 GetNumShapes() const { return m_NumShapes; }
    s32 GetNumMaterials() const { return m_NumMaterials; }
    s32 GetLodCount() const { return _8c; }
    ShapeObj* GetShape(int index) const { return &m_Shapes[index]; }
    MaterialObj* GetMaterial(int index) const { return &m_Materials[index]; }
    const Sphere* GetBounding() const { return m_pBounding; }

    bool IsBlockBufferValid() const { return (m_Flag & 1) != 0; }
    void ClearBoneVisible();
    void ClearMaterialVisible();
    // device supplies the alignment and storage requirements for GPU blocks.
    size_t GetBlockBufferAlignment(gfx::Device* device) const;
    size_t CalculateBlockBufferSize(gfx::Device* device);
    // pool supplies size bytes at offset; device initializes each object's GPU blocks.
    bool SetupBlockBuffer(gfx::Device* device, gfx::MemoryPool* pool, ptrdiff_t offset, size_t size);
    // device owns the GPU blocks being released.
    void CleanupBlockBuffer(gfx::Device* device);
    // world is the model-to-world transform applied to the skeleton.
    void CalculateWorld(const nn::util::Matrix4x3fType& world);
    // lodIndex selects the model bounds, falling back to each shape's first mesh.
    void CalculateBounding(int lodIndex);
    // bufferIndex selects the buffered GPU block to update.
    void CalculateSkeleton(int bufferIndex);
    void CalculateShape(int bufferIndex);
    void CalculateMaterial(int bufferIndex);
    // viewIndex and view select the camera; bufferIndex selects the buffered GPU block.
    void CalculateView(int viewIndex, const nn::util::Matrix4x3fType& view, int bufferIndex);
    void UpdateViewDependency();
    void SetShapeAnimCalculationEnabled();
    void SetShapeAnimCalculationDisabled();
    // callback receives the material and texture slot that changed.
    void SetTextureChangeCallback(MaterialObj::TextureChangeCallback callback);

    u32* GetBoneVisibilityArray() const { return m_BoneVisibility; }
    VisibilityCallback GetBoneVisibilityCallback() const { return m_VisibilityCallback; }

    bool IsBoneVisible(int index) const {
        return (m_BoneVisibility[static_cast<u32>(index) >> 5] & (1u << (index & 31))) != 0;
    }

    bool IsMaterialVisible(int index) const {
        return (m_MaterialVisibility[index >> 5] & (1u << (index & 31))) != 0;
    }

    // index selects the material; isVisible is its new visibility state.
    void SetMaterialVisible(int index, bool isVisible) {
        bool isPrevVisible = IsMaterialVisible(index);
        u32 bit = 1u << (index & 31);
        u32& word = m_MaterialVisibility[index >> 5];
        word = (word & ~bit) | (static_cast<u32>(isVisible) << (index & 31));
        VisibilityCallback callback = m_MaterialVisibilityCallback;
        if ((callback != nullptr) && isPrevVisible != isVisible) {
            callback(this, index);
        }
    }

  private:
    struct InitializeArgument {
        const ResModel* resource;
        SkeletonObj* skeleton;
        int skeletonBufferCount;
        int shapeBufferCount;
        int materialBufferCount;
        int viewCount;
        int lodCount;
        const void* shapeUserArea;
        bool boundingEnabled;
        size_t memorySize;
        size_t memoryAlignment;
        detail::WorkMemoryBlock blocks[9];
        void CalculateMemorySize();
    };

    bool Initialize(const InitializeArgument& arg, void* buffer, size_t bufferSize);

    const ResModel* m_ResModel;
    u32* m_BoneVisibility;
    u32* m_MaterialVisibility;
    u8 _18;
    u8 m_ViewDependent;
    u16 m_Flag;
    void* _20;
    void* _28;
    u16 m_NumShapes;
    u16 m_NumMaterials;
    SkeletonObj* m_Skeleton;
    ShapeObj* m_Shapes;
    MaterialObj* m_Materials;
    Sphere* m_pBounding;
    void* m_UserData;
    void* _60;
    gfx::MemoryPool* m_MemoryPool;
    ptrdiff_t m_MemoryPoolOffset;
    VisibilityCallback m_VisibilityCallback;
    VisibilityCallback m_MaterialVisibilityCallback;
    bool _88;
    int _8c;
};

} // namespace nn::g3d
