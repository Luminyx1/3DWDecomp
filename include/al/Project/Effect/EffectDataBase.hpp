#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace al {
class EffectDataBase;
class Resource;
struct EffectUserInfo;

struct EffectDrawCategoryInfo {
    EffectDrawCategoryInfo();

    const char* mName;
    s32 mParticleNumMaxBit;
    s32 mStripeVertexNumBit;
    bool mIsEnableZSort;
    bool mIsAlwaysUpdateUbo;
    bool mIsScreenEffect;
};

static_assert(sizeof(EffectDrawCategoryInfo) == 0x18);

class EffectDataBase {
public:
    EffectDataBase(const char* pArchiveName);

    s32 getDrawCategoryNum() const { return mDrawCategoryNum; }
    const EffectDrawCategoryInfo& getDrawCategory(s32 index) const {
        return mDrawCategories[index];
    }

    s32 mUserNum;
    EffectUserInfo** mUsers;
    s32 mDrawCategoryNum;
    EffectDrawCategoryInfo* mDrawCategories;
};

static_assert(sizeof(EffectDataBase) == 0x20);
}  // namespace al

namespace alEffectDataBaseFunction {
al::EffectUserInfo* createEffectUserInfo(const al::EffectDataBase* pDataBase,
                                         const al::Resource* pResource,
                                         const sead::SafeString& rFileName);
}  // namespace alEffectDataBaseFunction
