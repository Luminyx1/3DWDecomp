#include <nn/atk/atk_StreamSoundFileReader.h>

namespace nn::atk::detail {
// file points to an FSTM resource. Failed checks preserve the previous info pointer.
void StreamSoundFileReader::Initialize(const void* file) {
    if (!IsValidFileHeader(file)) return;
    mHeader = static_cast<const StreamSoundFile::FileHeader*>(file);
    const auto* block = static_cast<const u8*>(file) + mHeader->GetInfoBlockOffset();
    if (*reinterpret_cast<const u32*>(block) != 0x4f464e49) return;
    mInfo = reinterpret_cast<const StreamSoundFile::InfoBlockBody*>(block + 8);
}
// file points to a readable binary header whose signature, byte order and version are checked.
bool StreamSoundFileReader::IsValidFileHeader(const void* file) {
    const auto* header = static_cast<const StreamSoundFile::FileHeader*>(file);
    return header->signature == 0x4d545346 && header->byteOrder == 0xfeff &&
        header->version >= 0x10000 && header->version <= 0x60300;
}
void StreamSoundFileReader::Finalize() { mHeader = nullptr; mInfo = nullptr; }
bool StreamSoundFileReader::IsTrackInfoAvailable() const { return mHeader->version <= 0x20000; }
bool StreamSoundFileReader::IsOriginalLoopAvailable() const { return IsOriginalLoopAvailableImpl(mHeader); }
// header supplies the format version used to decide whether original loop positions exist.
bool StreamSoundFileReader::IsOriginalLoopAvailableImpl(const StreamSoundFile::FileHeader* header) {
    return header->version >= 0x40000;
}
bool StreamSoundFileReader::IsCrc32CheckAvailable() const { return mHeader->version >= 0x50000; }
bool StreamSoundFileReader::IsRegionIndexCheckAvailable() const { return mHeader->version >= 0x60100; }
// info receives the stream settings, with version-dependent loop and checksum defaults.
bool StreamSoundFileReader::ReadStreamSoundInfo(StreamSoundFile::StreamSoundInfo* info) const {
    const auto* source = mInfo->GetStreamSoundInfo();
    info->sampleFormat = source->sampleFormat;
    info->loop = source->loop;
    info->channelCount = source->channelCount;
    info->regionCount = source->regionCount;
    info->sampleRate = source->sampleRate;
    info->loopStart = source->loopStart;
    info->loopEnd = source->loopEnd;
    info->blockCount = source->blockCount;
    info->blockSize = source->blockSize;
    info->blockSampleCount = source->blockSampleCount;
    info->lastBlockSize = source->lastBlockSize;
    info->lastBlockSampleCount = source->lastBlockSampleCount;
    info->lastBlockPaddedSize = source->lastBlockPaddedSize;
    info->seekInfoSize = source->seekInfoSize;
    info->seekIntervalSamples = source->seekIntervalSamples;
    info->sampleData = source->sampleData;
    info->regionInfoSize = source->regionInfoSize;
    info->regionData = source->regionData;
    if (IsOriginalLoopAvailable()) {
        info->originalLoopStart = source->originalLoopStart;
        info->originalLoopEnd = source->originalLoopEnd;
    } else {
        info->originalLoopStart = source->loopStart;
        info->originalLoopEnd = source->loopEnd;
    }
    info->crc32 = IsCrc32CheckAvailable() ? source->crc32 : 0;
    return true;
}
// info receives track settings and at most two channel indices; track selects a table entry.
// A missing table or track beyond its signed count returns false.
bool StreamSoundFileReader::ReadStreamTrackInfo(TrackInfo* info, int track) const {
    const auto* table = mInfo->GetTrackInfoTable();
    if (!table || track >= static_cast<int>(table->count)) return false;
    const auto* source = table->GetTrackInfo(track);
    info->volume = source->volume;
    info->pan = source->pan;
    info->surroundPan = source->surroundPan;
    info->_03 = source->_03;
    info->channelCount = source->GetChannelIndices()->count;
    unsigned count = info->channelCount < 2 ? info->channelCount : 2;
    for (unsigned i = 0; i < count; ++i) info->channels[i] = source->GetChannelIndices()->indices[i];
    return true;
}
// param and loop receive the decoder and loop contexts for the valid channel index.
// A channel without DSP ADPCM information returns false without writing either output.
bool StreamSoundFileReader::ReadDspAdpcmChannelInfo(DspAdpcmParam* param, DspAdpcmLoopParam* loop, int channel) const {
    const auto* source = mInfo->GetChannelInfoTable()->GetChannelInfo(channel)->GetDspAdpcmChannelInfo();
    if (!source) return false;
    *param = source->param;
    *loop = source->loop;
    return true;
}
}
