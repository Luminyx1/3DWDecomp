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
    /**
     * @brief Access a typed constant value in the animation's base-value storage.
     * @tparam T Constant channel representation, aligned to a four-byte word.
     * @param wordIndex Four-byte offset within the resource's base-value storage.
     * @return Read-only pointer to the selected constant channel.
     */
    template <class T> const T* GetBaseValue(int wordIndex) const {
        return reinterpret_cast<const T*>(static_cast<const u8*>(baseValues) + wordIndex * 4);
    }

    /**
     * @brief Access the constant translation stored for this bone animation.
     * @return Read-only translation vector at the resource's base-value index.
     */
    const util::Float3& GetBaseTranslation() const {
        return *reinterpret_cast<const util::Float3*>(static_cast<const float*>(baseValues) + translationBaseIndex);
    }
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
    u8 translationBaseIndex;
    u8 _30[8];
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

    int GetFrameCount() const { return frameCount; }
    int GetCurveCount() const { return curveCount; }
    int GetBoneAnimCount() const { return boneAnimCount; }
    bool IsCurveBaked() const { return flags & 1; }
    bool IsLooped() const { return flags & 4; }
    const char* GetName() const { return name.Get()->GetData(); }

    u32 signature;
    u32 flags;
    nn::util::BinPtrToString name;
    u8 _10[0x8];
    const ResSkeleton* boundSkeleton;
    u16* bindIndices;
    ResBoneAnim* boneAnims;
    u8 _30[0x10];
    int frameCount;
    int curveCount;
    u32 bakedSize;
    u16 boneAnimCount;
    u16 _4e;
};
}
