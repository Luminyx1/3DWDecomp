#pragma once

#include <basis/seadTypes.h>
#include <container/seadOffsetList.h>

namespace al {
class SeRequest;
class SeadAudioPlayer;

struct SeVolumeSetting {
    const char* mName;
    f32 mVolumes[8];
};

class SeVolumeCtrl {
  public:
    using RequestList = sead::OffsetList<SeRequest>;

    SeVolumeCtrl(RequestList* pRequests, SeadAudioPlayer* pPlayer, const char* pName);
    void update(bool isAfterGoal);
    void setVolumeSetting(const char* pName, f32 frames);

  private:
    void applyPlayerVolume(SeRequest* pRequest, u32 playerId, bool isAfterGoal) const;

    RequestList* mRequests;
    f32* mVolumes;
    f32* mBaseVolumes;
    s32 mFadeFrames = 0;
    SeadAudioPlayer* mPlayer;
    const SeVolumeSetting* mSetting = nullptr;
};

static_assert(sizeof(SeVolumeCtrl) == 0x30);
} // namespace al
