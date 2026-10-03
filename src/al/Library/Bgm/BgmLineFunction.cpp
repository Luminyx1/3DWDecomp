#include "Library/Bgm/BgmLineFunction.hpp"

#include "Library/Audio/AudioEventController.hpp"
#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Audio/System/AudioRequestKeeperSyncedBgm.hpp"
#include "Library/Bgm/BgmDirector.hpp"
#include "Library/Bgm/BgmKeeper.hpp"
#include "Library/Bgm/BgmLine.hpp"
#include "Library/Bgm/BgmRhythmCtrl.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Project/Action/Common/ActionBgmCtrl.hpp"
#include "Project/Action/Common/ActorActionKeeper.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Bgm/BgmPlayingRequest.hpp"

namespace {
al::BgmDirector* getUpperLayerBgmDirector(const al::IUseAudioKeeper* pUser) {
    al::IUseAudioKeeper* upperUser = al::getUpperLayerAudioUser(pUser);

    if (upperUser == nullptr) {
        return nullptr;
    }

    return al::getBgmDirector(upperUser);
}

al::AudioEventController* getAudioEventController(const al::IUseAudioKeeper* pUser) {
    al::AudioKeeper* audioKeeper = pUser->getAudioKeeper();

    if (audioKeeper == nullptr) {
        return nullptr;
    }

    return audioKeeper->getAudioEventController();
}

al::AudioRequestKeeperSyncedBgm* getAudioRequestKeeperSyncedBgm(const al::IUseAudioKeeper* pUser) {
    al::AudioKeeper* audioKeeper = pUser->getAudioKeeper();

    if (audioKeeper == nullptr) {
        return nullptr;
    }

    return audioKeeper->getAudioRequestKeeperSyncedBgm();
}

al::BgmRhythmCtrl* getActiveBgmRhythmCtrl(const al::IUseAudioKeeper* pUser) {
    al::BgmDirector* director = al::getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return nullptr;
    }

    return director->getBgmRhythmCtrl();
}
}  // namespace

namespace al {
/**
 * Gets the BGM director of an audio user.
 * @param pUser Audio user.
 * @return BGM director, or nullptr.
 */
BgmDirector* getBgmDirector(const IUseAudioKeeper* pUser) {
    AudioKeeper* audioKeeper = pUser->getAudioKeeper();

    if (audioKeeper == nullptr || audioKeeper->getBgmKeeper() == nullptr) {
        return nullptr;
    }

    return audioKeeper->getBgmKeeper()->getBgmDirector();
}

/**
 * Gets the BGM director of an audio user if it has one.
 * @param pUser Audio user.
 * @return BGM director, or nullptr.
 */
BgmDirector* tryGetBgmDirector(const IUseAudioKeeper* pUser) {
    AudioKeeper* audioKeeper = pUser->getAudioKeeper();

    if (audioKeeper == nullptr || audioKeeper->getBgmKeeper() == nullptr) {
        return nullptr;
    }

    return audioKeeper->getBgmKeeper()->getBgmDirector();
}

/**
 * Gets the upper layer audio user of an audio user.
 * @param pUser Audio user.
 * @return Upper layer audio user, or nullptr.
 */
IUseAudioKeeper* getUpperLayerAudioUser(const IUseAudioKeeper* pUser) {
    AudioKeeper* audioKeeper = pUser->getAudioKeeper();

    if (audioKeeper == nullptr) {
        return nullptr;
    }

    return audioKeeper->getUpperLayerAudioUser();
}

/**
 * Gets the upper layer audio user of an audio user if it has one.
 * @param pUser Audio user.
 * @return Upper layer audio user, or nullptr.
 */
IUseAudioKeeper* tryGetUpperLayerAudioUser(const IUseAudioKeeper* pUser) {
    AudioKeeper* audioKeeper = pUser->getAudioKeeper();

    if (audioKeeper == nullptr) {
        return nullptr;
    }

    return audioKeeper->getUpperLayerAudioUser();
}

/**
 * Gets the BGM director with an active line, preferring the upper layer one.
 * @param pUser Audio user.
 * @return BGM director, or nullptr.
 */
BgmDirector* getActiveBgmDirector(const IUseAudioKeeper* pUser) {
    if (pUser == nullptr) {
        return nullptr;
    }

    IUseAudioKeeper* upperUser = getUpperLayerAudioUser(pUser);

    if (upperUser == nullptr) {
        return getBgmDirector(pUser);
    }

    BgmDirector* upperDirector = getBgmDirector(upperUser);

    if (upperDirector == nullptr) {
        return getBgmDirector(pUser);
    }

    if (upperDirector->getActiveBgmLine() == nullptr) {
        return getBgmDirector(pUser);
    }

    return upperDirector;
}

/**
 * Gets the BGM director with an active line if there is one, preferring the upper layer one.
 * @param pUser Audio user.
 * @return BGM director, or nullptr.
 */
BgmDirector* tryGetActiveBgmDirector(const IUseAudioKeeper* pUser) {
    if (pUser == nullptr) {
        return nullptr;
    }

    IUseAudioKeeper* upperUser = tryGetUpperLayerAudioUser(pUser);

    if (upperUser != nullptr) {
        BgmDirector* upperDirector = tryGetBgmDirector(upperUser);

        if (upperDirector != nullptr && upperDirector->getActiveBgmLine() != nullptr) {
            return upperDirector;
        }
    }

    return tryGetBgmDirector(pUser);
}

/**
 * Gets the play name of the BGM playing on the active line.
 * @param pUser Audio user.
 * @return Play name, or nullptr.
 */
const char* getCurPlayingBgmPlayName(const IUseAudioKeeper* pUser) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return nullptr;
    }

    BgmLine* line = director->getActiveBgmLine();

    if (line == nullptr) {
        return nullptr;
    }

    return line->getCurPlayName();
}

/**
 * Starts a BGM on the upper layer audio user.
 * @param pUser Audio user.
 * @param pName Play name.
 * @param fadeInFrames Fade in frames.
 * @param startDelayFrames Start delay frames.
 */
void startSequenceBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeInFrames, s32 startDelayFrames) {
    BgmDirector* director = getUpperLayerBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    BgmPlayingRequest request(pName, fadeInFrames, startDelayFrames);
    director->startBgm(request);
}

/**
 * Starts a BGM.
 * @param pUser Audio user.
 * @param pName Play name.
 * @param fadeInFrames Fade in frames.
 * @param startDelayFrames Start delay frames.
 * @param fadeOutFrames Fade out frames of the current BGM.
 * @param unk Unknown.
 */
void startBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeInFrames, s32 startDelayFrames,
              s32 fadeOutFrames, s32 unk) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    BgmPlayingRequest request(pName, fadeInFrames, startDelayFrames, fadeOutFrames, false, unk);
    director->startBgm(request);
}

/**
 * Starts a BGM on the upper layer audio user.
 * @param pUser Audio user.
 * @param rRequest Play request.
 */
void startSequenceBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest) {
    BgmDirector* director = getUpperLayerBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->startBgm(rRequest);
}

/**
 * Starts a BGM.
 * @param pUser Audio user.
 * @param rRequest Play request.
 */
void startBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->startBgm(rRequest);
}

/**
 * Stops a BGM on the upper layer audio user.
 * @param pUser Audio user.
 * @param pName Play name.
 * @param fadeOutFrames Fade out frames.
 */
void stopSequenceBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeOutFrames) {
    BgmDirector* director = getUpperLayerBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->stopBgm(pName, fadeOutFrames, -1);
}

/**
 * Stops a BGM.
 * @param pUser Audio user.
 * @param pName Play name.
 * @param fadeOutFrames Fade out frames.
 * @param unk Unknown.
 */
void stopBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeOutFrames, s32 unk) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->stopBgm(pName, fadeOutFrames, unk);
}

/**
 * Stops a BGM on the upper layer audio user.
 * @param pUser Audio user.
 * @param rRequest Play request.
 */
void stopSequenceBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest) {
    BgmDirector* director = getUpperLayerBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->stopBgm(rRequest.name, rRequest.fadeInFrames, -1);
}

/**
 * Stops a BGM.
 * @param pUser Audio user.
 * @param rRequest Play request.
 */
void stopBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->stopBgm(rRequest.name, rRequest.fadeInFrames, -1);
}

/**
 * Stops the BGM playing on the active line on the upper layer audio user.
 * @param pUser Audio user.
 * @param fadeOutFrames Fade out frames.
 */
void stopActiveSequenceBgm(const IUseAudioKeeper* pUser, s32 fadeOutFrames) {
    const char* name = getCurPlayingBgmPlayName(pUser);

    if (name == nullptr) {
        return;
    }

    BgmDirector* director = getUpperLayerBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->stopBgm(name, fadeOutFrames, -1);
}

/**
 * Prepares a BGM on the upper layer audio user.
 * @param pUser Audio user.
 * @param rRequest Play request.
 */
void prepareSequenceBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest) {
    BgmDirector* director = getUpperLayerBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->prepareBgm(rRequest);
}

/**
 * Prepares a BGM.
 * @param pUser Audio user.
 * @param rRequest Play request.
 */
void prepareBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->prepareBgm(rRequest);
}

/**
 * Starts a prepared BGM on the upper layer audio user.
 * @param pUser Audio user.
 * @param pName Play name.
 */
void startSequencePreparedBgm(const IUseAudioKeeper* pUser, const char* pName) {
    BgmDirector* director = getUpperLayerBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->startPreparedBgm(pName);
}

/**
 * Starts a prepared BGM.
 * @param pUser Audio user.
 * @param pName Play name.
 */
void startPreparedBgm(const IUseAudioKeeper* pUser, const char* pName) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->startPreparedBgm(pName);
}

/**
 * Starts the BGM of the BGM change area player one is in on the upper layer audio user.
 * @param pUser Audio user.
 * @param isRestart Whether to restart the BGM.
 * @param fadeInFrames Fade in frames.
 * @param startDelayFrames Start delay frames.
 * @param fadeOutFrames Fade out frames of the current BGM.
 */
void startSequenceBgmWithAreaCheck(const IUseAudioKeeper* pUser, bool isRestart, s32 fadeInFrames,
                                   s32 startDelayFrames, s32 fadeOutFrames) {
    IUseAudioKeeper* upperUser = getUpperLayerAudioUser(pUser);

    if (upperUser == nullptr) {
        return;
    }

    AudioEventController* controller = getAudioEventController(pUser);

    if (controller == nullptr || controller->isInBgmStopArea()) {
        return;
    }

    const char* name = controller->getBgmPlayNameByAreaChecker(true);

    if (name != nullptr) {
        BgmPlayingRequest request(name, fadeInFrames, startDelayFrames, fadeOutFrames, isRestart);
        startBgm(upperUser, request);
    }

    const char* situationName = controller->getBgmSituationNameByAreaChecker();

    if (situationName != nullptr) {
        changeBgmSituation(upperUser, situationName);
    }
}

/**
 * Changes the BGM situation of the audio user and its upper layer audio user.
 * @param pUser Audio user.
 * @param pName Situation name.
 */
void changeBgmSituation(const IUseAudioKeeper* pUser, const char* pName) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director != nullptr) {
        director->changeSituation(pName);
    }

    BgmDirector* upperDirector = getUpperLayerBgmDirector(pUser);

    if (upperDirector != nullptr) {
        upperDirector->changeSituation(pName);
    }
}

/**
 * Pauses a BGM on the upper layer audio user.
 * @param pUser Audio user.
 * @param pName Play name.
 * @param fadeFrames Fade frames.
 */
void pauseSequenceBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeFrames) {
    BgmDirector* director = getUpperLayerBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->pauseBgm(pName, fadeFrames);
}

/**
 * Pauses a BGM.
 * @param pUser Audio user.
 * @param pName Play name.
 * @param fadeFrames Fade frames.
 */
void pauseBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeFrames) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->pauseBgm(pName, fadeFrames);
}

/**
 * Resumes a BGM on the upper layer audio user.
 * @param pUser Audio user.
 * @param pName Play name.
 * @param fadeFrames Fade frames.
 */
void resumeSequenceBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeFrames) {
    BgmDirector* director = getUpperLayerBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->resumeBgm(pName, fadeFrames);
}

/**
 * Resumes a BGM.
 * @param pUser Audio user.
 * @param pName Play name.
 * @param fadeFrames Fade frames.
 */
void resumeBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeFrames) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->resumeBgm(pName, fadeFrames);
}

/**
 * Prepares the BGM of the BGM change area player one is in.
 * @param pUser Audio user.
 */
void prepareBgmWithAreaCheck(const IUseAudioKeeper* pUser) {
    AudioEventController* controller = getAudioEventController(pUser);

    if (controller == nullptr || controller->isInBgmStopArea()) {
        return;
    }

    BgmPlayingRequest request(controller->getBgmPlayNameByAreaChecker(false));
    prepareBgm(pUser, request);
}

/**
 * Starts the BGM of the BGM change area player one is in.
 * @param pUser Audio user.
 * @param isRestart Whether to restart the BGM.
 * @param fadeInFrames Fade in frames.
 * @param startDelayFrames Start delay frames.
 * @param fadeOutFrames Fade out frames of the current BGM.
 */
void startBgmWithAreaCheck(const IUseAudioKeeper* pUser, bool isRestart, s32 fadeInFrames, s32 startDelayFrames,
                           s32 fadeOutFrames) {
    AudioEventController* controller = getAudioEventController(pUser);

    if (controller == nullptr || controller->isInBgmStopArea()) {
        return;
    }

    BgmPlayingRequest request(controller->getBgmPlayNameByAreaChecker(false), fadeInFrames, startDelayFrames,
                              fadeOutFrames, isRestart);
    startBgm(pUser, request);
    const char* situationName = controller->getBgmSituationNameByAreaChecker();

    if (situationName != nullptr) {
        changeBgmSituation(pUser, situationName);
    }
}

/**
 * Pauses the active BGM.
 * @param pUser Audio user.
 * @param fadeFrames Fade frames.
 */
void pauseActiveBgm(const IUseAudioKeeper* pUser, s32 fadeFrames) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->pauseActiveBgm(fadeFrames);
}

/**
 * Resumes the active BGM.
 * @param pUser Audio user.
 * @param fadeFrames Fade frames.
 */
void resumeActiveBgm(const IUseAudioKeeper* pUser, s32 fadeFrames) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->resumeActiveBgm(fadeFrames);
}

/**
 * Pauses the island BGM.
 * @param pUser Audio user.
 * @param fadeFrames Fade frames.
 */
void pauseIslandBgm(const IUseAudioKeeper* pUser, s32 fadeFrames) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->pauseIslandBgm(fadeFrames);
}

/**
 * Resumes the island BGM.
 * @param pUser Audio user.
 * @param fadeFrames Fade frames.
 */
void resumeIslandBgm(const IUseAudioKeeper* pUser, s32 fadeFrames) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->resumeIslandBgm(fadeFrames);
}

/**
 * Pauses the ocean BGM.
 * @param pUser Audio user.
 * @param fadeFrames Fade frames.
 */
void pauseOceanBgm(const IUseAudioKeeper* pUser, s32 fadeFrames) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->pauseOceanBgm(fadeFrames);
}

/**
 * Resumes the ocean BGM.
 * @param pUser Audio user.
 * @param fadeFrames Fade frames.
 */
void resumeOceanBgm(const IUseAudioKeeper* pUser, s32 fadeFrames) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->resumeOceanBgm(fadeFrames);
}

/**
 * Checks whether the active BGM is paused.
 * @param pUser Audio user.
 * @return True if the active BGM is paused.
 */
bool isPauseActiveBgm(const IUseAudioKeeper* pUser) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return false;
    }

    return director->isPauseActiveBgm();
}

/**
 * Checks whether a BGM is paused.
 * @param pUser Audio user.
 * @param pName Play name.
 * @return True if the BGM is paused.
 */
bool isPauseBgm(const IUseAudioKeeper* pUser, const char* pName) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return false;
    }

    return director->isPauseBgm(pName);
}

/**
 * Stops all BGMs.
 * @param pUser Audio user.
 * @param fadeOutFrames Fade out frames.
 */
void stopAllBgm(const IUseAudioKeeper* pUser, s32 fadeOutFrames) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->stopAllBgm(fadeOutFrames);
}

/**
 * Stops all BGMs of the upper layer audio user.
 * @param pUser Audio user.
 * @param fadeOutFrames Fade out frames.
 */
void stopAllSequenceBgm(const IUseAudioKeeper* pUser, s32 fadeOutFrames) {
    IUseAudioKeeper* upperUser = pUser->getAudioKeeper()->getUpperLayerAudioUser();
    BgmDirector* director = getBgmDirector(upperUser);

    if (upperUser == nullptr || director == nullptr) {
        return;
    }

    director->stopAllBgm(fadeOutFrames);
}

/**
 * Stops all BGMs of the audio user and its upper layer audio user.
 * @param pUser Audio user.
 * @param fadeOutFrames Fade out frames.
 */
void tryStopAllBgm(const IUseAudioKeeper* pUser, s32 fadeOutFrames) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->tryStopAllBgm(fadeOutFrames);
    IUseAudioKeeper* upperUser = pUser->getAudioKeeper()->getUpperLayerAudioUser();
    BgmDirector* upperDirector = getBgmDirector(upperUser);

    if (upperUser == nullptr || upperDirector == nullptr) {
        return;
    }

    getBgmDirector(upperUser)->tryStopAllBgm(fadeOutFrames);
}

/**
 * Checks whether a BGM is currently playing.
 * @param pUser Audio user.
 * @param pName Play name.
 * @return True if the BGM is playing.
 */
bool isBgmCurrentlyPlaying(const IUseAudioKeeper* pUser, const char* pName) {
    if (pName == nullptr) {
        return false;
    }

    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return false;
    }

    return director->isBgmCurrentlyPlaying(pName);
}

/**
 * Pauses the BGM of the actor's area if the BGM area at the given position differs.
 * @param pActor Actor.
 * @param rPos Position.
 * @param fadeFrames Fade frames.
 */
void tryPauseBgmIfDifferBgmArea(const LiveActor* pActor, const sead::Vector3f& rPos, s32 fadeFrames) {
    AudioEventController* controller = getAudioEventController(pActor);
    const char* curName = controller->getBgmPlayNameInThisPosition(getTrans(pActor));
    const char* nextName = controller->getBgmPlayNameInThisPosition(rPos);

    if (curName == nullptr) {
        return;
    }

    if (nextName == nullptr) {
        getActiveBgmDirector(pActor)->pauseBgm(curName, fadeFrames);
        return;
    }

    if (!isEqualString(curName, nextName)) {
        getActiveBgmDirector(pActor)->tryPauseBgmIfNotPlaying(nextName, fadeFrames);
    }
}

/**
 * Disables situation changes of the audio user and its upper layer audio user.
 * @param pUser Audio user.
 */
void disableChangeSituation(const IUseAudioKeeper* pUser) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director != nullptr) {
        director->setIsDisableChangeSituation(true);
    }

    BgmDirector* upperDirector = getUpperLayerBgmDirector(pUser);

    if (upperDirector != nullptr) {
        upperDirector->setIsDisableChangeSituation(true);
    }
}

/**
 * Enables situation changes of the audio user and its upper layer audio user.
 * @param pUser Audio user.
 */
void enableChangeSituation(const IUseAudioKeeper* pUser) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director != nullptr) {
        director->setIsDisableChangeSituation(false);
    }

    BgmDirector* upperDirector = getUpperLayerBgmDirector(pUser);

    if (upperDirector != nullptr) {
        upperDirector->setIsDisableChangeSituation(false);
    }
}

/**
 * Gets the situation name of a BGM line.
 * @param pUser Audio user.
 * @param pLineName Line name.
 * @return Situation name.
 */
const char* getBgmLineSituationName(const IUseAudioKeeper* pUser, const char* pLineName) {
    return getBgmDirector(pUser)->getBgmLineSituationName(pLineName);
}

/**
 * Checks whether the situation name of a BGM line is the given one.
 * @param pUser Audio user.
 * @param pLineName Line name.
 * @param pName Situation name.
 * @return True if the situation names are equal.
 */
bool isEqualBgmLineSituationName(const IUseAudioKeeper* pUser, const char* pLineName, const char* pName) {
    const char* situationName = getBgmLineSituationName(pUser, pLineName);

    if (situationName == nullptr) {
        return false;
    }

    return isEqualString(situationName, pName);
}

/**
 * Checks whether the situation name of the active BGM line is the given one.
 * @param pUser Audio user.
 * @param pName Situation name.
 * @return True if the situation names are equal.
 */
bool isEqualBgmActiveLineSituationName(const IUseAudioKeeper* pUser, const char* pName) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return false;
    }

    BgmLine* line = director->getActiveBgmLine();

    if (line == nullptr || line->getSituationName() == nullptr) {
        return false;
    }

    return isEqualString(line->getSituationName(), pName);
}

/**
 * Disables BGM change areas.
 * @param pUser Audio user.
 */
void disableBgmChangeArea(const IUseAudioKeeper* pUser) {
    pUser->getAudioKeeper()->getAudioEventController()->setIsDisableBgmChangeArea(true);
}

/**
 * Enables BGM change areas.
 * @param pUser Audio user.
 */
void enableBgmChangeArea(const IUseAudioKeeper* pUser) {
    pUser->getAudioKeeper()->getAudioEventController()->setIsDisableBgmChangeArea(false);
}

/**
 * Requests starting a BGM synced to the beat of the current BGM.
 * @param pUser Audio user.
 * @param rRequest Play request.
 * @param unk Unknown.
 */
void startBgmSyncedCurBgmBeat(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest, s32 unk) {
    getAudioRequestKeeperSyncedBgm(pUser)->requestBgm(static_cast<BgmPlayingType>(0), rRequest, unk);
}

/**
 * Requests stopping a BGM synced to the beat of the current BGM.
 * @param pUser Audio user.
 * @param rRequest Play request.
 * @param unk Unknown.
 */
void stopBgmSyncedCurBgmBeat(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest, s32 unk) {
    getAudioRequestKeeperSyncedBgm(pUser)->requestBgm(static_cast<BgmPlayingType>(1), rRequest, unk);
}

/**
 * Changes the auto stop mode of a BGM line.
 * @param pUser Audio user.
 * @param pLineName Line name.
 * @param isAutoStop Whether the line stops automatically.
 */
void changeLineAutoStopMode(const IUseAudioKeeper* pUser, const char* pLineName, bool isAutoStop) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->changeLineAutoStopMode(pLineName, isAutoStop);
}

/**
 * Disables or enables BGM line changes.
 * @param pUser Audio user.
 * @param isDisable Whether to disable line changes.
 */
void disableLineChange(const IUseAudioKeeper* pUser, bool isDisable) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->disableLineChange(isDisable);
}

/**
 * Disables starting BGMs.
 * @param pUser Audio user.
 */
void disableBgmStart(const IUseAudioKeeper* pUser) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->setIsDisableBgmStart(true);
}

/**
 * Enables starting BGMs.
 * @param pUser Audio user.
 */
void enableBgmStart(const IUseAudioKeeper* pUser) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->setIsDisableBgmStart(false);
}

/**
 * Changes the BGM volume.
 * @param pUser Audio user.
 * @param volume Volume.
 * @param frames Frames to change over.
 */
void changeBgmVolume(const IUseAudioKeeper* pUser, f32 volume, s32 frames) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->changeBgmVolume(volume, frames);
}

/**
 * Disables BGM volume changes.
 * @param pUser Audio user.
 */
void disableVolumeChange(const IUseAudioKeeper* pUser) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->setIsDisableVolumeChange(true);
}

/**
 * Enables BGM volume changes.
 * @param pUser Audio user.
 */
void enableVolumeChange(const IUseAudioKeeper* pUser) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->setIsDisableVolumeChange(false);
}

/**
 * Changes the island map BGM volume.
 * @param pUser Audio user.
 * @param unk1 Unknown.
 * @param unk2 Unknown.
 * @param unk3 Unknown.
 */
void changeIslandMapBgmVolume(const IUseAudioKeeper* pUser, s32 unk1, s32 unk2, bool unk3) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->changeIslandMapBgmVolume(unk1, unk2, unk3);
}

/**
 * Gets the sample position of a BGM.
 * @param pUser Audio user.
 * @param pName Play name.
 * @return Sample position, or -1.
 */
s32 getBgmSamplePos(const IUseAudioKeeper* pUser, const char* pName) {
    BgmDirector* director = getBgmDirector(pUser);

    if (director == nullptr) {
        return -1;
    }

    return director->getBgmSamplePos(pName);
}

/**
 * Prepares the first BGM of an action.
 * @param pActor Actor.
 * @param pActionName Action name.
 */
void tryPrepareActionFirstBgm(const LiveActor* pActor, const char* pActionName) {
    pActor->getActorActionKeeper()->getBgmCtrl()->tryPrepareActionFirstBgm(pActionName, false, 0);
}

/**
 * Sets the pitch of the active BGM.
 * @param pUser Audio user.
 * @param pitch Pitch.
 */
void setActiveBgmPitch(const IUseAudioKeeper* pUser, f32 pitch) {
    BgmDirector* director = getActiveBgmDirector(pUser);

    if (director == nullptr) {
        return;
    }

    director->setActiveBgmPitch(pitch);
}

/**
 * Checks whether rhythm animations are enabled, failing if another BGM than the given one is playing.
 * @param pUser Audio user.
 * @param pName Play name.
 * @return True if rhythm animations are enabled.
 */
bool isEnableRhythmAnim(const IUseAudioKeeper* pUser, const char* pName) {
    if (pName != nullptr) {
        const char* curName = getCurPlayingBgmPlayName(pUser);

        if (curName != nullptr && !isEqualString(pName, curName)) {
            return false;
        }
    }

    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return false;
    }

    return rhythmCtrl->isEnableRhythmAnim();
}

/**
 * Checks whether the BGM restarted.
 * @param pUser Audio user.
 * @return True if the BGM restarted.
 */
bool isTriggerRestartBgm(const IUseAudioKeeper* pUser) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return false;
    }

    return rhythmCtrl->isTriggerRestartBgm();
}

/**
 * Checks whether the given beat was reached.
 * @param pUser Audio user.
 * @param beat Beat.
 * @return True if the beat was reached.
 */
bool isTriggerBeat(const IUseAudioKeeper* pUser, s32 beat) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return false;
    }

    return rhythmCtrl->isTriggerBeat(beat);
}

/**
 * Checks whether the given animation beat was reached.
 * @param pUser Audio user.
 * @param beat Beat.
 * @return True if the beat was reached.
 */
bool isTriggerBeatForAnime(const IUseAudioKeeper* pUser, s32 beat) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return false;
    }

    return rhythmCtrl->isTriggerBeatForAnime(beat);
}

/**
 * Checks whether a rhythm was triggered.
 * @param pUser Audio user.
 * @return True if a rhythm was triggered.
 */
bool isTriggerRhythm(const IUseAudioKeeper* pUser) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return false;
    }

    return rhythmCtrl->isTriggerRhythm();
}

/**
 * Checks whether the rhythm animation changed.
 * @param pUser Audio user.
 * @return True if the rhythm animation changed.
 */
bool isTriggerRhythmAnimChange(const IUseAudioKeeper* pUser) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return false;
    }

    return rhythmCtrl->isTriggerAnimChange();
}

/**
 * Gets the rhythm animation type.
 * @param pUser Audio user.
 * @return Animation type, or -1.
 */
s32 getRhythmAnimType(const IUseAudioKeeper* pUser) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return -1;
    }

    return rhythmCtrl->getAnimType();
}

/**
 * Gets the rhythm animation frame.
 * @param pUser Audio user.
 * @return Animation frame, or -1.
 */
f32 getRhythmAnimFrame(const IUseAudioKeeper* pUser) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return -1.0f;
    }

    return rhythmCtrl->getAnimFrame();
}

/**
 * Gets the beat rate.
 * @param pUser Audio user.
 * @return Beat rate, or -1.
 */
f32 getBeatRate(const IUseAudioKeeper* pUser) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return -1.0f;
    }

    return rhythmCtrl->getBeatRate();
}

/**
 * Gets the beat rate for animations.
 * @param pUser Audio user.
 * @return Beat rate, or -1.
 */
f32 getBeatRateForAnime(const IUseAudioKeeper* pUser) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return -1.0f;
    }

    return rhythmCtrl->getBeatRateForAnime();
}

/**
 * Gets the current beat.
 * @param pUser Audio user.
 * @return Current beat, or -1.
 */
f32 getCurBeat(const IUseAudioKeeper* pUser) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return -1.0f;
    }

    return rhythmCtrl->getCurBeat();
}

/**
 * Gets the beats per frame.
 * @param pUser Audio user.
 * @return Beats per frame, or -1.
 */
f32 getBeatPerFrame(const IUseAudioKeeper* pUser) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return -1.0f;
    }

    return rhythmCtrl->getBeatPerFrame();
}

/**
 * Gets the frame rate.
 * @param pUser Audio user.
 * @return Frame rate, or -1.
 */
f32 getFrameRate(const IUseAudioKeeper* pUser) {
    BgmRhythmCtrl* rhythmCtrl = getActiveBgmRhythmCtrl(pUser);

    if (rhythmCtrl == nullptr) {
        return -1.0f;
    }

    return rhythmCtrl->getFrameRate();
}

/**
 * Does nothing.
 * @param pUser Audio user.
 */
void pauseOnBgm(const IUseAudioKeeper* pUser) {}

/**
 * Does nothing.
 * @param pUser Audio user.
 */
void pauseOffBgm(const IUseAudioKeeper* pUser) {}

/**
 * Does nothing.
 * @param pUser Audio user.
 * @param track Track.
 * @param unk Unknown.
 */
void muteOnRunningLineTrack(IUseAudioKeeper* pUser, u32 track, bool unk) {}

/**
 * Does nothing.
 * @param pUser Audio user.
 * @param track Track.
 * @param unk Unknown.
 */
void muteOffRunningLineTrack(IUseAudioKeeper* pUser, u32 track, bool unk) {}

}  // namespace al
