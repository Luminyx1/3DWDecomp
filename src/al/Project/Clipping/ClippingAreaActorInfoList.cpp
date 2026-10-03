#include "Project/Clipping/ClippingAreaActorInfoList.hpp"

#include "Project/Clipping/ClippingAreaActorInfo.hpp"

namespace al {
/**
 * Creates an empty info list.
 * @param pHolder owning view holder
 * @param maxInfos maximum number of infos (unused)
 */
ClippingAreaActorInfoList::ClippingAreaActorInfoList(ClippingAreaActorViewHolder* pHolder,
                                                     s32 maxInfos)
    : mHolder(pHolder) {
    // the list node of ClippingAreaActorInfo is right after its vtable
    mList.initOffset(sizeof(void*));
    mQueue.initOffset(sizeof(void*));
}

/**
 * Adds an info to the list.
 * @param pInfo info to add
 */
void ClippingAreaActorInfoList::registerInfo(ClippingAreaActorInfo* pInfo) {
    mList.pushBack(pInfo);
}

/**
 * Removes an info from the list.
 * @param pInfo info to remove
 */
void ClippingAreaActorInfoList::removeInfo(ClippingAreaActorInfo* pInfo) {
    mList.erase(pInfo);
}

/**
 * Queues an info to be added to the list.
 * @param pInfo info to queue
 */
void ClippingAreaActorInfoList::queueInfo(ClippingAreaActorInfo* pInfo) {
    mQueue.pushBack(pInfo);
}

/**
 * Moves all queued infos to the list.
 */
void ClippingAreaActorInfoList::dequeueList() {
    for (auto it = mQueue.begin(); it != mQueue.end();) {
        ClippingAreaActorInfo* info = &*it;
        ++it;
        mQueue.erase(info);
        mList.pushBack(info);
    }
}

/**
 * Checks the unclipped infos and queues the ones to clip.
 * @param pClippedQueue queue of the clipped list
 */
void ClippingAreaActorInfoList::updateUnclipped(ClippingAreaActorInfoOffsetList* pClippedQueue) {
    for (auto it = mList.begin(); it != mList.end();) {
        ClippingAreaActorInfo* info = &*it;
        ++it;
        if (info->checkClipping(mHolder)) {
            mList.erase(info);
            pClippedQueue->pushBack(info);
        }
    }
}

/**
 * Checks the clipped infos and queues the ones to unclip.
 * @param pUnclippedQueue queue of the unclipped list
 */
void ClippingAreaActorInfoList::updateClipped(ClippingAreaActorInfoOffsetList* pUnclippedQueue) {
    for (auto it = mList.begin(); it != mList.end();) {
        ClippingAreaActorInfo* info = &*it;
        ++it;
        if (info->checkStillClipping(mHolder)) {
            mList.erase(info);
            pUnclippedQueue->pushBack(info);
        }
    }
}
}  // namespace al
