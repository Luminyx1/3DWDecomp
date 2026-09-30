#pragma once

#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;

class MtxConnector {
public:
    MtxConnector();
    MtxConnector(const sead::Quatf& rQuat, const sead::Vector3f& rTrans);

    virtual ~MtxConnector() {}
    virtual bool isConnecting() const;
    virtual void clear();

    void init(const sead::Matrix34f* pParentMtx, const sead::Matrix34f& rMtx);
    void init(const sead::Matrix34f* pParentMtx);
    void multVec(sead::Vector3f* pOut, const sead::Vector3f& rVec) const;
    void multTrans(sead::Vector3f* pOut, const sead::Vector3f& rTrans) const;
    void multMtx(sead::Matrix34f* pOut, const sead::Matrix34f& rMtx) const;
    void multQT(sead::Quatf* pQuat, sead::Vector3f* pTrans) const;
    void multQT(sead::Quatf* pQuat, sead::Vector3f* pTrans, const sead::Quatf& rQuat,
                const sead::Vector3f& rTrans) const;
    const sead::Quatf& getBaseQuat() const;
    const sead::Vector3f& getBaseTrans() const;
    void setBaseQuatTrans(const sead::Quatf& rQuat, const sead::Vector3f& rTrans);
    void calcConnectInfo(sead::Vector3f* pTrans, sead::Quatf* pQuat, sead::Vector3f* pScale,
                         const sead::Vector3f& rOffsetTrans,
                         const sead::Vector3f& rOffsetRotate) const;
    void calcMtxWithOffset(sead::Matrix34f* pOut, const sead::Vector3f& rOffsetTrans,
                           const sead::Vector3f& rOffsetRotate) const;
    bool tryGetParentTrans(sead::Vector3f* pTrans) const;

private:
    sead::Matrix34f mMtx = sead::Matrix34f::ident;
    const sead::Matrix34f* mParentMtx = nullptr;
    sead::Quatf mBaseQuat = sead::Quatf::unit;
    sead::Vector3f mBaseTrans = sead::Vector3f::zero;
};

static_assert(sizeof(MtxConnector) == 0x60);

MtxConnector* createMtxConnector(const LiveActor*);

void attachMtxConnectorToCollision(MtxConnector*, const LiveActor*, bool);

const sead::Vector3f& getConnectBaseTrans(const MtxConnector*);

void connectPoseTrans(LiveActor*, const MtxConnector*, const sead::Vector3f&);
}  // namespace al
