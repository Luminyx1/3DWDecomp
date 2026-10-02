#include "Project/Anim/AnimPlayerSimple.hpp"

#include <agl/g3d/aglNW4FToNN.h>
#include <attributes.h>
#include <basis/seadNew.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResMaterialAnim.h>
#include <nn/gfx/gfx_ResTextureData.h>

#include "Library/Resource/Resource.hpp"
#include "Project/Anim/AnimInfo.hpp"
#include "Project/Anim/InitResourceDataAnim.hpp"

namespace al {

namespace {

/**
 * Gets the material animation table matching an animation type.
 * @param pInfo Initialization info.
 * @param matType Material animation type (0: mcl, 1: mtp, 2: mts).
 * @return Table of the animations, or nullptr for an unknown type.
 */
inline AnimInfoTable* getMatAnimInfoTable(const AnimPlayerInitInfo* pInfo, s32 matType) {
    AnimInfoTable* table = nullptr;

    switch (matType) {
    case 0:
        table = pInfo->mInitResourceDataAnim->getMclAnimInfoTable();
        break;
    case 1:
        table = pInfo->mInitResourceDataAnim->getMtpAnimInfoTable();
        break;
    case 2:
        table = pInfo->mInitResourceDataAnim->getMtsAnimInfoTable();
        break;
    }

    return table;
}

/**
 * Creates a texture reference for a texture resource.
 * @param pTexture Texture resource.
 * @return Reference to the texture's view and descriptor slot.
 */
inline nn::g3d::TextureRef makeTextureRef(const nn::gfx::ResTexture* pTexture) {
    const nn::gfx::ResTextureData& rData = pTexture->ToData();
    return nn::g3d::TextureRef(static_cast<const nn::gfx::TextureView*>(rData.pTextureView.Get()),
                               rData.userDescriptorSlot.value);
}

}  // namespace

/**
 * Creates a material animation player if the model has animations of a type.
 * @param pInfo Initialization info.
 * @param matType Material animation type (0: mcl, 1: mtp, 2: mts).
 * @return The player, or nullptr if there is no animation of that type.
 */
AnimPlayerMat* AnimPlayerMat::tryCreate(const AnimPlayerInitInfo* pInfo, s32 matType) {
    if (pInfo->mInitResourceDataAnim == nullptr) {
        return nullptr;
    }

    AnimPlayerMat* player = nullptr;

    if (getMatAnimInfoTable(pInfo, matType) != nullptr) {
        player = new AnimPlayerMat(matType);
        player->init(pInfo);
    }

    return player;
}

/**
 * Creates the material animation object for the model.
 * @param pInfo Initialization info.
 */
void AnimPlayerMat::init(const AnimPlayerInitInfo* pInfo) {
    mModelAnim->mModelObj = pInfo->mModelObj;
    AnimInfoTable* table = getMatAnimInfoTable(pInfo, mMatType);
    mInfoTable = table;
    mModelResource = pInfo->mModelResource;
    mAnimResource = pInfo->mAnimResource;

    nn::g3d::MaterialAnimObj* animObj = new nn::g3d::MaterialAnimObj();
    nn::g3d::MaterialAnimObj::Builder builder;
    builder.Reserve(pInfo->mModelObj->GetResource());

    for (s32 i = 0; i < table->getInfoCount(); i++) {
        builder.Reserve(static_cast<const nn::g3d::ResMaterialAnim*>(table->getResInfo(i).resAnim));
    }

    builder.CalculateMemorySize();
    size_t size = builder.GetWorkMemorySize();
    u8* buffer = new (8) u8[size];
    builder.Build(animObj, buffer, size);
    mModelAnim->mAnimObj = animObj;
}

/**
 * Binds an animation to the model.
 * @param pInfo Animation to bind.
 */
void AnimPlayerMat::setAnimToModel(const AnimResInfo* pInfo) {
    if (pInfo == nullptr) {
        return;
    }

    nn::g3d::MaterialAnimObj* animObj =
        static_cast<nn::g3d::MaterialAnimObj*>(mModelAnim->mAnimObj);
    animObj->SetResource(static_cast<const nn::g3d::ResMaterialAnim*>(pInfo->resAnim));
    animObj->Bind(mModelAnim->mModelObj);

    if (mMatType != 1 || mAnimResource == mModelResource) {
        return;
    }

    const nn::g3d::ResMaterialAnim* resAnim = animObj->GetResource();

    for (s32 i = 0; i < resAnim->GetTextureCount(); i++) {
        if (animObj->GetTexture(i).IsValid()) {
            continue;
        }

        nn::gfx::ResTexture* texture = agl::g3d::ResFile::GetTexture(
            mModelResource->getResFile(), resAnim->GetTextureName(i));

        if (texture != nullptr) {
            animObj->SetTexture(i, makeTextureRef(texture));
        }
    }
}

/**
 * Constructs a player without an animation.
 * Not inlined into AnimPlayerMat::tryCreate in the original binary.
 */
NOINLINE AnimPlayerSimple::AnimPlayerSimple() {
    mModelAnim = new AnimPlayerModelAnim();
}

/**
 * Starts an animation.
 * @param pName Name of the animation.
 */
void AnimPlayerSimple::startAnim(const char* pName) {
    mPlayingAnim = mInfoTable->findAnimInfo(pName);
    setAnimToModel(mPlayingAnim);
    applyTo();
    _10 = true;
    _11 = true;
}

/**
 * Advances the animation by one step.
 */
void AnimPlayerSimple::update() {
    if (!_11 || _10) {
        return;
    }

    mModelAnim->mAnimObj->GetFrameCtrl().UpdateFrame();
}

/**
 * Clears the result of the animation.
 */
void AnimPlayerSimple::clearAnim() {
    mModelAnim->mAnimObj->ClearResult();
}

/**
 * Gets the current frame of the animation.
 * @return Current frame.
 */
f32 AnimPlayerSimple::getAnimFrame() const {
    return mModelAnim->mAnimObj->GetFrameCtrl().GetFrame();
}

/**
 * Sets the current frame of the animation.
 * @param frame New frame.
 */
void AnimPlayerSimple::setAnimFrame(f32 frame) {
    mModelAnim->mAnimObj->GetFrameCtrl().SetFrame(frame);
    applyTo();
    _10 = true;
    _11 = true;
}

/**
 * Gets the last frame of the playing animation.
 * @return Last frame.
 */
f32 AnimPlayerSimple::getAnimFrameMax() const {
    return mModelAnim->mAnimObj->GetFrameCtrl().GetEndFrame();
}

/**
 * Gets the last frame of an animation.
 * @param pName Name of the animation.
 * @return Last frame.
 */
f32 AnimPlayerSimple::getAnimFrameMax(const char* pName) const {
    return mInfoTable->findAnimInfo(pName)->frameMax;
}

/**
 * Gets the playback rate of the animation.
 * @return Frames advanced per step.
 */
f32 AnimPlayerSimple::getAnimFrameRate() const {
    return mModelAnim->mAnimObj->GetFrameCtrl().GetStep();
}

/**
 * Sets the playback rate of the animation.
 * @param rate Frames advanced per step.
 */
void AnimPlayerSimple::setAnimFrameRate(f32 rate) {
    mModelAnim->mAnimObj->GetFrameCtrl().SetStep(rate);
    applyTo();
    _11 = true;
}

/**
 * Checks whether an animation exists.
 * @param pName Name of the animation.
 * @return Whether the animation exists.
 */
bool AnimPlayerSimple::isAnimExist(const char* pName) const {
    return mInfoTable->tryFindAnimInfo(pName) != nullptr;
}

/**
 * Checks whether the animation reached its last frame.
 * @return Whether the animation ended, or true if nothing is playing.
 */
bool AnimPlayerSimple::isAnimEnd() const {
    if (mPlayingAnim == nullptr) {
        return true;
    }

    const nn::g3d::AnimFrameCtrl& frameCtrl = mModelAnim->mAnimObj->GetFrameCtrl();
    return frameCtrl.GetEndFrame() <= frameCtrl.GetFrame();
}

/**
 * Checks whether the playing animation is played only once.
 * @return Whether the animation does not loop.
 */
bool AnimPlayerSimple::isAnimOneTime() const {
    return mModelAnim->mAnimObj->GetFrameCtrl().GetPlayPolicy() ==
           nn::g3d::AnimFrameCtrl::PlayOneTime;
}

/**
 * Checks whether an animation is played only once.
 * @param pName Name of the animation.
 * @return Whether the animation does not loop.
 */
bool AnimPlayerSimple::isAnimOneTime(const char* pName) const {
    return !mInfoTable->findAnimInfo(pName)->isLoopAnim;
}

/**
 * Checks whether an animation is playing.
 * @return Whether an animation is playing.
 */
bool AnimPlayerSimple::isAnimPlaying() const {
    return mPlayingAnim != nullptr;
}

/**
 * Gets the name of the playing animation.
 * @return Name of the animation, or an empty string if nothing is playing.
 */
const char* AnimPlayerSimple::getPlayingAnimName() const {
    if (mPlayingAnim != nullptr) {
        return mPlayingAnim->name;
    }

    return "";
}

/**
 * Applies the animation, and detaches it once a one-time animation has ended.
 * @return Whether the animation was applied.
 */
bool AnimPlayerSimple::calcNeedUpdateAnimNext() {
    if (!_11) {
        return false;
    }

    applyTo();

    if (getAnimFrameRate() <= 0.0f || (isAnimOneTime() && isAnimEnd())) {
        setAnimToModel(nullptr);
        _11 = false;
    }

    return true;
}

/**
 * Calculates the animation and applies it to the model.
 */
void AnimPlayerSimple::applyTo() {
    nn::g3d::ModelAnimObj* animObj = mModelAnim->mAnimObj;

    if (!animObj->IsBound()) {
        return;
    }

    animObj->Calculate();
    mModelAnim->mAnimObj->ApplyTo(mModelAnim->mModelObj);
}

/**
 * Constructs a player without an animation table.
 * Originally its own translation unit (AnimPlayerBase.cpp), hence never inlined.
 */
NOINLINE AnimPlayerBase::AnimPlayerBase() = default;

}  // namespace al
