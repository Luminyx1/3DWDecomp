#pragma once

#include <math/seadVector.h>

namespace al {
struct PlacementInfo;
class RailPart;

class Rail {
public:
    Rail();

    void init(const PlacementInfo&);
    void calcPos(sead::Vector3f*, f32) const;
    s32 getIncludedSection(const RailPart**, f32*, f32) const;
    void calcUpDir(sead::Vector3f*, f32) const;
    f32 normalizeLength(f32) const;
    void calcDirection(sead::Vector3f*, f32) const;
    void calcPosDir(sead::Vector3f*, sead::Vector3f*, f32) const;
    f32 getTotalLength() const;
    f32 getPartLength(s32) const;
    f32 getLengthToPoint(s32) const;
    void calcRailPointPos(sead::Vector3f*, s32) const;
    void calcNearestRailPointPos(sead::Vector3f*, const sead::Vector3f&) const;
    f32 calcNearestRailPosCoord(const sead::Vector3f&, f32) const;
    f32 calcNearestRailPos(sead::Vector3f*, const sead::Vector3f&, f32) const;
    bool isNearRailPoint(f32, f32) const;
    bool isNearEndRailPoint(f32, f32) const;
    bool isNearStartRailPoint(f32, f32) const;
    s32 calcRailPointNum(f32, f32);
    f32 getIncludedSectionLength(f32*, f32*, f32) const;
    s32 getIncludedSectionIndex(f32) const;
    void getAccels(s32, f32*, f32*);
    bool getAngleS(s32, f32*);
    bool getAngleE(s32, f32*);
    bool isIncludeBezierRailPart() const;

    PlacementInfo* getRailPoint(s32 index) const { return mRailPoints[index]; }

    s32 getRailPartCount() const { return mRailPartCount; }

    s32 getRailPointsCount() const { return mRailPointsCount; }

    bool isClosed() const { return mIsClosed; }

private:
    PlacementInfo** mRailPoints = nullptr;  // _0
    RailPart* mRailParts = nullptr;         // _8
    s32 mRailPartCount = 0;                 // _10
    s32 mRailPointsCount = 0;               // _14
    bool mIsClosed = false;                 // _18
};

}  // namespace al
