#pragma once

#include "audio/seadAudio3DListenerNin.h"
#include "container/seadOffsetList.h"

namespace sead {
class Audio3DMgrNin;

class Audio3DListenerGroupNin : public hostio::Node {
public:
    Audio3DListenerGroupNin();
    ~Audio3DListenerGroupNin();

    void append(Audio3DListenerNin& rListener);
    void remove(Audio3DListenerNin& rListener);
    void removeAll();
    void setMatrix(const Matrix34f& rMtx);
    void resetMatrix();
    void setVelocity(const Vector3f& rVelocity);
    void setInteriorSize(f32 size);
    void setMaxVolumeDistance(f32 distance);
    void setUnitDistance(f32 distance);
    void setUnitBiquadFilterValue(f32 value);
    void setMaxBiquadFilterValue(f32 value);
    void setUserParam(u32 param);
    void setOutputTypeFlag(u32 flag);
    void setOutputType(nn::atk::Sound3DListener::ListenerOutputType type);
    void setParameterAll(const Audio3DListenerParameterNin& rParam);
    void getParameterAll(Audio3DListenerParameterNin* pParam) const;
    void genMessage(hostio::Context* pContext);
    void listenPropertyEvent(const hostio::PropertyEvent* pEvent);

private:
    friend class Audio3DMgrNin;

    void reflectGroupParamToListener_(Audio3DListenerNin& rListener);
    void reflectGroupParamToListenerAll_();
    void updateInteriorSizeAll_();
    void updateMaxVolumeDistanceAll_();
    void updateUnitDistanceAll_();
    void updateUnitBiquadFilterValueAll_();
    void updateMaxBiquadFilterValueAll_();
    void updateUserParamAll_();
    void updateOutputTypeFlagAll_();

    Audio3DMgrNin* mMgr = nullptr;
    Audio3DListenerParameterNin mParam;
    OffsetList<Audio3DListenerNin> mListeners;
    ListNode mListNode;
};
static_assert(sizeof(Audio3DListenerGroupNin) == 0x58);
}  // namespace sead
