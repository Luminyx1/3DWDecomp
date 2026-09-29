#include "Project/Audio/AudioRequestKeeperSyncedBgm.hpp"

#include "Project/Bgm/BgmUtil.hpp"

namespace al {
namespace {
const s32 cRequestNumMax = 10;
}

/**
 * @brief Constructs the keeper with a fixed pool of empty requests.
 */
AudioRequestKeeperSyncedBgm::AudioRequestKeeperSyncedBgm() {
    mRequests = new sead::PtrArray<SyncedRequest>();
    mRequests->allocBuffer(cRequestNumMax, nullptr);
    for (s32 i = 0; i < cRequestNumMax; i++) {
        mRequests->pushBack(new SyncedRequest);
    }
}

/**
 * @brief Creates the audio keeper used to query the BGM beat and play the requests.
 * @param pDirector The audio director.
 */
void AudioRequestKeeperSyncedBgm::init(const AudioDirector* pDirector) {
    mAudioKeeper = createAudioKeeper(nullptr, pDirector);
}

/**
 * @brief Carries out every pending request whose beat is triggered this frame.
 */
void AudioRequestKeeperSyncedBgm::update() {
    if (!isEnableRhythmAnim(this, nullptr)) {
        return;
    }

    bool isTriggered[cRequestNumMax] = {};
    for (s32 i = 0; i < mRequests->size(); i++) {
        SyncedRequest* request = mRequests->unsafeAt(i);
        if (request->isDone) {
            continue;
        }
        isTriggered[i] = isTriggerBeat(this, request->beat);
    }

    for (s32 i = 0; i < mRequests->size(); i++) {
        SyncedRequest* request = mRequests->unsafeAt(i);
        if (request->isDone || !isTriggered[i]) {
            continue;
        }

        const BgmPlayingRequest& bgmRequest = request->request;
        switch (request->type) {
        case BgmPlayingType_Start:
            startBgm(this, bgmRequest);
            break;
        case BgmPlayingType_Stop:
            stopBgm(this, bgmRequest);
            break;
        case BgmPlayingType_Pause:
            pauseBgm(this, bgmRequest.name, bgmRequest._8);
            break;
        case BgmPlayingType_Resume:
            resumeBgm(this, bgmRequest.name, bgmRequest._8);
            break;
        default:
            break;
        }
        request->isDone = true;
    }
}

/**
 * @brief Queues a BGM request to be carried out on a later beat.
 * @param type What to do with the BGM.
 * @param rRequest The BGM request.
 * @param beat The beat the request waits for.
 */
void AudioRequestKeeperSyncedBgm::requestBgm(BgmPlayingType type, const BgmPlayingRequest& rRequest, s32 beat) {
    SyncedRequest* request = nullptr;
    for (s32 i = 0; i < mRequests->size(); i++) {
        if (mRequests->unsafeAt(i)->isDone) {
            request = mRequests->unsafeAt(i);
            break;
        }
    }

    if (request == nullptr) {
        return;
    }

    request->type = type;
    request->request.name = rRequest.name;
    request->request._8 = rRequest._8;
    request->request._c = rRequest._c;
    request->request._10 = rRequest._10;
    request->request._14 = rRequest._14;
    request->request._15 = rRequest._15;
    request->request._18 = rRequest._18;
    request->request._1c = rRequest._1c;
    request->beat = beat;
    request->isDone = false;
}

/**
 * @brief Gets the audio keeper the requests are played with.
 * @return The audio keeper.
 */
AudioKeeper* AudioRequestKeeperSyncedBgm::getAudioKeeper() const {
    return mAudioKeeper;
}
}  // namespace al
