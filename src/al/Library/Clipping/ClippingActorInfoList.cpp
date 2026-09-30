#include "Library/Clipping/ClippingActorInfo.hpp"

namespace al {
/**
 * Constructs an empty clipping info list.
 * @param maxInfos capacity of the list
 */
ClippingActorInfoList::ClippingActorInfoList(s32 maxInfos) : mMaxInfos(maxInfos) {
    mInfos = new ClippingActorInfo*[maxInfos];

    for (s32 i = 0; i < mMaxInfos; i++) {
        mInfos[i] = nullptr;
    }
}

/**
 * Appends a clipping info.
 * @param pInfo info to append
 */
void ClippingActorInfoList::add(ClippingActorInfo* pInfo) {
    mInfos[mNumInfos] = pInfo;
    mNumInfos++;
}

/**
 * Removes the clipping info of an actor, moving the last info into its slot.
 * @param pActor actor to remove
 * @return the removed info
 */
ClippingActorInfo* ClippingActorInfoList::remove(LiveActor* pActor) {
    s32 index = 0;
    ClippingActorInfo* info = find(pActor, &index);
    mInfos[index] = mInfos[mNumInfos - 1];
    mNumInfos--;
    return info;
}

/**
 * Finds the clipping info of an actor.
 * @param pActor actor to look for
 * @param pIndex output index, or nullptr
 * @return the info, or the first info if the actor isn't in the list
 */
ClippingActorInfo* ClippingActorInfoList::find(const LiveActor* pActor, s32* pIndex) const {
    for (s32 i = 0; i < mNumInfos; i++) {
        if (mInfos[i]->getLiveActor() == pActor) {
            if (pIndex) {
                *pIndex = i;
            }

            return mInfos[i];
        }
    }

    return mInfos[0];
}

/**
 * Finds the clipping info of an actor, searching from the back.
 * @param pActor actor to look for
 * @return the info, or nullptr if the actor isn't in the list
 */
ClippingActorInfo* ClippingActorInfoList::tryFind(const LiveActor* pActor) const {
    for (s32 i = mNumInfos - 1; i >= 0; i--) {
        if (mInfos[i]->getLiveActor() == pActor) {
            return mInfos[i];
        }
    }

    return nullptr;
}

/**
 * Checks whether an actor is in the list.
 * @param pActor actor to look for
 * @return true if the actor is in the list
 */
bool ClippingActorInfoList::isInList(const LiveActor* pActor) const {
    for (s32 i = 0; i < mNumInfos; i++) {
        if (mInfos[i]->getLiveActor() == pActor) {
            return true;
        }
    }

    return false;
}
}  // namespace al
