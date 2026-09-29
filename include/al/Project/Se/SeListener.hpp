#pragma once

#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace al {
class ISeListenerParam;
class ISeListenerPoser;

class SeListener {
public:
    SeListener(s32 poserNumMax);

    void addPoser(ISeListenerPoser* pPoser);
    void setCurrentPoserIndex(s32 index);
    void setCurrentPoser(const sead::SafeString& rName);
    void calcListenerMatrix(const ISeListenerParam& rParam);
    ISeListenerPoser* getCurrentPoser() const;

    sead::PtrArray<ISeListenerPoser> mPosers;  // _0
    s32 mCurrentPoserIndex;                    // _10
    sead::Matrix34f mListenerMatrix;           // _14
    sead::Vector3f mListenerPos;               // _44
};
}  // namespace al
