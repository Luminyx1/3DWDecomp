#pragma once

#include <basis/seadTypes.h>

namespace alNerveFunction {
class NerveActionCollector;
}  // namespace alNerveFunction

namespace al {
class NerveAction;

class NerveActionCtrl {
public:
    NerveActionCtrl(alNerveFunction::NerveActionCollector* pCollector);

    NerveAction* findNerve(const char* pName) const;

    s32 mNumActions = 0;
    NerveAction** mActions = nullptr;
};
}  // namespace al
