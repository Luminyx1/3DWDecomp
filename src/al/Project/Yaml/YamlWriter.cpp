#include "Project/Yaml/YamlFormatter.hpp"

namespace al {
/**
 * Creates a YAML writer that writes to a memory buffer.
 * @param pBuffer output buffer
 * @param bufferSize size of the output buffer
 */
YamlWriterToMemory::YamlWriterToMemory(u8* pBuffer, u32 bufferSize)
    : mBuffer(pBuffer), mBufferSize(bufferSize),
      mRamStream(pBuffer, bufferSize, sead::Stream::Modes::Binary) {
    mStream = &mRamStream;
}

/**
 * Gets the number of bytes written.
 * @return the number of bytes written
 */
u32 YamlWriterToMemory::getUsedBufferSize() const {
    return mRamStream.getSrc().getCurrentPos();
}
}  // namespace al
