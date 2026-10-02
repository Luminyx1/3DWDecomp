#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadStrTreeMap.h>

namespace nn::g3d {
struct LocalMtx;
class SkeletalAnimObj;
class SkeletonObj;
}  // namespace nn::g3d

namespace al {

/// Holds the per-animation interpolation frame counts read from an archive's AnimInterp BYAML.
class ModelAnimInterpInfoHolder {
public:
    using FrameMap = sead::StrTreeMap<64, s32>;

    ModelAnimInterpInfoHolder();

    void initWithArcPath(const char* pArcPath, s32 unused);
    s32 getInterpFrame(const char* pAnimName, const char* pNextAnimName) const;

private:
    FrameMap* mFrameMap;
    s32 mDefaultFrame;
};

static_assert(sizeof(ModelAnimInterpInfoHolder) == 0x10);

/// Interpolates skeleton local matrices from a saved pose when switching skeletal animations.
class ModelAnimInterp {
public:
    using SklAnimBuffer = sead::Buffer<nn::g3d::SkeletalAnimObj*>;

    ModelAnimInterp(nn::g3d::SkeletonObj* pSkeleton);

    void update();
    void prepareAnimInterp(nn::g3d::SkeletonObj* pSkeleton, const char* pAnimName,
                           const char* pNextAnimName, const SklAnimBuffer& rAnimObjs);
    void prepareAnimInterp(nn::g3d::SkeletonObj* pSkeleton, s32 interpFrame,
                           const SklAnimBuffer& rAnimObjs);
    void prepareNoInterp();
    void initWithArcPath(const char* pArcPath, s32 unused);
    bool interpAnim(nn::g3d::SkeletonObj* pSkeleton) const;

private:
    nn::g3d::LocalMtx* mLocalMtxs = nullptr;
    s32* mBindFlags = nullptr;
    ModelAnimInterpInfoHolder* mInfoHolder = nullptr;
    s32 mInterpFrameMax = 0;
    s32 mInterpFrame = 0;
};

static_assert(sizeof(ModelAnimInterp) == 0x20);

}  // namespace al
