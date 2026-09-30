#pragma once
#include <nn/types.h>

namespace nn::atk::detail::fnd {
struct FndResult { u32 value; };
static_assert(sizeof(FndResult) == 4, "FndResult size");
class FsAccessLog {
public:
    // owner identifies the stream whose underlying read is beginning or ending.
    virtual void OnReadBegin(void* owner) = 0;
    virtual void OnReadEnd(void* owner) = 0;
};
class Stream {
public:
    enum SeekOrigin { SeekOrigin_Begin, SeekOrigin_End, SeekOrigin_Current };
    // Derived streams inline this empty destructor; retain its exported entry points.
    __attribute__((used)) virtual ~Stream() {}
    virtual void Close() = 0;
    virtual bool IsOpened() const = 0;
    virtual size_t Read(void* output, size_t size, FndResult* result) = 0;
    virtual size_t Write(const void* input, size_t size, FndResult* result) = 0;
    virtual FndResult Seek(long offset, SeekOrigin origin) = 0;
    virtual size_t GetCurrentPosition() const = 0;
    virtual size_t GetSize() const = 0;
    virtual bool CanRead() const = 0;
    virtual bool CanWrite() const = 0;
    virtual bool CanSeek() const = 0;
};
class FileStream : public Stream {
public:
    enum AccessMode { AccessMode_Read = 1, AccessMode_Write = 2 };
    virtual FndResult Open(const char* path, AccessMode mode) = 0;
    virtual void Flush() = 0;
    virtual void EnableCache(void* buffer, size_t size) = 0;
    virtual void DisableCache() = 0;
    virtual bool IsCacheEnabled() const = 0;
    virtual size_t GetIoBufferAlignment() const = 0;
    virtual bool CanSetFsAccessLog() const = 0;
    virtual FileStream* SetFsAccessLog(FsAccessLog* log) = 0;
    virtual size_t GetCachePosition() = 0;
    virtual size_t GetCachedLength() = 0;
};
}
