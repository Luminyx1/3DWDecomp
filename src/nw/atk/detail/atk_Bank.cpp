#include <nn/atk/atk_Bank.h>
#include <nn/atk/atk_WaveFileReader.h>

namespace nn::atk::detail::driver {
/** @brief Constructs a stateless instrument bank driver. */
Bank::Bank() = default;
/** @brief Destroys the stateless bank driver. */
Bank::~Bank() = default;

/**
 * @brief Starts a note using the bank's matching key and velocity region.
 * @param rBank Initialized bank reader supplying instrument regions.
 * @param rWaveArchive Initialized wave archive containing the selected region's sample.
 * @param rInfo Note, channel-allocation, callback and output settings for the new voice.
 * @return Allocated playing channel, or nullptr if region reading, wave reading or allocation fails.
 */
Channel* Bank::NoteOn(const BankFileReader& rBank, const WaveArchiveFileReader& rWaveArchive,
                      const NoteOnInfo& rInfo) const {
    VelocityRegionInfo region;
    if (!rBank.ReadVelocityRegionInfo(&region, rInfo.program, rInfo.key, rInfo.velocity)) {
        return nullptr;
    }
    WaveInfo waveInfo;
    WaveFileReader waveReader(rWaveArchive.GetWaveFile(region.waveId.waveIndex), 0);
    if (!waveReader.ReadWaveInfo(&waveInfo, nullptr)) {
        return nullptr;
    }
    int channelCount = waveInfo.channelCount < 2 ? waveInfo.channelCount : 2;
    Channel* pChannel =
        Channel::AllocChannel(channelCount, rInfo.priority, rInfo.callback, rInfo.pCallbackArgument);
    if (pChannel == nullptr) {
        return nullptr;
    }
    pChannel->SetKey(rInfo.key, region.originalKey);
    pChannel->SetVelocityVolume(CalcChannelVelocityVolume(rInfo.velocity));
    pChannel->SetVolume(CalcChannelVelocityVolume(region.volume));
    pChannel->SetPitch(region.pitch);
    auto& rEnvelope = pChannel->GetEnvelope();
    rEnvelope.SetAttack(region.attack);
    rEnvelope.SetHold(region.hold);
    rEnvelope.SetDecay(region.decay);
    rEnvelope.SetSustain(region.sustain);
    rEnvelope.SetRelease(region.release);
    pChannel->SetPan((rInfo.pan + region.pan - 64) / 63.0f);
    pChannel->SetKeyGroup(region.keyGroup);
    pChannel->SetIgnoreNoteOff(region.ignoreNoteOff);
    pChannel->SetInterpolationType(region.interpolationType);
    pChannel->SetUpdateType(rInfo.updateType);
    pChannel->SetOutputReceiver(rInfo.pOutputReceiver);
    pChannel->Start(waveInfo, rInfo.length, 0, false);
    return pChannel;
}

/**
 * @brief Converts a seven-bit note velocity or region volume to linear gain.
 * @param velocity Nominal velocity in [0, 127]; the original conversion also accepts other byte values.
 * @return Linear gain, with 127 corresponding to unity.
 */
float Bank::CalcChannelVelocityVolume(u8 velocity) { return velocity * (1.0f / 127.0f); }
} // namespace nn::atk::detail::driver
