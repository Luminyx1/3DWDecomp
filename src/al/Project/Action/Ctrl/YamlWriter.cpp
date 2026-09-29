#include "Project/Action/Ctrl/YamlWriter.hpp"

namespace al {
    /**
     * @brief Constructs a writer that writes into the given buffer.
     * @param pBuffer The buffer to write the YAML text into.
     * @param bufferSize The size of the buffer in bytes.
     */
    YamlWriterToMemory::YamlWriterToMemory(u8* pBuffer, u32 bufferSize)
        : mBuffer(pBuffer), mBufferSize(bufferSize), mRamStream(pBuffer, bufferSize, sead::Stream::Modes::Binary) {
        mStream = &mRamStream;
    }

    /**
     * @brief Gets how much of the buffer has been written to.
     * @return The number of bytes written.
     */
    u32 YamlWriterToMemory::getUsedBufferSize() const {
        return mRamStream.getSrc().getCurrentPos();
    }
};
