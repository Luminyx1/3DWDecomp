#include <stream/seadFileDeviceStream.h>

namespace sead
{
FileDeviceStreamSrc::FileDeviceStreamSrc() = default;

FileDeviceStreamSrc::FileDeviceStreamSrc(FileHandle* pFileHandle)
    : mFileHandle(pFileHandle), mStartingPos(pFileHandle->getCurrentSeekPos()),
      mFileSize(pFileHandle->getFileSize())
{
}

FileDeviceStreamSrc::~FileDeviceStreamSrc()
{
    if (mIsHandleOpen && mFileHandle)
    {
        mFileHandle->close();
    }
}

u32 FileDeviceStreamSrc::read(void* pBuffer, u32 size)
{
    return mFileHandle->read(static_cast<u8*>(pBuffer), size);
}

u32 FileDeviceStreamSrc::write(const void* pBuffer, u32 size)
{
    return mFileHandle->write(static_cast<const u8*>(pBuffer), size);
}

u32 FileDeviceStreamSrc::skip(s32 offset)
{
    if (!mFileHandle->seek(offset, FileDevice::cSeekOrigin_Current))
    {
        return 0;
    }
    return offset;
}

void FileDeviceStreamSrc::rewind()
{
    mFileHandle->seek(mStartingPos, FileDevice::cSeekOrigin_Begin);
}

bool FileDeviceStreamSrc::isEOF()
{
    return mFileHandle->getCurrentSeekPos() >= mFileSize;
}

void FileDeviceStreamSrc::setFileHandle(sead::FileHandle* pFileHandle)
{
    mFileHandle = pFileHandle;
    if (pFileHandle)
    {
        mStartingPos = pFileHandle->getCurrentSeekPos();
        mFileSize = pFileHandle->getFileSize();
    }
}

FileDeviceWriteStream::FileDeviceWriteStream(Stream::Modes mode)
{
    setSrc(&src);
    setMode(mode);
}

FileDeviceWriteStream::FileDeviceWriteStream(StreamFormat* pFormat)
{
    setSrc(&src);
    setUserFormat(pFormat);
}

FileDeviceWriteStream::FileDeviceWriteStream(FileHandle* pFileHandle, Stream::Modes mode)
    : src(pFileHandle)
{
    setSrc(&src);
    setMode(mode);
}

FileDeviceWriteStream::FileDeviceWriteStream(FileHandle* pFileHandle, StreamFormat* pFormat)
    : src(pFileHandle)
{
    setSrc(&src);
    setUserFormat(pFormat);
}

FileDeviceWriteStream::~FileDeviceWriteStream()
{
    flush();
    mSrc = nullptr;
}

void FileDeviceWriteStream::setFileHandle(sead::FileHandle* pFileHandle)
{
    if (src.getFileHandle())
    {
        flush();
        rewind();
    }
    src.setFileHandle(pFileHandle);
}

FileDeviceReadStream::FileDeviceReadStream(Stream::Modes mode)
{
    setSrc(&src);
    setMode(mode);
}

FileDeviceReadStream::FileDeviceReadStream(StreamFormat* pFormat)
{
    setSrc(&src);
    setUserFormat(pFormat);
}

FileDeviceReadStream::FileDeviceReadStream(FileHandle* pFileHandle, Stream::Modes mode)
    : src(pFileHandle)
{
    setSrc(&src);
    setMode(mode);
}

FileDeviceReadStream::FileDeviceReadStream(FileHandle* pFileHandle, StreamFormat* pFormat)
    : src(pFileHandle)
{
    setSrc(&src);
    setUserFormat(pFormat);
}

FileDeviceReadStream::~FileDeviceReadStream()
{
    mSrc = nullptr;
}

void FileDeviceReadStream::setFileHandle(sead::FileHandle* pFileHandle)
{
    if (src.getFileHandle())
    {
        rewind();
    }

    src.setFileHandle(pFileHandle);
}

BufferFileDeviceWriteStream::BufferFileDeviceWriteStream(Stream::Modes mode)
    : FileDeviceWriteStream(mode), mBufferSrc(getSrc(), PtrUtil::align(mBuffer, 0x20), 0x100)
{
    setSrc(&mBufferSrc);
}

BufferFileDeviceWriteStream::BufferFileDeviceWriteStream(StreamFormat* pFormat)
    : FileDeviceWriteStream(pFormat), mBufferSrc(getSrc(), PtrUtil::align(mBuffer, 0x20), 0x100)
{
    setSrc(&mBufferSrc);
}

BufferFileDeviceWriteStream::BufferFileDeviceWriteStream(FileHandle* pFileHandle,
                                                         Stream::Modes mode)
    : FileDeviceWriteStream(pFileHandle, mode),
      mBufferSrc(getSrc(), PtrUtil::align(mBuffer, 0x20), 0x100)
{
    setSrc(&mBufferSrc);
}

BufferFileDeviceWriteStream::BufferFileDeviceWriteStream(FileHandle* pFileHandle,
                                                         StreamFormat* pFormat)
    : FileDeviceWriteStream(pFileHandle, pFormat),
      mBufferSrc(getSrc(), PtrUtil::align(mBuffer, 0x20), 0x100)
{
    setSrc(&mBufferSrc);
}

BufferFileDeviceReadStream::BufferFileDeviceReadStream(Stream::Modes mode)
    : FileDeviceReadStream(mode), mBufferSrc(getSrc(), PtrUtil::align(mBuffer, 0x20), 0x100)
{
    setSrc(&mBufferSrc);
}

BufferFileDeviceReadStream::BufferFileDeviceReadStream(StreamFormat* pFormat)
    : FileDeviceReadStream(pFormat), mBufferSrc(getSrc(), PtrUtil::align(mBuffer, 0x20), 0x100)
{
    setSrc(&mBufferSrc);
}

BufferFileDeviceReadStream::BufferFileDeviceReadStream(FileHandle* pFileHandle, Stream::Modes mode)
    : FileDeviceReadStream(pFileHandle, mode),
      mBufferSrc(getSrc(), PtrUtil::align(mBuffer, 0x20), 0x100)
{
    setSrc(&mBufferSrc);
}

BufferFileDeviceReadStream::BufferFileDeviceReadStream(FileHandle* pFileHandle,
                                                       StreamFormat* pFormat)
    : FileDeviceReadStream(pFileHandle, pFormat),
      mBufferSrc(getSrc(), PtrUtil::align(mBuffer, 0x20), 0x100)
{
    setSrc(&mBufferSrc);
}

}  // namespace sead
