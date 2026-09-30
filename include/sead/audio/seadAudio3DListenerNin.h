#pragma once

#include <nn/atk/atk_Sound3DListener.h>

#include "basis/seadTypes.h"
#include "container/seadListImpl.h"
#include "hostio/seadHostIONode.h"
#include "math/seadMatrix.h"
#include "math/seadVector.h"

namespace sead {
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio

struct Audio3DListenerParameterNin {
    f32 mInteriorSize;
    f32 mMaxVolumeDistance;
    f32 mUnitDistance;
    u32 mUserParam;
    f32 mUnitBiquadFilterValue;
    f32 mMaxBiquadFilterValue;
    u32 mOutputTypeFlag;
};

class Audio3DListenerNin : public nn::atk::Sound3DListener, public hostio::Node {
public:
    enum GroupParamFlag {
        cGroupParam_InteriorSize = 1 << 0,
        cGroupParam_MaxVolumeDistance = 1 << 1,
        cGroupParam_UnitDistance = 1 << 2,
        cGroupParam_UnitBiquadFilterValue = 1 << 3,
        cGroupParam_MaxBiquadFilterValue = 1 << 4,
        cGroupParam_UserParam = 1 << 5,
        cGroupParam_OutputTypeFlag = 1 << 6,
        cGroupParam_Matrix = 1 << 8,
        cGroupParam_Velocity = 1 << 9
    };

    Audio3DListenerNin();
    virtual ~Audio3DListenerNin();

    void setMatrix(const Matrix34f& rMtx);
    Matrix34f getMatrix() const;
    void resetMatrix();
    Vector3f getPosition() const;
    void setVelocity(const Vector3f& rVelocity);
    Vector3f getVelocity() const;
    void setInteriorSize(f32 size);
    void setMaxVolumeDistance(f32 distance);
    void setUnitDistance(f32 distance);
    void setUnitBiquadFilterValue(f32 value);
    void setMaxBiquadFilterValue(f32 value);
    void setUserParam(u32 param);
    void setOutputTypeFlag(u32 flag);
    void setOutputType(nn::atk::Sound3DListener::ListenerOutputType type);
    void setParameterAll(const Audio3DListenerParameterNin& rParam);
    f32 getInteriorSize() const;
    f32 getMaxVolumeDistance() const;
    f32 getUnitDistance() const;
    f32 getUnitBiquadFilterValue() const;
    f32 getMaxBiquadFilterValue() const;
    u32 getUserParam() const;
    u32 getOutputTypeFlag() const;
    void getParameterAll(Audio3DListenerParameterNin* pParam) const;
    void genMessage(hostio::Context* pContext);
    void listenPropertyEvent(const hostio::PropertyEvent* pEvent);

    nn::atk::Sound3DListener& getNwListener() { return *this; }
    bool isFollowingGroup(u32 flag) const { return mGroupParamFlag & flag; }

private:
    friend class Audio3DMgrNin;
    friend class Audio3DListenerGroupNin;

    u16 mGroupParamFlag = 0xffff;
    ListNode mListNode;
};
static_assert(sizeof(Audio3DListenerNin) == 0xc0);
}  // namespace sead
