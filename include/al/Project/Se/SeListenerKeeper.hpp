#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace sead {
struct Audio3DListenerParameterNin;
class PerspectiveProjection;
}  // namespace sead

namespace al {
class ISeListenerParam;
class SeadAudio3DMgr;
class SeListener;

class SeListenerKeeper {
public:
    SeListenerKeeper(bool isUnused, s32 listenerNum);

    void init(SeadAudio3DMgr* pMgr, const sead::Vector3f* pCameraPos, const sead::Matrix34f* pCameraMtx,
              sead::PerspectiveProjection* pProjection, const sead::Vector3f* pCameraAt, const char* pPoserName);
    void update();
    void changeListenerParam(sead::Audio3DListenerParameterNin& rParam);
    void resetListenerParam();
    void changeListenerPoser(const char* pName);
    void changeListenerPoserToLast();

private:
    ISeListenerParam* mListenerParam = nullptr;
    SeadAudio3DMgr* mAudio3DMgr = nullptr;
    const char* mDefaultPoserName = nullptr;
    const char* mLastPoserName = nullptr;
    sead::PtrArray<SeListener> mListeners;
};

static_assert(sizeof(SeListenerKeeper) == 0x30);
}  // namespace al
