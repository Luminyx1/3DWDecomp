#include "Project/Audio/System/AudioPlayer.hpp"

#include <audio/seadAudioSoundDataMgrNin.h>
#include <nn/atk/atk_BankFileReader.h>

#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs the player.
 */
SeadAudioPlayer::SeadAudioPlayer() = default;

/**
 * Loads a sound item by name.
 * @param rName Item name.
 * @param loadFlag Load flags.
 * @return True on success.
 */
bool SeadAudioPlayer::loadSoundItem(const sead::SafeString& rName, u32 loadFlag) {
    return getSoundDataMgr()->loadData(rName.cstr(), loadFlag, 0, nullptr);
}

/**
 * Loads a sound item by id.
 * @param id Item id.
 * @param loadFlag Load flags.
 * @return True on success.
 */
bool SeadAudioPlayer::loadSoundItem(u32 id, u32 loadFlag) {
    return getSoundDataMgr()->loadData(id, loadFlag, 0, nullptr);
}

/**
 * Checks whether a sound item is loaded.
 * @param id Item id.
 * @param loadFlag Load flags.
 * @return True if loaded.
 */
bool SeadAudioPlayer::isLoadedSoundItem(u32 id, u32 loadFlag) const {
    return getSoundDataMgr()->IsDataLoaded(id, loadFlag);
}

/**
 * Gets the id of a sound from its name.
 * @param pName Sound name.
 * @return Sound id.
 */
u32 SeadAudioPlayer::getSoundId(const char* pName) const {
    return getSoundDataMgr()->getSoundArchive()->GetItemId(pName);
}

/**
 * Gets the mounted sound archive.
 * @return Sound archive.
 */
const nn::atk::SoundArchive* SeadAudioPlayer::getSoundArchive() const {
    return getSoundDataMgr()->getSoundArchive();
}

/**
 * Gets the name of a sound.
 * @param id Sound id.
 * @return Sound name.
 */
const char* SeadAudioPlayer::getSoundName(u32 id) const {
    return sead::AudioPlayerNin::getSoundName(id);
}

/**
 * Gets the type of a sound.
 * @param id Sound id.
 * @return Sound type.
 */
u32 SeadAudioPlayer::getSoundType(u32 id) const {
    return getSoundDataMgr()->getSoundArchive()->GetSoundType(id);
}

/**
 * Gets the number of sounds.
 * @return Sound count.
 */
u32 SeadAudioPlayer::getSoundCount() const {
    return sead::AudioPlayerNin::getSoundCount();
}

/**
 * Gets the id of the sound at an index.
 * @param index Sound index.
 * @return Sound id.
 */
u32 SeadAudioPlayer::getSoundIdFromIndex(s32 index) const {
    return getSoundDataMgr()->getSoundArchive()->GetSoundIdFromIndex(index);
}

/**
 * Reads the sequence information of a sound.
 * @param pInfo Receives the information.
 * @param id Sound id.
 * @return True on success.
 */
bool SeadAudioPlayer::readSequenceSoundInfo(SequenceSoundInfo* pInfo, u32 id) const {
    return getSoundDataMgr()->getSoundArchive()->ReadSequenceSoundInfo(pInfo, id);
}

/**
 * Reads the information of a sound.
 * @param pInfo Receives the information.
 * @param id Sound id.
 * @return True on success.
 */
bool SeadAudioPlayer::readSoundInfo(SoundInfo* pInfo, u32 id) const {
    return getSoundDataMgr()->getSoundArchive()->ReadSoundInfo(pInfo, id);
}

/**
 * Gets the file size of a bank.
 * @param id Bank id.
 * @return File size in bytes.
 */
u32 SeadAudioPlayer::getBankInfoSize(u32 id) const {
    return getFileSize(id);
}

/**
 * Gets the total size of the wave archives used by a loaded bank.
 * @param id Bank id.
 * @return Size in bytes.
 */
u32 SeadAudioPlayer::getBankWaveArcSize(u32 id) const {
    nn::atk::SoundArchive::FileId fileId = getSoundArchive()->GetItemFileId(id);
    const void* file = getSoundDataMgr()->detail_GetFileAddress(fileId);
    if (file == nullptr) {
        return 0;
    }

    nn::atk::detail::BankFileReader reader(file);
    const nn::atk::detail::WaveIdTable* table = reader.GetWaveIdTable();
    u32 size = 0;
    u32 prevId = AudioConst::SOUND_ITEM_ID_INVALID;
    for (u32 i = 0; i < table->count; i++) {
        if (prevId != table->items[i].waveArchiveId) {
            size += getWaveArcSize(table->items[i].waveArchiveId);
            prevId = table->items[i].waveArchiveId;
        }
    }

    return size;
}

/**
 * Gets the file size of a wave archive.
 * @param id Wave archive id.
 * @return File size in bytes.
 */
u32 SeadAudioPlayer::getWaveArcSize(u32 id) const {
    return getFileSize(id);
}

/**
 * Gets the memory size needed to load a sound item.
 * @param id Sound id.
 * @return Size in bytes.
 */
u32 SeadAudioPlayer::getSoundItemSize(u32 id) const {
    u32 type = getSoundType(id);
    if (type == nn::atk::SoundArchive::SoundType_Wave) {
        return getBankTotalSize(id);
    }

    if (type == nn::atk::SoundArchive::SoundType_Sequence) {
        return getSequenceSoundSize(id);
    }

    return 0;
}

/**
 * Gets the size of a bank including its wave archives.
 * @param id Bank id.
 * @return Size in bytes.
 */
u32 SeadAudioPlayer::getBankTotalSize(u32 id) const {
    return getBankInfoSize(id) + getBankWaveArcSize(id);
}

/**
 * Gets the size of a sequence sound including its banks.
 * @param id Sound id.
 * @return Size in bytes.
 */
u32 SeadAudioPlayer::getSequenceSoundSize(u32 id) const {
    const nn::atk::SoundArchive* archive = getSoundArchive();
    nn::atk::SoundArchive::FileId fileId = archive->GetItemFileId(id);
    nn::atk::SoundArchive::FileInfo fileInfo;
    archive->detail_ReadFileInfo(fileId, &fileInfo);
    u32 size = fileInfo.fileSize;
    nn::atk::SoundArchive::SequenceSoundInfo info;
    archive->ReadSequenceSoundInfo(&info, id);
    for (s32 i = 0; i < 4; i++) {
        u32 bankId = info.bankIds[i];
        if (bankId != AudioConst::BANK_ID_INVALID) {
            size += getBankTotalSize(bankId);
        }
    }

    return size;
}

/**
 * Gets the number of sound players.
 * @return Sound player count.
 */
u32 SeadAudioPlayer::getSoundPlayerCount() const {
    return GetSoundPlayerCount();
}

/**
 * Gets the id of the sound player at an index.
 * @param index Player index.
 * @return Player id.
 */
u32 SeadAudioPlayer::getSoundPlayerIdFromIndex(s32 index) {
    return index | 0x4000000;
}

/**
 * Gets the sound heap.
 * @return Sound heap.
 */
sead::AudioSoundHeapNin* SeadAudioPlayer::getSeadAudioSoundHeap() {
    return getSoundHeap();
}

/**
 * Finds the memory pool handler of a sound data file.
 * @param pName File name inside SoundData.
 * @return Handler, or nullptr if none.
 */
sead::SoundMemoryPoolHandler* SeadAudioPlayer::tryGetSoundMemoryPoolHandler(const char* pName) {
    StringTmp<128> path("SoundData/%s", pName);
    return tryGetSoundMemoryPoolHandlerByFilePath(path.cstr());
}

/**
 * Finds the memory pool handler of a file path.
 * @param pPath File path.
 * @return Handler, or nullptr if none.
 */
sead::SoundMemoryPoolHandler* SeadAudioPlayer::tryGetSoundMemoryPoolHandlerByFilePath(const char* pPath) {
    for (s32 i = 0; i < getMemoryPoolHandlers()->size(); i++) {
        sead::SoundMemoryPoolHandler* handler = getMemoryPoolHandlers()->unsafeAt(i);
        StringTmp<128> handlerPath("SoundData/%s", handler->getName());
        if (isEqualString(handlerPath.cstr(), pPath)) {
            return handler;
        }
    }

    return nullptr;
}
}  // namespace al
