#include <nn/atk/atk_StreamSoundPrefetchFileReader.h>
#include <cstring>

namespace nn::atk::detail {
StreamSoundPrefetchFileReader::StreamSoundPrefetchFileReader() { Finalize(); }
StreamSoundPrefetchFileReader::~StreamSoundPrefetchFileReader() { Finalize(); }
void StreamSoundPrefetchFileReader::Finalize() {
    mHeader = nullptr;
    mInfo = nullptr;
    mPrefetch = nullptr;
    mRegionOffset = 0;
    mRegionInfoBytes = 0;
}
// file is an FSTP resource containing INFO and PDAT blocks; invalid headers preserve prior state.
void StreamSoundPrefetchFileReader::Initialize(const void* file) {
    if (!IsValidFileHeader(file)) return;
    mHeader = static_cast<const StreamSoundPrefetchFile::FileHeader*>(file);
    const auto* info = mHeader->GetInfoBlock();
    if (info->signature != 0x4f464e49) return;
    const auto* prefetch = mHeader->GetPrefetchDataBlock();
    if (prefetch->signature != 0x54414450) return;
    mInfo = &info->body;
    mPrefetch = &prefetch->body;
    mRegionOffset = GetRegionDataOffset();
    mRegionInfoBytes = GetRegionInfoBytes();
}
// file points to a readable header whose signature, byte order and version are checked.
bool StreamSoundPrefetchFileReader::IsValidFileHeader(const void* file) const {
    const auto* header = static_cast<const StreamSoundPrefetchFile::FileHeader*>(file);
    return header->signature == 0x50545346 && header->byteOrder == 0xfeff &&
        header->version >= 0x10000 && header->version <= 0x50200;
}
u32 StreamSoundPrefetchFileReader::GetRegionDataOffset() const {
    if (!mHeader || !mHeader->HasRegionBlock()) return 0;
    u32 offset = mHeader->GetRegionBlockOffset();
    return offset + mInfo->GetStreamSoundInfo()->regionData.offset + 8;
}
u16 StreamSoundPrefetchFileReader::GetRegionInfoBytes() const { return mInfo->GetStreamSoundInfo()->regionInfoSize; }
bool StreamSoundPrefetchFileReader::IsIncludeRegionInfo() const { return mHeader->version >= 0x30000; }
bool StreamSoundPrefetchFileReader::IsCrc32CheckAvailable() const { return mHeader->version >= 0x40000; }
bool StreamSoundPrefetchFileReader::IsRegionIndexCheckAvailable() const { return mHeader->version >= 0x50100; }
// info receives the complete stream settings stored in the prefetch resource.
bool StreamSoundPrefetchFileReader::ReadStreamSoundInfo(StreamSoundFile::StreamSoundInfo* info) const {
    *info = *mInfo->GetStreamSoundInfo();
    return true;
}
// param and loop receive decoder and loop contexts for a valid channel; absent ADPCM data returns false.
bool StreamSoundPrefetchFileReader::ReadDspAdpcmChannelInfo(DspAdpcmParam* param, DspAdpcmLoopParam* loop, int channel) const {
    const auto* source = mInfo->GetChannelInfoTable()->GetChannelInfo(channel)->GetDspAdpcmChannelInfo();
    if (!source) return false;
    *param = source->param;
    *loop = source->loop;
    return true;
}
// info receives the prefetch range and samples; index must identify a valid table entry.
bool StreamSoundPrefetchFileReader::ReadPrefetchDataInfo(PrefetchDataInfo* info, int index) const {
    const auto* source = &mPrefetch->entries[static_cast<u32>(index)];
    info->startFrame = source->startFrame;
    info->sampleBytes = source->sampleBytes;
    info->samples = source->GetPrefetchSample()->GetSampleAddress();
    return true;
}
// info receives a fixed-size region record; index selects a record using the stored byte stride.
// Missing region offset or stride returns false; the caller must supply a valid index otherwise.
bool StreamSoundPrefetchFileReader::ReadRegionInfo(StreamSoundFile::RegionInfo* info, u32 index) const {
    if (!mRegionOffset || !mRegionInfoBytes) return false;
    size_t offset = mRegionOffset + size_t(mRegionInfoBytes) * index;
    std::memcpy(info, reinterpret_cast<const u8*>(mHeader) + offset, sizeof(*info));
    return true;
}

}
