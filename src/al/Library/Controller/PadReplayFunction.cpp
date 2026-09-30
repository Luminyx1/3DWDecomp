#include "Library/Controller/PadReplayFunction.hpp"

#include <controller/seadControllerMgr.h>

#include "Library/Controller/ReplayController.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Controller/PadDataArcReader.hpp"

namespace al {
namespace {
inline ReplayController* findReplayController(u32 port) {
    sead::Controller* controller = sead::ControllerMgr::instance()->getControllerUnsafe(port);
    return controller->getWrapperAs<ReplayController*>();
}
}  // namespace

/**
 * Creates a replay controller for a port.
 * @param port controller port
 */
void createReplayController(u32 port) {
    new ReplayController(sead::ControllerMgr::instance()->getController(port));
}

/**
 * Unregisters the replay controller of a port.
 * @param port controller port
 */
void unregistReplayController(u32 port) {
    findReplayController(port)->unregist();
}

/**
 * Returns the replay controller of a port.
 * @param port controller port
 * @return the replay controller, or nullptr
 */
ReplayController* getReplayController(u32 port) {
    return findReplayController(port);
}

/**
 * Sets the reader of recorded input of a port.
 * @param pReader pad data reader
 * @param port controller port
 */
void setPadDataReader(IUsePadDataReader* pReader, u32 port) {
    findReplayController(port)->setPadDataReader(pReader);
}

/**
 * Creates a reader for recorded input in an archive and sets it for a port.
 * @param pArchiveName archive name
 * @param pFileName base name of the recorded input
 * @param port controller port
 */
void createAndSetPadDataArcReader(const char* pArchiveName, const char* pFileName, u32 port) {
    StringTmp<64> resourceName;
    resourceName.format("%s_port%d", pFileName, port == 1 ? 5 : port - 1);
    auto* reader = new PadDataArcReader(pArchiveName, resourceName.cstr());
    findReplayController(port)->setPadDataReader(reader);
}

/**
 * Starts replaying recorded input on a port.
 * @param port controller port
 */
void startPadReplay(u32 port) {
    findReplayController(port)->startReplay();
}

/**
 * Pauses replaying recorded input on a port.
 * @param port controller port
 */
void pausePadReplay(u32 port) {
    findReplayController(port)->pauseReplay();
}

/**
 * Ends replaying recorded input on a port.
 * @param port controller port
 */
void endPadReplay(u32 port) {
    findReplayController(port)->endReplay();
}

/**
 * Sets the writer for recording input of a port.
 * @param pWriter pad data writer
 * @param port controller port
 */
void setPadDataWriter(IUsePadDataWriter* pWriter, u32 port) {
    findReplayController(port)->setPadDataWriter(pWriter);
}

/**
 * Starts recording input on a port.
 * @param port controller port
 */
void startPadRecording(u32 port) {
    findReplayController(port)->startRecord();
}

/**
 * Ends recording input on a port.
 * @param port controller port
 */
void endPadRecording(u32 port) {
    findReplayController(port)->endRecord();
}

/**
 * Checks whether recorded input is replayed on a port.
 * @param port controller port
 * @return whether it is replaying
 */
bool isPadReplaying(u32 port) {
    return findReplayController(port)->isReplaying();
}

/**
 * Returns the number of frames left to replay on a port.
 * @param port controller port
 * @return remaining frames
 */
s32 getPadReplayRemainFrame(u32 port) {
    return findReplayController(port)->getReplayRemainFrame();
}

/**
 * Checks whether input is recorded on a port.
 * @param port controller port
 * @return whether it is recording
 */
bool isPadRecording(u32 port) {
    ReplayController* controller = findReplayController(port);
    if (!controller) {
        return false;
    }
    return controller->isRecording();
}

/**
 * Disables replaying on a port.
 * @param port controller port
 */
void invalidatePadReplay(u32 port) {
    findReplayController(port)->setValidPadReplay(false);
}

/**
 * Enables replaying on a port.
 * @param port controller port
 */
void validatePadReplay(u32 port) {
    findReplayController(port)->setValidPadReplay(true);
}

/**
 * Checks whether a port has a replay controller that is enabled.
 * @param port controller port
 * @return whether the replay controller is valid
 */
bool isValidReplayController(u32 port) {
    ReplayController* controller = findReplayController(port);
    if (!controller) {
        return false;
    }
    return controller->isValidPadReplay();
}

/**
 * Checks whether replayed input was read on a port this frame.
 * @param port controller port
 * @return whether replayed input was read
 */
bool isReadPadReplayData(u32 port) {
    ReplayController* controller = findReplayController(port);
    if (!controller) {
        return false;
    }
    return controller->isReadPadReplayData();
}
}  // namespace al
