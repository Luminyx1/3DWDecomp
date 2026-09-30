#include "Project/Camera/CameraOffsetPreset.hpp"

#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
const CameraOffsetPresetData sDefaultPresets[20] = {
    {"Default", {0.0f, 180.0f, 0.0f}},
    {"Default150", {0.0f, 150.0f, 0.0f}},
    {"[Test]Y0m", {0.0f, 0.0f, 0.0f}},
    {"Y0.5m", {0.0f, 50.0f, 0.0f}},
    {"[Test]Y1m", {0.0f, 100.0f, 0.0f}},
    {"[Test]Y3m", {0.0f, 300.0f, 0.0f}},
    {"[Test]Y4m", {0.0f, 400.0f, 0.0f}},
    {"[Test]Y5m", {0.0f, 500.0f, 0.0f}},
    {"[Test]Y-1m", {0.0f, -100.0f, 0.0f}},
    {"[Test]Y-2m", {0.0f, -200.0f, 0.0f}},
    {"[Test]Y-3m", {0.0f, -300.0f, 0.0f}},
    {"[Test]Y-3m", {0.0f, -400.0f, 0.0f}},
    {"[Test]Y-5m", {0.0f, -500.0f, 0.0f}},
    {"Y7m", {0.0f, 700.0f, 0.0f}},
    {"ゴーレム用", {0.0f, 850.0f, 0.0f}},
    {"クッパ用", {0.0f, 360.0f, 0.0f}},
    {"崩落クッパ用", {0.0f, 450.0f, 0.0f}},
    {"For video", {0.0f, 210.0f, 0.0f}},
    {"森ボス用", {0.0f, 916.0f, 0.0f}},
    {"GigaOffset (Y-16m)", {0.0f, 3000.0f, 0.0f}},
};
}  // namespace

/**
 * Creates a preset selector over the built-in offset presets.
 */
CameraOffsetPreset::CameraOffsetPreset()
    : mPresetData(sDefaultPresets),
      mPresetNum(sizeof(sDefaultPresets) / sizeof(CameraOffsetPresetData)) {}

/**
 * Creates a preset selector over a custom preset list.
 * @param pPresetData Preset list.
 * @param presetNum Number of presets in the list.
 */
CameraOffsetPreset::CameraOffsetPreset(const CameraOffsetPresetData* pPresetData, s32 presetNum)
    : mPresetData(pPresetData), mPresetNum(presetNum) {}

/**
 * Selects the preset named by the "OffsetName" parameter, if present.
 * @param rIter Camera parameter iterator.
 */
void CameraOffsetPreset::loadParam(const ByamlIter& rIter) {
    const char* offsetName = tryGetByamlKeyStringOrNULL(rIter, "OffsetName");

    if (!offsetName) {
        return;
    }

    mCurrentPresetIndex = -1;

    for (s32 i = 0; i < mPresetNum; i++) {
        if (isEqualString(mPresetData[i].name, offsetName)) {
            mCurrentPresetIndex = i;
            return;
        }
    }
}

}  // namespace al
