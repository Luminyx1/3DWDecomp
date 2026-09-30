#pragma once
#include <nn/atk/atkfnd_FileStream.h>

namespace nn::atk::detail::fnd {
class StreamCache {
public:
    StreamCache();
    StreamCache(Stream* stream, void* buffer, size_t size);
    virtual ~StreamCache() {}
    void Initialize(Stream* stream, void* buffer, size_t size);
    void Finalize();
    size_t Read(void* output, size_t size, FndResult* result, FsAccessLog* log, void* owner);
    size_t Write(const void* input, size_t size, FndResult* result);
    FndResult Seek(long offset, Stream::SeekOrigin origin);
    FndResult FlushWriteCache();
    size_t GetReadCacheHitLength(size_t size) const;
    FndResult SyncStreamCurrentPosition(long position);
    void ClearCache();
    size_t GetWritableCacheLength(size_t size) const;
    bool IsInitialized() const { return mStream && mBuffer && mBufferSize; }
    size_t GetCurrentPosition() const { return mPosition; }
    size_t GetCachePosition() const { return mCachePosition; }
    size_t GetCachedLength() const { return mCachedLength; }
private:
    Stream* mStream;
    size_t mPosition;
    void* mBuffer;
    size_t mBufferSize;
    size_t mCachePosition;
    size_t mCachedLength;
    enum CacheState : u8 { Empty, Reading, Writing };
    CacheState mState;
};
static_assert(sizeof(StreamCache) == 0x40, "StreamCache size");
}
