#include "System/GameSystem.hpp"
#include <eui/euiConstantBuffer.h>
#include <eui/euiScreenMgr.h>
#include <heap/seadHeapMgr.h>
#include "Library/Controller/GamePadSystem.hpp"
#include "Library/Effect/EffectSystem.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Layout/LayoutSystem.hpp"
#include "Library/Memory/HeapUtil.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/Message/MessageSystem.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nfp/NfpDirector.hpp"
#include "Library/Sequence/Sequence.hpp"
#include "Library/System/GameSystemInfo.hpp"
#include "Project/Audio/System/AudioSystem.hpp"
#include "Project/Controller/WaveVibrationHolder.hpp"
#include "Raidon/PlayReport.hpp"
#include "System/Application.hpp"
#include "System/AssetLoadingThread.hpp"
#include "System/RootTask.hpp"
#include "System/SequenceFactory.hpp"

namespace {
    NERVE_DECL(GameSystem, Play);
    NERVES_MAKE_NOSTRUCT(GameSystem, Play)
};  // namespace

/**
 * @brief Access the application's game framework.
 * @return The NX game framework owned by the application.
 */
static al::GameFrameworkNx* getGameFramework() {
    return static_cast<al::GameFrameworkNx*>(Application::instance()->getFramework());
}

/**
 * @brief Create the game system before subsystem initialization.
 */
GameSystem::GameSystem()
    : al::NerveExecutor("ゲームシステム"), mpSequence(nullptr), mpInfo(nullptr),
      mpAudio(nullptr), mpUnknown30(nullptr), mpUnknown38(nullptr), mpUnknown40(nullptr) {}

/**
 * @brief Create every engine subsystem and start the first game sequence.
 */
void GameSystem::init() {
    mpInfo = new al::GameSystemInfo();
    initNerve(&NrvGameSystemPlay, 0);

    al::NfpDirector* pNfpDirector = new al::NfpDirector(al::NfpDirector::cDeviceNumMax);
    mpInfo->setNfpDirector(pNfpDirector);
    pNfpDirector->initialize();
    mpInfo->setApplicationMessageReceiver(getGameFramework()->mMessageReceiver);

    mpGamePad = new al::GamePadSystem(false);
    mpInfo->setDrawSystemInfo(getGameFramework()->mDrawSystemInfo);
    mpInfo->setGamePadSystem(mpGamePad);
    initAudio();
    rc::AssetLoadingThread::sInstance->setAudioSystemInfo(mpAudio->getAudioSystemInfo());

    agl::DrawContext* pDrawContext = getGameFramework()->mDrawContext;
    sead::Heap* pHeap = al::getCurrentHeap();
    al::EffectSystem* pEffectSystem =
        al::EffectSystem::initializeSystemWithPatchResource(pDrawContext, pHeap, false);
    mpInfo->setEffectSystem(pEffectSystem);

    al::LayoutSystem* pLayoutSystem = new al::LayoutSystem();
    pLayoutSystem->init(false);
    mpInfo->setLayoutSystem(pLayoutSystem);

    al::MessageSystem* pMessageSystem = new al::MessageSystem();
    mpInfo->setMessageSystem(pMessageSystem);

    al::WaveVibrationHolder* pWaveVibrationHolder = new al::WaveVibrationHolder(mpGamePad);
    mpInfo->setWaveVibrationHolder(pWaveVibrationHolder);
    mpAudio->addAudiioFrameProccess(pWaveVibrationHolder);
    PlayReport::getInstance()->Init();
    al::createSequenceHeap();
    setStartupSequence();
}

/**
 * @brief Create the audio system and share it with the other subsystems.
 */
void GameSystem::initAudio() {
    al::AudioSystemInitInfo info;
    info.isUseMic = true;
    info.tvOutputVolume = 0.5f;
    info.otherOutputVolume = 1.0f;
    info._14 = 1.0f;
    info.archiveName = "SoundData/SeData.bfsar";
    info.masterVolume = 0.75f;

    mpAudio = new al::AudioSystem();
    mpAudio->init(info);
    mpInfo->setAudioSystem(mpAudio);
    mpGamePad->setAudioSystem(mpAudio);
}

/**
 * @brief Switch to the sequence the game boots into.
 */
void GameSystem::setStartupSequence() {
    tryChangeSequence("ProductSequence");
}

/**
 * @brief Update input, the nerve state and audio once per frame.
 */
void GameSystem::movement() {
    mpGamePad->update();
    updateNerve();
    mpAudio->update();
}

/**
 * @brief Draw the active sequence to the main view.
 */
void GameSystem::drawMain() {
    if (mpSequence == nullptr) {
        getGameFramework()->clearFrameBuffer();
    }

    al::LayoutSystem* pLayoutSystem = mpInfo->getLayoutSystem();
    if (pLayoutSystem != nullptr) {
        pLayoutSystem->getScreenMgr()->getConstantBuffer()->map();
    }

    if (mpSequence != nullptr) {
        mpSequence->drawMain();
    }

    pLayoutSystem = mpInfo->getLayoutSystem();
    if (pLayoutSystem != nullptr) {
        pLayoutSystem->getScreenMgr()->getConstantBuffer()->flipBufferIndex();
    }
}

/**
 * @brief Handle the unused secondary view.
 */
void GameSystem::drawSub() {}

/**
 * @brief Update the active game sequence when one exists.
 */
void GameSystem::exePlay() {
    if (mpSequence != nullptr) {
        mpSequence->update();
    }
}

/**
 * @brief Replace the active sequence with a newly created one.
 * @param pName The class name of the sequence to create.
 * @return false if the sequence could not be created, true otherwise (including when the
 *         current sequence cannot be disposed yet).
 */
bool GameSystem::tryChangeSequence(const char* pName) {
    if (mpSequence != nullptr) {
        if (!mpSequence->isDisposable()) {
            return true;
        }

        delete mpSequence;
        mpSequence = nullptr;
        al::freeAllSequenceHeap();
    }

    sead::ScopedCurrentHeapSetter setter(al::getSequenceHeap());
    al::Sequence* pSequence = SequenceFactory::createSequence(pName);
    if (pSequence == nullptr) {
        return false;
    }

    al::SequenceInitInfo info(mpInfo);
    pSequence->init(info);
    mpSequence = pSequence;
    return true;
}

/**
 * @brief Access the application's game system.
 * @return The game system owned by the initialized root task.
 */
GameSystem* GameSystemFunction::getGameSystem() {
    return Application::instance()->getRootTask()->getGameSystem();
}
