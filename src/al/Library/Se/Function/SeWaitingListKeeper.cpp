#include "Library/Se/Function/SeWaitingListKeeper.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Audio/System/AudioKeeperFunction.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Library/Se/Project/SeRequest.hpp"
#include "Library/Se/Project/SeRequestKeeper.hpp"

namespace al {
/**
 * @brief Allocates 77 empty delayed-sound entries.
 */
SeWaitingListKeeper::SeWaitingListKeeper() {
    mEntries = new Entry*[mCapacity];
    for (s32 i = 0; i < mCapacity; i++) {
        mEntries[i] = new Entry;
    }
}

/**
 * @brief Advances delays and submits requests whose countdown has expired.
 * @param pKeeper Non-null request keeper that receives expired requests.
 */
void SeWaitingListKeeper::update(SeRequestKeeper* pKeeper) {
    for (s32 i = 0; i < mCapacity; i++) {
        Entry* pEntry = mEntries[i];
        if (pEntry->mRequest == nullptr) {
            continue;
        }
        s32 nextFrames = pEntry->mFrames - 1;
        if (nextFrames >= 0) {
            pEntry->mFrames = nextFrames;
        }
        if (pEntry->mFrames == 0) {
            pKeeper->addRequestDirect(pEntry->mRequest);
            pEntry->mRequest = nullptr;
            pEntry->mFrames = -1;
        }
    }
}

/**
 * @brief Queues a request with its resource-defined delay and optional volume multiplier.
 * @param pRequest Non-null request with resource-specific settings; remains valid until submitted.
 */
void SeWaitingListKeeper::addSe(SeRequest* pRequest) {
    for (s32 i = 0; i < mCapacity; i++) {
        if (mEntries[i]->mRequest != nullptr) {
            continue;
        }
        const SeResourceSpecificInfo* pInfo = pRequest->getSpecificInfo();
        if (pInfo->mDelayVolume >= 0.0f) {
            pRequest->setMulParamVolume(pInfo->mDelayVolume);
        }
        mEntries[i]->mFrames = pInfo->mDelayFrame * pRequest->getDelayMultiplier();
        mEntries[i]->mRequest = pRequest;
        return;
    }
}

/**
 * @brief Creates an audio keeper using the actor's audio director.
 * @param pName SE and BGM user name passed to the keeper's initialization.
 * @param rInfo Actor initialization information containing the audio director.
 * @return Newly allocated audio keeper.
 */
AudioKeeper* createAudioKeeper(const char* pName, const ActorInitInfo& rInfo) {
    AudioKeeper* pKeeper = new AudioKeeper();
    pKeeper->init(rInfo.getAudioDirector(), pName, pName, nullptr, nullptr, nullptr, nullptr);
    return pKeeper;
}

/**
 * @brief Creates an audio keeper using an explicit audio director.
 * @param pName SE and BGM user name passed to the keeper's initialization.
 * @param pDirector Audio director used to initialize the keeper.
 * @return Newly allocated audio keeper.
 */
AudioKeeper* createAudioKeeper(const char* pName, const AudioDirector* pDirector) {
    AudioKeeper* pKeeper = new AudioKeeper();
    pKeeper->init(pDirector, pName, pName, nullptr, nullptr, nullptr, nullptr);
    return pKeeper;
}
} // namespace al
