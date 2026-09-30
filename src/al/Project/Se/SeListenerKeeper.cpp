#include "Project/Se/SeListenerKeeper.hpp"

#include <audio/seadAudio3DListenerNin.h>
#include <gfx/seadProjection.h>

#include "Project/Audio/System/SeadAudio3DMgr.hpp"
#include "Project/Se/SeListener.hpp"
#include "Project/Se/SeListenerParamTargetViewPos.hpp"
#include "Project/Se/SeListenerPoserAdjustMiddlePos.hpp"
#include "Project/Se/SeListenerPoserMiddlePos.hpp"
#include "Project/Se/SeListenerPoserViewPos.hpp"
#include "Project/Se/SeListenerPoserViewPosOffset.hpp"
#include "Project/Se/SeListenerPoserViewPosOffsetFovy.hpp"

namespace al {
/**
 * Creates the listener and its posers.
 * @param isUnused Unused.
 * @param listenerNum Number of listeners.
 */
SeListenerKeeper::SeListenerKeeper(bool isUnused, s32 listenerNum) {
    mListeners.allocBuffer(listenerNum, nullptr);
    SeListener* listener = new SeListener(10);
    mListeners.pushBack(listener);
    listener->addPoser(new SeListenerPoserViewPos("カメラ位置", "CameraPosition"));
    listener->addPoser(new SeListenerPoserMiddlePos("ターゲット寄り中間", "MiddlePositionCloseToTarget", 0.8f));
    listener->addPoser(new SeListenerPoserMiddlePos("コースセレクト用", "CourseSelect", 0.94f));
    listener->addPoser(new SeListenerPoserMiddlePos("キノピオ探検隊用", "ForKinopioBrigadeMembers", 0.5f));
    listener->addPoser(new SeListenerPoserMiddlePos("固定カメラ小部屋用", "FixedCameraForSmallRooms", 0.5f));
    listener->addPoser(new SeListenerPoserMiddlePos("回転部屋用", "ForRotatingRooms", 0.3f));
    listener->addPoser(new SeListenerPoserMiddlePos("カメラ寄り中間", "MiddlePositionCloseToCamera", 0.2f));
    listener->addPoser(new SeListenerPoserAdjustMiddlePos("可変中間位置", "AdjustableMiddlePosition"));
    listener->addPoser(
        new SeListenerPoserViewPosOffset("カメラ位置オフセット", "CameraPositionOffset", sead::Vector3f(0.0f, 0.0f, 0.0f)));
    listener->addPoser(new SeListenerPoserViewPosOffsetFovy("カメラ位置オフセットFovy", "CameraPositionOffsetFovy"));
}

/**
 * Sets the camera the listener follows and the default listener parameters.
 * @param pMgr 3D audio manager.
 * @param pCameraPos Camera position.
 * @param pCameraMtx Camera view matrix.
 * @param pProjection Camera projection.
 * @param pCameraAt Camera target position.
 * @param pPoserName Default poser name, or nullptr.
 */
void SeListenerKeeper::init(SeadAudio3DMgr* pMgr, const sead::Vector3f* pCameraPos,
                            const sead::Matrix34f* pCameraMtx, sead::PerspectiveProjection* pProjection,
                            const sead::Vector3f* pCameraAt, const char* pPoserName) {
    mListenerParam = new SeListenerParamTargetViewPos(pCameraPos, pCameraMtx, pProjection, pCameraAt);
    mAudio3DMgr = pMgr;
    mDefaultPoserName = pPoserName != nullptr ? pPoserName : "ターゲット寄り中間";
    mListeners.unsafeAt(0)->setCurrentPoser(mDefaultPoserName);
    sead::Audio3DListenerParameterNin param;
    param.mOutputTypeFlag = 1;
    param.mInteriorSize = 1500.0f;
    param.mMaxVolumeDistance = 450.0f;
    param.mUnitDistance = 1000.0f;
    param.mUserParam = 0;
    param.mUnitBiquadFilterValue = 0.5f;
    param.mMaxBiquadFilterValue = 1.0f;
    mAudio3DMgr->setDefaultListenerParameter(param);
}

/**
 * Updates the listener matrix.
 */
void SeListenerKeeper::update() {
    SeListener* listener = mListeners.unsafeAt(0);
    listener->calcListenerMatrix(*mListenerParam);
    mAudio3DMgr->setDefaultListenerMatrix(listener->getListenerMatrix());
}

/**
 * Changes the listener parameters, replacing negative values by the defaults.
 * @param rParam Listener parameters.
 */
void SeListenerKeeper::changeListenerParam(sead::Audio3DListenerParameterNin& rParam) {
    if (rParam.mInteriorSize < 0.0f) {
        rParam.mInteriorSize = 1500.0f;
    }
    if (rParam.mMaxVolumeDistance < 0.0f) {
        rParam.mMaxVolumeDistance = 450.0f;
    }
    if (rParam.mUnitDistance < 0.0f) {
        rParam.mUnitDistance = 1000.0f;
    }
    if (rParam.mUnitBiquadFilterValue < 0.0f) {
        rParam.mUnitBiquadFilterValue = 0.5f;
    }
    if (rParam.mMaxBiquadFilterValue < 0.0f) {
        rParam.mMaxBiquadFilterValue = 1.0f;
    }
    mAudio3DMgr->setDefaultListenerParameter(rParam);
}

/**
 * Restores the default listener parameters.
 */
void SeListenerKeeper::resetListenerParam() {
    sead::Audio3DListenerParameterNin param;
    param.mOutputTypeFlag = 1;
    param.mInteriorSize = 1500.0f;
    param.mMaxVolumeDistance = 450.0f;
    param.mUnitDistance = 1000.0f;
    param.mUserParam = 0;
    param.mUnitBiquadFilterValue = 0.5f;
    param.mMaxBiquadFilterValue = 1.0f;
    mAudio3DMgr->setDefaultListenerParameter(param);
}

/**
 * Changes the listener poser and remembers the current one.
 * @param pName Poser name, or nullptr for the default poser.
 */
void SeListenerKeeper::changeListenerPoser(const char* pName) {
    mLastPoserName = mListeners.unsafeAt(0)->getCurrentPoser()->getName().cstr();
    if (pName != nullptr) {
        mListeners.unsafeAt(0)->setCurrentPoser(pName);
    } else {
        mListeners.unsafeAt(0)->setCurrentPoser(mDefaultPoserName);
    }
}

/**
 * Changes the listener poser back to the remembered one.
 */
void SeListenerKeeper::changeListenerPoserToLast() {
    const char* lastName = mLastPoserName;
    mLastPoserName = mListeners.unsafeAt(0)->getCurrentPoser()->getName().cstr();
    if (lastName != nullptr) {
        mListeners.unsafeAt(0)->setCurrentPoser(lastName);
    } else {
        mListeners.unsafeAt(0)->setCurrentPoser(mDefaultPoserName);
    }
}
}  // namespace al
