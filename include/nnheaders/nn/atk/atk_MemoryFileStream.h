#pragma once
#include <nn/atk/atkfnd_FileStream.h>

namespace nn::atk::detail {
class MemoryFileStream : public fnd::FileStream {
public:
    MemoryFileStream(const void* memory, size_t size);
    ~MemoryFileStream() override;
    void Close() override;
    bool IsOpened() const override;
    size_t Read(void* output, size_t size, fnd::FndResult* result) override;
    size_t Write(const void* input, size_t size, fnd::FndResult* result) override;
    fnd::FndResult Seek(long offset, SeekOrigin origin) override;
    size_t GetCurrentPosition() const override;
    size_t GetSize() const override;
    bool CanRead() const override;
    bool CanWrite() const override;
    bool CanSeek() const override;
    fnd::FndResult Open(const char* path, AccessMode mode) override;
    void Flush() override;
    void EnableCache(void* buffer, size_t size) override;
    void DisableCache() override;
    bool IsCacheEnabled() const override;
    size_t GetIoBufferAlignment() const override;
    bool CanSetFsAccessLog() const override;
    fnd::FsAccessLog* SetFsAccessLog(fnd::FsAccessLog* log) override;
    size_t GetCachePosition() override;
    size_t GetCachedLength() override;

private:
    const u8* mMemory;
    size_t mSize;
    size_t mPosition;
};
static_assert(sizeof(MemoryFileStream) == 0x20, "MemoryFileStream size");
}
