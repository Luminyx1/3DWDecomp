#include "Project/Anim/InitResourceDataAnim.hpp"

#include <nn/g3d/g3d_BoneVisibilityAnimObj.h>
#include <nn/g3d/g3d_ResFile.h>
#include <nn/g3d/g3d_ResSkeletalAnim.h>

#include "Library/Resource/Resource.hpp"
#include "Project/Anim/AnimInfo.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {

/**
 * Gets a skeletal animation of a resource's graphics file.
 * @param pResource The resource.
 * @param index Index of the animation.
 * @return The skeletal animation.
 */
inline const nn::g3d::ResSkeletalAnim* getSklAnim(const Resource* pResource, s32 index) {
    return &pResource->getResFile()->ToData().pSkeletalAnimArray.Get()[index];
}

/**
 * Gets a material animation of a resource's graphics file.
 * @param pResource The resource.
 * @param index Index of the animation.
 * @return The material animation.
 */
inline const nn::g3d::ResMaterialAnim* getMatAnim(const Resource* pResource, s32 index) {
    return &pResource->getResFile()->ToData().pMaterialAnimArray.Get()[index];
}

/**
 * Gets a bone visibility animation of a resource's graphics file.
 * @param pResource The resource.
 * @param index Index of the animation.
 * @return The bone visibility animation.
 */
inline const nn::g3d::ResBoneVisibilityAnim* getVisAnim(const Resource* pResource, s32 index) {
    return &pResource->getResFile()->ToData().pBoneVisibilityAnimArray.Get()[index];
}

/**
 * Checks whether a table already holds an animation with a name.
 * @param pTable The table.
 * @param pName The animation name.
 * @return Whether the animation is already registered.
 */
inline bool isExistAnimInfo(const AnimInfoTable* pTable, const char* pName) {
    for (s32 i = 0; i < pTable->getInfoCount(); i++) {
        if (isEqualString(pTable->getResInfo(i).name, pName)) {
            return true;
        }
    }

    return false;
}

/**
 * Checks whether a table already holds an animation with the name of an animation resource.
 * @param pTable The table.
 * @param pAnim The animation resource.
 * @return Whether the animation is already registered.
 */
template <typename T>
inline bool isExistAnimInfo(const AnimInfoTable* pTable, const T* pAnim) {
    for (s32 i = 0; i < pTable->getInfoCount(); i++) {
        if (isEqualString(pTable->getResInfo(i).name, pAnim->GetName())) {
            return true;
        }
    }

    return false;
}

/**
 * Counts the material animations of a resource whose names end with a suffix.
 * @param pResource The resource.
 * @param pSuffix The name suffix of the material animation type.
 * @return Number of matching animations.
 */
inline s32 countMatAnim(const Resource* pResource, const char* pSuffix) {
    s32 animNum = pResource->getResFile()->GetMaterialAnimCount();
    s32 count = 0;

    for (s32 i = 0; i < animNum; i++) {
        if (isEndWithString(getMatAnim(pResource, i)->GetName(), pSuffix)) {
            count++;
        }
    }

    return count;
}

/**
 * Adds the material animations of a resource whose names end with a suffix, without the suffix.
 * @param pTable The table to add to.
 * @param pResource The resource.
 * @param pSuffix The name suffix of the material animation type.
 * @param isCheckExist Whether to skip animations that are already registered.
 */
__attribute__((noinline)) void addMatAnimInfo(AnimInfoTable* pTable, const Resource* pResource,
                                              const char* pSuffix, bool isCheckExist) {
    s32 animNum = pResource->getResFile()->GetMaterialAnimCount();

    for (s32 i = 0; i < animNum; i++) {
        const nn::g3d::ResMaterialAnim* anim = getMatAnim(pResource, i);

        if (!isEndWithString(anim->GetName(), pSuffix)) {
            continue;
        }

        char name[256] = {};
        removeStringFromEnd(name, sizeof(name), pSuffix, anim->GetName());

        if (isCheckExist && isExistAnimInfo(pTable, name)) {
            continue;
        }

        pTable->add(name, const_cast<nn::g3d::ResMaterialAnim*>(anim), anim->GetFrameCount(),
                    anim->IsLooped());
    }
}

/**
 * Creates the table of one material animation type.
 * @param pResources The resources to collect animations from.
 * @param resourceNum Number of resources.
 * @param pSuffix The name suffix of the material animation type.
 * @param isCheckExist Whether to skip animations that are already registered.
 * @return The sorted table, or nullptr if there is no matching animation.
 */
__attribute__((always_inline)) inline AnimInfoTable*
createMatAnimInfoTable(Resource* const* pResources, s32 resourceNum, const char* pSuffix,
                       bool isCheckExist) {
    s32 animNum = 0;

    for (s32 i = 0; i < resourceNum; i++) {
        animNum += countMatAnim(pResources[i], pSuffix);
    }

    if (animNum <= 0) {
        return nullptr;
    }

    AnimInfoTable* table = new AnimInfoTable(animNum);

    for (s32 i = 0; i < resourceNum; i++) {
        addMatAnimInfo(table, pResources[i], pSuffix, isCheckExist);
    }

    table->sort();
    return table;
}

/**
 * Creates the skeletal animation table.
 * @param pResources The resources to collect animations from.
 * @param resourceNum Number of resources.
 * @param isCheckExist Whether to skip animations that are already registered.
 * @return The sorted table, or nullptr if there is no skeletal animation.
 */
inline AnimInfoTable* createSklAnimInfoTable(Resource* const* pResources, s32 resourceNum,
                                             bool isCheckExist) {
    s32 animNum = 0;

    for (s32 i = 0; i < resourceNum; i++) {
        animNum += pResources[i]->getResFile()->GetSkeletalAnimCount();
    }

    if (animNum == 0) {
        return nullptr;
    }

    AnimInfoTable* table = new AnimInfoTable(animNum);

    for (s32 i = 0; i < resourceNum; i++) {
        s32 resAnimNum = pResources[i]->getResFile()->GetSkeletalAnimCount();

        for (s32 j = 0; j < resAnimNum; j++) {
            const nn::g3d::ResSkeletalAnim* anim = getSklAnim(pResources[i], j);

            if (isCheckExist && isExistAnimInfo(table, anim)) {
                continue;
            }

            table->add(anim->GetName(), const_cast<nn::g3d::ResSkeletalAnim*>(anim),
                       anim->GetFrameCount(), anim->IsLooped());
        }
    }

    table->sort();
    return table;
}

/**
 * Creates the bone visibility animation table.
 * @param pResources The resources to collect animations from.
 * @param resourceNum Number of resources.
 * @param isCheckExist Whether to skip animations that are already registered.
 * @return The sorted table, or nullptr if there is no bone visibility animation.
 */
inline AnimInfoTable* createVisAnimInfoTable(Resource* const* pResources, s32 resourceNum,
                                             bool isCheckExist) {
    s32 animNum = 0;

    for (s32 i = 0; i < resourceNum; i++) {
        animNum += pResources[i]->getResFile()->GetBoneVisibilityAnimCount();
    }

    if (animNum == 0) {
        return nullptr;
    }

    AnimInfoTable* table = new AnimInfoTable(animNum);

    for (s32 i = 0; i < resourceNum; i++) {
        s32 resAnimNum = pResources[i]->getResFile()->GetBoneVisibilityAnimCount();

        for (s32 j = 0; j < resAnimNum; j++) {
            const nn::g3d::ResBoneVisibilityAnim* anim = getVisAnim(pResources[i], j);

            if (isCheckExist && isExistAnimInfo(table, anim)) {
                continue;
            }

            table->add(anim->GetName(), const_cast<nn::g3d::ResBoneVisibilityAnim*>(anim),
                       anim->GetFrameCount(), anim->IsLooped());
        }
    }

    table->sort();
    return table;
}

}  // namespace

/**
 * Collects the animations of up to three resources into animation tables.
 * @param pModelRes The model resource.
 * @param pAnimRes The animation resource, may be nullptr.
 * @param pOtherRes An additional resource taking priority over the others, may be nullptr.
 * @return The animation data, or nullptr if no resource holds any animation.
 */
InitResourceDataAnim* InitResourceDataAnim::tryCreate(Resource* pModelRes, Resource* pAnimRes,
                                                      Resource* pOtherRes) {
    Resource* resources[3] = {pModelRes, nullptr, nullptr};
    s32 resourceNum = 0;

    if (pOtherRes != nullptr) {
        resources[resourceNum++] = pOtherRes;
    }

    resources[resourceNum++] = pModelRes;

    if (pAnimRes != nullptr && pAnimRes != pModelRes) {
        resources[resourceNum++] = pAnimRes;
    }

    bool isExistResFile = false;

    for (s32 i = 0; i < resourceNum; i++) {
        if (resources[i]->getResFile() != nullptr) {
            isExistResFile = true;
            break;
        }
    }

    if (!isExistResFile) {
        return nullptr;
    }

    AnimInfoTable* sklTable = createSklAnimInfoTable(resources, resourceNum, pOtherRes != nullptr);

    AnimInfoTable* mclTable = createMatAnimInfoTable(resources, resourceNum, "_fcl",
                                                     pOtherRes != nullptr);
    AnimInfoTable* mtsTable = createMatAnimInfoTable(resources, resourceNum, "_fts",
                                                     pOtherRes != nullptr);
    AnimInfoTable* mtpTable = createMatAnimInfoTable(resources, resourceNum, "_ftp",
                                                     pOtherRes != nullptr);

    AnimInfoTable* visTable = createVisAnimInfoTable(resources, resourceNum, pOtherRes != nullptr);

    if (sklTable == nullptr && mclTable == nullptr && mtsTable == nullptr &&
        mtpTable == nullptr && visTable == nullptr) {
        return nullptr;
    }

    return new InitResourceDataAnim(pModelRes, sklTable, mclTable, mtsTable, mtpTable, visTable);
}

/**
 * Constructs the animation data from its animation tables.
 * @param pResource The resource the animations belong to (unused).
 * @param pSklAnimInfoTable Skeletal animation table.
 * @param pMclAnimInfoTable Material color animation table.
 * @param pMtsAnimInfoTable Texture SRT animation table.
 * @param pMtpAnimInfoTable Texture pattern animation table.
 * @param pVisAnimInfoTable Bone visibility animation table.
 */
InitResourceDataAnim::InitResourceDataAnim(Resource* pResource,
                                           const AnimInfoTable* pSklAnimInfoTable,
                                           const AnimInfoTable* pMclAnimInfoTable,
                                           const AnimInfoTable* pMtsAnimInfoTable,
                                           const AnimInfoTable* pMtpAnimInfoTable,
                                           const AnimInfoTable* pVisAnimInfoTable)
    : mSklAnimInfoTable(const_cast<AnimInfoTable*>(pSklAnimInfoTable)),
      mMclAnimInfoTable(const_cast<AnimInfoTable*>(pMclAnimInfoTable)),
      mMtsAnimInfoTable(const_cast<AnimInfoTable*>(pMtsAnimInfoTable)),
      mMtpAnimInfoTable(const_cast<AnimInfoTable*>(pMtpAnimInfoTable)),
      mVisAnimInfoTable(const_cast<AnimInfoTable*>(pVisAnimInfoTable)) {}

}  // namespace al
