#pragma once
#include <nn/atk/atk_StreamSoundFileReader.h>

namespace nn::atk::detail {
namespace fnd {
class FileStream;
}

class IRegionInfoReadable {
public:
    virtual ~IRegionInfoReadable();
    /**
     * @brief Read a stream region's playback information.
     * @param pInfo Destination for the region information; must not be null.
     * @param index Zero-based region index in the stream resource.
     * @return Whether the region information was read successfully.
     */
    virtual bool ReadRegionInfo(StreamSoundFile::RegionInfo* pInfo, u32 index) const = 0;
};

class StreamSoundFileLoader : public IRegionInfoReadable {
public:
    ~StreamSoundFileLoader() override;
    bool ReadRegionInfo(StreamSoundFile::RegionInfo* pInfo, u32 index) const override;

private:
    fnd::FileStream* mFileStream;
    u32 mSeekBlockOffset;
    u32 mRegionBlockOffset;
    u16 mRegionInfoBytes;
};
}
