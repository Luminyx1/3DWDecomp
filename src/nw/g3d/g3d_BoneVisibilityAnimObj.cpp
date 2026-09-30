#include <nn/g3d/g3d_BoneVisibilityAnimObj.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResSkeleton.h>
#include <cstring>
#include <cstddef>

namespace nn::g3d {
void BoneVisibilityAnimObj::InitializeArgument::CalculateMemorySize() {
    int animations = animCount > 0 ? animCount : 0;
    int targets = animCount > 0 ? targetCount : 0;
    int bindings = targets < animations ? animations : targets;
    blocks[0].Initialize(((animations + 31) & ~31) / 8);
    blocks[1].Initialize(bindings * sizeof(u32));
    blocks[2].Initialize(0);
    blocks[2].size = cacheAvailable && cacheEnabled ? (curveCount * sizeof(AnimFrameCache) + 7) & ~size_t(7) : 0;
    memorySize = 0;
    memoryAlignment = 8;

    for (int i = 0; i < 3; ++i) {
        if (blocks[i].size) {
            size_t offset = (memorySize + 7) & ~size_t(7);
            memorySize = offset + blocks[i].size;
            memoryAlignment = 8;
            blocks[i].offset = offset;
        }
    }
}

// argument specifies capacities and caching; memory provides size bytes of working storage.
bool BoneVisibilityAnimObj::Initialize(const InitializeArgument& argument, void* memory, size_t size) {
    if (!argument.memoryAlignment) return false;

    if (argument.targetCount < 0) return false;

    if (argument.animCount < 0) return false;

    if (argument.curveCount < 0) return false;

    if (argument.memorySize > size) return false;
    int bindings = argument.GetBindCount();
    int curves = argument.curveCount;
    mWorkMemory = memory;
    mResource = nullptr;
    mBindTable.Initialize(argument.blocks[1].GetPointer<u32>(memory), bindings);
    mContext.Initialize(argument.blocks[2].GetPointer<AnimFrameCache>(memory), curves);
    mResult = argument.blocks[0].GetPointer(memory);
    mAnimCapacity = argument.animCount;
    return true;
}

// resource supplies curves, initial visibility bits, and playback bounds.
void BoneVisibilityAnimObj::SetResource(const ResBoneVisibilityAnim* resource) {
    mResource = resource;
    mCurves = resource->curves;
    mCurveCount = resource->curveCount;
    mBindTable.mFlags &= ~1;
    bool loop = resource->flags & 4;
    int frames = resource->frameCount;
    ResetFrameCtrl(frames, loop);
    mBindTable.mAnimCount = resource->animCount;
    mContext.SetCurveCount(resource->curveCount);
}

// model supplies the bone names to bind to the animation's named targets.
BindResult BoneVisibilityAnimObj::Bind(const ResModel* model) {
    const ResSkeleton* skeleton = model->ToData().pSkeleton.Get();
    const nn::util::ResDic* dictionary = skeleton->ToData().pBoneDic.Get();
    mBindTable.ClearAll(skeleton->GetBoneCount());
    BindResult result;
    int count = mBindTable.mAnimCount;
    const nn::util::BinPtrToString* names = mResource->names;

    for (int i = 0; i < count; ++i) {
        int target = dictionary->FindIndex(names[i].Get()->GetData());

        if (target >= 0) {
            mBindTable.mEntries[i] &= 0x3fff8000;
            mBindTable.mEntries[i] |= target & 0x7fff;
            mBindTable.mEntries[target] &= 0xc0007fff;
            mBindTable.mEntries[target] |= (i << 15) & 0x3fff8000;
            result.Merge(BindResult(BindResult::Flag_Success));
        } else result.Merge(BindResult(BindResult::Flag_Failure));
    }

    mBindTable.mFlags |= 1;
    mContext.Reset();
    BoneVisibilityAnimObj::ClearResult();
    return result;
}

void BoneVisibilityAnimObj::ClearResult() {
    size_t size = ((mBindTable.mAnimCount + 31) >> 5) * sizeof(u32);
    memcpy(mResult, mResource->baseValues, size);
}

// model supplies its resource for virtual dispatch to the resource-binding implementation.
BindResult BoneVisibilityAnimObj::Bind(const ModelObj* model) { BindResult result; result.Merge(Bind(model->GetResource())); return result; }
// model must match the resource's precomputed bone-binding indices.
void BoneVisibilityAnimObj::BindFast(const ResModel* model) {
    mBindTable.ClearAll(model->ToData().pSkeleton.Get()->GetBoneCount());
    mBindTable.BindAll(mResource->bindIndices);
    mBindTable.mFlags |= 1;
    mContext.Reset();
    BoneVisibilityAnimObj::ClearResult();
}

void BoneVisibilityAnimObj::Calculate() {
    float lastFrame = mContext.mLastFrame;
    float frame = mFrameCtrlPointer->GetFrame();

    if (lastFrame == frame) return;
    u32* result = static_cast<u32*>(mResult);

    if (!mContext.IsCacheValid()) {
        // Walk target fields at the curve stride; only enabled bindings need a curve evaluation.
        ptrdiff_t fieldOffset = offsetof(ResAnimCurve, targetOffset);

        for (ptrdiff_t i = 0; i < mCurveCount; ++i, fieldOffset += sizeof(ResAnimCurve)) {
            const u8* field = reinterpret_cast<const u8*>(mCurves) + fieldOffset;
            int target = *reinterpret_cast<const s32*>(field);

            if (!(mBindTable.mEntries[target] & 0x40000000)) {
                const ResAnimCurve* curve = reinterpret_cast<const ResAnimCurve*>(field - offsetof(ResAnimCurve, targetOffset));
                AnimFrameCache cache;
                bool visible = curve->EvaluateInt(frame, &cache) != 0;
                u32* word = &result[target >> 5];
                u32 mask = 1u << (target & 31);
                *word &= ~mask;
                *word |= static_cast<u32>(visible) << (target & 31);
            }
        }
    } else {
        for (int i = 0; i < mCurveCount; ++i) {
            const ResAnimCurve* curve = &mCurves[i];
            int target = curve->targetOffset;

            if (!(mBindTable.mEntries[target] & 0x40000000)) {
                bool visible = curve->EvaluateInt(frame, &mContext.mCache[i]) != 0;
                u32* word = &result[target >> 5];
                u32 mask = 1u << (target & 31);
                u32 value = *word;
                *word = (value & ~mask) | (static_cast<u32>(visible) << (target & 31));
            }
        }
    }

    mContext.mLastFrame = mFrameCtrlPointer->GetFrame();
}

// model receives the calculated visibility values for bindings with application enabled.
void BoneVisibilityAnimObj::ApplyTo(ModelObj* model) const {
    int count = mBindTable.mAnimCount;
    const u32* result = static_cast<const u32*>(mResult);

    for (int i = 0; i < count; ++i) {
        u32 binding = mBindTable.mEntries[i];

        if (!(binding & 0x80000000))
            model->SetBoneVisible(binding & 0x7fff, (result[static_cast<unsigned>(i) >> 5] & (1u << (i & 31))) != 0);
    }
}

// model receives its skeleton's default visibility for bindings with application enabled.
void BoneVisibilityAnimObj::RevertTo(ModelObj* model) const {
    int count = mBindTable.mAnimCount;
    const SkeletonObj* skeleton = model->GetSkeleton();

    for (int i = 0; i < count; ++i) {
        u32 binding = mBindTable.mEntries[i];

        if (!(binding & 0x80000000)) {
            int target = binding & 0x7fff;
            model->SetBoneVisible(target, skeleton->GetBone(target)->IsVisible());
        }
    }
}
}
