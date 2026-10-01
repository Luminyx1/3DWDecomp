#include <nn/atk/atk_WaveFileReader.h>

namespace nn::atk::detail {
// file supplies the wave resource; type 0 selects FWAV and type 1 selects raw DSP ADPCM.
WaveFileReader::WaveFileReader(const void* file, s8 type)
    : mHeader(nullptr), mInfo(nullptr), mData(nullptr), mDsp(), mType(type) {
    switch (type) {
    case 0: {
        auto* header = static_cast<const WaveFile::FileHeader*>(file);

        if (header->signature != 0x56415746 || header->byteOrder != 0xfeff ||
            header->version < 0x10000 || header->version > 0x10200) return;
        mHeader = header;
        const auto* info = mHeader->GetInfoBlock();
        const auto* data = mHeader->GetDataBlock();

        if (info == nullptr || data == nullptr || info->signature != 0x4f464e49 || data->signature != 0x41544144) return;
        mInfo = &info->body;
        mData = data->data;
        break;
    }
    case 1:
        mDsp.mHeader = static_cast<const DspadpcmHeader*>(file);
        break;
    }
}

bool WaveFileReader::IsOriginalLoopAvailable() const { return mHeader->version >= 0x10200; }
// info receives playback metadata; waveData is unused because this build reads embedded sample data.
// As in the original, unsupported types leave info untouched and still report success.
bool WaveFileReader::ReadWaveInfo(WaveInfo* info, const void* waveData) const {
    switch (mType) {
    case 0: {
        u32 format = GetSampleFormat(mInfo->sampleFormat);
        int channelCount = mInfo->channelCount;
        info->sampleFormat = format;
        info->channelCount = channelCount;
        info->sampleRate = mInfo->sampleRate;
        info->loop = mInfo->loop == 1;
        info->loopStart = mInfo->loopStart;
        info->loopEnd = mInfo->loopEnd;
        info->dataSize = mHeader->fileSize - size_t(0x20);
        info->originalLoopStart = IsOriginalLoopAvailable() ? mInfo->originalLoopStart : mInfo->loopStart;

        for (int i = 0; i < channelCount; ++i) {
            // Only the first two channel records are populated.
            if (i >= 2) continue;
            const auto* channel = mInfo->GetChannelInfo(i);

            if (channel->adpcm.offset) {
                const auto* adpcm = channel->GetDspAdpcmInfo();
                info->channels[i].adpcm = adpcm->adpcm;
                info->channels[i].loopContext = adpcm->loopContext;
            }

            info->channels[i].samples = channel->GetSamplesAddress(mData);
            const auto* last = mInfo->GetChannelInfo(channelCount - 1);
            info->channels[i].size = mHeader->GetDataBlock()->size - 8 - last->samples.offset;
        }

        break;
    }
    case 1:
        mDsp.ReadWaveInfo(info);
        break;
    }

    return true;
}

// channel provides the sample offset; waveData is unused in favor of the reader's stored data base.
const void* WaveFileReader::GetWaveDataAddress(const WaveFile::ChannelInfo* channel, const void* waveData) const {
    return channel->GetSamplesAddress(mData);
}

DspadpcmReader::DspadpcmReader() : mHeader(nullptr) {}
// info receives the mono DSP ADPCM stream's format, sample address, coefficients, and initial context.
bool DspadpcmReader::ReadWaveInfo(WaveInfo* info) const {
    const auto* header = mHeader;
    info->sampleFormat = 2;
    info->loop = false;
    info->channelCount = 1;
    info->sampleRate = header->sampleRate;
    info->loopStart = 0;
    info->loopEnd = header->sampleCount;
    info->channels[0].samples = mHeader + 1;
    info->channels[0].adpcm.parameter = header->parameter;
    info->channels[0].adpcm.context = header->context;
    return true;
}
}
