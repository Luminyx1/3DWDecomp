#pragma once
#include <nn/atk/atkfnd_StreamCache.h>
#include <nn/fs/fs_types.h>

namespace nn::atk::detail::fnd {
class FileStreamImpl : public FileStream {
public:
    FileStreamImpl();
    ~FileStreamImpl() override;
    FndResult Open(const char* path, AccessMode mode) override;
    void Close() override;
    void Flush() override;
    bool IsOpened() const override;
    bool CanRead() const override;
    bool CanWrite() const override;
    bool CanSeek() const override;
    size_t GetSize() const override;
    size_t Read(void* output, size_t size, FndResult* result) override;
    size_t Write(const void* input, size_t size, FndResult* result) override;
    FndResult Seek(long offset, SeekOrigin origin) override;
    size_t GetCurrentPosition() const override;
    void EnableCache(void* buffer, size_t size) override;
    void DisableCache() override;
    bool IsCacheEnabled() const override;
    size_t GetIoBufferAlignment() const override;
    bool CanSetFsAccessLog() const override;
    FsAccessLog* SetFsAccessLog(FsAccessLog* log) override;
    size_t GetCachePosition() override;
    size_t GetCachedLength() override;
    size_t ReadDirect(void* output, size_t size, FndResult* result);
    size_t WriteDirect(const void* input, size_t size, FndResult* result);
    FndResult SeekDirect(long offset, SeekOrigin origin);
    void ValidateAlignment(const void* buffer) const;
private:
    class DirectStream : public Stream {
    public:
        DirectStream() = default;
        // owner is the file stream whose uncached operations this adapter exposes.
        void SetOwner(FileStreamImpl* owner) { mOwner = owner; }
        ~DirectStream() override = default;
        void Close() override;
        bool IsOpened() const override;
        size_t Read(void* output, size_t size, FndResult* result) override;
        size_t Write(const void* input, size_t size, FndResult* result) override;
        FndResult Seek(long offset, SeekOrigin origin) override;
        size_t GetCurrentPosition() const override;
        size_t GetSize() const override;
        bool CanRead() const override;
        bool CanWrite() const override;
        bool CanSeek() const override;
    private:
        FileStreamImpl* mOwner;
    };
    fs::FileHandle mHandle;
    bool mOpened;
    mutable size_t mFileSize;
    size_t mPosition;
    StreamCache mCache;
    DirectStream mDirectStream;
    FsAccessLog* mLog;
};
static_assert(sizeof(FileStreamImpl) == 0x80, "FileStreamImpl size");
}
