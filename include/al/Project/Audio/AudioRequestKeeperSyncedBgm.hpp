#pragma once

#include <container/seadPtrArray.h>

#include "Project/Audio/IUseAudioKeeper.hpp"
#include "Project/Bgm/BgmPlayingRequest.hpp"

namespace al {
class AudioDirector;

/// Holds BGM requests that are only carried out on a beat of the currently playing BGM.
class AudioRequestKeeperSyncedBgm : public IUseAudioKeeper {
public:
    struct SyncedRequest {
        s32 type = -1;              // _0
        BgmPlayingRequest request;  // _8
        s32 beat = 1;               // _28
        bool isDone = true;         // _2C
    };

    AudioRequestKeeperSyncedBgm();

    void init(const AudioDirector* pDirector);
    void update();
    void requestBgm(BgmPlayingType type, const BgmPlayingRequest& rRequest, s32 beat);
    AudioKeeper* getAudioKeeper() const override;

    sead::PtrArray<SyncedRequest>* mRequests;  // _8
    AudioKeeper* mAudioKeeper = nullptr;       // _10
};

AudioKeeper* createAudioKeeper(const char* pName, const AudioDirector* pDirector);
}  // namespace al
