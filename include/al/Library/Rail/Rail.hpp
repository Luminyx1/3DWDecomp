#pragma once

#include <math/seadVector.h>

namespace al {
struct PlacementInfo;
class RailPart;

class Rail {
public:
    Rail();

    void init(const PlacementInfo& rInfo);
    void calcPos(sead::Vector3f* pPos, f32 distance) const;
    s32 getIncludedSection(const RailPart** pPart, f32* pPartDistance, f32 distance) const;
    void calcUpDir(sead::Vector3f* pUp, f32 distance) const;
    f32 normalizeLength(f32 distance) const;
    void calcDirection(sead::Vector3f* pDir, f32 distance) const;
    void calcPosDir(sead::Vector3f* pPos, sead::Vector3f* pDir, f32 distance) const;
    f32 getTotalLength() const;
    f32 getPartLength(s32 index) const;
    f32 getLengthToPoint(s32 index) const;
    void calcRailPointPos(sead::Vector3f* pPos, s32 index) const;
    void calcNearestRailPointPos(sead::Vector3f* pRailPos, const sead::Vector3f& rPos) const;
    f32 calcNearestRailPosCoord(const sead::Vector3f& rPos, f32 interval) const;
    f32 calcNearestRailPosCoord(const sead::Vector3f& rPos, f32 interval, f32* pDistance) const;
    f32 calcNearestRailPos(sead::Vector3f* pRailPos, const sead::Vector3f& rPos,
                           f32 interval) const;
    bool isNearRailPoint(f32 distance, f32 epsilon) const;
    bool isNearEndRailPoint(f32 distance, f32 epsilon) const;
    bool isNearStartRailPoint(f32 distance, f32 epsilon) const;
    s32 calcRailPointNum(f32 distance1, f32 distance2);
    f32 getIncludedSectionLength(f32* pPartDistance, f32* pLength, f32 distance) const;
    s32 getIncludedSectionIndex(f32 distance) const;
    void getAccels(s32 index, f32* pAccelStart, f32* pAccelEnd);
    bool getAngleS(s32 index, f32* pAngle);
    bool getAngleE(s32 index, f32* pAngle);
    bool isIncludeBezierRailPart() const;

    PlacementInfo* getRailPoint(s32 index) const { return mRailPoints[index]; }
    s32 getRailPartCount() const { return mRailPartCount; }
    s32 getRailPointsCount() const { return mRailPointsCount; }
    bool isClosed() const { return mIsClosed; }

private:
    PlacementInfo** mRailPoints = nullptr;
    RailPart* mRailPart = nullptr;
    s32 mRailPartCount = 0;
    s32 mRailPointsCount = 0;
    bool mIsClosed = false;
};
}  // namespace al
