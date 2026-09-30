#pragma once

#include <nn/atk/atk_Sound3DManager.h>

#include "audio/seadAudio3DListenerGroupNin.h"
#include "audio/seadAudio3DListenerNin.h"
#include "audio/seadAudio3DMgr.h"
#include "container/seadOffsetList.h"

namespace sead {
class Audio3DMgrNin : public Audio3DMgr {
    SEAD_RTTI_OVERRIDE(Audio3DMgrNin, Audio3DMgr)

public:
    explicit Audio3DMgrNin(bool createDefaultListener);
    ~Audio3DMgrNin() override;

    void initialize(AudioMgr& rMgr, Heap* pHeap) override;
    void finalize() override;

    void appendListener(Audio3DListenerNin& rListener);
    void removeListener(Audio3DListenerNin& rListener);
    void appendListenerGroup(Audio3DListenerGroupNin& rGroup);
    void removeListenerGroup(Audio3DListenerGroupNin& rGroup);
    bool isListenerAddedToNw(Audio3DListenerNin& rListener) const;

    void setMaxPriorityReduction(s32 reduction);
    s32 getMaxPriorityReduction();
    void setPanRange(f32 range);
    f32 getPanRange();
    void setSonicVelocity(f32 velocity);
    f32 getSonicVelocity();
    void setBiquadFilterType(s32 type);
    s32 getBiquadFilterType() const;
    void setDefaultListenerMatrix(const Matrix34f& rMtx);
    void resetDefaultListenerMatrix();
    void setDefaultListenerParameter(const Audio3DListenerParameterNin& rParam);
    void getDefaultListenerParameter(Audio3DListenerParameterNin* pParam);

    void genMessage(hostio::Context* pContext);
    void listenPropertyEvent(const hostio::PropertyEvent* pEvent);

    nn::atk::Sound3DManager* getSound3DManager() const { return mSound3DManager; }
    Audio3DListenerNin* getDefaultListener() const { return mDefaultListener; }

private:
    nn::atk::Sound3DManager* mSound3DManager = nullptr;
    u8* mWorkBuffer = nullptr;
    Audio3DListenerNin* mDefaultListener = nullptr;
    OffsetList<Audio3DListenerNin> mListeners;
    OffsetList<Audio3DListenerGroupNin> mListenerGroups;
};
static_assert(sizeof(Audio3DMgrNin) == 0x60);
}  // namespace sead
