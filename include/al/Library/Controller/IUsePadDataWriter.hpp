#pragma once

namespace al {
struct PadDataPack;

class IUsePadDataWriter {
public:
    virtual void write(const PadDataPack& rFrameData) = 0;
    virtual void open() = 0;
    virtual void close() = 0;
};
}  // namespace al
