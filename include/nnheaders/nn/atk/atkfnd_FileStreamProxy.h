#pragma once
#include <nn/atk/atkfnd_FileStream.h>

namespace nn::atk::detail::fnd {
class FileStreamProxy : public FileStream {
public:
    FileStreamProxy(FileStream* stream, long begin, size_t length);
    ~FileStreamProxy() override;
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
    FileStream* SetFsAccessLog(FsAccessLog* log) override;
    size_t GetCachePosition() override;
    size_t GetCachedLength() override;
private:
    FileStream* mStream;
    long mBegin;
    size_t mLength;
};
static_assert(sizeof(FileStreamProxy) == 0x20, "FileStreamProxy size");
}
