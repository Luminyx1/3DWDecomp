#include <nn/atk/atk_MemoryFileStream.h>
#include <cstring>

namespace nn::atk::detail {
// memory supplies the read-only bytes; size is the length of the buffer.
MemoryFileStream::MemoryFileStream(const void* memory, size_t size)
    : mMemory(static_cast<const u8*>(memory)), mSize(size), mPosition(0) {}
MemoryFileStream::~MemoryFileStream() = default;
void MemoryFileStream::Close() { mMemory = nullptr; mSize = 0; mPosition = 0; }
bool MemoryFileStream::IsOpened() const { return mMemory != nullptr; }

// output receives up to size bytes; result is unused by the original memory reader.
size_t MemoryFileStream::Read(void* output, size_t size, fnd::FndResult* result) {
    size_t available = mSize - mPosition;
    size_t count = available < size ? available : size;
    std::memcpy(output, mMemory + mPosition, count);
    mPosition += count;
    return count;
}

// input, size, and result are unused: writing to this stream is unsupported.
size_t MemoryFileStream::Write(const void* input, size_t size, fnd::FndResult* result) { return 0; }

// offset is relative to origin; end-relative offsets are subtracted from the size.
fnd::FndResult MemoryFileStream::Seek(long offset, SeekOrigin origin) {
    switch (origin) {
    case SeekOrigin_Begin: mPosition = offset; break;
    case SeekOrigin_End: mPosition = mSize - offset; break;
    case SeekOrigin_Current: mPosition += offset; break;
    default: return {0x80000000};
    }
    return {0};
}
size_t MemoryFileStream::GetCurrentPosition() const { return mPosition; }
size_t MemoryFileStream::GetSize() const { return mSize; }
bool MemoryFileStream::CanRead() const { return true; }
bool MemoryFileStream::CanWrite() const { return false; }
bool MemoryFileStream::CanSeek() const { return true; }
// path and mode are unused: this stream is initialized from memory, not a filename.
fnd::FndResult MemoryFileStream::Open(const char* path, AccessMode mode) { return {0x80000000}; }
void MemoryFileStream::Flush() {}
// buffer and size are unused because memory streams do not support a separate cache.
void MemoryFileStream::EnableCache(void* buffer, size_t size) {}
void MemoryFileStream::DisableCache() {}
bool MemoryFileStream::IsCacheEnabled() const { return false; }
size_t MemoryFileStream::GetIoBufferAlignment() const { return 1; }
bool MemoryFileStream::CanSetFsAccessLog() const { return false; }
// log is unused; memory streams do not attach filesystem access logs.
fnd::FileStream* MemoryFileStream::SetFsAccessLog(fnd::FsAccessLog* log) { return nullptr; }
size_t MemoryFileStream::GetCachePosition() { return 0; }
size_t MemoryFileStream::GetCachedLength() { return 0; }
}
