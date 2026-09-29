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

    SoundArchive();
    virtual ~SoundArchive();

    bool IsAvailable() const;
    u32 GetSoundCount() const;
    const char* GetItemLabel(ItemId id) const;
    ItemId GetItemId(const char* pLabel) const;

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
