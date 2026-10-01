#include "Library/Controller/ReplayController.hpp"

#include "Library/Controller/IUsePadDataReader.hpp"
#include "Library/Controller/IUsePadDataWriter.hpp"
#include "Library/Controller/PadDataPack.hpp"

namespace al {
/**
 * Creates a replay controller wrapping a controller.
 * @param pController controller to wrap
 */
ReplayController::ReplayController(sead::Controller* pController) {
    registerWith(pController, true);
}

/**
 * Unregisters the replay controller from its controller.
 */
void ReplayController::unregist() {
    unregister();
}

/**
 * Starts replaying recorded input.
 */
void ReplayController::startReplay() {
    mIsReplaying = true;
}

/**
 * Pauses replaying recorded input.
 */
void ReplayController::pauseReplay() {
    mIsReplaying = false;
}

/**
 * Ends replaying recorded input and closes the reader.
 */
void ReplayController::endReplay() {
    if (mIsReplaying && mPadDataReader != nullptr) {
        mPadDataReader->close();
    }

    mIsReplaying = false;
}

/**
 * Checks whether recorded input is being replayed.
 * @return whether it is replaying
 */
bool ReplayController::isReplaying() const {
    return mIsReplaying;
}

/**
 * Checks whether input is being recorded.
 * @return whether it is recording
 */
bool ReplayController::isRecording() const {
    return mIsRecording;
}

/**
 * Returns the number of frames left to replay.
 * @return remaining frames, or 0 when not replaying
 */
s32 ReplayController::getReplayRemainFrame() const {
    if (mIsReplaying && mPadDataReader != nullptr) {
        return mPadDataReader->getRemainFrame();
    }

    return 0;
}

/**
 * Updates the input, replacing it with replayed input or recording it.
 * @param prevHold held buttons of the previous frame
 * @param prevPointerOn whether the pointer was on in the previous frame
 */
void ReplayController::calc(u32 prevHold, bool prevPointerOn) {
    mIsReadPadReplayData = false;
    sead::ControllerWrapper::calc(prevHold, prevPointerOn);

    if (mIsReplaying && mPadDataReader != nullptr) {
        PadDataPack frameData;
        mPadDataReader->read(&frameData);
        mPadTrig.setDirect(frameData.trig);
        mPadHold.setDirect(frameData.hold);
        mLeftStick = frameData.leftStick;
        mPointer = frameData.pointer;

        if (mPadDataReader->isEnd()) {
            endReplay();
        }

        mIsReadPadReplayData = true;
    }

    if (mIsRecording && mPadDataWriter != nullptr) {
        PadDataPack frameData;
        frameData.trig = mPadTrig.getDirect();
        frameData.hold = mPadHold.getDirect();
        frameData.leftStick = mLeftStick;
        frameData.pointer = mPointer;
        mPadDataWriter->write(frameData);
    }
}

/**
 * Starts recording input.
 */
void ReplayController::startRecord() {
    if (!mIsRecording && mPadDataWriter != nullptr) {
        mPadDataWriter->open();
    }

    mIsRecording = true;
}

/**
 * Ends recording input.
 */
void ReplayController::endRecord() {
    if (mIsRecording && mPadDataWriter != nullptr) {
        mPadDataWriter->close();
    }

    mIsRecording = false;
}
}  // namespace al
