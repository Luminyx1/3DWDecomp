#include "Library/Light/DirectionalLightKeeper.hpp"

#include "utility/aglResParameter.h"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GraphicsAreaDirector.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace al {
/**
 * Gets the light direction.
 * @return The light direction.
 */
const sead::Vector3f& DirLightParam::getDirectionFrom() const {
    return mDirection->getDirection();
}

/**
 * Gets the specular light direction.
 * @return The specular light direction.
 */
const sead::Vector3f& DirLightParam::getSpcDirectionFrom() const {
    return mSpcDirection->getDirection();
}

/**
 * Creates and registers the directional light parameters.
 * @param rDir Initial light direction.
 * @param rSpcDir Initial specular light direction.
 * @param rColor Initial light color.
 * @param rSpcColor Initial specular color.
 * @param spcPower Initial specular power.
 */
void DirLightParam::init(const sead::Vector3f& rDir, const sead::Vector3f& rSpcDir,
                         const sead::Color4f& rColor, const sead::Color4f& rSpcColor,
                         f32 spcPower) {
    mDirection = new DirectionParam();
    mDirection->initializeDir(rDir, &mParamObj, "DirectionParam", "方向");
    mSpcDirection = new DirectionParam();
    mSpcDirection->initializeDir(rSpcDir, &mParamObj, "SpecularDirectionParam", "スペキュラ方向");
    mColor.init(rColor, "Color", "Color", &mParamObj);
    mSpcColor.init(rSpcColor, "SpecularColor", "SpecularColor", &mParamObj);
    mSpcPower.init(spcPower, "SpecularPower", "SpecularPower", "Min=1,Max=10000", &mParamObj);
}

/**
 * Creates a named directional light parameter with an empty name.
 */
NamedDirLightParam::NamedDirLightParam() {
    mName.init(sead::FixedSafeString<64>(""), "Name", "Name", getParamObj());
}

/**
 * Compares two directional light parameters approximately.
 * @param rOther Parameter to compare with.
 * @return Whether the parameters are nearly equal.
 */
bool DirLightParam::operator==(const DirLightParam& rOther) const {
    return isNearDirection(getDirectionFrom(), rOther.getDirectionFrom(), 0.01f) &&
           isNearDirection(getSpcDirectionFrom(), rOther.getSpcDirectionFrom(), 0.01f) &&
           isNear(*mColor, *rOther.mColor, 0.001f) &&
           isNear(*mSpcColor, *rOther.mSpcColor, 0.001f) &&
           isNearZero(*mSpcPower - *rOther.mSpcPower, 0.001f);
}

/**
 * Copies a directional light parameter.
 * @param rOther Parameter to copy.
 * @return This parameter.
 */
DirLightParam& DirLightParam::operator=(const DirLightParam& rOther) {
    mDirection->syncFromDirection(rOther.getDirectionFrom());
    mSpcDirection->syncFromDirection(rOther.getSpcDirectionFrom());
    *mColor = *rOther.mColor;
    *mSpcColor = *rOther.mSpcColor;
    *mSpcPower = *rOther.mSpcPower;
    return *this;
}

/**
 * Interpolates between two directional light parameters.
 * @param rA Start parameter.
 * @param rB End parameter.
 * @param rate Interpolation rate.
 */
void DirLightParam::interp(const DirLightParam& rA, const DirLightParam& rB, f32 rate) {
    mDirection->lerp(*rA.mDirection, *rB.mDirection, rate);
    mSpcDirection->lerp(*rA.mSpcDirection, *rB.mSpcDirection, rate);
    mColor.copyLerp(rA.mColor, rB.mColor, rate);
    mSpcColor.copyLerp(rA.mSpcColor, rB.mSpcColor, rate);
    mSpcPower.copyLerp(rA.mSpcPower, rB.mSpcPower, rate);
}

/**
 * Creates the default and named directional light parameters.
 * @param pGraphicsSystemInfo Graphics system info.
 */
DirectionalLightKeeper::DirectionalLightKeeper(GraphicsSystemInfo* pGraphicsSystemInfo)
    : mGraphicsSystemInfo(pGraphicsSystemInfo) {
    mParamFilePath = new GraphicsParamFilePath("DirectionalLight", "agldirlit");
    mIsEnableDefaultParam.init(false, "IsEnableDefaultParam", "Enable Edit",
                               mDefaultParam.getParamObj());
    mParamIO.addObj(mDefaultParam.getParamObj(), "DefaultDirLit");

    for (s32 i = 0; i < mNamedParams.capacity(); i++) {
        NamedDirLightParam* param = new NamedDirLightParam();
        param->getNameString().format("", i);
        mNamedParams.pushBack(param);
        sead::FixedSafeString<64> name;
        name.format("DirLit%02d", i);
        mParamIO.addObj(param->getParamObj(), name);
    }
}

/**
 * Ends initialization.
 */
void DirectionalLightKeeper::endInit() {
    mInterp.endInit();
}

/**
 * Gets the current light direction.
 * @return The current light direction.
 */
const sead::Vector3f& DirectionalLightKeeper::getLightDirFrom() const {
    return mInterp.getCurrentParam().getDirectionFrom();
}

/**
 * Gets the current specular light direction.
 * @return The current specular light direction.
 */
const sead::Vector3f& DirectionalLightKeeper::getSpecularLightDirFrom() const {
    return mInterp.getCurrentParam().getSpcDirectionFrom();
}

/**
 * Clears the light request of this frame.
 */
void DirectionalLightKeeper::clearRequest() {
    mInterp.clearRequest();
}

/**
 * Requests the light of the current graphics area and updates the interpolation.
 */
void DirectionalLightKeeper::execute() {
    GraphicsAreaDirector* areaDirector = mGraphicsSystemInfo->getGraphicsAreaDirector();

    if (areaDirector == nullptr) {
        if (*mIsEnableDefaultParam) {
            mInterp.requestParam(-1, 1, mDefaultParam);
        }
    } else {
        CurrentGraphicsAreaParam areaParam;
        areaDirector->getCurrentGraphicsAreaParam(&areaParam,
                                                  static_cast<GraphicsAreaParamType>(1));
        NamedDirLightParam* param = findDirLightParamByName(areaParam.mParamName);

        if (param != nullptr) {
            mInterp.requestParam(areaParam.mPriority, areaParam._14, *param);
        } else if (*mIsEnableDefaultParam) {
            mInterp.requestParam(-1, areaParam._14, mDefaultParam);
        }

        if (areaDirector->isLerpPaused()) {
            return;
        }
    }

    mInterp.updateInterp();
}

/**
 * Finds a named directional light parameter.
 * @param pName Parameter name.
 * @return The parameter, or nullptr.
 */
NamedDirLightParam* DirectionalLightKeeper::findDirLightParamByName(const char* pName) const {
    if (pName == nullptr || isEqualString(pName, "")) {
        return nullptr;
    }

    s32 num = mNamedParams.size();

    for (s32 i = 0; i < num; i++) {
        NamedDirLightParam* param = mNamedParams[i];

        if (isEqualString(pName, param->getName())) {
            return param;
        }
    }

    return nullptr;
}

/**
 * Requests a directional light.
 * @param priority Request priority.
 * @param step Interpolation step.
 * @param rParam Requested parameter.
 */
void DirectionalLightKeeper::requestDirectionalLight(s32 priority, s32 step,
                                                     const DirLightParam& rParam) {
    mInterp.requestParam(priority, step, rParam);
}

/**
 * Finds a named directional light parameter, falling back to the current one.
 * @param pName Parameter name.
 * @return The named parameter, or the current parameter.
 */
const DirLightParam* DirectionalLightKeeper::tryGetNamedOrCurrentDirLight(const char* pName) const {
    const DirLightParam* param = findDirLightParamByName(pName);

    if (param != nullptr) {
        return param;
    }

    return &mInterp.getCurrentParam();
}

/**
 * Finds a named directional light parameter, falling back to the default one.
 * @param pName Parameter name.
 * @return The named parameter, or the default parameter.
 */
const DirLightParam* DirectionalLightKeeper::tryGetNamedOrDefaultDirLight(const char* pName) const {
    if (pName == nullptr) {
        return &mDefaultParam;
    }

    const DirLightParam* param = findDirLightParamByName(pName);
    return param != nullptr ? param : &mDefaultParam;
}

/**
 * Gets a named directional light parameter by index.
 * @param index Parameter index.
 * @return The parameter, or nullptr if out of range.
 */
NamedDirLightParam* DirectionalLightKeeper::getDirLightByIndex(s32 index) {
    return mNamedParams.at(index);
}

/**
 * Loads the directional light parameters of a stage.
 * @param pResource Stage resource.
 * @param pStageName Stage name.
 */
void DirectionalLightKeeper::initStageResource(const Resource* pResource, const char* pStageName) {
    StringTmp<256> path;
    mParamFilePath->makeBinaryPath(&path);

    if (pResource == nullptr || !pResource->isExistFile(path)) {
        mIsLoaded = false;
        return;
    }

    const void* file = pResource->getOtherFile(path, nullptr);
    mParamIO.applyResParameterArchive(agl::utl::ResParameterArchive(file));
    mIsLoaded = true;
    mDefaultParam.getDirection()->syncToDirection();
    mDefaultParam.getSpcDirection()->syncToDirection();

    for (s32 i = 0; i < mNamedParams.size(); i++) {
        mNamedParams.unsafeAt(i)->getDirection()->syncToDirection();
        mNamedParams.unsafeAt(i)->getSpcDirection()->syncToDirection();
    }
}

}  // namespace al

namespace DirLightFunction {
/**
 * Gets the directional light keeper of an actor's scene.
 * @param pActor Actor.
 * @return The directional light keeper.
 */
al::DirectionalLightKeeper* getDirectionalLightKeeper(const al::LiveActor* pActor) {
    return pActor->getSceneInfo()->graphicsSystemInfo->getDirectionalLightKeeper();
}
}  // namespace DirLightFunction
