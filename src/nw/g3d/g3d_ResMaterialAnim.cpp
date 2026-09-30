#include <nn/g3d/g3d_ResMaterialAnim.h>
#include <cstring>

namespace nn::g3d {
// result receives curve values at frame; indices skips unbound parameters; cache retains intervals when cached.
template <bool cached>
void ResPerMaterialAnim::EvaluateShaderParamAnim(void* result, float frame, const u16* indices, AnimFrameCache* cache) const {
    int count = parameterCount;
    const ResAnimCurve* allCurves = curves;
    const ResShaderParamAnimInfo* info = parameters;
    for (int i = 0; i < count; ++i, ++info) {
        if (indices[i] == 0xffff) continue;
        int first = info->firstCurve;
        int integers = info->intCount;
        const ResAnimCurve* curve = &allCurves[first];
        int floats = info->floatCount;
        int* intOutput = static_cast<int*>(result) + first;
        for (int j = 0; j < integers; ++j, ++intOutput) {
            if (cached) { *intOutput = curve->EvaluateInt(frame, cache); }
            else { AnimFrameCache temporary; *intOutput = curve->EvaluateInt(frame, &temporary); }
            ++curve;
            if (cached) ++cache;
        }
        float* floatOutput = static_cast<float*>(result) + (first + integers);
        for (int j = 0; j < floats; ++j, ++floatOutput) {
            if (cached) { *floatOutput = curve->EvaluateFloat(frame, cache); }
            else { AnimFrameCache temporary; *floatOutput = curve->EvaluateFloat(frame, &temporary); }
            ++curve;
            if (cached) ++cache;
        }
    }
}
// result receives bound texture indices at frame; indices skips unbound samplers; cache retains intervals when cached.
template <bool cached>
void ResPerMaterialAnim::EvaluateTexturePatternAnim(int* result, float frame, const u16* indices, AnimFrameCache* cache) const {
    int count = textureCount;
    const ResTexturePatternAnimInfo* info = textures;
    int output = 0;
    for (int i = 0; i < count; ++i, ++info) {
        if (indices[i] == 0xffff || info->curveIndex == 0xffff) continue;
        if (cached) result[output] = curves[info->curveIndex].EvaluateInt(frame, &cache[output]);
        else { const ResAnimCurve* curve = &curves[info->curveIndex]; AnimFrameCache temporary; result[output] = curve->EvaluateInt(frame, &temporary); }
        ++output;
    }
}
// result receives visibility at frame; cache retains the curve interval when cached.
template <bool cached>
void ResPerMaterialAnim::EvaluateVisibilityAnim(int* result, float frame, AnimFrameCache* cache) const {
    int index = visibilityCurve;
    if (index == 0xffff) return;
    const ResAnimCurve* curve = &curves[index];
    if (cached) *result = curve->EvaluateInt(frame, cache);
    else { AnimFrameCache temporary; *result = curve->EvaluateInt(frame, &temporary); }
}
template void ResPerMaterialAnim::EvaluateShaderParamAnim<true>(void*, float, const u16*, AnimFrameCache*) const;
template void ResPerMaterialAnim::EvaluateShaderParamAnim<false>(void*, float, const u16*, AnimFrameCache*) const;
template void ResPerMaterialAnim::EvaluateTexturePatternAnim<true>(int*, float, const u16*, AnimFrameCache*) const;
template void ResPerMaterialAnim::EvaluateTexturePatternAnim<false>(int*, float, const u16*, AnimFrameCache*) const;
template void ResPerMaterialAnim::EvaluateVisibilityAnim<true>(int*, float, AnimFrameCache*) const;
template void ResPerMaterialAnim::EvaluateVisibilityAnim<false>(int*, float, AnimFrameCache*) const;
// material supplies the shader-parameter and sampler names to resolve and retain.
BindResult ResPerMaterialAnim::PreBind(const ResMaterial* material) {
    BindResult result;
    int count = parameterCount;
    ResShaderParamAnimInfo* info = parameters;
    for (int i = 0; i < count; ++i, ++info) {
        int index = material->FindShaderParamIndex(info->name.Get()->GetData());
        info->bindIndex = index;
        if (index == -1) result.Merge(BindResult(BindResult::Flag_Failure));
        else result.Merge(BindResult(BindResult::Flag_Success));
    }
    count = textureCount;
    ResTexturePatternAnimInfo* texture = textures;
    for (int i = 0; i < count; ++i, ++texture) {
        int index = material->FindSamplerIndex(texture->name.Get()->GetData());
        texture->bindIndex = index;
        if (index == -1) result.Merge(BindResult(BindResult::Flag_Failure));
        else result.Merge(BindResult(BindResult::Flag_Success));
    }
    return result;
}
// material supplies shader-parameter and sampler names to check without retaining bindings.
BindResult ResPerMaterialAnim::BindCheck(const ResMaterial* material) const {
    BindResult result;
    int count = parameterCount;
    const ResShaderParamAnimInfo* info = parameters;
    for (int i = 0; i < count; ++i, ++info) {
        int index = material->FindShaderParamIndex(info->name.Get()->GetData());
        if (index == -1) result.Merge(BindResult(BindResult::Flag_Failure));
        else result.Merge(BindResult(BindResult::Flag_Success));
    }
    count = textureCount;
    const ResTexturePatternAnimInfo* texture = textures;
    for (int i = 0; i < count; ++i, ++texture) {
        int index = material->FindSamplerIndex(texture->name.Get()->GetData());
        if (index == -1) result.Merge(BindResult(BindResult::Flag_Failure));
        else result.Merge(BindResult(BindResult::Flag_Success));
    }
    return result;
}
// model supplies named materials; retain it and the successful parameter/sampler bindings.
BindResult ResMaterialAnim::PreBind(const ResModel* model) {
    boundModel = model;
    BindResult result;
    int count = materialAnimCount;
    u16* indices = bindIndices;
    for (int i = 0; i < count; ++i) {
        ResPerMaterialAnim* anim = &materialAnims[i];
        const ResMaterial* material = model->FindMaterial(anim->name.Get()->GetData());
        if (material != nullptr) {
            indices[i] = material->GetIndex();
            if (anim->visibilityCurve != 0xffff || anim->visibilityBase != 0xffff) result.Merge(BindResult(BindResult::Flag_Success));
            result.Merge(anim->PreBind(material));
        } else {
            indices[i] = 0xffff;
            result.Merge(BindResult(BindResult::Flag_Failure));
        }
    }
    return result;
}
// model supplies named materials to check without changing stored bindings.
BindResult ResMaterialAnim::BindCheck(const ResModel* model) const {
    BindResult result;
    int count = materialAnimCount;
    for (int i = 0; i < count; ++i) {
        const ResPerMaterialAnim* anim = &materialAnims[i];
        const ResMaterial* material = model->FindMaterial(anim->name.Get()->GetData());
        if (material != nullptr) {
            if (anim->visibilityCurve != 0xffff || anim->visibilityBase != 0xffff) result.Merge(BindResult(BindResult::Flag_Success));
            result.Merge(anim->BindCheck(material));
        } else result.Merge(BindResult(BindResult::Flag_Failure));
    }
    return result;
}
// callback resolves missing texture views and descriptors; user is passed to that callback.
BindResult ResMaterialAnim::BindTexture(TextureBindCallback callback, void* user) {
    BindResult result;
    int count = textureCount;
    for (int i = 0; i < count; ++i) {
        if ((GetTextureView(i) != nullptr) && GetTextureDescriptorSlot(i) != TextureRef::InvalidDescriptorSlot) continue;
        TextureRef texture = callback(GetTextureName(i), user);
        ForceBindTexture(i, texture);
        if ((texture.GetTextureView() == nullptr) || texture.GetDescriptorSlot() == TextureRef::InvalidDescriptorSlot) result.Merge(BindResult(BindResult::Flag_Failure));
        else result.Merge(BindResult(BindResult::Flag_Success));
    }
    return result;
}
// texture replaces every binding whose stored name equals name.
bool ResMaterialAnim::ForceBindTexture(const TextureRef& texture, const char* name) {
    bool found = false;
    int count = textureCount;
    for (int i = 0; i < count; ++i) {
        if (strcmp(GetTextureName(i), name) == 0) { ForceBindTexture(i, texture); found = true; }
    }
    return found;
}
void ResMaterialAnim::ReleaseTexture() {
    int count = textureCount;
    for (int i = 0; i < count; ++i) ReleaseTexture(i);
}
// buffer supplies size writable bytes for baked samples; an empty request already succeeds.
bool ResMaterialAnim::BakeCurve(void* buffer, size_t size) {
    if (!size) return true;
    if ((buffer == nullptr) || bakedSize > size) return false;
    u8* output = static_cast<u8*>(buffer);
    int count = materialAnimCount;
    for (int i = 0; i < count; ++i) {
        ResPerMaterialAnim* anim = &materialAnims[i];
        int curves = anim->curveCount;
        for (int j = 0; j < curves; ++j) {
            ResAnimCurve* curve = &anim->curves[j];
            size_t bytes = curve->flags & 0x40 ? curve->CalculateBakedIntSize() : curve->CalculateBakedFloatSize();
            if (curve->flags & 0x40) curve->BakeInt(output, bytes);
            else curve->BakeFloat(output, bytes);
            output += bytes;
        }
    }
    flags |= 1;
    return true;
}
void* ResMaterialAnim::ResetCurve() {
    if (!(flags & 1)) return nullptr;
    void* buffer = nullptr;
    bool found = false;
    int count = materialAnimCount;
    for (int i = 0; i < count; ++i) {
        ResPerMaterialAnim* anim = &materialAnims[i];
        int curves = anim->curveCount;
        for (int j = 0; j < curves; ++j) {
            ResAnimCurve* curve = &anim->curves[j];
            int type = curve->flags & 0x70;
            if (!found && (type == 0x20 || type == 0x70 || type == 0x50)) { buffer = curve->keys; found = true; }
            curve->Reset();
        }
    }
    flags ^= 1;
    return buffer;
}
void ResMaterialAnim::Reset() {
    boundModel = nullptr;
    int count = materialAnimCount;
    u16* indices = bindIndices;
    for (int i = 0; i < count; ++i) {
        ResPerMaterialAnim* anim = &materialAnims[i];
        int parameters = anim->parameterCount;
        ResShaderParamAnimInfo* info = anim->parameters;
        for (int j = 0; j < parameters; ++j, ++info) info->bindIndex = 0xffff;
        int textures = anim->textureCount;
        ResTexturePatternAnimInfo* texture = anim->textures;
        for (int j = 0; j < textures; ++j, ++texture) texture->bindIndex = 0xff;
        indices[i] = 0xffff;
    }
    if (!(flags & 1)) return;
    count = materialAnimCount;
    for (int i = 0; i < count; ++i) {
        ResPerMaterialAnim* anim = &materialAnims[i];
        int curves = anim->curveCount;
        for (int j = 0; j < curves; ++j) anim->curves[j].Reset();
    }
    flags ^= 1;
}
}
