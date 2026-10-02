#include "Library/Anim/ModelAnimInterp.hpp"

#include <basis/seadNew.h>
#include <math/seadMatrix.h>
#include <nn/g3d/g3d_SkeletalAnimObj.h>
#include <nn/g3d/g3d_SkeletonObj.h>
#include <nn/util/util_MatrixApi.h>

#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * Allocates the saved pose and bind flag buffers for a skeleton.
 * @param pSkeleton The skeleton to interpolate.
 */
ModelAnimInterp::ModelAnimInterp(nn::g3d::SkeletonObj* pSkeleton) {
    mLocalMtxs = new (16) nn::g3d::LocalMtx[pSkeleton->GetBoneCount()];
    mBindFlags = new s32[pSkeleton->GetBoneCount()];
    mInfoHolder = new ModelAnimInterpInfoHolder();
}

/**
 * Advances the interpolation by one frame.
 */
void ModelAnimInterp::update() {
    if (mInterpFrame > 0) {
        mInterpFrame--;
    }
}

/**
 * Saves the current pose and starts an interpolation whose length depends on the animations.
 * @param pSkeleton The skeleton whose pose is saved.
 * @param pAnimName Name of the animation that starts.
 * @param pNextAnimName Name of the animation used to look up a specific interpolation length.
 * @param rAnimObjs The skeletal animation objects applied to the skeleton.
 */
void ModelAnimInterp::prepareAnimInterp(nn::g3d::SkeletonObj* pSkeleton, const char* pAnimName,
                                        const char* pNextAnimName,
                                        const SklAnimBuffer& rAnimObjs) {
    s32 interpFrame = mInfoHolder->getInterpFrame(pAnimName, pNextAnimName);
    prepareAnimInterp(pSkeleton, interpFrame, rAnimObjs);
}

/**
 * Saves the current pose and starts an interpolation.
 * @param pSkeleton The skeleton whose pose is saved.
 * @param interpFrame Length of the interpolation in frames.
 * @param rAnimObjs The skeletal animation objects applied to the skeleton.
 */
void ModelAnimInterp::prepareAnimInterp(nn::g3d::SkeletonObj* pSkeleton, s32 interpFrame,
                                        const SklAnimBuffer& rAnimObjs) {
    if (interpFrame <= 0) {
        prepareNoInterp();
        return;
    }

    mInterpFrameMax = interpFrame;
    mInterpFrame = interpFrame;
    const nn::g3d::LocalMtx* localMtxs = pSkeleton->GetLocalMtxArray();

    for (s32 i = 0; i < pSkeleton->GetBoneCount(); i++) {
        nn::g3d::LocalMtx localMtx = localMtxs[i];
        mLocalMtxs[i] = localMtx;
        mBindFlags[i] = -1;

        for (s32 j = 0; j < rAnimObjs.size(); j++) {
            if (rAnimObjs[j] != nullptr) {
                mBindFlags[i] = rAnimObjs[j]->GetBindFlagImpl(i);
            }
        }
    }
}

/**
 * Cancels the interpolation.
 */
void ModelAnimInterp::prepareNoInterp() {
    mInterpFrameMax = 0;
    mInterpFrame = 0;
}

/**
 * Reads the interpolation lengths of an archive.
 * @param pArcPath The archive path.
 * @param unused Unused.
 */
void ModelAnimInterp::initWithArcPath(const char* pArcPath, s32 unused) {
    mInfoHolder->initWithArcPath(pArcPath, unused);
}

/**
 * Blends the skeleton's current local matrices with the saved pose.
 * @param pSkeleton The skeleton to update.
 * @return Whether an interpolation is in progress.
 */
bool ModelAnimInterp::interpAnim(nn::g3d::SkeletonObj* pSkeleton) const {
    if (mInterpFrame == 0 || mInterpFrameMax == 0) {
        return false;
    }

    f32 rate = 1.0f - static_cast<f32>(mInterpFrame) / static_cast<f32>(mInterpFrameMax);
    nn::g3d::LocalMtx* localMtxs = pSkeleton->GetLocalMtxArray();

    for (s32 i = 0; i < pSkeleton->GetBoneCount(); i++) {
        if (mBindFlags[i] & nn::g3d::AnimObj::BindFlag_SkipApply) {
            continue;
        }

        nn::g3d::LocalMtx& localMtx = localMtxs[i];

        sead::Matrix34f mtx;
        nn::util::MatrixStore(reinterpret_cast<nn::util::FloatColumnMajor4x3*>(&mtx),
                              localMtx.mtx);
        sead::Matrix34f savedMtx;
        nn::util::MatrixStore(reinterpret_cast<nn::util::FloatColumnMajor4x3*>(&savedMtx),
                              mLocalMtxs[i].mtx);

        sead::Vector3f trans;
        mtx.getTranslation(trans);
        sead::Vector3f savedTrans;
        savedMtx.getTranslation(savedTrans);
        trans.set(savedTrans.x + rate * (trans.x - savedTrans.x),
                  savedTrans.y + rate * (trans.y - savedTrans.y),
                  savedTrans.z + rate * (trans.z - savedTrans.z));

        sead::Matrix34f interpMtx;
        sead::Matrix34CalcCommon<f32>::slerpTo(interpMtx, savedMtx, mtx, rate);
        interpMtx.setTranslation(trans);
        nn::util::MatrixLoad(&localMtx.mtx,
                             reinterpret_cast<const nn::util::FloatColumnMajor4x3&>(interpMtx));

        float32x4_t savedScale = mLocalMtxs[i].scale._v;
        localMtx.scale._v = vfmaq_f32(savedScale, vdupq_n_f32(rate),
                                      vsubq_f32(localMtx.scale._v, savedScale));

        const u32 rotTransZero = nn::g3d::ResBone::Flag_RotTransZero;
        if ((mLocalMtxs[i].flag & rotTransZero) != rotTransZero) {
            localMtx.flag &= ~rotTransZero;
        }

        const u32 scaleOne = nn::g3d::ResBone::Flag_ScaleOne;
        if ((mLocalMtxs[i].flag & scaleOne) != scaleOne) {
            localMtx.flag &= ~scaleOne;
        }
    }

    return true;
}

/**
 * Constructs an empty holder with no interpolation lengths.
 */
__attribute__((noinline)) ModelAnimInterpInfoHolder::ModelAnimInterpInfoHolder()
    : mFrameMap(new FrameMap()), mDefaultFrame(0) {}

/**
 * Reads the interpolation lengths from the AnimInterp BYAML of an archive.
 * @param pArcPath The archive path.
 * @param unused Unused.
 */
__attribute__((noinline)) void ModelAnimInterpInfoHolder::initWithArcPath(const char* pArcPath,
                                                                         s32 unused) {
    Resource* resource = findOrCreateResource(pArcPath, nullptr);

    if (!resource->isExistByml("AnimInterp")) {
        return;
    }

    ByamlIter iter(resource->getByml("AnimInterp"));
    s32 size = iter.getSize();
    mFrameMap->allocBuffer(size, nullptr);
    iter.tryGetIntByKey(&mDefaultFrame, "Default");

    for (s32 i = 0; i < iter.getSize(); i++) {
        const char* animName = nullptr;
        s32 frame = -1;

        if (tryGetByamlKeyAndIntByIndex(&animName, &frame, iter, i) &&
            !isEqualString("Default", animName)) {
            mFrameMap->insert(animName, frame);
        }
    }
}

/**
 * Gets the interpolation length to use when switching to an animation.
 * @param pAnimName Name of the animation.
 * @param pNextAnimName Name looked up first, may be nullptr.
 * @return The registered interpolation length, or the default length.
 */
__attribute__((noinline)) s32
ModelAnimInterpInfoHolder::getInterpFrame(const char* pAnimName, const char* pNextAnimName) const {
    FrameMap::Node* node = nullptr;

    if (pNextAnimName != nullptr) {
        node = mFrameMap->find(pNextAnimName);
    }

    if (node == nullptr) {
        node = mFrameMap->find(pAnimName);
    }

    if (node != nullptr && node->value() != -1) {
        return node->value();
    }

    return mDefaultFrame;
}

}  // namespace al
