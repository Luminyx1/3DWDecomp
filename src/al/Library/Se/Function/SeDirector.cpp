#include "Library/Se/Function/SeDirector.hpp"
#include "Library/Se/Project/SeRequestKeeper.hpp"
#include "Project/Se/SeListenerKeeper.hpp"
#include "Project/Base/StringUtil.hpp"
#include <attributes.h>
#include "Library/Se/Function/MeInfoKeeper.hpp"
#include "Library/Se/Project/SePlayParamList.hpp"
#include "Library/Se/Project/SeCategory.hpp"
#include "Library/Se/Project/SeMaterialInfoKeeper.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Project/Audio/System/SoundHeapPtrWrapper.hpp"

namespace al {
/** @brief Stops every sound immediately before the director is finalized. */
void SeDirector::finalize() {
    forEachRequestKeeper([](SeRequestKeeper* pKeeper) { pKeeper->stopAll(0, nullptr); });
}
/**
 * @brief Finds a request keeper by its routing name.
 * @param pName Name to compare; all four keepers must exist if earlier names do not match.
 * @return Matching keeper, or nullptr when no name matches.
 */
SeRequestKeeper* SeDirector::findRequestKeeper(const char* pName) const {
    if (isEqualString(mMainKeeper->getName(), pName)) {
        return mMainKeeper;
    }
    if (isEqualString(mDemoKeeper->getName(), pName)) {
        return mDemoKeeper;
    }
    if (isEqualString(mPlayerKeeper->getName(), pName)) {
        return mPlayerKeeper;
    }
    if (isEqualString(mSubKeeper->getName(), pName)) {
        return mSubKeeper;
    }
    return nullptr;
}
/**
 * @brief Stops all requests, optionally restricted to a named keeper.
 * @param fadeFrames Fade duration in update frames; zero stops immediately.
 * @param pExceptName Optional named stop-exception policy, passed to each keeper.
 * @param pKeeperName Keeper routing name, or nullptr to stop all initialized keepers.
 */
void SeDirector::stopAll(u32 fadeFrames, const char* pExceptName, const char* pKeeperName) {
    if (pKeeperName == nullptr) {
        forEachRequestKeeper([&](SeRequestKeeper* pKeeper) { pKeeper->stopAll(fadeFrames, pExceptName); });
    } else {
        SeRequestKeeper* pKeeper = findRequestKeeper(pKeeperName);
        if (pKeeper != nullptr) {
            pKeeper->stopAll(fadeFrames, pExceptName);
        }
    }
}
/** @brief Updates request playback and the optional 3D listener controller. */
void SeDirector::update() {
    const f32 distanceLimit = mIsDistancePauseEnabled ? 7500.0f : -1.0f;
    forEachRequestKeeper([&](SeRequestKeeper* pKeeper) { pKeeper->update(distanceLimit); });
    if (mListenerKeeper != nullptr) {
        mListenerKeeper->update();
    }
}
/**
 * @brief Stops sounds other than the supplied sound names in every keeper.
 * @param fadeFrames Fade duration in update frames; zero stops immediately.
 * @param pExceptList Array of sound names to preserve, or nullptr for none.
 * @param exceptNum Number of names in pExceptList.
 */
void SeDirector::stopAllExcept(u32 fadeFrames, const char** pExceptList, u32 exceptNum) {
    forEachRequestKeeper(
        [&](SeRequestKeeper* pKeeper) { pKeeper->stopAllExceptList(fadeFrames, pExceptList, exceptNum); });
}
/** @brief Stops the main keeper's sounds. @param fadeFrames Fade duration in update frames. */
void SeDirector::stopAllMain(u32 fadeFrames) { mMainKeeper->stopAll(fadeFrames, nullptr); }
/** @brief Stops the demo keeper's sounds. @param fadeFrames Fade duration in update frames. */
void SeDirector::stopAllDemo(u32 fadeFrames) { mDemoKeeper->stopAll(fadeFrames, nullptr); }
/**
 * @brief Changes a pause reason across every keeper.
 * @param isPause True to pause, false to release this pause reason.
 * @param pName Recognized system pause reason name.
 * @param fadeFrames Fade duration for the pause transition in update frames.
 */
void SeDirector::pauseSystem(bool isPause, const char* pName, u32 fadeFrames) {
    forEachRequestKeeper([&](SeRequestKeeper* pKeeper) { pKeeper->pauseSystem(isPause, pName, fadeFrames); });
}
/**
 * @brief Changes a pause reason on the main, demo, and player keepers.
 * @param isPause True to pause, false to release this pause reason.
 * @param pName Recognized system pause reason name.
 * @param fadeFrames Fade duration for the pause transition in update frames.
 */
void SeDirector::pauseSystemExceptSub(bool isPause, const char* pName, u32 fadeFrames) {
    mMainKeeper->pauseSystem(isPause, pName, fadeFrames);
    mDemoKeeper->pauseSystem(isPause, pName, fadeFrames);
    mPlayerKeeper->pauseSystem(isPause, pName, fadeFrames);
}
/** @brief Enables a named request keeper. @param pName Routing name of the keeper to enable. */
void SeDirector::activateRequestKeeper(const char* pName) {
    SeRequestKeeper* pKeeper = findRequestKeeper(pName);
    if (pKeeper != nullptr) {
        pKeeper->activateSystem();
    }
}
/** @brief Disables a named request keeper. @param pName Routing name of the keeper to disable. */
void SeDirector::deactivateRequestKeeper(const char* pName) {
    SeRequestKeeper* pKeeper = findRequestKeeper(pName);
    if (pKeeper != nullptr) {
        pKeeper->deactivateSystem();
    }
}
/**
 * @brief Suspends playback associated with a source in every keeper.
 * @param pSource Source whose requests should be suspended; must be initialized.
 * @param fadeFrames Fade duration in update frames.
 * @param isClipped Whether suspension is caused by the source being clipped.
 */
void SeDirector::deactivateSeFromSource(SeSource* pSource, u32 fadeFrames, bool isClipped) {
    forEachRequestKeeper(
        [&](SeRequestKeeper* pKeeper) { pKeeper->deactivateSeFromSource(pSource, fadeFrames, isClipped); });
}
/** @brief Restores a source's suspended sounds. @param pSource Previously suspended source. */
void SeDirector::reactivateSeFromSource(SeSource* pSource) {
    forEachRequestKeeper([&](SeRequestKeeper* pKeeper) { pKeeper->reactivateSeFromSource(pSource); });
}
/**
 * @brief Stops all requests attached to a source.
 * @param pSource Source whose requests should stop.
 * @param fadeFrames Fade duration in update frames.
 */
void SeDirector::stopAllFromSource(SeSource* pSource, u32 fadeFrames) {
    forEachRequestKeeper([&](SeRequestKeeper* pKeeper) { pKeeper->stopAllFromSource(pSource, fadeFrames); });
}
/**
 * @brief Replaces material-dependent sounds after their source changes surfaces or water state.
 * @param pSource Source whose material changed.
 * @param pMaterialName New material name, or nullptr when no surface replacement is requested.
 * @param waterState Water-state selector passed to the material replacement table.
 */
void SeDirector::notifiedUpdateMaterial(SeSource* pSource, const char* pMaterialName, s32 waterState) {
    mMainKeeper->notifiedUpdateMaterial(pSource, pMaterialName, waterState, mMaterialInfoKeeper);
    mPlayerKeeper->notifiedUpdateMaterial(pSource, pMaterialName, waterState, mMaterialInfoKeeper);
    mDemoKeeper->notifiedUpdateMaterial(pSource, pMaterialName, waterState, mMaterialInfoKeeper);
}
/**
 * @brief Applies a volume preset to the player, demo, and optional sub keepers.
 * @param pName Volume preset name.
 * @param fadeFrames Volume transition duration in update frames.
 */
void SeDirector::setAllKeeperVolumeSettingExceptMain(const char* pName, s32 fadeFrames) {
    mPlayerKeeper->setVolumeSetting(pName, fadeFrames);
    mDemoKeeper->setVolumeSetting(pName, fadeFrames);
    if (mSubKeeper != nullptr) {
        mSubKeeper->setVolumeSetting(pName, fadeFrames);
    }
}
/**
 * @brief Applies a volume preset to every keeper.
 * @param pName Volume preset name.
 * @param fadeFrames Volume transition duration in update frames.
 */
void SeDirector::setAllKeeperVolumeSetting(const char* pName, s32 fadeFrames) {
    mMainKeeper->setVolumeSetting(pName, fadeFrames);
    setAllKeeperVolumeSettingExceptMain(pName, fadeFrames);
}
/** @brief Enters a demo sound policy. @param type Demo policy selector; stage demo is zero. */
NOINLINE void SeDirector::startDemo(alSeFunction::DemoType type) {
    switch (type) {
    case 0:
        mMainKeeper->stopAllTrigSe(0);
        mMainKeeper->pauseSystem(true, "デモ", 80);
        mPlayerKeeper->setVolumeSetting("ステージデモ", 80);
        mIsInDemo = true;
        return;
    case 1:
        mMainKeeper->setVolumeSetting("SystemDemo", 30);
        break;
    case 2:
        mMainKeeper->setVolumeSetting("AtmosphereDemo", 30);
        break;
    case 3:
        mMainKeeper->setVolumeSetting("SystemAtmosphereDemo", 30);
        break;
    case 4:
        mIsInDemo = true;
        return;
    default:
        break;
    }
    setAllKeeperVolumeSettingExceptMain("SystemVoiceDemo", 30);
    mMainKeeper->stopSeForCameraDemo(type);
    mIsInDemo = true;
}
/** @brief Leaves a demo sound policy and restores normal volumes. @param type Policy being ended; four is
 * ignored. */
NOINLINE void SeDirector::endDemo(alSeFunction::DemoType type) {
    switch (type) {
    case 4:
        return;
    case 0: {
        mMainKeeper->pauseSystem(false, "デモ", 80);
        mPlayerKeeper->setVolumeSetting("通常", 40);
        break;
    }
    default: {
        mMainKeeper->startPausedSeFromCameraDemo(type);
        setAllKeeperVolumeSetting("通常", 10);
        break;
    }
    }
    mDemoKeeper->stopAll(30, nullptr);
    mIsInDemo = false;
}
/**
 * @brief Changes between demo sound policies.
 * @param from Policy to leave.
 * @param to Policy to enter.
 */
void SeDirector::changeDemo(alSeFunction::DemoType from, alSeFunction::DemoType to) {
    endDemo(from);
    startDemo(to);
}
/** @brief Sets post-goal volume behavior in every keeper. @param isAfterGoal Whether the stage goal has been
 * reached. */
void SeDirector::setIsStateAfterGoal(bool isAfterGoal) {
    mMainKeeper->setIsStateAfterGoal(isAfterGoal);
    if (mSubKeeper != nullptr) {
        mSubKeeper->setIsStateAfterGoal(isAfterGoal);
    }
    mDemoKeeper->setIsStateAfterGoal(isAfterGoal);
    mPlayerKeeper->setIsStateAfterGoal(isAfterGoal);
}
/** @brief Enables promotional-playback sound filtering in every keeper. */
void SeDirector::setIsExcludeCmNgSe() {
    forEachRequestKeeper([](SeRequestKeeper* pKeeper) { pKeeper->setIsExcludeCmNgSe(); });
}
/**
 * @brief Changes the volume preset of a named keeper.
 * @param pKeeperName Routing name of the keeper.
 * @param pSettingName Volume preset name.
 * @param fadeFrames Transition duration in update frames.
 * @param isForce When true, the original implementation suppresses this change during a demo.
 */
void SeDirector::setVolumeSetting(const char* pKeeperName, const char* pSettingName, s32 fadeFrames,
                                  bool isForce) {
    if (isForce && mIsInDemo) {
        return;
    }
    SeRequestKeeper* pKeeper = findRequestKeeper(pKeeperName);
    if (pKeeper != nullptr) {
        pKeeper->setVolumeSetting(pSettingName, fadeFrames);
    }
}
/** @brief Replaces listener parameters. @param rParam Initialized listener parameters to copy. */
void SeDirector::changeListenerParam(sead::Audio3DListenerParameterNin& rParam) {
    mListenerKeeper->changeListenerParam(rParam);
}
/** @brief Restores default listener parameters. */
void SeDirector::resetListenerParam() { mListenerKeeper->resetListenerParam(); }
/** @brief Selects a listener-position policy. @param pName Registered poser name. */
void SeDirector::changeListenerPoser(const char* pName) { mListenerKeeper->changeListenerPoser(pName); }
/** @brief Restores the previously selected listener-position policy. */
void SeDirector::changeListenerPoserToLast() { mListenerKeeper->changeListenerPoserToLast(); }

/**
 * @brief Selects an explicit routing name or the current main/demo keeper.
 * @param pName Routing name, or nullptr to follow the current demo state.
 * @return Selected keeper, or nullptr when an explicit name is not found.
 */
inline SeRequestKeeper* SeDirector::selectRequestKeeper(const char* pName) const {
    return pName != nullptr ? findRequestKeeper(pName) : (mIsInDemo ? mDemoKeeper : mMainKeeper);
}
/**
 * @brief Applies the material-dependent low-pass filter to newly requested playback.
 * @param pParams Playback parameters, or nullptr when the request was rejected.
 * @param pSpecificInfo Non-null resource settings controlling material filtering.
 * @param waterState Water-state selector; two selects the underwater filter.
 * @param isBeyondWall Whether the source is occluded by a wall.
 */
static inline void applyMaterialLpf(SePlayParamList* pParams, const SeResourceSpecificInfo* pSpecificInfo,
                                    s32 waterState, bool isBeyondWall) {
    if (waterState != 2 && !isBeyondWall) {
        return;
    }
    if (pParams == nullptr || !pSpecificInfo->mIsValidMatCodeLpf) {
        return;
    }
    if (waterState == 2) {
        pParams->setLpfFreq(-0.39f);
    } else {
        pParams->setLpfFreq(-0.34f);
    }
}
/**
 * @brief Creates a routed sound request with material substitution and musical-effect parameters.
 * @param id Original sound identifier.
 * @param pSource Source attached to the request.
 * @param isLoop Whether the requested sound should loop.
 * @param pSpecificInfo Resource settings; nullptr rejects the request.
 * @param pMeInfo Musical-effect state passed to parameter selection when the sound is a musical effect.
 * @param pMaterialName Surface material name, or nullptr for no surface substitution.
 * @param waterState Water-state selector; nonzero enables material substitution and two enables underwater
 * filtering.
 * @param isBeyondWall Whether wall occlusion should apply material filtering.
 * @param pPlayName Keeper routing name, or nullptr for the current main/demo keeper.
 * @return Mutable playback parameters, or nullptr when the request is rejected.
 */
SePlayParamList* SeDirector::addRequest(u32 id, SeSource* pSource, bool isLoop,
                                        const SeResourceSpecificInfo* pSpecificInfo, MeInfo* pMeInfo,
                                        const char* pMaterialName, s32 waterState, bool isBeyondWall,
                                        const char* pPlayName) {
    SeRequestKeeper* pKeeper = selectRequestKeeper(pPlayName);
    if (pSpecificInfo == nullptr || pKeeper == nullptr) {
        return nullptr;
    }
    if (pMaterialName != nullptr || waterState != 0) {
        id = mMaterialInfoKeeper->findReplacedId(id, pMaterialName, waterState, pSpecificInfo);
    }
    const AudioMixVolume* pMixVolume = nullptr;
    if (mCategoryParamsController != nullptr) {
        pMixVolume = mCategoryParamsController->getMixVolume(pSpecificInfo->mPlayerId);
        if (pMixVolume == nullptr) {
            return nullptr;
        }
    }
    SePlayParamList* pParams = pKeeper->addRequest(id, pSource, isLoop, pSpecificInfo, pMixVolume);
    const bool isMe = mMeInfoKeeper->isMe(id);
    if (pParams != nullptr && isMe) {
        mMeInfoKeeper->applyMeInfoToParams(alSoundNameUtil::getSoundName(id, false), pParams, pMeInfo);
    }
    applyMaterialLpf(pParams, pSpecificInfo, waterState, isBeyondWall);
    return pParams;
}
/**
 * @brief Extends or starts a held sound request, keeping musical-effect lookup tied to its original ID.
 * @param id Original sound identifier used for musical-effect lookup.
 * @param pSource Source attached to the request.
 * @param pSpecificInfo Resource settings; nullptr rejects the request.
 * @param pMeInfo Musical-effect state passed to parameter selection when applicable.
 * @param pMaterialName Surface material name, or nullptr for no surface substitution.
 * @param waterState Water-state selector; nonzero enables substitution and two enables underwater filtering.
 * @param isBeyondWall Whether wall occlusion should apply material filtering.
 * @param pPlayName Keeper routing name, or nullptr for the current main/demo keeper.
 * @return Mutable playback parameters, or nullptr when the request is rejected.
 */
SePlayParamList* SeDirector::addHoldRequest(u32 id, SeSource* pSource,
                                            const SeResourceSpecificInfo* pSpecificInfo, MeInfo* pMeInfo,
                                            const char* pMaterialName, s32 waterState, bool isBeyondWall,
                                            const char* pPlayName) {
    SeRequestKeeper* pKeeper = selectRequestKeeper(pPlayName);
    if (pSpecificInfo == nullptr || pKeeper == nullptr) {
        return nullptr;
    }
    u32 replacedId = id;
    if (pMaterialName != nullptr || waterState != 0) {
        replacedId = mMaterialInfoKeeper->findReplacedId(id, pMaterialName, waterState, pSpecificInfo);
    }
    const AudioMixVolume* pMixVolume = nullptr;
    if (mCategoryParamsController != nullptr) {
        pMixVolume = mCategoryParamsController->getMixVolume(pSpecificInfo->mPlayerId);
        if (pMixVolume == nullptr) {
            return nullptr;
        }
    }
    SePlayParamList* pParams = pKeeper->addHoldRequest(replacedId, pSource, pSpecificInfo, pMixVolume);
    const bool isMe = mMeInfoKeeper->isMe(id);
    if (pParams != nullptr && isMe) {
        mMeInfoKeeper->applyMeInfoToParams(alSoundNameUtil::getSoundName(id, false), pParams, pMeInfo);
    }
    applyMaterialLpf(pParams, pSpecificInfo, waterState, isBeyondWall);
    return pParams;
}
/**
 * @brief Stops a sound in the explicitly named or current main/demo keeper.
 * @param id Sound identifier to stop.
 * @param pSource Source attached to the sound.
 * @param fadeFrames Fade duration in update frames.
 * @param pPlayName Keeper routing name, or nullptr for current main/demo routing.
 */
void SeDirector::stop(u32 id, SeSource* pSource, u32 fadeFrames, const char* pPlayName) {
    SeRequestKeeper* pKeeper = selectRequestKeeper(pPlayName);
    if (pKeeper != nullptr) {
        pKeeper->stop(id, pSource, fadeFrames);
    }
}
/**
 * @brief Stops a sound across the primary keepers or one explicitly named keeper.
 * @param id Sound identifier to stop.
 * @param pSource Source attached to the sound.
 * @param fadeFrames Fade duration in update frames.
 * @param pPlayName Keeper routing name, or nullptr to stop demo, main, and player requests.
 */
void SeDirector::stopAllId(u32 id, SeSource* pSource, u32 fadeFrames, const char* pPlayName) {
    if (pPlayName == nullptr) {
        mDemoKeeper->stop(id, pSource, fadeFrames);
        mMainKeeper->stop(id, pSource, fadeFrames);
        mPlayerKeeper->stop(id, pSource, fadeFrames);
    } else {
        SeRequestKeeper* pKeeper = findRequestKeeper(pPlayName);
        if (pKeeper != nullptr) {
            pKeeper->stop(id, pSource, fadeFrames);
        }
    }
}
} // namespace al
