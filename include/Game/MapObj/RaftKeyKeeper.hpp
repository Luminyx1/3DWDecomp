#pragma once
#include <basis/seadTypes.h>
#include <math/seadVector.h>
namespace al { class ActorInitInfo; }
class RaftKeyKeeper {
public:
    RaftKeyKeeper();
    void init(const al::ActorInitInfo&);
    void calcClippingSphere(sead::Vector3f*, float*, float);
    int getKeyCount() const { return mKeyCount; }
    float getTotalLength() const { return mTotalLength; }
    float getAppearEndCoord() const { return mAppearEndCoord; }
private:
    int mUnknown0;
    void* mKeys;
    int mKeyCount;
    u8 mUnreconstructed14[0x3c - 0x14];
    float mTotalLength;
    float mAppearEndCoord;
};
