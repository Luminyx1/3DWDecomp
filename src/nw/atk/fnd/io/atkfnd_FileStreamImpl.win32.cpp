#include <nn/atk/atkfnd_FileStreamImpl.h>
#include <nn/fs/fs_files.h>
#include <nn/diag.h>

namespace nn::atk::detail::fnd {
namespace {
const int OpenModes[5] = {2, 3, 1, 1, 6};
}

// path names the file; mode selects the filesystem access flags.
// As in the original, the opened flag is set before the filesystem call, including when it fails.
FndResult FileStreamImpl::Open(const char* path, AccessMode mode) {
    mPosition = 0;
    mOpened = true;
    int openMode = 1;
    unsigned index = unsigned(mode) - 2;

    if (index <= 4) openMode = OpenModes[static_cast<int>(index)];
    auto result = fs::OpenFile(&mHandle, path, openMode);

    if (result.IsSuccess()) return {0};

    if (result.GetModule() == 2) {
        u32 descriptionBits = result.GetInnerValueForDebug() & 0x3ffe00;

        if (descriptionBits == 0x200) return {0x81000001};

        if (unsigned(result.GetDescription() - 4000) < 1000) return {0x81000002};

        if (descriptionBits == 0xe00) return {0x81000003};
    }

    return {0x81000000};
}

void FileStreamImpl::Close() {
    if (IsOpened()) {
        mPosition = 0;
        mOpened = false;
        fs::CloseFile(mHandle);
    }
}

void FileStreamImpl::Flush() { if (IsOpened()) fs::FlushFile(mHandle); }
bool FileStreamImpl::IsOpened() const { return mOpened; }
size_t FileStreamImpl::GetSize() const {
    if (mFileSize == 0xffffffffu) {
        long size;

        if (fs::GetFileSize(&size, mHandle).IsFailure()) {
            nn::diag::detail::AbortImpl("", "", "", 0);
            __builtin_unreachable();
        }

        mFileSize = size;
    }

    return mFileSize;
}

size_t FileStreamImpl::GetIoBufferAlignment() const { return 1; }
// output receives up to size bytes; optional result receives success, short-read, or failure status.
size_t FileStreamImpl::ReadDirect(void* output, size_t size, FndResult* result) {
    ValidateAlignment(output);
    size_t read = 0;
    u32 status;

    if (fs::ReadFile(&read, mHandle, mPosition, output, size).IsSuccess()) {
        status = read != size;
        mPosition += read;
    } else status = 0x81000000;

    if (result) result->value = status;
    return read;
}

// input supplies size bytes; optional result receives the write status.
// The original returns size even on failure and does not initialize the WriteOption field.
size_t FileStreamImpl::WriteDirect(const void* input, size_t size, FndResult* result) {
    ValidateAlignment(input);
    mFileSize = 0xffffffffu;
    fs::WriteOption option;
    u32 status;

    if (fs::WriteFile(mHandle, mPosition, input, size, option).IsSuccess()) {
        mPosition += size;
        status = 0;
    } else status = 0x81000000;

    if (result) result->value = status;
    return size;
}

// offset and origin select the direct position. End-relative negative offsets move beyond EOF,
// while current-relative seeks clamp to the file bounds, matching the original's distinct rules.
FndResult FileStreamImpl::SeekDirect(long offset, SeekOrigin origin) {
    long size = GetSize();
    long position = 0;

    switch (origin) {
    case SeekOrigin_Begin: position = offset > 0 ? offset : 0; break;
    case SeekOrigin_End: position = size - (offset < 0 ? offset : 0); break;
    case SeekOrigin_Current:
        if (offset > 0) {
            long next = GetCurrentPosition() + offset;
            position = next > size ? size : next;
        } else if (offset < 0) {
            if (static_cast<long>(GetCurrentPosition()) <= -offset) position = 0;
            else position = GetCurrentPosition() + offset;
        } else position = GetCurrentPosition();
        break;
    default: break;
    }

    mPosition = position;
    return {0};
}

FileStreamImpl::~FileStreamImpl() = default;
size_t FileStreamImpl::GetCurrentPosition() const { return IsCacheEnabled() ? mCache.GetCurrentPosition() : mPosition; }
bool FileStreamImpl::IsCacheEnabled() const { return mCache.IsInitialized(); }
bool FileStreamImpl::CanSetFsAccessLog() const { return true; }
// log is the filesystem access log to attach; the returned stream is this object.
FileStream* FileStreamImpl::SetFsAccessLog(FsAccessLog* log) { mLog = log; return this; }
size_t FileStreamImpl::GetCachePosition() { return IsCacheEnabled() ? mCache.GetCachePosition() : 0; }
size_t FileStreamImpl::GetCachedLength() { return IsCacheEnabled() ? mCache.GetCachedLength() : 0; }

void FileStreamImpl::DirectStream::Close() {}
bool FileStreamImpl::DirectStream::IsOpened() const { return mOwner->IsOpened(); }
// output, size and result are forwarded to the owner's uncached read operation.
size_t FileStreamImpl::DirectStream::Read(void* output, size_t size, FndResult* result) { return mOwner->ReadDirect(output, size, result); }
// input, size and result are forwarded to the owner's uncached write operation.
size_t FileStreamImpl::DirectStream::Write(const void* input, size_t size, FndResult* result) { return mOwner->WriteDirect(input, size, result); }
// offset and origin select the owner's uncached stream position.
FndResult FileStreamImpl::DirectStream::Seek(long offset, SeekOrigin origin) { return mOwner->SeekDirect(offset, origin); }
size_t FileStreamImpl::DirectStream::GetCurrentPosition() const { return mOwner->mPosition; }
size_t FileStreamImpl::DirectStream::GetSize() const { return mOwner->GetSize(); }
bool FileStreamImpl::DirectStream::CanRead() const { return mOwner->CanRead(); }
bool FileStreamImpl::DirectStream::CanWrite() const { return mOwner->CanWrite(); }
bool FileStreamImpl::DirectStream::CanSeek() const { return mOwner->CanSeek(); }
}
