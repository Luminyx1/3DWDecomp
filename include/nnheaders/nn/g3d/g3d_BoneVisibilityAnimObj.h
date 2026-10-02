#pragma once
#include <nn/g3d/g3d_AnimObj.h>
#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_ResSkeleton.h>

namespace nn::g3d {
struct ResBoneVisibilityAnim {
    u32 signature;
    u16 flags;
    u8 _6[2];
    nn::util::BinPtrToString name;
    u8 _10[0x20 - 0x10];
    const u16* bindIndices;
    const ResAnimCurve* curves;
    const u32* baseValues;
    const nn::util::BinPtrToString* names;
    u8 _40[0x10];
    int frameCount;
    u32 _54;
    u16 animCount;
    u16 curveCount;

    int GetFrameCount() const { return frameCount; }
    bool IsLooped() const { return flags & 4; }
    const char* GetName() const { return name.Get()->GetData(); }
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
    // Builder accumulates the capacities needed by a set of models and animations.
    class Builder : public InitializeArgument {
    public:
        Builder() {
            curveCount = -1;
            targetCount = -1;
            animCount = -1;
            cacheEnabled = true;
            cacheAvailable = false;
            memorySize = 0;
            memoryAlignment = 0;

            for (int i = 0; i < 3; ++i) blocks[i].Initialize(0);
        }
        // model supplies the number of bones to bind to.
        void Reserve(const ResModel* model) {
            targetCount = model->ToData().pSkeleton.Get()->GetBoneCount();
        }
        // resource raises the animation and curve capacities to fit it.
        void Reserve(const ResBoneVisibilityAnim* resource) {
            animCount = animCount < resource->animCount ? resource->animCount : animCount;
            curveCount = curveCount < resource->curveCount ? resource->curveCount : curveCount;
            cacheAvailable |= (resource->flags & 1) == 0;
        }
        size_t GetWorkMemorySize() const { return memorySize; }
        bool Build(BoneVisibilityAnimObj* object, void* memory, size_t size) const {
            return object->Initialize(*this, memory, size);
        }
    };
    BoneVisibilityAnimObj() : mResource(nullptr), mAnimCapacity(0), mCurveCount(0), mCurves(nullptr) {}
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
