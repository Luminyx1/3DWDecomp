#pragma once

#include <basis/seadTypes.h>
#include <container/seadOffsetList.h>

namespace al {
class ClippingAreaActorInfo;
class ClippingAreaActorViewHolder;

typedef sead::OffsetList<ClippingAreaActorInfo> ClippingAreaActorInfoOffsetList;

/**
 * List of clipping infos with a queue of infos waiting to be added to it.
 */
class ClippingAreaActorInfoList {
public:
    ClippingAreaActorInfoList(ClippingAreaActorViewHolder* pHolder, s32 maxInfos);

    void registerInfo(ClippingAreaActorInfo* pInfo);
    void removeInfo(ClippingAreaActorInfo* pInfo);
    void queueInfo(ClippingAreaActorInfo* pInfo);
    void dequeueList();
    void updateUnclipped(ClippingAreaActorInfoOffsetList* pClippedQueue);
    void updateClipped(ClippingAreaActorInfoOffsetList* pUnclippedQueue);

    ClippingAreaActorInfoOffsetList* getQueue() { return &mQueue; }

    ClippingAreaActorInfoOffsetList mList;
    ClippingAreaActorInfoOffsetList mQueue;
    ClippingAreaActorViewHolder* mHolder;
};

static_assert(sizeof(ClippingAreaActorInfoList) == 0x38);
}  // namespace al
