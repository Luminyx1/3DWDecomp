#pragma once
#include <basis/seadTypes.h>
#include "Project/Audio/System/SoundHeapPtrWrapper.hpp"

namespace al {
class BgmRhythmCtrl;
class MeInfo {
  public:
    s32 _0;
    s32 mChordOffset;
    s32 mScaleOffset;
    s32 mPitchOffset;
};
class SePlayParamList;
struct MeInfoEntry {
    s32 mVariable = 0;
    s32 mChord = 0;
    s32 mScale = 0;
    s32 mPitch = 0;
};
static_assert(sizeof(MeInfoEntry) == 0x10);

class MeInfoList {
  public:
    const char* mName = nullptr;
    u32 mSoundId = AudioConst::SOUND_ID_INVALID;
    MeInfoEntry* mEntries = nullptr;
    s32 mEntryNum = 0;
};
static_assert(sizeof(MeInfoList) == 0x20);

class MeInfoKeeper {
  public:
    MeInfoKeeper();
    void init(BgmRhythmCtrl* pRhythmCtrl);
    bool isMe(s32 soundId) const;
    MeInfoList* tryFindMeInfoList(const char* pName) const;
    MeInfoList* tryFindMeInfoListById(s32 soundId) const;
    void applyMeInfoToParams(const char* pName, SePlayParamList* pParams, MeInfo* pInfo);
    void applyMeInfoToParams(s32 soundId, SePlayParamList* pParams, MeInfo* pInfo);
    void applyMeInfoToParams(MeInfoList* pList, SePlayParamList* pParams, MeInfo* pInfo, const char* pName);

  private:
    BgmRhythmCtrl* mRhythmCtrl = nullptr;
    MeInfoList** mLists = nullptr;
    s32 mListNum = 0;
};
static_assert(sizeof(MeInfoKeeper) == 0x18);
} // namespace al
