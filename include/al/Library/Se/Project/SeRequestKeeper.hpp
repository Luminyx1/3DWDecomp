#pragma once

#include <container/seadOffsetList.h>
#include <container/seadPtrArray.h>

namespace alSeFunction {
enum DemoType : s32;
}

namespace al {
class SeRequest;
class SeSource;
class SePlayParamList;
class SeResourceSpecificInfo;
class AudioMixVolume;
class SeadAudio3DMgr;
class SeadAudioPlayer;
class InactiveSeListHolder;
class SeWaitingListKeeper;
class SeVolumeCtrl;
class SeMaterialInfoKeeper;

class SeRequestKeeper {
  public:
    using RequestList = sead::OffsetList<SeRequest>;
    SeRequestKeeper(SeadAudio3DMgr* pMgr, SeadAudioPlayer* pPlayer, const char* pName, s32 requestNum,
                    f32 volume);
    SePlayParamList* addRequest(u32 soundId, SeSource* pSource, bool isLoop,
                                const SeResourceSpecificInfo* pSpecificInfo,
                                const AudioMixVolume* pMixVolume);
    SePlayParamList* addHoldRequest(u32 soundId, SeSource* pSource,
                                    const SeResourceSpecificInfo* pSpecificInfo,
                                    const AudioMixVolume* pMixVolume);
    void addRequestDirect(SeRequest* pRequest);
    void stop(u32 soundId, SeSource* pSource, u32 fadeFrames);
    void deactivateSeFromSource(SeSource* pSource, u32 fadeFrames, bool isClipped);
    void reactivateSeFromSource(SeSource* pSource);
    void stopAllFromSource(SeSource* pSource, u32 fadeFrames);
    void stopAll(u32 fadeFrames, const char* pExceptName);
    void notifiedUpdateMaterial(SeSource* pSource, const char* pMaterialName, s32 waterState,
                                SeMaterialInfoKeeper* pMaterialKeeper);
    void stopAllTrigSe(u32 fadeFrames);
    void stopAllExceptList(u32 fadeFrames, const char** pExceptList, u32 exceptNum);
    void stopSeForCameraDemo(alSeFunction::DemoType type);
    bool isStopCategoryForDemo(RequestList::robustIterator it, alSeFunction::DemoType type);
    bool isCategoryPlayer(RequestList::robustIterator it);
    void startPausedSeFromCameraDemo(alSeFunction::DemoType type);
    void pauseSystem(bool isPause, const char* pName, u32 fadeFrames);
    void updatePauseFlag(bool isPause, const char* pName);
    void update(f32 distanceLimit);
    void findIdAndSetIsPlayNext(RequestList::iterator it);
    void activateSystem();
    void deactivateSystem();
    void setVolumeSetting(const char* pName, s32 fadeFrames);

    /** @brief Gets the routing name of this keeper. @return Name supplied at construction. */
    const char* getName() const { return mName; }
    /** @brief Enables post-goal volume overrides. @param isAfterGoal Whether the stage goal has been reached.
     */
    void setIsStateAfterGoal(bool isAfterGoal) { mIsAfterGoal = isAfterGoal; }
    /** @brief Enables filtering for sounds marked unsuitable for promotional playback. */
    void setIsExcludeCmNgSe() { mIsExcludeCmNgSe = true; }

  private:
    void stopAndRemove(SeRequest* pRequest, u32 fadeFrames);
    /** @brief Tests whether any system pause reason is active. @return True while a pause remains set. */
    bool isSystemPaused() const { return mIsSystemPause || mIsDemoPause || mIsErrorPause; }
    s32 mRequestNum;
    sead::PtrArray<SeRequest> mRequests;
    RequestList mActiveRequests;
    SeadAudio3DMgr* mAudio3DMgr = nullptr;
    SeadAudioPlayer* mPlayer = nullptr;
    InactiveSeListHolder* mInactiveRequests = nullptr;
    SeWaitingListKeeper* mWaitingRequests = nullptr;
    SeVolumeCtrl* mVolumeCtrl = nullptr;
    bool mIsAfterGoal = false;
    bool mIsSystemPause = false;
    bool mIsDemoPause = false;
    bool mIsErrorPause = false;
    bool mIsActive = true;
    const char* mName;
    bool mIsExcludeCmNgSe = false;
    f32 mBaseVolume;
};
static_assert(sizeof(SeRequestKeeper) == 0x70);
} // namespace al
