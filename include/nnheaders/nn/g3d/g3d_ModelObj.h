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

// TODO
class ModelObj {
public:
    // model identifies the changed object; index selects the bone whose visibility changed.
    using VisibilityCallback = void (*)(ModelObj* model, int index);
    const ResModel* GetResource() const { return m_ResModel; }
    // index identifies the bone; visible is its new visibility state.
    void SetBoneVisible(int index, bool visible) {
        u32 mask = 1u << (index & 31);
        bool previous = (m_BoneVisibility[static_cast<unsigned>(index) >> 5] & mask) != 0;
        m_BoneVisibility[static_cast<unsigned>(index) >> 5] = (m_BoneVisibility[static_cast<unsigned>(index) >> 5] & ~mask) | (static_cast<u32>(visible) << (index & 31));
        if ((m_VisibilityCallback != nullptr) & (previous != visible)) m_VisibilityCallback(this, index);
    }
    SkeletonObj* GetSkeleton() const { return m_Skeleton; }

    s32 GetNumShapes() const { return m_NumShapes; }
    s32 GetNumMaterials() const { return m_NumMaterials; }
    s32 GetLodCount() const { return _8c; }
    ShapeObj* GetShape(int index) const { return &m_Shapes[index]; }
    MaterialObj* GetMaterial(int index) const { return &m_Materials[index]; }
    const Sphere* GetBounding() const { return m_pBounding; }

    bool IsBlockBufferValid() const { return (_1a & 1) != 0; }
    void CleanupBlockBuffer(gfx::Device* pDevice);

    u32* GetBoneVisibilityArray() const { return m_BoneVisibility; }
    VisibilityCallback GetBoneVisibilityCallback() const { return m_VisibilityCallback; }

    bool IsBoneVisible(int index) const {
        return (m_BoneVisibility[static_cast<u32>(index) >> 5] & (1 << index)) != 0;
    }

    bool IsMaterialVisible(int index) const {
        return (static_cast<const u32*>(_10)[index >> 5] & (1 << index)) != 0;
    }

    void SetMaterialVisible(int index, bool isVisible) {
        bool isPrevVisible = IsMaterialVisible(index);
        u32 bit = 1 << index;
        u32& word = static_cast<u32*>(_10)[index >> 5];
        word = (word & ~bit) | (static_cast<u32>(isVisible) << index);
        auto callback = reinterpret_cast<void (*)(ModelObj*, int)>(_80);
        if (callback && isPrevVisible != isVisible) {
            callback(this, index);
        }
    }

private:
    struct InitializeArgument;

    bool Initialize(const InitializeArgument& arg, void* buffer, size_t bufferSize);

    const ResModel* m_ResModel;
    u32* m_BoneVisibility;
    void* _10;
    u8 _18;
    u8 _19;
    u16 _1a;
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
    void* _68;
    void* _70;
    VisibilityCallback m_VisibilityCallback;
    void* _80;
    bool _88;
    int _8c;
};

}  // namespace nn::g3d
