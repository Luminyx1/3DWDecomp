#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUsePadDataReader;
class IUsePadDataWriter;
class ReplayController;

void createReplayController(u32 port);
void unregistReplayController(u32 port);
ReplayController* getReplayController(u32 port);
void setPadDataReader(IUsePadDataReader* pReader, u32 port);
void createAndSetPadDataArcReader(const char* pArchiveName, const char* pFileName, u32 port);
void startPadReplay(u32 port);
void pausePadReplay(u32 port);
void endPadReplay(u32 port);
void setPadDataWriter(IUsePadDataWriter* pWriter, u32 port);
void startPadRecording(u32 port);
void endPadRecording(u32 port);
bool isPadReplaying(u32 port);
s32 getPadReplayRemainFrame(u32 port);
bool isPadRecording(u32 port);
void invalidatePadReplay(u32 port);
void validatePadReplay(u32 port);
bool isValidReplayController(u32 port);
bool isReadPadReplayData(u32 port);
}  // namespace al
