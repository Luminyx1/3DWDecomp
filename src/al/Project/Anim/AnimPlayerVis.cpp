#include "Project/Anim/AnimPlayerSimple.hpp"

#include <basis/seadNew.h>
#include <nn/g3d/g3d_BoneVisibilityAnimObj.h>
#include <nn/g3d/g3d_ModelObj.h>

#include "Project/Anim/AnimInfo.hpp"
#include "Project/Anim/InitResourceDataAnim.hpp"

namespace al {

/**
 * Creates a bone visibility animation player if the model has visibility animations.
 * @param pInfo Initialization info.
 * @return The player, or nullptr if there is no visibility animation.
 */
AnimPlayerVis* AnimPlayerVis::tryCreate(const AnimPlayerInitInfo* pInfo) {
    if (pInfo->mInitResourceDataAnim == nullptr) {
        return nullptr;
    }

    if (pInfo->mInitResourceDataAnim->getVisAnimInfoTable() == nullptr) {
        return nullptr;
    }

    AnimPlayerVis* player = new AnimPlayerVis();
    player->init(pInfo);
    return player;
}

/**
 * Constructs a player without an animation.
 */
AnimPlayerVis::AnimPlayerVis() = default;

/**
 * Creates the bone visibility animation object for the model.
 * @param pInfo Initialization info.
 */
void AnimPlayerVis::init(const AnimPlayerInitInfo* pInfo) {
    mModelAnim->mModelObj = pInfo->mModelObj;
    AnimInfoTable* table = pInfo->mInitResourceDataAnim->getVisAnimInfoTable();
    mInfoTable = table;

    nn::g3d::BoneVisibilityAnimObj* animObj = new nn::g3d::BoneVisibilityAnimObj();
    nn::g3d::BoneVisibilityAnimObj::Builder builder;
    builder.Reserve(pInfo->mModelObj->GetResource());

    for (s32 i = 0; i < table->getInfoCount(); i++) {
        builder.Reserve(
            static_cast<const nn::g3d::ResBoneVisibilityAnim*>(table->getResInfo(i).resAnim));
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
void AnimPlayerVis::setAnimToModel(const AnimResInfo* pInfo) {
    if (pInfo == nullptr) {
        return;
    }

    nn::g3d::BoneVisibilityAnimObj* animObj =
        static_cast<nn::g3d::BoneVisibilityAnimObj*>(mModelAnim->mAnimObj);
    animObj->SetResource(static_cast<const nn::g3d::ResBoneVisibilityAnim*>(pInfo->resAnim));
    animObj->Bind(mModelAnim->mModelObj);
}

}  // namespace al
