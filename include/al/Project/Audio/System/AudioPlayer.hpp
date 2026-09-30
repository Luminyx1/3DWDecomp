#pragma once

#include <audio/seadAudioPlayerNin.h>
#include <nn/atk/atk_SoundArchive.h>
#include <prim/seadSafeString.h>

namespace sead {
class AudioSoundHeapNin;
}

namespace al {
namespace AudioConst {
extern const u32 BANK_ID_INVALID;
extern const u32 SOUND_ITEM_ID_INVALID;
}  // namespace AudioConst

class SeadAudioPlayer;

struct SequenceSoundInfo : public nn::atk::SoundArchive::SequenceSoundInfo {};

struct SoundInfo : public nn::atk::SoundArchive::SoundInfo {};

class IAudioResourceInfoAccessor {
public:
    virtual u32 getSoundId(const char* pName) const = 0;
    virtual const char* getSoundName(u32 id) const = 0;
    virtual u32 getSoundType(u32 id) const = 0;
    virtual u32 getSoundCount() const = 0;
};

class IUseSeadAudioPlayer {
public:
    virtual SeadAudioPlayer* getSeadAudioPlayer() const = 0;
};

class SeadAudioPlayer : public sead::AudioPlayerNin, public IAudioResourceInfoAccessor {
    SEAD_RTTI_OVERRIDE(SeadAudioPlayer, sead::AudioPlayerNin)

public:
    SeadAudioPlayer();

    u32 getSoundCount() const override;
    const char* getSoundName(u32 id) const override;
    u32 getSoundId(const char* pName) const override;
    virtual bool loadSoundItem(const sead::SafeString& rName, u32 loadFlag);
    virtual bool loadSoundItem(u32 id, u32 loadFlag);
    u32 getSoundType(u32 id) const override;

    bool isLoadedSoundItem(u32 id, u32 loadFlag) const;
    const nn::atk::SoundArchive* getSoundArchive() const;
    u32 getSoundIdFromIndex(s32 index) const;
    bool readSequenceSoundInfo(SequenceSoundInfo* pInfo, u32 id) const;
    bool readSoundInfo(SoundInfo* pInfo, u32 id) const;
    u32 getBankInfoSize(u32 id) const;
    u32 getBankWaveArcSize(u32 id) const;
    u32 getWaveArcSize(u32 id) const;
    u32 getSoundItemSize(u32 id) const;
    u32 getBankTotalSize(u32 id) const;
    u32 getSequenceSoundSize(u32 id) const;
    u32 getSoundPlayerCount() const;
    static u32 getSoundPlayerIdFromIndex(s32 index);
    sead::AudioSoundHeapNin* getSeadAudioSoundHeap();
    sead::SoundMemoryPoolHandler* tryGetSoundMemoryPoolHandler(const char* pName);
    sead::SoundMemoryPoolHandler* tryGetSoundMemoryPoolHandlerByFilePath(const char* pPath);

private:
    u32 getFileSize(u32 id) const {
        const nn::atk::SoundArchive* archive = getSoundArchive();
        nn::atk::SoundArchive::FileId fileId = archive->GetItemFileId(id);
        nn::atk::SoundArchive::FileInfo info;
        archive->detail_ReadFileInfo(fileId, &info);
        return info.fileSize;
    }
};

static_assert(sizeof(SeadAudioPlayer) == 0x3d0);
}  // namespace al
