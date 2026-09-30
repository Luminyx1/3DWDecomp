#include "Library/PostProcessing/LightStreakParam.hpp"

namespace al {

/**
 * Initializes all light streak parameters with their default values.
 */
void LightStreakParam::init() {
    mIntensity.init(0.0f, "Intensity", "強さ", "Min=0.0f, Max=0.1f", &mParamObj);
    mStreakScale.init(1.0f, "StreakScale", "スケール", "Min=0.0f, Max=1.0f", &mParamObj);
    mAttn.init(0.9f, "Attn", "減衰", "Min=0.5, Max=0.95", &mParamObj);
    mThreshold.init(1.0f, "Threshold", "閾値", "Min=0.0f, Max=100.0f", &mParamObj);
    mRotateDegree.init(12.0f, "RotateDegree", "回転角度", "Min=-180.0f, Max=180.0f", &mParamObj);
    mStreakType.init(0, "StreakType", "ストリークタイプ", &mParamObj);
    mPassNum.init(2, "PassNum", "描画パス数", "Min=1, Max=4", &mParamObj);
    mStreakColor1.init(sead::Color4f::cWhite, "StreakColor1", "カラー１", &mParamObj);
    mStreakColor2.init(sead::Color4f::cWhite, "StreakColor2", "カラー２", &mParamObj);
    mStreakColor3.init(sead::Color4f::cWhite, "StreakColor3", "カラー３", &mParamObj);
}

/**
 * Initializes the parameters for the system defaults.
 */
void LightStreakParam::initSystem() {
    init();
}

}  // namespace al
