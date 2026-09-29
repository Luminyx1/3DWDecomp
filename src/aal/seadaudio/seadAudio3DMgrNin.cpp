#include "audio/seadAudio3DMgrNin.h"

#include "audio/seadAudioMgr.h"
#include "audio/seadAudioPlayerNin.h"
#include "audio/seadAudioSoundDataMgrNin.h"
#include "basis/seadNew.h"

namespace sead {
/**
 * Constructs a 3D manager.
 * @param createDefaultListener Whether to create a default listener.
 */
Audio3DMgrNin::Audio3DMgrNin(bool createDefaultListener) {
    mSound3DManager = new nn::atk::Sound3DManager();
    if (createDefaultListener) {
        mDefaultListener = new (static_cast<s32>(alignof(Audio3DListenerNin))) Audio3DListenerNin();
    }
    mListeners.initOffset(offsetof(Audio3DListenerNin, mListNode));
    mListenerGroups.initOffset(offsetof(Audio3DListenerGroupNin, mListNode));
}

/**
 * Finalizes the 3D manager and destroys the default listener.
 */
Audio3DMgrNin::~Audio3DMgrNin() {
    mSound3DManager->Finalize();
    if (mWorkBuffer) {
        delete[] mWorkBuffer;
        mWorkBuffer = nullptr;
    }
    if (mDefaultListener) {
        delete mDefaultListener;
        mDefaultListener = nullptr;
    }
    if (mSound3DManager) {
        delete mSound3DManager;
        mSound3DManager = nullptr;
    }
}

/**
 * Initializes the Nintendo 3D sound manager and registers the default listener.
 * @param rMgr Audio manager.
 * @param pHeap Heap for the work buffer.
 */
void Audio3DMgrNin::initialize(AudioMgr& rMgr, Heap* pHeap) {
    AudioPlayerNin* player = DynamicCast<AudioPlayerNin>(rMgr.getPlayer());
    const nn::atk::SoundArchive* archive = player->getSoundDataMgr()->getSoundArchive();
    size_t size = mSound3DManager->GetRequiredMemSize(archive);
    mWorkBuffer = new (pHeap, 0x20) u8[size];
    if (mSound3DManager->Initialize(archive, mWorkBuffer, size) && mDefaultListener) {
        appendListener(*mDefaultListener);
    }
}

/**
 * Registers a listener.
 * @param rListener Listener.
 */
void Audio3DMgrNin::appendListener(Audio3DListenerNin& rListener) {
    if (!isListenerAddedToNw(rListener)) {
        mSound3DManager->AddListener(&rListener);
    }
    mListeners.pushBack(&rListener);
}

/**
 * Finalizes the Nintendo 3D sound manager.
 */
void Audio3DMgrNin::finalize() {
    mSound3DManager->Finalize();
    if (mWorkBuffer) {
        delete[] mWorkBuffer;
        mWorkBuffer = nullptr;
    }
}

/**
 * Sets the maximum priority reduction.
 * @param reduction Maximum priority reduction.
 */
void Audio3DMgrNin::setMaxPriorityReduction(s32 reduction) {
    mSound3DManager->SetMaxPriorityReduction(reduction);
}

/**
 * Gets the maximum priority reduction.
 * @return Maximum priority reduction.
 */
s32 Audio3DMgrNin::getMaxPriorityReduction() {
    return mSound3DManager->GetMaxPriorityReduction();
}

/**
 * Sets the pan range.
 * @param range Pan range.
 */
void Audio3DMgrNin::setPanRange(f32 range) {
    mSound3DManager->SetPanRange(range);
}

/**
 * Gets the pan range.
 * @return Pan range.
 */
f32 Audio3DMgrNin::getPanRange() {
    return mSound3DManager->GetPanRange();
}

/**
 * Sets the sonic velocity used for the doppler effect.
 * @param velocity Sonic velocity.
 */
void Audio3DMgrNin::setSonicVelocity(f32 velocity) {
    mSound3DManager->SetSonicVelocity(velocity);
}

/**
 * Gets the sonic velocity used for the doppler effect.
 * @return Sonic velocity.
 */
f32 Audio3DMgrNin::getSonicVelocity() {
    return mSound3DManager->GetSonicVelocity();
}

/**
 * Sets the biquad filter type.
 * @param type Biquad filter type.
 */
void Audio3DMgrNin::setBiquadFilterType(s32 type) {
    mSound3DManager->SetBiquadFilterType(type);
}

/**
 * Gets the biquad filter type.
 * @return Biquad filter type.
 */
s32 Audio3DMgrNin::getBiquadFilterType() const {
    return mSound3DManager->GetBiquadFilterType();
}

/**
 * Sets the matrix of the default listener.
 * @param rMtx Listener matrix.
 */
void Audio3DMgrNin::setDefaultListenerMatrix(const Matrix34f& rMtx) {
    mDefaultListener->setMatrix(rMtx);
}

/**
 * Resets the matrix of the default listener.
 */
void Audio3DMgrNin::resetDefaultListenerMatrix() {
    mDefaultListener->resetMatrix();
}

/**
 * Sets the parameters of the default listener.
 * @param rParam Listener parameters.
 */
void Audio3DMgrNin::setDefaultListenerParameter(const Audio3DListenerParameterNin& rParam) {
    mDefaultListener->setInteriorSize(rParam.mInteriorSize);
    mDefaultListener->setMaxVolumeDistance(rParam.mMaxVolumeDistance);
    mDefaultListener->setUnitDistance(rParam.mUnitDistance);
    mDefaultListener->setUserParam(rParam.mUserParam);
    mDefaultListener->setUnitBiquadFilterValue(rParam.mUnitBiquadFilterValue);
    mDefaultListener->setMaxBiquadFilterValue(rParam.mMaxBiquadFilterValue);
}

/**
 * Gets the parameters of the default listener.
 * @param pParam Receives the listener parameters.
 */
void Audio3DMgrNin::getDefaultListenerParameter(Audio3DListenerParameterNin* pParam) {
    pParam->mInteriorSize = mDefaultListener->getInteriorSize();
    pParam->mMaxVolumeDistance = mDefaultListener->getMaxVolumeDistance();
    pParam->mUnitDistance = mDefaultListener->getUnitDistance();
    pParam->mUserParam = mDefaultListener->getUserParam();
    pParam->mUnitBiquadFilterValue = mDefaultListener->getUnitBiquadFilterValue();
    pParam->mMaxBiquadFilterValue = mDefaultListener->getMaxBiquadFilterValue();
}

/**
 * Checks whether a listener is registered to the Nintendo 3D sound manager.
 * @param rListener Listener.
 * @return True if the listener is registered.
 */
bool Audio3DMgrNin::isListenerAddedToNw(Audio3DListenerNin& rListener) const {
    const auto& list = mSound3DManager->GetListenerList();
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (&*it == &rListener) {
            return true;
        }
    }
    return false;
}

/**
 * Unregisters a listener.
 * @param rListener Listener.
 */
void Audio3DMgrNin::removeListener(Audio3DListenerNin& rListener) {
    mListeners.erase(&rListener);
    if (isListenerAddedToNw(rListener)) {
        mSound3DManager->RemoveListener(&rListener);
    }
}

/**
 * Registers a listener group and all of its listeners.
 * @param rGroup Listener group.
 */
void Audio3DMgrNin::appendListenerGroup(Audio3DListenerGroupNin& rGroup) {
    if (!rGroup.mListeners.isEmpty()) {
        for (auto it = rGroup.mListeners.begin(); it != rGroup.mListeners.end(); ++it) {
            if (!isListenerAddedToNw(*it)) {
                mSound3DManager->AddListener(&*it);
            }
        }
    }
    mListenerGroups.pushBack(&rGroup);
    rGroup.mMgr = this;
}

/**
 * Unregisters a listener group and all of its listeners.
 * @param rGroup Listener group.
 */
void Audio3DMgrNin::removeListenerGroup(Audio3DListenerGroupNin& rGroup) {
    if (!rGroup.mListeners.isEmpty()) {
        for (auto it = rGroup.mListeners.begin(); it != rGroup.mListeners.end(); ++it) {
            if (isListenerAddedToNw(*it)) {
                mSound3DManager->RemoveListener(&*it);
            }
        }
    }
    mListenerGroups.erase(&rGroup);
    rGroup.mMgr = nullptr;
}

/**
 * Generates the host IO message (stripped in release builds).
 * @param pContext Host IO context.
 */
void Audio3DMgrNin::genMessage(hostio::Context* pContext) {
    if (!mSound3DManager) {
        return;
    }
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
        }
    }
    if (!mListenerGroups.isEmpty()) {
        for (auto it = mListenerGroups.begin(); it != mListenerGroups.end(); ++it) {
        }
    }
}

/**
 * Handles a host IO property event (no-op in release builds).
 * @param pEvent Property event.
 */
void Audio3DMgrNin::listenPropertyEvent(const hostio::PropertyEvent* pEvent) {}
}  // namespace sead
