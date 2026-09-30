#include "Library/Shader/ForwardRendering/SkyboxDirector.hpp"

#include "utility/aglResParameter.h"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Resource/Resource.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GraphicsAreaDirector.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace al {
/**
 * Creates the skybox parameters.
 * @param pGraphicsSystemInfo Graphics system info.
 */
SkyboxDirector::SkyboxDirector(GraphicsSystemInfo* pGraphicsSystemInfo)
    : mGraphicsSystemInfo(pGraphicsSystemInfo) {
    mParamFilePath = new GraphicsParamFilePath("Skybox", "aglskybox");
    mIsEnableDefaultParam.init(false, "IsEnableDefaultParam", "Enable Edit", "",
                               &mDefaultParam.mParamObj);
    mParamIO.addObj(&mDefaultParam.mParamObj, "DefaultDirLit");

    for (s32 i = 0; i < mNamedParams.capacity(); i++) {
        NamedSkyboxParam* param = new NamedSkyboxParam();
        param->mName->format("", i);
        mNamedParams.pushBack(param);
        sead::FixedSafeString<64> name;
        name.format("DirLit%02d", i);
        mParamIO.addObj(&param->mParamObj, name);
    }
}

/**
 * Finishes the initialization.
 */
void SkyboxDirector::endInit() {}

/**
 * Clears the skybox requests.
 */
void SkyboxDirector::clearRequest() {}

/**
 * Looks up the skybox parameter of the current graphics area.
 */
void SkyboxDirector::execute() {
    GraphicsAreaDirector* areaDirector = mGraphicsSystemInfo->mGraphicsAreaDirector;

    if (!areaDirector) {
        return;
    }

    CurrentGraphicsAreaParam areaParam;
    areaDirector->getCurrentGraphicsAreaParam(&areaParam, static_cast<GraphicsAreaParamType>(1));
    const char* name = areaParam.mParamName;

    if (!name || isEqualString(name, "")) {
        return;
    }

    s32 num = mNamedParams.size();

    for (s32 i = 0; i < num; i++) {
        if (isEqualString(name, mNamedParams[i]->getName())) {
            break;
        }
    }
}

/**
 * Finds a skybox parameter by name.
 * @param pName Parameter name.
 * @return The parameter, or nullptr.
 */
NamedSkyboxParam* SkyboxDirector::findSkyboxParamByName(const char* pName) const {
    if (!pName || isEqualString(pName, "")) {
        return nullptr;
    }

    s32 num = mNamedParams.size();

    for (s32 i = 0; i < num; i++) {
        NamedSkyboxParam* param = mNamedParams[i];

        if (isEqualString(pName, param->getName())) {
            return param;
        }
    }

    return nullptr;
}

/**
 * Requests a skybox parameter.
 * @param priority Request priority.
 * @param step Interpolation step.
 * @param rParam Requested parameter.
 */
void SkyboxDirector::requestDirectionalLight(s32 priority, s32 step, const SkyboxParam& rParam) {}

/**
 * Loads the skybox parameters of a stage.
 * @param pResource Stage resource.
 * @param pStageName Stage name.
 */
void SkyboxDirector::initStageResource(const Resource* pResource, const char* pStageName) {
    StringTmp<256> path;
    mParamFilePath->makeBinaryPath(&path);
    bool isLoaded = false;

    if (pResource && pResource->isExistFile(path)) {
        const void* file = pResource->getOtherFile(path, nullptr);
        mParamIO.applyResParameterArchive(agl::utl::ResParameterArchive(file));
        isLoaded = true;
    }

    mIsLoadedParam = isLoaded;
}
}  // namespace al
