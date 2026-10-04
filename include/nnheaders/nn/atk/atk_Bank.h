#pragma once

#include <nn/atk/atk_BankFileReader.h>
#include <nn/atk/atk_Channel.h>
#include <nn/atk/atk_WaveArchiveFileReader.h>

namespace nn::atk::detail::driver {
struct NoteOnInfo {
    int program;
    int key;
    int velocity;
    int length;
    int pan;
    int priority;
    Channel::ChannelCallback callback;
    void* pCallbackArgument;
    OutputReceiver* pOutputReceiver;
    UpdateType updateType;
};
static_assert(sizeof(NoteOnInfo) == 0x38, "NoteOnInfo size");

class Bank {
  public:
    Bank();
    ~Bank();
    Channel* NoteOn(const BankFileReader& rBank, const WaveArchiveFileReader& rWaveArchive,
                    const NoteOnInfo& rInfo) const;
    static float CalcChannelVelocityVolume(u8 velocity);
};
} // namespace nn::atk::detail::driver
