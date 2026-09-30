#include <nn/atk/atkfnd_FileStreamProxy.h>

namespace nn::atk::detail::fnd {
// stream supplies the underlying operations; begin and length define the permitted seek range.
FileStreamProxy::FileStreamProxy(FileStream* stream, long begin, size_t length)
    : mStream(stream), mBegin(begin), mLength(length) {}
FileStreamProxy::~FileStreamProxy() { mStream = nullptr; mBegin = 0; mLength = 0; }
// path and mode are passed to the underlying stream; only the result's low 32 bits are returned.
FndResult FileStreamProxy::Open(const char* path, AccessMode mode) {
    return {static_cast<u32>(mStream->Open(path, mode).value)};
}
void FileStreamProxy::Close() { mStream->Close(); }
void FileStreamProxy::Flush() { mStream->Flush(); }
bool FileStreamProxy::IsOpened() const { return mStream->IsOpened(); }
bool FileStreamProxy::CanRead() const { return mStream->CanRead(); }
bool FileStreamProxy::CanWrite() const { return mStream->CanWrite(); }
bool FileStreamProxy::CanSeek() const { return mStream->CanSeek(); }
size_t FileStreamProxy::GetSize() const { return mStream->GetSize(); }
// output receives up to size bytes; result receives the underlying stream's status.
size_t FileStreamProxy::Read(void* output, size_t size, FndResult* result) {
    return mStream->Read(output, size, result);
}
// input supplies size bytes; result receives the underlying stream's status.
size_t FileStreamProxy::Write(const void* input, size_t size, FndResult* result) {
    return mStream->Write(input, size, result);
}
// offset is interpreted relative to origin, then clamped to the proxy's byte range.
// The original passes the computed position and the unchanged origin to the underlying stream.
FndResult FileStreamProxy::Seek(long offset, SeekOrigin origin) {
    long length = mLength;
    long position;
    switch (origin) {
    case SeekOrigin_Begin: position = mBegin + offset; break;
    case SeekOrigin_End: position = mBegin + length - offset; break;
    case SeekOrigin_Current: position = mStream->GetCurrentPosition() + offset; break;
    default: return {0x80000000};
    }
    long end = mBegin + length;
    long clamped = position > end ? end : position;
    if (position < mBegin) clamped = mBegin;
    return {static_cast<u32>(mStream->Seek(clamped, origin).value)};
}
size_t FileStreamProxy::GetCurrentPosition() const { return mStream->GetCurrentPosition(); }
// buffer and size describe the cache storage to attach to the underlying stream.
void FileStreamProxy::EnableCache(void* buffer, size_t size) { mStream->EnableCache(buffer, size); }
void FileStreamProxy::DisableCache() { mStream->DisableCache(); }
bool FileStreamProxy::IsCacheEnabled() const { return mStream->IsCacheEnabled(); }
size_t FileStreamProxy::GetIoBufferAlignment() const { return mStream->GetIoBufferAlignment(); }
bool FileStreamProxy::CanSetFsAccessLog() const { return mStream->CanSetFsAccessLog(); }
// log is the filesystem access log to attach; the underlying stream supplies the returned stream pointer.
FileStream* FileStreamProxy::SetFsAccessLog(FsAccessLog* log) { return mStream->SetFsAccessLog(log); }
size_t FileStreamProxy::GetCachePosition() { return mStream->GetCachePosition(); }
size_t FileStreamProxy::GetCachedLength() { return mStream->GetCachedLength(); }
}
