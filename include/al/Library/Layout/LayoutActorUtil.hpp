#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>

namespace al {
class IUseLayout;
class LayoutActor;

void updateLayoutPaneRecursive(LayoutActor* pActor);
void setPaneString(IUseLayout* pLayout, const char* pPaneName, const char16_t* pString,
                   u16 pos = 0, u32 = 0xffffffff);
const sead::Matrix34f* getPaneMtxRaw(const IUseLayout* pLayout, const char* pPaneName);
}  // namespace al
