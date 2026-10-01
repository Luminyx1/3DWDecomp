#include "Library/Collision/KCollisionServer.hpp"

#include <cmath>

#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"

namespace al {
namespace {
template <typename T>
inline const T* getOffsetPtr(const void* pBase, uintptr_t offset) {
    return reinterpret_cast<const T*>(offset + reinterpret_cast<uintptr_t>(pBase));
}

inline bool checkHitFace(const f32* pDist, u8* pLocation, f32 maxDepth) {
    if (maxDepth < *pDist) {
        return false;
    }

    *pLocation = 1;
    return true;
}

inline void calcCornerVec(sead::Vector3f* pVec, f32 distA, f32 distB,
                          const sead::Vector3f& rNormalA, const sead::Vector3f& rNormalB, f32 cos) {
    f32 rateA = (distB * cos - distA) / (cos * cos + -1.0f);
    f32 rateB = distB - cos * rateA;
    pVec->x = rNormalA.x * rateA + rNormalB.x * rateB;
    pVec->y = rateA * rNormalA.y + rNormalB.y * rateB;
    pVec->z = rateA * rNormalA.z + rateB * rNormalB.z;
}
}  // namespace

/**
 * Constructs an empty collision server.
 */
KCollisionServer::KCollisionServer() = default;

/**
 * Sets the collision data and creates the attribute iterator.
 * @param pData KCL data
 * @param pAttributeData attribute BYAML data, may be null
 */
void KCollisionServer::initKCollisionServer(void* pData, const void* pAttributeData) {
    setData(pData);

    if (pAttributeData != nullptr) {
        mAttributeIter = new ByamlIter(static_cast<const u8*>(pAttributeData));
    }
}

/**
 * Sets the collision data and collects the headers of all inner models.
 * @param pData KCL data
 */
void KCollisionServer::setData(void* pData) {
    mData = static_cast<KCollisionHeader*>(pData);
    u8* data = static_cast<u8*>(pData);
    mModelOffsets = getOffsetPtr<u32>(data, mData->mModelOffsetsOffset);
    mOctreeData = getOffsetPtr<u8>(data, mData->mOctreeOffset);
    mAreaWidthShift[0] = mData->mAreaWidthShift[0] - 1;
    mAreaWidthShift[1] = mData->mAreaWidthShift[1] - 1;
    mAreaWidthShift[2] = mData->mAreaWidthShift[2] - 1;
    mAreaWidthMask[0] = -1 << mData->mAreaWidthShift[0];
    mAreaWidthMask[1] = -1 << mData->mAreaWidthShift[1];
    mAreaWidthMask[2] = -1 << mData->mAreaWidthShift[2];
    mModelsData.allocBuffer(mData->mModelNum, nullptr);

    for (s32 i = 0; i < mData->mModelNum; i++) {
        mModelsData.pushBack(const_cast<KCPrismHeader*>(getInnerKcl(i)));
    }
}

/**
 * Gets the header of an inner model from the model offset table.
 * @param index model index
 * @return model header
 */
const KCPrismHeader* KCollisionServer::getInnerKcl(s32 index) const {
    return reinterpret_cast<const KCPrismHeader*>(reinterpret_cast<const u8*>(mData) +
                                                  mModelOffsets[index]);
}

/**
 * Gets the number of inner models.
 * @return model count
 */
s32 KCollisionServer::getNumInnerKcl() const {
    return mData->mModelNum;
}

/**
 * Gets the header of a collected inner model.
 * @param index model index
 * @return model header, null if out of range
 */
const KCPrismHeader* KCollisionServer::getV1Header(s32 index) const {
    return mModelsData.at(index);
}

/**
 * Disables degenerate prisms and calculates the distance of the farthest vertex.
 * @return true if no model contains a prism
 */
bool KCollisionServer::calcFarthestVertexDistance() {
    bool isEmpty = true;
    f32 maxDistSq = 0.0f;

    for (s32 i = 0; i < getNumInnerKcl(); i++) {
        const KCPrismHeader* header = getV1Header(i);
        u32 triangleNum = getTriangleNum(header);

        for (u32 j = 0; j < triangleNum; j++) {
            isEmpty = false;
            KCPrismData* data = const_cast<KCPrismData*>(getPrismData(j, header));

            if (isNearParallelNormal(data, header)) {
                data->mLength = -sead::Mathf::abs(data->mLength);
                continue;
            }

            if (isNanPrism(data, header)) {
                f32 length = -sead::Mathf::abs(data->mLength);
                data->mLength = sead::Mathf::isNan(data->mLength) ? 0.0f : length;
                continue;
            }

            for (s32 k = 0; k < 3; k++) {
                sead::Vector3f pos;
                calcPosLocal(&pos, data, k, header);
                f32 distSq = pos.squaredLength();

                if (maxDistSq < distSq) {
                    maxDistSq = distSq;
                }
            }
        }
    }

    mFarthestVertexDistance = sead::Mathf::sqrt(maxDistSq);
    return isEmpty;
}

/**
 * Gets the number of prisms of a model.
 * @param pHeader model header
 * @return prism count
 */
u32 KCollisionServer::getTriangleNum(const KCPrismHeader* pHeader) const {
    return (static_cast<u64>(pHeader->mOctreeOffset) - pHeader->mTrianglesOffset) /
           sizeof(KCPrismData);
}

/**
 * Gets a prism of a model.
 * @param index prism index
 * @param pHeader model header
 * @return prism
 */
const KCPrismData* KCollisionServer::getPrismData(u32 index, const KCPrismHeader* pHeader) const {
    return &getOffsetPtr<KCPrismData>(pHeader, pHeader->mTrianglesOffset)[static_cast<s32>(index)];
}

/**
 * Checks whether two edge normals of a prism are nearly parallel.
 * @param pData prism
 * @param pHeader model header
 * @return true if nearly parallel
 */
bool KCollisionServer::isNearParallelNormal(const KCPrismData* pData,
                                            const KCPrismHeader* pHeader) const {
    sead::Vector3f edge1 = getEdgeNormal1(pData, pHeader);
    sead::Vector3f edge2 = getEdgeNormal2(pData, pHeader);
    sead::Vector3f edge3 = getEdgeNormal3(pData, pHeader);

    return isParallelDirection(edge1, edge2, 0.01f) || isParallelDirection(edge1, edge3, 0.01f) ||
           isParallelDirection(edge2, edge3, 0.01f);
}

bool KCollisionServer::isNanPrism(const KCPrismData* pData, const KCPrismHeader* pHeader) const {
    if (sead::Mathf::isNan(pData->mLength)) {
        return true;
    }

    KCFxyz faceNormal = static_cast<const KCFxyz&>(getFaceNormal(pData, pHeader));
    KCFxyz edgeNormal1 = static_cast<const KCFxyz&>(getEdgeNormal1(pData, pHeader));
    KCFxyz edgeNormal2 = static_cast<const KCFxyz&>(getEdgeNormal2(pData, pHeader));
    KCFxyz edgeNormal3 = static_cast<const KCFxyz&>(getEdgeNormal3(pData, pHeader));
    sead::Vector3f pos0 = getVertexData(pData->mPosIndex, pHeader);
    KCFxyz cross1;
    calXvec(&edgeNormal2, &faceNormal, &cross1);
    sead::Vector3f pos1 =
        cross1 * (pData->mLength /
                  sead::Mathf::max(cross1.dot(edgeNormal3), sead::Mathf::epsilon())) +
        pos0;

    if (faceNormal.isNan() || edgeNormal1.isNan() || edgeNormal2.isNan() ||
        edgeNormal3.isNan() || pos0.isNan() || pos1.isNan()) {
        return true;
    }

    KCFxyz cross2;
    calXvec(&faceNormal, &edgeNormal1, &cross2);
    sead::Vector3f pos2 =
        cross2 * (pData->mLength /
                  sead::Mathf::max(cross2.dot(edgeNormal3), sead::Mathf::epsilon())) +
        pos0;
    return pos2.isNan();
}

void KCollisionServer::calcPosLocal(sead::Vector3f* pPos, const KCPrismData* pData, s32 index,
                                    const KCPrismHeader* pHeader) const {
    const KCFxyz* vertices = getOffsetPtr<KCFxyz>(pHeader, pHeader->mPositionsOffset);
    const KCFxyz* normals = getOffsetPtr<KCFxyz>(pHeader, pHeader->mNormalsOffset);
    KCFxyz cross;

    switch (index) {
    case 0:
        *pPos = sead::Vector3f(vertices[pData->mPosIndex]);
        return;
    case 1:
        calXvec(&normals[pData->mEdgeNormalIndex[1]], &normals[pData->mFaceNormalIndex], &cross);
        break;
    case 2:
        calXvec(&normals[pData->mFaceNormalIndex], &normals[pData->mEdgeNormalIndex[0]], &cross);
        break;
    default:
        pPos->set(0.0f, 0.0f, 0.0f);
        return;
    }

    f32 dot = cross.dot(normals[pData->mEdgeNormalIndex[2]]);
    f32 scale = pData->mLength / sead::Mathf::max(dot, sead::Mathf::epsilon());
    const KCFxyz& vertex = vertices[pData->mPosIndex];
    pPos->x = vertex.x + cross.x * scale;
    pPos->y = vertex.y + cross.y * scale;
    pPos->z = vertex.z + cross.z * scale;
}

/**
 * Gets the bounding box of the collision data.
 * @param pMin output minimum
 * @param pMax output maximum
 */
void KCollisionServer::getMinMax(sead::Vector3f* pMin, sead::Vector3f* pMax) const {
    *pMin = sead::Vector3f(mData->mMin);
    *pMax = sead::Vector3f(mData->mMax);
}

/**
 * Gets the size of the octree area of a model.
 * @param pSize output size
 * @param pHeader model header
 */
void KCollisionServer::getAreaSpaceSize(sead::Vector3f* pSize,
                                        const KCPrismHeader* pHeader) const {
    s32 sizeX;
    s32 sizeY;
    s32 sizeZ;
    getAreaSpaceSize(&sizeX, &sizeY, &sizeZ, pHeader);
    pSize->set(sizeX, sizeY, sizeZ);
}

/**
 * Gets the size of the octree area of a model.
 * @param pSizeX output size on x
 * @param pSizeY output size on y
 * @param pSizeZ output size on z
 * @param pHeader model header
 */
void KCollisionServer::getAreaSpaceSize(s32* pSizeX, s32* pSizeY, s32* pSizeZ,
                                        const KCPrismHeader* pHeader) const {
    *pSizeX = ~pHeader->mWidthMask[0];
    *pSizeY = ~pHeader->mWidthMask[1];
    *pSizeZ = ~pHeader->mWidthMask[2];
}

/**
 * Gets the size of the octree area of a model.
 * @param pSize output size
 * @param pHeader model header
 */
void KCollisionServer::getAreaSpaceSize(sead::Vector3u* pSize,
                                        const KCPrismHeader* pHeader) const {
    pSize->x = ~pHeader->mWidthMask[0];
    pSize->y = ~pHeader->mWidthMask[1];
    pSize->z = ~pHeader->mWidthMask[2];
}

/**
 * Finds a prism containing a point.
 * @param pPos point
 * @param thicknessScale scale applied to the prism thickness
 * @param pDist output depth below the face
 * @return prism, null if none
 */
const KCPrismData* KCollisionServer::checkPoint(KCFxyz* pPos, f32 thicknessScale, f32* pDist) {
    for (s32 i = 0; i < getNumInnerKcl(); i++) {
        const KCPrismHeader* header = getV1Header(i);
        sead::Vector3u block;
        block.x = static_cast<s32>(pPos->x - header->mOctreeOrigin.x);

        if ((block.x & header->mWidthMask[0]) != 0) {
            continue;
        }

        block.y = static_cast<s32>(pPos->y - header->mOctreeOrigin.y);

        if ((block.y & header->mWidthMask[1]) != 0) {
            continue;
        }

        block.z = static_cast<s32>(pPos->z - header->mOctreeOrigin.z);

        if ((block.z & header->mWidthMask[2]) != 0) {
            continue;
        }

        f32 thickness = header->mThickness * thicknessScale;
        s32 shift;
        const u16* prismIndex = searchBlock(&shift, block, header);

        while (*++prismIndex != 0xffff) {
            const KCPrismData* data = getPrismData(*prismIndex, header);

            if (data->mLength <= 0.0f) {
                continue;
            }

            const sead::Vector3f& vertex = getVertexData(data->mPosIndex, header);
            sead::Vector3f pos = *pPos - vertex;

            if (pos.dot(getEdgeNormal1(data, header)) > 0.0f) {
                continue;
            }

            if (pos.dot(getEdgeNormal2(data, header)) > 0.0f) {
                continue;
            }

            if (pos.dot(getEdgeNormal3(data, header)) > data->mLength) {
                continue;
            }

            const sead::Vector3f& faceNormal = getFaceNormal(data, header);
            f32 dist = -pos.x * faceNormal.x - pos.y * faceNormal.y - pos.z * faceNormal.z;

            if (!(dist < 0.0f) && !(thickness < dist)) {
                *pDist = dist;
                return data;
            }
        }
    }

    return nullptr;
}

const u16* KCollisionServer::searchBlock(s32* pShift, const sead::Vector3u& rBlock,
                                         const KCPrismHeader* pHeader) const {
    u32 offset = calcAreaBlockOffset(rBlock, pHeader);
    const u8* octree = getOffsetPtr<u8>(pHeader, pHeader->mOctreeOffset);
    *pShift = pHeader->mBlockWidthShift;

    if (pHeader->mAreaXYWidthShift == -1 && pHeader->mAreaXWidthShift == -1) {
        offset = 0;
    }

    uintptr_t data = getBlockData(reinterpret_cast<const u32*>(octree), offset);

    while ((data & 0x80000000) == 0) {
        octree += data;
        (*pShift)--;
        offset = calcChildBlockOffset(rBlock, *pShift);
        data = getBlockData(reinterpret_cast<const u32*>(octree), offset);
    }

    return getOffsetPtr<u16>(octree, data & 0x7fffffff);
}

s32 KCollisionServer::checkSphere(KCFxyz* pPos, f32 radius, f32 thicknessScale, u32 maxHitNum,
                                  HitInfoBuffer* pHitInfos) {
    KCHitInfo hitInfo;
    hitInfo.mHeader = nullptr;
    hitInfo.mData = nullptr;
    hitInfo.mDist = 0.0f;
    hitInfo.mCollisionLocation = 0;
    sead::Vector3f max(pPos->x + radius, pPos->y + radius, pPos->z + radius);
    sead::Vector3f min(pPos->x - radius, pPos->y - radius, pPos->z - radius);
    const u16* lastPrismList = nullptr;
    const u16* skipPrismList = nullptr;
    u32 hitNum = 0;

    for (s32 i = 0; i < getNumInnerKcl(); i++) {
        const KCPrismHeader* header = getV1Header(i);
        sead::Vector3u blockMin;
        sead::Vector3u blockMax;

        if (!outCheckAndCalcArea(&blockMin, &blockMax, min, max, header)) {
            continue;
        }

        u32 z = blockMin.z;

        do {
            s32 stepZ = 1000000;

            u32 y = blockMin.y;

            do {
                s32 maxStepY = 0;
                s32 stepY = 1000000;

                u32 x = blockMin.x;

                do {
                    sead::Vector3u block(x, y, z);
                    s32 shift;
                    const u16* prismList = searchBlock(&shift, block, header);
                    s32 blockSize = 1 << shift;
                    s32 blockMask = blockSize - 1;
                    s32 offsetX = blockMask & x;
                    s32 offsetY = blockMask & y;
                    s32 offsetZ = blockMask & z;
                    stepZ = sead::Mathi::min(blockSize - offsetZ, stepZ);
                    stepY = sead::Mathi::min(blockSize - offsetY, stepY);

                    if (blockSize - offsetY > maxStepY && prismList[1] != 0) {
                        maxStepY = blockSize - offsetY;
                        lastPrismList = prismList;
                    }

                    s32 stepX = blockSize - offsetX;

                    if (skipPrismList == nullptr || skipPrismList != prismList) {
                        while (*++prismList != 0xffff) {
                            const KCPrismData* data = getPrismData(*prismList, header);
                            hitInfo.mData = data;

                            if (data->mLength <= 0.0f) {
                                continue;
                            }

                            u32 hitIndex = 0;

                            for (; hitIndex < static_cast<u32>(pHitInfos->size()); hitIndex++) {
                                if ((*pHitInfos)(hitIndex).mData == data) {
                                    break;
                                }
                            }

                            if (hitIndex != static_cast<u32>(pHitInfos->size())) {
                                continue;
                            }

                            bool isHit = KCHitSphere(data, header, pPos, radius, thicknessScale,
                                                     &hitInfo.mDist, &hitInfo.mCollisionLocation);

                            if (hitNum < maxHitNum && isHit) {
                                hitInfo.mHeader = header;
                                pHitInfos->pushBack(hitInfo);
                                hitNum++;
                            }
                        }
                    }

                    x += stepX;
                } while (x <= blockMax.x);

                skipPrismList = lastPrismList;
                y += stepY;
            } while (y <= blockMax.y);

            z += stepZ;
        } while (z <= blockMax.z);
    }

    return hitNum;
}

/**
 * Converts a box to clamped octree block coordinates.
 * @param pBlockMin output minimum block
 * @param pBlockMax output maximum block
 * @param rMin box minimum
 * @param rMax box maximum
 * @param pHeader model header
 * @return true if the box overlaps the area
 */
bool KCollisionServer::outCheckAndCalcArea(sead::Vector3u* pBlockMin, sead::Vector3u* pBlockMax,
                                           const sead::Vector3f& rMin, const sead::Vector3f& rMax,
                                           const KCPrismHeader* pHeader) const {
    objectSpaceToAreaOffsetSpace(pBlockMin, rMin, pHeader);
    objectSpaceToAreaOffsetSpace(pBlockMax, rMax, pHeader);

    if (static_cast<s32>(pBlockMin->x) < 0) {
        pBlockMin->x = 0;
    }

    if (static_cast<s32>(pBlockMin->y) < 0) {
        pBlockMin->y = 0;
    }

    if (static_cast<s32>(pBlockMin->z) < 0) {
        pBlockMin->z = 0;
    }

    if (static_cast<s32>(~pHeader->mWidthMask[0]) < static_cast<s32>(pBlockMax->x)) {
        pBlockMax->x = ~pHeader->mWidthMask[0];
    }

    if (static_cast<s32>(~pHeader->mWidthMask[1]) < static_cast<s32>(pBlockMax->y)) {
        pBlockMax->y = ~pHeader->mWidthMask[1];
    }

    if (static_cast<s32>(~pHeader->mWidthMask[2]) < static_cast<s32>(pBlockMax->z)) {
        pBlockMax->z = ~pHeader->mWidthMask[2];
    }

    for (s32 i = 0; i < 3; i++) {
        if (static_cast<s32>(pBlockMax->e[i]) < static_cast<s32>(pBlockMin->e[i])) {
            return false;
        }
    }

    return true;
}

bool KCollisionServer::KCHitSphere(const KCPrismData* pData, const KCPrismHeader* pHeader,
                                   const KCFxyz* pPos, f32 radius, f32 thicknessScale, f32* pDist,
                                   u8* pLocation) {
    f32 thickness = pHeader->mThickness;
    *pLocation = 0;
    const sead::Vector3f& vertex = getVertexData(pData->mPosIndex, pHeader);
    sead::Vector3f pos = *pPos - vertex;

    const sead::Vector3f& edge1 = getEdgeNormal1(pData, pHeader);
    f32 dist1 = pos.dot(edge1);

    if (dist1 >= radius) {
        return false;
    }

    const sead::Vector3f& edge2 = getEdgeNormal2(pData, pHeader);
    f32 dist2 = pos.dot(edge2);

    if (dist2 >= radius) {
        return false;
    }

    const sead::Vector3f& edge3 = getEdgeNormal3(pData, pHeader);
    f32 dist3 = pos.dot(edge3) - pData->mLength;

    if (dist3 >= radius) {
        return false;
    }

    f32 faceDist = pos.dot(getFaceNormal(pData, pHeader));
    *pDist = radius - faceDist;

    if (*pDist < 0.0f) {
        return false;
    }

    f32 radiusSq = radius * radius;
    f32 maxDepth = thickness * thicknessScale;
    f32 cos;
    f32 cornerDistSq;
    f32 cornerDist;
    sead::Vector3f cornerVec;

    if (dist1 > dist2) {
        if (dist1 > dist3) {
            if (dist1 <= 0.0f) {
                return checkHitFace(pDist, pLocation, maxDepth);
            }

            if (dist2 > dist3) {
                cos = edge1.dot(edge2);

                if (dist1 * cos > dist2) {
                    goto hitEdge1;
                }

                goto hitCorner12;
            }

            cos = edge1.dot(edge3);

            if (dist1 * cos > dist3) {
                goto hitEdge1;
            }

            goto hitCorner31;
        }

        goto major3;
    }

    if (dist2 > dist3) {
        if (dist2 <= 0.0f) {
            return checkHitFace(pDist, pLocation, maxDepth);
        }

        if (dist3 > dist1) {
            cos = edge2.dot(edge3);

            if (dist2 * cos > dist3) {
                goto hitEdge2;
            }

            goto hitCorner23;
        }

        cos = edge2.dot(edge1);

        if (dist2 * cos > dist1) {
            goto hitEdge2;
        }

        goto hitCorner12;
    }

major3:
    if (dist3 <= 0.0f) {
        return checkHitFace(pDist, pLocation, maxDepth);
    }

    if (dist1 > dist2) {
        cos = edge3.dot(edge1);

        if (dist3 * cos > dist1) {
            goto hitEdge3;
        }

        goto hitCorner31;
    }

    cos = edge3.dot(edge2);

    if (dist3 * cos > dist2) {
        goto hitEdge3;
    }

    goto hitCorner23;

hitEdge1:
    if (dist1 > faceDist) {
        return false;
    }

    *pDist = radiusSq - dist1 * dist1;
    *pLocation = 2;
    goto calcDepth;

hitEdge2:
    if (dist2 > faceDist) {
        return false;
    }

    *pDist = radiusSq - dist2 * dist2;
    *pLocation = 3;
    goto calcDepth;

hitEdge3:
    if (dist3 > faceDist) {
        return false;
    }

    *pDist = radiusSq - dist3 * dist3;
    *pLocation = 4;
    goto calcDepth;

hitCorner12:
    calcCornerVec(&cornerVec, dist1, dist2, edge1, edge2, cos);
    *pLocation = 5;
    goto hitCorner;

hitCorner23:
    calcCornerVec(&cornerVec, dist2, dist3, edge2, edge3, cos);
    *pLocation = 6;
    goto hitCorner;

hitCorner31:
    calcCornerVec(&cornerVec, dist3, dist1, edge3, edge1, cos);
    *pLocation = 7;

hitCorner:
    cornerDistSq = cornerVec.squaredLength();
    cornerDist = sead::Mathf::sqrt(cornerDistSq);

    if (cornerDist >= radius || cornerDist * 0.707106f > faceDist) {
        *pLocation = 0;
        return false;
    }

    *pDist = radiusSq - cornerDistSq;

calcDepth:
    *pDist = sead::Mathf::sqrt(*pDist) - faceDist;

    if (*pDist < 0.0f) {
        *pLocation = 0;
        return false;
    }

    return true;
}

const KCPrismData* KCollisionServer::checkArrow(const sead::Vector3f& rPos,
                                                const sead::Vector3f& rDir,
                                                HitInfoBuffer* pHitInfos, u32* pHitNum,
                                                u32 maxHitNum) const {
    if (rDir.x == 0.0f && rDir.y == 0.0f && rDir.z == 0.0f) {
        return nullptr;
    }

    f32 length;
    sead::Vector3f dir = rDir;
    separateScalarAndDirection(&length, &dir, dir);

    if (isNearZero(dir, 0.001f)) {
        return nullptr;
    }

    const KCPrismData* result = nullptr;
    u32 hitNum = 0;

    for (s32 i = 0; i < getNumInnerKcl(); i++) {
        const KCPrismHeader* header = getV1Header(i);
        sead::Vector3f pos;
        objectSpaceToAreaOffsetSpaceV3f(&pos, rPos, header);
        sead::Vector3u block(static_cast<s32>(pos.x), static_cast<s32>(pos.y),
                             static_cast<s32>(pos.z));
        f32 rate;

        if (isInsideMinMaxInAreaOffsetSpace(block, header)) {
            rate = 0.0f;
        } else {
            sead::Vector3u areaSize;
            getAreaSpaceSize(&areaSize, header);
            bool isEnter = false;

            if (dir.x != 0.0f) {
                f32 plane = dir.x > 0.0f ? 0.0f : areaSize.x;
                rate = (plane - pos.x) / dir.x;

                if (rate >= 0.0f && rate <= length) {
                    sead::Vector3f enterPos(pos.x + dir.x * rate, pos.y + rate * dir.y,
                                            pos.z + rate * dir.z);
                    block.set(static_cast<s32>(enterPos.x), static_cast<s32>(enterPos.y),
                              static_cast<s32>(enterPos.z));

                    if (isInsideMinMaxInAreaOffsetSpace(block, header)) {
                        pos = enterPos;
                        isEnter = true;
                    }
                }
            }

            if (!isEnter && dir.y != 0.0f) {
                f32 plane = dir.y > 0.0f ? 0.0f : areaSize.y;
                rate = (plane - pos.y) / dir.y;

                if (rate >= 0.0f && rate <= length) {
                    sead::Vector3f enterPos(pos.x + dir.x * rate, pos.y + dir.y * rate,
                                            pos.z + rate * dir.z);
                    block.set(static_cast<s32>(enterPos.x), static_cast<s32>(enterPos.y),
                              static_cast<s32>(enterPos.z));

                    if (isInsideMinMaxInAreaOffsetSpace(block, header)) {
                        pos = enterPos;
                        isEnter = true;
                    }
                }
            }

            if (!isEnter && dir.z != 0.0f) {
                f32 plane = dir.z > 0.0f ? 0.0f : areaSize.z;
                rate = (plane - pos.z) / dir.z;

                if (rate >= 0.0f && rate <= length) {
                    sead::Vector3f enterPos(pos.x + dir.x * rate, pos.y + dir.y * rate,
                                            pos.z + dir.z * rate);
                    block.set(static_cast<s32>(enterPos.x), static_cast<s32>(enterPos.y),
                              static_cast<s32>(enterPos.z));

                    if (isInsideMinMaxInAreaOffsetSpace(block, header)) {
                        pos = enterPos;
                        isEnter = true;
                    }
                }
            }

            if (!isEnter) {
                continue;
            }
        }

        s32 nextX;
        s32 nextY;
        s32 nextZ;
        s32 prevX;
        s32 prevY;
        s32 prevZ;
        s32* pDistX = dir.x < 0.0f ? &prevX : &nextX;
        s32 stepX = dir.x < 0.0f ? -1 : 1;
        s32* pDistY = dir.y < 0.0f ? &prevY : &nextY;
        s32 stepY = dir.y < 0.0f ? -1 : 1;
        s32* pDistZ = dir.z < 0.0f ? &prevZ : &nextZ;
        s32 stepZ = dir.z < 0.0f ? -1 : 1;

        do {
            s32 shift;
            const u16* prismList = searchBlock(&shift, block, header);
            s32 blockSize = 1 << shift;
            s32 blockMask = blockSize - 1;
            prevX = -(blockMask & block.x);
            nextX = blockSize - (blockMask & block.x);
            prevY = -(blockMask & block.y);
            nextY = blockSize - (blockMask & block.y);
            prevZ = -(blockMask & block.z);
            nextZ = blockSize - (blockMask & block.z);

            if (*pDistX == 0) {
                *pDistX = stepX;
            }

            if (*pDistY == 0) {
                *pDistY = stepY;
            }

            if (*pDistZ == 0) {
                *pDistZ = stepZ;
            }

            KCHitInfo hitInfo;
            hitInfo.mHeader = nullptr;
            hitInfo.mData = nullptr;
            hitInfo.mDist = 0.0f;
            hitInfo.mCollisionLocation = 0;

            if (pHitInfos != nullptr) {
                f32 minDist = 1.0f;

                while (*++prismList != 0xffff) {
                    const KCPrismData* data = getPrismData(*prismList, header);
                    hitInfo.mData = data;

                    if (data->mLength <= 0.0f) {
                        continue;
                    }

                    if (!KCHitArrow(data, header, rPos, rDir, &hitInfo.mDist,
                                    &hitInfo.mCollisionLocation)) {
                        continue;
                    }

                    hitInfo.mHeader = header;
                    pHitInfos->pushBack(hitInfo);
                    hitNum++;

                    if (hitInfo.mDist < minDist) {
                        minDist = hitInfo.mDist;
                        result = data;
                    }

                    if (hitNum == maxHitNum) {
                        if (pHitNum != nullptr) {
                            *pHitNum = maxHitNum;
                        }

                        return result;
                    }
                }
            } else {
                f32 minDist = 1.0f;

                while (*++prismList != 0xffff) {
                    const KCPrismData* data = getPrismData(*prismList, header);
                    hitInfo.mData = data;

                    if (data->mLength <= 0.0f) {
                        continue;
                    }

                    if (KCHitArrow(data, header, rPos, rDir, &hitInfo.mDist,
                                   &hitInfo.mCollisionLocation) &&
                        hitInfo.mDist < minDist) {
                        result = data;
                        minDist = hitInfo.mDist;
                    }
                }
            }

            if (pHitInfos == nullptr && result != nullptr) {
                break;
            }

            f32 rateX = isNearZero(dir.x, 0.001f) ? 1000000000.0f : *pDistX / dir.x;
            f32 rateY = isNearZero(dir.y, 0.001f) ? 1000000000.0f : *pDistY / dir.y;
            f32 rateZ = isNearZero(dir.z, 0.001f) ? 1000000000.0f : *pDistZ / dir.z;
            f32 minRate = rateY < rateX ? rateY : rateX;
            minRate = rateZ < minRate ? rateZ : minRate;

            if (length - rate <= minRate) {
                break;
            }

            pos.x += minRate * dir.x;
            block.x = static_cast<s32>(pos.x);

            if ((block.x & header->mWidthMask[0]) != 0) {
                break;
            }

            pos.y += minRate * dir.y;
            block.y = static_cast<s32>(pos.y);

            if ((block.y & header->mWidthMask[1]) != 0) {
                break;
            }

            pos.z += minRate * dir.z;
            block.z = static_cast<s32>(pos.z);
            rate += minRate;
        } while ((block.z & header->mWidthMask[2]) == 0 && rate < length);
    }

    if (pHitNum != nullptr) {
        *pHitNum = hitNum;
    }

    return result;
}

/**
 * Converts a position to the octree area space of a model.
 * @param pAreaPos output position
 * @param rPos position
 * @param pHeader model header
 */
void KCollisionServer::objectSpaceToAreaOffsetSpaceV3f(sead::Vector3f* pAreaPos,
                                                       const sead::Vector3f& rPos,
                                                       const KCPrismHeader* pHeader) const {
    *pAreaPos = rPos - pHeader->mOctreeOrigin;
}

/**
 * Checks whether a block is inside the octree area of a model.
 * @param rBlock block coordinates
 * @param pHeader model header
 * @return true if inside
 */
bool KCollisionServer::isInsideMinMaxInAreaOffsetSpace(const sead::Vector3u& rBlock,
                                                       const KCPrismHeader* pHeader) const {
    if ((rBlock.x & pHeader->mWidthMask[0]) != 0) {
        return false;
    }

    if ((rBlock.y & pHeader->mWidthMask[1]) != 0) {
        return false;
    }

    return (rBlock.z & pHeader->mWidthMask[2]) == 0;
}

/**
 * Checks a segment against a prism.
 * @param pData prism
 * @param pHeader model header
 * @param rPos segment start
 * @param rDir segment vector
 * @param pDist output hit rate along the segment
 * @param pLocation output hit location
 * @return true if hit
 */
bool KCollisionServer::KCHitArrow(const KCPrismData* pData, const KCPrismHeader* pHeader,
                                  const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                                  f32* pDist, u8* pLocation) const {
    const sead::Vector3f& vertex = getVertexData(pData->mPosIndex, pHeader);
    const sead::Vector3f& faceNormal = getFaceNormal(pData, pHeader);
    sead::Vector3f pos = rPos - vertex;
    f32 faceDist = pos.dot(faceNormal);

    if (faceDist <= 0.0f) {
        *pLocation = 0;
        return false;
    }

    f32 dirDot = faceNormal.dot(rDir);

    if (faceDist + dirDot > 0.0f) {
        *pLocation = 0;
        return false;
    }

    f32 rate = faceDist / -dirDot;
    sead::Vector3f hitPos(pos.x + rDir.x * rate, pos.y + rDir.y * rate, pos.z + rDir.z * rate);

    f32 dist1 = hitPos.dot(getEdgeNormal1(pData, pHeader));

    if (dist1 > 0.01f) {
        *pLocation = 0;
        return false;
    }

    bool isOnEdge1 = dist1 >= 0.0f && dist1 <= 0.01f;
    f32 dist2 = hitPos.dot(getEdgeNormal2(pData, pHeader));

    if (dist2 > 0.01f) {
        *pLocation = 0;
        return false;
    }

    bool isOnEdge2 = dist2 >= 0.0f && dist2 <= 0.01f;
    f32 dist3 = hitPos.dot(getEdgeNormal3(pData, pHeader));

    if (pData->mLength + 0.01f < dist3) {
        *pLocation = 0;
        return false;
    }

    bool isOnEdge3 = dist3 >= 0.0f && dist3 <= 0.01f;
    *pDist = rate;

    if (isOnEdge1) {
        if (isOnEdge2) {
            *pLocation = isOnEdge3 ? 1 : 5;
        } else {
            *pLocation = isOnEdge3 ? 7 : 2;
        }
    } else if (isOnEdge2) {
        *pLocation = isOnEdge3 ? 6 : 3;
    } else {
        *pLocation = isOnEdge3 ? 4 : 1;
    }

    return true;
}

s32 KCollisionServer::checkSphereForPlayer(KCFxyz* pPos, f32 radius, u32 maxHitNum,
                                           HitInfoBuffer* pHitInfos) {
    KCHitInfo hitInfo;
    hitInfo.mHeader = nullptr;
    hitInfo.mData = nullptr;
    hitInfo.mDist = 0.0f;
    hitInfo.mCollisionLocation = 0;
    sead::Vector3f max(pPos->x + radius, pPos->y + radius, pPos->z + radius);
    sead::Vector3f min(pPos->x - radius, pPos->y - radius, pPos->z - radius);
    const u16* lastPrismList = nullptr;
    const u16* skipPrismList = nullptr;
    u32 hitNum = 0;

    for (s32 i = 0; i < getNumInnerKcl(); i++) {
        const KCPrismHeader* header = getV1Header(i);
        sead::Vector3u blockMin;
        sead::Vector3u blockMax;

        if (!outCheckAndCalcArea(&blockMin, &blockMax, min, max, header)) {
            continue;
        }

        u32 z = blockMin.z;

        do {
            s32 stepZ = 1000000;

            u32 y = blockMin.y;

            do {
                s32 maxStepY = 0;
                s32 stepY = 1000000;

                u32 x = blockMin.x;

                do {
                    sead::Vector3u block(x, y, z);
                    s32 shift;
                    const u16* prismList = searchBlock(&shift, block, header);
                    s32 blockSize = 1 << shift;
                    s32 blockMask = blockSize - 1;
                    s32 offsetX = blockMask & x;
                    s32 offsetY = blockMask & y;
                    s32 offsetZ = blockMask & z;
                    stepZ = sead::Mathi::min(blockSize - offsetZ, stepZ);
                    stepY = sead::Mathi::min(blockSize - offsetY, stepY);

                    if (blockSize - offsetY > maxStepY && prismList[1] != 0) {
                        maxStepY = blockSize - offsetY;
                        lastPrismList = prismList;
                    }

                    s32 stepX = blockSize - offsetX;

                    if (skipPrismList == nullptr || skipPrismList != prismList) {
                        while (*++prismList != 0xffff) {
                            const KCPrismData* data = getPrismData(*prismList, header);
                            hitInfo.mData = data;

                            if (data->mLength <= 0.0f) {
                                continue;
                            }

                            u32 hitIndex = 0;

                            for (; hitIndex < static_cast<u32>(pHitInfos->size()); hitIndex++) {
                                if ((*pHitInfos)(hitIndex).mData == data) {
                                    break;
                                }
                            }

                            if (hitIndex != static_cast<u32>(pHitInfos->size())) {
                                continue;
                            }

                            bool isHit = KCHitSphereForPlayer(data, header, pPos, radius,
                                                              &hitInfo.mDist,
                                                              &hitInfo.mCollisionLocation);

                            if (hitNum < maxHitNum && isHit) {
                                hitInfo.mHeader = header;
                                pHitInfos->pushBack(hitInfo);
                                hitNum++;
                            }
                        }
                    }

                    x += stepX;
                } while (x <= blockMax.x);

                skipPrismList = lastPrismList;
                y += stepY;
            } while (y <= blockMax.y);

            z += stepZ;
        } while (z <= blockMax.z);
    }

    return hitNum;
}

bool KCollisionServer::KCHitSphereForPlayer(const KCPrismData* pData,
                                            const KCPrismHeader* pHeader, KCFxyz* pPos,
                                            f32 radius, f32* pDist, u8* pLocation) {
    *pLocation = 0;
    const sead::Vector3f& vertex = getVertexData(pData->mPosIndex, pHeader);
    sead::Vector3f pos = *pPos - vertex;

    const sead::Vector3f& edge1 = getEdgeNormal1(pData, pHeader);
    f32 dist1 = pos.dot(edge1);

    if (dist1 >= radius) {
        return false;
    }

    const sead::Vector3f& edge2 = getEdgeNormal2(pData, pHeader);
    f32 dist2 = pos.dot(edge2);

    if (dist2 >= radius) {
        return false;
    }

    const sead::Vector3f& edge3 = getEdgeNormal3(pData, pHeader);
    f32 dist3 = pos.dot(edge3) - pData->mLength;

    if (dist3 >= radius) {
        return false;
    }

    f32 faceDist = pos.dot(getFaceNormal(pData, pHeader));
    *pDist = radius - faceDist;

    if (*pDist < 0.0f) {
        return false;
    }

    f32 radiusSq = radius * radius;
    f32 cos;
    f32 cornerDistSq;
    f32 cornerDist;
    sead::Vector3f cornerVec;

    if (dist1 > dist2) {
        if (dist1 > dist3) {
            if (dist1 <= 0.0f) {
                return checkHitFace(pDist, pLocation, pHeader->mThickness);
            }

            if (dist2 > dist3) {
                cos = edge1.dot(edge2);

                if (dist1 * cos > dist2) {
                    goto hitEdge1;
                }

                goto hitCorner12;
            }

            cos = edge1.dot(edge3);

            if (dist1 * cos > dist3) {
                goto hitEdge1;
            }

            goto hitCorner31;
        }

        goto major3;
    }

    if (dist2 > dist3) {
        if (dist2 <= 0.0f) {
            return checkHitFace(pDist, pLocation, pHeader->mThickness);
        }

        if (dist3 > dist1) {
            cos = edge2.dot(edge3);

            if (dist2 * cos > dist3) {
                goto hitEdge2;
            }

            goto hitCorner23;
        }

        cos = edge2.dot(edge1);

        if (dist2 * cos > dist1) {
            goto hitEdge2;
        }

        goto hitCorner12;
    }

major3:
    if (dist3 <= 0.0f) {
        return checkHitFace(pDist, pLocation, pHeader->mThickness);
    }

    if (dist1 > dist2) {
        cos = edge3.dot(edge1);

        if (dist3 * cos > dist1) {
            goto hitEdge3;
        }

        goto hitCorner31;
    }

    cos = edge3.dot(edge2);

    if (dist3 * cos > dist2) {
        goto hitEdge3;
    }

    goto hitCorner23;

hitEdge1:
    if (dist1 > faceDist) {
        return false;
    }

    *pDist = radiusSq - dist1 * dist1;
    *pLocation = 2;
    goto calcDepth;

hitEdge2:
    if (dist2 > faceDist) {
        return false;
    }

    *pDist = radiusSq - dist2 * dist2;
    *pLocation = 3;
    goto calcDepth;

hitEdge3:
    if (dist3 > faceDist) {
        return false;
    }

    *pDist = radiusSq - dist3 * dist3;
    *pLocation = 4;
    goto calcDepth;

hitCorner12:
    calcCornerVec(&cornerVec, dist1, dist2, edge1, edge2, cos);
    *pLocation = 5;
    goto hitCorner;

hitCorner23:
    calcCornerVec(&cornerVec, dist2, dist3, edge2, edge3, cos);
    *pLocation = 6;
    goto hitCorner;

hitCorner31:
    calcCornerVec(&cornerVec, dist3, dist1, edge3, edge1, cos);
    *pLocation = 7;

hitCorner:
    cornerDistSq = cornerVec.squaredLength();
    cornerDist = sead::Mathf::sqrt(cornerDistSq);

    if (cornerDist >= radius || cornerDist * 0.707106f > faceDist) {
        *pLocation = 0;
        return false;
    }

    *pDist = radiusSq - cornerDistSq;

calcDepth:
    *pDist = sead::Mathf::sqrt(*pDist) - faceDist;

    if (*pDist < 0.0f) {
        *pLocation = 0;
        return false;
    }

    return true;
}

s32 KCollisionServer::checkDisk(KCFxyz* pPos, f32 radius, f32 halfHeight,
                                const sead::Vector3f& rNormal, f32 thicknessScale, u32 maxHitNum,
                                HitInfoBuffer* pHitInfos) {
    KCHitInfo hitInfo;
    hitInfo.mHeader = nullptr;
    hitInfo.mData = nullptr;
    hitInfo.mDist = 0.0f;
    hitInfo.mCollisionLocation = 0;
    f32 extent = sead::Mathf::sqrt(2.0f) * radius;
    sead::Vector3f max(pPos->x + extent, pPos->y + extent, pPos->z + extent);
    sead::Vector3f min(pPos->x - extent, pPos->y - extent, pPos->z - extent);
    const u16* lastPrismList = nullptr;
    const u16* skipPrismList = nullptr;
    u32 hitNum = 0;

    for (s32 i = 0; i < getNumInnerKcl(); i++) {
        const KCPrismHeader* header = getV1Header(i);
        sead::Vector3u blockMin;
        sead::Vector3u blockMax;

        if (!outCheckAndCalcArea(&blockMin, &blockMax, min, max, header)) {
            continue;
        }

        u32 z = blockMin.z;

        do {
            s32 stepZ = 1000000;

            u32 y = blockMin.y;

            do {
                s32 maxStepY = 0;
                s32 stepY = 1000000;

                u32 x = blockMin.x;

                do {
                    sead::Vector3u block(x, y, z);
                    s32 shift;
                    const u16* prismList = searchBlock(&shift, block, header);
                    s32 blockSize = 1 << shift;
                    s32 blockMask = blockSize - 1;
                    s32 offsetX = blockMask & x;
                    s32 offsetY = blockMask & y;
                    s32 offsetZ = blockMask & z;
                    stepZ = sead::Mathi::min(blockSize - offsetZ, stepZ);
                    stepY = sead::Mathi::min(blockSize - offsetY, stepY);

                    if (blockSize - offsetY > maxStepY && prismList[1] != 0) {
                        maxStepY = blockSize - offsetY;
                        lastPrismList = prismList;
                    }

                    s32 stepX = blockSize - offsetX;

                    if (skipPrismList == nullptr || skipPrismList != prismList) {
                        while (*++prismList != 0xffff) {
                            const KCPrismData* data = getPrismData(*prismList, header);
                            hitInfo.mData = data;

                            if (data->mLength <= 0.0f) {
                                continue;
                            }

                            u32 hitIndex = 0;

                            for (; hitIndex < static_cast<u32>(pHitInfos->size()); hitIndex++) {
                                if ((*pHitInfos)(hitIndex).mData == data) {
                                    break;
                                }
                            }

                            if (hitIndex != static_cast<u32>(pHitInfos->size())) {
                                continue;
                            }

                            bool isHit =
                                KCHitDisk(data, header, pPos, radius, thicknessScale, halfHeight,
                                          rNormal, &hitInfo.mDist, &hitInfo.mCollisionLocation);

                            if (hitNum < maxHitNum && isHit) {
                                hitInfo.mHeader = header;
                                pHitInfos->pushBack(hitInfo);
                                hitNum++;
                            }
                        }
                    }

                    x += stepX;
                } while (x <= blockMax.x);

                skipPrismList = lastPrismList;
                y += stepY;
            } while (y <= blockMax.y);

            z += stepZ;
        } while (z <= blockMax.z);
    }

    return hitNum;
}

bool KCollisionServer::KCHitDisk(const KCPrismData* pData, const KCPrismHeader* pHeader,
                                 KCFxyz* pPos, f32 radius, f32 thicknessScale, f32 halfHeight,
                                 const sead::Vector3f& rNormal, f32* pDist, u8* pLocation) {
    sead::Vector3f vertex;
    sead::Vector3f pos1;
    sead::Vector3f pos2;
    calcPosLocal(&vertex, pData, 0, pHeader);
    calcPosLocal(&pos1, pData, 1, pHeader);
    calcPosLocal(&pos2, pData, 2, pHeader);
    sead::Vector3f pos = *pPos;
    sead::Vector3f relPos0 = vertex - pos;
    sead::Vector3f relPos1 = pos1 - pos;
    sead::Vector3f relPos2 = pos2 - pos;
    f32 height0 = relPos0.dot(rNormal);
    f32 height1 = relPos1.dot(rNormal);
    f32 height2 = rNormal.dot(relPos2);
    *pLocation = 0;
    *pDist = 0.0f;

    if (height0 < -halfHeight && height1 < -halfHeight && height2 < -halfHeight) {
        return false;
    }

    if (height0 > halfHeight && height1 > halfHeight && height2 > halfHeight) {
        return false;
    }

    sead::Vector3f top = pos + rNormal * halfHeight;
    sead::Vector3f bottom = pos + rNormal * -halfHeight;
    sead::Vector3f relTop = top - vertex;
    sead::Vector3f relBottom = bottom - vertex;

    const sead::Vector3f& edge1 = getEdgeNormal1(pData, pHeader);
    f32 topDist1 = relTop.dot(edge1);
    f32 bottomDist1 = relBottom.dot(edge1);
    f32 radius1 =
        sinf(acosf(sead::Mathf::clamp(sead::Mathf::abs(rNormal.dot(edge1)), -1.0f, 1.0f))) * radius;

    if (topDist1 >= radius1 && bottomDist1 >= radius1) {
        return false;
    }

    f32 minDist1 = topDist1 < bottomDist1 ? topDist1 : bottomDist1;

    const sead::Vector3f& edge2 = getEdgeNormal2(pData, pHeader);
    f32 topDist2 = relTop.dot(edge2);
    f32 bottomDist2 = relBottom.dot(edge2);
    f32 radius2 =
        sinf(acosf(sead::Mathf::clamp(sead::Mathf::abs(rNormal.dot(edge2)), -1.0f, 1.0f))) * radius;

    if (topDist2 >= radius2 && bottomDist2 >= radius2) {
        return false;
    }

    f32 minDist2 = topDist2 < bottomDist2 ? topDist2 : bottomDist2;

    const sead::Vector3f& edge3 = getEdgeNormal3(pData, pHeader);
    f32 topDist3 = relTop.dot(edge3) - pData->mLength;
    f32 bottomDist3 = relBottom.dot(edge3) - pData->mLength;
    f32 radius3 =
        sinf(acosf(sead::Mathf::clamp(sead::Mathf::abs(rNormal.dot(edge3)), -1.0f, 1.0f))) * radius;

    if (topDist3 >= radius3 && bottomDist3 >= radius3) {
        return false;
    }

    f32 minDist3 = topDist3 < bottomDist3 ? topDist3 : bottomDist3;

    const sead::Vector3f& faceNormal = getFaceNormal(pData, pHeader);
    f32 topDistFace = relTop.dot(faceNormal);
    f32 bottomDistFace = relBottom.dot(faceNormal);
    f32 sinFace =
        sinf(acosf(sead::Mathf::clamp(sead::Mathf::abs(rNormal.dot(faceNormal)), -1.0f, 1.0f)));

    if (bottomDistFace <= 0.0f && topDistFace <= 0.0f) {
        return false;
    }

    f32 radiusFace = sinFace * radius;

    if (bottomDistFace > radiusFace && topDistFace > radiusFace) {
        return false;
    }

    f32 minDistFace = topDistFace < bottomDistFace ? topDistFace : bottomDistFace;
    *pDist = radiusFace - minDistFace;

    s32 edgeIndex = minDist1 > minDist2 ? (minDist1 > minDist3 ? 0 : 2) :
                                          (minDist2 > minDist3 ? 1 : 2);

    switch (edgeIndex) {
    case 0:
        if (minDist1 > 0.0f) {
            if (minDist1 > minDistFace) {
                return false;
            }

            if (sead::Mathf::sqrt(radius1 * radius1 - minDist1 * minDist1) - minDistFace < 0.0f) {
                return false;
            }
        }

        break;
    case 1:
        if (minDist2 > 0.0f) {
            if (minDist2 > minDistFace) {
                return false;
            }

            if (sead::Mathf::sqrt(radius2 * radius2 - minDist2 * minDist2) - minDistFace < 0.0f) {
                return false;
            }
        }

        break;
    case 2:
        if (minDist3 > 0.0f) {
            if (minDist3 > minDistFace) {
                return false;
            }

            if (sead::Mathf::sqrt(radius3 * radius3 - minDist3 * minDist3) - minDistFace < 0.0f) {
                return false;
            }
        }

        break;
    }

    *pLocation = 1;
    return true;
}

void KCollisionServer::searchPrism(KCFxyz* pPos, f32 radius, PrismDelegate& rDelegate) {
    sead::Vector3f max(pPos->x + radius, pPos->y + radius, pPos->z + radius);
    sead::Vector3f min(pPos->x - radius, pPos->y - radius, pPos->z - radius);
    const u16* lastPrismList = nullptr;
    const u16* skipPrismList = nullptr;

    for (s32 i = 0; i < getNumInnerKcl(); i++) {
        const KCPrismHeader* header = getV1Header(i);
        sead::Vector3u blockMin;
        sead::Vector3u blockMax;

        if (!outCheckAndCalcArea(&blockMin, &blockMax, min, max, header)) {
            continue;
        }

        u32 z = blockMin.z;

        do {
            s32 stepZ = 1000000;

            u32 y = blockMin.y;

            do {
                s32 maxStepY = 0;
                s32 stepY = 1000000;

                u32 x = blockMin.x;

                do {
                    sead::Vector3u block(x, y, z);
                    s32 shift;
                    const u16* prismList = searchBlock(&shift, block, header);
                    s32 blockSize = 1 << shift;
                    s32 blockMask = blockSize - 1;
                    s32 offsetX = blockMask & x;
                    s32 offsetY = blockMask & y;
                    s32 offsetZ = blockMask & z;
                    stepZ = sead::Mathi::min(blockSize - offsetZ, stepZ);
                    stepY = sead::Mathi::min(blockSize - offsetY, stepY);

                    if (blockSize - offsetY > maxStepY && prismList[1] != 0) {
                        maxStepY = blockSize - offsetY;
                        lastPrismList = prismList;
                    }

                    s32 stepX = blockSize - offsetX;

                    if (skipPrismList == nullptr || skipPrismList != prismList) {
                        while (*++prismList != 0xffff) {
                            KCPrismData* data =
                                const_cast<KCPrismData*>(getPrismData(*prismList, header));

                            if (data->mLength <= 0.0f) {
                                continue;
                            }

                            rDelegate.invoke(data, header);
                        }
                    }

                    x += stepX;
                } while (x <= blockMax.x);

                skipPrismList = lastPrismList;
                y += stepY;
            } while (y <= blockMax.y);

            z += stepZ;
        } while (z <= blockMax.z);
    }
}

bool KCollisionServer::isParallelNormal(const KCPrismData* pData,
                                        const KCPrismHeader* pHeader) const {
    sead::Vector3f edge1 = getEdgeNormal1(pData, pHeader);
    sead::Vector3f edge2 = getEdgeNormal2(pData, pHeader);
    sead::Vector3f edge3 = getEdgeNormal3(pData, pHeader);

    return edge1 == edge2 || edge1 == edge3 || edge2 == edge3 || edge1 == -edge2 ||
           edge1 == -edge3 || edge2 == -edge3;
}

/**
 * Gets the face normal of a prism.
 * @param pData prism
 * @param pHeader model header
 * @return face normal
 */
const sead::Vector3f& KCollisionServer::getFaceNormal(const KCPrismData* pData,
                                                      const KCPrismHeader* pHeader) const {
    return getNormal(pData->mFaceNormalIndex, pHeader);
}

/**
 * Gets the first edge normal of a prism.
 * @param pData prism
 * @param pHeader model header
 * @return edge normal
 */
const sead::Vector3f& KCollisionServer::getEdgeNormal1(const KCPrismData* pData,
                                                       const KCPrismHeader* pHeader) const {
    return getNormal(pData->mEdgeNormalIndex[0], pHeader);
}

/**
 * Gets the second edge normal of a prism.
 * @param pData prism
 * @param pHeader model header
 * @return edge normal
 */
const sead::Vector3f& KCollisionServer::getEdgeNormal2(const KCPrismData* pData,
                                                       const KCPrismHeader* pHeader) const {
    return getNormal(pData->mEdgeNormalIndex[1], pHeader);
}

/**
 * Gets the third edge normal of a prism.
 * @param pData prism
 * @param pHeader model header
 * @return edge normal
 */
const sead::Vector3f& KCollisionServer::getEdgeNormal3(const KCPrismData* pData,
                                                       const KCPrismHeader* pHeader) const {
    return getNormal(pData->mEdgeNormalIndex[2], pHeader);
}

bool KCollisionServer::KCHitDisc(const KCPrismData* pData, const KCPrismHeader* pHeader,
                                 const sead::Vector3f& rPos, const sead::Vector3f& rNormal,
                                 f32 radius, f32 angle, sead::Vector3f* pFixDir, f32* pDist) {
    sead::Vector3f pos0 = getVertexData(pData->mPosIndex, pHeader);
    KCFxyz faceNormal = static_cast<const KCFxyz&>(getFaceNormal(pData, pHeader));
    KCFxyz edge2 = static_cast<const KCFxyz&>(getEdgeNormal2(pData, pHeader));
    sead::Vector3f edge3 = getEdgeNormal3(pData, pHeader);
    f32 prismLength = pData->mLength;
    KCFxyz edge1 = static_cast<const KCFxyz&>(getEdgeNormal1(pData, pHeader));
    f32 cos = rNormal.dot(faceNormal);

    if (cos > cosf(angle)) {
        return false;
    }

    if (cos < cosf(180.0f - angle)) {
        return false;
    }

    if (cos == 1.0f) {
        return false;
    }

    if (cos == -1.0f) {
        return false;
    }

    KCFxyz cross1;
    calXvec(&edge2, &faceNormal, &cross1);
    sead::Vector3f pos1 =
        pos0 + cross1 * (prismLength / sead::Mathf::max(cross1.dot(edge3), sead::Mathf::epsilon()));
    KCFxyz cross2;
    calXvec(&faceNormal, &edge1, &cross2);
    sead::Vector3f pos2 =
        pos0 + cross2 * (prismLength / sead::Mathf::max(cross2.dot(edge3), sead::Mathf::epsilon()));

    f32 height0 = (pos0 - rPos).dot(rNormal);
    f32 height1 = (pos1 - rPos).dot(rNormal);
    f32 height2 = (pos2 - rPos).dot(rNormal);

    if (height2 <= 0.0f && height0 <= 0.0f && height1 <= 0.0f) {
        return false;
    }

    if (height2 > 0.0f && height0 > 0.0f && height1 > 0.0f) {
        return false;
    }

    sead::Vector3f crossPos[2];
    s32 crossNum = 0;

    if (height0 * height1 < 0.0f) {
        f32 rate0 = sead::Mathf::abs(height0);
        f32 rate1 = sead::Mathf::abs(height1);
        f32 invRate = 1.0f / (rate0 + rate1);
        crossPos[crossNum++] = pos0 * rate1 * invRate + pos1 * rate0 * invRate;
    }

    if (height1 * height2 < 0.0f) {
        f32 rate1 = sead::Mathf::abs(height1);
        f32 rate2 = sead::Mathf::abs(height2);
        f32 invRate = 1.0f / (rate1 + rate2);
        crossPos[crossNum++] = pos1 * rate2 * invRate + pos2 * rate1 * invRate;
    }

    if (height2 * height0 < 0.0f) {
        f32 rate2 = sead::Mathf::abs(height2);
        f32 rate0 = sead::Mathf::abs(height0);
        f32 invRate = 1.0f / (rate2 + rate0);
        crossPos[crossNum++] = pos2 * rate0 * invRate + pos0 * rate2 * invRate;
    }

    if (crossNum != 2) {
        return false;
    }

    KCFxyz edge;
    edge.set(crossPos[1] - crossPos[0]);
    KCFxyz cross;
    calXvec(static_cast<const KCFxyz*>(&rNormal), &edge, &cross);

    if (getFaceNormal(pData, pHeader).dot(cross) < 0.0f) {
        cross.set(-cross);
        edge.set(-edge);
        sead::Vector3f tmp = crossPos[0];
        crossPos[0] = crossPos[1];
        crossPos[1] = tmp;
    }

    f32 length = cross.length();

    if (length == 0.0f) {
        return false;
    }

    f32 invLength = 1.0f / length;
    sead::Vector3f fixDir = cross * invLength;
    sead::Vector3f startDiff = rPos - crossPos[0];
    f32 height = fixDir.dot(startDiff);

    if (height < 0.0f) {
        return false;
    }

    sead::Vector3f edgeDir = edge * invLength;
    f32 edgeRate = edgeDir.dot(startDiff);

    if (edgeRate >= 0.0f && edgeRate <= length) {
        f32 dist = (crossPos[0] + edgeDir * edgeRate - rPos).length();

        if (dist >= radius) {
            return false;
        }

        *pFixDir = fixDir;
        *pDist = radius - dist;
        return true;
    }

    sead::Vector3f endDiff = rPos - crossPos[1];
    f32 startDist = startDiff.length();
    f32 endDist = endDiff.length();

    if (startDist >= radius && endDist >= radius) {
        return false;
    }

    if (startDist > endDist) {
        sead::Vector3f dir = endDiff;
        normalizeOrZero(&dir);

        if (dir.dot(fixDir) < 0.70710677f) {
            return false;
        }

        f32 endRate = edgeDir.dot(endDiff);
        f32 sq = radius * radius - endRate * endRate;
        if (sq > 0.0f) {
            *pDist = sead::Mathf::sqrt(sq) - sead::Mathf::abs(fixDir.dot(endDiff));
        } else {
            *pDist = -sead::Mathf::abs(fixDir.dot(endDiff));
        }
    } else {
        sead::Vector3f dir = startDiff;
        normalizeOrZero(&dir);

        if (dir.dot(fixDir) < 0.70710677f) {
            return false;
        }

        f32 sq = radius * radius - edgeRate * edgeRate;
        if (sq > 0.0f) {
            *pDist = sead::Mathf::sqrt(sq) - sead::Mathf::abs(height);
        } else {
            *pDist = -sead::Mathf::abs(height);
        }
    }

    *pFixDir = fixDir;
    return true;
}

/**
 * Gets the index of a prism in its model.
 * @param pData prism
 * @param pHeader model header
 * @return prism index
 */
s32 KCollisionServer::toIndex(const KCPrismData* pData, const KCPrismHeader* pHeader) const {
    return pData - getPrismData(0, pHeader);
}

/**
 * Gets a normal of a model.
 * @param index normal index
 * @param pHeader model header
 * @return normal
 */
const sead::Vector3f& KCollisionServer::getNormal(u32 index, const KCPrismHeader* pHeader) const {
    return getOffsetPtr<sead::Vector3f>(pHeader, pHeader->mNormalsOffset)[static_cast<s32>(index)];
}

/**
 * Calculates the cross product of the second vector with the first.
 * @param pA first vector
 * @param pB second vector
 * @param pOut output vector
 */
void KCollisionServer::calXvec(const KCFxyz* pA, const KCFxyz* pB, KCFxyz* pOut) {
    pOut->x = pA->z * pB->y - pA->y * pB->z;
    pOut->y = pA->x * pB->z - pA->z * pB->x;
    pOut->z = pA->y * pB->x - pA->x * pB->y;
}

/**
 * Gets a vertex of a model.
 * @param index vertex index
 * @param pHeader model header
 * @return vertex
 */
const sead::Vector3f& KCollisionServer::getVertexData(u32 index,
                                                      const KCPrismHeader* pHeader) const {
    const sead::Vector3f* vertices =
        getOffsetPtr<sead::Vector3f>(pHeader, pHeader->mPositionsOffset);
    return vertices[static_cast<s32>(index)];
}

/**
 * Gets the number of vertices of a model.
 * @param pHeader model header
 * @return vertex count
 */
s32 KCollisionServer::getVertexNum(const KCPrismHeader* pHeader) const {
    return &getNormal(0, pHeader) - &getVertexData(0, pHeader);
}

/**
 * Gets the number of normals of a model.
 * @param pHeader model header
 * @return normal count
 */
s32 KCollisionServer::getNormalNum(const KCPrismHeader* pHeader) const {
    return getTriangleNum(pHeader) * 4;
}

/**
 * Gets the number of attribute elements.
 * @return always 0
 */
s32 KCollisionServer::getAttributeElementNum() const {
    return 0;
}

/**
 * Gets the attributes of a prism.
 * @param pIter output attribute iterator
 * @param triIndex prism index
 * @param pHeader model header
 * @return true if found
 */
bool KCollisionServer::getAttributes(ByamlIter* pIter, u32 triIndex,
                                     const KCPrismHeader* pHeader) const {
    return getAttributes(pIter, getPrismData(triIndex, pHeader));
}

/**
 * Gets the attributes of a prism.
 * @param pIter output attribute iterator
 * @param pData prism
 * @return true if found
 */
bool KCollisionServer::getAttributes(ByamlIter* pIter, const KCPrismData* pData) const {
    return mAttributeIter->tryGetIterByIndex(pIter, pData->mCollisionType);
}

/**
 * Converts a position to octree block coordinates of a model.
 * @param pAreaPos output block coordinates
 * @param rPos position
 * @param pHeader model header
 */
void KCollisionServer::objectSpaceToAreaOffsetSpace(sead::Vector3u* pAreaPos,
                                                    const sead::Vector3f& rPos,
                                                    const KCPrismHeader* pHeader) const {
    pAreaPos->x = static_cast<s32>(rPos.x - pHeader->mOctreeOrigin.x);
    pAreaPos->y = static_cast<s32>(rPos.y - pHeader->mOctreeOrigin.y);
    pAreaPos->z = static_cast<s32>(rPos.z - pHeader->mOctreeOrigin.z);
}

/**
 * Converts octree block coordinates of a model to a position.
 * @param pPos output position
 * @param rAreaPos block coordinates
 * @param pHeader model header
 */
void KCollisionServer::areaOffsetSpaceToObjectSpace(sead::Vector3f* pPos,
                                                    const sead::Vector3u& rAreaPos,
                                                    const KCPrismHeader* pHeader) const {
    sead::Vector3f areaPos(rAreaPos.x, rAreaPos.y, rAreaPos.z);
    *pPos = pHeader->mOctreeOrigin + areaPos;
}

/**
 * Converts a box given by a corner and a size to clamped octree block coordinates.
 * @param pPos box corner
 * @param pSize box size
 * @param pBlockMin output minimum block
 * @param pBlockMax output maximum block
 * @param pHeader model header
 * @return true if the box overlaps the area
 */
bool KCollisionServer::doBoxCheck(const sead::Vector3f* pPos, const sead::Vector3f* pSize,
                                  sead::Vector3u* pBlockMin, sead::Vector3u* pBlockMax,
                                  const KCPrismHeader* pHeader) {
    sead::Vector3f end = *pPos + *pSize;
    sead::Vector3f min;
    sead::Vector3f max;
    min.x = pPos->x >= end.x ? end.x : pPos->x;
    max.x = pPos->x >= end.x ? pPos->x : end.x;
    min.y = pPos->y >= end.y ? end.y : pPos->y;
    max.y = pPos->y >= end.y ? pPos->y : end.y;
    min.z = pPos->z >= end.z ? end.z : pPos->z;
    max.z = pPos->z >= end.z ? pPos->z : end.z;

    if (min.x == max.x) {
        min.x += -1.0f;
        max.x += 1.0f;
    }

    if (min.y == max.y) {
        min.y += -1.0f;
        max.y += 1.0f;
    }

    if (min.z == max.z) {
        min.z += -1.0f;
        max.z += 1.0f;
    }

    return outCheckAndCalcArea(pBlockMin, pBlockMax, min, max, pHeader);
}

/**
 * Calculates the offset of the top level octree node containing a block.
 * @param rBlock block coordinates
 * @param pHeader model header
 * @return byte offset
 */
u32 KCollisionServer::calcAreaBlockOffset(const sead::Vector3u& rBlock,
                                          const KCPrismHeader* pHeader) const {
    s32 shift = pHeader->mBlockWidthShift;
    return (((rBlock.z >> shift) << pHeader->mAreaXYWidthShift) |
            ((rBlock.y >> shift) << pHeader->mAreaXWidthShift) | (rBlock.x >> shift))
           << 2;
}

/**
 * Calculates the offset of the child octree node containing a block.
 * @param rBlock block coordinates
 * @param shift node size shift
 * @return byte offset
 */
u32 KCollisionServer::calcChildBlockOffset(const sead::Vector3u& rBlock, s32 shift) {
    return ((((rBlock.z >> shift) & 1) << 2) | (((rBlock.y >> shift) & 1) << 1) |
            ((rBlock.x >> shift) & 1))
           << 2;
}

/**
 * Reads an octree node.
 * @param pData octree node array
 * @param offset byte offset
 * @return node value
 */
u32 KCollisionServer::getBlockData(const u32* pData, u32 offset) {
    return *reinterpret_cast<const u32*>(reinterpret_cast<const u8*>(pData) + offset);
}
}  // namespace al
