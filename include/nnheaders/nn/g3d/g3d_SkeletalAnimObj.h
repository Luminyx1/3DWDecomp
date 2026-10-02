#pragma once

#include <nn/g3d/g3d_AnimObj.h>
#include <nn/g3d/g3d_ResSkeletalAnim.h>
#include <nn/g3d/g3d_Resources.h>
#include <nn/types.h>
#include <nn/util/util_MathTypes.h>
#include <algorithm>

namespace nn::g3d {
class ResSkeleton;
class SkeletonObj;

class SkeletalAnimObj : public ModelAnimObj {
public:
    struct InitializeArgument {
        InitializeArgument() {
            boneCount = boneAnimCount = curveCount = -1;
            isContextEnabled = true;
            isContextAvailable = false;
            isRetargetingEnabled = false;
            memorySize = 0;
            memoryAlignment = 0;
            for (int i = 0; i < 4; ++i) {
                blocks[i].Initialize(0);
            }
        }

        // count is the number of bones in the target skeleton.
        void SetMaxBoneCount(int count) { boneCount = count; }

        // pResAnim is an animation this object must be able to play.
        void Reserve(const ResSkeletalAnim* pResAnim) {
            boneAnimCount = std::max(boneAnimCount, pResAnim->GetBoneAnimCount());
            curveCount = std::max(curveCount, pResAnim->GetCurveCount());
            isContextAvailable |= !pResAnim->IsCurveBaked();
        }

        void CalculateMemorySize();
        size_t GetWorkMemorySize() const { return memorySize; }

        int boneCount = -1;
        int boneAnimCount = -1;
        int curveCount = -1;
        bool isContextEnabled = true;
        bool isContextAvailable = false;
        bool isRetargetingEnabled = false;
        size_t memorySize = 0;
        size_t memoryAlignment = 0;
        detail::WorkMemoryBlock blocks[4];
    };

    static_assert(sizeof(InitializeArgument) == 0xa0);

    SkeletalAnimObj() {}
    ~SkeletalAnimObj() override = default;

    void ClearResult() override;
    void Calculate() override;
    BindResult Bind(const ResModel* pModel) override;
    BindResult Bind(const ModelObj* pModel) override;
    void BindFast(const ResModel* pModel) override;
    void ApplyTo(ModelObj* pModel) const override;

    bool Initialize(const InitializeArgument& rArg, void* pBuffer, size_t bufferSize);
    void SetResource(const ResSkeletalAnim* pRes);
    BindResult Bind(const ResSkeleton* pSkeleton);
    void ApplyTo(SkeletonObj* pSkeleton) const;

    const ResSkeletalAnim* GetResource() const { return mResource; }

private:
    const ResSkeletalAnim* mResource = nullptr;
    void* _70 = nullptr;
    void* _78 = nullptr;
    void* _80 = nullptr;
    void* _88 = nullptr;
};

static_assert(sizeof(SkeletalAnimObj) == 0x90);

struct SkeletalAnimBlendResult {
    u8 _0[0x10];
    nn::util::Vector3fType translate;
    u8 _20[0x30];
};

static_assert(sizeof(SkeletalAnimBlendResult) == 0x50);

class SkeletalAnimBlender {
public:
    struct InitializeArgument {
        InitializeArgument() { blocks[0].Initialize(0); }

        // count is the number of bones in the target skeleton.
        void SetMaxBoneCount(int count) { boneCount = count; }

        void CalculateMemorySize();
        size_t GetWorkMemorySize() const { return memorySize; }

        int boneCount = -1;
        size_t memorySize = 0;
        size_t memoryAlignment = 0;
        detail::WorkMemoryBlock blocks[1];
    };

    SkeletalAnimBlender() {}

    bool Initialize(const InitializeArgument& rArg, void* pBuffer, size_t bufferSize);
    void ClearResult();
    void Blend(SkeletalAnimObj* pAnimObj, float weight);
    void ApplyTo(SkeletonObj* pSkeleton) const;

    SkeletalAnimBlendResult* GetResult() const { return mResult; }
    int GetBoneCount() const { return mBoneCount; }
    int GetMaxBoneCount() const { return mMaxBoneCount; }

private:
    SkeletalAnimBlendResult* mResult = nullptr;
    u16 mBoneCount = 0;
    u16 mMaxBoneCount = 0;
    u32 _c = 0;
    void* _10 = nullptr;
    void* _18 = nullptr;
};

static_assert(sizeof(SkeletalAnimBlender) == 0x20);

}  // namespace nn::g3d
