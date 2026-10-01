#include "audio/seadAudio3DListenerGroupNin.h"

#include <nn/util/util_MatrixApi.h>
#include <nn/util/util_VectorApi.h>

#include "audio/seadAudio3DMgrNin.h"
#include "hostio/seadHostIOPropertyEvent.h"
#include "prim/seadSafeString.h"

namespace sead {
/**
 * Constructs an empty listener group with the default parameters.
 */
Audio3DListenerGroupNin::Audio3DListenerGroupNin()
    : mParam{1.0f, 1.0f, 1.0f, 0, 0.5f, 1.0f, nn::atk::Sound3DListener::ListenerOutputType_Tv} {
    mListeners.initOffset(offsetof(Audio3DListenerNin, mListNode));
}

/**
 * Destroys the listener group.
 */
Audio3DListenerGroupNin::~Audio3DListenerGroupNin() = default;

/**
 * Adds a listener to the group and applies the group parameters to it.
 * @param rListener Listener.
 */
void Audio3DListenerGroupNin::append(Audio3DListenerNin& rListener) {
    reflectGroupParamToListener_(rListener);

    if (mMgr != nullptr && !mMgr->isListenerAddedToNw(rListener)) {
        mMgr->getSound3DManager()->AddListener(&rListener);
    }

    mListeners.pushBack(&rListener);
}

/**
 * Applies the group parameters that the listener follows.
 * @param rListener Listener.
 */
void Audio3DListenerGroupNin::reflectGroupParamToListener_(Audio3DListenerNin& rListener) {
    if (rListener.isFollowingGroup(Audio3DListenerNin::cGroupParam_InteriorSize)) {
        rListener.SetInteriorSize(mParam.mInteriorSize);
    }

    if (rListener.isFollowingGroup(Audio3DListenerNin::cGroupParam_MaxVolumeDistance)) {
        rListener.SetMaxVolumeDistance(mParam.mMaxVolumeDistance);
    }

    if (rListener.isFollowingGroup(Audio3DListenerNin::cGroupParam_UnitDistance)) {
        rListener.SetUnitDistance(mParam.mUnitDistance);
    }

    if (rListener.isFollowingGroup(Audio3DListenerNin::cGroupParam_UnitBiquadFilterValue)) {
        rListener.SetUnitBiquadFilterValue(mParam.mUnitBiquadFilterValue);
    }

    if (rListener.isFollowingGroup(Audio3DListenerNin::cGroupParam_MaxBiquadFilterValue)) {
        rListener.SetMaxBiquadFilterValue(mParam.mMaxBiquadFilterValue);
    }

    if (rListener.isFollowingGroup(Audio3DListenerNin::cGroupParam_UserParam)) {
        rListener.SetUserParam(mParam.mUserParam);
    }

    if (rListener.isFollowingGroup(Audio3DListenerNin::cGroupParam_OutputTypeFlag)) {
        rListener.SetOutputTypeFlag(mParam.mOutputTypeFlag);
    }
}

/**
 * Removes a listener from the group.
 * @param rListener Listener.
 */
void Audio3DListenerGroupNin::remove(Audio3DListenerNin& rListener) {
    if (mMgr != nullptr && mMgr->isListenerAddedToNw(rListener)) {
        mMgr->getSound3DManager()->RemoveListener(&rListener);
    }

    mListeners.erase(&rListener);
}

/**
 * Removes every listener from the group.
 */
void Audio3DListenerGroupNin::removeAll() {
    if (mMgr != nullptr && !mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            if (mMgr->isListenerAddedToNw(*it)) {
                mMgr->getSound3DManager()->RemoveListener(&*it);
            }
        }
    }

    mListeners.clear();
}

/**
 * Sets the matrix of every listener that follows the group matrix.
 * @param rMtx Listener matrix.
 */
void Audio3DListenerGroupNin::setMatrix(const Matrix34f& rMtx) {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            if (it->isFollowingGroup(Audio3DListenerNin::cGroupParam_Matrix)) {
                nn::util::Matrix4x3fType mtx;
                nn::util::MatrixLoad(&mtx, reinterpret_cast<const nn::util::FloatColumnMajor4x3&>(rMtx));
                it->SetMatrix(mtx);
            }
        }
    }
}

/**
 * Resets the matrix of every listener that follows the group matrix.
 */
void Audio3DListenerGroupNin::resetMatrix() {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            if (it->isFollowingGroup(Audio3DListenerNin::cGroupParam_Matrix)) {
                it->ResetMatrix();
            }
        }
    }
}

/**
 * Sets the velocity of every listener that follows the group velocity.
 * @param rVelocity Velocity.
 */
void Audio3DListenerGroupNin::setVelocity(const Vector3f& rVelocity) {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            if (it->isFollowingGroup(Audio3DListenerNin::cGroupParam_Velocity)) {
                nn::util::Vector3fType velocity;
                velocity._v[0] = rVelocity.x;
                velocity._v[1] = rVelocity.y;
                velocity._v[2] = rVelocity.z;
                it->SetVelocity(velocity);
            }
        }
    }
}

/**
 * Sets the group interior size.
 * @param size Interior size.
 */
void Audio3DListenerGroupNin::setInteriorSize(f32 size) {
    mParam.mInteriorSize = size;
    updateInteriorSizeAll_();
}

/**
 * Applies the group interior size to the listeners that follow it.
 */
void Audio3DListenerGroupNin::updateInteriorSizeAll_() {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            if (it->isFollowingGroup(Audio3DListenerNin::cGroupParam_InteriorSize)) {
                it->SetInteriorSize(mParam.mInteriorSize);
            }
        }
    }
}

/**
 * Sets the group maximum volume distance.
 * @param distance Maximum volume distance.
 */
void Audio3DListenerGroupNin::setMaxVolumeDistance(f32 distance) {
    mParam.mMaxVolumeDistance = distance;
    updateMaxVolumeDistanceAll_();
}

/**
 * Applies the group maximum volume distance to the listeners that follow it.
 */
void Audio3DListenerGroupNin::updateMaxVolumeDistanceAll_() {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            if (it->isFollowingGroup(Audio3DListenerNin::cGroupParam_MaxVolumeDistance)) {
                it->SetMaxVolumeDistance(mParam.mMaxVolumeDistance);
            }
        }
    }
}

/**
 * Sets the group unit distance.
 * @param distance Unit distance.
 */
void Audio3DListenerGroupNin::setUnitDistance(f32 distance) {
    mParam.mUnitDistance = distance;
    updateUnitDistanceAll_();
}

/**
 * Applies the group unit distance to the listeners that follow it.
 */
void Audio3DListenerGroupNin::updateUnitDistanceAll_() {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            if (it->isFollowingGroup(Audio3DListenerNin::cGroupParam_UnitDistance)) {
                it->SetUnitDistance(mParam.mUnitDistance);
            }
        }
    }
}

/**
 * Sets the group biquad filter value per unit distance.
 * @param value Filter value.
 */
void Audio3DListenerGroupNin::setUnitBiquadFilterValue(f32 value) {
    mParam.mUnitBiquadFilterValue = value;
    updateUnitBiquadFilterValueAll_();
}

/**
 * Applies the group biquad filter value per unit distance to the listeners that follow it.
 */
void Audio3DListenerGroupNin::updateUnitBiquadFilterValueAll_() {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            if (it->isFollowingGroup(Audio3DListenerNin::cGroupParam_UnitBiquadFilterValue)) {
                it->SetUnitBiquadFilterValue(mParam.mUnitBiquadFilterValue);
            }
        }
    }
}

/**
 * Sets the group maximum biquad filter value.
 * @param value Filter value.
 */
void Audio3DListenerGroupNin::setMaxBiquadFilterValue(f32 value) {
    mParam.mMaxBiquadFilterValue = value;
    updateMaxBiquadFilterValueAll_();
}

/**
 * Applies the group maximum biquad filter value to the listeners that follow it.
 */
void Audio3DListenerGroupNin::updateMaxBiquadFilterValueAll_() {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            if (it->isFollowingGroup(Audio3DListenerNin::cGroupParam_MaxBiquadFilterValue)) {
                it->SetMaxBiquadFilterValue(mParam.mMaxBiquadFilterValue);
            }
        }
    }
}

/**
 * Sets the group user parameter.
 * @param param User parameter.
 */
void Audio3DListenerGroupNin::setUserParam(u32 param) {
    mParam.mUserParam = param;
    updateUserParamAll_();
}

/**
 * Applies the group user parameter to the listeners that follow it.
 */
void Audio3DListenerGroupNin::updateUserParamAll_() {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            if (it->isFollowingGroup(Audio3DListenerNin::cGroupParam_UserParam)) {
                it->SetUserParam(mParam.mUserParam);
            }
        }
    }
}

/**
 * Sets the group output type flags.
 * @param flag Output type flags.
 */
void Audio3DListenerGroupNin::setOutputTypeFlag(u32 flag) {
    mParam.mOutputTypeFlag = flag;
    updateOutputTypeFlagAll_();
}

/**
 * Applies the group output type flags to the listeners that follow them.
 */
void Audio3DListenerGroupNin::updateOutputTypeFlagAll_() {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            if (it->isFollowingGroup(Audio3DListenerNin::cGroupParam_OutputTypeFlag)) {
                it->SetOutputTypeFlag(mParam.mOutputTypeFlag);
            }
        }
    }
}

/**
 * Sets the output type (unsupported, does nothing).
 * @param type Output type.
 */
void Audio3DListenerGroupNin::setOutputType(nn::atk::Sound3DListener::ListenerOutputType type) {}

/**
 * Sets every group parameter and applies them to the listeners.
 * @param rParam Listener parameters.
 */
void Audio3DListenerGroupNin::setParameterAll(const Audio3DListenerParameterNin& rParam) {
    mParam = rParam;
    reflectGroupParamToListenerAll_();
}

/**
 * Applies the group parameters to every listener.
 */
void Audio3DListenerGroupNin::reflectGroupParamToListenerAll_() {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
            reflectGroupParamToListener_(*it);
        }
    }
}

/**
 * Gets every group parameter.
 * @param pParam Receives the listener parameters.
 */
void Audio3DListenerGroupNin::getParameterAll(Audio3DListenerParameterNin* pParam) const {
    *pParam = mParam;
}

/**
 * Generates the host IO message (stripped in release builds).
 * @param pContext Host IO context.
 */
void Audio3DListenerGroupNin::genMessage(hostio::Context* pContext) {
    if (!mListeners.isEmpty()) {
        for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {

        }
    }

    FormatFixedSafeString<32> name("address : %08x", reinterpret_cast<uintptr_t>(this));
}

/**
 * Applies an edited group parameter to the listeners.
 * @param pEvent Property event.
 */
void Audio3DListenerGroupNin::listenPropertyEvent(const hostio::PropertyEvent* pEvent) {
    if (pEvent->getIdValue() == reinterpret_cast<uintptr_t>(&mParam.mInteriorSize)) {
        updateInteriorSizeAll_();
    }

    if (pEvent->getIdValue() == reinterpret_cast<uintptr_t>(&mParam.mMaxVolumeDistance)) {
        updateMaxVolumeDistanceAll_();
    }

    if (pEvent->getIdValue() == reinterpret_cast<uintptr_t>(&mParam.mUnitDistance)) {
        updateUnitDistanceAll_();
    }

    if (pEvent->getIdValue() == reinterpret_cast<uintptr_t>(&mParam.mUnitBiquadFilterValue)) {
        updateUnitBiquadFilterValueAll_();
    }

    if (pEvent->getIdValue() == reinterpret_cast<uintptr_t>(&mParam.mMaxBiquadFilterValue)) {
        updateMaxBiquadFilterValueAll_();
    }
}
}  // namespace sead
