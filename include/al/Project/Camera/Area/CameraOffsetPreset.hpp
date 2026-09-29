#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;

/// A named camera target offset.
struct CameraOffsetPresetData {
    const char* mName;        // _0
    sead::Vector3f mOffset;   // _8
};

/// Selects one of a list of named camera target offsets.
class CameraOffsetPreset {
public:
    CameraOffsetPreset();
    CameraOffsetPreset(const CameraOffsetPresetData* pPresetData, s32 presetCount);

    void loadParam(const ByamlIter& rIter);

    const sead::Vector3f& getOffset() const { return mPresetData[mCurrentPresetIndex].mOffset; }

    const CameraOffsetPresetData* mPresetData;  // _0
    s32 mPresetCount;                           // _8
    s32 mCurrentPresetIndex = 0;                // _C
};
}  // namespace al
