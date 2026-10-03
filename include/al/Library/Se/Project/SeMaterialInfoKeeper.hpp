#pragma once

#include <basis/seadTypes.h>

namespace al {
class SeDataBase;
class SeResourceSpecificInfo;

class SeMaterialInfoKeeper {
  public:
    SeMaterialInfoKeeper(SeDataBase* pDataBase);
    void init();
    s32 findReplacedId(s32 soundId, const char* pMaterialName, s32 materialState,
                       const SeResourceSpecificInfo* pInfo);

  private:
    SeDataBase* mDataBase;
};
static_assert(sizeof(SeMaterialInfoKeeper) == 8);
} // namespace al
