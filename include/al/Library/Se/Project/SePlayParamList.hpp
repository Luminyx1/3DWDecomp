#pragma once

#include <basis/seadTypes.h>

namespace al {
class SePlayParam {
public:
    SePlayParam();

    void reset();
    void set(s32 type, f32 value, f32 unk);
    void setMul(s32 type, f32 value, f32 unk);
};

class SePlayParamList {
public:
    SePlayParamList();

    void reset();
    void setVolume(f32 volume);
    void setMulVolume(f32 volume);
    void setPitch(f32 pitch);
    void setTempo(f32 tempo);
    void setLpfFreq(f32 freq);
    void setBiquadFilter(f32 value, s32 type);
    void setLocalVariable(s32 index, s32 value);
    void setGlobalVariable(s32 index, s32 value);
    bool tryGetLocalVariable(s32& rValue, s32 index) const;
    void setSpeakerVolumeLfeCenter(f32 lfe, f32 center);
    void setSpeakerVolumeFrontLR(f32 left, f32 right);
    void setSpeakerVolumeRearLR(f32 left, f32 right);
    SePlayParam* getParam(s32 index) const;
    bool isParamEmpty() const;
};
}  // namespace al
