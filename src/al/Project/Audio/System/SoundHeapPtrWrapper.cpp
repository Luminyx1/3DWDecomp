#include "Project/Audio/System/SoundHeapPtrWrapper.hpp"

#include <audio/seadAudioSoundHeapNin.h>

#include "Project/Audio/System/AudioPlayer.hpp"

namespace al {
/**
 * Constructs a wrapper with no heap.
 */
SeadAudioSoundHeapPtrWrapper::SeadAudioSoundHeapPtrWrapper() : mHeap(nullptr) {}

/**
 * Sets the wrapped sound heap.
 * @param pHeap Sound heap.
 */
void SeadAudioSoundHeapPtrWrapper::setSoundHeap(sead::AudioSoundHeapNin* pHeap) {
    mHeap = pHeap;
}

/**
 * Saves the heap state.
 * @return New heap state level.
 */
s32 SeadAudioSoundHeapPtrWrapper::saveHeapState() {
    return mHeap->SaveState();
}

/**
 * Restores a heap state.
 * @param level Heap state level.
 */
void SeadAudioSoundHeapPtrWrapper::loadHeapState(s32 level) {
    mHeap->LoadState(level);
}

/**
 * Allocates memory from the heap.
 * @param size Size in bytes.
 * @return Allocated memory.
 */
void* SeadAudioSoundHeapPtrWrapper::alloc(size_t size) {
    return mHeap->Allocate(size);
}

/**
 * Gets the current heap state level.
 * @return Heap state level.
 */
s32 SeadAudioSoundHeapPtrWrapper::getCurrentHeapStateLevel() const {
    return mHeap->GetCurrentLevel();
}

/**
 * Gets the size of the heap.
 * @return Size in bytes.
 */
size_t SeadAudioSoundHeapPtrWrapper::getHeapSize() const {
    return mHeap->GetSize();
}

/**
 * Gets the free size of the heap.
 * @return Free size in bytes.
 */
size_t SeadAudioSoundHeapPtrWrapper::getHeapFreeSize() const {
    return mHeap->GetFreeSize();
}

/**
 * Dumps the heap contents.
 * @param pMgr Sound data manager.
 * @param pArchive Sound archive.
 */
void SeadAudioSoundHeapPtrWrapper::dumpHeap(nn::atk::SoundDataManager* pMgr, nn::atk::SoundArchive* pArchive) {
    mHeap->Dump(*pMgr, *pArchive);
}

/**
 * Destroys the name lookup helper.
 */
SoundNameUtil::~SoundNameUtil() {
    mAccessor;
}
}  // namespace al

namespace {
al::SoundNameUtil sSeNameUtil;
al::SoundNameUtil sBgmNameUtil;

al::SoundNameUtil& getNameUtil(bool isBgm) {
    return isBgm ? sBgmNameUtil : sSeNameUtil;
}
}  // namespace

namespace alSoundNameFunction {
/**
 * Sets the resource accessor used for sound name lookups.
 * @param pAccessor Resource accessor.
 * @param isBgm Whether the accessor is for BGM.
 */
void initializeNameUtil(al::IAudioResourceInfoAccessor* pAccessor, bool isBgm) {
    if (isBgm) {
        sBgmNameUtil.setAccessor(pAccessor);
    } else {
        sSeNameUtil.setAccessor(pAccessor);
    }
}
}  // namespace alSoundNameFunction

namespace alSoundNameUtil {
/**
 * Checks whether a sound item id exists.
 * @param id Item id.
 * @param isBgm Whether to look up BGM.
 * @return True if the item exists.
 */
bool isExistItemId(u32 id, bool isBgm) {
    return getNameUtil(isBgm).getAccessor()->getSoundName(id) != nullptr;
}

/**
 * Checks whether a sound item name exists.
 * @param pName Item name.
 * @param isBgm Whether to look up BGM.
 * @return True if the item exists.
 */
bool isExistItemName(const char* pName, bool isBgm) {
    return getSoundId(pName, isBgm) != al::AudioConst::SOUND_ID_INVALID;
}

/**
 * Gets the type of a sound.
 * @param id Sound id.
 * @param isBgm Whether to look up BGM.
 * @return Sound type.
 */
u32 getSoundType(u32 id, bool isBgm) {
    return getNameUtil(isBgm).getAccessor()->getSoundType(id);
}

/**
 * Gets the item type of an item id.
 * @param id Item id.
 * @return Item type.
 */
u32 getItemType(u32 id) {
    return id >> 24;
}

/**
 * Gets the name of a sound.
 * @param id Sound id.
 * @param isBgm Whether to look up BGM.
 * @return Sound name.
 */
const char* getSoundName(u32 id, bool isBgm) {
    return getNameUtil(isBgm).getAccessor()->getSoundName(id);
}

/**
 * Gets the id of a sound.
 * @param pName Sound name.
 * @param isBgm Whether to look up BGM.
 * @return Sound id, or SOUND_ID_INVALID.
 */
u32 getSoundId(const char* pName, bool isBgm) {
    if (pName == nullptr) {
        return al::AudioConst::SOUND_ID_INVALID;
    }

    return getNameUtil(isBgm).getAccessor()->getSoundId(pName);
}

/**
 * Tries to get the id of a sound.
 * @param pId Receives the sound id.
 * @param pName Sound name.
 * @param isBgm Whether to look up BGM.
 * @return True if the sound exists.
 */
bool tryGetSoundId(u32* pId, const char* pName, bool isBgm) {
    if (pName == nullptr) {
        return false;
    }

    u32 id = getNameUtil(isBgm).getAccessor()->getSoundId(pName);

    if (id == al::AudioConst::SOUND_ID_INVALID) {
        return false;
    }

    *pId = id;
    return true;
}
}  // namespace alSoundNameUtil
