#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

#include "Project/Audio/IUseAudioKeeper.hpp"
#include "Project/Bgm/BgmPlayingRequest.hpp"

namespace al {
class AudioDirector;

enum BgmPlayingType : s32 {
    BgmPlayingType_Start = 0,
    BgmPlayingType_Stop = 1,
    BgmPlayingType_Pause = 2,
    BgmPlayingType_Resume = 3,
};

struct SyncedBgmRequest {
    BgmPlayingType type = static_cast<BgmPlayingType>(-1);
    BgmPlayingRequest request = {nullptr};
    s32 beat = 1;
    bool isDone = true;
};
static_assert(sizeof(SyncedBgmRequest) == 0x30);

class AudioRequestKeeperSyncedBgm : public IUseAudioKeeper {
public:
    AudioRequestKeeperSyncedBgm();

    void init(const AudioDirector* pDirector);
    void update();
    void requestBgm(BgmPlayingType type, const BgmPlayingRequest& rRequest, s32 beat);

    AudioKeeper* getAudioKeeper() const override;

private:
    sead::PtrArray<SyncedBgmRequest>* mRequests;
    AudioKeeper* mAudioKeeper = nullptr;
};
static_assert(sizeof(AudioRequestKeeperSyncedBgm) == 0x18);
}  // namespace al
