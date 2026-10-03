#include <nn/g3d/g3d_SkeletalAnimObj.h>
#include <nn/g3d/g3d_SkeletonObj.h>
#include <nn/g3d/g3d_ModelObj.h>

namespace nn::g3d {
namespace detail {
struct SkeletalAnimObjUtil {
    enum TargetValue { TargetValue_Rotate = 1, TargetValue_Translate = 2, TargetValue_RotateTranslate = 3 };
    /**
     * @brief Restore selected constant channels before mirroring or retargeting.
     * @tparam target Channels to restore: rotation, translation, or both.
     * @param pResult Writable result receiving the selected constant channels.
     * @param pAnim Animation containing the channel values and their storage indices.
     * @param pBone Bone whose mirroring state determines whether rotation is reset; unused for translation
     * alone.
     */
    template <TargetValue target>
    static void ClearAnimResultValue(BoneAnimResult* pResult, const ResBoneAnim* pAnim,
                                     const ResBone* pBone) {
        if constexpr ((target & TargetValue_Rotate) != 0) {
            if (pBone->GetMirroringState() != 0x400000) {
                if ((pAnim->flags & 8) != 0) {
                    std::memcpy(&pResult->rotate, pAnim->GetBaseValue<util::Float3>(3), sizeof(util::Float3));
                } else {
                    std::memcpy(&pResult->rotate, pAnim->GetBaseValue<util::Float3>(0), sizeof(util::Float3));
                }
            }
        }
        if constexpr ((target & TargetValue_Translate) != 0) {
            pResult->translate = pAnim->GetBaseTranslation();
        }
    }
};
template void SkeletalAnimObjUtil::ClearAnimResultValue<SkeletalAnimObjUtil::TargetValue_Rotate>(
    BoneAnimResult*, const ResBoneAnim*, const ResBone*);
template void SkeletalAnimObjUtil::ClearAnimResultValue<SkeletalAnimObjUtil::TargetValue_Translate>(
    BoneAnimResult*, const ResBoneAnim*, const ResBone*);
template void SkeletalAnimObjUtil::ClearAnimResultValue<SkeletalAnimObjUtil::TargetValue_RotateTranslate>(
    BoneAnimResult*, const ResBoneAnim*, const ResBone*);
} // namespace detail

struct SkeletalAnimObj::Impl {
    using ClearFunction = void (SkeletalAnimObj::*)(const ResSkeleton*);
    static const ClearFunction s_pFuncClearImpl[4];
    using CalculateFunction = void (SkeletalAnimObj::*)();
    static const CalculateFunction s_pFuncCalculateImpl[4];
    using ApplyFunction = void (SkeletalAnimObj::*)(SkeletonObj*) const;
    static const ApplyFunction s_pFuncApplyToImpl[2];
};
/**
 * @brief Restore ordinary animation results from their bound bones without mirroring or retargeting.
 * @param pSkeleton Skeleton supplying defaults for bound bone animations; must not be null.
 */
template <> void SkeletalAnimObj::ClearImpl<false, false>(const ResSkeleton* pSkeleton) {
    int count = mBindTable.mAnimCount;
    auto* pResults = static_cast<BoneAnimResult*>(mResult);
    for (int i = 0; i < count; ++i) {
        unsigned target = mBindTable.mEntries[i] & 0x7fff;
        if (target != 0x7fff) {
            const ResBoneAnim* pAnim = &m_pBoneAnims[i];
            const ResBone* pBone = pSkeleton->GetBone(target);
            pAnim->Initialize(&pResults[i], pBone);
        }
    }
}
const SkeletalAnimObj::Impl::ClearFunction SkeletalAnimObj::Impl::s_pFuncClearImpl[4] = {
    &SkeletalAnimObj::ClearImpl<false, false>, &SkeletalAnimObj::ClearImpl<false, true>,
    &SkeletalAnimObj::ClearImpl<true, false>, &SkeletalAnimObj::ClearImpl<true, true>};

/** @brief Evaluate ordinary bone channels without mirroring or retargeting. */
template <> void SkeletalAnimObj::CalculateImpl<false, false>() {
    float frame = GetFrameCtrl().GetFrame();
    auto* pResults = static_cast<BoneAnimResult*>(mResult);
    if (mContext.IsCacheValid()) {
        int count = mBindTable.mAnimCount;
        unsigned cacheIndex = 0;
        for (int i = 0; i < count; ++i, ++pResults) {
            const ResBoneAnim* pAnim = &m_pBoneAnims[i];
            unsigned nextCacheIndex = cacheIndex + pAnim->curveCount;
            if ((mBindTable.mEntries[i] & 0x40000000) == 0) {
                pAnim->Evaluate(pResults, frame, &mContext.mCache[cacheIndex]);
            }
            cacheIndex = nextCacheIndex;
        }
    } else {
        int count = mBindTable.mAnimCount;
        for (int i = 0; i < count; ++i) {
            if ((mBindTable.mEntries[i] & 0x40000000) == 0) {
                m_pBoneAnims[i].Evaluate(&pResults[i], frame);
            }
        }
    }
}
const SkeletalAnimObj::Impl::CalculateFunction SkeletalAnimObj::Impl::s_pFuncCalculateImpl[4] = {
    &SkeletalAnimObj::CalculateImpl<false, false>, &SkeletalAnimObj::CalculateImpl<false, true>,
    &SkeletalAnimObj::CalculateImpl<true, false>, &SkeletalAnimObj::CalculateImpl<true, true>};
const SkeletalAnimObj::Impl::ApplyFunction SkeletalAnimObj::Impl::s_pFuncApplyToImpl[2] = {
    &SkeletalAnimObj::ApplyToImpl<QuatToMtx>, &SkeletalAnimObj::ApplyToImpl<EulerToMtx>};

/** @brief Calculate workspace blocks for bone results, bindings, curve caches and retargeting rotations. */
void SkeletalAnimObj::InitializeArgument::CalculateMemorySize() {
    int bindings = boneCount < boneAnimCount ? boneAnimCount : boneCount;
    for (int i = 0; i < 4; ++i) {
        blocks[i].Initialize(0);
    }
    blocks[0].size = boneAnimCount * sizeof(BoneAnimResult);
    blocks[1].size = bindings * sizeof(u32);
    int curves = curveCount;
    blocks[2].size =
        isContextAvailable && isContextEnabled ? (curves * sizeof(AnimFrameCache) + 7) & ~size_t(7) : 0;
    blocks[3].size = isRetargetingEnabled ? boneAnimCount * sizeof(util::Vector4fType) : 0;
    blocks[3].alignment = 16;
    blocks[3].pointer = nullptr;
    memorySize = 0;
    memoryAlignment = 8;
    for (int i = 0; i < 4; ++i) {
        detail::AppendWorkspaceBlock(blocks[i], memorySize, memoryAlignment, blocks[i].alignment);
    }
}
/**
 * @brief Attach skeletal animation storage to a previously calculated workspace layout.
 * @param rArg Initialization capacities and calculated block offsets.
 * @param pBuffer Caller-owned storage aligned to the calculated workspace alignment.
 * @param bufferSize Available bytes in pBuffer, at least the calculated workspace size.
 * @return True if the layout was calculated and fits in the supplied buffer.
 */
bool SkeletalAnimObj::Initialize(const InitializeArgument& rArg, void* pBuffer, size_t bufferSize) {
    if (rArg.memoryAlignment == 0) {
        return false;
    }
    if (rArg.memorySize > bufferSize) {
        return false;
    }
    int bindings = rArg.boneCount < rArg.boneAnimCount ? rArg.boneAnimCount : rArg.boneCount;
    int curves = rArg.curveCount;
    mWorkMemory = pBuffer;
    mResource = nullptr;
    mBindTable.Initialize(rArg.blocks[1].GetPointer<u32>(pBuffer), bindings);
    mContext.Initialize(rArg.blocks[2].GetPointer<AnimFrameCache>(pBuffer), curves);
    mResult = rArg.blocks[0].GetPointer(pBuffer);
    m_BoneAnimCapacity = rArg.boneAnimCount;
    m_pRetargeting = rArg.blocks[3].GetPointer<util::Vector4fType>(pBuffer);
    return true;
}
/**
 * @brief Select a skeletal animation and reset its playback, bindings and curve caches.
 * @param pRes Non-null animation resource whose counts fit the initialized capacities.
 */
void SkeletalAnimObj::SetResource(const ResSkeletalAnim* pRes) {
    mResource = pRes;
    m_pBoneAnims = pRes->boneAnims;
    mBindTable.mFlags &= ~1;
    bool loop = pRes->IsLooped();
    int frames = pRes->GetFrameCount();
    ResetFrameCtrl(frames, loop);
    mBindTable.mAnimCount = pRes->GetBoneAnimCount();
    mContext.SetCurveCount(pRes->GetCurveCount());
}
/**
 * @brief Resolve animation bone names against a skeleton and reset the evaluation cache.
 * @param pSkeleton Non-null skeleton whose bone count fits the binding-table capacity.
 * @return Combined success and failure flags for all animation bone bindings.
 */
BindResult SkeletalAnimObj::BindImpl(const ResSkeleton* pSkeleton) {
    mBindTable.ClearAll(pSkeleton->GetBoneCount());
    BindResult result;
    int count = mBindTable.mAnimCount;
    for (int i = 0; i < count; ++i) {
        const util::ResDic* pDic = pSkeleton->ToData().pBoneDic.Get();
        int target = pDic != nullptr ? pDic->FindIndex(m_pBoneAnims[i].name.Get()->GetData()) : -1;
        if (target >= 0) {
            mBindTable.mEntries[i] &= 0x3fff8000;
            mBindTable.mEntries[i] |= target & 0x7fff;
            mBindTable.mEntries[target] &= 0xc0007fff;
            mBindTable.mEntries[target] |= (i & 0x7fff) << 15;
            result.Merge(BindResult(BindResult::Flag_Success));
        } else {
            result.Merge(BindResult(BindResult::Flag_Failure));
        }
    }
    mBindTable.mFlags |= 1;
    mContext.Reset();
    return result;
}
/**
 * @brief Reset results using the active mirroring and retargeting policy.
 * @param pSkeleton Skeleton supplying the default transforms for bound bones.
 */
void SkeletalAnimObj::ClearResult(const ResSkeleton* pSkeleton) {
    (this->*Impl::s_pFuncClearImpl[(m_Flags >> 5) & 3])(pSkeleton);
}
/**
 * @brief Bind directly to a skeleton with mirroring and retargeting disabled.
 * @param pSkeleton Non-null skeleton supplying target bones and default transforms.
 * @return Combined success and failure flags from name-based binding.
 */
BindResult SkeletalAnimObj::Bind(const ResSkeleton* pSkeleton) {
    m_pBoundSkeleton = pSkeleton;
    m_Flags &= ~0x60;
    BindResult result;
    result.Merge(BindImpl(pSkeleton));
    ClearResult(pSkeleton);
    return result;
}
/**
 * @brief Bind to the resource of a skeleton object.
 * @param pSkeleton Initialized skeleton object supplying the target resource.
 * @return Combined success and failure flags from name-based binding.
 */
BindResult SkeletalAnimObj::Bind(const SkeletonObj* pSkeleton) { return Bind(pSkeleton->GetRes()); }
/**
 * @brief Bind to the skeleton belonging to a model resource.
 * @param pModel Non-null model resource containing the target skeleton.
 * @return Combined success and failure flags from name-based binding.
 */
BindResult SkeletalAnimObj::Bind(const ResModel* pModel) { return Bind(pModel->GetSkeleton()); }
/**
 * @brief Bind to the skeleton resource of an initialized model object.
 * @param pModel Initialized model object containing the target skeleton object.
 * @return Combined success and failure flags from name-based binding.
 */
BindResult SkeletalAnimObj::Bind(const ModelObj* pModel) { return Bind(pModel->GetSkeleton()->GetRes()); }
/**
 * @brief Bind using the bone indices precomputed in the animation resource.
 * @param pSkeleton Skeleton matching the animation's precomputed binding indices.
 */
void SkeletalAnimObj::BindFast(const ResSkeleton* pSkeleton) {
    m_Flags &= ~0x60;
    mBindTable.ClearAll(pSkeleton->GetBoneCount());
    mBindTable.BindAll(mResource->bindIndices);
    m_pBoundSkeleton = pSkeleton;
    mBindTable.mFlags |= 1;
    mContext.Reset();
    ClearResult(pSkeleton);
}
/**
 * @brief Bind a model resource using the animation's precomputed bone indices.
 * @param pModel Model resource whose skeleton matches the precomputed binding indices.
 */
void SkeletalAnimObj::BindFast(const ResModel* pModel) { BindFast(pModel->GetSkeleton()); }
/** @brief Restore resource-provided default results without using a bound skeleton. */
void SkeletalAnimObj::ClearResult() {
    int count = mBindTable.mAnimCount;
    auto* pResult = static_cast<BoneAnimResult*>(mResult);
    for (int i = 0; i < count; ++i, ++pResult) {
        m_pBoneAnims[i].Initialize(pResult, nullptr);
    }
}
/** @brief Evaluate the current playback frame only when it differs from the last calculated frame. */
void SkeletalAnimObj::Calculate() {
    if (mContext.mLastFrame != GetFrameCtrl().GetFrame()) {
        (this->*Impl::s_pFuncCalculateImpl[(m_Flags >> 5) & 3])();
        mContext.mLastFrame = GetFrameCtrl().GetFrame();
    }
}
/**
 * @brief Apply calculated bone transforms using the animation's rotation representation.
 * @param pSkeleton Initialized skeleton receiving transforms for applicable bound bones.
 */
void SkeletalAnimObj::ApplyTo(SkeletonObj* pSkeleton) const {
    (this->*Impl::s_pFuncApplyToImpl[(mResource->flags >> 12) & 7])(pSkeleton);
}
/**
 * @brief Apply calculated bone transforms to a model's skeleton.
 * @param pModel Initialized model object containing the bound target skeleton.
 */
void SkeletalAnimObj::ApplyTo(ModelObj* pModel) const { ApplyTo(pModel->GetSkeleton()); }
/**
 * @brief Set a binding policy for a bone and every bone in its contiguous descendant branch.
 * @param pSkeleton Skeleton defining the branch extent; must match the binding table.
 * @param boneIndex First bone index, within the skeleton's bone array.
 * @param flag Calculation and application policy for each bound bone in the branch.
 */
void SkeletalAnimObj::SetBindFlag(const ResSkeleton* pSkeleton, int boneIndex, BindFlag flag) {
    u32 flags = static_cast<u32>(flag) << 30;
    int endIndex = pSkeleton->GetBranchEndIndex(boneIndex);
    ptrdiff_t index = boneIndex;
    do {
        mBindTable.SetFlagsForTarget(index, flags);
        ++index;
    } while (index < endIndex);
}
} // namespace nn::g3d
