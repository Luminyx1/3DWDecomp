#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <container/seadRingBuffer.h>
#include <math/seadVector.h>
#include <prim/seadDelegate.h>

namespace al {
class ByamlIter;

class KCollisionHeader {
public:
    u32 mVersion;
    u32 mOctreeOffset;
    u32 mModelOffsetsOffset;
    s32 mModelNum;
    sead::Vector3f mMin;
    sead::Vector3f mMax;
    s32 mAreaWidthShift[3];
    u32 _34;
};

class KCPrismHeader {
public:
    u32 mPositionsOffset;
    u32 mNormalsOffset;
    u32 mTrianglesOffset;
    u32 mOctreeOffset;
    f32 mThickness;
    sead::Vector3f mOctreeOrigin;
    u32 mWidthMask[3];
    s32 mBlockWidthShift;
    s32 mAreaXWidthShift;
    s32 mAreaXYWidthShift;
    f32 mHitboxRadiusCap;
};

class KCPrismData {
public:
    f32 mLength;
    u16 mPosIndex;
    u16 mFaceNormalIndex;
    u16 mEdgeNormalIndex[3];
    u16 mCollisionType;
    u32 mTriIndex;
};

class KCHitInfo {
public:
    const KCPrismHeader* mHeader;
    const KCPrismData* mData;
    f32 mDist;
    u8 mCollisionLocation;
};

class KCFxyz : public sead::Vector3f {};

class KCollisionServer {
public:
    typedef sead::FixedRingBuffer<KCHitInfo, 512> HitInfoBuffer;
    typedef sead::IDelegate2<KCPrismData*, const KCPrismHeader*> PrismDelegate;

    KCollisionServer();

    void initKCollisionServer(void* pData, const void* pAttributeData);
    void setData(void* pData);
    const KCPrismHeader* getInnerKcl(s32 index) const;
    s32 getNumInnerKcl() const;
    const KCPrismHeader* getV1Header(s32 index) const;
    bool calcFarthestVertexDistance();

    f32 getFarthestVertexDistance() const { return mFarthestVertexDistance; }

    u32 getTriangleNum(const KCPrismHeader* pHeader) const;
    const KCPrismData* getPrismData(u32 index, const KCPrismHeader* pHeader) const;
    bool isNearParallelNormal(const KCPrismData* pData, const KCPrismHeader* pHeader) const;
    bool isNanPrism(const KCPrismData* pData, const KCPrismHeader* pHeader) const;
    void calcPosLocal(sead::Vector3f* pPos, const KCPrismData* pData, s32 index,
                      const KCPrismHeader* pHeader) const;
    void getMinMax(sead::Vector3f* pMin, sead::Vector3f* pMax) const;
    void getAreaSpaceSize(sead::Vector3f* pSize, const KCPrismHeader* pHeader) const;
    void getAreaSpaceSize(s32* pSizeX, s32* pSizeY, s32* pSizeZ,
                          const KCPrismHeader* pHeader) const;
    void getAreaSpaceSize(sead::Vector3u* pSize, const KCPrismHeader* pHeader) const;
    const KCPrismData* checkPoint(KCFxyz* pPos, f32 thicknessScale, f32* pDist);
    const u16* searchBlock(s32* pShift, const sead::Vector3u& rBlock,
                           const KCPrismHeader* pHeader) const;
    s32 checkSphere(KCFxyz* pPos, f32 radius, f32 thicknessScale, u32 maxHitNum,
                    HitInfoBuffer* pHitInfos);
    bool outCheckAndCalcArea(sead::Vector3u* pBlockMin, sead::Vector3u* pBlockMax,
                             const sead::Vector3f& rMin, const sead::Vector3f& rMax,
                             const KCPrismHeader* pHeader) const;
    bool KCHitSphere(const KCPrismData* pData, const KCPrismHeader* pHeader, const KCFxyz* pPos,
                     f32 radius, f32 thicknessScale, f32* pDist, u8* pLocation);
    const KCPrismData* checkArrow(const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                                  HitInfoBuffer* pHitInfos, u32* pHitNum, u32 maxHitNum) const;
    void objectSpaceToAreaOffsetSpaceV3f(sead::Vector3f* pAreaPos, const sead::Vector3f& rPos,
                                         const KCPrismHeader* pHeader) const;
    bool isInsideMinMaxInAreaOffsetSpace(const sead::Vector3u& rBlock,
                                         const KCPrismHeader* pHeader) const;
    bool KCHitArrow(const KCPrismData* pData, const KCPrismHeader* pHeader,
                    const sead::Vector3f& rPos, const sead::Vector3f& rDir, f32* pDist,
                    u8* pLocation) const;
    s32 checkSphereForPlayer(KCFxyz* pPos, f32 radius, u32 maxHitNum, HitInfoBuffer* pHitInfos);
    bool KCHitSphereForPlayer(const KCPrismData* pData, const KCPrismHeader* pHeader,
                              KCFxyz* pPos, f32 radius, f32* pDist, u8* pLocation);
    s32 checkDisk(KCFxyz* pPos, f32 radius, f32 halfHeight, const sead::Vector3f& rNormal,
                  f32 thicknessScale, u32 maxHitNum, HitInfoBuffer* pHitInfos);
    bool KCHitDisk(const KCPrismData* pData, const KCPrismHeader* pHeader, KCFxyz* pPos,
                   f32 radius, f32 thicknessScale, f32 halfHeight, const sead::Vector3f& rNormal,
                   f32* pDist, u8* pLocation);
    void searchPrism(KCFxyz* pPos, f32 radius, PrismDelegate& rDelegate);
    bool isParallelNormal(const KCPrismData* pData, const KCPrismHeader* pHeader) const;
    const sead::Vector3f& getFaceNormal(const KCPrismData* pData,
                                        const KCPrismHeader* pHeader) const;
    const sead::Vector3f& getEdgeNormal1(const KCPrismData* pData,
                                         const KCPrismHeader* pHeader) const;
    const sead::Vector3f& getEdgeNormal2(const KCPrismData* pData,
                                         const KCPrismHeader* pHeader) const;
    const sead::Vector3f& getEdgeNormal3(const KCPrismData* pData,
                                         const KCPrismHeader* pHeader) const;
    bool KCHitDisc(const KCPrismData* pData, const KCPrismHeader* pHeader,
                   const sead::Vector3f& rPos, const sead::Vector3f& rNormal, f32 radius,
                   f32 angle, sead::Vector3f* pFixDir, f32* pDist);
    s32 toIndex(const KCPrismData* pData, const KCPrismHeader* pHeader) const;
    const sead::Vector3f& getNormal(u32 index, const KCPrismHeader* pHeader) const;
    static void calXvec(const KCFxyz* pA, const KCFxyz* pB, KCFxyz* pOut);
    const sead::Vector3f& getVertexData(u32 index, const KCPrismHeader* pHeader) const;
    s32 getVertexNum(const KCPrismHeader* pHeader) const;
    s32 getNormalNum(const KCPrismHeader* pHeader) const;
    s32 getAttributeElementNum() const;
    bool getAttributes(ByamlIter* pIter, u32 triIndex, const KCPrismHeader* pHeader) const;
    bool getAttributes(ByamlIter* pIter, const KCPrismData* pData) const;
    void objectSpaceToAreaOffsetSpace(sead::Vector3u* pAreaPos, const sead::Vector3f& rPos,
                                      const KCPrismHeader* pHeader) const;
    void areaOffsetSpaceToObjectSpace(sead::Vector3f* pPos, const sead::Vector3u& rAreaPos,
                                      const KCPrismHeader* pHeader) const;
    bool doBoxCheck(const sead::Vector3f* pPos, const sead::Vector3f* pSize,
                    sead::Vector3u* pBlockMin, sead::Vector3u* pBlockMax,
                    const KCPrismHeader* pHeader);
    u32 calcAreaBlockOffset(const sead::Vector3u& rBlock, const KCPrismHeader* pHeader) const;
    static u32 calcChildBlockOffset(const sead::Vector3u& rBlock, s32 shift);
    static u32 getBlockData(const u32* pData, u32 offset);

private:
    typedef sead::PtrArray<KCPrismHeader> HeaderArray;

    HeaderArray mModelsData;
    KCollisionHeader* mData = nullptr;
    ByamlIter* mAttributeIter = nullptr;
    const u32* mModelOffsets = nullptr;
    const u8* mOctreeData = nullptr;
    s32 mAreaWidthShift[3] = {};
    s32 mAreaWidthMask[3] = {};
    f32 mFarthestVertexDistance = 1.0f;
};
}  // namespace al
