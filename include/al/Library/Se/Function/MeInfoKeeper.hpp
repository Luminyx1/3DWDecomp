#pragma once
#include <basis/seadTypes.h>

namespace al {
class MeInfo;
class SePlayParamList;
class MeInfoKeeper {
  public:
    bool isMe(s32 soundId) const;
    void applyMeInfoToParams(const char* pName, SePlayParamList* pParams, MeInfo* pInfo);
};
} // namespace al
