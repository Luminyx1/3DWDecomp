#pragma once

#include "Library/LiveActor/Common/LiveActorGroup.hpp"

namespace al {
template <class T>
class DeriveActorGroup : public LiveActorGroup {
public:
    DeriveActorGroup(const char* pName, s32 maxActors) : LiveActorGroup(pName, maxActors) {}

    T* getDeriveActor(s32 idx) const { return static_cast<T*>(getActor(idx)); }

    s32 getActorCount() const { return mNumActors; }

    s32 getMaxActorCount() const { return mMaxActors; }
};
}  // namespace al
