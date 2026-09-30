#include <nn/atk/atkfnd_FileStreamImpl.h>

namespace nn::atk::detail::fnd {
// output receives up to size bytes; result receives status from the selected cached or direct path.
size_t FileStreamImpl::Read(void* output, size_t size, FndResult* result) {
    if (IsCacheEnabled()) return mCache.Read(output, size, result, mLog, this);
    return ReadDirect(output, size, result);
}

// input supplies size bytes; result receives status from the selected cached or direct path.
size_t FileStreamImpl::Write(const void* input, size_t size, FndResult* result) {
    if (IsCacheEnabled()) return mCache.Write(input, size, result);
    return WriteDirect(input, size, result);
}

// offset and origin select a position through the active cached or direct path.
FndResult FileStreamImpl::Seek(long offset, SeekOrigin origin) {
    FndResult result;

    if (IsCacheEnabled()) result = mCache.Seek(offset, origin);
    else result = SeekDirect(offset, origin);
    return {static_cast<u32>(result.value)};
}

bool FileStreamImpl::CanRead() const { return IsOpened(); }
bool FileStreamImpl::CanWrite() const { return IsOpened(); }
bool FileStreamImpl::CanSeek() const { return IsOpened(); }
// buffer and size describe the supplied cache allocation. The cache starts at the next aligned byte.
void FileStreamImpl::EnableCache(void* buffer, size_t size) {
    if (mCache.IsInitialized()) mCache.Finalize();
    long alignment = static_cast<int>(GetIoBufferAlignment());
    uintptr_t start = (reinterpret_cast<uintptr_t>(buffer) + alignment - 1) & -alignment;
    size_t available = reinterpret_cast<uintptr_t>(buffer) + size - start;
    mCache.Initialize(&mDirectStream, reinterpret_cast<void*>(start), available);
}

void FileStreamImpl::DisableCache() { mCache.Finalize(); }
// buffer is unused in this build; the original only queries the stream's alignment.
void FileStreamImpl::ValidateAlignment(const void* buffer) const { GetIoBufferAlignment(); }
FileStreamImpl::FileStreamImpl()
    : mHandle{}, mOpened(false), mFileSize(0xffffffffu), mPosition(0), mCache() {
    mLog = nullptr;
    mDirectStream.SetOwner(this);
}
}
