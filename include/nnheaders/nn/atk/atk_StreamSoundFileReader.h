#pragma once
#include <nn/atk/atk_StreamSoundFile.h>

namespace nn::atk::detail {
class StreamSoundFileReader {
public:
    StreamSoundFileReader();
    struct TrackInfo { u8 volume, pan, surroundPan, _03, channelCount, channels[2]; };
    void Initialize(const void* file);
    static bool IsValidFileHeader(const void* file);
    void Finalize();
    bool IsTrackInfoAvailable() const;
    bool IsOriginalLoopAvailable() const;
    static bool IsOriginalLoopAvailableImpl(const StreamSoundFile::FileHeader* header);
    bool IsCrc32CheckAvailable() const;
    bool IsRegionIndexCheckAvailable() const;
    bool ReadStreamSoundInfo(StreamSoundFile::StreamSoundInfo* info) const;
    bool ReadStreamTrackInfo(TrackInfo* info, int track) const;
    bool ReadDspAdpcmChannelInfo(DspAdpcmParam* param, DspAdpcmLoopParam* loop, int channel) const;
private:
    const StreamSoundFile::FileHeader* mHeader;
    const StreamSoundFile::InfoBlockBody* mInfo;
};
static_assert(sizeof(StreamSoundFileReader) == 0x10, "StreamSoundFileReader size");
}
