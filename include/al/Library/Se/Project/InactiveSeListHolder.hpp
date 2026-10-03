#pragma once

#include <container/seadPtrArray.h>
#include "Project/Audio/System/SoundHeapPtrWrapper.hpp"

namespace al {
class AudioMixVolume;
class SeSource;
class SeResourceSpecificInfo;

struct InactiveSeParam {
    u32 mSoundId = AudioConst::SOUND_ID_INVALID;
    SeSource* mSource = nullptr;
    const SeResourceSpecificInfo* mSpecificInfo = nullptr;
    const AudioMixVolume* mMixVolume = nullptr;

    /** @brief Returns the retained request slot to its empty state. */
    void clear() {
        mSoundId = AudioConst::SOUND_ID_INVALID;
        mSource = nullptr;
        mSpecificInfo = nullptr;
        mMixVolume = nullptr;
    }
};

class InactiveSeListHolder {
  public:
    InactiveSeListHolder();
    void addSe(u32 soundId, SeSource* pSource, const SeResourceSpecificInfo* pSpecificInfo,
               const AudioMixVolume* pMixVolume);
    void getInfoIfEqualSource(InactiveSeParam* pInfo, s32 index, SeSource* pSource) const;
    void clear(s32 index);
    void clearIfEqualSource(SeSource* pSource);
    void clearAll();
    /** @brief Gets the number of allocated inactive slots. @return Slot count, including empty slots. */
    s32 getNum() const { return mParams.size(); }

  private:
    sead::PtrArray<InactiveSeParam> mParams;
};

static_assert(sizeof(InactiveSeParam) == 0x20);
static_assert(sizeof(InactiveSeListHolder) == 0x10);
} // namespace al
