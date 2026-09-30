#pragma once

#include <math/seadVector.h>

namespace al {
class BlockRailLink;
class BlockRailPartsGroup;
class BlockRailRider;

void setBlockRailRiderToNearestPos(BlockRailRider* pRider, const BlockRailPartsGroup* pGroup,
                                   const sead::Vector3f& rPos);
void calcNearBlockRailLinkAndCoord(BlockRailLink** pLink, f32* pRate,
                                   const BlockRailPartsGroup* pGroup, const sead::Vector3f& rPos);
void calcNearBlockRailPos(sead::Vector3f* pNearPos, const BlockRailPartsGroup* pGroup,
                          const sead::Vector3f& rPos);
void calcPosBlockRailRider(sead::Vector3f* pPos, const BlockRailRider* pRider);
void calcPosAndDirBlockRailRider(sead::Vector3f* pPos, sead::Vector3f* pDir,
                                 const BlockRailRider* pRider);
void moveBlockRailRider(BlockRailRider* pRider, f32 speed);
void moveBlockRailRiderAndCalcPos(sead::Vector3f* pPos, BlockRailRider* pRider, f32 speed);
void moveBlockRailRiderAndCalcPosDir(sead::Vector3f* pPos, sead::Vector3f* pDir,
                                     BlockRailRider* pRider, f32 speed);
void calcBlockRailClippingSphere(sead::Vector3f* pCenter, f32* pRadius,
                                 const BlockRailPartsGroup* pGroup,
                                 const sead::Vector3f& rBaseCenter, f32 baseRadius);
}  // namespace al
