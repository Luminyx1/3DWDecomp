#include "Project/Draw/StressParam.hpp"

#include <agl/frame_control/aglGPUStressChecker.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Project/Draw/GraphicsQualityController.hpp"
#include "Project/Draw/GraphicsStressDirector.hpp"

namespace {
using namespace al;

NERVE_DECL(GraphicsQualityController, Wait)
NERVE_DECL(GraphicsQualityController, Reduce)
NERVE_DECL(GraphicsQualityController, Recover)

GraphicsQualityControllerNrvWait NrvGraphicsQualityControllerWait;
GraphicsQualityControllerNrvReduce NrvGraphicsQualityControllerReduce;
GraphicsQualityControllerNrvRecover NrvGraphicsQualityControllerRecover;

GraphicsStressDirector::StressInfo sStressInfos[6] = {
    {1920, 1080, 1280, 720, 5, {152, 142, 129, 118, 107}, {85, 79, 72, 66, 60},
     {10.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f}},
    {1422, 800, 1280, 720, 2, {152, 142, 129, 118, 107}, {85, 79, 72, 66, 60},
     {10.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f}},
    {1600, 900, 1280, 720, 3, {152, 142, 129, 118, 107}, {85, 79, 72, 66, 60},
     {10.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f}},
    {1920, 1080, 1280, 720, 5, {148, 137, 125, 114, 102}, {83, 76, 69, 63, 57},
     {10.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f}},
    {1920, 1080, 1280, 720, 5, {147, 134, 122, 109, 96}, {82, 75, 68, 61, 54},
     {10.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f}},
    {1920, 1080, 1280, 720, 0, {160, 160, 160, 160, 160}, {90, 90, 90, 90, 90},
     {10.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f}},
};
}  // namespace

namespace al {

/**
 * Initializes all stress parameters with their default values.
 */
void StressParam::init() {
    mScreenWidthScale.init(160, "ScreenWidthScale", "画面幅", "Min=125,Max=160", &mParamObj);
    mScreenHeightScale.init(90, "ScreenHeightScale", "画面高さ", "Min=62,Max=90", &mParamObj);
    mIsUsing16BitDepth.init(false, "IsUsing16BitDepth", "１６ビットデプスを使うか", &mParamObj);
    mIsUsingLppSpcMask.init(true, "IsUsingLppSpcMask", "ライトプリパスでスペキュラマスクを使うか",
                            &mParamObj);
    mIsClearGBufferViewDepth.init(false, "IsClearGBufferViewDepth", "ビューデプスＧバッファクリア",
                                  &mParamObj);
    mIsClearLightBuffer.init(false, "IsClearLightBuffer", "ライトバッファクリア", &mParamObj);
    mIsClearGBufferViewNrm.init(false, "IsClearGBufferViewNrm", "法線Ｇバッファクリア", &mParamObj);
    mIsLppLight.init(false, "IsLppLight", "ライトプリパス軽量", &mParamObj);
    mAntiAliasingType.init(0, "AntiAliasingType", "アンチエイリアスタイプ", "Min=0, Max=2", &mParamObj);
    mAntiAliasDetectEdgeQuality.init(0, "AntiAliasDetectEdgeQuality", "FXAAのエッジ抽出品質",
                                     "Min=0, Max=2", &mParamObj);
    mReduceQualityPercentage.init(93.0f, "ReduceQualityPercentage",
                                  "何パーセントまで上がったらクオリティ下げるか？",
                                  "Min=50.0f, Max=98.0f", &mParamObj);
}

/**
 * Compares all parameter values with another stress parameter.
 * @param rOther Parameter to compare with.
 * @return Whether all values are equal.
 */
bool StressParam::operator==(const StressParam& rOther) const {
    if (*mScreenWidthScale != *rOther.mScreenWidthScale)
        return false;
    if (*mScreenHeightScale != *rOther.mScreenHeightScale)
        return false;
    if (*mIsUsing16BitDepth != *rOther.mIsUsing16BitDepth)
        return false;
    if (*mIsUsingLppSpcMask != *rOther.mIsUsingLppSpcMask)
        return false;
    if (*mIsClearLightBuffer != *rOther.mIsClearLightBuffer)
        return false;
    if (*mIsClearGBufferViewNrm != *rOther.mIsClearGBufferViewNrm)
        return false;
    if (*mIsClearGBufferViewDepth != *rOther.mIsClearGBufferViewDepth)
        return false;
    if (*mIsLppLight != *rOther.mIsLppLight)
        return false;
    if (*mAntiAliasingType != *rOther.mAntiAliasingType)
        return false;
    if (*mAntiAliasDetectEdgeQuality != *rOther.mAntiAliasDetectEdgeQuality)
        return false;
    return *mReduceQualityPercentage == *rOther.mReduceQualityPercentage;
}

/**
 * Copies all parameter values from another stress parameter.
 * @param rOther Parameter to copy from.
 * @return This parameter.
 */
StressParam& StressParam::operator=(const StressParam& rOther) {
    *mScreenWidthScale = *rOther.mScreenWidthScale;
    *mScreenHeightScale = *rOther.mScreenHeightScale;
    *mIsUsing16BitDepth = *rOther.mIsUsing16BitDepth;
    *mIsUsingLppSpcMask = *rOther.mIsUsingLppSpcMask;
    *mIsClearLightBuffer = *rOther.mIsClearLightBuffer;
    *mIsClearGBufferViewNrm = *rOther.mIsClearGBufferViewNrm;
    *mIsClearGBufferViewDepth = *rOther.mIsClearGBufferViewDepth;
    *mAntiAliasingType = *rOther.mAntiAliasingType;
    *mReduceQualityPercentage = *rOther.mReduceQualityPercentage;
    *mIsLppLight = *rOther.mIsLppLight;
    *mAntiAliasDetectEdgeQuality = *rOther.mAntiAliasDetectEdgeQuality;
    return *this;
}

/**
 * Interpolates between two stress parameters. Scales and the reduce percentage are blended,
 * everything else switches at the halfway point.
 * @param rA Parameter at rate 0.
 * @param rB Parameter at rate 1.
 * @param rate Interpolation rate.
 */
void StressParam::interp(const StressParam& rA, const StressParam& rB, f32 rate) {
    *mScreenWidthScale =
        (s32)lerpValueNew((f32)*rA.mScreenWidthScale, (f32)*rB.mScreenWidthScale, rate);
    *mScreenHeightScale =
        (s32)lerpValueNew((f32)*rA.mScreenHeightScale, (f32)*rB.mScreenHeightScale, rate);
    mReduceQualityPercentage.copyLerp(rA.mReduceQualityPercentage, rB.mReduceQualityPercentage,
                                      rate);

    mIsUsing16BitDepth.copy(rate < 0.5f ? rA.mIsUsing16BitDepth : rB.mIsUsing16BitDepth);
    mIsUsingLppSpcMask.copy(rate < 0.5f ? rA.mIsUsingLppSpcMask : rB.mIsUsingLppSpcMask);
    mIsClearLightBuffer.copy(rate < 0.5f ? rA.mIsClearLightBuffer : rB.mIsClearLightBuffer);
    mIsClearGBufferViewNrm.copy(rate < 0.5f ? rA.mIsClearGBufferViewNrm : rB.mIsClearGBufferViewNrm);
    mIsClearGBufferViewDepth.copy(rate < 0.5f ? rA.mIsClearGBufferViewDepth : rB.mIsClearGBufferViewDepth);
    mAntiAliasingType.copy(rate < 0.5f ? rA.mAntiAliasingType : rB.mAntiAliasingType);
    mIsLppLight.copy(rate < 0.5f ? rA.mIsLppLight : rB.mIsLppLight);
    mAntiAliasDetectEdgeQuality.copyLerp(rA.mAntiAliasDetectEdgeQuality,
                                         rB.mAntiAliasDetectEdgeQuality, rate);
}

/**
 * Creates the stress director and its quality controller.
 * @param pInfo Graphics system info.
 */
GraphicsStressDirector::GraphicsStressDirector(GraphicsSystemInfo* pInfo)
    : GraphicsParamRequestInterpKeeper<StressParam>(pInfo, 6, "GraphicsStress", "aglstress",
                                                    "StressParam") {
    mQualityController = new GraphicsQualityController(sStressInfos[0].recoverPercents);
    mIsForceStressOff = false;
    mIsFullResolution = false;
    mIsIgnoreQualityControl = false;
    mIsSingleMode = false;
    mIsForceDisable = false;
    mStressInfoModeBeforeForceOff = -1;
    mStressInfo = &sStressInfos[0];
}

/**
 * Calculates the horizontal render buffer size.
 * @return Buffer width in pixels.
 */
s32 GraphicsStressDirector::getBufferSizeX() const {
    s32 scale;
    if (mIsIgnoreQualityControl) {
        scale = getCurrentParam().getScreenWidthScale();
    } else {
        f32 rates[5] = {mQualityController->getRate(1), mQualityController->getRate(2),
                        mQualityController->getRate(3), mQualityController->getRate(4),
                        mQualityController->getRate(5)};

        s32 limit = 160;
        for (s32 i = mStressInfo->levelNum - 1; i >= 0; i--) {
            if (rates[i] == 0.0f) {
                limit = mStressInfo->widthScales[i];
                break;
            }
        }

        scale = sead::Mathi::min(getCurrentParam().getScreenWidthScale(), limit);
    }

    if (mIsFullResolution)
        return scale * 240 / 160 * 8 * mStressInfo->baseWidth / 1920;
    return scale * 8 * mStressInfo->renderWidth / 1280;
}

/**
 * Calculates the vertical render buffer size.
 * @return Buffer height in pixels.
 */
s32 GraphicsStressDirector::getBufferSizeY() const {
    s32 scale;
    if (mIsIgnoreQualityControl) {
        scale = getCurrentParam().getScreenHeightScale();
    } else {
        f32 rates[5] = {mQualityController->getRate(1), mQualityController->getRate(2),
                        mQualityController->getRate(3), mQualityController->getRate(4),
                        mQualityController->getRate(5)};

        s32 limit = 90;
        for (s32 i = mStressInfo->levelNum - 1; i >= 0; i--) {
            if (rates[i] == 0.0f) {
                limit = mStressInfo->heightScales[i];
                break;
            }
        }

        scale = sead::Mathi::min(getCurrentParam().getScreenHeightScale(), limit);
    }

    if (mIsFullResolution)
        return scale * 135 / 90 * 8 * mStressInfo->baseHeight / 1080;
    return scale * 8 * mStressInfo->renderHeight / 720;
}

/**
 * Gets the FXAA alpha output rate of the lowest quality level.
 * @return FXAA alpha output rate.
 */
f32 GraphicsStressDirector::getFXAAAlphaOut() const {
    return mQualityController->getRate(0);
}

/**
 * Enables or disables automatic quality control.
 * @param isEnable Whether quality control is enabled.
 */
void GraphicsStressDirector::setQualityControlEnable(bool isEnable) {
    mQualityController->setEnable(isEnable);
}

/**
 * Forces quality control off, restoring the previous state when released.
 * @param isForceDisable Whether quality control is forced off.
 */
void GraphicsStressDirector::setForceDisable(bool isForceDisable) {
    if (isForceDisable == mIsForceDisable)
        return;

    mIsForceDisable = isForceDisable;
    if (isForceDisable) {
        mIsQualityControlEnableBeforeDisable = mQualityController->isEnable();
        setQualityControlEnable(false);
    } else {
        setQualityControlEnable(mIsQualityControlEnableBeforeDisable);
    }
}

/**
 * Selects the stress info table used for resolution scaling.
 * @param isSingleMode Whether the game runs in single mode.
 * @param mode Stress info index.
 */
void GraphicsStressDirector::setStressInfoMode(bool isSingleMode, s32 mode) {
    mIsSingleMode = isSingleMode;
    s32 index = (u32)mode > 5 ? 0 : mode;
    if (mode < 0)
        index = 5;
    mStressInfo = &sStressInfos[index];
    mQualityController->setRecoverPercents(mStressInfo->recoverPercents);
}

/**
 * Sets the percentages the GPU load must drop by before quality is restored.
 * @param pRecoverPercents Percentage per quality level.
 */
void GraphicsQualityController::setRecoverPercents(f32* pRecoverPercents) {
    mRecoverPercents = pRecoverPercents;
}

/**
 * Gets the index of the current stress info table.
 * @return Stress info index.
 */
s32 GraphicsStressDirector::getStressInfoMode() const {
    return mStressInfo - &sStressInfos[0];
}

/**
 * Selects the stress info table for single or multi mode.
 * @param isSingleMode Whether the game runs in single mode.
 */
void GraphicsStressDirector::setSingleMode(bool isSingleMode) {
    mStressInfo = &sStressInfos[isSingleMode ? 3 : 0];
    mIsSingleMode = isSingleMode;
    mQualityController->setRecoverPercents(mStressInfo->recoverPercents);
}

/**
 * Forces the stress-free stress info table, restoring the previous one when released.
 * @param isForceStressOff Whether stress scaling is forced off.
 */
void GraphicsStressDirector::setForceStressOff(bool isForceStressOff) {
    mIsForceStressOff = isForceStressOff;
    if (isForceStressOff) {
        mStressInfoModeBeforeForceOff = getStressInfoMode();
        mStressInfo = &sStressInfos[5];
        mQualityController->setRecoverPercents(mStressInfo->recoverPercents);
        return;
    }

    s32 mode = mStressInfoModeBeforeForceOff;
    if (mode >= 0)
        mStressInfo = &sStressInfos[(u32)mode > 5 ? 0 : mode];
    else
        mStressInfo = &sStressInfos[mIsSingleMode ? 3 : 0];
    mQualityController->setRecoverPercents(mStressInfo->recoverPercents);
}

/**
 * Updates the pseudo anti-aliasing jitter, the requested parameters and the quality controller.
 */
void GraphicsStressDirector::movement() {
    if (mIsForceDisable)
        mQualityController->setEnable(false);

    mPseudoAAFrame = modi(mPseudoAAFrame + 3, 2);
    if (mPseudoAAFrame == 0)
        mPseudoAAOffset.set(0.0f, 0.0f);
    else if (mPseudoAAFrame == 1)
        mPseudoAAOffset.set(0.25f, 0.25f);

    updateRequest();
    mQualityController->setReduceQualityPercentage(getCurrentParam().getReduceQualityPercentage());
    mQualityController->updateNerve();
}

/**
 * Calculates the projection offset for pseudo anti-aliasing.
 * @param pOffset Output offset.
 * @param width Render width.
 * @param height Render height.
 */
void GraphicsStressDirector::calcPseudoAAProjOffset(sead::Vector2f* pOffset, s32 width,
                                                    s32 height) const {
    pOffset->set(mPseudoAAOffset.x / width, mPseudoAAOffset.y / height);
}

/**
 * Creates the quality controller with all quality levels at full rate.
 * @param pRecoverPercents Percentage per quality level.
 */
GraphicsQualityController::GraphicsQualityController(f32* pRecoverPercents)
    : NerveExecutor("グラフィックス処理負荷アナライザ"), mRecoverPercents(pRecoverPercents) {
    for (s32 i = 0; i < 6; i++) {
        mQualityLevels[i].level = i;
        mQualityLevels[i].rate = 1.0f;
    }

    initNerve(&NrvGraphicsQualityControllerWait, 0);
}

/**
 * Waits while watching the GPU load and reduces or recovers the quality level.
 */
void GraphicsQualityController::exeWait() {
    const agl::fctr::GPUStressChecker::Info& info =
        agl::fctr::GPUStressChecker::instance()->getInfo(0);
    f32 load = info.mLoad;
    f32 predict = info.mHistory.empty() ? load : info.mHistory(0).mPredict;
    f32 predictLoad = load + __builtin_fminf(predict - load, 15.0f);
    f32 scale = GameFrameworkNx::sInstance->_27b ? 0.5f : 1.0f;
    predictLoad *= scale;

    if (mReduceQualityPercentage < predictLoad) {
        if (!mIsEnable)
            return;
        if (mLevel < 5) {
            al::setNerve(this, &NrvGraphicsQualityControllerReduce);
            return;
        }
    } else if (!mIsEnable) {
        return;
    }

    if (mLevel < 0)
        return;

    f32 average = info.mHistory.empty() ? load : info.mHistory(0).mAverage5;
    if (scale * average < mReduceQualityPercentage - mRecoverPercents[mLevel])
        al::setNerve(this, &NrvGraphicsQualityControllerRecover);
}

/**
 * Lowers the quality by one level.
 */
void GraphicsQualityController::exeReduce() {
    if (al::isFirstStep(this))
        mLevel++;

    QualityLevel& level = mQualityLevels[mLevel];
    level.rate = 0.0f;
    if (al::isGreaterStep(this, 10)) {
        level.rate = 0.0f;
        al::setNerve(this, &NrvGraphicsQualityControllerWait);
    }
}

/**
 * Raises the quality by one level.
 */
void GraphicsQualityController::exeRecover() {
    al::isFirstStep(this);

    QualityLevel& level = mQualityLevels[mLevel];
    level.rate = 1.0f;
    if (al::isGreaterStep(this, 10)) {
        level.rate = 1.0f;
        mLevel--;
        al::setNerve(this, &NrvGraphicsQualityControllerWait);
    }
}

}  // namespace al
