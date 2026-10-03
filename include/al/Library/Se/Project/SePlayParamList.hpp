#pragma once

#include <basis/seadTypes.h>

namespace al {
class SePlayParam {
  public:
    SePlayParam();

    void reset();
    void set(s32 type, f32 value, f32 secondary);
    void setMul(s32 type, f32 value, f32 secondary);

    /** @brief Gets the record type, with zero indicating an unused record. @return Parameter type. */
    s32 getType() const { return mType; }
    /** @brief Gets the primary parameter value. @return Stored value. */
    f32 getValue() const { return mValue; }
    /** @brief Gets the secondary value or variable index. @return Stored secondary value. */
    f32 getSecondary() const { return mSecondary; }

  private:
    s32 mType;
    f32 mValue;
    f32 mSecondary;
};

class SePlayParamList {
  public:
    static constexpr s32 cRecordCount = 12;

    SePlayParamList();

    void reset();
    void setVolume(f32 volume);
    void setMulVolume(f32 volume);
    void setPitch(f32 pitch);
    void setTempo(f32 tempo);
    void setLpfFreq(f32 freq);
    void setBiquadFilter(f32 value, s32 type);
    void setLocalVariable(s32 value, s32 index);
    void setGlobalVariable(s32 value, s32 index);
    bool tryGetLocalVariable(s32& rValue, s32 index) const;
    void setSpeakerVolumeLfeCenter(f32 lfe, f32 center);
    void setSpeakerVolumeFrontLR(f32 left, f32 right);
    void setSpeakerVolumeRearLR(f32 left, f32 right);
    SePlayParam* getParam(s32 index) const;
    bool isParamEmpty() const;

    /** @brief Tests whether an output-line override is pending. @return True when an override is set. */
    bool hasOutputLine() const { return _8; }
    /** @brief Gets the requested output-line mask. @return Output-line bits for the sound. */
    s32 getOutputLine() const { return _c; }

    SePlayParam* findAvailableRecord(s32 type, s32 index) const;

  private:
    SePlayParam** mParams;
    bool _8;
    s32 _c;
};
static_assert(sizeof(SePlayParam) == 0xc);
static_assert(sizeof(SePlayParamList) == 0x10);
} // namespace al
