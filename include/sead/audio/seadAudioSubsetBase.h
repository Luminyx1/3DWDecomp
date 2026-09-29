#pragma once

#include "basis/seadTypes.h"
#include "container/seadListImpl.h"
#include "hostio/seadHostIONode.h"
#include "prim/seadRuntimeTypeInfo.h"

namespace sead {
class AudioMgr;
class Heap;

class AudioSubsetBase : public hostio::Node {
    SEAD_RTTI_BASE(AudioSubsetBase)

public:
    AudioSubsetBase() = default;
    virtual ~AudioSubsetBase() {}

    virtual void initialize(AudioMgr& rMgr, Heap* pHeap) = 0;
    virtual void finalize() = 0;
    virtual void calc() {}
    virtual void executeOnAppend(AudioMgr& rMgr) {}
    virtual void executeOnRemove() {}
    virtual void reset(s32 fadeFrames) {}
    virtual bool isResetting() const { return false; }
    virtual bool isResetDone() const { return true; }
    virtual void recoverReset() {}
    virtual void shutdown(s32 fadeFrames) {}
    virtual bool isShuttingDown() const { return false; }
    virtual bool isShutdownDone() const { return true; }
    virtual void pause(s32 fadeFrames) {}
    virtual void unpause(s32 fadeFrames) {}
    virtual bool isPausing() const { return false; }

    static s32 getListNodeOffset() { return offsetof(AudioSubsetBase, mListNode); }

private:
    ListNode mListNode;
};
}  // namespace sead
