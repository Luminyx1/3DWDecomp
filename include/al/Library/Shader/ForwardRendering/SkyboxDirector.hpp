#pragma once

#include <container/seadPtrArray.h>
#include <prim/seadSafeString.h>
#include "utility/aglParameter.h"
#include "utility/aglParameterIO.h"
#include "utility/aglParameterObj.h"

namespace al {
class GraphicsParamFilePath;
class GraphicsSystemInfo;
class Resource;

class SkyboxParam {
public:
    SkyboxParam();

    bool operator==(const SkyboxParam& rOther) const;
    SkyboxParam& operator=(const SkyboxParam& rOther);
    void interp(const SkyboxParam& rA, const SkyboxParam& rB, f32 rate);

    agl::utl::Parameter<sead::FixedSafeString<64>> mModelName;
    agl::utl::Parameter<sead::FixedSafeString<64>> mTextureName;
    agl::utl::Parameter<f32> mRotateDegreeY;
    agl::utl::ParameterObj mParamObj;
};

static_assert(sizeof(SkyboxParam) == 0x130);

class NamedSkyboxParam : public SkyboxParam {
public:
    NamedSkyboxParam();

    const char* getName() const { return mName->cstr(); }

    agl::utl::Parameter<sead::FixedSafeString<64>> mName;
};

static_assert(sizeof(NamedSkyboxParam) == 0x1a0);

class SkyboxDirector {
public:
    SkyboxDirector(GraphicsSystemInfo* pGraphicsSystemInfo);

    void endInit();
    void clearRequest();
    void execute();
    NamedSkyboxParam* findSkyboxParamByName(const char* pName) const;
    void requestDirectionalLight(s32 priority, s32 step, const SkyboxParam& rParam);
    void initStageResource(const Resource* pResource, const char* pStageName);

private:
    bool mIsLoadedParam = false;
    GraphicsSystemInfo* mGraphicsSystemInfo;
    SkyboxParam mDefaultParam;
    agl::utl::Parameter<bool> mIsEnableDefaultParam;
    sead::FixedPtrArray<NamedSkyboxParam, 64> mNamedParams;
    agl::utl::IParameterIO mParamIO;
    GraphicsParamFilePath* mParamFilePath;
};

static_assert(sizeof(SkyboxDirector) == 0x598);
}  // namespace al
