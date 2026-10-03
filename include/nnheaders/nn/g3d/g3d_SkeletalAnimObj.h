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
struct QuatToMtx;
struct EulerToMtx;

class SkeletalAnimObj : public ModelAnimObj {
  public:
    struct InitializeArgument {
        /** @brief Initialize unset capacities and empty workspace blocks with curve caching enabled. */
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

        /** @brief Set the maximum number of target bones.
         * @param count Nonnegative bone capacity sufficient for every target skeleton.
         */
        void SetMaxBoneCount(int count) { boneCount = count; }

        /**
         * @brief Increase capacities to accommodate a skeletal animation resource.
         * @param pResAnim Non-null animation this object must be able to evaluate.
         */
        void Reserve(const ResSkeletalAnim* pResAnim) {
            boneAnimCount = std::max(boneAnimCount, pResAnim->GetBoneAnimCount());
            curveCount = std::max(curveCount, pResAnim->GetCurveCount());
            isContextAvailable |= !pResAnim->IsCurveBaked();
        }

        void CalculateMemorySize();
        /** @brief Query the last calculated workspace requirement.
         * @return Required bytes, or zero before the layout is calculated.
         */
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

    struct BindArgument {
        const ResSkeleton* pTargetSkeleton;
        const ResSkeleton* pSourceSkeleton;
        bool isRetargetingEnabled;
        bool isMirroringEnabled;
    };

    /** @brief Construct an empty skeletal animation object without allocated working storage. */
    SkeletalAnimObj() {}
    /** @brief Destroy the animation object without releasing caller-owned workspace. */
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
    BindResult Bind(const SkeletonObj* pSkeleton);
    BindResult Bind(const BindArgument& rArg);
    BindResult BindFast(const BindArgument& rArg);
    BindResult Bind(const ResSkeleton* pTarget, const ResSkeleton* pSource);
    BindResult Bind(const SkeletonObj* pTarget, const SkeletonObj* pSource);
    BindResult Bind(const ResModel* pTarget, const ResModel* pSource);
    BindResult Bind(const ModelObj* pTarget, const ModelObj* pSource);
    BindResult BindFast(const ResSkeleton* pTarget, const ResSkeleton* pSource);
    BindResult BindFast(const ResModel* pTarget, const ResModel* pSource);
    void ClearResult(const ResSkeleton* pSkeleton);
    void BindFast(const ResSkeleton* pSkeleton);
    void SetBindFlag(const ResSkeleton* pSkeleton, int boneIndex, BindFlag flag);
    void ApplyTo(SkeletonObj* pSkeleton) const;

    /** @brief Access the selected skeletal animation resource.
     * @return Active resource, or nullptr before resource selection.
     */
    const ResSkeletalAnim* GetResource() const { return mResource; }

  private:
    const ResSkeletalAnim* mResource = nullptr;
    struct Impl;
    template <bool mirrored, bool retargeted> void ClearImpl(const ResSkeleton* pSkeleton);
    template <bool mirrored, bool retargeted> void CalculateImpl();
    template <class Converter> void ApplyToImpl(SkeletonObj* pSkeleton) const;
    BindResult BindImpl(const ResSkeleton* pSkeleton);
    BindResult InitRetargeting(const ResSkeleton* pTarget, const ResSkeleton* pSource);
    void BindFastImpl(const ResSkeleton* pTarget);
    const ResBoneAnim* m_pBoneAnims = nullptr;
    int m_BoneAnimCapacity = 0;
    u32 m_Flags = 0;
    util::Vector4fType* m_pRetargeting = nullptr;
    const ResSkeleton* m_pBoundSkeleton = nullptr;
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

} // namespace nn::g3d
