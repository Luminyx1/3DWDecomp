#pragma once

#include <nn/types.h>

namespace nn::atk {
namespace detail::fnd {
class FileStream;
}

namespace detail {
class SoundArchiveFile {
public:
    struct FileHeader {
        u32 GetStringBlockSize() const;
        u32 GetInfoBlockSize() const;
    };
};

class SoundArchiveFileReader {
public:
    u32 GetInfoBlockSize() const { return m_Header.GetInfoBlockSize(); }
    u32 GetStringBlockSize() const { return m_Header.GetStringBlockSize(); }

private:
    SoundArchiveFile::FileHeader m_Header;
};
}  // namespace detail

class SoundArchive {
public:
    typedef u32 ItemId;
    typedef ItemId FileId;

    enum SoundType {
        SoundType_Invalid,
        SoundType_Sequence,
        SoundType_Stream,
        SoundType_Wave,
        SoundType_AdvancedWave,
    };

    struct FileInfo {
        FileInfo() : fileSize(0xffffffff), offsetFromFileBlockHead(0xffffffff), externalFilePath(nullptr) {}

        u32 fileSize;
        u32 offsetFromFileBlockHead;
        const char* externalFilePath;
    };

    struct SequenceSoundInfo {
        SequenceSoundInfo() : startOffset(0), allocateTrackFlags(0), channelPriority(0), isReleasePriorityFix(false) {
            for (s32 i = 0; i < 4; i++) {
                bankIds[i] = 0xffffffff;
            }
        }

        u32 startOffset;
        u32 bankIds[4];
        u32 allocateTrackFlags;
        u8 channelPriority;
        bool isReleasePriorityFix;
    };

    struct SoundInfo {
        FileId fileId;
        ItemId playerId;
        u8 actorPlayerId;
        u8 playerPriority;
        u8 volume;
        u8 remoteFilter;
        s32 panMode;
        s32 panCurve;
        s32 singlePlayType;
        u16 singlePlayEffectiveDuration;
        bool isFrontBypass;
    };

    SoundArchive();
    virtual ~SoundArchive();

    bool IsAvailable() const;
    u32 GetSoundCount() const;
    const char* GetItemLabel(ItemId id) const;
    ItemId GetItemId(const char* pLabel) const;
    FileId GetItemFileId(ItemId id) const;
    SoundType GetSoundType(ItemId id) const;
    bool ReadSoundInfo(SoundInfo* pInfo, ItemId id) const;
    bool ReadSequenceSoundInfo(SequenceSoundInfo* pInfo, ItemId id) const;
    bool detail_ReadFileInfo(FileId id, FileInfo* pInfo) const;
    ItemId GetSoundIdFromIndex(u32 index) const { return index | 0x1000000; }

    virtual const void* detail_GetFileAddress(FileId fileId) const = 0;
    virtual size_t detail_GetRequiredStreamBufferSize() const = 0;

protected:
    virtual void FileAccessBegin() const {}
    virtual void FileAccessEnd() const {}
    virtual bool IsAddon() const { return false; }
    virtual detail::fnd::FileStream* OpenStream(void* pBuffer, size_t size, s64 begin,
                                                size_t length) const = 0;
    virtual detail::fnd::FileStream* OpenExtStream(void* pBuffer, size_t size, const char* pExtFilePath,
                                                   void* pCacheBuffer, size_t cacheSize) const = 0;

private:
    u8 _8[0x2a0 - 0x8];
};
static_assert(sizeof(SoundArchive) == 0x2a0);

class AddonSoundArchive : public SoundArchive {};

class FsSoundArchive : public SoundArchive {
public:
    enum FileAccessMode {
        FileAccessMode_Always,
        FileAccessMode_InFunction
    };

    FsSoundArchive();
    ~FsSoundArchive() override;

    bool Open(const char* pPath);
    void Close();
    bool LoadHeader(void* pBuffer, size_t size);
    bool LoadLabelStringData(void* pBuffer, size_t size);

    size_t GetHeaderSize() const { return m_ArchiveReader.GetInfoBlockSize(); }
    size_t GetLabelStringDataSize() const { return m_ArchiveReader.GetStringBlockSize(); }
    void SetFileAccessMode(FileAccessMode mode) { m_FileAccessMode = static_cast<u8>(mode); }

    size_t detail_GetRequiredStreamBufferSize() const override;
    const void* detail_GetFileAddress(FileId fileId) const override { return nullptr; }

protected:
    void FileAccessBegin() const override;
    void FileAccessEnd() const override;
    detail::fnd::FileStream* OpenStream(void* pBuffer, size_t size, s64 begin,
                                        size_t length) const override;
    detail::fnd::FileStream* OpenExtStream(void* pBuffer, size_t size, const char* pExtFilePath,
                                           void* pCacheBuffer, size_t cacheSize) const override;

private:
    detail::SoundArchiveFileReader m_ArchiveReader;
    u8 _2a1[0x368 - 0x2a1];
    bool m_IsOpened;
    u8 m_FileAccessMode;
    u8 _36a[0x610 - 0x36a];
};
static_assert(sizeof(FsSoundArchive) == 0x610);

class MemorySoundArchive : public SoundArchive {
public:
    MemorySoundArchive();
    ~MemorySoundArchive() override;

    bool Initialize(const void* pSoundArchiveData);
    void Finalize();

    size_t detail_GetRequiredStreamBufferSize() const override;
    const void* detail_GetFileAddress(FileId fileId) const override;

protected:
    detail::fnd::FileStream* OpenStream(void* pBuffer, size_t size, s64 begin,
                                        size_t length) const override;
    detail::fnd::FileStream* OpenExtStream(void* pBuffer, size_t size, const char* pExtFilePath,
                                           void* pCacheBuffer, size_t cacheSize) const override;

private:
    u8 _2a0[0x2f0 - 0x2a0];
};
static_assert(sizeof(MemorySoundArchive) == 0x2f0);
}  // namespace nn::atk
