#pragma once

#include <basis/seadTypes.h>

namespace al {
u8* getSaveDataWorkBuffer();
bool isInitializedSaveData();
bool requestInitSaveDir(const char* pFileName, u32 dirSize, u32 version);
bool requestFormatSaveData(s32 a, s32 b);
bool requestReadSaveData(const char* pFileName, u32 readSize, u32 version);
bool requestWriteSaveData(const char* pFileName, u32 writeSize, u32 version, bool isFlushNeeded);
bool requestFlushSaveData();
bool initSaveDirSync(const char* pFileName, u32 dirSize, u32 version);
bool formatSaveDataSync(s32 a, s32 b);
bool readSaveDataSync(const char* pFileName, u32 readSize, u32 version);
bool writeSaveDataSync(const char* pFileName, u32 writeSize, u32 version);
void copyReadSaveDataFromBuffer(void* pBuffer, u32 size);
void copyWriteSaveDataToBuffer(const void* pBuffer, u32 size);
bool updateSaveDataSequence();
bool isSuccessSaveDataSequence();
bool isDoneSaveDataSequence();
bool isCorruptedSaveDataSequenceRead();
bool isCorruptedSaveDataSequenceResult();
s32 getSaveDataSequenceResult();
s32 getLastSaveDataFSErrorCode();
}  // namespace al
