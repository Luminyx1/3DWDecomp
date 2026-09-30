#include <nn/g3d/g3d_ResSkeletalAnim.h>
#include <cstring>

namespace nn::g3d {
static_assert(sizeof(ResBoneAnim) == 0x38, "Bone animation resource size");
static_assert(sizeof(ResSkeletalAnim) == 0x50, "Skeletal animation resource size");
static const nn::util::Float3 DefaultScale = {1.0f, 1.0f, 1.0f};
static const nn::util::Float4 DefaultRotate = {0.0f, 0.0f, 0.0f, 1.0f};

// result receives constant channels; bone supplies missing channels, or identity defaults when null.
void ResBoneAnim::Initialize(BoneAnimResult* result, const ResBone* bone) const {
    u32 flag = flags;
    const u8* data = static_cast<const u8*>(baseValues);
    if (flag & 8) {
        std::memcpy(&result->scale, data, 12);
        data += 12;
    } else result->scale = bone ? bone->GetScale() : DefaultScale;
    if (flag & 16) {
        std::memcpy(&result->rotate, data, 16);
        data += 16;
    } else result->rotate = bone ? bone->GetRotateQuat() : DefaultRotate;
    if (flag & 32) std::memcpy(&result->translate, data, 12);
    else result->translate = bone ? bone->GetTranslate() : *reinterpret_cast<const nn::util::Float3*>(&DefaultRotate);
    result->flags = flag & 0x0f800000;
    if (bone) result->flags |= bone->GetRotateMode();
}
// result receives curve samples at frame; each sample uses a temporary interval cache.
void ResBoneAnim::Evaluate(BoneAnimResult* result, float frame) const {
    int count = curveCount;
    for (int i = 0; i < count; ++i) {
        const ResAnimCurve* curve = &curves[i];
        u32 offset = curve->targetOffset;
        AnimFrameCache temporary;
        *reinterpret_cast<float*>(reinterpret_cast<u8*>(result) + offset) = curve->EvaluateFloat(frame, &temporary);
    }
}
// result receives curve samples at frame; cache stores one interval per curve.
void ResBoneAnim::Evaluate(BoneAnimResult* result, float frame, AnimFrameCache* cache) const {
    int count = curveCount;
    for (int i = 0; i < count; ++i) {
        const ResAnimCurve* curve = &curves[i];
        u32 offset = curve->targetOffset;

        *reinterpret_cast<float*>(reinterpret_cast<u8*>(result) + offset) = curve->EvaluateFloat(frame, &cache[i]);
    }
}
// skeleton supplies the named bones; retain it and populate the per-animation bind indices.
BindResult ResSkeletalAnim::PreBind(const ResSkeleton* skeleton) {
    boundSkeleton = skeleton;
    BindResult result;
    int count = boneAnimCount;
    u16* indices = bindIndices;
    for (int i = 0; i < count; ++i) {
        int index = skeleton->FindBoneIndex(boneAnims[i].name.Get()->GetData());
        if (index >= 0) {
            indices[i] = index;
            result.Merge(BindResult(BindResult::Flag_Success));
        } else {
            indices[i] = 0xffff;
            result.Merge(BindResult(BindResult::Flag_Failure));
        }
    }
    return result;
}
// skeleton supplies named bones to check; resource bindings remain unchanged by this query.
BindResult ResSkeletalAnim::BindCheck(const ResSkeleton* skeleton) const {
    BindResult result;
    int count = boneAnimCount;
    for (int i = 0; i < count; ++i) {
        int index = skeleton->FindBoneIndex(boneAnims[i].name.Get()->GetData());
        if (index >= 0) result.Merge(BindResult(BindResult::Flag_Success));
        else result.Merge(BindResult(BindResult::Flag_Failure));
    }
    return result;
}
// buffer supplies size writable bytes; an empty request already succeeds.
bool ResSkeletalAnim::BakeCurve(void* buffer, size_t size) {
    if (!size) return true;
    if (!buffer || bakedSize > size) return false;
    u8* output = static_cast<u8*>(buffer);
    int count = boneAnimCount;
    for (int i = 0; i < count; ++i) {
        ResBoneAnim* anim = &boneAnims[i];
        int curves = anim->curveCount;
        for (int j = 0; j < curves; ++j) {
            ResAnimCurve* curve = &anim->curves[j];
            size_t bytes = curve->CalculateBakedFloatSize();
            curve->BakeFloat(output, bytes);
            output += bytes;
        }
    }
    flags |= 1;
    return true;
}
void* ResSkeletalAnim::ResetCurve() {
    if (!(flags & 1)) return nullptr;
    void* buffer = nullptr;
    bool found = false;
    int count = boneAnimCount;
    for (int i = 0; i < count; ++i) {
        ResBoneAnim* anim = &boneAnims[i];
        int curves = anim->curveCount;
        for (int j = 0; j < curves; ++j) {
            ResAnimCurve* curve = &anim->curves[j];
            if (((curve->flags & 0x70) == 0x20) & !found) {
                buffer = curve->keys;
                found = true;
            }
            curve->Reset();
        }
    }
    flags ^= 1;
    return buffer;
}
void ResSkeletalAnim::Reset() {
    boundSkeleton = nullptr;
    int count = boneAnimCount;
    u16* indices = bindIndices;
    for (int i = 0; i < count; ++i) indices[i] = 0xffff;
    if (!(flags & 1)) return;
    count = boneAnimCount;
    for (int i = 0; i < count; ++i) {
        ResBoneAnim* anim = &boneAnims[i];
        int curves = anim->curveCount;
        for (int j = 0; j < curves; ++j) anim->curves[j].Reset();
    }
    flags ^= 1;
}
}
