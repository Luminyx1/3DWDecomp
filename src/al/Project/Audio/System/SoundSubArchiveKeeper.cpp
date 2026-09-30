#include "Project/Audio/System/SoundSubArchiveKeeper.hpp"

#include <audio/seadAudioSoundDataMgrNin.h>

#include "Library/Audio/System/AudioKeeperFunction.hpp"
#include "Library/Memory/Util.hpp"
#include "Project/Audio/Sound/SoundHandle.hpp"

namespace al {
/**
 * Mounts the BGM sub archive and creates its player and sound heap.
 * @param heapSize Size of the sound heap.
 */
SoundSubArchiveKeeper::SoundSubArchiveKeeper(s32 heapSize) {
    mAudioPlayer = new SeadAudioPlayer();
    mAudioPlayer->getSoundDataMgr()->setContentRootPath("content:");
    mAudioPlayer->getSoundDataMgr()->mountSoundArchiveFromFs("SoundData/BgmData.bfsar", getCurrentHeap(), false,
                                                             true);
    mAudioPlayer->setupDataManagement(0, 0, 0, getCurrentHeap(), 0);
    mAudioPlayer->createSoundHeap(heapSize, getCurrentHeap());
    mSoundHandle = new AcLSoundHandle();
}

/**
 * Updates the audio player.
 */
void SoundSubArchiveKeeper::update() {
    mAudioPlayer->calc();
}

/**
 * Loads a sound item.
 * @param id Item id.
 * @param loadFlag Load flags.
 * @return True on success.
 */
bool SoundSubArchiveKeeper::loadSoundItem(u32 id, u32 loadFlag) {
    return alAudioSystemFunction::loadSoundItem(this, id, loadFlag);
}

/**
 * Checks whether a sound item is loaded.
 * @param id Item id.
 * @return True if loaded.
 */
bool SoundSubArchiveKeeper::isLoadedSoundItem(u32 id) {
    return alAudioSystemFunction::isLoadedSoundItem(this, id);
}

/**
 * Saves the sound heap state.
 * @return New heap state level.
 */
s32 SoundSubArchiveKeeper::saveHeapState() {
    return alAudioSystemFunction::saveHeapState(this);
}

/**
 * Restores a sound heap state.
 * @param level Heap state level.
 */
void SoundSubArchiveKeeper::loadHeapState(s32 level) {
    alAudioSystemFunction::loadHeapState(this, level);
}

/**
 * Gets the current sound heap state level.
 * @return Heap state level.
 */
s32 SoundSubArchiveKeeper::getCurrentHeapStateLevel() {
    return alAudioSystemFunction::getCurrentHeapStateLevel(this);
}

/**
 * Gets the free size of the sound heap.
 * @return Free size in bytes.
 */
u64 SoundSubArchiveKeeper::getSoundResourceHeapFreeSize() {
    return alAudioSystemFunction::getSoundResourceHeapFreeSize(this);
}

/**
 * Gets the audio player of the sub archive.
 * @return Audio player.
 */
SeadAudioPlayer* SoundSubArchiveKeeper::getSeadAudioPlayer() const {
    return mAudioPlayer;
}
}  // namespace al
