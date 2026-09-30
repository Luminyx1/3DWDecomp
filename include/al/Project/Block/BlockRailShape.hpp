#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class ByamlIter;

class BlockRailShape {
public:
    BlockRailShape(const char* pName);

    void init(const ActorInitInfo& rInfo, const ByamlIter& rIter);

    virtual void init(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
                      const ByamlIter& rIter);
    virtual bool isRide(f32* pRate, const sead::Vector3f& rPrevPos,
                        const sead::Vector3f& rPos) const;
    virtual f32 getTotalLength() const;
    virtual void calcPos(sead::Vector3f* pPos, f32 rate) const;
    virtual void calcDir(sead::Vector3f* pDir, f32 rate) const;
    virtual void calcNearestParam(sead::Vector3f* pPos, f32* pRate,
                                  const sead::Vector3f& rPos) const;
    virtual bool isTerminate() const;
    virtual void calcOffset(const sead::Vector3f& rBaseTrans);
    virtual void updateLinkedTrans(const sead::Vector3f& rBaseTrans);

    const char* mName;
    sead::Vector3f mOffset;
};
}  // namespace al
