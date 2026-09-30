#include <nn/atk/atkfnd_StreamCache.h>
#include <cstring>

namespace nn::atk::detail::fnd {
StreamCache::StreamCache()
    : mStream(nullptr), mPosition(0), mBuffer(nullptr), mBufferSize(0),
      mCachePosition(0xffffffffu), mCachedLength(0), mState(Empty) {}

// stream supplies I/O; buffer points to size bytes of caller-owned cache storage.
StreamCache::StreamCache(Stream* stream, void* buffer, size_t size)
    : mStream(stream), mPosition(0), mBuffer(buffer), mBufferSize(size),
      mCachePosition(0xffffffffu), mCachedLength(0), mState(Empty) {}

// stream supplies I/O; buffer points to size bytes of caller-owned cache storage.
void StreamCache::Initialize(Stream* stream, void* buffer, size_t size) {
    mStream = stream;
    mPosition = 0;
    mBuffer = buffer;
    mBufferSize = size;
    mCachePosition = 0xffffffffu;
    mCachedLength = 0;
    mState = Empty;
}

void StreamCache::Finalize() {
    mStream = nullptr;
    mPosition = 0;
    mBuffer = nullptr;
    mBufferSize = 0;
    mCachePosition = 0xffffffffu;
    mCachedLength = 0;
    mState = Empty;
}

// output receives up to size bytes; result is optional. log brackets underlying
// reads, passing owner through to identify the stream in each callback.
size_t StreamCache::Read(void* output, size_t size, FndResult* result, FsAccessLog* log, void* owner) {
    if (!IsInitialized()) {
        if (log) log->OnReadBegin(owner);
        size_t count = mStream->Read(output, size, result);

        if (log) log->OnReadEnd(owner);
        return count;
    }

    FndResult flush = FlushWriteCache();

    if (static_cast<s32>(flush.value) < 0) {
        if (result) *result = flush;
        return 0;
    }

    size_t hit = GetReadCacheHitLength(size);

    if (hit) {
        std::memcpy(output, static_cast<u8*>(mBuffer) + (mPosition - mCachePosition), hit);
        mPosition += hit;
    }

    if (hit >= size)
        return size;
    FndResult sync = SyncStreamCurrentPosition(mPosition);

    if (static_cast<s32>(sync.value) < 0) {
        if (result) *result = sync;
        return 0;
    }

    size_t remaining = size - hit;
    size_t actual;

    if (remaining > mBufferSize) {
        if (log) log->OnReadBegin(owner);
        FndResult status = {0};
        actual = mStream->Read(output, remaining, &status);

        if (log) log->OnReadEnd(owner);

        if (result) *result = status;

        if (static_cast<s32>(status.value) < 0) {
            ClearCache();
            return hit;
        }

        size_t retained = mBufferSize < actual ? mBufferSize : actual;
        // Preserve the original signed-16-bit displacement and full-buffer copy.
        size_t position = mPosition;
        int displacement = -static_cast<s16>(retained);
        u8* source = static_cast<u8*>(output) + displacement;
        mCachePosition = position - retained;
        mCachedLength = retained;
        std::memcpy(mBuffer, source, mBufferSize);
    } else {
        if (log) log->OnReadBegin(owner);
        FndResult status = {0};
        actual = mStream->Read(mBuffer, mBufferSize, &status);

        if (log) log->OnReadEnd(owner);

        if (result) *result = status;

        if (static_cast<s32>(status.value) < 0) {
            ClearCache();
            return hit;
        }

        mState = Reading;
        mCachePosition = mPosition;
        mCachedLength = actual;
        size_t count = actual < remaining ? actual : remaining;
        std::memcpy(static_cast<u8*>(output) + hit, mBuffer, count);
    }

    mPosition += remaining;
    size_t total = hit + actual;

    if (total >= size) {
        if (result) result->value = 0;
        return size;
    }

    return total;
}

FndResult StreamCache::FlushWriteCache() {
    if (!mCachedLength || mState != Writing)
        return {1};
    FndResult result = SyncStreamCurrentPosition(mCachePosition);

    if (static_cast<s32>(result.value) < 0)
        return result;
    result.value = 0;
    mStream->Write(mBuffer, mCachedLength, &result);
    mState = Empty;
    mCachePosition = 0xffffffffu;
    mCachedLength = 0;
    return result;
}

// size is the requested byte count beginning at the logical stream position.
size_t StreamCache::GetReadCacheHitLength(size_t size) const {
    if (!IsInitialized() || mState != Reading || mCachePosition == 0xffffffffu)
        return 0;
    if (static_cast<long>(mPosition) < static_cast<long>(mCachePosition))
        return 0;
    long offset = mPosition - mCachePosition;

    if (mCachedLength < static_cast<size_t>(offset) + size)
        return (mCachedLength > static_cast<size_t>(offset) ? mCachedLength : static_cast<size_t>(offset)) - offset;
    return size;
}

// position is the absolute byte offset required for the next underlying operation.
FndResult StreamCache::SyncStreamCurrentPosition(long position) {
    if (mStream->GetCurrentPosition() == static_cast<size_t>(position))
        return {1};
    if (!mStream->CanSeek())
        return {0x80000000u};
    return mStream->Seek(position, Stream::SeekOrigin_Begin);
}

void StreamCache::ClearCache() {
    FlushWriteCache();
    mCachePosition = 0xffffffffu;
    mCachedLength = 0;
}

// input supplies size bytes; result optionally receives the underlying I/O status.
size_t StreamCache::Write(const void* input, size_t size, FndResult* result) {
    if (!IsInitialized())
        return mStream->Write(input, size, result);
    if (size > mBufferSize) {
        FndResult status = FlushWriteCache();

        if (static_cast<s32>(status.value) < 0) {
            if (result) *result = status;
            return 0;
        }

        return mStream->Write(input, size, result);
    }

    if (GetWritableCacheLength(size) < size) {
        FndResult status = FlushWriteCache();

        if (static_cast<s32>(status.value) < 0) {
            if (result) *result = status;
            return 0;
        }
    }

    std::memcpy(static_cast<u8*>(mBuffer) + mCachedLength, input, size);

    if (mState == Writing) {
        mCachedLength += size;
    } else {
        mState = Writing;
        mCachePosition = mPosition;
        mCachedLength = size;
    }

    mPosition += size;

    if (mCachedLength && mCachedLength == mBufferSize) {
        FndResult status = FlushWriteCache();

        if (static_cast<s32>(status.value) < 0) {
            if (result) *result = status;
            return size;
        }
    }

    if (result) result->value = 0;
    return size;
}

// size is the number of bytes proposed for a write into the cache.
size_t StreamCache::GetWritableCacheLength(size_t size) const {
    if (!IsInitialized())
        return 0;
    size_t available = mBufferSize;

    if (mState != Reading)
        available -= mCachedLength;
    return available < size ? available : size;
}

// offset is relative to origin; successful seeks change only the logical position.
FndResult StreamCache::Seek(long offset, Stream::SeekOrigin origin) {
    long size = mStream->GetSize();

    if (!IsInitialized())
        return mStream->Seek(offset, origin);
    if (!mStream->CanSeek())
        return {0x80000000u};
    switch (origin) {
    case Stream::SeekOrigin_Begin:
        if (offset < 0 || offset >= size)
            return {0x80000000u};
        mPosition = offset;
        return {0};
    case Stream::SeekOrigin_Current: {
        long position = mPosition + offset;

        if (size <= position)
            return {0x80000000u};
        mPosition = position;
        return {0};
    }
    case Stream::SeekOrigin_End: {
        long position = size - offset;

        if (size < offset)
            return {0x80000000u};
        mPosition = position;
        return {0};
    }
    default:
        return {0x80000000u};
    }
}

}
