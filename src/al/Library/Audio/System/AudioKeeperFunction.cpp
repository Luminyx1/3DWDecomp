#include "Library/Audio/System/AudioKeeperFunction.hpp"

#include <attributes.h>
#include <audio/seadAudioMgr.h>
#include <audio/seadAudioSoundDataMgrNin.h>
#include <audio/seadAudioSoundHeapNin.h>
#include <heap/seadHeapMgr.h>
#include <nn/atk/atk_HardwareManager.h>
#include <nn/atk/atk_SoundDataManager.h>

#include "Library/Audio/AudioDirector.hpp"
#include "Library/Audio/AudioEventController.hpp"
#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/Bgm/BgmDirector.hpp"
#include "Library/Bgm/BgmLineFunction.hpp"
#include "Library/Memory/HeapUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Se/Function/SeDirector.hpp"
#include "Library/Se/Function/SeEffectController.hpp"
#include "Library/System/GameSystemInfo.hpp"
#include "Project/Audio/System/AudioPlayer.hpp"
#include "Project/Audio/System/AudioSystem.hpp"
#include "Project/Audio/System/SoundHeapPtrWrapper.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
Resource* findOrCreateResourceCategory(const sead::SafeString& rPath, const sead::SafeString& rCategory,
                                       const char* pExt);
}

namespace {
void attachSoundMemoryPool(sead::SoundMemoryPoolHandler* pHandler, al::SeadAudioPlayer* pPlayer) {
    pPlayer->getSoundDataMgr()->SetFileAddressInGroupFile(pHandler->getData(), pHandler->getDataSize());
    nn::audio::AcquireMemoryPool(
        &nn::atk::detail::driver::HardwareManager::GetInstance().GetAudioRendererConfig(),
        pHandler->getMemoryPool(), pHandler->getData(), pHandler->getBufferSize());
    nn::audio::RequestAttachMemoryPool(pHandler->getMemoryPool());
    pHandler->setMemoryPoolAttached(true);

    while (!nn::audio::IsMemoryPoolAttached(pHandler->getMemoryPool())) {
    }
}
}  // namespace

namespace alAudioSystemFunction {
/**
 * Gets the audio system information of the game system.
 * @param pInfo Game system information.
 * @return Audio system information.
 */
al::AudioSystemInfo* getAudioSystemInfo(const al::GameSystemInfo* pInfo) {
    return static_cast<al::AudioSystem*>(pInfo->_0)->getAudioSystemInfo();
}

/**
 * Loads a sound group file that the user manages and registers its memory pool.
 * @param pFileName File name inside SoundData.
 * @param pPlayer Audio player that uses the file.
 * @param isAttachMemoryPool Whether to attach the memory pool right away.
 * @return Size of the loaded file, or 0.
 */
u32 loadResourceFromUserManagementFile(const char* pFileName, al::SeadAudioPlayer* pPlayer,
                                       bool isAttachMemoryPool) {
    al::StringTmp<128> archivePath("SoundData/%s", pFileName);
    al::Resource* resource =
        al::findOrCreateResourceCategory(archivePath.cstr(), "常駐[オーディオ]", "sarc");
    sead::ScopedCurrentHeapSetter heapSetter(al::tryFindNamedHeap("AudioStationedResourceHeap"));
    sead::SoundMemoryPoolHandler* handler = new sead::SoundMemoryPoolHandler(pFileName);
    handler->setMemoryPoolAttached(false);

    if (!pPlayer->trySetSoundMemoryPoolHandler(handler)) {
        return 0;
    }

    al::StringTmp<128> groupFileName("%s.bfgrp", pFileName);
    void* data = resource->getOtherFile(groupFileName.cstr(), nullptr);
    u32 size = resource->getFileSize(groupFileName.cstr());
    handler->setData(data, size);

    if (isAttachMemoryPool) {
        attachSoundMemoryPool(handler, pPlayer);
    }

    return size;
}

/**
 * Attaches the memory pool of a registered sound group file.
 * @param pFileName File name inside SoundData.
 * @param pPlayer Audio player that uses the file.
 * @return True if the memory pool is attached.
 */
bool tryAttachMemoryPool(const char* pFileName, al::SeadAudioPlayer* pPlayer) {
    sead::SoundMemoryPoolHandler* handler = pPlayer->tryGetSoundMemoryPoolHandler(pFileName);

    if (handler == nullptr) {
        return false;
    }

    if (!handler->isMemoryPoolAttached()) {
        attachSoundMemoryPool(handler, pPlayer);
    }

    return true;
}

/**
 * Pauses or resumes SE and BGM because of a system error.
 * @param pDirector Main audio director.
 * @param pSubDirector Sub audio director.
 * @param isPause Whether to pause.
 * @param fadeFrames Fade length in frames.
 * @return Result of the BGM pause of the sub director.
 */
bool pauseBySystemError(const al::AudioDirector* pDirector, const al::AudioDirector* pSubDirector, bool isPause,
                        u32 fadeFrames) {
    bool result;
    bool isActiveSubBgm = false;

    if (pSubDirector != nullptr) {
        pSubDirector->getSeDirector()->pauseSystemExceptSub(isPause, "システムポーズ", fadeFrames);
        al::BgmDirector* bgmDirector = pSubDirector->getBgmDirector();
        isActiveSubBgm = bgmDirector->getActiveBgmLine() != nullptr;

        if (isPause) {
            result = bgmDirector->pauseActiveBgmById(1, fadeFrames);
        } else {
            result = bgmDirector->resumeActiveBgmById(1, fadeFrames);
        }
    }

    if (pDirector != nullptr) {
        pDirector->getSeDirector()->pauseSystemExceptSub(isPause, "システムポーズ", fadeFrames);

        if (!isActiveSubBgm) {
            if (isPause) {
                return pDirector->getBgmDirector()->pauseActiveBgmById(1, fadeFrames);
            }

            return pDirector->getBgmDirector()->resumeActiveBgmById(1, fadeFrames);
        }
    }

    return result;
}

/**
 * Pauses or resumes SE and the active BGM for a system pause.
 * @param pDirector Audio director.
 * @param pUser Audio user owning the BGM.
 * @param isPause Whether to pause.
 * @param fadeFrames Fade length in frames.
 */
void pauseSystem(const al::AudioDirector* pDirector, const al::IUseAudioKeeper* pUser, bool isPause,
                 u32 fadeFrames) {
    if (pDirector == nullptr) {
        return;
    }

    if (pDirector->getSeDirector() != nullptr) {
        pDirector->getSeDirector()->pauseSystemExceptSub(isPause, "システムポーズ", fadeFrames);
    }

    if (pUser == nullptr) {
        return;
    }

    al::BgmDirector* bgmDirector = al::getActiveBgmDirector(pUser);

    if (bgmDirector == nullptr) {
        return;
    }

    if (isPause) {
        bgmDirector->pauseActiveBgmById(1, fadeFrames);
    } else {
        bgmDirector->resumeActiveBgmById(1, fadeFrames);
    }
}

/**
 * Pauses or resumes SE and the active BGM for debugging.
 * @param pDirector Audio director.
 * @param pUser Audio user owning the BGM.
 * @param isPause Whether to pause.
 * @param fadeFrames Fade length in frames.
 */
void pauseSystemForDebug(const al::AudioDirector* pDirector, const al::IUseAudioKeeper* pUser, bool isPause,
                         u32 fadeFrames) {
    if (pDirector != nullptr && pDirector->getSeDirector() != nullptr) {
        pDirector->getSeDirector()->pauseSystemExceptSub(isPause, "システムポーズ", fadeFrames);
    }

    if (pUser == nullptr) {
        return;
    }

    al::BgmDirector* bgmDirector = al::tryGetActiveBgmDirector(pUser);

    if (bgmDirector == nullptr) {
        return;
    }

    if (isPause) {
        bgmDirector->pauseActiveBgmById(4, fadeFrames);
    } else {
        bgmDirector->resumeActiveBgmById(4, fadeFrames);
    }
}

/**
 * Does nothing in release builds.
 * @param pDirector Audio director.
 * @param isPause Whether to pause.
 * @param fadeFrames Fade length in frames.
 */
void pauseAudioDirectorForDebug(al::AudioDirector* pDirector, bool isPause, u32 fadeFrames) {}

/**
 * Starts a demo for SE.
 * @param pDirector Audio director.
 * @param type Demo type.
 */
void startDemo(al::AudioDirector* pDirector, alSeFunction::DemoType type) {
    pDirector->getSeDirector()->startDemo(type);
}

/**
 * Checks whether a demo is running.
 * @param pDirector Audio director.
 * @return True if in a demo.
 */
bool isInDemo(al::AudioDirector* pDirector) {
    return pDirector->getSeDirector()->isInDemo();
}

/**
 * Ends a demo for SE.
 * @param pDirector Audio director.
 * @param type Demo type.
 */
void endDemo(al::AudioDirector* pDirector, alSeFunction::DemoType type) {
    pDirector->getSeDirector()->endDemo(type);
}

/**
 * Changes the running demo type.
 * @param pDirector Audio director.
 * @param from Current demo type.
 * @param to New demo type.
 */
void changeDemo(al::AudioDirector* pDirector, alSeFunction::DemoType from, alSeFunction::DemoType to) {
    pDirector->getSeDirector()->changeDemo(from, to);
}

/**
 * Stops all demo SE.
 * @param pDirector Audio director.
 */
void forceStopDemoSe(al::AudioDirector* pDirector) {
    pDirector->getSeDirector()->stopAllDemo(30);
}

/**
 * Stops all SE and BGM of both directors.
 * @param pDirector Main audio director.
 * @param pSubDirector Sub audio director.
 */
void softReset(const al::AudioDirector* pDirector, const al::AudioDirector* pSubDirector) {
    if (pDirector != nullptr) {
        if (pDirector->getSeDirector() != nullptr) {
            pDirector->getSeDirector()->stopAll(30, nullptr, nullptr);
        }

        if (pDirector->getBgmDirector() != nullptr) {
            pDirector->getBgmDirector()->stopAllBgm(30);
        }
    }

    if (pSubDirector != nullptr) {
        if (pSubDirector->getSeDirector() != nullptr) {
            pSubDirector->getSeDirector()->stopAll(30, nullptr, nullptr);
        }

        if (pSubDirector->getBgmDirector() != nullptr) {
            pSubDirector->getBgmDirector()->stopAllBgm(30);
        }
    }
}

/**
 * Stops all SE after a demo is skipped.
 * @param pDirector Audio director.
 * @param fadeFrames Fade length in frames.
 */
void stopAllSeAfterDemoSkip(const al::AudioDirector* pDirector, u32 fadeFrames) {
    pDirector->getSeDirector()->stopAll(fadeFrames, nullptr, nullptr);
}

/**
 * Gets the used size of the SE sound heap.
 * @param pDirector Audio director.
 * @return Used size in bytes.
 */
s32 getSeSoundHeapUsedSize(const al::AudioDirector* pDirector) {
    sead::AudioSoundHeapNin* heap =
        pDirector->getAudioSystemInfo()->getSeadAudioPlayerForSe()->getSeadAudioSoundHeap();
    return heap->GetSize() - heap->GetFreeSize();
}

/**
 * Gets the used size of the BGM sound heap.
 * @param pDirector Audio director.
 * @return Used size in bytes.
 */
s32 getBgmSoundHeapUsedSize(const al::AudioDirector* pDirector) {
    sead::AudioSoundHeapNin* heap =
        pDirector->getAudioSystemInfo()->getSeadAudioPlayerForBgm()->getSeadAudioSoundHeap();
    return heap->GetSize() - heap->GetFreeSize();
}

/**
 * Gets the free size of the SE sound heap.
 * @param pDirector Audio director.
 * @return Free size in bytes.
 */
u64 getHeapFreeSize(const al::AudioDirector* pDirector) {
    al::SeadAudioSoundHeapPtrWrapper wrapper;
    wrapper.setSoundHeap(pDirector->getAudioSystemInfo()->getSeadAudioPlayerForSe()->getSoundHeap());
    return wrapper.getHeapFreeSize();
}

/**
 * Gets the size of the SE sound heap.
 * @param pDirector Audio director.
 * @return Size in bytes.
 */
u64 getHeapSize(const al::AudioDirector* pDirector) {
    al::SeadAudioSoundHeapPtrWrapper wrapper;
    wrapper.setSoundHeap(pDirector->getAudioSystemInfo()->getSeadAudioPlayerForSe()->getSoundHeap());
    return wrapper.getHeapSize();
}

/**
 * Loads a sound item.
 * @param pUser Audio player user.
 * @param id Item id.
 * @param loadFlag Load flags.
 * @return True on success.
 */
bool loadSoundItem(al::IUseSeadAudioPlayer* pUser, u32 id, u32 loadFlag) {
    return pUser->getSeadAudioPlayer()->loadSoundItem(id, loadFlag);
}

/**
 * Checks whether a sound item is loaded.
 * @param pUser Audio player user.
 * @param id Item id.
 * @return True if loaded.
 */
bool isLoadedSoundItem(al::IUseSeadAudioPlayer* pUser, u32 id) {
    return pUser->getSeadAudioPlayer()->isLoadedSoundItem(id, 0xffffffff);
}

/**
 * Saves the sound heap state.
 * @param pUser Audio player user.
 * @return New heap state level.
 */
s32 saveHeapState(al::IUseSeadAudioPlayer* pUser) {
    return pUser->getSeadAudioPlayer()->getSeadAudioSoundHeap()->SaveState();
}

/**
 * Restores a sound heap state.
 * @param pUser Audio player user.
 * @param level Heap state level.
 */
void loadHeapState(al::IUseSeadAudioPlayer* pUser, s32 level) {
    pUser->getSeadAudioPlayer()->getSeadAudioSoundHeap()->LoadState(level);
}

/**
 * Gets the current sound heap state level.
 * @param pUser Audio player user.
 * @return Heap state level.
 */
s32 getCurrentHeapStateLevel(al::IUseSeadAudioPlayer* pUser) {
    return pUser->getSeadAudioPlayer()->getSeadAudioSoundHeap()->GetCurrentLevel();
}

/**
 * Gets the free size of the sound heap.
 * @param pUser Audio player user.
 * @return Free size in bytes.
 */
u64 getSoundResourceHeapFreeSize(al::IUseSeadAudioPlayer* pUser) {
    return pUser->getSeadAudioPlayer()->getSeadAudioSoundHeap()->GetFreeSize();
}

/**
 * Does nothing in this version.
 * @param pFileName File name.
 * @param pPlayer Audio player.
 * @param pSubPlayer Sub audio player.
 * @return Always nullptr.
 */
al::SeadAudioPlayer* tryFindAudioPlayerRegistedSoundMemoryPoolHandler(const char* pFileName,
                                                                      al::SeadAudioPlayer* pPlayer,
                                                                      al::SeadAudioPlayer* pSubPlayer) {
    return nullptr;
}

/**
 * Does nothing in this version.
 * @param pPath File path.
 * @param pPlayer Audio player.
 * @return Always true.
 */
bool tryDisableSoundMemoryPoolHandlerByFilePath(const char* pPath, al::SeadAudioPlayer* pPlayer) {
    return true;
}

/**
 * Does nothing in this version.
 * @param pDirector Audio director.
 * @param pRumbleDirector Pad rumble director.
 */
void setPadRumbleDirectorForSe(al::AudioDirector* pDirector, al::PadRumbleDirector* pRumbleDirector) {}
}  // namespace alAudioSystemFunction

namespace al {
/**
 * Constructs empty audio system information.
 */
AudioSystemInfo::AudioSystemInfo() = default;

/**
 * Gets the audio player used for SE.
 * @return SE audio player.
 */
NOINLINE SeadAudioPlayer* AudioSystemInfo::getSeadAudioPlayerForSe() const {
    return static_cast<SeadAudioPlayer*>(mAudioMgr->getPlayer());
}

/**
 * Gets the audio player used for BGM.
 * @return BGM audio player.
 */
NOINLINE SeadAudioPlayer* AudioSystemInfo::getSeadAudioPlayerForBgm() const {
    return mBgmAudioPlayer;
}

}  // namespace al

namespace al {
/**
 * Changes the audio effect.
 * @param pUser Audio user.
 * @param pName Effect name.
 */
void changeAudioEffect(const IUseAudioKeeper* pUser, const char* pName) {
    SeEffectController* controller = pUser->getAudioKeeper()->getSeEffectController();

    if (controller != nullptr) {
        controller->changeEffect(pName);
    }
}

/**
 * Changes the audio effect to the one of the area the upper layer user is in.
 * @param pUser Audio user.
 */
void changeSequenceAudioEffectWithAreaCheck(const IUseAudioKeeper* pUser) {
    AudioKeeper* keeper = pUser->getAudioKeeper();

    if (keeper == nullptr) {
        return;
    }

    IUseAudioKeeper* upperUser = keeper->getUpperLayerAudioUser();

    if (upperUser == nullptr) {
        return;
    }

    changeAudioEffectWithAreaCheck(upperUser);
}

/**
 * Changes the audio effect to the one of the area the player is in.
 * @param pUser Audio user.
 */
void changeAudioEffectWithAreaCheck(const IUseAudioKeeper* pUser) {
    const char* name = pUser->getAudioKeeper()->getAudioEventController()->getAudioEffectNameByAreaChecker();

    if (name == nullptr) {
        return;
    }

    changeAudioEffect(pUser, name);
}

/**
 * Gets the name of the current audio effect.
 * @param pUser Audio user.
 * @return Effect name, or nullptr.
 */
const char* getCurAudioEffectName(const IUseAudioKeeper* pUser) {
    SeEffectController* controller = pUser->getAudioKeeper()->getSeEffectController();

    if (controller == nullptr) {
        return nullptr;
    }

    return controller->getCurEffectName();
}

/**
 * Activates the audio event controller.
 * @param pUser Audio user.
 */
void activateAudioEventController(const IUseAudioKeeper* pUser) {
    pUser->getAudioKeeper()->getAudioEventController()->activate();
}

/**
 * Deactivates the audio event controller.
 * @param pUser Audio user.
 */
void deactivateAudioEventController(const IUseAudioKeeper* pUser) {
    pUser->getAudioKeeper()->getAudioEventController()->deactivate();
}

/**
 * Activates the SE play event.
 * @param pUser Audio user.
 */
void activateSePlayEvent(const IUseAudioKeeper* pUser) {
    pUser->getAudioKeeper()->getAudioEventController()->activateEachAudioEvent(
        AudioEventController::EVENT_PLAY_SE);
}

/**
 * Deactivates the SE play event.
 * @param pUser Audio user.
 */
void deactivateSePlayEvent(const IUseAudioKeeper* pUser) {
    pUser->getAudioKeeper()->getAudioEventController()->deactivateEachAudioEvent(
        AudioEventController::EVENT_PLAY_SE);
}

/**
 * Activates the audio effect change event.
 * @param pUser Audio user.
 */
void activateAudioEffectChangeEvent(const IUseAudioKeeper* pUser) {
    pUser->getAudioKeeper()->getAudioEventController()->activateEachAudioEvent(
        AudioEventController::EVENT_CHANGE_AUIO_EFFECT);
}

/**
 * Deactivates the audio effect change event.
 * @param pUser Audio user.
 */
void deactivateAudioEffectChangeEvent(const IUseAudioKeeper* pUser) {
    pUser->getAudioKeeper()->getAudioEventController()->deactivateEachAudioEvent(
        AudioEventController::EVENT_CHANGE_AUIO_EFFECT);
}
}  // namespace al
