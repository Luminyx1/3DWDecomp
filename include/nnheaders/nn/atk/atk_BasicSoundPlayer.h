#pragma once

#include <nn/atk/atk_OutputParam.h>
#include <nn/os.h>

namespace nn::atk {
class OutputReceiver;
}  // namespace nn::atk
namespace nn::atk::detail {
class OutputAdditionalParam;
}  // namespace nn::atk::detail

namespace nn::atk::detail::driver {
/**
 * @brief Driver-side base of every sound player: activity flags and the base output parameters.
 */
class BasicSoundPlayer {
public:
    BasicSoundPlayer();
    /** @brief Releases the player's event. */
    virtual ~BasicSoundPlayer() { os::FinalizeEvent(&mEvent); }
    virtual void Initialize(OutputReceiver* pReceiver);
    virtual void Finalize();
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Pause(bool isPause) = 0;
    /**
     * @brief Marks the player as holding live data.
     * @param isActive Whether the player is active.
     */
    virtual void SetActiveFlag(bool isActive) { mActiveFlag = isActive; }

    /** @brief Tests whether the player holds live data. @return Whether the player is active. */
    bool IsActive() const { return mActiveFlag; }
    /** @brief Tests whether playback has started. @return Whether the player is started. */
    bool IsStarted() const { return mStartedFlag; }
    /** @brief Tests whether the player is paused. @return Whether the player is paused. */
    bool IsPause() const { return mPauseFlag; }

protected:
    os::EventType mEvent;
    OutputReceiver* mOutputReceiver;
    bool mActiveFlag;
    bool mStartedFlag;
    bool mPauseFlag;
    bool mFinishFlag;
    bool mIsFinalizedForCannotAllocateResource;
    u8 _3d[3];
    float mBaseVolume;
    float mBasePitch;
    float mBaseLpfFreq;
    float mBaseBiquadValue;
    u8 mBaseBiquadType;
    u8 _51[3];
    int mPanMode;
    int mPanCurve;
    int mBaseOutputLine;
    OutputParam mTvParam;
    OutputAdditionalParam* mTvAdditionalParam;
    u8 _b8[8];
};
static_assert(sizeof(BasicSoundPlayer) == 0xc0, "BasicSoundPlayer size");
}  // namespace nn::atk::detail::driver
