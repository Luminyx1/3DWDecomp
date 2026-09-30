#include "audio/seadAudioMgr.h"

#include "audio/seadAudioPlayerNin.h"
#include "audio/seadAudioResetter.h"
#include "audio/seadAudioResourceLoader.h"
#include "audio/seadAudioSettingParameter.h"
#include "audio/seadAudioSystemNin.h"
#include "heap/seadHeapMgr.h"

namespace sead {
SEAD_SINGLETON_DISPOSER_IMPL(AudioMgr)

/**
 * Constructs an audio manager with no components.
 */
AudioMgr::AudioMgr() {
    mSubsetList.initOffset(AudioSubsetBase::getListNodeOffset());
}

/**
 * Finalizes the audio manager and destroys the components it created itself.
 */
AudioMgr::~AudioMgr() {
    exit();

    if (mIsAudioSystemOwned && mAudioSystem) {
        delete mAudioSystem;
        mAudioSystem = nullptr;
    }

    if (mIsPlayerOwned && mPlayer) {
        delete mPlayer;
        mPlayer = nullptr;
    }

    if (mIsResetterOwned && mResetter) {
        delete mResetter;
        mResetter = nullptr;
    }
}

/**
 * Finalizes the player, the resource loader, every subset and the audio system.
 */
void AudioMgr::exit() {
    if (!mIsPrepared) {
        return;
    }

    if (mPlayer) {
        mPlayer->finalize();
    }

    if (mResourceLoader) {
        mResourceLoader->finalize();
    }

    if (!mSubsetList.isEmpty()) {
        for (auto it = mSubsetList.begin(); it != mSubsetList.end(); ++it) {
            it->finalize();
        }
    }

    if (mAudioSystem) {
        mAudioSystem->finalize();
    }

    mIsPrepared = false;
}

/**
 * Creates the missing components and initializes the whole audio system.
 * @param pParam Components and subsets to use, or nullptr for the defaults.
 * @param pHeap Heap for the components, or nullptr for the current heap.
 * @param addonArchiveCount Number of add-on sound archives the player is set up for.
 */
void AudioMgr::prepare(AudioSettingParameter* pParam, Heap* pHeap, s32 addonArchiveCount) {
    mHeap = pHeap ? pHeap : HeapMgr::instance()->getCurrentHeap();

    if (pParam) {
        mAudioSystem = pParam->mAudioSystem;
        mResetter = pParam->mResetter;
        mPlayer = pParam->mPlayer;
        mResourceLoader = pParam->mResourceLoader;

        while (!pParam->mSubsetList.isEmpty()) {
            mSubsetList.pushBack(pParam->mSubsetList.popFront());
        }
    }

    if (!mAudioSystem) {
        mAudioSystem = new (pHeap) AudioSystemNin();
        mIsAudioSystemOwned = true;
    }

    if (!mPlayer) {
        mPlayer = new (pHeap) AudioPlayerNin();
        mIsPlayerOwned = true;
    }

    if (!mResetter) {
        mResetter = new (pHeap) AudioResetterNin();
        mIsResetterOwned = true;
    }

    if (mAudioSystem) {
        static_cast<AudioSystemNin*>(mAudioSystem)->setVoiceCountMax(75);
        mAudioSystem->initialize();
    }

    if (mPlayer) {
        mPlayer->initialize();
    }

    if (mResetter) {
        mResetter->initialize(*this);
    }

    if (mResourceLoader) {
        mResourceLoader->initialize(*this);
    }

    if (!mSubsetList.isEmpty()) {
        for (auto it = mSubsetList.begin(); it != mSubsetList.end(); ++it) {
            it->initialize(*this, pHeap);
        }
    }

    DynamicCast<AudioSystemNin>(mAudioSystem)->mAddonArchiveCount = addonArchiveCount;

    if (mResourceLoader) {
        mResourceLoader->load();
    }

    mIsPrepared = true;
}

/**
 * Adds an audio subset.
 * @param pSubset Audio subset.
 */
void AudioMgr::appendAudioSubset(AudioSubsetBase* pSubset) {
    pSubset->executeOnAppend(*this);
    mSubsetList.pushBack(pSubset);
}

/**
 * Removes an audio subset.
 * @param pSubset Audio subset.
 * @return True if the subset was found and removed.
 */
bool AudioMgr::removeAudioSubset(AudioSubsetBase* pSubset) {
    auto it = mSubsetList.begin();

    for (; it != mSubsetList.end(); ++it) {
        if (&*it == pSubset) {
            break;
        }
    }

    if (it == mSubsetList.end()) {
        return false;
    }

    pSubset->executeOnRemove();
    mSubsetList.erase(&*it);
    return true;
}

/**
 * Updates the player, the resetter and every subset.
 */
void AudioMgr::calc() {
    if (!mIsPrepared) {
        return;
    }

    if (mPlayer) {
        mPlayer->calc();
    }

    if (mResetter) {
        mResetter->calc();
    }

    if (!mSubsetList.isEmpty()) {
        for (auto it = mSubsetList.begin(); it != mSubsetList.end(); ++it) {
            it->calc();
        }
    }
}

/**
 * Registers the audio manager to the host IO tree (no-op in release builds).
 * @param pNode Parent node.
 */
void AudioMgr::initHostIO(hostio::Node* pNode) {}

/**
 * Generates the host IO message (stripped in release builds).
 * @param pContext Host IO context.
 */
void AudioMgr::genMessage(hostio::Context* pContext) {
    if (mAudioSystem) {
        DynamicCast<AudioSystemNin>(mAudioSystem);
    }

    if (mPlayer) {
        DynamicCast<AudioPlayerNin>(mPlayer);
    }

    if (!mSubsetList.isEmpty()) {
        for (auto it = mSubsetList.begin(); it != mSubsetList.end(); ++it) {
        }
    }
}

/**
 * Handles a host IO property event (no-op in release builds).
 * @param pEvent Property event.
 */
void AudioMgr::listenPropertyEvent(const hostio::PropertyEvent* pEvent) {}
}  // namespace sead
