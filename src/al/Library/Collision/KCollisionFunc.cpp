#include "Library/Collision/KCollisionFunc.hpp"

#include <attributes.h>

#include "Library/Collision/KCollisionServer.hpp"

namespace alKCollisionFunc {
namespace {
ALWAYS_INLINE inline void calcHitPos(sead::Vector3f* pHitPos, const al::KCollisionServer* pServer,
                       const sead::Vector3f& rPos, const al::KCPrismData& rData,
                       const al::KCPrismHeader* pHeader, u8 location) {
    switch (location) {
    case 1:
    case 2:
    case 3:
    case 4: {
        sead::Vector3f planePos;
        pServer->calcPosLocal(&planePos, &rData, 0, pHeader);
        projectToPlane(pHitPos, rPos, planePos,
                       pServer->getNormal(rData.mFaceNormalIndex, pHeader));

        if (location == 1) {
            return;
        }

        sead::Vector3f edgeNormal;
        sead::Vector3f edgeDiff = rPos;

        if (location == 2) {
            edgeNormal = pServer->getNormal(rData.mEdgeNormalIndex[0], pHeader);
            edgeDiff -= planePos;
        } else if (location == 3) {
            edgeNormal = pServer->getNormal(rData.mEdgeNormalIndex[1], pHeader);
            edgeDiff -= planePos;
        } else if (location == 4) {
            edgeNormal = pServer->getNormal(rData.mEdgeNormalIndex[2], pHeader);
            sead::Vector3f edgePos;
            pServer->calcPosLocal(&edgePos, &rData, 1, pHeader);
            edgeDiff -= edgePos;
        }

        f32 dist = edgeDiff.dot(edgeNormal);
        *pHitPos -= edgeNormal * dist;
        break;
    }

    case 5:
        pServer->calcPosLocal(pHitPos, &rData, 0, pHeader);
        break;
    case 6:
        pServer->calcPosLocal(pHitPos, &rData, 1, pHeader);
        break;
    case 7:
        pServer->calcPosLocal(pHitPos, &rData, 2, pHeader);
        break;
    }
}
}  // namespace

/**
 * Calculates the closest position on a prism to a sphere, given where the sphere hit the prism.
 * @param pHitPos output hit position
 * @param pServer collision server owning the prism
 * @param rPos center of the sphere
 * @param rData prism data
 * @param pHeader prism header
 * @param location face, edge or corner that was hit
 */
void calcSphereHitPos(sead::Vector3f* pHitPos, const al::KCollisionServer* pServer,
                      const sead::Vector3f& rPos, const al::KCPrismData& rData,
                      const al::KCPrismHeader* pHeader, u8 location) {
    calcHitPos(pHitPos, pServer, rPos, rData, pHeader, location);
}

/**
 * Projects a point onto a plane.
 * @param pOut output projected position
 * @param rPos position to project
 * @param rPlanePos point on the plane
 * @param rPlaneNormal normal of the plane
 */
void projectToPlane(sead::Vector3f* pOut, const sead::Vector3f& rPos,
                    const sead::Vector3f& rPlanePos, const sead::Vector3f& rPlaneNormal) {
    f32 dist = (rPos - rPlanePos).dot(rPlaneNormal);
    pOut->setScaleAdd(-dist, rPlaneNormal, rPos);
}

/**
 * Calculates the closest position on a prism to a disk, given where the disk hit the prism.
 * @param pHitPos output hit position
 * @param pServer collision server owning the prism
 * @param rPos center of the disk
 * @param radius radius of the disk
 * @param rDir normal of the disk
 * @param rData prism data
 * @param pHeader prism header
 * @param location face, edge or corner that was hit
 */
void calcDiskHitPos(sead::Vector3f* pHitPos, const al::KCollisionServer* pServer,
                    const sead::Vector3f& rPos, f32 radius, const sead::Vector3f& rDir,
                    const al::KCPrismData& rData, const al::KCPrismHeader* pHeader, u8 location) {
    calcHitPos(pHitPos, pServer, rPos, rData, pHeader, location);
}
}  // namespace alKCollisionFunc
