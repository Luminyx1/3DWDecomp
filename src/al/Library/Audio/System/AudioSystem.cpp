#include "Project/Audio/System/AudioSystem.hpp"

#include <aal/components/aalAudioFrameProcessMgr.h>
#include <audio/seadAudioMgr.h>
#include <audio/seadAudioSettingParameter.h>
#include <audio/seadAudioSystemNin.h>
#include <heap/seadExpHeap.h>
#include <heap/seadHeapMgr.h>
#include <nn/atk/atk_HardwareManager.h>
#include <nn/atk/atk_Sound3DManager.h>
#include <nn/audio.h>

#include "Library/Audio/AudioMic.hpp"
#include "Library/Audio/System/AudioKeeperFunction.hpp"
#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/Audio/System/NWSound3DEngineCustomCAFE.hpp"
#include "Library/Bgm/BgmDataBase.hpp"
#include "Library/Memory/HeapUtil.hpp"
#include "Library/Se/DataBase/SeDataBase.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Project/Audio/System/AudioEffectDataBase.hpp"
#include "Project/Audio/System/AudioResourceDirector.hpp"
#include "Project/Audio/System/AudioSystemDebug.hpp"
#include "Project/Audio/System/SeadAudio3DMgr.hpp"
#include "Project/Audio/System/SeadAudioResourceLoader.hpp"
#include "Project/Audio/System/SoundHeapPtrWrapper.hpp"
#include "Project/Audio/System/SoundSubArchiveKeeper.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
al::NWSound3DEngineCustomCAFE sSound3DEngine;
}  // namespace

namespace al {

/**
 * Sets the master volume of the sound hardware.
 * @param volume Master volume.
 */
static void setMasterVolume(f32 volume) {
    nn::atk::detail::Util::Singleton<nn::atk::detail::driver::HardwareManager>::GetInstance()
        .SetMasterVolume(volume, 0);
}

/**
 * Constructs the audio system, creating its system info, debug module and frame process manager.
 */
AudioSystem::AudioSystem() {
    mAudioSystemInfo = new AudioSystemInfo();
    mAudioSystemDebug = new AudioSystemDebug();
    mAudioFrameProcessMgr = new aal::AudioFrameProcessMgr();
}

/**
 * Initializes the audio system with default device volumes.
 * @param rArchiveName Path of the sound archive.
 * @param masterVolume Master volume.
 * @param isUseMic Whether to create the microphone.
 */
void AudioSystem::init(const sead::SafeString& rArchiveName, f32 masterVolume, bool isUseMic) {
    AudioSystemInitInfo info;
    info.archiveName = rArchiveName.cstr();
    info.masterVolume = masterVolume;
    info.isUseMic = isUseMic;
    init(info);
}

/**
 * Initializes the audio heaps, the sead audio manager and all audio databases and resources.
 * @param rInfo Initialization parameters.
 */
void AudioSystem::init(const AudioSystemInitInfo& rInfo) {
    sead::Heap* audioHeap = tryFindNamedHeap("AudioHeap");

    if (audioHeap == nullptr) {
        audioHeap = sead::ExpHeap::create(0x1800000, "AudioHeap", nullptr, 8,
                                          sead::Heap::cHeapDirection_Forward, false);
        addNamedHeap(audioHeap, nullptr);
    }

    sead::Heap* audioSubHeap = tryFindNamedHeap("AudioSubHeap");

    if (audioSubHeap == nullptr) {
        audioSubHeap = sead::ExpHeap::create(0x400000, "AudioSubHeap", audioHeap, 8,
                                             sead::Heap::cHeapDirection_Forward, false);
        addNamedHeap(audioSubHeap, nullptr);
    }

    sead::Heap* audioSystemHeap = sead::ExpHeap::create(0x600000, "AudioSystemHeap", audioHeap, 8,
                                                        sead::Heap::cHeapDirection_Forward, false);
    addNamedHeap(audioSystemHeap, nullptr);
    sead::ScopedCurrentHeapSetter heapSetter(audioSystemHeap);

    sead::AudioSettingParameter* param = new sead::AudioSettingParameter();
    SeadAudio3DMgr* audio3DMgr = initSeadAudio3DMgr(param);
    sead::SafeString archivePath = rInfo.archiveName;
    SeadAudioResourceLoader* resourceLoader = new SeadAudioResourceLoader();
    resourceLoader->setArchivePath(archivePath);
    sead::AudioSystemNin* audioSystem = new sead::AudioSystemNin();
    param->setResourceLoader(resourceLoader);
    SeadAudioPlayer* audioPlayer = new SeadAudioPlayer();
    param->setPlayer(audioPlayer);
    param->setAudioSystem(audioSystem);

    if (rInfo.isUseMic) {
        mAudioMic = new AudioMic(*param, 0x2000);
    }

    {
        sead::ScopedCurrentHeapSetter audioHeapSetter(audioHeap);
        mAudioMgr = sead::AudioMgr::createInstance(audioHeap);
        mAudioMgr->prepare(param, audioHeap, 0);
    }

    {
        sead::ScopedCurrentHeapSetter audioSubHeapSetter(audioSubHeap);
        mSubArchiveKeeper = new SoundSubArchiveKeeper(0x3c00);
    }

    alSoundNameFunction::initializeNameUtil(getSeadAudioPlayer(), false);
    alSoundNameFunction::initializeNameUtil(getSubArchiveSeadAudioPlayer(), true);

    mSoundHeapPtrWrapper = new SeadAudioSoundHeapPtrWrapper();
    mSoundHeapPtrWrapper->setSoundHeap(getSeadAudioPlayer()->getSoundHeap());
    initDataBase();
    initResourceDirector();
    initSystemInfo(audio3DMgr);

    alAudioSystemFunction::loadResourceFromUserManagementFile(
        UMF_SE_STATIONED_SYSTEM, mAudioSystemInfo->getSeadAudioPlayerForSe(), true);
    alAudioSystemFunction::loadResourceFromUserManagementFile(
        UMF_BGM_STATIONED_1ST, mAudioSystemInfo->getSeadAudioPlayerForBgm(), true);

    nn::audio::AcquireAudioDeviceNotificationForOutput(&mDeviceNotificationEvent);
    mMasterVolume = rInfo.masterVolume;
    mTvOutputVolume = rInfo.tvOutputVolume;
    mOtherOutputVolume = rInfo.otherOutputVolume;
    mIsStereo = nn::audio::GetActiveAudioDeviceChannelCountForOutput() < 3;
    updateSoundChannelSetting();
    applyDeviceVolume();
}

/**
 * Creates the sead 3D audio manager using the custom 3D sound engine.
 * @param pParam Audio setting parameter the manager is appended to.
 * @return The created 3D audio manager.
 */
SeadAudio3DMgr* AudioSystem::initSeadAudio3DMgr(sead::AudioSettingParameter* pParam) {
    SeadAudio3DMgr* audio3DMgr = new SeadAudio3DMgr();
    audio3DMgr->setSonicVelocity(1700.0f / 3.0f);
    audio3DMgr->getSound3DManager()->SetEngine(&sSound3DEngine);
    audio3DMgr->getDefaultListener()->getNwListener().SetOutputTypeFlag(
        nn::atk::Sound3DListener::ListenerOutputType_Tv);
    pParam->appendSubset(audio3DMgr);
    return audio3DMgr;
}

/**
 * Initializes the debug module. Does nothing in release builds.
 * @param pParam Audio setting parameter.
 */
void AudioSystem::initDebugModule(sead::AudioSettingParameter* pParam) {}

/**
 * Gets the audio player of the sub archive, falling back to the main audio player.
 * @return The sub archive audio player if it exists, otherwise the main audio player.
 */
SeadAudioPlayer* AudioSystem::getSubArchiveSeadAudioPlayer() const {
    SeadAudioPlayer* player = mSubArchiveKeeper->getSeadAudioPlayer();

    if (player != nullptr) {
        return player;
    }

    return getSeadAudioPlayer();
}

/**
 * Initializes the sound spy. Does nothing in release builds.
 */
void AudioSystem::initSpy() {}

/**
 * Creates the audio effect, sound effect and BGM databases.
 */
void AudioSystem::initDataBase() {
    mAudioEffectDataBase = new AudioEffectDataBase(AudioEffectDataBase::ARC_NAME, nullptr);
    mSeDataBase = new SeDataBase(SeDataBase::ARC_NAME, getSeadAudioPlayer(), nullptr);
    mBgmDataBase = new BgmDataBase();
}

/**
 * Creates the resource directors of the main archive and the sub archive.
 */
void AudioSystem::initResourceDirector() {
    mResourceDirector = new AudioResourceDirector(5, 0x400, this, this, getSeadAudioPlayer());
    mSubArchiveResourceDirector =
        new AudioResourceDirector(5, 0x400, mSubArchiveKeeper, mSubArchiveKeeper,
                                  getSubArchiveSeadAudioPlayer());
}

/**
 * Fills the audio system info with the created audio objects.
 * @param pAudio3DMgr 3D audio manager.
 */
void AudioSystem::initSystemInfo(SeadAudio3DMgr* pAudio3DMgr) {
    mAudioSystemInfo->mAudioMgr = mAudioMgr;
    mAudioSystemInfo->mAudioEffectDataBase = mAudioEffectDataBase;
    mAudioSystemInfo->mSeDataBase = mSeDataBase;
    mAudioSystemInfo->mBgmDataBase = mBgmDataBase;
    mAudioSystemInfo->mAudio3DMgr = pAudio3DMgr;
    mAudioSystemInfo->_28 = mResourceDirector;
    mAudioSystemInfo->_30 = mSubArchiveResourceDirector;
    mAudioSystemInfo->mBgmAudioPlayer = getSubArchiveSeadAudioPlayer();
    mAudioSystemInfo->_38 = mAudioMic;
}

/**
 * Applies the stereo or surround output mode depending on the output device.
 */
void AudioSystem::updateSoundChannelSetting() {
    nn::atk::OutputMode outputMode =
        mIsStereo ? nn::atk::OutputMode_Stereo : nn::atk::OutputMode_Surround;
    nn::atk::detail::Util::Singleton<nn::atk::detail::driver::HardwareManager>::GetInstance()
        .SetOutputMode(outputMode, nn::atk::OutputDevice_Main);
}

/**
 * Applies the volume of every audio output device and the master volume.
 */
void AudioSystem::applyDeviceVolume() {
    nn::audio::AudioDeviceName deviceNames[4];
    nn::audio::ListAudioDeviceName(deviceNames, 4);

    for (s32 i = 0; i < 4; i++) {
        if (isEqualString(deviceNames[i].raw_name, "AudioTvOutput")) {
            nn::audio::SetAudioDeviceOutputVolume(&deviceNames[i], mTvOutputVolume);
        } else {
            nn::audio::SetAudioDeviceOutputVolume(&deviceNames[i], mOtherOutputVolume);
        }
    }

    setMasterVolume(mMasterVolume);
}

/**
 * Updates the audio manager and the sub archive, and reacts to output device changes.
 */
void AudioSystem::update() {
    mAudioMgr->calc();

    if (mSubArchiveKeeper != nullptr) {
        mSubArchiveKeeper->update();
    }

    if (mDeviceNotificationEvent.TryWait()) {
        mIsStereo = nn::audio::GetActiveAudioDeviceChannelCountForOutput() < 3;
        updateSoundChannelSetting();
    }
}

/**
 * Pauses the audio system immediately. Does nothing on this platform.
 * @param isPause Whether to pause or resume.
 * @param pName Name of the pause requester.
 * @param isForce Whether to force the pause.
 */
void AudioSystem::pauseSystemImmediately(bool isPause, const char* pName, bool isForce) {}

/**
 * Loads a sound item.
 * @param id Sound item id.
 * @param loadFlag Load flags.
 * @return True if the item was loaded.
 */
bool AudioSystem::loadSoundItem(u32 id, u32 loadFlag) {
    return alAudioSystemFunction::loadSoundItem(this, id, loadFlag);
}

/**
 * Checks whether a sound item is loaded.
 * @param id Sound item id.
 * @return True if the item is loaded.
 */
bool AudioSystem::isLoadedSoundItem(u32 id) {
    return alAudioSystemFunction::isLoadedSoundItem(this, id);
}

/**
 * Saves the state of the sound heap.
 * @return The heap state level after saving.
 */
s32 AudioSystem::saveHeapState() {
    return alAudioSystemFunction::saveHeapState(this);
}

/**
 * Restores the sound heap to a saved state.
 * @param level Heap state level to restore.
 */
void AudioSystem::loadHeapState(s32 level) {
    alAudioSystemFunction::loadHeapState(this, level);
}

/**
 * Gets the current state level of the sound heap.
 * @return The current heap state level.
 */
s32 AudioSystem::getCurrentHeapStateLevel() {
    return alAudioSystemFunction::getCurrentHeapStateLevel(this);
}

/**
 * Gets the free size of the sound resource heap.
 * @return The free size in bytes.
 */
u64 AudioSystem::getSoundResourceHeapFreeSize() {
    return alAudioSystemFunction::getSoundResourceHeapFreeSize(this);
}

/**
 * Gets the main audio player.
 * @return The audio player of the audio manager.
 */
SeadAudioPlayer* AudioSystem::getSeadAudioPlayer() const {
    return static_cast<SeadAudioPlayer*>(mAudioMgr->getPlayer());
}

/**
 * Registers an audio frame process.
 * @param pProcess Process to register.
 */
void AudioSystem::addAudiioFrameProccess(aal::IAudioFrameProcess* pProcess) {
    mAudioFrameProcessMgr->addProcess(pProcess);
}

/**
 * Unregisters an audio frame process.
 * @param pProcess Process to unregister.
 */
void AudioSystem::removeAudiioFrameProccess(aal::IAudioFrameProcess* pProcess) {
    mAudioFrameProcessMgr->removeProcess(pProcess);
}

/**
 * Registers an audio frame process again, moving it to the end of the process list.
 * @param pProcess Process to register again.
 */
void AudioSystem::reRegisterCallback(aal::IAudioFrameProcess* pProcess) {
    mAudioFrameProcessMgr->removeProcess(pProcess);
    mAudioFrameProcessMgr->addProcess(pProcess);
}

/**
 * Destroys the audio system, finalizing the audio device notification event.
 */
AudioSystem::~AudioSystem() = default;

}  // namespace al
