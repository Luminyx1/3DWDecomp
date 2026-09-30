#pragma once
#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_ResSkeleton.h>
#include <nn/g3d/g3d_ResAnimCurve.h>

namespace nn::g3d {
struct BoneAnimResult {
    u32 flags;
    nn::util::Float3 scale;
    nn::util::Float3 translate;
    u32 _1c;
    nn::util::Float4 rotate;
};
class ResBoneAnim {
public:
    // result receives constant channels; bone supplies defaults for channels absent from the animation.
    void Initialize(BoneAnimResult* result, const ResBone* bone) const;
    // result receives samples at frame; optional cache holds one interval per curve.
    void Evaluate(BoneAnimResult* result, float frame) const;
    void Evaluate(BoneAnimResult* result, float frame, AnimFrameCache* cache) const;

    nn::util::BinPtrToString name;
    ResAnimCurve* curves;
    void* baseValues;
    u8 _18[0x10];
    u32 flags;
    u8 _2c[2];
    u8 curveCount;
    u8 _2f[9];
};
class ResSkeletalAnim {
public:
    // skeleton supplies named bones to resolve and retain for later animation binding.
    BindResult PreBind(const ResSkeleton* skeleton);
    // skeleton supplies named bones to check without changing resource bindings.
    BindResult BindCheck(const ResSkeleton* skeleton) const;
    // buffer supplies size writable bytes for replacing curves with baked samples.
    bool BakeCurve(void* buffer, size_t size);
    void* ResetCurve();
    void Reset();

    u32 signature;
    u32 flags;
    u8 _8[0x10];
    const ResSkeleton* boundSkeleton;
    u16* bindIndices;
    ResBoneAnim* boneAnims;
    u8 _30[0x18];
    u32 bakedSize;
    u16 boneAnimCount;
    u16 _4e;
};
}
