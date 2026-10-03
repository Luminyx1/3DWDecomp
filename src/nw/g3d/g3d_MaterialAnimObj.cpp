#include <nn/g3d/g3d_ResMaterialAnim.h>
#include <nn/g3d/g3d_ModelObj.h>

namespace nn::g3d {
namespace {
/**
 * @brief Access a four-byte shader parameter value at its resource-defined offset.
 * @param pData Writable shader parameter storage.
 * @param offset Byte offset of a naturally aligned four-byte value within the parameter.
 * @return Reference to the selected value's raw bits.
 */
inline u32& ParameterWord(u8* pData, u32 offset) { return *reinterpret_cast<u32*>(pData + offset); }
/**
 * @brief Read a four-byte shader parameter value from its resource data.
 * @param pData Read-only shader parameter storage.
 * @param offset Byte offset of a naturally aligned four-byte value within the parameter.
 * @return Reference to the selected value's raw bits.
 */
inline const u32& ParameterWord(const u8* pData, u32 offset) {
    return *reinterpret_cast<const u32*>(pData + offset);
}
/**
 * @brief Access typed results using the animation resource's four-byte result indices.
 * @tparam T Four-byte result element type.
 * @param pResults Base address of the evaluated animation results.
 * @param index Four-byte index within the allocated result storage.
 * @return Read-only pointer to the selected result.
 */
template <typename T> inline const T* ResultAt(const void* pResults, int index) {
    return reinterpret_cast<const T*>(static_cast<const u8*>(pResults) + index * 4);
}

} // namespace
/** @brief Calculate aligned storage for results, bindings, curve caches and textures. */
void MaterialAnimObj::InitializeArgument::CalculateMemorySize() {
    int bindings = materialCount < materialAnimCount ? materialAnimCount : materialCount;
    for (int i = 0; i < 6; ++i) {
        blocks[i].Initialize(0);
    }
    blocks[0].size = curveCount * sizeof(u32);
    blocks[1].size = bindings * sizeof(u32);
    blocks[2].size = (paramAnimCount * sizeof(u16) + 3) & ~size_t(3);
    blocks[3].size =
        cacheAvailable && cacheEnabled ? (curveCount * sizeof(AnimFrameCache) + 7) & ~size_t(7) : 0;
    blocks[4].size = textureCount * sizeof(const gfx::TextureView*);
    blocks[5].size = textureCount * sizeof(u64);
    memorySize = 0;
    memoryAlignment = 8;
    for (int i = 0; i < 6; ++i) {
        blocks[i].AppendTo(memorySize, memoryAlignment);
    }
}
/**
 * @brief Initialize material animation storage from a calculated workspace layout.
 * @param rArgument Nonnegative capacities and a previously calculated memory layout.
 * @param pMemory Workspace aligned to the argument's required alignment.
 * @param size Available workspace bytes, at least the calculated requirement.
 * @return True if capacities and workspace size permit initialization.
 */
bool MaterialAnimObj::Initialize(const InitializeArgument& rArgument, void* pMemory, size_t size) {
    if (rArgument.memoryAlignment == 0) {
        return false;
    }
    if (rArgument.materialCount < 0) {
        return false;
    }
    if (rArgument.materialAnimCount < 0) {
        return false;
    }
    if (rArgument.paramAnimCount < 0) {
        return false;
    }
    if (rArgument.curveCount < 0) {
        return false;
    }
    if (rArgument.textureCount < 0) {
        return false;
    }
    if (rArgument.memorySize > size) {
        return false;
    }
    int bindings = rArgument.materialCount < rArgument.materialAnimCount ? rArgument.materialAnimCount
                                                                         : rArgument.materialCount;
    int curves = rArgument.curveCount;
    mWorkMemory = pMemory;
    m_pRes = nullptr;
    mBindTable.Initialize(rArgument.blocks[1].GetPointer<u32>(pMemory), bindings);
    mContext.Initialize(rArgument.blocks[3].GetPointer<AnimFrameCache>(pMemory), curves);
    mResult = rArgument.blocks[0].GetPointer(pMemory);
    m_MaterialAnimCapacity = rArgument.materialAnimCount;
    m_ParamAnimCapacity = rArgument.paramAnimCount;
    m_TextureCapacity = rArgument.textureCount;
    m_CurveCapacity = rArgument.curveCount;
    m_pSubBindIndices = rArgument.blocks[2].GetPointer<u16>(pMemory);
    m_ppTextureArray = rArgument.blocks[4].GetPointer<const gfx::TextureView*>(pMemory);
    m_pTextureSlotArray = rArgument.blocks[5].GetPointer<u64>(pMemory);
    return true;
}
/**
 * @brief Select an animation, reset playback and copy its texture bindings.
 * @param pResource Animation whose counts fit the initialized capacities; must not be null.
 */
void MaterialAnimObj::SetResource(const ResMaterialAnim* pResource) {
    m_pRes = pResource;
    m_pMaterialAnims = pResource->ToData().materialAnims;
    mBindTable.mFlags &= ~1;
    bool loop = pResource->IsLooped();
    int frames = pResource->GetFrameCount();
    ResetFrameCtrl(frames, loop);
    mBindTable.mAnimCount = pResource->GetPerMaterialAnimCount();
    mContext.SetCurveCount(pResource->GetCurveCount());
    int count = m_pRes->GetTextureCount();
    for (int i = 0; i < count; ++i) {
        m_ppTextureArray[i] = m_pRes->GetTextureView(i);
        m_pTextureSlotArray[i] = m_pRes->GetTextureDescriptorSlot(i);
    }
}
/**
 * @brief Resolve animation material names and their parameter and sampler bindings.
 * @param pModel Model resource supplying named target materials; must not be null.
 * @return Combined success and failure flags for attempted bindings.
 */
BindResult MaterialAnimObj::Bind(const ResModel* pModel) {
    mBindTable.ClearAll(pModel->GetMaterialCount());
    BindResult result;
    int count = mBindTable.mAnimCount;
    int subBindIndex = 0;
    for (int i = 0; i < count; ++i) {
        const ResPerMaterialAnim* pAnim = &m_pMaterialAnims[i];
        const util::ResDic* pDic = pModel->ToData().pMaterialDic.Get();
        int target = pDic != nullptr ? pDic->FindIndex(pAnim->name.Get()->GetData()) : -1;
        if (target >= 0) {
            if (pAnim->visibilityCurve != 0xffff || pAnim->visibilityBase != 0xffff) {
                result.Merge(BindResult(BindResult::Flag_Success));
            }
            BindResult subResult =
                SubBind(pAnim, &pModel->ToData().pMaterialArray.Get()[target], subBindIndex);
            subBindIndex += pAnim->parameterCount + pAnim->textureCount;
            if (subResult.IsAnySuccess() || pAnim->visibilityCurve != 0xffff ||
                pAnim->visibilityBase != 0xffff) {
                mBindTable.mEntries[i] &= 0x3fff8000;
                mBindTable.mEntries[i] |= target & 0x7fff;
                mBindTable.mEntries[target] &= 0xc0007fff;
                mBindTable.mEntries[target] |= (i << 15) & 0x3fff8000;
            }
            result.Merge(subResult);
        } else {
            result.Merge(BindResult(BindResult::Flag_Failure));
        }
    }
    mBindTable.mFlags |= 1;
    mContext.Reset();
    return result;
}
/**
 * @brief Bind parameter and sampler names for one material animation.
 * @param pAnim Animation describing names to resolve; must not be null.
 * @param pMaterial Target material resource containing the name dictionaries.
 * @param subBindIndex First writable entry in the combined sub-binding array.
 * @return Combined success and failure flags for parameter and sampler lookups.
 */
BindResult MaterialAnimObj::SubBind(const ResPerMaterialAnim* pAnim, const ResMaterial* pMaterial,
                                    int subBindIndex) {
    BindResult result;
    int count = pAnim->parameterCount;
    const ResShaderParamAnimInfo* pInfo = pAnim->parameters;
    for (int i = 0; i < count; ++i, ++pInfo) {
        int target = pMaterial->FindShaderParamIndex(pInfo->name.Get()->GetData());
        m_pSubBindIndices[subBindIndex + i] = target;
        result.Merge(BindResult(target >= 0 ? BindResult::Flag_Success : BindResult::Flag_Failure));
    }
    subBindIndex += pAnim->parameterCount;
    count = pAnim->textureCount;
    const ResTexturePatternAnimInfo* pTexture = pAnim->textures;
    for (int i = 0; i < count; ++i, ++pTexture) {
        int target = pMaterial->FindSamplerIndex(pTexture->name.Get()->GetData());
        m_pSubBindIndices[subBindIndex + i] = target;
        result.Merge(BindResult(target >= 0 ? BindResult::Flag_Success : BindResult::Flag_Failure));
    }
    return result;
}
/**
 * @brief Bind through a model object's resource using virtual resource binding.
 * @param pModel Model object supplying the target resource; must not be null.
 * @return Combined binding status from the resource binding operation.
 */
BindResult MaterialAnimObj::Bind(const ModelObj* pModel) {
    BindResult result;
    result.Merge(Bind(pModel->GetResource()));
    return result;
}
/**
 * @brief Bind using material and parameter indices previously stored in the resource.
 * @param pModel Model resource matching the animation's precomputed bindings.
 */
void MaterialAnimObj::BindFast(const ResModel* pModel) {
    mBindTable.ClearAll(pModel->GetMaterialCount());
    mBindTable.BindAll(m_pRes->ToData().bindIndices);
    int count = mBindTable.mAnimCount;
    int subBindIndex = 0;
    for (int i = 0; i < count; ++i) {
        int target = mBindTable.mEntries[i] & 0x7fff;
        if (target != 0x7fff) {
            const ResPerMaterialAnim* pAnim = &m_pMaterialAnims[i];
            BindResult result = SubBindFast(pAnim, subBindIndex);
            subBindIndex += pAnim->parameterCount + pAnim->textureCount;
            if (result.IsFailure()) {
                mBindTable.mEntries[i] |= 0xc0007fff;
                mBindTable.mEntries[target] |= 0x3fff8000;
            }
        }
    }
    mBindTable.mFlags |= 1;
    mContext.Reset();
}
/**
 * @brief Copy precomputed shader parameter and sampler bindings for one material.
 * @param pAnim Animation containing pre-bound target indices; must not be null.
 * @param subBindIndex First writable entry in the combined sub-binding array.
 * @return Combined success and failure flags, treating sentinel indices as failures.
 */
BindResult MaterialAnimObj::SubBindFast(const ResPerMaterialAnim* pAnim, int subBindIndex) {
    BindResult result;
    int count = pAnim->parameterCount;
    const ResShaderParamAnimInfo* pInfo = pAnim->parameters;
    for (int i = 0; i < count; ++i, ++pInfo) {
        m_pSubBindIndices[subBindIndex + i] = pInfo->bindIndex;
        result.Merge(
            BindResult(pInfo->bindIndex == 0xffff ? BindResult::Flag_Failure : BindResult::Flag_Success));
    }
    subBindIndex += pAnim->parameterCount;
    count = pAnim->textureCount;
    const ResTexturePatternAnimInfo* pTexture = pAnim->textures;
    for (int i = 0; i < count; ++i, ++pTexture) {
        m_pSubBindIndices[subBindIndex + i] = static_cast<s8>(pTexture->bindIndex);
        result.Merge(
            BindResult(pTexture->bindIndex == 0xff ? BindResult::Flag_Failure : BindResult::Flag_Success));
    }
    return result;
}
/**
 * @brief Evaluate enabled per-material animations using the selected cache policy.
 * @tparam cached Whether the initialized frame caches are available for curve evaluation.
 * @param frame Playback frame at which all enabled curves are evaluated.
 * @param pAnim Enabled per-material animation supplying curve and result offsets.
 * @param rSubBindIndex First sub-binding; advanced past this animation's parameters and samplers.
 */
template <bool cached>
ALWAYS_INLINE inline void MaterialAnimObj::CalculateMaterialImpl(const ResPerMaterialAnim* pAnim, float frame,
                                                                 int& rSubBindIndex) {
    int index = pAnim->shaderParamResultIndex;
    if (index != 0xffff) {
        pAnim->EvaluateShaderParamAnim<cached>(static_cast<u32*>(mResult) + index, frame,
                                               m_pSubBindIndices + rSubBindIndex,
                                               cached ? mContext.mCache + index : nullptr);
    }
    rSubBindIndex += pAnim->parameterCount;
    index = pAnim->texturePatternResultIndex;
    if (index != 0xffff) {
        pAnim->EvaluateTexturePatternAnim<cached>(static_cast<int*>(mResult) + index, frame,
                                                  m_pSubBindIndices + rSubBindIndex,
                                                  cached ? mContext.mCache + index : nullptr);
    }
    rSubBindIndex += pAnim->textureCount;
    if (pAnim->visibilityCurve != 0xffff) {
        index = pAnim->visibilityResultIndex;
        pAnim->EvaluateVisibilityAnim<cached>(static_cast<int*>(mResult) + index, frame,
                                              cached ? mContext.mCache + index : nullptr);
    }
}
/** @brief Evaluate enabled material animations when the playback frame changes. */
void MaterialAnimObj::Calculate() {
    float lastFrame = mContext.mLastFrame;
    float frame = mFrameCtrlPointer->GetFrame();
    if (lastFrame == frame) {
        return;
    }
    int count = mBindTable.mAnimCount;
    if (mContext.IsCacheValid()) {
        int subBindIndex = 0;
        for (int i = 0; i < count; ++i) {
            if ((mBindTable.mEntries[i] & 0x40000000) == 0) {
                CalculateMaterialImpl<true>(&m_pMaterialAnims[i], frame, subBindIndex);
            }
        }
    } else {
        int subBindIndex = 0;
        for (int i = 0; i < count; ++i) {
            if ((mBindTable.mEntries[i] & 0x40000000) == 0) {
                CalculateMaterialImpl<false>(&m_pMaterialAnims[i], frame, subBindIndex);
            }
        }
    }
    mContext.mLastFrame = mFrameCtrlPointer->GetFrame();
}
/**
 * @brief Apply enabled material animation results to a bound model.
 * @param pModel Model matching the current material bindings; must not be null.
 */
void MaterialAnimObj::ApplyTo(ModelObj* pModel) const {
    int count = mBindTable.mAnimCount;
    int subBindIndex = 0;
    for (int i = 0; i < count; ++i) {
        u32 binding = mBindTable.mEntries[i];
        if ((binding & 0x80000000) == 0) {
            int target = binding & 0x7fff;
            MaterialObj* pMaterial = pModel->GetMaterial(target);
            const ResPerMaterialAnim* pAnim = &m_pMaterialAnims[i];
            ApplyTo(pMaterial, pAnim, subBindIndex);
            subBindIndex += pAnim->parameterCount + pAnim->textureCount;
            const u32* pVisibility;
            if (pAnim->visibilityCurve != 0xffff) {
                int resultIndex = pAnim->visibilityResultIndex;
                pVisibility = static_cast<const u32*>(mResult) + resultIndex;
            } else if (pAnim->visibilityBase != 0xffff) {
                pVisibility = &pAnim->constants[pAnim->visibilityBase].value;
            } else {
                continue;
            }
            pModel->SetMaterialVisible(target, *pVisibility != 0);
        }
    }
}
/**
 * @brief Apply animated parameter words, constants and texture selections to one material.
 * @param pMaterial Writable target material matching the sub-bindings.
 * @param pAnim Resource describing curve and constant destinations.
 * @param subBindIndex First parameter binding, followed by sampler bindings for this material.
 */
void MaterialAnimObj::ApplyTo(MaterialObj* pMaterial, const ResPerMaterialAnim* pAnim,
                              int subBindIndex) const {
    const u32* pResults = static_cast<const u32*>(mResult);
    int count = pAnim->parameterCount;
    int resultIndex = pAnim->shaderParamResultIndex;
    pResults = ResultAt<u32>(pResults, resultIndex);
    const ResShaderParamAnimInfo* pInfo = pAnim->parameters;
    for (int i = 0; i < count; ++i, ++pInfo, ++subBindIndex) {
        int target = m_pSubBindIndices[subBindIndex];
        if (target != 0xffff) {
            u8* pDestination = pMaterial->EditShaderParam<u8>(target);
            unsigned end = pInfo->firstCurve + pInfo->floatCount + pInfo->intCount;
            for (unsigned j = pInfo->firstCurve; j < end; ++j) {
                u32 offset = pAnim->curves[j].targetOffset;
                ParameterWord(pDestination, offset) = pResults[j];
            }
            end = pInfo->firstConstant + pInfo->constantCount;
            for (unsigned j = pInfo->firstConstant; j < end; ++j) {
                ParameterWord(pDestination, pAnim->constants[j].targetOffset) = pAnim->constants[j].value;
            }
        }
    }
    const int* pPatterns = static_cast<const int*>(mResult) + pAnim->texturePatternResultIndex;
    count = pAnim->textureCount;
    const ResTexturePatternAnimInfo* pTexture = pAnim->textures;
    int outputIndex = 0;
    for (int i = 0; i < count; ++i, ++pTexture, ++subBindIndex) {
        int target = m_pSubBindIndices[subBindIndex];
        if (target != 0xffff) {
            if (pTexture->curveIndex != 0xffff) {
                int value = pPatterns[outputIndex];
                if (value < 0) {
                    value = 0;
                } else {
                    int textureCount = m_pRes->GetTextureCount();
                    value = value < textureCount ? value : textureCount - 1;
                }
                pMaterial->SetTexture(target, GetTexture(value));
                ++outputIndex;
            } else {
                int baseIndex = pTexture->baseValueIndex;
                int value = pAnim->constants[baseIndex].value;
                pMaterial->SetTexture(target, GetTexture(value));
            }
        }
    }
}
/**
 * @brief Restore enabled animated materials and their original visibility.
 * @param pModel Model matching the current bindings; must not be null.
 */
void MaterialAnimObj::RevertTo(ModelObj* pModel) const {
    int count = mBindTable.mAnimCount;
    int subBindIndex = 0;
    for (int i = 0; i < count; ++i) {
        u32 binding = mBindTable.mEntries[i];
        if ((binding & 0x80000000) == 0) {
            int target = binding & 0x7fff;
            MaterialObj* pMaterial = pModel->GetMaterial(target);
            const ResPerMaterialAnim* pAnim = &m_pMaterialAnims[i];
            RevertTo(pMaterial, pAnim, subBindIndex);
            subBindIndex += pAnim->parameterCount + pAnim->textureCount;
            if (pAnim->visibilityCurve != 0xffff || pAnim->visibilityBase != 0xffff) {
                pModel->SetMaterialVisible(target, (pMaterial->GetResource()->ToData().flags & 1) != 0);
            }
        }
    }
}
/**
 * @brief Restore animated parameter words and textures from their material resource.
 * @param pMaterial Writable material whose resource supplies original values.
 * @param pAnim Animation identifying which parameter words and samplers to restore.
 * @param subBindIndex First parameter binding, followed by sampler bindings for this material.
 */
void MaterialAnimObj::RevertTo(MaterialObj* pMaterial, const ResPerMaterialAnim* pAnim,
                               int subBindIndex) const {
    const ResMaterial* pResource = pMaterial->GetResource();
    int count = pAnim->parameterCount;
    const ResShaderParamAnimInfo* pInfo = pAnim->parameters;
    for (int i = 0; i < count; ++i, ++pInfo, ++subBindIndex) {
        int target = m_pSubBindIndices[subBindIndex];
        if (target != 0xffff) {
            u8* pDestination = pMaterial->EditShaderParam<u8>(target);
            const u8* pSource = static_cast<const u8*>(pResource->ToData().pSourceParamData.Get()) +
                                pResource->GetShaderParam(target)->GetSrcOffset();
            unsigned end = pInfo->firstCurve + pInfo->floatCount + pInfo->intCount;
            for (unsigned j = pInfo->firstCurve; j < end; ++j) {
                u32 offset = pAnim->curves[j].targetOffset;
                ParameterWord(pDestination, offset) = ParameterWord(pSource, offset);
            }
            end = pInfo->firstConstant + pInfo->constantCount;
            for (unsigned j = pInfo->firstConstant; j < end; ++j) {
                u32 offset = pAnim->constants[j].targetOffset;
                ParameterWord(pDestination, offset) = ParameterWord(pSource, offset);
            }
        }
    }
    count = pAnim->textureCount;
    for (int i = 0; i < count; ++i, ++subBindIndex) {
        int target = m_pSubBindIndices[subBindIndex];
        if (target != 0xffff) {
            pMaterial->SetTexture(target, TextureRef(pResource->ToData().pTextureArray.Get()[target],
                                                     pResource->ToData().pTextureSlotArray.Get()[target]));
        }
    }
}
/** @brief Leave result storage untouched; evaluation supplies the animated values. */
void MaterialAnimObj::ClearResult() {}
} // namespace nn::g3d
