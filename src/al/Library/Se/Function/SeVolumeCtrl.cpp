#include "Library/Se/Function/SeVolumeCtrl.hpp"

#include <attributes.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Library/Se/Project/SeRequest.hpp"
#include "Project/Audio/System/AudioPlayer.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
const SeVolumeSetting cVolumeSettings[] = {
    {"通常", {1.0f, 1.0f, 0.75f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"コースセレクトデモ", {0.2f, 0.2f, 0.2f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"コースセレクトコースイン", {0.0f, 0.0f, 0.2f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"ステージデモ", {1.0f, 1.0f, 0.3f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"ステージ開始デモ", {0.3f, 0.3f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"景観ポイント", {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"土管入り", {0.0f, 0.0f, 0.3f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}},
    {"ファンファーレ", {0.4f, 0.4f, 0.4f, 0.4f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"エンディング後デモ", {0.0f, 0.0f, 0.75f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"エンディング後デモワイプ", {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f}},
    {"ハリーアップ", {1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 0.25f, 1.0f}},
    {"ステージ終了", {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"TitleVolumes", {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f}},
    {"AtmosphereDemo", {0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}},
    {"SystemAtmosphereDemo", {0.0f, 0.0f, 0.4f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"SystemDemo", {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"SystemVoiceDemo", {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {"DisasterBgmPhase0", {1.0f, 1.0f, 0.525f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
};

/**
 * @brief Finds a named preset of sound-player volume multipliers.
 * @param pName Name of the requested volume preset.
 * @return Matching preset, or nullptr if the name is unknown.
 */
NOINLINE const SeVolumeSetting* findVolumeSetting(const char* pName) {
    for (s32 i = 0; i < 18; i++) {
        if (isEqualString(pName, cVolumeSettings[i].mName)) {
            return &cVolumeSettings[i];
        }
    }
    return nullptr;
}

/**
 * @brief Advances a volume toward its target and clamps it to the supported range.
 * @param current Current volume multiplier.
 * @param target Target volume multiplier.
 * @param frames Positive number of frames used to calculate the step.
 * @return Updated multiplier, clamped to [0, 2] when changed.
 */
inline f32 advanceVolume(f32 current, f32 target, const s32& frames) {
    if (current == target) {
        return current;
    }
    f32 frameCount = frames;
    f32 difference = target - current;
    f32 distance = difference > 0.0f ? difference : -difference;
    current = converge(current, target, distance / frameCount);
    if (current < 0.0f) {
        current = 0.0f;
    } else if (current > 2.0f) {
        current = 2.0f;
    }
    return current;
}
} // namespace

/**
 * @brief Creates a volume controller using the normal preset.
 * @param pRequests Non-null list of active requests to update.
 * @param pPlayer Non-null player used to read sound-archive metadata.
 * @param pName Unused; the initial preset is always the normal preset.
 */
SeVolumeCtrl::SeVolumeCtrl(RequestList* pRequests, SeadAudioPlayer* pPlayer, const char* pName)
    : mRequests(pRequests), mPlayer(pPlayer) {
    mVolumes = new f32[8];
    mBaseVolumes = new f32[8];
    for (s32 i = 0; i < 8; i++) {
        mVolumes[i] = 1.0f;
        mBaseVolumes[i] = 1.0f;
    }
    mSetting = findVolumeSetting("通常");
}

/**
 * @brief Applies the preset multiplier assigned to a sound-archive player.
 * @param pRequest Non-null request whose volume is updated.
 * @param playerId Sound-archive player identifier used to select a volume group.
 * @param isAfterGoal State forwarded to the request; currently unused by its volume calculation.
 */
ALWAYS_INLINE inline void SeVolumeCtrl::applyPlayerVolume(SeRequest* pRequest, u32 playerId,
                                                          bool isAfterGoal) const {
    switch (playerId) {
    case 0x04000001:
    case 0x04000006:
        pRequest->applyVolume(isAfterGoal, mVolumes[5]);
        return;
    case 0x04000008:
    case 0x0400000a:
        pRequest->applyVolume(isAfterGoal, mVolumes[6]);
        return;
    case 0x04000003:
        pRequest->applyVolume(isAfterGoal, mVolumes[1]);
        return;
    case 0x04000004:
        pRequest->applyVolume(isAfterGoal, mVolumes[0]);
        return;
    case 0x04000005:
        pRequest->applyVolume(isAfterGoal, mVolumes[2]);
        return;
    case 0x04000007:
        pRequest->applyVolume(isAfterGoal, mVolumes[4]);
        return;
    default:
        if (playerId == 0x0400000b) {
            pRequest->applyVolume(isAfterGoal, mVolumes[0]);
            return;
        } else {
            pRequest->applyVolume(isAfterGoal, mVolumes[3]);
            return;
        }
    }
}

/**
 * @brief Updates preset volumes and applies the appropriate sound-player volume to each request.
 * @param isAfterGoal State forwarded to each request's volume calculation; currently unused there.
 */
void SeVolumeCtrl::update(bool isAfterGoal) {
    if (mSetting != nullptr) {
        if (mFadeFrames > 0) {
            for (s32 i = 0; i < 8; i++) {
                mVolumes[i] = advanceVolume(mVolumes[i], mSetting->mVolumes[i], mFadeFrames);
            }
        } else {
            for (s32 i = 0; i < 8; i++) {
                mVolumes[i] = mSetting->mVolumes[i];
            }
        }
    }
    for (auto it = mRequests->begin(); it != mRequests->end(); ++it) {
        SoundInfo info;
        if (mPlayer->readSoundInfo(&info, it->getSoundId())) {
            applyPlayerVolume(&*it, info.playerId, isAfterGoal);
            f32 titleVolume = mVolumes[7];
            if (titleVolume != 1.0f && it->getSpecificInfo() != nullptr &&
                it->getSpecificInfo()->mIsIgnoreInTitleScene) {
                it->applyVolume(isAfterGoal, titleVolume);
            }
        } else {
            it->applyVolume(isAfterGoal, 1.0f);
        }
    }
}

/**
 * @brief Selects a named volume preset and its transition duration.
 * @param pName Preset name; unknown names leave the current preset unchanged.
 * @param frames Transition duration, converted to an integer number of frames.
 */
void SeVolumeCtrl::setVolumeSetting(const char* pName, f32 frames) {
    const SeVolumeSetting* pSetting = findVolumeSetting(pName);
    if (pSetting != nullptr) {
        mSetting = pSetting;
        mFadeFrames = frames;
    }
}
} // namespace al
