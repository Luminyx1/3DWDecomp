#include "Project/Collision/CollisionPartsTriangle.hpp"

#include "Library/Collision/KCollisionServer.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/HitDb.hpp"

namespace al {
/**
 * Constructs an empty triangle.
 */
Triangle::Triangle() : mCollisionParts(nullptr), mPrismData(nullptr), mPrismHeader(nullptr) {
    mNormals[0] = sead::Vector3f::zero;
    mNormals[1] = sead::Vector3f::zero;
    mNormals[2] = sead::Vector3f::zero;
    mNormals[3] = sead::Vector3f::zero;
    mPos[0] = sead::Vector3f::zero;
    mPos[1] = sead::Vector3f::zero;
    mPos[2] = sead::Vector3f::zero;
}

/**
 * Constructs a triangle from a collision prism.
 * @param rParts collision parts owning the prism
 * @param pData prism data
 * @param pHeader prism header
 */
Triangle::Triangle(const CollisionParts& rParts, KCPrismData* pData,
                   const KCPrismHeader* pHeader) {
    fillData(rParts, pData, pHeader);
}

/**
 * Sets the triangle from a collision prism, transformed to world space.
 * @param rParts collision parts owning the prism
 * @param pData prism data
 * @param pHeader prism header
 */
void Triangle::fillData(const CollisionParts& rParts, KCPrismData* pData,
                        const KCPrismHeader* pHeader) {
    mCollisionParts = &rParts;
    mPrismData = pData;
    mPrismHeader = pHeader;

    KCollisionServer* server = rParts.mKColServer;
    const sead::Matrix34f& baseMtx = rParts.mBaseMtx;

    mNormals[0].setRotated(baseMtx, server->getFaceNormal(pData, mPrismHeader));
    mNormals[1].setRotated(baseMtx, server->getEdgeNormal1(pData, mPrismHeader));
    mNormals[2].setRotated(baseMtx, server->getEdgeNormal2(pData, mPrismHeader));
    mNormals[3].setRotated(baseMtx, server->getEdgeNormal3(pData, mPrismHeader));

    normalize(&mNormals[0]);
    normalize(&mNormals[1]);
    normalize(&mNormals[2]);
    normalize(&mNormals[3]);

    server->calcPosLocal(&mPos[0], pData, 0, mPrismHeader);
    server->calcPosLocal(&mPos[1], pData, 1, mPrismHeader);
    server->calcPosLocal(&mPos[2], pData, 2, mPrismHeader);

    mPos[0].mul(baseMtx);
    mPos[1].mul(baseMtx);
    mPos[2].mul(baseMtx);
}

/**
 * Sets the triangle from three world positions without a collision prism.
 * @param rPos0 first vertex
 * @param rPos1 second vertex
 * @param rPos2 third vertex
 */
void Triangle::fill(const sead::Vector3f& rPos0, const sead::Vector3f& rPos1,
                    const sead::Vector3f& rPos2) {
    mPos[0] = rPos0;
    mPos[1] = rPos1;
    mPos[2] = rPos2;

    sead::Vector3f edge01 = rPos1 - rPos0;
    sead::Vector3f edge02 = rPos2 - rPos0;
    sead::Vector3f edge12 = rPos2 - rPos1;

    mNormals[0].setCross(edge01, edge02);
    normalizeOrZero(&mNormals[0]);
    mNormals[1].setCross(edge01, mNormals[0]);
    normalizeOrZero(&mNormals[1]);
    mNormals[2].setCross(mNormals[0], edge02);
    normalizeOrZero(&mNormals[2]);
    mNormals[3].setCross(edge12, mNormals[0]);
    normalizeOrZero(&mNormals[3]);

    mPrismData = nullptr;
}

/**
 * Checks whether the collision parts of the triangle moved.
 * @return true if moved
 */
bool Triangle::isHostMoved() const {
    return mCollisionParts->_154 == 0;
}

/**
 * Checks whether the triangle references a collision prism.
 * @return true if valid
 */
bool Triangle::isValid() const {
    return mPrismData != nullptr;
}

/**
 * Gets a normal; 0 is the face normal, 1 to 3 are the edge normals.
 * @param index normal index
 * @return normal
 */
const sead::Vector3f* Triangle::getNormal(s32 index) const {
    return &mNormals[index];
}

/**
 * Gets the face normal.
 * @return face normal
 */
const sead::Vector3f* Triangle::getFaceNormal() const {
    return &mNormals[0];
}

/**
 * Gets an edge normal.
 * @param index edge index
 * @return edge normal
 */
const sead::Vector3f* Triangle::getEdgeNormal(s32 index) const {
    return &mNormals[index + 1];
}

/**
 * Gets a vertex.
 * @param index vertex index
 * @return vertex position
 */
const sead::Vector3f* Triangle::getPos(s32 index) const {
    return &mPos[index];
}

/**
 * Recalculates a normal from the collision prism.
 * @param index normal index; 0 is the face normal, 1 to 3 are the edge normals
 * @return recalculated normal
 */
sead::Vector3f* Triangle::calcAndGetNormal(s32 index) {
    const CollisionParts* parts = mCollisionParts;
    KCollisionServer* server = parts->mKColServer;

    switch (index) {
    case 0:
        return calcAndGetFaceNormal();
    case 1:
        mNormals[1] = server->getEdgeNormal1(mPrismData, mPrismHeader);
        mNormals[1].setRotated(parts->getBaseMtx(), mNormals[1]);
        normalize(&mNormals[1]);
        return &mNormals[index];
    case 2:
        mNormals[2] = server->getEdgeNormal2(mPrismData, mPrismHeader);
        mNormals[2].setRotated(parts->getBaseMtx(), mNormals[2]);
        normalize(&mNormals[2]);
        return &mNormals[index];
    case 3:
        mNormals[3] = server->getEdgeNormal3(mPrismData, mPrismHeader);
        mNormals[3].setRotated(parts->getBaseMtx(), mNormals[3]);
        normalize(&mNormals[3]);
        return &mNormals[index];
    default:
        return &mNormals[index];
    }
}

/**
 * Recalculates the face normal from the collision prism.
 * @return recalculated face normal
 */
sead::Vector3f* Triangle::calcAndGetFaceNormal() {
    const CollisionParts* parts = mCollisionParts;
    mNormals[0] = parts->mKColServer->getFaceNormal(mPrismData, mPrismHeader);
    mNormals[0].setRotated(parts->getBaseMtx(), mNormals[0]);
    normalize(&mNormals[0]);
    return &mNormals[0];
}

/**
 * Recalculates an edge normal from the collision prism.
 * @param index edge index
 * @return recalculated edge normal
 */
sead::Vector3f* Triangle::calcAndGetEdgeNormal(s32 index) {
    const CollisionParts* parts = mCollisionParts;
    KCollisionServer* server = parts->mKColServer;

    switch (index) {
    case 0:
        mNormals[1] = server->getEdgeNormal1(mPrismData, mPrismHeader);
        mNormals[1].rotate(parts->getBaseMtx());
        normalize(&mNormals[1]);
        return &mNormals[index + 1];
    case 1:
        mNormals[2] = server->getEdgeNormal2(mPrismData, mPrismHeader);
        mNormals[2].rotate(parts->getBaseMtx());
        normalize(&mNormals[2]);
        return &mNormals[index + 1];
    case 2:
        mNormals[3] = server->getEdgeNormal3(mPrismData, mPrismHeader);
        mNormals[3].rotate(parts->getBaseMtx());
        normalize(&mNormals[3]);
        return &mNormals[index + 1];
    default:
        return &mNormals[index + 1];
    }
}

/**
 * Recalculates a vertex from the collision prism.
 * @param index vertex index
 * @return recalculated vertex position
 */
sead::Vector3f* Triangle::calcAndGetPos(s32 index) {
    mCollisionParts->mKColServer->calcPosLocal(&mPos[index], mPrismData, index, mPrismHeader);
    mPos[index].mul(mCollisionParts->getBaseMtx());
    return &mPos[index];
}

/**
 * Calculates the centroid of the triangle.
 * @param pCenter output centroid
 */
void Triangle::calcCenterPos(sead::Vector3f* pCenter) const {
    *pCenter = (mPos[0] + mPos[1] + mPos[2]) * (1 / 3.0f);
}

/**
 * Gets a vertex in the local space of the collision parts.
 * @param pPos output position
 * @param index vertex index
 */
void Triangle::getLocalPos(sead::Vector3f* pPos, s32 index) const {
    mCollisionParts->mKColServer->calcPosLocal(pPos, mPrismData, index, mPrismHeader);
}

/**
 * Calculates how far the collision parts moved a position during the last frame.
 * @param pPower output movement
 * @param rPos position on the collision parts
 */
void Triangle::calcForceMovePower(sead::Vector3f* pPower, const sead::Vector3f& rPos) const {
    mCollisionParts->calcForceMovePower(pPower, rPos);
}

/**
 * Calculates how far the collision parts rotated during the last frame.
 * @param pPower output rotation
 */
void Triangle::calcForceRotatePower(sead::Quatf* pPower) const {
    mCollisionParts->calcForceRotatePower(pPower);
}

/**
 * Gets the attributes of the collision prism.
 * @param pIter output attribute iterator
 * @return true if the triangle is valid and has attributes
 */
bool Triangle::getAttributes(ByamlIter* pIter) const {
    if (!isValid()) {
        return false;
    }

    return mCollisionParts->mKColServer->getAttributes(pIter, mPrismData);
}

/**
 * Gets the sensor connected to the collision parts.
 * @return sensor
 */
HitSensor* Triangle::getSensor() const {
    return mCollisionParts->getSensor();
}

/**
 * Gets the transform of the collision parts.
 * @return base matrix
 */
const sead::Matrix34f* Triangle::getBaseMtx() const {
    return &mCollisionParts->getBaseMtx();
}

/**
 * Gets the inverse transform of the collision parts.
 * @return inverse base matrix
 */
const sead::Matrix34f* Triangle::getBaseInvMtx() const {
    return &mCollisionParts->getBaseInvMtx();
}

/**
 * Gets the transform of the collision parts in the previous frame.
 * @return previous base matrix
 */
const sead::Matrix34f* Triangle::getPrevBaseMtx() const {
    return &mCollisionParts->mPrevBaseMtx;
}
}  // namespace al

/**
 * Compares two triangles.
 * @param rLhs first triangle
 * @param rRhs second triangle
 * @return true if both reference the same prism of the same collision parts
 */
bool operator==(const al::Triangle& rLhs, const al::Triangle& rRhs) {
    return rLhs.mCollisionParts == rRhs.mCollisionParts && rLhs.mPrismData == rRhs.mPrismData;
}

/**
 * Compares two triangles.
 * @param rLhs first triangle
 * @param rRhs second triangle
 * @return true if the triangles differ
 */
bool operator!=(const al::Triangle& rLhs, const al::Triangle& rRhs) {
    return !(rLhs == rRhs);
}

namespace al {
/**
 * Constructs an empty hit info.
 */
HitInfo::HitInfo() = default;

/**
 * Checks whether the hit is on the face of the triangle.
 * @return true on the face
 */
bool HitInfo::isCollisionAtFace() const {
    return mCollisionLocation == CollisionLocation::Face;
}

/**
 * Checks whether the hit is on an edge of the triangle.
 * @return true on an edge
 */
bool HitInfo::isCollisionAtEdge() const {
    return mCollisionLocation == CollisionLocation::Edge1 ||
           mCollisionLocation == CollisionLocation::Edge2 ||
           mCollisionLocation == CollisionLocation::Edge3;
}

/**
 * Checks whether the hit is on a corner of the triangle.
 * @return true on a corner
 */
bool HitInfo::isCollisionAtCorner() const {
    return mCollisionLocation == CollisionLocation::Corner1 ||
           mCollisionLocation == CollisionLocation::Corner2 ||
           mCollisionLocation == CollisionLocation::Corner3;
}

/**
 * Calculates the vector that pushes a sphere out of the hit.
 * @param pFix output push vector
 * @param pFixNormal output push direction scaled by the face normal component
 */
void SphereHitInfo::calcFixVector(sead::Vector3f* pFix, sead::Vector3f* pFixNormal) const {
    if (isCollisionAtFace()) {
        calcFixVectorNormal(pFix, pFixNormal);
        return;
    }

    sead::Vector3f dir;
    dir.x = _80.x - mPos.x;
    dir.y = _80.y - mPos.y;
    dir.z = _80.z - mPos.z;
    normalizeOrZero(&dir);

    const sead::Vector3f& faceNormal = *mTriangle.getFaceNormal();
    f32 fixLength = dir.dot(faceNormal * _70);
    f32 normalLength = dir.dot(faceNormal);
    *pFix = dir * fixLength;
    *pFixNormal = dir * normalLength;
}

/**
 * Calculates the vector that pushes a sphere out of a face hit.
 * @param pFix output push vector
 * @param pFixNormal output face normal, may be null
 */
void SphereHitInfo::calcFixVectorNormal(sead::Vector3f* pFix, sead::Vector3f* pFixNormal) const {
    f32 length = _70;
    pFix->x = mTriangle.getFaceNormal()->x * length;
    pFix->y = mTriangle.getFaceNormal()->y * length;
    pFix->z = mTriangle.getFaceNormal()->z * length;

    if (pFixNormal) {
        pFixNormal->set(*mTriangle.getFaceNormal());
    }
}

/**
 * Calculates the vector that pushes a disk out of the hit.
 * @param pFix output push vector
 * @param pFixNormal output push direction scaled by the face normal component
 */
void DiskHitInfo::calcFixVector(sead::Vector3f* pFix, sead::Vector3f* pFixNormal) const {
    if (isCollisionAtFace()) {
        calcFixVectorNormal(pFix, pFixNormal);
        return;
    }

    sead::Vector3f dir;
    dir.x = _80.x - mPos.x;
    dir.y = _80.y - mPos.y;
    dir.z = _80.z - mPos.z;
    normalizeOrZero(&dir);

    const sead::Vector3f& faceNormal = *mTriangle.getFaceNormal();
    f32 fixLength = dir.dot(faceNormal * _70);
    f32 normalLength = dir.dot(faceNormal);
    *pFix = dir * fixLength;
    *pFixNormal = dir * normalLength;
}

/**
 * Calculates the vector that pushes a disk out of a face hit.
 * @param pFix output push vector
 * @param pFixNormal output face normal, may be null
 */
void DiskHitInfo::calcFixVectorNormal(sead::Vector3f* pFix, sead::Vector3f* pFixNormal) const {
    f32 length = _70;
    pFix->x = mTriangle.getFaceNormal()->x * length;
    pFix->y = mTriangle.getFaceNormal()->y * length;
    pFix->z = mTriangle.getFaceNormal()->z * length;

    if (pFixNormal) {
        pFixNormal->set(*mTriangle.getFaceNormal());
    }
}
}  // namespace al
