#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;

class KCPrismHeader;

class KCPrismData {
public:
    f32 mLength;
    u16 mPosIndex;
    u16 mFaceNormalIndex;
    u16 mEdgeNormalIndex[3];
    u16 mCollisionType;
    u32 mTriIndex;
};

class KCollisionServer {
public:
    KCollisionServer();

    const sead::Vector3f& getFaceNormal(const KCPrismData* pData,
                                        const KCPrismHeader* pHeader) const;
    const sead::Vector3f& getEdgeNormal1(const KCPrismData* pData,
                                         const KCPrismHeader* pHeader) const;
    const sead::Vector3f& getEdgeNormal2(const KCPrismData* pData,
                                         const KCPrismHeader* pHeader) const;
    const sead::Vector3f& getEdgeNormal3(const KCPrismData* pData,
                                         const KCPrismHeader* pHeader) const;
    void calcPosLocal(sead::Vector3f* pPos, const KCPrismData* pData, s32 index,
                      const KCPrismHeader* pHeader) const;
    bool getAttributes(ByamlIter* pIter, const KCPrismData* pData) const;
};
}  // namespace al
