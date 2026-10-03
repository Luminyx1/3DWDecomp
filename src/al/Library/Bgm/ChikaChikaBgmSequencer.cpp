#include "Library/Bgm/ChikaChikaBgmSequencer.hpp"

#include <prim/seadSafeString.h>

#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Audio/System/AudioKeeperFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"

namespace {
/** Number of beats in a measure. */
constexpr s32 cBeatNumPerMeasure = 4;

/** Number of frames in a beat. */
constexpr s32 cFrameNumPerBeat = 60;

/** Number of frames between two sound effect slots. */
constexpr s32 cFrameNumPerSlot = 30;

/** Number of sound effect slots in a measure. */
constexpr s32 cSlotNum = 8;

/** Number of measures in the sequence. */
constexpr s32 cMeasureNum = 22;

/** Number of intro measures that are not repeated. */
constexpr s32 cIntroMeasureNum = 2;

/** Number of measures in the repeated part of the sequence. */
constexpr s32 cLoopMeasureNum = 20;

/**
 * Sound effects of one measure of a part, one per slot.
 */
struct SeMeasure {
    const char* names[cSlotNum];
};

// The measure templates and sequences are referenced by their original data labels.
// clang-format off
const SeMeasure cNone = {};
SeMeasure sDsMeasure asm("lbl_7101AFDF70") = {
    {"Ds1", "Ds2", "Ds1", "Ds2", "Ds1", "Ds2", "Ds1", "Ds2"}};
SeMeasure sBa1Measure asm("lbl_7101AFDFB0") = {
    {"Ba1", "Ba2", "Ba3", "Ba4", "Ba1", "Ba2", "Ba3", "Ba4"}};
SeMeasure sBa2Measure asm("lbl_7101AFDFF0") = {
    {"Ba5", "Ba6", "Ba7", "Ba8", "Ba5", "Ba6", "Ba7", "Ba8"}};
SeMeasure sBa3Measure asm("lbl_7101AFE030") = {
    {"Ba5", "Ba6", "Ba7", "Ba8", "Ba9", "Ba10", "Ba11", "Ba12"}};
SeMeasure sBa4Measure asm("lbl_7101AFE070") = {
    {"Ba5", "Ba6", "Ba7", "Ba13", "Ba14", "Ba15", "Ba16", "Ba17"}};
SeMeasure sBa5Measure asm("lbl_7101AFE0B0") = {
    {"Ba5", "Ba6", "Ba7", "Ba8", "Ba18", "Ba19", "Ba20", "Ba21"}};
SeMeasure sGt1Measure asm("lbl_7101AFE0F0") = {
    {"Gt1", "Gt2", "Gt3", "Gt4", "Gt5", nullptr, nullptr, nullptr}};
SeMeasure sGt2Measure asm("lbl_7101AFE130") = {
    {"Gt6", "Gt7", "Gt8", "Gt9", "Gt10", nullptr, nullptr, nullptr}};
SeMeasure sGtLMeasure asm("lbl_7101AFE170") = {
    {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "GtL1"}};
SeMeasure sSt1Measure asm("lbl_7101AFE1B0") = {
    {"St1", "St2", "St3", "St4", nullptr, nullptr, nullptr, nullptr}};
SeMeasure sSt2Measure asm("lbl_7101AFE1F0") = {
    {"St1", "St2", "St3", "St5", "St6", "St7", nullptr, nullptr}};
SeMeasure sSq1Measure asm("lbl_7101AFE230") = {
    {"Sq1", "Sq2", "Sq3", "Sq4", "Sq1", "Sq2", "Sq3", "Sq4"}};
SeMeasure sSq2Measure asm("lbl_7101AFE270") = {
    {"Sq5", "Sq6", "Sq7", "Sq8", "Sq5", "Sq6", "Sq7", "Sq8"}};
SeMeasure sPd1Measure asm("lbl_7101AFE2B0") = {
    {"Pd1", nullptr, "Pd2", nullptr, "Pd3", nullptr, nullptr, nullptr}};
SeMeasure sPd2Measure asm("lbl_7101AFE2F0") = {
    {"Pd4", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};
SeMeasure sPd3Measure asm("lbl_7101AFE330") = {
    {"Pd5", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};
SeMeasure sPd4Measure asm("lbl_7101AFE370") = {
    {"Pd6", nullptr, "Pd7", nullptr, "Pd8", nullptr, nullptr, nullptr}};
SeMeasure sPd5Measure asm("lbl_7101AFE3B0") = {
    {"Pd9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};
SeMeasure sPd6Measure asm("lbl_7101AFE3F0") = {
    {"Pd10", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};

SeMeasure sDsSequence[cMeasureNum] asm("lbl_7102245B88") = {
    sDsMeasure,   sDsMeasure,   sDsMeasure,   sDsMeasure,   sDsMeasure,   sDsMeasure,
    sDsMeasure,   sDsMeasure,   sDsMeasure,   sDsMeasure,   sDsMeasure,   sDsMeasure,
    sDsMeasure,   sDsMeasure,   sDsMeasure,   sDsMeasure,   sDsMeasure,   sDsMeasure,
    sDsMeasure,   sDsMeasure,   sDsMeasure,   sDsMeasure,
};
SeMeasure sBaSequence[cMeasureNum] = {
    sBa1Measure,  sBa1Measure,  sBa1Measure,  sBa1Measure,  sBa1Measure,  sBa1Measure,
    sBa1Measure,  sBa1Measure,  sBa1Measure,  sBa1Measure,  sBa2Measure,  sBa3Measure,
    sBa2Measure,  sBa4Measure,  sBa2Measure,  sBa3Measure,  sBa2Measure,  sBa5Measure,
    cNone,        cNone,        sBa1Measure,  sBa1Measure,
};
SeMeasure sGtSequence[cMeasureNum] asm("lbl_7102246688") = {
    cNone,        cNone,        cNone,        cNone,        cNone,        cNone,
    cNone,        sGt1Measure,  cNone,        sGt1Measure,  cNone,        sGt2Measure,
    cNone,        sGt2Measure,  cNone,        sGt2Measure,  cNone,        sGt2Measure,
    cNone,        cNone,        cNone,        cNone,
};
SeMeasure sGtLSequence[cMeasureNum] = {
    cNone,        cNone,        cNone,        cNone,        cNone,        sGtLMeasure,
    sGtLMeasure,  sGtLMeasure,  sGtLMeasure,  cNone,        cNone,        cNone,
    cNone,        cNone,        cNone,        cNone,        cNone,        sGtLMeasure,
    sGtLMeasure,  sGtLMeasure,  sGtLMeasure,  cNone,
};
SeMeasure sStSequence[cMeasureNum] asm("lbl_7102247188") = {
    cNone,        cNone,        cNone,        cNone,        cNone,        cNone,
    cNone,        cNone,        cNone,        cNone,        sSt1Measure,  cNone,
    sSt2Measure,  cNone,        sSt1Measure,  cNone,        sSt2Measure,  cNone,
    cNone,        cNone,        cNone,        cNone,
};
SeMeasure sSqSequence[cMeasureNum] = {
    sSq1Measure,  sSq1Measure,  sSq1Measure,  sSq1Measure,  sSq1Measure,  sSq1Measure,
    sSq1Measure,  sSq1Measure,  sSq1Measure,  sSq1Measure,  sSq2Measure,  sSq2Measure,
    sSq2Measure,  sSq2Measure,  sSq2Measure,  sSq2Measure,  sSq2Measure,  sSq2Measure,
    sSq1Measure,  sSq1Measure,  sSq1Measure,  sSq1Measure,
};
SeMeasure sPdSequence[cMeasureNum] asm("lbl_7102245608") = {
    cNone,        cNone,        sPd1Measure,  sPd2Measure,  sPd1Measure,  sPd3Measure,
    cNone,        cNone,        cNone,        cNone,        cNone,        cNone,
    cNone,        cNone,        sPd4Measure,  sPd5Measure,  sPd4Measure,  sPd6Measure,
    cNone,        cNone,        cNone,        cNone,
};
// clang-format on

/**
 * Starts a sound effect if one is set.
 * @param pUser Audio keeper user.
 * @param pName Name of the sound effect, or nullptr.
 */
inline void startSeIfExist(const al::IUseAudioKeeper* pUser, const char* pName) {
    if (pName != nullptr) {
        al::startSe(pUser, pName);
    }
}
}  // namespace

namespace al {
/**
 * Constructs a sequencer.
 */
ChikaChikaBgmSequencer::ChikaChikaBgmSequencer() = default;

/**
 * Creates the audio keeper.
 * @param rInfo Actor init info.
 */
void ChikaChikaBgmSequencer::init(ActorInitInfo& rInfo) {
    mAudioKeeper = createAudioKeeper("ChikaChikaBgmSequencer", rInfo);
}

/**
 * Starts the sound effects of the current slot.
 * @param frame Frame in the measure, or in the beat if counting beats.
 * @param measure Current measure, or current beat if counting beats.
 */
void ChikaChikaBgmSequencer::update(s32 frame, s32 measure) {
    if (mIsBeatCount) {
        frame += (measure % cBeatNumPerMeasure) * cFrameNumPerBeat;
        measure /= cBeatNumPerMeasure;
    }

    if (measure > cIntroMeasureNum) {
        measure = (measure - cIntroMeasureNum) % cLoopMeasureNum + cIntroMeasureNum;
    }

    for (s32 i = 0; i < cSlotNum; i++) {
        if (frame == i * cFrameNumPerSlot) {
            startSeIfExist(this, sDsSequence[measure].names[i]);
            startSeIfExist(this, sBaSequence[measure].names[i]);
            startSeIfExist(this, sGtSequence[measure].names[i]);
            startSeIfExist(this, sGtLSequence[measure].names[i]);
            startSeIfExist(this, sStSequence[measure].names[i]);
            startSeIfExist(this, sSqSequence[measure].names[i]);
            startSeIfExist(this, sPdSequence[measure].names[i]);
        }
    }

    mIsPlaying = true;
    mAudioKeeper->update();
}

/**
 * Stops the sound effects and plays the delay tail while the scene is stopped.
 */
void ChikaChikaBgmSequencer::updateSceneStop() {
    if (mIsPlaying) {
        mStopDelayFrame = 40;
        mIsPlaying = false;
        stopAllSeFromUser(this, 0);
    }

    if (mStopDelayFrame >= 0) {
        if (mStopDelayFrame % 10 == 0) {
            startSeWithParam(this, "Delay", mStopDelayFrame / 10);
        }

        if (mStopDelayFrame % 10 == 5) {
            stopSeByName(this, "Delay");
        }

        mStopDelayFrame--;
    }
}

/**
 * Stops all sound effects.
 */
void ChikaChikaBgmSequencer::stopAll() {
    stopAllSeFromUser(this, 0);
}
}  // namespace al
