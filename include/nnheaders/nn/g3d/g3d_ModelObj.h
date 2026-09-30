#pragma once

#include <nn/types.h>
#include <nn/g3d/g3d_SkeletonObj.h>

namespace nn::g3d {

class MaterialObj;
class ResModel;
class ShapeObj;
class SkeletonObj;

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
    void* _50;
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
