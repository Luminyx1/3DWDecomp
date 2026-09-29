#pragma once

#include <stream/seadRamStream.h>
#include "Project/Action/Ctrl/YamlFormatter.hpp"

namespace al {
    /// Writes YAML text into a memory buffer.
    class YamlWriterToMemory : public YamlFormatter {
    public:
        YamlWriterToMemory(u8* pBuffer, u32 bufferSize);

        virtual ~YamlWriterToMemory() {}

        u32 getUsedBufferSize() const;

        u8* mBuffer;                        // _18
        u32 mBufferSize;                    // _20
        sead::RamWriteStream mRamStream;    // _28
    };
};
