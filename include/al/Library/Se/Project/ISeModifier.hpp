#pragma once

#include <basis/seadTypes.h>

namespace al {
class SePlayParamList;

class ISeModifier {
public:
    virtual u32 modifySoundId(u32 id) = 0;
    virtual void modifyStartParam(u32 id, SePlayParamList* pParamList) = 0;
    virtual void modifyHoldParam(u32 id, SePlayParamList* pParamList) = 0;
};
}  // namespace al
