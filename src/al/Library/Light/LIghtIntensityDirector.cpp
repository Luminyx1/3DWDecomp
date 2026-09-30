#include "Library/Light/LightIntensityDirector.hpp"

#include <cmath>
#include "postfx/aglBloom.h"
#include "utility/aglParameterIO.h"
#include "utility/aglResParameter.h"

#include "Library/Math/MathUtil.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Project/Draw/GraphicsAreaDirector.hpp"

#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Registers the name parameter of a named graphics parameter.
 * @param pDefaultName Initial name.
 */
GraphicsNamedParamBase::GraphicsNamedParamBase(const char* pDefaultName) {
    mName = new agl::utl::Parameter<sead::FixedSafeString<64>>(
        sead::FixedSafeString<64>(pDefaultName), "name", "パラメータ名", this);
}

/**
 * Interpolates the parameters of two named parameters.
 * @param rA Start parameter.
 * @param rB End parameter.
 * @param rate Interpolation rate.
 */
void GraphicsNamedParamBase::interp(const GraphicsNamedParamBase& rA,
                                    const GraphicsNamedParamBase& rB, f32 rate) {
    copyLerp(rA, rB, rate);
}

/**
 * Compares the names of two named parameters.
 * @param rOther Parameter to compare with.
 * @return Whether both parameters have the same name.
 */
bool GraphicsNamedParamBase::operator==(const GraphicsNamedParamBase& rOther) const {
    return isEqualString(getName(), rOther.getName());
}

/**
 * Registers the exposure parameter.
 * @param isDefault Whether this is the default parameter.
 */
ExposureParam::ExposureParam(bool isDefault)
    : GraphicsNamedParamBase(isDefault ? "Default" : "※露光パラメータ名を入力してください") {
    mExposure = new agl::utl::Parameter<f32>(1.0f, "exposure", "露出(-:暗 0:標準 +:明)", this);
}

/**
 * Gets the parameter type.
 * @return The exposure parameter type.
 */
s32 ExposureParam::getParamType() const {
    return 3;
}

/**
 * Registers the bloom parameters.
 * @param isDefault Whether this is the default parameter.
 */
BloomNamedParam::BloomNamedParam(bool isDefault)
    : GraphicsNamedParamBase(isDefault ? "Default" : "※ブルーム名を入力してください") {
    mBloomParameter = new agl::pfx::BloomParameter();
    mBloomParameter->initialize(this, nullptr);
    mEnable = new agl::utl::Parameter<bool>(false, "enable", "有効", this);
    mReduceScale.init(1.0f, "ReduceScale", "ReduceScale", "Min=0.125f, Max=4.0f", this);
}

/**
 * Gets the parameter type.
 * @return The bloom parameter type.
 */
s32 BloomNamedParam::getParamType() const {
    return 0;
}

/**
 * Constructs the light intensity director.
 * @param pAreaObjDirector Area object director.
 * @param pPlayerHolder Player holder.
 */
LightIntensityDirector::LightIntensityDirector(AreaObjDirector* pAreaObjDirector,
                                               const PlayerHolder* pPlayerHolder)
    : mExposureAreaParam(new CurrentGraphicsAreaParam()), mLerpBloomParam(new BloomNamedParam(false)),
      mPrevLerpBloomParam(new BloomNamedParam(false)), mBloomAreaParam(new CurrentGraphicsAreaParam()),
      mAreaObjDirector(pAreaObjDirector) {}

/**
 * Creates the exposure and bloom parameters of every graphics area and loads the stage defaults.
 * @param pGraphicsAreaDirector Graphics area director.
 * @param pStageName Stage name.
 */
void LightIntensityDirector::initGraphicsAreaParam(GraphicsAreaDirector* pGraphicsAreaDirector,
                                                   const char* pStageName) {
    mGraphicsAreaDirector = pGraphicsAreaDirector;
    mParamNum = pGraphicsAreaDirector->getGraphicsAreaNum() + 1;

    mExposureParamIO = new agl::utl::IParameterIO();
    mExposureParams = new ExposureParam*[mParamNum];
    for (s32 i = 0; i < mParamNum; i++) {
        mExposureParams[i] = new ExposureParam(i == 0);
        mExposureParamIO->addObj(mExposureParams[i],
                                 StringTmp<64>("%s_%d", "exposure_param", i).cstr());
    }
    mDefaultExposureParam = mExposureParams[0];

    const void* exposureFile =
        tryFindStageParameterFileDesign(pStageName, "DefaultParam.baglexp", 1);
    if (exposureFile) {
        agl::utl::ResParameterArchive archive(exposureFile);
        mExposureParamIO->applyResParameterArchive(archive);
        mIsLoadedExposureParam = true;
    }
    for (s32 i = 0; i < mParamNum; i++) {
        if (isEqualString("※露光パラメータ名を入力してください", mExposureParams[i]->getName())) {
            mExposureParamIO->removeObj(mExposureParams[i]);
        }
    }

    mBloomParamIO = new agl::utl::IParameterIO();
    mBloomParams = new BloomNamedParam*[mParamNum];
    for (s32 i = 0; i < mParamNum; i++) {
        mBloomParams[i] = new BloomNamedParam(i == 0);
        mBloomParamIO->addObj(mBloomParams[i],
                              i == 0 ? sead::SafeString("bloom") :
                                       sead::SafeString(StringTmp<64>("%s_%d", "bloom_param", i).cstr()));
    }

    const void* bloomFile = tryFindStageParameterFileDesign(pStageName, "DefaultParam.baglblm", 1);
    if (bloomFile) {
        agl::utl::ResParameterArchive archive(bloomFile);
        mBloomParamIO->applyResParameterArchive(archive);
        mIsLoadedBloomParam = true;
    }
    mLerpBloomParam->copy(*mBloomParams[0]);
    mPrevLerpBloomParam->copy(*mBloomParams[0]);
    for (s32 i = 0; i < mParamNum; i++) {
        if (isEqualString("※ブルーム名を入力してください", mBloomParams[i]->getName())) {
            mBloomParamIO->removeObj(mBloomParams[i]);
        }
    }
}

/**
 * Finishes the initialization.
 */
void LightIntensityDirector::endInit() {
    mIsEndInit = true;
}

/**
 * Updates the exposure and bloom parameters of the current graphics area.
 */
void LightIntensityDirector::execute() {
    if (!mGraphicsAreaDirector) {
        return;
    }
    updateExposure();
    updateBloom();
}

/**
 * Updates the exposure parameters of the current graphics area.
 */
void LightIntensityDirector::updateExposure() {
    mGraphicsAreaDirector->getCurrentGraphicsAreaParam(mExposureAreaParam,
                                                       static_cast<GraphicsAreaParamType>(3));
    if (!mCurrentExposureParam || !mPrevExposureParam) {
        ExposureParam* param = findExposureParam(mExposureAreaParam->mParamName);
        mCurrentExposureParam = param;
        mPrevExposureParam = param;
        return;
    }
    ExposureParam* param = findExposureParam(mExposureAreaParam->mParamName);
    ExposureParam* prevParam = findExposureParam(mExposureAreaParam->mPrevParamName);
    if (param != mCurrentExposureParam || prevParam != mPrevExposureParam) {
        mPrevExposureParam = mCurrentExposureParam;
        mCurrentExposureParam = param;
    }
}

/**
 * Updates the bloom parameters of the current graphics area.
 */
void LightIntensityDirector::updateBloom() {
    mGraphicsAreaDirector->getCurrentGraphicsAreaParam(mBloomAreaParam,
                                                       static_cast<GraphicsAreaParamType>(0));
    if (!mCurrentBloomParam) {
        mCurrentBloomParam = findBloomParam(mBloomAreaParam->mParamName);
        return;
    }

    const char* currentName = mCurrentBloomParam->getName() ? mCurrentBloomParam->getName() : "";
    bool isChanged = false;
    if (!mBloomAreaParam->mIsNoParam) {
        const char* name = mBloomAreaParam->mParamName ? mBloomAreaParam->mParamName : "";
        isChanged = !isEqualString(currentName, name);
    }
    if (mBloomAreaParam->mIsNoParam || isChanged) {
        if (mPrevBloomParam) {
            mPrevLerpBloomParam->copy(*mLerpBloomParam);
        } else {
            mPrevLerpBloomParam->copy(*mCurrentBloomParam);
        }
        mPrevBloomParam = mCurrentBloomParam;
        mCurrentBloomParam = findBloomParam(mBloomAreaParam->mParamName);
        if (isChanged) {
            mLerpBloomParam->copy(*mCurrentBloomParam);
        } else {
            mLerpBloomParam->setApplied(false);
        }
    }

    if (mBloomAreaParam->mIsLerp) {
        BloomNamedParam* prevParam = mPrevLerpBloomParam;
        BloomNamedParam* currentParam = mCurrentBloomParam;
        BloomNamedParam* lerpParam = mLerpBloomParam;
        f32 prevIntensity = *prevParam->getBloomParameter()->mMain.mIntensity;
        f32 currentIntensity = *currentParam->getBloomParameter()->mMain.mIntensity;
        f32 rate = mBloomAreaParam->mRate;
        if (!(prevIntensity <= currentIntensity)) {
            rate = 1.0 - std::pow(1.0f - rate, 10);
        } else {
            rate = std::pow(rate, 10);
        }
        lerpParam->copyLerp(*prevParam, *currentParam, rate);
        f32 reduceScale = mLerpBloomParam->getReduceScale();
        if (reduceScale != reduceScale) {
            mLerpBloomParam->setReduceScale(1.0f);
        }
    } else if (!mLerpBloomParam->isApplied()) {
        mLerpBloomParam->copy(*mCurrentBloomParam);
        mLerpBloomParam->setApplied(true);
    }
}

/**
 * Gets the exposure of the current graphics area.
 * @return The exposure.
 */
f32 LightIntensityDirector::getExposure() const {
    if (mForceExposure > 0.0f) {
        return mForceExposure;
    }
    if (!mExposureParamIO) {
        return 1.0f;
    }
    if (mExposureAreaParam->mIsLerp) {
        if (mCurrentExposureParam && mPrevExposureParam) {
            return lerpValue(mExposureAreaParam->mRate, mPrevExposureParam->getExposure(),
                             mCurrentExposureParam->getExposure());
        }
        if (mCurrentExposureParam) {
            return mCurrentExposureParam->getExposure();
        }
        if (mPrevExposureParam) {
            return mPrevExposureParam->getExposure();
        }
        return mDefaultExposureParam->getExposure();
    }
    if (mCurrentExposureParam) {
        return mCurrentExposureParam->getExposure();
    }
    return mDefaultExposureParam->getExposure();
}

/**
 * Gets the exposure as a power of two.
 * @return The binary logarithm of the exposure.
 */
f32 LightIntensityDirector::getExposureExp() const {
    return sead::Mathf::log(getExposure()) * 1.442695f;
}

/**
 * Gets the bloom parameter of the current graphics area.
 * @return The bloom parameter, or nullptr if none was loaded.
 */
const BloomNamedParam* LightIntensityDirector::getCurrentParam() const {
    if (mForceBloomParam) {
        return mForceBloomParam;
    }
    if (!mIsLoadedBloomParam) {
        return nullptr;
    }
    if (mBloomAreaParam->mIsLerp) {
        return mLerpBloomParam;
    }
    return mCurrentBloomParam;
}

/**
 * Applies the current bloom parameter.
 * @param pBloom Bloom to apply to.
 * @param context Context index.
 */
void LightIntensityDirector::applyBloomParameter(agl::pfx::Bloom* pBloom, s32 context) const {
    const BloomNamedParam* param = getCurrentParam();
    if (param) {
        pBloom->copyParameter(context, *param);
    }
}

/**
 * Gets the bloom reduce scale of the current graphics area.
 * @return The reduce scale.
 */
f32 LightIntensityDirector::getCurrentReduceScale() const {
    const BloomNamedParam* param = getCurrentParam();
    if (param) {
        return param->getReduceScale();
    }
    return 1.0f;
}

/**
 * Finds an exposure parameter by name.
 * @param pName Parameter name.
 * @return The found parameter, or the default parameter.
 */
ExposureParam* LightIntensityDirector::findExposureParam(const char* pName) const {
    if (pName) {
        for (s32 i = 0; i < mParamNum; i++) {
            if (isEqualString(pName, mExposureParams[i]->getName())) {
                return mExposureParams[i];
            }
        }
    }
    return mDefaultExposureParam;
}

/**
 * Finds a bloom parameter by name.
 * @param pName Parameter name.
 * @return The found parameter, or the default parameter.
 */
BloomNamedParam* LightIntensityDirector::findBloomParam(const char* pName) const {
    if (!pName) {
        return mBloomParams[0];
    }
    for (s32 i = 1; i < mParamNum; i++) {
        if (isEqualString(pName, mBloomParams[i]->getName())) {
            return mBloomParams[i];
        }
    }
    return mBloomParams[0];
}
}  // namespace al
