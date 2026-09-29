#pragma once

#include <basis/seadTypes.h>

namespace alNerveFunction {
class NerveActionCollector;
}

namespace al {
class NerveAction;

class NerveActionCtrl {
public:
    NerveActionCtrl(alNerveFunction::NerveActionCollector* pCollector);

    NerveAction* findNerve(const char* pName) const;

    s32 mNumActions;         // _0
    NerveAction** mActions;  // _8
};
}  // namespace al
