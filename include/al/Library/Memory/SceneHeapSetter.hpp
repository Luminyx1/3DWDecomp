#pragma once

#include <heap/seadHeapMgr.h>

namespace al {
class SceneHeapSetter {
public:
    SceneHeapSetter();

    sead::ScopedCurrentHeapSetter mSetter;
    sead::Heap* mSceneHeap;
};
}  // namespace al
