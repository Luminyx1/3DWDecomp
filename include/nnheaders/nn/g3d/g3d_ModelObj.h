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

    /**
     * @brief Construct an empty model object without allocated GPU or working storage.
     */
    ModelObj()
        : m_ResModel(nullptr), m_BoneVisibility(nullptr), m_MaterialVisibility(nullptr), _18(0),
          m_ViewDependent(0), m_Flag(0), _20(nullptr), _28(nullptr), m_NumShapes(0),
          m_NumMaterials(0), m_Skeleton(nullptr), m_Shapes(nullptr), m_Materials(nullptr),
          m_pBounding(nullptr), m_UserData(nullptr), _60(nullptr), m_MemoryPool(nullptr),
          m_MemoryPoolOffset(0), m_VisibilityCallback(nullptr),
          m_MaterialVisibilityCallback(nullptr), _88(false), _8c(0) {}

    /**
     * @brief Get the model resource.
     * @return Resource used to initialize the model.
     */
    const ResModel* GetResource() const { return m_ResModel; }
    /**
     * @brief Change a bone's visibility and notify its callback when the value changes.
     * @param index Bone index within the model skeleton.
     * @param visible New visibility state for the bone.
     */
    void SetBoneVisible(int index, bool visible) {
        u32 mask = 1u << (index & 31);
        bool previous = (m_BoneVisibility[static_cast<unsigned>(index) >> 5] & mask) != 0;
        m_BoneVisibility[static_cast<unsigned>(index) >> 5] =
            (m_BoneVisibility[static_cast<unsigned>(index) >> 5] & ~mask) |
            (static_cast<u32>(visible) << (index & 31));
        if ((m_VisibilityCallback != nullptr) & (previous != visible))
            m_VisibilityCallback(this, index);
    }
    /**
     * @brief Get the model's skeleton object.
     * @return Skeleton object associated with this model.
     */
    SkeletonObj* GetSkeleton() const { return m_Skeleton; }

    /**
     * @brief Get the number of shapes in the model.
     * @return Shape count.
     */
    s32 GetNumShapes() const { return m_NumShapes; }
    /**
     * @brief Get the number of materials in the model.
     * @return Material count.
     */
    s32 GetNumMaterials() const { return m_NumMaterials; }
    /**
     * @brief Get the number of configured levels of detail.
     * @return Level-of-detail count.
     */
    s32 GetLodCount() const { return _8c; }
    /**
     * @brief Get the number of views the model was initialized for.
     * @return View count.
     */
    int GetViewCount() const { return _18; }
    /**
     * @brief Get a shape object.
     * @param index Shape index in the range [0, GetNumShapes()).
     * @return Selected shape object.
     */
    ShapeObj* GetShape(int index) const { return &m_Shapes[index]; }
    /**
     * @brief Get a material object.
     * @param index Material index in the range [0, GetNumMaterials()).
     * @return Selected material object.
     */
    MaterialObj* GetMaterial(int index) const { return &m_Materials[index]; }
    /**
     * @brief Get the model's bounding-sphere array.
     * @return Bounding spheres by level of detail, or nullptr when bounding storage is disabled.
     */
    const Sphere* GetBounding() const { return m_pBounding; }

    /**
     * @brief Check whether the model's GPU blocks have been initialized.
     * @return True if block-buffer setup completed successfully.
     */
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

    /**
     * @brief Get the packed bone-visibility flags.
     * @return Array containing one visibility bit per bone.
     */
    u32* GetBoneVisibilityArray() const { return m_BoneVisibility; }
    /**
     * @brief Get the packed material-visibility flags.
     * @return Array containing one visibility bit per material.
     */
    u32* GetMaterialVisibilityArray() const { return m_MaterialVisibility; }
    /**
     * @brief Get the callback for bone-visibility changes.
     * @return Configured callback, or nullptr when notifications are disabled.
     */
    VisibilityCallback GetBoneVisibilityCallback() const { return m_VisibilityCallback; }

    /**
     * @brief Check a bone's visibility.
     * @param index Bone index within the model skeleton.
     * @return True if the bone's visibility bit is set.
     */
    bool IsBoneVisible(int index) const {
        return (m_BoneVisibility[static_cast<u32>(index) >> 5] & (1u << (index & 31))) != 0;
    }

    /**
     * @brief Check a material's visibility.
     * @param index Material index in the range [0, GetNumMaterials()).
     * @return True if the material's visibility bit is set.
     */
    bool IsMaterialVisible(int index) const {
        return (m_MaterialVisibility[index >> 5] & (1u << (index & 31))) != 0;
    }

    /**
     * @brief Change a material's visibility and notify its callback when the value changes.
     * @param index Material index in the range [0, GetNumMaterials()).
     * @param isVisible New visibility state for the material.
     */
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

    struct InitializeArgument {
        /**
         * @brief Set up the default arguments for a model resource.
         * @param pResource Model resource; its largest mesh count is the level-of-detail count.
         */
        explicit InitializeArgument(const ResModel* pResource)
            : resource(pResource), skeleton(nullptr), skeletonBufferCount(1), shapeBufferCount(1),
              materialBufferCount(1), viewCount(1), lodCount(1), shapeUserArea(nullptr),
              boundingEnabled(false) {
            for (detail::WorkMemoryBlock& block : blocks) {
                block.Initialize(0);
            }

            memorySize = 0;
            memoryAlignment = 0;

            for (int i = 0; i < pResource->GetShapeCount(); i++) {
                int meshCount = pResource->GetShape(i)->GetMeshCount();
                lodCount = lodCount < meshCount ? meshCount : lodCount;
            }
        }

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

  private:

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
