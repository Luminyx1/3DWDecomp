#pragma once
#include <nn/g3d/g3d_AnimObj.h>
#include <nn/g3d/g3d_Resources.h>

namespace nn::g3d {
struct ResBoneVisibilityAnim {
    u32 signature;
    u16 flags;
    u8 _6[0x20 - 6];
    const u16* bindIndices;
    const ResAnimCurve* curves;
    const u32* baseValues;
    const nn::util::BinPtrToString* names;
    u8 _40[0x10];
    int frameCount;
    u32 _54;
    u16 animCount;
    u16 curveCount;
};
class BoneVisibilityAnimObj : public ModelAnimObj {
public:
    struct InitializeArgument {
        int targetCount;
        int animCount;
        int curveCount;
        bool cacheEnabled;
        bool cacheAvailable;
        size_t memorySize;
        size_t memoryAlignment;
        detail::WorkMemoryBlock blocks[3];
        void CalculateMemorySize();
        int GetBindCount() const {
            int targets = animCount > 0 ? targetCount : 0;
            int animations = animCount > 0 ? animCount : 0;
            return targets < animations ? animations : targets;
        }
    };
    virtual ~BoneVisibilityAnimObj() {}
    bool Initialize(const InitializeArgument& argument, void* memory, size_t size);
    void SetResource(const ResBoneVisibilityAnim* resource);
    virtual BindResult Bind(const ResModel* model);
    virtual BindResult Bind(const ModelObj* model);
    virtual void BindFast(const ResModel* model);
    virtual void ClearResult();
    virtual void Calculate();
    virtual void ApplyTo(ModelObj* model) const;
    void RevertTo(ModelObj* model) const;
private:
    const ResBoneVisibilityAnim* mResource;
    u16 mAnimCapacity;
    u16 _72;
    int mCurveCount;
    const ResAnimCurve* mCurves;
};
static_assert(sizeof(BoneVisibilityAnimObj) == 0x80, "Bone visibility animation object size");
}
