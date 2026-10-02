#pragma once

#include <basis/seadTypes.h>
#include <container/seadSafeArray.h>

namespace al {
class AudioMicBreathChecker {
public:
    AudioMicBreathChecker();

    void calc(s16* pSamples, s32 sampleNum);
    void analyzeByBandPass(s16* pSamples, s32 sampleNum);
    void start();
    void stop();
    void analyzeByLowFreq(s16* pSamples, s32 sampleNum);
    void analyzeByZeroCross(s16* pSamples, s32 sampleNum);

    bool isActive() const { return mIsActive; }

    bool isBreath() const { return mIsBreath; }

private:
    void reset() {
        mIsBreath = false;
        mPrevLoudSampleNum = 0;
        mHistory.fill(false);
        mHistoryIndex = 0;
    }

    void pushHistory(bool isBreath) {
        mHistory[mHistoryIndex] = isBreath;

        if (mHistoryIndex + 1 < cHistoryNum) {
            mHistoryIndex++;
        } else {
            mHistoryIndex = 0;
        }
    }

    void updateBreath() {
        s32 count = 0;

        for (s32 i = 0; i < cHistoryNum; i++) {
            count += mHistory[i];
        }

        mIsBreath = count > 3;
    }

    static constexpr s32 cHistoryNum = 10;

    bool mIsActive = false;
    bool mIsBreath = false;
    s32 mPrevLoudSampleNum = 0;
    sead::SafeArray<bool, cHistoryNum> mHistory = {};
    s32 mHistoryIndex = 0;
};

static_assert(sizeof(AudioMicBreathChecker) == 0x18);
}  // namespace al
