#pragma once
#include <nn/atk/atkfnd_FileStream.h>

namespace nn::atk::detail::fnd {
class StreamCache {
public:
    StreamCache();
    virtual ~StreamCache() {}
    void Initialize(Stream* stream, void* buffer, size_t size);
    void Finalize();
    size_t Read(void* output, size_t size, FndResult* result, FsAccessLog* log, void* owner);
    size_t Write(const void* input, size_t size, FndResult* result);
    FndResult Seek(long offset, Stream::SeekOrigin origin);
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
    bool mDirty;
};
static_assert(sizeof(StreamCache) == 0x40, "StreamCache size");
}
