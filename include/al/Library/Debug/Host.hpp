#pragma once

#include <prim/seadSafeString.h>

#include "Project/Base/StringUtil.hpp"

namespace al {
bool tryGetComputerName(sead::BufferedSafeString* pComputerName);
void getComputerName(sead::BufferedSafeString* pComputerName);
void getUserName(sead::BufferedSafeString* pUserName);
void makeUniqueTemporaryFilename(sead::BufferedSafeString* pOut, const char* pFileName);
void expandEnvironmentString(sead::BufferedSafeString* pOut, const sead::SafeString& rStr);
StringTmp<128> makeTmpExpandEnvironmentString(const sead::SafeString& rStr);
StringTmp<128> makeTmpFileFullPath(const char* pFileName);
const char* getALCommon();
}  // namespace al
